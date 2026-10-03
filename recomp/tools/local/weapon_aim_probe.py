"""D07 original weapon research on an isolated SDL run; no RAM mutation."""
import struct
from ttk_state_probe import sample

TARGETS=(0x8003ca54,0x8003c500,0x80071b88,0x800718ac,0x8006ddb8,0x8006dd2c,0x8006d980,0x8003c690,0x8006edd0,0x80073220)

def exercise(call,xdo,eventually,directory,report,flush,fixtures=False,near_cover=False,slots=range(5,15)):
    rows=report['weapon_research']=[]
    def frames(n):
        end=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=end,30)
    def capture(name):
        s=sample(call)
        assert not s['player_flags']&2, 'dead player'
        ram=bytes.fromhex(call('read_ram',addr='0x800d7198',len=0x8a4)['hex'])
        pool=bytes.fromhex(call('read_ram',addr='0x800da978',len=0x2400)['hex'])
        (directory/(name+'-player.bin')).write_bytes(ram)
        (directory/(name+'-projectiles.bin')).write_bytes(pool)
        rows.append(dict(name=name,input=call('ttk_input')['input'],state={k:v for k,v in s.items() if k!='raw_hex'},weapon_slot=ram[0x3b9],ammo=[struct.unpack_from('<h',ram,0x2c6+4*i)[0] for i in range(32)],trace=call('fntrace_dump',count=2048)))
        call('screenshot_file',path=str(directory/(name+'.png')))
        shot=call('present_shot',path=str(directory/(name+'-present.png')))
        eventually(lambda:call('present_shot_seq')['seq']>shot['seq'],10)
        flush()
    call('turbo',enabled=0)
    sample(call)
    for target in TARGETS:call('fntrace_arm',target=hex(target))
    call('fntrace_clear')
    xdo('key','F10');frames(3);capture('weapon-before')
    xdo('mousemove_relative','--',120,-20);frames(4)
    xdo('mousedown',1);frames(24);xdo('mouseup',1);capture('weapon-fire')
    frames(12);capture('weapon-after')
    if near_cover:
        xdo('keydown','w');frames(150);xdo('keyup','w');frames(12);capture('cover-position')
        xdo('mousedown',1);frames(24);xdo('mouseup',1);capture('cover-fire')
    if fixtures:
        from pathlib import Path
        exe=(Path(__file__).resolve().parents[2]/'disc/SLUS_005.83').read_bytes()
        report['fixture_writes']=[]
        for weapon in slots:
            sample(call)
            addr=0x800d7198+0x2c4+weapon*4
            data=struct.pack('<HH',1,200)
            report['fixture_writes'].append(dict(address=hex(addr),hex=data.hex(),purpose='isolated inventory grant; original keys perform selection; never save'))
            for i,b in enumerate(data):call('write_ram',addr=hex(addr+i),val=hex(b))
            if fixtures=='synthetic':
                addr=0x800d7198+0x3b9
                report['fixture_writes'].append(dict(address=hex(addr),hex=bytes([weapon]).hex(),purpose='synthetic slot selection; no animation/acquisition claim'))
                call('write_ram',addr=hex(addr),val=hex(weapon));frames(3)
            else:
                xdo('keydown','h');frames(6);xdo('keydown','Right');frames(12);xdo('keyup','Right');frames(6);xdo('keyup','h');frames(60)
            actual=int(call('read_ram',addr=hex(0x800d7198+0x3b9),len=1)['hex'],16)
            report.setdefault('weapon_selection',[]).append(dict(expected=weapon,actual=actual))
            capture('slot-%d-selected'%weapon)
            assert actual==weapon, 'original weapon cycle did not select fixture'
            call('fntrace_clear');capture('slot-%d-before'%weapon)
            xdo('mousedown',1);frames(24);xdo('mouseup',1);capture('slot-%d-fire'%weapon)
            frames(6)
    xdo('key','Escape');frames(3);capture('aim-released')
    assert not call('ttk_input')['input']['controls']['aim'].get('reticle',False)
    call('fntrace_arm_clear');sample(call)
