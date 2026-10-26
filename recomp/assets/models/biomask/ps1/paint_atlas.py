"""Original PS1-scale painted metal and grip art, authored in Blender.
Flamethrower-style edge/recess contrast and Buffalo-style painted grip bands.
No retail pixels or photographic textures. Geometry carries no tube corrugation.
"""
import bpy
from pathlib import Path
O=Path(__file__).resolve().parent
im=bpy.data.images.new('PS1 mask - painted metal and grip',width=256,height=128,alpha=False)
# Housing front: 32x16; secondary steel: 32x16; tube strip: 32x16.
# Stepped painted steel reflections, not a runtime gloss/specular material.
steel=(142,151,156);light=(244,244,230);mid=(190,199,201)
dark=(28,35,40);recess=(58,68,76);black=(12,18,23)
# Buffalo barrel reference: one cool narrow reflection rolling into deep steel,
# rather than multiple bright flat bands. Original painted values, no retail art.
metal_ramp=[(18,24,32),(26,34,43),(40,51,63),(62,77,92),
            (96,114,132),(144,163,181),(188,202,214),(121,143,162),
            (73,94,113),(43,59,75),(29,41,55),(68,87,106),
            (145,165,183),(204,216,226),(110,133,154),(32,45,60)]
def housing(u,v):
 c=metal_ramp[v]
 # Broad stepped reflection across each cheek, with an oily shaded seam.
 if u in (0,31):c=dark
 if u in (1,30):c=metal_ramp[min(15,v+1)] if v>=3 else dark
 if u in (2,29) and 6<=v<=13:c=(94,110,122)
 # Raised bright lip around a dark inset, like the weapon's painted metalwork.
 if 5<=u<=26 and 6<=v<=12:
  c=recess
  if v==12:c=(145,165,183)
  elif v==11:c=(68,87,106)
  elif v==6:c=black
  elif u in (5,26):c=dark
  elif 8<=u<=23 and v in (8,10):c=(18,25,33)
  elif 8<=u<=23 and v in (7,9):
   c=[(42,56,72),(74,94,114),(123,145,164),(185,202,215),
      (154,175,193),(104,127,148),(65,84,103),(34,47,62)][(u-8)//2]
 # Chunky painted recessed fasteners with a lit rim and dark slot.
 for x in (3,28):
  if abs(u-x)<=1 and 3<=v<=5:
   c=light if v==5 else dark if v==3 else (139,155,166)
   if u==x and v==4:c=black
 # Sparse edge nicks and short scuffs; deliberately placed, never random noise.
 if (u,v) in {(7,14),(8,14),(9,14),(22,13),(23,13),(2,8),(29,7),(4,10),(27,9)}:c=(170,190,206)
 if (u,v) in {(7,13),(8,13),(22,12),(2,7),(29,6),(4,9),(27,8)}:c=(70,82,91)
 if (u,v) in {(11,14),(12,14),(20,14),(21,14)}:c=(87,109,130)
 return c
def secondary(u,v):
 # Collars: a narrow white steel edge, broad reflection and black oily groove.
 row=[dark,black,light,(205,214,214),(158,173,183),(99,117,133),
      (54,70,83),(32,43,53),(41,53,63),(89,109,126),
      (158,175,187),(222,228,221),light,(143,160,171),dark,black]
 c=row[v]
 if u%8==0 and 4<=v<=10:c=black
 if (u,v) in {(5,11),(6,11),(19,3),(20,3)}:c=light
 if (u,v) in {(5,10),(19,4)}:c=dark
 return c
def grip(u,v):
 # Match the pump's measured near-black neutral palette (8/16/24/33).
 # One narrow highlight per groove, no silver bands or geometric ribs.
 value=(8,16,33,24,24,16)[u%6]
 # Small painted variations along the grip; retain broad flat pentagon faces.
 if v in (0,15) and value>16:value=24
 if (u%6==2 and v in (3,9,14)) or (u%6==4 and v in (5,11)):value=24 if value==33 else 16
 if u in (0,31):value=16
 # Six small worn flecks in the existing dark palette; leave the grip intact.
 if (u,v) in {(4,4),(9,7),(15,11),(19,3),(25,9),(28,13)}:value=33
 return (value,value,value)
pixels=[]
for y in range(128):
 for x in range(256):
  if x<128:rgb=housing(x//4,y//4) if y<64 else secondary(x//4,(y-64)//4)
  else:rgb=grip((x-128)//4,y//8)
  pixels.extend([c/255 for c in rgb]+[1])
im.pixels.foreach_set(pixels);im.filepath_raw=str(O/'biomask-atlas.png');im.file_format='PNG';im.save();bpy.data.images.remove(im)
