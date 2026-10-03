"""Private D08 polish route; real inputs, health-only death fixture explicitly logged."""
import struct,math,time,os
P=0x800d7198;C=0x800d6eb0

def exercise(call,xdo,eventually,directory,report,flush):
    rows=report['polish']=[]
    def frames(n):
        end=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=end,30)
    def sample(name):
        b=bytes.fromhex(call('read_ram',addr=hex(C),len=P+0x840-C)['hex']);o=P-C
        word=lambda off:struct.unpack_from('<i',b,off)[0]
        pos=[word(o+x) for x in (4,8,12)];anchor=[word(o+0x7bc+x) for x in (0,4,8)];anchor[1]+=word(0x88)
        cam=[word(x) for x in (0x64,0x68,0x6c)]
        r={'name':name,'frame':call('frame')['frame'],'position':pos,'delta':struct.unpack_from('<hhh',b,o+0xfc),'anim':struct.unpack_from('<h',b,o+0x60)[0],'state':list(b[o+0x22c:o+0x22e]),'flags':word(o),'health':struct.unpack_from('<h',b,o+0x32)[0],'equipment':b[o+0x3b8],'ammo':struct.unpack_from('<h',b,o+0x2c6+4*4)[0],'camera':cam,'anchor':anchor,'distance':math.dist(anchor,cam),'input':call('ttk_input')['input']}
        rows.append(r);flush();return r
    def shot(name):
        x=call('present_shot',path=str(directory/(name+'.png')));eventually(lambda:call('present_shot_seq')['seq']>x['seq'],10)
    call('turbo',enabled=0)
    r=sample('entry')
    if not r['input']['captured']:xdo('key','F10');frames(8)
    if os.environ.get('DNTTK_POLISH_FINAL'):
        before=sample('interact-before');assert before['equipment']==2
        xdo('keydown','e');frames(3);xdo('keyup','e')
        for i in range(24):frames(3);sample('interact-'+str(i))
        after=rows[-1]
        if after['equipment']!=0:
            shot('e-failure');xdo('keydown','c');frames(6);xdo('keyup','c');frames(60);sample('manual-c-comparison')
        assert after['equipment']==0,'E did not finish original holster'
        assert after['ammo']==before['ammo'],'E consumed ammunition'
        assert after['input']['controls']['aim']['shots']==before['input']['controls']['aim']['shots'],'E fired a shot'
        assert any(x['input']['pad']&16384==0 and x['equipment']==0 for x in rows if x['name'].startswith('interact-')),'queued E action missing'
        report['interaction_assertions']='Tap E holsters and emits only unarmed action; ammunition and shot count unchanged'
        xdo('keydown','c');frames(8);xdo('keyup','c');frames(40)
        assert sample('c-redraw')['equipment']==2,'C did not redraw'
        xdo('keydown','Return');frames(8);xdo('keyup','Return');frames(20)
        assert not sample('paused')['input']['captured'],'automatic capture stole pause menu'
        xdo('keydown','Return');frames(8);xdo('keyup','Return')
        eventually(lambda:call('ttk_input')['input']['captured'],10);sample('resumed')
        report['automatic_capture_assertions']='Initial gameplay and pause resume capture automatically; pause menu releases capture'

    # Open street to camera-back from first spawn. Paired start/stop measurements.
    for label,keys in [('walk',['s']),('run',['s','Shift_L'])]:
        xdo('keydown',*keys)
        for i in range(18):frames(2);sample(label+'-'+str(i))
        xdo('keyup',*keys)
        for i in range(10):frames(2);sample(label+'-stop-'+str(i))
    shot('movement')
    # Keep original enemies/simulation responsible for death. This only shortens waiting.
    report['fixture_writes']=[{'address':hex(P+0x32),'value':100,'purpose':'private HUD 1 health fixture to reproduce enemy death/restart; no camera/position/state write'}]
    call('write_ram',addr=hex(P+0x32),val='0x64');call('write_ram',addr=hex(P+0x33),val='0')
    # Return toward the open street and explicitly fire to engage original enemies.
    # This is a separate fire input AFTER the E no-fire assertions.
    xdo('keydown','w','Shift_L');frames(25);xdo('keyup','w','Shift_L')
    xdo('mousedown',1);frames(8);xdo('mouseup',1);sample('deliberate-enemy-engagement')

    for i in range(60):
        frames(30);r=sample('death-wait-'+str(i))
        if r['flags']&2:break
    shot('death-wait-end')
    assert r['flags']&2,'enemy death not reached'
    frames(160);sample('dead');shot('dead')
    # The original death menu uses Cross to restart.
    xdo('keydown','x');frames(8);xdo('keyup','x');frames(30)
    xdo('keydown','x');frames(8);xdo('keyup','x')
    for i in range(80):
        frames(15);r=sample('restart-'+str(i))
        if not r['flags']&2 and r['health']>0 and r['anim']==63:break
    shot('restart')
    if r['flags']&2:
        report['restart_unresolved']=True;flush();return
    for i in range(12):frames(10);sample('respawn-'+str(i))
    shot('respawn')
    xdo('key','F10');frames(30);sample('released');xdo('key','F10');frames(30);sample('recaptured');shot('recaptured')
    if os.environ.get('DNTTK_POLISH_FINAL'):
        stable=[x for x in rows if x['name'].startswith('respawn-')]
        assert stable and stable[-1]['input']['captured'],'respawn lost Modernized capture'
        assert stable[-1]['distance']>2400,'respawn camera remained too close in open spawn'
        after=rows[-1];assert abs(stable[-1]['distance']-after['distance'])<350,'F10 still needed to repair distance'
        report['respawn_assertions']='Original enemy death/restart returns captured normal camera; no F10 repair needed'
    xdo('key','Escape')
