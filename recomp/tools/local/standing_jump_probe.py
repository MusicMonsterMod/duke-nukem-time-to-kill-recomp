"""Isolated real-SDL standing takeoffs; health-only fixture, no movement writes."""
import hashlib, math, os, struct
from pathlib import Path
P=0x800d7198

def exercise(call,xdo,eventually,directory,report,flush):
    report['standing_driver_sha256']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    rows=report['standing_jumps']=[]
    def frames(n):
        end=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=end,30)
    def snap(label):
        b=bytes.fromhex(call('read_ram',addr=hex(P),len=0x840)['hex'])
        h=lambda o:struct.unpack_from('<h',b,o)[0]
        w=lambda o:struct.unpack_from('<i',b,o)[0]
        r=dict(label=label,frame=call('frame')['frame'],position=[w(o) for o in (4,8,12)],velocity=[w(o) for o in (0x1f4,0x1f8,0x1fc)],anim=h(0x60),upper=h(0x74),requested=h(0x23e),state=list(b[0x22c:0x22e]),health=h(0x32),flags=w(0),input=call('ttk_input')['input'])
        rows.append(r);flush();return r
    call('turbo',enabled=0)
    cases=[('w',0),('s',0),('d',1),('a',1),('wd',0),('sa',0),('',0),('w',3)]
    if os.environ.get('DNTTK_STANDING_REVERSE'): cases=[('w',-1),('s',-3),('d',-2),('a',-3),('wd',-1),('sa',-3),('',0),('w',3)]
    if os.environ.get('DNTTK_STANDING_BASELINE'): cases=[('w',0)]
    results=report['standing_results']=[]
    for index,(keys,delay) in enumerate(cases):
        label=f'{index}-{keys or "vertical"}-{delay}'
        report.setdefault('fixture_writes',[]).append(dict(address=hex(P+0x32),value=30000,purpose='private health longevity only',case=label))
        for i,v in enumerate(struct.pack('<H',30000)):call('write_ram',addr=hex(P+0x32+i),val=hex(v))
        eventually(lambda:call('ttk_input')['input']['controls']['ready'],15)
        frames(20);before=snap(label+'-before')
        assert before['anim']==63, before
        # One xdotool process sends simultaneous requested keys. Jump uses the
        # private profile's J rebind, exercising actions rather than hardcoded Space.
        if delay<0:
            xdo('keydown','j');frames(1);xdo('keyup','j')
            frames(-delay);snap(label+'-direction-start');xdo('keydown',*keys)
        elif delay:
            xdo('keydown',*keys);frames(delay);xdo('keydown','j')
        else:xdo('keydown',*keys,'j')
        report.setdefault('events',[]).append(dict(case=label,down=keys+'j',frame=call('frame')['frame']))
        frames(1);xdo('keyup','j')
        samples=[]
        for n in range(25):
            frames(2);samples.append(snap(label+'-'+str(n)))
            if n==0 and keys: xdo('keyup',*keys) # direction fixed despite release in preparation
        frames(35)
        after=snap(label+'-after')
        flight=next((r for r in samples if r['state'][0]==9 and r['anim'] in (97,98,103,104)),None)
        assert flight, label+' no flight'
        result=dict(case=label,animation=flight['anim'],velocity=flight['velocity'],before=before['position'],after=after['position'])
        if keys:
            yaw=before['input']['controls']['yaw'];mx=('d' in keys)-('a' in keys);my=('w' in keys)-('s' in keys)
            dx=math.cos(yaw)*mx+math.sin(yaw)*my;dz=-math.sin(yaw)*mx+math.cos(yaw)*my
            vx,_,vz=flight['velocity'];length=math.hypot(vx,vz)
            result['alignment']=(vx*dx+vz*dz)/(length*math.hypot(dx,dz)) if length else 0
            result['corrected']=flight['input']['controls']['jumps']-before['input']['controls']['jumps']
            if not os.environ.get('DNTTK_STANDING_BASELINE'):
                assert flight['anim'] in (98,103,104) and result['alignment']>.98, result
                assert result['corrected']==1, result
                assert after['input']['controls']['jumps']==flight['input']['controls']['jumps'], 'repeated impulse'
        else:
            assert flight['anim']==97, result
            assert after['input']['controls']['jumps']==before['input']['controls']['jumps'], result
        assert all(r['health']>0 and not r['flags']&2 for r in samples), 'death-contaminated'
        results.append(result);flush()
        shot=call('present_shot',path=str(directory/(label+'.png')))
        eventually(lambda:call('present_shot_seq')['seq']>shot['seq'],10)
    report['standing_assertions']='baseline observation' if os.environ.get('DNTTK_STANDING_BASELINE') else 'passed'
    flush()
