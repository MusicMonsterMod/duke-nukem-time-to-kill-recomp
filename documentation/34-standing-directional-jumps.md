# D08C — Directional jumps from standstill: accepted

**Done on explicit user acceptance, 2026-09-27.** The user first confirmed that
W→Space worked. After testing the Space→W follow-up, they reported:

> its working!! beautiful. update the docs big time with this one

This accepts the bounded directional standing/walking jump and input-order
iteration. The accepted player build is
`92b6cac55a2e83f4f7c9e555a649d382a431b041d4378ed2cc228fc98f1724f5`.
This acceptance supersedes earlier D08C Needs playtest statements. D08's accepted
running jumps and ladder transfers remain established regression requirements.

## Accepted player behavior

| Input | Behavior |
| --- | --- |
| W/A/S/D then Space | Directional jump from standing or walking, without a run-up |
| Space then W/A/S/D during preparation | Neutral preparation picks up the first direction and launches toward it |
| Diagonal direction plus Space | Camera-relative diagonal jump; both input orders supported during preparation |
| Space alone | Original vertical jump |
| Release direction after it is chosen | Accepted takeoff direction persists |
| Continue holding movement through a clear running-jump landing | Existing running continuation remains |

The original takeoff animation is not restarted to wait for input. Direction is
fixed for the jump; this does not add full airborne steering. A movement press
after vertical flight has already begun is outside the new preparation handoff.
Original clearance can refuse a directional takeoff near an obstruction.

No bindings or profile schema changed. Custom bindings and Vanilla remain intact.
E stays interaction-only, C stays holster, right-click stays precision aim, wheel
selects weapons, Alt-wheel adjusts camera distance, Escape/Enter send Start, and
F10 toggles capture. These remain regression requirements for subsequent work.

## Why the two fixes were necessary

The initial replay reproduced simultaneous forward+jump entering original
preparation **96**, requesting vertical flight **97**. The movement/camera lease
recognized ordinary ground and running flight, but dropped out during preparation.
Modernized WASD therefore stopped supplying Forward while original `8005492c`
was choosing the jump direction. Vertical flight has a horizontal velocity field,
but its handler suppresses horizontal ascent movement: velocity alignment alone
was insufficient evidence that the jump actually moved forward.

A second path entered walking **72** just before the jump press. The original
walking handler `80053404` has no jump-selection branch. Preserving the preparation
lease alone could not fix that path, so a fresh buffered jump press now requests
original preparation from verified standing/walking states.

The first D08C delivery still required direction at initializer entry. That fixed
W→Space but missed Space→W, as the user reported. The accepted follow-up separates
ownership of **neutral preparation** from a **chosen direction**. Neutral 96 keeps
the camera/input handoff without sending synthetic Forward. The first direction
arriving during preparation is then latched without restarting the animation.

## Current engineering contract

At original lower-body update `8005a210`, authenticated by player, table argument
`800c2754` and caller return `80041b34`, a fresh eight-input-frame jump press with
movement requests preparation 96 from standing 63 or walking 72–75. It consumes
that buffered press and resets only the body track's initialization/reverse fields.
Original initialization, animation events and completion still run. Running 76–79
and active jetpack thrust bypass this request; key repeat cannot renew the buffer.

Original-selected preparation is recognized at initializer `800493a4`, called by
`8005a210` at returns `8005a3a0` or `8005a4d0`. It requires uninitialized animation
96, normal ground state 0/0, a recent authenticated grounded-camera sample and
active Modernized input. **Direction is optional at this point.** A separate
recent-ground sample survives the camera-before-initializer seam, expires after
four input frames and cannot cross capture epochs.

Preparation ownership is bounded to 60 host input frames and to the original
preparation state; this is a safety timeout, not a promised one-second input
window. If still neutral, the verified lower-body update accepts the first nonzero
camera-relative direction. It does not rewrite animation progress. Once direction
is chosen, releasing or changing the keys cannot replace that jump's direction.
Neutral preparation supplies no synthetic Forward, so Space alone stays vertical.

Owned directional preparation supplies original Forward to `8005492c`. At the
`800797f4` call to `800780b4`, both the 375-unit clearance-query vector and endpoint
are rotated to the chosen direction. Original room, headroom, wall and subsequent
segment tests still decide whether to select directional jump **98**. No clearance
result is fabricated and no player position is written.

