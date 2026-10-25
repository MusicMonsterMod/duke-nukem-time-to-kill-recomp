# D35F1 - Chain visibility across room boundaries

2026-10-25. Done, user-accepted. User: "totally accept. lock it in and close us out".
Explicit implementation, documentation, commit and push closeout authorized.

## Reproduction and cause

The current UI slot 10 was copied to private test storage, with its thumbnail
and title sidecar. No player memory card was copied or opened. Save SHA-256:
`8f2de7201621a0f57236d910ed7a90198ff1d1d7b443c3eb4ae5b409be615438`.
The attached chain is type 613 at `0x801dda64`, owned by room 43. It extends
up into room 44. Duke starts attached at `[-3102, -4604, 34552]`.

The accepted build reproduces the report. While looking up and ascending,
room 43 disappears from the native visible-room list between the sampled
heights -4772 and -5025. Rooms 44 and 49 remain. The chain's attachment and
first-person policy remain valid, but the native object pass at `0x800374cc`
only walks objects belonging to listed rooms. Consequently the whole chain
stops reaching the renderer. Its bounding-sphere near check was still passing
before that exclusion; this was not a near-plane failure.

Evidence combines native-code disassembly, the visible-room/attachment trace,
primitive traces and before/after images. Temporary address-specific tracing
was removed from the candidate.

## Correction

`pole_visibility.inc` submits the attached pole-family static object through
the original `0x80031d10` renderer when its owning room is absent. The existing
`0x8002ffec` scene hook provides the insertion point. It does not add a room,
draw other objects in that room, move the chain or change attachment/collision.
The original object renderer and polygon clipper retain their visibility,
transparency and depth behavior. A visible owning room means no extra draw.

The rule requires Modernized, the independent camera following Duke, a valid
orbit, authenticated code/overlay, pole-family flags, the static renderer kind,
and a valid visible-room list. Both action-perspective choices use it. Vanilla
returns before this path. No type ID, level ID or reported object address is
hard-coded in the fix.

The preceding attachment is retained only through native exit poses 191/196,
then cleared. It is cleared on world/save changes and included in the existing
render replay snapshot. The native draw runs on a CPU copy with stack and
scratchpad restoration. New code guards cover the renderer and insertion call.
No generated C, hook configuration, framework source or submodule pin changed.

## Verification

Accepted executable SHA-256:
`c604343e8378f0f8dcb438e892fa4e1082f3d94ad58cabbfba6085cc1303b306`.

- Player build passes. The unchanged movie/math shard checks passed during
  the full build; no code-generation/configuration change was required.
- Controls suite passes against the owned EXE, clean LEVEL00 fixture,
  LEVEL01 and all 21 selectable overlays. Added cases cover both action
  choices, room exclusion versus duplicate-draw prevention, caller/object
  restrictions, code mismatch, Vanilla exclusion, unchanged source CPU,
  stack/scratch restoration, unchanged attachment/room list, exit provenance,
  replay restoration and clearing that provenance on save load.
- Native input, aiming and near-clipping suites pass.
- Private Xvfb/OpenGL first-person route at the 120 FPS setting: ascent
  through the previously failing region, upward/downward look, turning away
  and descent. Chain and arms inspected in the images. No newly visible
  through-wall chain was observed in these views.
- First-person route at 60 and Third Person action route at 120: upper-chain
  approach, looking down the chain, native 196 climb-off and return to ground
  control. Third Person stays selected for the action and returns to the
  global first-person view after the exit.
- Private 120-setting drop route: Ctrl held for eight host frames releases
  the chain into native fall 108, then landing 105 with first-person control.
- With room 43 absent, the final 120-setting primitive trace records chain
  mesh draws and emitted triangles in the live process and all three replay
  workers (6 live meshes; 5, 6 and 2 worker meshes in the bounded capture).

Private scripts, traces, copied save and captures: `recomp/analysis/d35f1/`.
The early multi-load coverage script left later cases paused; those later
cases are not claimed as evidence. Final angle and exit routes use separate
fresh sessions. Rate settings are not performance benchmarks; dummy audio
is not an audible-sound test.

## Acceptance and coverage limits

The user fully accepted the candidate and authorized closeout. Their acceptance
does not enumerate additional playtest routes. Whole-campaign
pole/chain occlusion is not established by this single reported fixture.
A save loaded directly into an attachment-cleared exit has no preceding
host attachment provenance; native room visibility still applies there.

Authored implementation, tests and documentation are included in the authorized
MusicMonsterMod closeout. Runtime patch re-export was unchanged; applying it to
the pinned clean framework index reproduced all 51 changed/new files. No pins
changed. No post-acceptance gameplay edits, builds or launches; no player data
touched and no next job started.

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Gameplay captures automatically. P selects global First Person if needed;
CHAIN/POLE VIEW is under OPTIONS > ADVANCED > ACTION PERSPECTIVE.
