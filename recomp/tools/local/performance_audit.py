#!/usr/bin/env python3
"""Measure a bounded TTK performance route on the existing build.

Writes only to a NEW directory under recomp/analysis. Starts from empty private
cards, never imports player saves, and refuses to overlap another game. Uses
SDL offscreen real-GPU rendering and dummy audio. This is not a
physical-display, audible-quality or multi-hour campaign certification.
See documentation/252-d23-performance-baseline.md for budgets and limitations.
"""
import argparse,hashlib,json,os,signal,struct,subprocess,sys,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools/local'))
from debug_client import request
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--mode',choices=('modernized','vanilla'),default='modernized')
parser.add_argument('--output',type=Path,required=True,help='new directory under recomp/analysis')
parser.add_argument('--settings-file',type=Path,default=ROOT/'config/player-profiles.json',help='read-only source profile')
parser.add_argument('--port',type=int,default=9453)
parser.add_argument('--sustained-seconds',type=int,default=600)
parser.add_argument('--smoke',action='store_true',help='short harness check only, no travel/endurance route')
parser.add_argument('--smoke-seconds',type=int,default=10)
args=parser.parse_args()
if not 1024<=args.port<=65535:parser.error('port must be 1024..65535')
if args.smoke_seconds<1:parser.error('smoke-seconds must be positive')
if args.sustained_seconds<60:parser.error('sustained-seconds must be at least 60')
MODE=args.mode
PORT=args.port
OUT=args.output.resolve()
if not OUT.is_relative_to((ROOT/'analysis').resolve()):parser.error('output must be under recomp/analysis')
if OUT.exists():parser.error('output already exists; use a new run directory')
OUT.mkdir(parents=True)
STEER=OUT/'steer.input'
EXE=ROOT/'build-local/Duke_Nukem__Time_to_Kill_Recompiled'
def call(cmd,**kw):
 r=request(dict(cmd=cmd,**kw),PORT)
 if not r.get('ok'):raise RuntimeError(str(r))
 return r
def ram(a,n):return bytes.fromhex(call('read_ram',addr=hex(a),len=n)['hex'])
def state():
 h=lambda a:struct.unpack('<H',ram(a,2))[0]
 return dict(mode=h(0x800bcbb0),phase=h(0x800ce1b0),level=struct.unpack('<I',ram(0x800be570,4))[0],end=ram(0x800be55e,1)[0],pause=h(0x800be568),hp=h(0x800d7198+0x32),xyz=struct.unpack('<3i',ram(0x800d719c,12)))
def inp(s=''):STEER.write_text('play '+s if MODE=='modernized' else '')
def pids():
 out=[]
 for p in Path('/proc').iterdir():
  if p.name.isdigit():
   try:
    if Path(os.readlink(p/'exe')).name==EXE.name:out.append(int(p.name))
   except OSError:pass
 return out
