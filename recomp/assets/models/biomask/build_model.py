import bpy,math,json
from pathlib import Path
from mathutils import Vector
O=Path(__file__).resolve().parent
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
scene=bpy.context.scene
scene.render.engine='CYCLES';scene.cycles.samples=48;scene.cycles.use_denoising=False
scene.render.film_transparent=True
scene.view_settings.view_transform='Standard';scene.view_settings.look='Medium High Contrast';scene.view_settings.exposure=0;scene.view_settings.gamma=1
atlas=bpy.data.images.load(str(O/'biomask-atlas.png'));atlas.pack()
def material(name,gain):
 m=bpy.data.materials.new(name);m.use_nodes=True;n=m.node_tree.nodes;bs=n.get('Principled BSDF');bs.inputs['Roughness'].default_value=.9;bs.inputs['Specular IOR Level'].default_value=.15
 tex=n.new('ShaderNodeTexImage');tex.image=atlas;tex.interpolation='Closest'
 mul=n.new('ShaderNodeMixRGB');mul.blend_type='MULTIPLY';mul.inputs[0].default_value=1;mul.inputs[2].default_value=(*gain,1)
 m.node_tree.links.new(tex.outputs['Color'],mul.inputs[1]);m.node_tree.links.new(mul.outputs[0],bs.inputs['Base Color']);return m
metal=material('CC0 worn steel - matte',(.25,.27,.30));rubber=material('CC0 dark rubber', (1.1,1.1,1.1));edge=material('CC0 steel edge',(.40,.42,.45))
model=[]
def mesh(name,vs,fs,mat,region='metal'):
 m=bpy.data.meshes.new(name);m.from_pydata(vs,[],fs);m.update();o=bpy.data.objects.new(name,m);bpy.context.collection.objects.link(o);o.data.materials.append(mat);model.append(o)
 uv=m.uv_layers.new(name='AtlasUV')
 for p in m.polygons:
  p.use_smooth=False
  for li in p.loop_indices:
   co=m.vertices[m.loops[li].vertex_index].co
   normal=p.normal
   if abs(normal.y)>=max(abs(normal.x),abs(normal.z)):u=(co.x+3)*.25;v=(co.z+.5)*1.25
   elif abs(normal.x)>abs(normal.z):u=(co.y+.5)*1.5;v=(co.z+.5)*1.25
   else:u=(co.x+3)*.25;v=(co.y+.5)*1.5
   u=u%1;v=v%1
   uv.data[li].uv=((.505 if region=='rubber' else .005)+u*.49,.01+v*.98)
 return o
# Open-bottom angular mouth housing; explicit extruded outline.
outline=[(-.70,-.42),(-.70,.04),(-.48,.28),(.48,.28),(.70,.04),(.70,-.42),(.46,-.42),(.33,-.12),(-.33,-.12),(-.46,-.42)]
def extrude(name,outline,front,back,mat):
 n=len(outline);vs=[(x,y,z) for y in [front,back] for x,z in outline];fs=[tuple(range(n-1,-1,-1)),tuple(range(n,n*2))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
 o=mesh(name,vs,fs,mat);return o
housing=extrude('Angular respirator housing',outline,-.19,.12,metal)
b= housing.modifiers.new('Single-segment machined bevel','BEVEL');b.width=.028;b.segments=1
# Narrow dark gasket underneath the metal shell.
gasket=extrude('Rubber inner gasket',[(x*.94,z-.025) for x,z in outline],-.14,.14,rubber)
for p in gasket.data.uv_layers.active.data:p.uv.x+=.5
# One hose is authored, the other is its geometric reflection.
N=8;ribs=16;length=1.72;vs=[];fs=[]
for j in range(ribs*4+1):
 t=j/(ribs*4);x=.67+length*t;z=-.08-.36*t-.1*t*t;y=.02+.12*t*t
 radius=[.137,.178,.178,.137][j%4]
 tangent=Vector((length,.24*t,-.36-.2*t)).normalized();a=Vector((0,-1,0));a=(a-tangent*a.dot(tangent)).normalized();b=tangent.cross(a)
 for k in range(N):
  q=Vector((x,y,z))+radius*(math.cos(2*math.pi*k/N+math.pi/8)*a+math.sin(2*math.pi*k/N+math.pi/8)*b);vs.append(tuple(q))
for j in range(ribs*4):
 for k in range(N):fs.append((j*N+k,j*N+(k+1)%N,(j+1)*N+(k+1)%N,(j+1)*N+k))
fs.extend([tuple(range(N-1,-1,-1)),tuple(range((ribs*4)*N,(ribs*4+1)*N))])
hose=mesh('Right hose - 16 regular ribs - octagonal',vs,fs,rubber,'rubber');mir=hose.modifiers.new('Exact left-right symmetry','MIRROR');mir.use_axis[0]=True
# Low sided collars and bolt heads, mirrored consistently.
def cylinder(name,loc,radius,depth,mat,axis=(1,0,-.2),vertices=8):
 bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=radius,depth=depth,location=loc)
 o=bpy.context.object;o.name=name;o.rotation_euler=Vector(axis).to_track_quat('Z','Y').to_euler();o.data.materials.append(mat);model.append(o)
 for p in o.data.polygons:p.use_smooth=False
 for uv in o.data.uv_layers.active.data:uv.uv.x=.005+uv.uv.x*.49
 return o
