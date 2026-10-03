"""D07A real-input replay; read-only guest observation, isolated parent driver."""
import math
import time
from ttk_state_probe import sample


def exercise(call, xdo, eventually, directory, report, flush):
    rows=report['facing_samples']=[]
    def frames(n):
        target=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=target,30)
    def capture(name, aligned=True):
        s=sample(call)
        row={'name':name,'state':{k:v for k,v in s.items() if k!='raw_hex'},'input':call('ttk_input')['input']}
        rows.append(row);flush()
        assert not s['player_flags']&2 and s['player_animation']!=218, name+' death'
        m=s['camera_matrix_q12'];angle=math.atan2(m[6],m[8])*4096/(2*math.pi)
        error=(s['player_rotation_candidate'][0]-angle+2048)%4096-2048
        row['heading_error_units']=error
        if aligned:
            assert row['input']['controls']['ready'], name+' unsupported state'
            assert abs(error)<40, name+' heading not view aligned'
        call('screenshot_file',path=str(directory/(name+'.png')));flush()
        return row
    def drawn():return int(call('read_ram',addr='0x800d7550',len=1)['hex'],16)==2
    def toggle(wanted):
        xdo('keydown','--delay',0,'h');time.sleep(.045);xdo('keyup','--delay',0,'h')
        eventually(lambda:drawn()==wanted,10);frames(24)
        eventually(lambda:call('ttk_input')['input']['controls']['ready'],10)
    call('turbo',enabled=0);xdo('key','F10')
    try:
        eventually(lambda:call('ttk_input')['input']['controls']['ready'])
    except Exception:
        report['readiness_failure']={'input':call('ttk_input'),'state':sample(call)}
        import hashlib,re
        report['guard_digests']=[{'address':a,'expected':h,'actual':hashlib.sha256(bytes.fromhex(call('read_ram',addr=a,len=int(n))['hex'])).hexdigest()} for a,n,h in re.findall(r'\{(0x[0-9a-f]+), (\d+), "([0-9a-f]+)"', (directory.parents[2]/'src/ttk/control_guards.inc').read_text())]
        flush();raise
    (directory/'level00-guard-fixture.bin').write_bytes(bytes.fromhex(call('read_ram',addr='0x800ca968',len=9668)['hex']))
    capture('initial-drawn')
    if drawn():toggle(False)
    capture('holstered-start')
    xdo('mousemove_relative','--',180,-30);frames(24);capture('holstered-look')
    toggle(True);capture('drawn')
    xdo('mousemove_relative','--',-110,50);frames(24);capture('armed-look')
    for key in ('a','d','s','w'):
        before=capture(key+'-before');xdo('keydown',key);frames(10);xdo('keyup',key)
        after=capture(key+'-after');frames(12)
        a=before['state']['player_position_candidate'];b=after['state']['player_position_candidate']
        m=before['state']['camera_matrix_q12'];fx,fz=m[6],m[8]
        dx,dz={'w':(fx,fz),'s':(-fx,-fz),'a':(-fz,fx),'d':(fz,-fx)}[key]
        assert (b[0]-a[0])*dx+(b[2]-a[2])*dz>0,key+' wrong movement direction'
    xdo('mousedown',1);frames(18);xdo('mouseup',1);capture('armed-fire')
    xdo('mousedown',3);frames(8);before=capture('precision-before',False)
    xdo('keydown','Right');xdo('mousemove_relative','--',80,-20);frames(6);xdo('keyup','Right')
    xdo('mousedown',1);frames(12);xdo('mouseup',1);after=capture('precision-held',False)
    assert after['input']['controls']['facings']==before['input']['controls']['facings']
    assert after['input']['controls']['arms']==before['input']['controls']['arms']
    assert after['input']['controls']['aim']['shots']==before['input']['controls']['aim']['shots']
    assert not after['input']['controls']['aim']['reticle']
    xdo('mouseup',3);frames(30);capture('precision-released')
    toggle(False);capture('holstered-again')
    xdo('key','Escape');frames(4);before=capture('released',False)
    xdo('mousemove_relative','--',300,100);frames(4);after=capture('released-motion',False)
    assert after['input']['controls']['facings']==before['input']['controls']['facings']
    report['facing_assertions']='passed; images and gameplay acceptance require review';flush()