def host():
 # Counters permit deltas and correlation; swap occupancy alone is not pressure.
 r={'utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'mono':time.monotonic(),'loadavg':Path('/proc/loadavg').read_text(),'pressure':{k:Path('/proc/pressure'/Path(k)).read_text() for k in ('cpu','io','memory')},'meminfo':Path('/proc/meminfo').read_text(),'vmstat':Path('/proc/vmstat').read_text(),'stat':Path('/proc/stat').read_text(),'processes':{}}
 for pid in pids():
  try:r['processes'][str(pid)]={k:Path(f'/proc/{pid}/{k}').read_text() for k in ('status','stat','io','smaps_rollup')}
  except OSError:pass
 r['top']=subprocess.run(['ps','-eo','pid,comm,pcpu,pmem','--sort=-pcpu'],capture_output=True,text=True).stdout.splitlines()[:16]
 r['background']=subprocess.run(['pgrep','-af','[t]imeshift|[r]sync|[u]nattended-upgrade|[a]pt-get|[d]pkg'],capture_output=True,text=True).stdout
 return r
def snap():
 return {'t':time.monotonic(),'frame':call('frame'),'replay':call('render_replay'),'audio':call('audio_stats'),'state':state()}
def savejson(name,data): (OUT/name).write_text(json.dumps(data,indent=2))
def travel(n):
 inp();t=time.monotonic();r=call('console',line=f'level {n}');print('TRAVEL',n,r,flush=True)
 if 'Travelling to ' not in r.get('reply',''):
  raise RuntimeError(('travel refused',r))
 left=False
 while time.monotonic()-t<60:
  s=state()
  if s['mode']!=1 or s['phase']!=2 or s['end']!=1:left=True
  if left and s['mode']==1 and s['phase']==2 and s['end']==1 and s['level']==n:
   result={'level':n,'seconds':time.monotonic()-t,'reply':r,'state':s};time.sleep(3);return result
  time.sleep(.1)
 raise TimeoutError(('travel',n,state()))
def window(name,seconds,move=True):
 print('WINDOW',name,seconds,flush=True)
 inp();time.sleep(2)
 rows=[];hosts=[];hot0=call('phase_hot',top=20);start=snap();t=time.monotonic();i=0
 if not start['audio']['out']['active']:raise RuntimeError('Audio output is inactive; cannot measure its counters')
 while time.monotonic()-t<seconds:
  if move:
   # Modest back/forth plus view sweep. Vanilla uses original pad turning.
   if MODE=='modernized':inp(('a dx=0.4' if (i//3)%2 else 'd dx=-0.4'))
   else:call('press',buttons=65535-(128 if (i//3)%2 else 32),frames=30)
  rows.append(snap())
  if i%10==0:hosts.append(host())
  if i and i%120==0:print('PROGRESS',name,round(time.monotonic()-t,1),'seconds',rows[-1]['state'],flush=True)
  i+=1;time.sleep(max(0,t+i*.5-time.monotonic()))
 inp();end=snap();phase=call('phase_profile',window=min(60,max(2,int(seconds))))
 out={'name':name,'seconds':seconds,'start':start,'end':end,'samples':rows,'hosts':hosts,'phase':phase,'hot_start':hot0,'hot_end':call('phase_hot',top=20)}
 savejson(name+'.json',out)
 dt=end['t']-start['t'];r0=start['replay'];r1=end['replay']
 summary={'name':name,'elapsed':dt,'fields_s':(end['frame']['frame']-start['frame']['frame'])/dt,'presents_s':(r1.get('presents',0)-r0.get('presents',0))/dt if r1.get('owns') else None,'flips_s':(r1.get('flips',0)-r0.get('flips',0))/dt if r1.get('owns') else None,'underruns':end['audio']['out']['underruns']-start['audio']['out']['underruns'],'state':end['state']}
 print(json.dumps(summary),flush=True);return summary
if pids():raise SystemExit('A game is already running; do not overlap measurements')
profile=json.loads(args.settings_file.read_text())
for pr in profile['profiles'].values():pr['presentation'].update(display='windowed',window_width=1280)
profile['profiles']['modernized']['controls'].update(frame_rate='120',cpu_timing='fast',cpu_overclock=150,view='first',widescreen='16:9')
profile['profiles']['modernized']['presentation'].update(internal_scale=4,renderer='opengl')
profile['profiles']['vanilla']['presentation'].update(internal_scale=1,renderer='opengl')
savejson('profiles.json',profile);STEER.write_text('')
env=dict(os.environ,SDL_VIDEODRIVER='offscreen',SDL_AUDIODRIVER='dummy',DNTTK_TEST_INPUT=str(STEER),DNTTK_TEST_UNCAPTURED='1',PSX_FORENSICS='0')
env.pop('DISPLAY',None)
cmd=[sys.executable,str(ROOT/'tools/local/run.py'),'--settings-file',str(OUT/'profiles.json'),'--memcard-dir',str(OUT/'cards'),'--debug-port',str(PORT),'--no-session-log']+(['--vanilla'] if MODE=='vanilla' else [])
meta={'mode':MODE,'cmd':cmd,'harness_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'audio_driver':'dummy','exe_sha256':hashlib.sha256(EXE.read_bytes()).hexdigest(),'host_before':host(),'windows':[],'transitions':[]}
savejson('meta.json',meta)
log=(OUT/'runtime.log').open('w');p=subprocess.Popen(cmd,cwd=ROOT,env=env,stdout=log,stderr=log,start_new_session=True)
t0=time.monotonic()
try:
 while time.monotonic()-t0<60:
  if p.poll() is not None:raise RuntimeError('launch failed')
  try:call('frame');break
  except (OSError,RuntimeError):time.sleep(.2)
 else:raise TimeoutError('startup')
 # Fresh boot, empty private cards, no player savestate or card imported.
 for i in range(100):
  s=state()
  if s['mode']==1 and s['phase']==2 and s['end']==1:break
  if MODE=='modernized':
   STEER.write_text('enter');time.sleep(.12);STEER.write_text('');time.sleep(.4)
  else:call('press',buttons=65535-(8 if i%2==0 else 16384),frames=6);time.sleep(.5)
 else:raise TimeoutError(('newgame',state()))
 time.sleep(3);meta['startup_to_gameplay_s']=time.monotonic()-t0
 meta['initial_state']=state();meta['levels']=call('console',line='levels')
 # Private-process invulnerability prevents deaths invalidating endurance.
 call('write_ram',addr='0x800c3cc6',val='1');call('write_ram',addr='0x800c3cc7',val='0')
 savejson('boot.json',snap());print('BOOT',meta['initial_state'],flush=True)
 if args.smoke:
  meta['windows'].append(window('smoke',args.smoke_seconds))
 else:
  for cycle in range(2):
   for level in (0,6,9,21):
    meta['transitions'].append(travel(level));savejson('meta.json',meta)
    meta['windows'].append(window(f'cycle{cycle}-level{level}',20));savejson('meta.json',meta)
  meta['transitions'].append(travel(0))
  meta['windows'].append(window('sustained',args.sustained_seconds));savejson('meta.json',meta)
finally:
 inp();t=time.monotonic()
 try:call('quit')
 except Exception as e:meta['quit_error']=repr(e)
 try:p.wait(15)
 except subprocess.TimeoutExpired:os.killpg(p.pid,signal.SIGTERM);p.wait(5);meta['forced_shutdown']=True
 meta['shutdown_s']=time.monotonic()-t;meta['returncode']=p.returncode;meta['host_after']=host();savejson('meta.json',meta);print('STOP',meta['shutdown_s'],p.returncode,flush=True)