for side in [-1,1]:
 cylinder('Hose metal collar '+str(side),(side*.72,0,-.10),.194,.16,metal,(side,0,-.2))
 cylinder('Collar rim '+str(side),(side*.80,0,-.117),.202,.025,edge,(side,0,-.2))
 cylinder('Recessed bolt '+str(side),(side*.58,-.223,-.08),.043,.019,edge,(0,-1,0),6)
 # Physical slot across the bolt face, low-poly dark inset strip.
 slot=extrude('Bolt slot '+str(side),[(side*.58-.025,-.085),(side*.58+.025,-.085),(side*.58+.025,-.075),(side*.58-.025,-.075)],-.235,-.23,rubber)
 for u in slot.data.uv_layers.active.data:u.uv.x+=.5
# Fix normals; keep every face flat and no subdivision modifiers.
for o in model:
 bpy.context.view_layer.objects.active=o;o.select_set(True)
 bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.mesh.normals_make_consistent(inside=False);bpy.ops.object.mode_set(mode='OBJECT');o.select_set(False)
world=bpy.data.worlds.new('Neutral studio');scene.world=world;world.use_nodes=True;world.node_tree.nodes['Background'].inputs[0].default_value=(.23,.25,.28,1);world.node_tree.nodes['Background'].inputs[1].default_value=.7
for name,loc,power,size in [('Key',(-3,-4,5),650,4),('Fill',(3,-2,2),350,3),('Rim',(0,2,4),550,3)]:
 data=bpy.data.lights.new(name,'AREA');data.energy=power;data.shape='DISK';data.size=size;o=bpy.data.objects.new(name,data);bpy.context.collection.objects.link(o);o.location=loc;o.rotation_euler=(Vector((0,0,0))-o.location).to_track_quat('-Z','Y').to_euler()
def camera(name,loc,target,scale):
 d=bpy.data.cameras.new(name);d.type='ORTHO';d.ortho_scale=scale;o=bpy.data.objects.new(name,d);bpy.context.collection.objects.link(o);o.location=loc;o.rotation_euler=(Vector(target)-o.location).to_track_quat('-Z','Y').to_euler();return o
studio=camera('Studio front three-quarter',(2.1,-7,2.3),(0,0,-.15),5.65)
front=camera('Front symmetry check',(0,-8,.0),(0,0,0),5.4)
game=camera('Duke viewpoint - low frame',(0,-8,1.59),(0,0,1.59),6.4)
scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA';scene.render.resolution_percentage=100
scene.camera=studio;scene.render.resolution_x=1280;scene.render.resolution_y=640
bpy.ops.wm.save_as_mainfile(filepath=str(O/'biomask-lowpoly.blend'))
for cam,w,h,filename in [(studio,1280,640,'studio.png'),(front,1280,420,'front.png'),(game,1280,720,'viewpoint.png')]:
 scene.camera=cam;scene.render.resolution_x=w;scene.render.resolution_y=h;scene.render.filepath=str(O/filename);bpy.ops.render.render(write_still=True)
# Portable geometry export for review / future conversion, not a working game loader.
bpy.ops.object.select_all(action='DESELECT')
for o in model:o.select_set(True)
bpy.ops.export_scene.gltf(filepath=str(O/'biomask-lowpoly.glb'),use_selection=True,export_apply=True,export_format='GLB')
deps=bpy.context.evaluated_depsgraph_get();triangles=0
for o in model:
 m=o.evaluated_get(deps).to_mesh();m.calc_loop_triangles();triangles+=len(m.loop_triangles);o.evaluated_get(deps).to_mesh_clear()
(O/'mesh-stats.json').write_text(json.dumps({'triangles':triangles,'hose_sides':8,'ribs_per_side':16,'texture_size':[256,128],'subdivision':False,'flat_shaded':True,'runtime_integrated':False},indent=2))
scene.camera=studio;scene.render.resolution_x=1280;scene.render.resolution_y=640
bpy.ops.wm.save_as_mainfile(filepath=str(O/'biomask-lowpoly.blend'))
print('MODEL_COMPLETE',triangles)