Original 98 initialization supplies TTK's standing-directional ballistic parameters.
Its first `80055904` ballistic update redirects only horizontal velocity once,
before original gravity, swept collision and integration. Vertical velocity is
not retuned; later impacts retain their own responses. Original vertical **97**
remains when no direction is chosen or clearance refuses 98. Running **103/104**
retain their previous selection and speed. Owned 98 also retains the established
airborne camera/input/E-reach handoff.

Wrong actors/callers, dead actors, incompatible flags, precision aim, crouch,
stale capture epochs, focus/menu interruption and unsupported states are rejected.
Vanilla and the original-camera option bypass this extension. Original code,
dispatch tables and ballistic tables remain identity guarded. The implementation
is bounded to authenticated first-map normal-ground ownership.

## Evidence and acceptance

Human evidence is separate from automated evidence:

- The user explicitly reported W→Space working on the initial delivery.
- The user then accepted the Space→W follow-up on the current delivered iteration.
- The user did not enumerate every diagonal, terrain, map or traversal combination;
  this sign-off is not converted into a claim that all such cases were human-tested.

Final-delivery automated evidence is recorded in
[the input-order report](reports/d08c-input-order.json):

| Check | Result and scope |
| --- | --- |
| Build | Established incremental CMake workflow; imported/generated/patch inputs from the preceding full build, no new generated hooks |
| Python | 57 passed, zero skipped |
| Native | Input, controls, aim and scene suites pass; late direction, neutral no-Forward, unchanged animation progress, release, focus/epoch, airborne rejection and existing contracts covered |
| Real SDL input-order route | Six Space→direction cases (four cardinal directions, two diagonals), Space alone and W→Space pass; exit 0 |
| Direction-start samples | Every Space-first case is sampled in original 96 requesting vertical 97 before direction arrives, then enters 98 with one correctly aligned correction |
| Vanilla route | Same binary completes 14 checkpoints, exit 0; original vertical-jump capture reviewed |

The SDL driver uses a private J jump rebind to exercise the action layer, private
settings/cards/display, and logged health-only longevity fixtures. It never writes
player position, velocity or animation. Nominal waits and grouped event summaries
are not exact human-input latency measurements. Automated captures are not human
feel acceptance. All private test sessions exited after verification.

## Historical implementation evidence

[The first-delivery report](reports/d08c-standing-jumps.json) retains its original
binary identity `808878cfc35a02451f720f473b0dd141b74326b659833c4bd75d32a9ad69fecf`
and results rather than attributing those runs to the later build:

- Baseline `d08c-baseline-01` reproduces the vertical jump on the earlier controls
  build `fdc8d4ece50a4a98248933684e5efa7f51b182b965b3cf7f347c4971e5065c28`.
- `d08c-standing-01` exposed incomplete initializer/camera ownership; `-02`
  exposed the walking handler's missing jump branch and same-update initializer
  caller. These rejected iterations are not counted as passes.
- `d08c-standing-03` passes seven directional cases and a vertical control, with
  roughly 1,640–1,790 units of directional horizontal movement versus about 60
  for the vertical control. Both early release and short run-ups are covered.
- `d08c-running-01` passes six running jumps in all four directions: five clear
  landings continue running, one obstacle impact retains original recovery.
  That route sampled 103; both 103/104 are covered by native checks.
- The earlier binary also completed its own 14-checkpoint Vanilla route. Those
  captures and its full build/import/patch results remain historical evidence.

## Remaining scope and handoff

D08C is **Done**; do not reopen the accepted input-order/feel work without new
feedback. Broader coverage stays under D08B: other overlays/maps, moving supports,
slopes, ceilings, oblique ladder/ledge combinations, swimming, scripted cameras and
campaign progression. Exact ladder transfer was not replayed in the D08C pass;
D08's earlier human acceptance and existing native reach/E-never-fire checks remain.
Full airborne steering would be a separately selected behavior change.

D08A's unverified item/era criteria and D10's broader camera criteria remain open.
This documentation acceptance turn changed no code, executable, settings, cards or
original media, and launched no game or new job.

Launch the accepted build with saved preferences:

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```

Modernized captures supported gameplay automatically; F10 toggles capture.
