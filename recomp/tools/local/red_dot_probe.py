"""D07D isolated real-input red-dot observations (pc_input_probe --controls red-dot).

Natural pistol scans in third person, held original aim (Mouse2) and first
person. Records the marker hook's hidden count per segment and presented-window
captures. No guest memory writes; the state sampler is not used.
"""


def exercise(call,xdo,eventually,directory,report,flush):
    rows=report['red_dot_segments']=[]
    def frames(n):
        end=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=end,30)
    def hidden():return call('ttk_input')['input']['controls']['aim']['hidden_markers']
    def shot(name):
        s=call('present_shot',path=str(directory/(name+'.png')))
        eventually(lambda:call('present_shot_seq')['seq']>s['seq'],10)
    def scan(name,steps=8,before=None,after=None):
        start=hidden();targets=0
        if before:before()
        for i in range(steps):
            xdo('mousemove_relative','--',35,0);frames(5)
            ram=call('read_ram',addr='0x800d7420',len=4)['hex'] # player +0x288 acquired actor
            if int.from_bytes(bytes.fromhex(ram),'little'):targets+=1
            shot(f'{name}-{i}')
            xdo('mousedown',1);frames(8);xdo('mouseup',1);frames(3)
        if after:after()
        rows.append({'segment':name,'hidden_markers':hidden()-start,'frames_with_acquired_target':targets,
                     'view':call('ttk_input')['input']['controls'].get('view')})
        flush()
    call('turbo',enabled=0);frames(6)
    scan('third')
    scan('held-aim',before=lambda:(xdo('mousedown',3),frames(10)),after=lambda:xdo('mouseup',3))
    xdo('key','p');frames(40)
    scan('first-person')
    xdo('key','p');frames(40)
    verify(report)
    flush()


def verify(report):
    rows=report['red_dot_segments']
    acquired=[r for r in rows if r['frames_with_acquired_target']]
    assert acquired,'no natural target acquisition; marker was never requested'
    if report['red_dot']=='off':
        assert all(r['hidden_markers']>0 for r in acquired),'a segment with an acquired target drew the red dot'
    else:
        assert all(r['hidden_markers']==0 for r in rows),'red dot hidden although enabled'
    report['red_dot_assertions']='passed: hidden count per segment matches the red_dot setting'
