"""Run with blender -b --python to export the reviewed model for the title renderer."""
import bpy, math, argparse, sys
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser()
parser.add_argument('--asset-dir',type=Path,default=ROOT/'assets/models/biomask')
parser.add_argument('--output',type=Path,default=ROOT/'src/ttk/bio_mask_mesh.h')
args=parser.parse_args(sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else [])
ASSET=args.asset_dir
bpy.ops.wm.open_mainfile(filepath=str(ASSET/'biomask-lowpoly.blend'))
deps=bpy.context.evaluated_depsgraph_get();out=[]
light=Vector((-.25,-.8,.6)).normalized()
for obj in bpy.context.scene.objects:
 if obj.type!='MESH':continue
 eo=obj.evaluated_get(deps);m=eo.to_mesh();m.calc_loop_triangles();uv=m.uv_layers.active
 for tri in m.loop_triangles:
  norm=(obj.matrix_world.to_3x3()@tri.normal).normalized()
  # Two broad light sources, baked per flat face; no smooth normals or gloss.
  shade=.55+.45*max(0,norm.dot(light))
  mat=m.materials[tri.material_index]
  mix=next(n for n in mat.node_tree.nodes if n.type=='MIX_RGB')
  gain=mix.inputs[2].default_value[:3]
  rgb=[min(255,round(128*shade*g**.45)) for g in gain]
  row=[]
  for vi,li in zip(tri.vertices,tri.loops):
   p=obj.matrix_world@m.vertices[vi].co;u,v=uv.data[li].uv
   row.append('{'+','.join([*(f'{x:.7f}f' for x in p),f'{max(0,min(255,u*256)):.5f}f',f'{max(0,min(127,(1-v)*128)):.5f}f',*(str(c) for c in rgb)])+'}')
  out.append('{'+','.join(row)+'}')
 eo.to_mesh_clear()
header='// Exported by tools/local/export_biomask_mesh.py from the original Blender model.\n#pragma once\n#include <cstdint>\nnamespace ttk {\nstruct BioMaskVertex {float x,y,z,u,v;uint8_t r,g,b;};\ninline constexpr BioMaskVertex bio_mask_mesh[][3]={\n'+',\n'.join(out)+'\n};\n}\n'
args.output.write_text(header)
# Blender image pixels are linear floats in bottom-up rows. Use the packed sRGB image
# via its byte conversion only for RGB export; the following standalone PNG read
# step in the build workflow produces exact 1555 atlas texels.
print('EXPORTED',len(out),'triangles')
