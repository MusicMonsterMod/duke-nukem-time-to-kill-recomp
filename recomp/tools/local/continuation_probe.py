"""Isolated D07 follow-up observations, real SDL inputs; no gameplay RAM writes."""
import json
import time
from ttk_state_probe import sample


def exercise(call, xdo, eventually, directory, report, flush):
    rows = report['continuation'] = []
    def frames(n):
        end = call('frame')['frame'] + n
        eventually(lambda: call('frame')['frame'] >= end, 30)
    def capture(name):
        s = sample(call)
        assert not s['player_flags']&2, name+' player died; reject this replay'
        row = {'name': name, 'state': {k:v for k,v in s.items() if k != 'raw_hex'},
               'input': call('ttk_input')['input'], 'audio': call('audio_stats'),
               'frame': call('frame')['frame'], 'time': time.monotonic()}
        rows.append(row); flush()
        return row
    call('turbo', enabled=0)
    xdo('key','F10');frames(8)
    (directory/'level00-guard-fixture.bin').write_bytes(bytes.fromhex(call('read_ram',addr='0x800ca968',len=9668)['hex']))
    capture('warmup')
    # Fixed wall-clock window measures output starvation with a dummy SDL device.
    # It is not listening evidence. No debugger polling inside each window.
    for name in ('idle', 'look'):
        before = capture(name+'-before')
        if name == 'idle': time.sleep(2)
        else:
            for i in range(20):
                xdo('mousemove_relative','--',8 if i<10 else -8,0);time.sleep(.1)
        after = capture(name+'-after')
        report.setdefault('timing',{})[name] = {
            'guest_fps': (after['frame']-before['frame'])/(after['time']-before['time']),
            'underrun_samples_added': after['audio']['out']['underruns']-before['audio']['out']['underruns']}
        flush()
    xdo('keydown','e');frames(3)
    armed=capture('interact-armed')
    assert armed['input']['pad'] & 16384, 'E must never fire a drawn weapon'
    xdo('keyup','e')
    xdo('keydown','s');frames(6);capture('speed-walk')
    xdo('keydown','Shift_L');frames(3);capture('speed-run')
    xdo('keyup','Shift_L');frames(3);capture('speed-walk-again')
    xdo('keyup','s');frames(30)
    for name, keys in [('run',['s','Shift_L'])]:
        xdo('keydown',*keys)
        for i in range(5): frames(3);capture(name+str(i))
        xdo('keydown','j');frames(4);capture(name+'-jump');xdo('keyup','j')
        frames(12);capture(name+'-air');xdo('keyup',*keys);frames(40);capture(name+'-land')
    verify(report)
    shot=call('present_shot',path=str(directory/'final.png'))
    eventually(lambda:call('present_shot_seq')['seq']>shot['seq'],10)
    xdo('key','Escape')


def verify(report):
    rows={r['name']:r for r in report['continuation']}
    assert not any(r['state']['player_flags']&2 for r in rows.values()), 'death-contaminated route'
    before=rows['run4'];after=rows['run-air']
    a=before['state']['player_position_candidate'];b=after['state']['player_position_candidate']
    matrix=before['state']['camera_matrix_q12']
    assert (b[0]-a[0])*matrix[6]+(b[2]-a[2])*matrix[8]<0, 'running jump reversed backpedal'
    assert after['input']['controls']['jumps']>before['input']['controls']['jumps'], 'jump adapter inactive'
    assert after['input']['controls']['cameras']>rows['run-jump']['input']['controls']['cameras'], 'airborne normal camera lease lost'
    assert rows['speed-walk']['state']['player_animation'] in (72,74)
    assert rows['speed-run']['state']['player_animation'] in (76,78)
    assert rows['speed-walk-again']['state']['player_animation'] in (72,74)
    assert rows['speed-walk-again']['input']['controls']['speed_changes']>=2
    report['continuation_assertions']='passed: armed E does not fire; speed transitions; backward running-jump direction and airborne camera ownership'
