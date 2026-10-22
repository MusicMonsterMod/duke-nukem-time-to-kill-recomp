# D08K5 - Final crouch-walk coverage

Status: Done (user-accepted 2026-10-22), completing D08K too. User:
"total acceptance! lets lock it in and close this one out!!!"
Documentation, commit and push closeout explicitly authorized. Verification
of the final accepted feature, with no implementation or player-setting changes.

User reports extensive continued use and is already happy with crouch walking.
This supplements, rather than specifies, the individual cases below.

Executable: `c8524d2c08d0b935b92f54780642d9e2e92e9f218ac88823212278d491aa0318`
(the D17R2 accepted build). Native targets were current: Ninja reported no
work to do. Existing framework edits were preserved without modification.

## Fresh verification

Private offscreen OpenGL runs, copied prior lab saves/cards, private settings
and port 9415. Evidence and scripts: `recomp/analysis/d08k5-coverage/` (local,
ignored). No player save/card/settings directory was opened by these runs.

| Case | Evidence |
| --- | --- |
| Several levels/eras | Levels 0, 3, 6 and 9, third person at 120 Hz; levels 3 and 9, first person at 120 Hz. 156 recorded samples: enter, forward/back/strafe/diagonal, stop and release, weapon selection/fire, save/load and recovery. No original roll 181-184 in these samples. |
| Weapon use/combat | Shot counters advance during crouched fire. Actual enemy hits reduce health; the factory also exercises native hit reactions 161/162, followed by successful crouch/stand recovery. Its final first-person walk sample is a lethal hit, not a successful walking sample. |
| Crouched save/load | Five valid held-crouch restores return to 178 and can move/stand. First-person level 9's attempted crouch save instead catches an enemy hit reaction; counted as combat recovery, not a sixth crouched restore. |
| Cameras/rates | First-person requested/supported throughout both 120 Hz level samples; presented captures reviewed. 60 Hz first-person checks cover crouch, jump, ladder and swimming transitions. Detailed six-weapon high-refresh presentation remains covered by accepted D08K10 and its rerun native contracts. |
| Jump | Release crouch, Space enters native airborne 97/mode 9, then native landing. Holding Ctrl with Space retains crouch; it is not evidence of a jump. Held-crouch touchdown is covered by accepted D08K9 and its rerun native cases. |
| Ladder | Crouch/release, E attaches (186/mode 3), S descends to ground, Ctrl+W crouch-walks (178), release stands (63). |
| Mantle | Three crate approaches after crouching enter native climb 139/140, mode 8, then crouch-walk (178) and stand (63). Initial approach missed because the test's mouse delta was applied more than once; closed-loop facing fixes the harness. No gameplay correction. |
| Swim | Private deep-water fixture: Ctrl dives (133/mode 5), W swims (128/mode 5), Space surfaces (122/mode 4). It does not become a crouch. Shallow/mid-water and shoreline coverage is retained from accepted D08K11, D08AC/AD/AE. |
| Death/Continue | Actual enemy kill while crouched, normal death menu, Continue restores health to 10000 and resumes gameplay; Ctrl+W returns to 178, release to 63. Verified first person at 60 Hz with `DNTTK_TEST_EDGES=1`. Earlier attempts without edge mode reasserted cleared menu keys and are not counted as passes. |
| Vanilla route | `vanilla_regression.py --name d08k5-20261022-01 --mode vanilla --port 9415`: exit 0; captures reviewed for original menus, 100 health/200 ammo spawn, firing/ammo decrease, holster/draw, airborne jump/landing, inventory open/close, forward travel and turning. Enemy damage during the route is visible and expected. |
| Native regression | `ttk-input-test`, `ttk-aim-test EXE`, `ttk-controls-test EXE LEVEL00_FIXTURE LEVEL01 LEVELS_DIR`: pass, including all 21 selectable overlay guards, low hull, standing clearance, object tops, slope/edge refusals, landing, shallow water, crouched viewmodel and Vanilla exclusion contracts. |

## Terrain evidence and limits

Accepted [D08K2](185-d08k2-crouch-hull.md) records curb/stair traversal,
edge stops in levels 5 and 6, wall sliding and clearance tests. Accepted
[D08K7](189-d08k7-crouch-on-objects.md) covers object tops and safe stand-up.
Those results are retained rather than presented as new runs here. The fresh
native suite rechecks the final hull/stand-up logic, including low clearance,
steep slope, excessive steps/drops, walls and object handling.

Earlier scans found no natural crouch-only passage near the sampled level
starts. Low-ceiling/release-under-cover safety is fixture-proven; there is no
new claim of a real crouch-only vent route. The original object probe does not
make standing-blocking props passable by crouching. This is bounded multi-era
coverage, not an exhaustive campaign traversal.

The accepted feature still uses the dominant directional animation for
diagonals and has no crouch-walk footsteps. Neither changed in this pass.

## Review outcome

No gameplay defect found and no implementation change required. The user
fully accepted this pass, completing D08K5 and the D08K parent. The existing
extensive playtest is recorded above; no full campaign replay was requested.
The executable and launcher behavior are unchanged. No next job started.

Launch from the workspace: `python3 recomp/tools/local/run.py` (F10 if mouse
capture is needed).
