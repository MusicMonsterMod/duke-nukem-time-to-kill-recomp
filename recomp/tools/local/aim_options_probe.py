"""D07B isolated real-input marker/lock observations and selection fixture."""
import json
import struct
from ttk_state_probe import sample


def exercise(call,xdo,eventually,directory,report,flush):
    rows=report['aim_options']=[]
    targets=set()
    call('fntrace_arm',target='0x8006f444')
    call('fntrace_clear')
    def frames(n):
        end=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=end,30)
    def capture(name):
        s=sample(call)
        assert not s['player_flags']&2,'player died'
        ram=bytes.fromhex(call('read_ram',addr='0x800d7198',len=0x8a4)['hex'])
        actor=struct.unpack_from('<I',ram,0x288)[0]
        if 0x800d0000<=actor<=0x801ff000 and not actor&3: targets.add(actor)
        target_states={}
        for actor in targets:
            data=bytes.fromhex(call('read_ram',addr=hex(actor),len=0x208)['hex'])
            target_states[hex(actor)]={'flags':struct.unpack_from('<I',data,0)[0],'raw_word_204':struct.unpack_from('<i',data,0x204)[0]}
        rows.append({'actor_contact_trace':call('fntrace_dump',count=256), 'target_states':target_states,'name':name,'state':{k:v for k,v in s.items() if k!='raw_hex'},
                     'input':call('ttk_input')['input'],
                     'target_fields':[hex(struct.unpack_from('<I',ram,x)[0]) for x in [0x280,0x284,0x288,0x28c,0x2b0]],
                     'ammo':[struct.unpack_from('<h',ram,0x2c6+4*i)[0] for i in range(15)]})
        shot=call('present_shot',path=str(directory/(name+'.png')))
        eventually(lambda:call('present_shot_seq')['seq']>shot['seq'],10);flush()
    call('turbo',enabled=0);xdo('key','F10');frames(6);capture('start')
    # Natural aim scan and pistol shots; no target/player position or health writes.
    for i in range(8):
        xdo('mousemove_relative','--',35,0);frames(5)
        capture('scan-'+str(i))
        xdo('mousedown',1);frames(8);xdo('mouseup',1);frames(3)
    capture('after-fire')
    # Explicit inventory fixture tests request -> ORIGINAL draw/selection sequence.
    # This is not natural acquisition evidence and does not patch equipped slot.
    addr=0x800d7198+0x2c4+5*4;data=struct.pack('<HH',1,20)
    report['fixture_writes']=[{'address':hex(addr),'hex':data.hex(),'purpose':'isolated shotgun inventory fixture; no equipped-slot/model writes, never save'}]
    for i,b in enumerate(data):call('write_ram',addr=hex(addr+i),val=hex(b))
    xdo('key','2');frames(60);capture('number-2')
    actual=int(call('read_ram',addr='0x800d7551',len=1)['hex'],16)
    assert actual==5,'number 2 failed original equipment transition'
    xdo('key','1');frames(60);capture('number-1')
    actual=int(call('read_ram',addr='0x800d7551',len=1)['hex'],16)
    assert actual==4,'number 1 failed original equipment transition'
    xdo('key','Escape');frames(3);capture('released')
    call('fntrace_arm_clear')
    verify(report,directory)
    flush()


def verify(report,directory):
    rows=report['aim_options']
    last=rows[-1]['input']['controls']
    assert last['selections']>=2,'numbered selection callback did not run'
    if report['red_dot']=='off': assert last['aim']['hidden_markers']>0,'marker suppression inactive'
    if report['crosshair']=='off': assert not any(r['input']['controls']['aim']['reticle'] for r in rows)
    if report['aim_assist']=='original-lock' and report['weapon_aim']=='view':
        assert last['aim']['assisted_shots']>0,'no live assisted shot'
        targets={key.lower() for r in rows for key in r['target_states']}
        entries=rows[-1]['actor_contact_trace']['entries']
        contacts=[e for e in entries if e['a0'].lower() in targets and e['ra']=='0x8006FD64']
        log=(directory/'runtime.log').read_text()
        owned=[e for e in contacts if 'ttk-flight projectile='+e['a1'].lower()[2:] in log]
        assert owned,'no player projectile contact with acquired target'
        report['actor_contact_assertions']={'contacts':len(owned),'targets':sorted(targets),'scope':'original projectile-to-actor contact, not quantified health/damage or all-weapon coverage'}
    report['aim_options_assertions']='passed: selection, independent marker configuration and live assist/contact checks'
