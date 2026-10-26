# D08A31 - Number keys while holding run

2026-10-26. Done - closed with no action required by explicit user direction:
"close this one with no action required and close us out". Documentation
closeout and commit/push authorized. No gameplay fix claimed or required.

## Findings

U: Left Shift + W + 1/2 slows Duke without selecting melee/pistol. Shift
while stationary and W without Shift work. Slots 3/4 work while running.
User reproduced on slot 4 after dnmonsters and dnstuff.

S: input uses physical SDL scancodes. Both Shift keys satisfy the run binding;
number selection uses the same native request path as the wheel. D08A29
already admits run-ready upper loops 18/19.

L: isolated X11 key events on the unchanged build select 1/2/3/4 at 60/120.
An exact slot-4 copy with both cheats and Shift+W at 120 completes 1/2
selection while lower 78 continues. Earlier longer slot-8 routes reached
idle/wall-stop states; those do not prove uninterrupted locomotion.

L + U: private physical-key trace, correlated and confirmed by the user,
shows repeated Left Shift releases/represses while W stays held and the
user presses 1/2 without physically releasing Shift. No 1/2 event reaches
the game. At frames 2000/2066 and 2342/2398, 3/4 arrive with Shift held and
select weapons normally. This explains both walking and missing selection.

L + U: Caps Lock RUN MODE receives 1/2 at frames 5718/5790 and 6100/6174,
selecting Boot 0/pistol 4 with running lower 76/78. Right Shift with autorun
off also receives 1/2 at 7776/7853 and 8096/8160 with running lower 76/78.
Both alternatives work, but the user rejects Right Shift as an ergonomic
FPS replacement. Left Shift remains the intended run binding.

U: identical failure reproduced in EDuke. This is strong evidence for an
external input-combination problem. Keyboard hardware/firmware is the leading
hypothesis, not confirmed by a second-keyboard test. A bounded XInput check
recorded no matching events and provides no extra attribution. Direct input
device-node access was unavailable; no privileges or system settings changed.

## Closeout

Temporary opt-in run-key tracing and its additional native input regression
matrix were removed at no-action closeout. The matrix passed during the
investigation, covering all ten slots, WASD, both Shift keys and RUN MODE.
The normal player was rebuilt from unchanged accepted implementation source.
Closeout build and the restored native SDL input suite under Xvfb pass.
Executable SHA256 exactly matches the previously accepted D36K1 build:
`1af5089a7d377ae6480dadfd496a97724797f743e538259ca29b3cd18c430427`.
No gameplay, control binding, Vanilla, framework or manual change remains.
The existing framework working-tree patch was preserved untouched.

All test writes used private profile/card copies. Player originals were not
writable targets. Local evidence remains under ignored `recomp/analysis/d08a31/`:
`slot4-120/` and `physical-left-shift/runtime.log` contain the focused evidence.
The traced game and bounded input monitor are stopped. No next job started.

Launch: `python3 recomp/tools/local/run.py`.
