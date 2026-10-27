#!/usr/bin/env python3
"""D08K14: local textured walk/run flipbook. Retail-derived output stays ignored."""
import argparse
import json
import math
import os
from pathlib import Path

import numpy as np
from PIL import Image
import ttk_anim as t
import ttk_crouch_walk as cw


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    parser.add_argument('--html-only', action='store_true', help='Refresh the page using existing pose sheets')
    args = parser.parse_args()
    out = args.output
    out.mkdir(parents=True, exist_ok=True)
    if not args.html_only:
        group = t.Group(0)
        hierarchy = group.hierarchies[t.DUKE_HIERARCHY]
        original, _ = group.animation_quats(178, 0)
        views = {'side': (-math.pi / 2, .05), 'rear': (-math.pi * .22, .3), 'front': (math.pi, .08)}
        metrics = {}
        for direction in cw.DIRECTIONS:
            for running in (False, True):
                name = 'run' if running else 'walk'
                frames = cw.mocap_frames(group, direction, running=running)
                metrics[direction + '-' + name] = {'height': cw.pose_top(group, frames)}
                for view, (yaw, pitch) in views.items():
                    sheet = Image.new('RGBA', (320 * 48, 360))
                    for i in range(48):
                        frame = frames[round(i * len(frames) / 48) % len(frames)]
                        quats = dict(original)
                        quats.update(t.joint_quats_for_frame(frame))
                        quats.update(frame['upper'])
                        mats = t.pose(hierarchy, quats, root_position=(0, -frame['commands'][0]['root'][1], 0))
                        path = out / 'frame.png'
                        t.render(group, hierarchy, mats, path, size=(360, 320), yaw=yaw, pitch=pitch,
                                 distance=1900, center=np.array([0., -300., 0.]), background=(24, 26, 31))
                        with Image.open(path) as image:
                            pixels=np.array(image.convert('RGB')); alpha=np.where(np.all(pixels==[24,26,31],axis=2),0,255).astype('uint8')
                            sheet.paste(Image.fromarray(np.dstack((pixels,alpha))), (320*i, 0))
                    sheet.save(out / f'{direction}-{name}-{view}.png')
                print(direction, name, metrics[direction + '-' + name], flush=True)
        (out / 'frame.png').unlink()
        (out / 'metrics.json').write_text(json.dumps(metrics, indent=2) + '\n')
    html = HTML.replace('__RUN_CYCLE__', str(cw.RUN_CYCLE)).replace('__RUN_STRAFE__', str(cw.RUN_STRAFE_CYCLE))
    root = Path(__file__).resolve().parents[3]
    for name in ('walk', 'run'):
        reference = root / 'research/screencaps' / f'mgs-crouch-{name}.mp4'
        html = html.replace('__REFERENCE_' + name.upper() + '__', os.path.relpath(reference, out))
    html = html.replace('__ENGINEERING_NOTE__', os.path.relpath(root / 'documentation/286-d08k13-d08k14-crouch-movement.md', out))
    (out / 'index.html').write_text(html)


