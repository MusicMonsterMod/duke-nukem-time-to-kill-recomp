#!/usr/bin/env python3
"""Summarize performance_audit.py evidence without launching a game.

PSS sums the game and its replay workers without double-counting shared RAM.
Presentation intervals use deduplicated runtime timestamps bounded by runtime
snapshots; SDL and Python clock epochs must not be mixed. No trace means no
interval claim (not zero milliseconds), as in Vanilla's native present path.
"""
import argparse,json,math,re,statistics,sys
from pathlib import Path

def pct(a,p):
 return sorted(a)[min(len(a)-1,math.ceil(p*len(a))-1)] if a else None

def pss(h):
 total=0
 for proc in h['processes'].values():
  m=re.search(r'^Pss:\s+(\d+)',proc.get('smaps_rollup',''),re.M)
  if m:total+=int(m[1])/1024
 return total

def psi(h,k,kind='some'):
 line=next(x for x in h['pressure'][k].splitlines() if x.startswith(kind+' '))
 return dict((a,float(b)) for a,b in (x.split('=') for x in line.split()[1:]))

def reduce(path):
 r=json.loads(path.read_text());a=r['start'];b=r['end'];ss=r['samples'];dt=b['t']-a['t'];ar=a['replay'];br=b['replay']
 delta=lambda k:br.get(k,0)-ar.get(k,0)
 def trace_times(s):
  return [float(e.split('/')[8]) for e in (s['replay'].get('plugin') or {}).get('trace','').split() if len(e.split('/'))>8]
 # SDL performance-counter and Python monotonic clocks have different epochs.
 # Bound by the two runtime traces, never compare their absolute timestamps.
 ta=trace_times(a);tb=trace_times(b)
 lo=max(ta) if ta else 0;hi=max(tb) if tb else 0
 stamps=set()
 for s in [a]+ss+[b]:
  for entry in (s['replay'].get('plugin') or {}).get('trace','').split():
   parts=entry.split('/')
   if len(parts)>8:
    t=float(parts[8])
    if lo<=t<=hi:stamps.add(t)
 stamps=sorted(stamps);intervals=[1000*(y-x) for x,y in zip(stamps,stamps[1:])]
 hs=r['hosts'];m=[pss(h) for h in hs];steady=[(h['mono']-hs[0]['mono'],pss(h)) for h in hs if h['mono']-hs[0]['mono']>=60]
 slope=None
 if len(steady)>2:
  xm=statistics.mean(x for x,y in steady);ym=statistics.mean(y for x,y in steady)
  slope=60*sum((x-xm)*(y-ym) for x,y in steady)/sum((x-xm)**2 for x,y in steady)
 au=a['audio']['out'];bu=b['audio']['out'];p0=(ar.get('plugin') or {});p1=(br.get('plugin') or {})
 fields=(b['frame']['frame']-a['frame']['frame'])/dt
 out={'name':r['name'],'elapsed_s':round(dt,3),'field_hz':round(fields,3),'present_hz':round(delta('presents')/dt,3) if br.get('owns') else None,'game_image_hz':round(delta('flips')/dt,3) if br.get('owns') else None,'present_intervals':len(intervals),'trace_coverage_s':round(stamps[-1]-stamps[0],3) if stamps else 0,'present_p50_ms':pct(intervals,.5),'present_p95_ms':pct(intervals,.95),'present_p99_ms':pct(intervals,.99),'present_max_ms':max(intervals) if intervals else None,'worst_rolling_cadence':{k:max(s['replay'].get('cadence',{}).get(k,0) for s in ss) for k in ('p95_us','p99_us','max_us')},'output_underruns':bu['underruns']-au['underruns'],'overflow_drops':bu['overflow_drops']-au['overflow_drops'],'audio_fill_ms_min':min(s['audio']['out']['fill_ms'] for s in ss),'audio_fill_ms_max':max(s['audio']['out']['fill_ms'] for s in ss),'emu_sheds':p1.get('emu_sheds',0)-p0.get('emu_sheds',0),'pace_divisors':sorted(set((s['replay'].get('plugin') or {}).get('pace_div',1) for s in ss)),'pss_start_mib':m[0],'pss_end_mib':m[-1],'pss_peak_mib':max(m),'pss_after_60s_slope_mib_min':slope,'pressure_peak_avg10':{k:max(psi(h,k)['avg10'] for h in hs) for k in ('cpu','io','memory')},'background_activity':sorted(set(h['background'] for h in hs if h['background'])),'state_start':a['state'],'state_end':b['state'],'all_samples_active':all(s['state']['mode']==1 and s['state']['phase']==2 and s['state']['end']==1 and not s['state']['pause'] for s in ss),'phase':{k:v for k,v in r['phase'].items() if k.endswith('share')}}
 for k,v in out.items():
  if isinstance(v,float):out[k]=round(v,4)
 return out

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('runs',type=Path,nargs='+')
 parser.add_argument('--output',type=Path,required=True)
 args=parser.parse_args()
 results={}
 for d in args.runs:
  name=d.name
  if name in results:parser.error('run directory names must be unique')
  meta=json.loads((d/'meta.json').read_text())
  windows=[reduce(p) for p in sorted(d.glob('*.json')) if p.name.startswith(('cycle','sustained','smoke'))]
  results[name]={'mode':meta['mode'],'executable_sha256':meta['exe_sha256'],'startup_to_gameplay_s':meta.get('startup_to_gameplay_s'),'shutdown_s':meta.get('shutdown_s'),'returncode':meta.get('returncode'),'forced_shutdown':meta.get('forced_shutdown',False),'transitions':meta['transitions'],'windows':windows,'level_names':meta.get('levels',{}).get('reply','')}
 args.output.parent.mkdir(parents=True,exist_ok=True)
 args.output.write_text(json.dumps(results,indent=2)+'\n')
 print(json.dumps(results,indent=2))
