#!/usr/bin/env python3
"""Read-only owned DB00 crouch asset audit; writes hashes/derived data locally.

Run from workspace root. This is a frame/root-command inspector, not a validated
skeletal animation importer. See documentation/43-crouch-walk-audit.md.
"""
import sys,struct,json,hashlib
from pathlib import Path
sys.path.insert(0,'recomp/tools/local');from disc_lab import Disc,EXE_SHA256
exe=Path('recomp/disc/SLUS_005.83').read_bytes();assert hashlib.sha256(exe).hexdigest()==EXE_SHA256
d=Disc('recomp/disc/time-to-kill.bin')
files={f['path'].split('/')[-1]:d.read(f['lba'],f['size']) for f in d.files() if '/DB00/' in f['path'] and any(x in f['path'] for x in ['ANM','FRM','ROT','HIE','COL'])};d.close()
a=files['DB00ANM.BIN;1'];f=files['DB00FRM.BIN;1'];rot=files['DB00ROT.BIN;1']
def signed14(x):return (x&16383)-((x&8192)<<1)
rows=[]
for anim in [63,72,74,76,78,176,177,178,179,180,181,182,183,184]:
 tracks=struct.unpack_from('<4h',exe,0x800c2d64+anim*8-0x80010000+2048)
 clip=tracks[0];start,n=struct.unpack_from('<HH',a,4*clip);frames=[]
 for i in range(start,start+n):
  flags,ticks,begin,count=struct.unpack_from('<4H',f,8*i);assert (begin+count)*8<=len(rot)
  commands=[]
  for j in range(begin,begin+count):
   w,z=struct.unpack_from('<II',rot,j*8)
   commands.append(dict(joint=z>>27,flags=w>>28,values=[signed14(w),signed14(w>>14),signed14(z),((z>>14)&8191)-((z>>26&1)<<13)]))
  frames.append(dict(events=flags,ticks=ticks,root=[c['values'][:3] for c in commands if c['flags']&2],joint0=[c for c in commands if c['joint']==0 and not c['flags']&2]))
 rows.append(dict(animation=anim,tracks=tracks,frames=n,total_ticks=sum(x['ticks'] for x in frames),data=frames))
out=dict(executable_sha256=EXE_SHA256,assets={k:dict(bytes=len(v),sha256=hashlib.sha256(v).hexdigest()) for k,v in files.items()},animations=rows,evidence='593b4 ANM start/count; 58c70 and 597e4 FRM/ROT reader; root translation flag bit29, joint word1 top5; signed 14-bit components; quaternion/rotation interpretation still requires model validation')
destination=Path('recomp/analysis/iteration42/asset-audit.json')
destination.parent.mkdir(parents=True,exist_ok=True)
destination.write_text(json.dumps(out,indent=2))
for r in rows:print(r['animation'],r['tracks'],r['frames'],r['total_ticks'])
print(destination)