HTML = '''<!doctype html><html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>Duke - crouch movement study</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#101216;color:#e9e8e3;font:16px/1.5 system-ui,sans-serif}
main{max-width:1160px;margin:auto;padding:40px 24px}small{color:#e8b656;letter-spacing:.15em}h1{font-size:clamp(32px,5vw,60px);line-height:1.05;margin:16px 0}p{max-width:780px;color:#b4b7be}
.controls{display:flex;flex-wrap:wrap;gap:18px;align-items:center;padding:18px 0;border-block:1px solid #383b42;margin:28px 0}button,select{font:inherit;color:inherit;background:#282c34;border:1px solid #555c68;padding:8px 12px;border-radius:5px}label{display:flex;gap:8px;align-items:center}.grid{display:grid;grid-template-columns:1fr 1fr;gap:24px}.card{background:#181a1f;border:1px solid #383b42;border-radius:8px;overflow:hidden}.title{padding:18px 22px;display:flex;justify-content:space-between;gap:10px}.title span{color:#b4b7be;font-size:14px}canvas{display:block;width:100%;height:auto}.foot{padding:20px;background:#20232a;margin-top:26px;border-left:3px solid #e8b656}a{color:#e8b656}@media(max-width:650px){.grid{grid-template-columns:1fr}}
</style><main><small>D08K13 + D08K14 / ACCEPTED 27 OCT 2026</small><h1>Contact. Weight. Recovery.</h1>
<p>Accepted revision 2: rebuilt walk and run poses on Duke's skeleton. Hands hang lower, wrists follow the forearms, and the pelvis and opposing arms follow foot contact. Travel speed is unchanged. The run uses a longer stride with a short airborne recovery to reduce the extreme cadence.</p>
<div class="controls"><label>Direction <select id="direction"><option value="forward">Forward</option><option value="backward">Backward</option><option value="strafe_pos_x">Right strafe</option><option value="strafe_neg_x">Left strafe</option></select></label><label>View <select id="view"><option value="side">Side</option><option value="rear">Rear three-quarter</option><option value="front">Front</option></select></label><button id="play">Pause</button><label>Playback <select id="rate"><option value="1">1x</option><option value="0.5">0.5x</option><option value="0.25">0.25x</option></select></label><label><input id="sync" type="checkbox"> Match cycle timing</label><label>Phase <input id="phase" type="range" min="0" max="47" value="0"></label></div>
<div class="grid"><section class="card"><div class="title"><strong>Accepted walk</strong><span id="walkInfo"></span></div><canvas id="walk" width="320" height="360"></canvas></section><section class="card"><div class="title"><strong>Accepted run</strong><span id="runInfo"></span></div><canvas id="run" width="320" height="360"></canvas></section></div>
<div class="foot"><strong>Accepted movement study</strong><p>Look for grounded stance feet, a low head and torso, toe-off, opposite arm swing and clearance of the recovering boot. Pause and scrub to compare poses. Match cycle timing isolates the shape change.</p><p>At 1x, cadence uses 30 player updates per second and the configured walk/run step lengths. Actual gameplay cadence varies with simulation timing. This is a 48-pose textured flipbook, not a recording of in-game collision or walk/run blending. The accepted build passed sustained backwards crouch movement and direction/run switches in both views with both crouch controls. Native clearance, collision and ledge fixtures pass; natural vent traversal remains unverified.</p></div><p><strong>Locked in by user approval on 27 October 2026.</strong> Walk and run travel speeds are unchanged. The earlier 1000-unit run cycle was rejected; the accepted forward/backward run uses 1600 units per cycle with a 40% planted interval per foot and brief airborne recovery. Walk/run changes retain gait phase and blend from the displayed pose over six updates. <a href="__ENGINEERING_NOTE__">Complete engineering record, evidence and limitations</a>.</p><h2>Reference study</h2><p>Original local MGS references. The steady sections repeat at approximately 1.75s per walk cycle and 0.72s per run cycle. Duke retains the requested travel speed, so the run remains faster than this reference. Compare hand height, wrist alignment, torso control and the foot recovery shape.</p><div class="grid"><section><h3>MGS crouch walk</h3><video controls preload="metadata" style="width:100%" src="__REFERENCE_WALK__"></video></section><section><h3>MGS crouch run</h3><video controls preload="metadata" style="width:100%" src="__REFERENCE_RUN__"></video></section></div><p>Side-view ground ticks move at the modeled travel rate. Matching cycle timing is for pose comparison only; it changes preview time, not the player build. Duke's retail hands are rigid meshes, so fingers remain closed even with the relaxed wrist posture.</p></main>
<script>
const ids=['direction','view','play','rate','sync','phase'];const e=Object.fromEntries(ids.map(id=>[id,document.getElementById(id)]));let playing=true,time=0,last=performance.now();const images={};
function load(){for(const name of ['walk','run']){const im=new Image();im.src=`${e.direction.value}-${name}-${e.view.value}.png`;images[name]=im;}}
for(const id of ['direction','view'])e[id].onchange=load;e.play.onclick=()=>{playing=!playing;e.play.textContent=playing?'Pause':'Play';};e.phase.oninput=()=>{playing=false;e.play.textContent='Play';};
function draw(now){if(playing)time+=(now-last)/1000*Number(e.rate.value);last=now;const strafe=e.direction.value.startsWith('strafe');for(const name of ['walk','run']){const cycle=name==='walk'?(strafe?380:640):(strafe?__RUN_STRAFE__:__RUN_CYCLE__);const speed=(name==='walk'?20.8154296875:97.6171875)*30;const hz=e.sync.checked?1:speed/cycle;const frame=playing?Math.floor(time*hz*48)%48:Number(e.phase.value);const c=document.getElementById(name),ctx=c.getContext('2d');ctx.fillStyle='#181a1f';ctx.fillRect(0,0,320,360);if(e.view.value==='side'&&!strafe){const travel=playing?time*(e.sync.checked?cycle:speed):Number(e.phase.value)/48*cycle;const offset=((travel*576/1900*(e.direction.value==='backward'?-1:1))%68+68)%68;ctx.strokeStyle='#535966';ctx.beginPath();ctx.moveTo(0,271);ctx.lineTo(320,271);for(let x=-offset;x<320;x+=68){ctx.moveTo(x,271);ctx.lineTo(x,277);}ctx.stroke();}if(images[name]?.complete && images[name].naturalWidth)ctx.drawImage(images[name],frame*320,0,320,360,0,0,320,360);document.getElementById(name+'Info').textContent=`${cycle} units / cycle | ${(1/hz).toFixed(2)}s`; }requestAnimationFrame(draw);}load();requestAnimationFrame(draw);
</script>'''

if __name__ == '__main__':
    main()
