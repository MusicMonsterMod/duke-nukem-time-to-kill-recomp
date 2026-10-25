import json,subprocess,os,shutil
from pathlib import Path
P=os.environ.get('PHOTON_BIN') or shutil.which('photon') or str(Path.home()/'.photon/bin/photon');O=Path(__file__).resolve().parent

def call(a):
 r=json.loads(subprocess.check_output([P]+a));assert r['ok'],r;return r
for name in ['rusty_metal_02','rubber_tiles']:
 r=call(['session','open','--params',json.dumps({'path':str(O/'sources'/f'{name}.jpg'),'mode':'headless'})])['result'];s=r['sessionId'];d=r['documentId'];rev=call(['inspect','--session',s,'--document',d])['revision']
 def edit(op,params):
  global rev
  r=call(['execute',op,'--session',s,'--document',d,'--revision',str(rev),'--params',json.dumps(params)]);rev=r.get('revision',rev);return r
 if name=='rubber_tiles':edit('document.operation',{'operation':{'kind':'crop','spec':{'rect':{'x':35,'y':40,'width':180,'height':180},'deleteCroppedPixels':True}}})
 else:edit('document.adjustment',{'kind':'desaturate','layerIds':[x['id'] for x in call(['inspect','--session',s,'--document',d])['result']['layers']]})
 edit('document.operation',{'operation':{'kind':'imageSize','spec':{'width':128,'height':128,'resolution':72,'resample':True,'filter':'box'}}})
 edit('document.save',{'path':str(O/f'{name}-128.png'),'asCopy':True})
 edit('document.save',{'path':str(O/f'{name}-edit.photon'),'asCopy':True})
r=call(['session','create','--params',json.dumps({'width':256,'height':128,'background':'transparent','mode':'headless','title':'Bio Mask CC0 texture atlas'})])['result'];s=r['sessionId'];d=r['documentId'];rev=call(['inspect','--session',s,'--document',d])['revision']
for x,n in [(0,'rusty_metal_02'),(128,'rubber_tiles')]:edit('image.place',{'path':str(O/f'{n}-128.png'),'name':n+' CC0 Poly Haven','rect':{'x':x,'y':0,'width':128,'height':128}})
edit('document.save',{'path':str(O/'biomask-atlas.png'),'asCopy':True})
edit('document.save',{'path':str(O/'biomask-atlas.photon'),'asCopy':True})
print('Photon texture atlas complete')
