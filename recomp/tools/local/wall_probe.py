"""Natural first-map approach and turn/backpedal wall regression, no RAM writes."""
import math
import hashlib
from pathlib import Path
from ttk_state_probe import sample


def exercise(call,xdo,eventually,directory,report,flush):
    report['wall_driver_sha256']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    rows=report['wall_samples']=[]
    def frames(n):
        end=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=end,30)
    def capture(name):
        s=sample(call);i=call('ttk_input')['input'];m=s['camera_matrix_q12']
        assert not (s['player_flags'] & 2) and s['player_animation'] != 218, name+' player died; reject route'
        rows.append({'name':name,'state':{k:v for k,v in s.items() if k!='raw_hex'},'input':i,'actual_yaw':math.atan2(m[6],m[8])});flush()
        call('screenshot_file',path=str(directory/(name+'.png')))
        return rows[-1]
    call('turbo',enabled=0);xdo('key','F10');frames(6)
    assert capture('start')['input']['controls'].get('orientations',0)>0, 'Final orientation hook inactive'
    xdo('keydown','Shift_L','w')
    for j in range(6):frames(30);capture('approach-'+str(j))
    xdo('keyup','Shift_L','w');frames(12);capture('wall-stop')
    for j in range(8):
        xdo('mousemove_relative','--',75,0);frames(6);capture('turn-'+str(j))
    # Sweep past a full turn in both directions, while still against the entrance wall.
    for direction in (1,-1):
        previous=capture('orbit-start-'+str(direction))
        total=0
        for j in range(5):
            xdo('mousemove_relative','--',direction*600,0);frames(16)
            current=capture('orbit-'+str(direction)+'-'+str(j))
            d=math.atan2(math.sin(current['actual_yaw']-previous['actual_yaw']),
                         math.cos(current['actual_yaw']-previous['actual_yaw']))
            error=math.atan2(math.sin(current['actual_yaw']-current['input']['controls']['yaw']),
                             math.cos(current['actual_yaw']-current['input']['controls']['yaw']))
            assert abs(error)<math.radians(20), 'Visible yaw pinned away from requested view'
            assert direction*d>0, 'Camera reversed while mouse kept turning'
            total+=d;previous=current
        assert direction*total > math.pi, 'Camera sweep did not turn through half a circle'
        report['orbit_total_'+str(direction)]=total;flush()

    before=capture('back-before');xdo('keydown','s');frames(45);xdo('keyup','s');after=capture('back-after')
    a=before['state']['player_position_candidate'];b=after['state']['player_position_candidate']
    report['back_displacement']=math.hypot(b[0]-a[0],b[2]-a[2]);flush()

    assert report['back_displacement'] > 100, 'Could not back away from wall'

    # Speed contract through real SDL/SIO, with no saved preference edits.
    for name,keys,walk in [('default-walk',['s'],True),('shift-run',['s','Shift_L'],False),
                           ('caps-run',['s'],False),('caps-shift-walk',['s','Shift_L'],True),
                           ('caps-off-walk',['s'],True)]:
        if name in ('caps-run','caps-off-walk'):xdo('key','Caps_Lock')
        xdo('keydown',*keys);frames(16);r=capture(name)
        assert bool(r['input']['pad'] & 1024) != walk, name+' incorrect speed pad'
        assert bool(r['state']['player_flags_224'] & 2) != walk, name+' incorrect guest speed'
        xdo('keyup',*keys);frames(16)
