"""Blender side of ttk_anim.py (run by it: blender -b --python this -- ...).

export SPEC.json OUT.blend: armature with one bone per joint (rest orientation
identity, so a bone's pose quaternion is the joint's local rotation), one mesh
with rigid vertex groups and the original texture pages, and one action per
clip (a key per original frame, at the frame's start tick; ticks are 1/250 s,
scene fps 250).
import OUT.blend ACTION TEMPLATE.json DUMP.json: per template frame, each bone's
pose quaternion and the root translation, sampled at the frame's start tick.

Axes: the armature object is rotated -90 degrees about X so the game's Y-down
coordinates stand upright in Blender (Z up); joint data stay in game units.
"""
import json
import math
import sys

import bpy
from mathutils import Matrix, Quaternion, Vector

args = sys.argv[sys.argv.index('--') + 1:]


def ok():
    print('TTK_BLENDER_OK')


def clear():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def frame_starts(clip):
    t, out = 0, []
    for f in clip['frames']:
        out.append(t)
        t += max(1, f['ticks'])
    return out, t


def export(spec_path, out_path):
    spec = json.load(open(spec_path))
    clear()
    scene = bpy.context.scene
    scene.render.fps = 250
    arm_data = bpy.data.armatures.new('ttk_skeleton')
    arm = bpy.data.objects.new('TTK_Duke', arm_data)
    scene.collection.objects.link(arm)
    arm.rotation_euler = (-math.pi / 2, 0, 0)
    arm['ttk_group'] = spec['group']
    arm['ttk_hierarchy'] = spec['hierarchy']
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode='EDIT')
    bones = []
    for j in spec['joints']:
        b = arm_data.edit_bones.new(j['name'])
        head = Vector(j['head'])
        b.head = head
        b.tail = head + Vector((0, 40, 0))
        b.roll = 0
        b['ttk_rot_id'] = j['rot_id']
        if j['parent'] >= 0:
            b.parent = bones[j['parent']]
        bones.append(b)
    bpy.ops.object.mode_set(mode='OBJECT')
    for pb in arm.pose.bones:
        pb.rotation_mode = 'QUATERNION'
    # materials per texture page
    mats = {}
    for key, path in spec['pages'].items():
        m = bpy.data.materials.new(f'page_{key}')
        m.use_nodes = True
        nt = m.node_tree
        bsdf = nt.nodes.get('Principled BSDF')
        tex = nt.nodes.new('ShaderNodeTexImage')
        tex.image = bpy.data.images.load(path)
        tex.image.pack()
        tex.interpolation = 'Closest'
        nt.links.new(tex.outputs['Color'], bsdf.inputs['Base Color'])
        nt.links.new(tex.outputs['Alpha'], bsdf.inputs['Alpha'])
        m.blend_method = 'CLIP'
        mats[key] = m
    verts, faces, face_mats, uvs, groups = [], [], [], [], []
    keys = list(mats)
    for mesh in spec['meshes']:
        base = len(verts)
        verts += [tuple(v) for v in mesh['vertices']]
        groups.append((mesh['joint'], list(range(base, len(verts)))))
        for f in mesh['faces']:
            faces.append(tuple(base + i for i in f['idx']))
            face_mats.append(keys.index(f['page']))
            uvs.append([((u + 0.5) / 256.0, 1.0 - (v + 0.5) / 256.0) for u, v in f['uv']])
    me = bpy.data.meshes.new('ttk_mesh')
    me.from_pydata(verts, [], faces, shade_flat=True)
    for m in keys:
        me.materials.append(mats[m])
    uv_layer = me.uv_layers.new(name='UV')
    for poly, fm, fuv in zip(me.polygons, face_mats, uvs):
        poly.material_index = fm
        for li, uv in zip(poly.loop_indices, fuv):
            uv_layer.data[li].uv = uv
    me.validate()
    ob = bpy.data.objects.new('TTK_Duke_mesh', me)
    scene.collection.objects.link(ob)
    ob.parent = arm
    for joint, idx in groups:
        vg = ob.vertex_groups.new(name=spec['joints'][joint]['name'])
        vg.add(idx, 1.0, 'REPLACE')
    mod = ob.modifiers.new('Armature', 'ARMATURE')
    mod.object = arm
    # actions
    by_rot = {pb.bone['ttk_rot_id']: pb for pb in arm.pose.bones}
    arm.animation_data_create()
    last = 0
    for act in spec['actions']:
        clip = act['clip']
        a = bpy.data.actions.new(act['name'])
        a.use_fake_user = True
        a['ttk_clip'] = clip['clip']
        a['ttk_group'] = clip['group']
        arm.animation_data.action = a
        starts, end = frame_starts(clip)
        for f, t in zip(clip['frames'], starts):
            for c in f['commands']:
                if 'bq' in c and c['joint'] in by_rot:
                    pb = by_rot[c['joint']]
                    pb.rotation_quaternion = Quaternion(c['bq'])
                    pb.keyframe_insert('rotation_quaternion', frame=t)
                elif 'root' in c and 0 in by_rot:
                    pb = by_rot[0]
                    pb.location = Vector(c['root'])
                    pb.keyframe_insert('location', frame=t)
        for fc in a.fcurves:
            for kp in fc.keyframe_points:
                kp.interpolation = 'LINEAR'
        last = max(last, end)
    if spec['actions']:
        arm.animation_data.action = bpy.data.actions[spec['actions'][0]['name']]
    scene.frame_start, scene.frame_end = 0, max(1, last)
    bpy.ops.wm.save_as_mainfile(filepath=out_path)
    ok()


def do_import(blend, action, template, dump):
    bpy.ops.wm.open_mainfile(filepath=blend)
    arm = next(o for o in bpy.data.objects if o.type == 'ARMATURE')
    act = bpy.data.actions[action]
    arm.animation_data.action = act
    # sample at the template's frame start ticks (the clip's timing)
    times, _ = frame_starts(json.load(open(template)))
    frames = []
    scene = bpy.context.scene
    for t in times:
        scene.frame_set(int(round(t)))
        bones = {}
        root = None
        for pb in arm.pose.bones:
            rid = pb.bone.get('ttk_rot_id')
            q = pb.rotation_quaternion.normalized()
            bones[str(rid)] = [q.w, q.x, q.y, q.z]
            if rid == 0:
                root = list(pb.location)
        frames.append(dict(time=t, bones=bones, root=root))
    json.dump(dict(action=action, frames=frames), open(dump, 'w'))
    ok()


if args[0] == 'export':
    export(args[1], args[2])
elif args[0] == 'import':
    do_import(args[1], args[2], args[3], args[4])
