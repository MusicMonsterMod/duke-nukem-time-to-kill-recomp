"""D07A natural pistol pitch/walking presentation; no guest memory writes."""
import math
from ttk_state_probe import sample


def exercise(call,xdo,eventually,directory,report,flush):
    rows=report['presentation_samples']=[]
    def frames(n):
        end=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=end,30)
    def capture(name):
        s=sample(call);i=call('ttk_input')['input']
        assert not s['player_flags']&2 and s['player_animation']!=218,name+' death'
        m=s['camera_matrix_q12'];pitch=math.degrees(math.atan2(m[7],math.hypot(m[6],m[8])))
        rows.append({'name':name,'state':{k:v for k,v in s.items() if k!='raw_hex'},'input':i,'view_pitch_degrees':pitch});flush()
        call('screenshot_file',path=str(directory/(name+'.png')))
        shot=call('present_shot',path=str(directory/(name+'-present.png')))
        eventually(lambda:call('present_shot_seq')['seq']>shot['seq'],10)
        return rows[-1]
    call('turbo',enabled=0);xdo('key','F10');frames(6)
    start=capture('start')
    xdo('mousemove_relative','--',220,-240);frames(36)
    xdo('mousedown',1);frames(18);up=capture('aim-up');xdo('mouseup',1)
    assert up['input']['controls']['arms']>start['input']['controls']['arms']
    assert up['view_pitch_degrees'] < -10
    xdo('mousemove_relative','--',-140,420);frames(36)
    xdo('keydown','w');xdo('mousedown',1);frames(18)
    down=capture('walk-fire-down');xdo('mouseup',1);xdo('keyup','w');frames(12)
    assert down['view_pitch_degrees']>10
    assert down['input']['controls']['moves']>up['input']['controls']['moves']
    assert down['input']['controls']['arms']>up['input']['controls']['arms']
    assert down['state']['equipment_state_3b8']==2 and not down['state']['player_flags_224']&2
    assert down['input']['controls']['aim']['reticle']
    assert down['state']['player_rotation_candidate'][1]==0 and up['state']['player_rotation_candidate'][1]==0
    xdo('key','Escape');frames(6);released=capture('released')
    assert not released['input']['controls']['aim']['reticle']
    report['presentation_assertions']='passed; images need review; not all-animation continuity';flush()
