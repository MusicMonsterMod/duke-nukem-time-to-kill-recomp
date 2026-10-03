"""Private D08 movement observations; original SDL movement, read-only capture, logged health fixture."""
import struct,time,os,json,math,hashlib
from pathlib import Path
P=0x800d7198

def exercise(call,xdo,eventually,directory,report,flush):
    report['traversal_driver_sha256']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    rows=report['traversal']=[]
    def frames(n):
        end=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=end,30)
    def pulse(key):
        xdo('keydown',key);frames(8);xdo('keyup',key);frames(3)
    def capture(name):
        b=bytes.fromhex(call('read_ram',addr=hex(P),len=0x840)['hex'])
        def h(o):return struct.unpack_from('<h',b,o)[0]
        def w(o):return struct.unpack_from('<i',b,o)[0]
        r={'name':name,'frame':call('frame')['frame'],'position':[w(o) for o in (4,8,12)],'anim':h(0x60),'upper':h(0x74),'ammo':h(0x2d6),'state':list(b[0x22c:0x22e]),'equipment':b[0x3b8],'heading':h(0x1c),'velocity':[w(o) for o in (0x1f4,0x1f8,0x1fc)],'delta':[h(o) for o in (0xfc,0xfe,0x100)],'dt':h(0xba),'floor':w(0x1c4),'raw32':h(0x32),'flags':w(0),'flags224':w(0x224),'requested':h(0x23e),'input':call('ttk_input')['input']}
        target=struct.unpack_from('<I',b,0x288)[0]
        if 0x800d0000<=target<0x801ff000 and target%4==0:
            actor=bytes.fromhex(call('read_ram',addr=hex(target),len=0x100)['hex'])
            r['locked_actor']={'address':hex(target),'health':struct.unpack_from('<h',actor,0x32)[0],'flags':struct.unpack_from('<I',actor)[0],'position':struct.unpack_from('<iii',actor,4),'target_point':struct.unpack_from('<iii',actor,0xec)}
        rows.append(r);flush()
        if os.environ.get('DNTTK_PROBE_EXPLORE'):
            assert r['raw32']>0 and not r['flags']&2, 'death-contaminated exploration: '+name
        return r
    call('turbo',enabled=0)
    # Original player health is hundredths (spawn 10000 = HUD 100).
    # Explicit research-only longevity fixture, never saved; movement untouched.
    report['fixture_writes']=[{'address':hex(P+0x32),'value':30000,'purpose':'private player-health longevity fixture; spawn reads 10000 for HUD 100; no movement/state writes'}]
    for i,v in enumerate(struct.pack('<H',30000)):call('write_ram',addr=hex(P+0x32+i),val=hex(v))
    if not call('ttk_input')['input']['captured']:xdo('key','F10')
    frames(8);capture('start')
    # Escape opens the original pause menu and returns without F10 repair.
    pulse('Escape');frames(20)
    assert not capture('escape-paused')['input']['captured']
    pulse('Escape')
    eventually(lambda:call('ttk_input')['input']['captured'],10)
    capture('escape-resumed')
    report['escape_assertions']='Original pause releases mouse; Escape resumes automatic Modernized capture'
    flush()
    if os.environ.get('DNTTK_PROBE_EXPLORE'):
        def suspended():
            return any(int.from_bytes(bytes.fromhex(call('read_ram',addr=hex(a),len=2)['hex']),'little')
                       for a in (0x800be568,0x800d2540))
        def pause_game():
            if not suspended():pulse('Return')
            eventually(suspended,10)
            frames(4)
            assert suspended(), 'pause did not remain active'
        def resume_game():
            assert suspended(), 'exploration lost original pause between operations'
            pulse('Return')
            eventually(lambda:not suspended(),10)
            frames(12)
        pause_game()
        (directory/'ready').touch()
        deadline=time.monotonic()+1800
        while time.monotonic()<deadline:
            command=directory/'request.json'
            if not command.exists():time.sleep(.05);continue
            op=json.loads(command.read_text());command.unlink()
            resume_game()
            if os.environ.get('DNTTK_REQUIRE_AUTO_CAPTURE'):
                eventually(lambda:call('ttk_input')['input']['captured'],10)
            elif not call('ttk_input')['input']['captured']:
                report.setdefault('capture_repairs',[]).append({'operation':op.get('name'),'frame':call('frame')['frame']})
                xdo('key','F10');frames(8)
            if op.get('stop'):break
            if op.get('health_fixture'):
                report['fixture_writes'].append({'address':hex(P+0x32),'value':30000,'at':op['name'],'purpose':'renew private player longevity fixture; enemy health and movement untouched'})
                for i,v in enumerate(struct.pack('<H',30000)):call('write_ram',addr=hex(P+0x32+i),val=hex(v))
            if 'look' in op:xdo('mousemove_relative','--',*op['look']);frames(8)
            if op.get('tap'):
                for key in op['tap']:pulse(key)
            op['start_frame']=call('frame')['frame']
            if op.get('mouse_down'):xdo('mousedown',op['mouse_down'])
            if op.get('down'):xdo('keydown',*op['down'])
            if op.get('runup'):
                frames(op['runup']);pulse('j')
            count=op.get('frames',1)
            step=op.get('sample_step',count)
            events=sorted(op.get('events',[]),key=lambda e:e['at'])
            elapsed=0
            while elapsed<count:
                while events and events[0]['at']<=elapsed:
                    event=events.pop(0)
                    if event.get('down'):xdo('keydown',*event['down'])
                    if event.get('up'):xdo('keyup',*event['up'])
                    if event.get('mouse_up'):xdo('mouseup',event['mouse_up'])
                    report.setdefault('input_events',[]).append({'operation':op['name'],'event':event,'frame':call('frame')['frame']})
                end=min(count,elapsed+step,events[0]['at'] if events else count)
                frames(end-elapsed);elapsed=end
                if 'sample_step' in op:capture(op['name']+'-'+str(elapsed))
            if op.get('up'):xdo('keyup',*op['up'])
            if op.get('mouse_up'):xdo('mouseup',op['mouse_up'])
            row=capture(op['name'])
            shot=call('present_shot',path=str(directory/(op['name']+'.png')))
            eventually(lambda:call('present_shot_seq')['seq']>shot['seq'],10)
            pause_game()
            report.setdefault('exploration_operations',[]).append(op)
            flush()
            (directory/'response.json').write_text(json.dumps(row))
        xdo('key','Escape');return
    sequence=[('s',15)] if os.environ.get('DNTTK_PROBE_BACK_JUMP') else [('d',18),('a',27),('d',27),('a',18),('w',18),('s',27)]
    jump_hold=1  # Brief real SDL tap; production buffer bridges guest update polling.
    report['jump_sequence']={'directions':sequence,'jump_hold_frames':jump_hold}
    for key,delay in sequence:
        label=f'{key}-{delay}'
        eventually(lambda:call('ttk_input')['input']['controls']['ready'],10)
        xdo('keydown',key,'Shift_L');frames(delay);capture(label+'-before')
        xdo('keydown','j');frames(jump_hold);xdo('keyup','j')
        for i in range(24):frames(3);capture(label+'-'+str(i))
        xdo('keyup',key,'Shift_L');frames(20)
    verify(report);flush()
    shot=call('present_shot',path=str(directory/'final.png'))
    eventually(lambda:call('present_shot_seq')['seq']>shot['seq'],10)
    xdo('key','Escape')


