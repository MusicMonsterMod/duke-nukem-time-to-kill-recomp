"""D05/D06 route extension: real SDL events, read-only state, isolated D04 driver."""
import json
import math
import struct
import time
from ttk_state_probe import sample


def exercise(call, xdo, eventually, directory, report, flush, camera=False):
    rows = report['modern_controls'] = []
    def capture(name):
        s=sample(call)
        row={'name':name,'input':call('ttk_input')['input'],
             'state':{k:v for k,v in s.items() if k!='raw_hex'}}
        rows.append(row); flush()
        assert not (s['player_flags'] & 2) and s['player_animation'] != 218, name+' player died; not accepted as controls evidence'
        call('screenshot_file',path=str(directory/(name+'.png')))
        return row
    def frames(count):
        end=call('frame')['frame']+count
        eventually(lambda:call('frame')['frame']>=end,30)
    call('turbo',enabled=0)
    xdo('key','F10')
    eventually(lambda:call('ttk_input')['input']['controls']['ready'],20)
    capture('controls-start')
    (directory/'level00-guard-fixture.bin').write_bytes(bytes.fromhex(call('read_ram',addr='0x800ca968',len=9668)['hex']))
    movement_cases=[('forward',['w','Shift_L']),('back',['s','Shift_L']),('left',['a','Shift_L']),('right',['d','Shift_L']),('walk',['w']),('diagonal',['w','d','Shift_L'])]
    for name,keys in ([] if camera else movement_cases):
        before=capture(name+'-before')
        xdo('keydown',*keys); frames(18); xdo('keyup',*keys)
        after=capture(name+'-after'); frames(30)
        a=before['state']['player_position_candidate'];b=after['state']['player_position_candidate']
        displacement=[b[i]-a[i] for i in (0,2)]
        after['horizontal_displacement']=displacement
        assert after['input']['controls']['moves']>before['input']['controls']['moves'],name+' movement hook inactive'
        assert after['input']['controls']['probes']>before['input']['controls']['probes'],name+' collision probe hook inactive'
        assert math.hypot(*displacement)>10,name+' no displacement'
        # Expected view-relative sign. Walls may shorten but must not reverse it.
        m=before['state']['camera_matrix_q12'];fx,fz=m[6],m[8];n=math.hypot(fx,fz);fx/=n;fz/=n
        rx,rz=fz,-fx
        right=('d' in keys)-('a' in keys);forward=('w' in keys)-('s' in keys)
        expected=(fx*forward+rx*right,fz*forward+rz*right)
        assert sum(displacement[i]*expected[i] for i in (0,1))>0,name+' wrong direction'
        flush()
    if camera:
        before=capture('look-before')
        for _ in range(8): xdo('mousemove_relative','--',24,-8);frames(2)
        after=capture('look-after')
        assert after['input']['controls']['looks']>before['input']['controls']['looks']
        assert after['state']['camera_matrix_q12']!=before['state']['camera_matrix_q12']
        assert abs(after['state']['player_rotation_candidate'][0]-before['state']['player_rotation_candidate'][0])>8
        assert after['input']['controls']['facings']>before['input']['controls']['facings']
        # Separate stationary fire from running; inspect ammo in the capture.
        xdo('mousedown',1)
        for _ in range(10): xdo('mousemove_relative','--',-4,2);frames(2)
        xdo('mouseup',1);capture('standing-fire-look')
        moving_before=capture('move-look-before')
        xdo('keydown','w')
        for _ in range(6): xdo('mousemove_relative','--',-8,0);frames(2)
        moving_after=capture('move-look-after')
        assert moving_after['input']['controls']['moves']>moving_before['input']['controls']['moves']
        xdo('mousedown',1)
        for _ in range(8): xdo('mousemove_relative','--',-16,4);frames(2)
        xdo('keyup','w');xdo('mouseup',1);capture('move-fire-look');frames(20)
        xdo('key','Escape');frames(2)
        released=capture('released')
        xdo('mousemove_relative','--',400,200);frames(2)
        xdo('key','F10');frames(3);recaptured=capture('recaptured')
        assert recaptured['input']['controls']['looks']==released['input']['controls']['looks']
        # Real window focus loss during gameplay, with relative motion outside
        # capture. Re-entry must not consume those counts.
        sink=xdo('search','--name','^D04 focus sink$').splitlines()[0]
        game=xdo('search','--name','Duke').splitlines()[0]
        previous=recaptured['input']['controls']['looks']
        xdo('windowfocus','--sync',sink);frames(2)
        assert not call('ttk_input')['input']['captured']
        xdo('mousemove_relative','--',200,100)
        xdo('windowfocus','--sync',game);frames(2)
        assert not call('ttk_input')['input']['captured']
        xdo('key','F10');frames(3);focused=capture('focus-recaptured')
        assert focused['input']['controls']['looks']==previous
        # Pause and unpause through the real key path. Input is released before
        # the guest sees Start. Screenshot is retained for menu/state review.
        xdo('keydown','Return');frames(3);xdo('keyup','Return');frames(3)
        assert not call('ttk_input')['input']['captured']
        call('screenshot_file',path=str(directory/'paused.png'))
        xdo('mousemove_relative','--',200,100)
        xdo('keydown','Return');frames(3);xdo('keyup','Return');frames(3)
        xdo('key','F10');frames(3);resumed=capture('pause-recaptured')
        assert resumed['input']['controls']['looks']==previous
    else:
        xdo('keydown','Right');frames(30);xdo('keyup','Right');frames(30)
        before=capture('rotated-before');xdo('keydown','w');frames(18);xdo('keyup','w')
        after=capture('rotated-after')
        a=before['state']['player_position_candidate'];b=after['state']['player_position_candidate'];m=before['state']['camera_matrix_q12']
        assert (b[0]-a[0])*m[6]+(b[2]-a[2])*m[8]>0
    xdo('key','Escape');frames(5)
    assert not call('ttk_input')['input']['controls']['ready']
    report['modern_controls_assertions']='passed; screenshots and terrain acceptance require review'
    flush()
