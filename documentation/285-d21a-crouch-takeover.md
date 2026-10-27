# D21A - Momentary crouch takes over the toggle

2026-10-27. Done, user-accepted. User confirmed D08K12 crouch running works and
requested this small correction to D21's Ctrl cancellation behavior.

## Behavior

With C-toggle crouch active, pressing the configured momentary CROUCH action
clears the toggle but keeps crouch asserted throughout the physical hold.
Releasing requests standing through the existing headroom check. No fresh press
is needed, and there is no standing transition at takeover. This works for
rebound keyboard and mouse inputs because both use `momentary_crouch_press`.

The change clears `crouch_cancel_held` rather than setting it when momentary
crouch takes over a latch. Space cancellation retains its independent consumed
jump/held-crouch handling. C still toggles; ordinary Ctrl remains momentary.
Run selection, collision, blocked standing and save/load ownership are unchanged.
No framework, generated-code, profile-schema or animation changes.

## Verification

Player build and native SDL input suite pass. Updated assertions cover Ctrl
press, sustained hold, key repeat and release from a latched crouch, plus the
same sequence with crouch rebound to Mouse4. Existing C toggle, Space cancellation,
fresh jump, pause/load/death/context clearing and binding tests remain passing.
The native controls suite passes, including low-headroom standing refusals and
D08K12 movement/ledge regressions. That suite models collision contracts.

Private real-SDL/Xvfb gameplay at 60 presentation verifies C latch, Ctrl takeover
held across repeated samples, crouch running during the hold, standing on release,
fresh Ctrl crouch, pause/load and Space cancellation followed by a fresh jump.
A second 120 presentation run passes the same sequence with momentary crouch
rebound to Z, toggle to G and run to T. The runs use copied lab saves/cards and private settings; player data is untouched.
Local scripts/logs: `recomp/analysis/d21a/`.

User accepted the revised handoff. No natural low-ceiling
route was newly played; existing stand-up clearance remains authoritative.
Closeout, implementation/tests/documentation commit and push are authorized
as MusicMonsterMod: "perfect! lock it in, close it out". D08K12 and D21A are
Done; no next job started.

```sh
python3 recomp/tools/local/run.py
```