def verify(report):
    """Separate clear running landings from the original obstacle-impact path."""
    rows=report['traversal']
    assert all(r['raw32']>0 and not r['flags']&2 for r in rows), 'death-contaminated route'
    results=[]
    for before in (r for r in rows if r['name'].endswith('-before')):
        label=before['name'][:-7]
        samples=[r for r in rows if r['name'].startswith(label+'-') and not r['name'].endswith('-before')]
        airborne=next(r for r in samples if r['anim'] in (103,104) and r['state'][0]==9)
        yaw=before['input']['controls']['yaw']
        forward=(math.sin(yaw),math.cos(yaw));right=(math.cos(yaw),-math.sin(yaw))
        direction={'w':forward,'s':tuple(-x for x in forward),'d':right,'a':tuple(-x for x in right)}[label[0]]
        vx,_,vz=airborne['velocity']
        alignment=(vx*direction[0]+vz*direction[1])/math.hypot(vx,vz)
        assert alignment>.98, (label,'jump direction',alignment)
        assert airborne['input']['controls']['jumps']==before['input']['controls']['jumps']+1, label+' launch correction'
        assert samples[-1]['input']['controls']['jumps']==airborne['input']['controls']['jumps'], label+' repeated correction'
        # The original selector briefly changes state before replacing the flight animation.
        landed=next(i for i,r in enumerate(samples) if r['state'][0]==0 and r['anim'] not in (103,104))
        impact=any(r['anim'] in (107,108) for r in samples[:landed])
        if not impact:
            assert samples[landed]['anim'] in (76,78), label+' clear landing failed to continue running'
            assert not any(r['anim'] in (63,105) for r in samples[:landed+1]), label+' stationary recovery before clear landing'
        results.append({'sequence':label,'jump_animation':airborne['anim'],'direction_alignment':alignment,'landing_animation':samples[landed]['anim'],'original_impact':impact,'clear_running_continuation':not impact})
    assert results
    report['jump_assertions']={'passed':True,'results':results}
