# D08U - Top-of-ladder mount

Status: **Done** (user-accepted 2026-10-01). Modernized only; Vanilla unchanged.

## What the original game does

Research on private copies of the user's slot-12 sewer state and the alley
dumpster state (`recomp/analysis/d08u-ladder-top`), with Vanilla pad routes and
disassembly of the owned executable:

- A plain ladder is an object whose type flags (`*(0x800d2660) + 28 * type`)
  include 0x200. Flags 0x400 (type 160/327, anim 191 family) and 0x100 (197
  family) are other climbable surfaces. The ladder's collision box (index 0 of
  `0x80073ae8`) is a flat panel: 378 wide, 2046 (alley) or 6142 (sewer) tall,
  zero thick. Object +0x1c is its yaw. Both faces are climbable.
- The slot-12 ladder is type 308 at (-4136, -2047, 76328), yaw 1024. Its top is
  at Y -5118, 507 below the walkway (-5625); its bottom reaches a floor at
  about Y 510. The first alley ladder (type 46, (9225, -10239, 13769)) has the
  same layout: top 497 below its platform.
- Mounting: `0x80051cf0` (Up + Cross into a touched object) starts 185 from
  below for 0x200 and 191 for 0x400 (191 lifts Duke onto a climbing wall; it is
  not a descent). The airborne reach (`0x80047480..0x8004765c`) catches a
  ladder by setting +0x17c = ladder, +0x180 = box, +0x1c4 = 0, mode 3 and
  transfer anim 156 (or 207). 156 ends in climb 186 (`0x8004d1fc`).
- There is **no top mount for a plain ladder**. At the slot-12 edge, Vanilla
  Up/Up+X/X/Down/Down+X, backing off the edge facing the walkway (84, 85, then
  fall 108) and forcing the airborne reach all end in the water. The only
  original way down is a jump.
- Once attached the original handler owns everything: 186/188/189 climb 200
  units off the panel facing it; 190 exits at the top onto the platform; Down
  alone stops at the lowest rung, and Down + Cross continues and steps off onto
  the floor (185 played as a dismount, mode 8 after 3). Climbing needs Duke
  holstered (armed, the handler does not move).
- A research fixture that wrote the airborne catch's fields (plus a position at
  the top of the climbing line) while holstered produced a complete original
  climb: down to the bottom, back up, exit 190 onto the walkway.

## Change (`ladder_top.inc`, `modern_controls.cpp`, `pc_input.cpp`)

- **E at a ladder top.** `ladder_top_find()` walks Duke's cell object list
  (`player+0x220`, the list `0x80077f28` uses) for a plain ladder whose panel
  top is 300..800 below Duke's feet, within 420 units of the panel and within its
  width + 120. Settled ground only (normal lease, mode 0/0, land gait, not in
  water, jetpack, grab or mantle). E there (after the usual E stow when armed)
  asks for the mount instead of pulsing Cross.
- **Mount.** At the player update (`0x8005a210`) the host writes the airborne
  catch's attachment (+0x17c, +0x180, +0x1c4, mode 3, anim 156) and, over the 12
  player updates of 156, blends Duke's position (smoothstep) from where he stood
  to the climbing line on the far face (200 off the panel, 320 below its top)
  and his heading to face the panel. Then the original 186 snaps him onto the
  line and owns climbing. Vanilla, death, context, identity or state loss stop
  the blend.
- **Descent.** S on a plain ladder also holds Cross, so Duke climbs all the way
  down and steps off instead of stopping at the lowest rung.
- **Exits.** The original top exit 190 and bottom step-off 185 (mode 8 after
  the climb, never the 185 mount from the ground) join the D08V camera-only
  traversal lease, with directions neutral while they play: no tank fallback,
  no `ORIGINAL MOVEMENT` banner, and a held S no longer starts an original
  backstep (82) as Duke lands.
- **Discoverability.** The first two ladder tops of a session show
  `E TO CLIMB DOWN` (the bound key name).
- New code identity guards: the 156 end transition, its dispatcher table entry
  and the box lookup `0x80073ae8`.

## Evidence (binary `6129f2ab76e533ec9670bf17eb1a9c4f08471fa2d0fa7d883fd82a11da945683`)

Private Xvfb runs, real keys and mouse, dummy audio, isolated profile/cards:

- Slot-12 sewer, third person (`mount.py third`): E (pistol drawn) stows, 63 ->
  156 (3/3) -> 186; blend ends exactly at (-4336, -4798, 76328), heading 1024;
  largest step between samples about 190 units over the ~800 unit descent. S:
  189/188 down, 185 step-off, 72 then 63 on the floor at Y 510, pistol redrawn
  (equipment 2), modern lease ready. No tank fallback line.
- Same in first person (`mount.py first`): blend to the orbit during the mount,
  same attach and descent.
- Side-view capture sequence (`side-*.png`): stow, turn to the ladder, lower
  onto it with the transfer's arms-up grip.
- Regressions (`regress.py`, `runoff.py`): walking into the edge stops as
  before; Shift-run off the edge falls into the water as before (no mount);
  strafing along the edge past the ladder never mounts; E after backing away
  from the edge is not a mount (`available` false).
- First alley ladder (`alley.py`, after a logged fixture placing Duke at its
  base): W + E climbs with the original 185/188/189, exits with 190 under the
  camera-only lease, pistol redrawn on the platform; E back at the top mounts
  (156 -> 186); S descends and steps off at the bottom, pistol redrawn.
- Vanilla edge routes (`vanilla_edge.py`) on the new binary: identical states
  and end positions to the baseline.
- Native: `ttk-controls-test` (new D08U fixture: reach/side/top/family/state
  gates, armed and Vanilla refusal, attach fields, blend endpoint, camera-only
  lease, 186 handoff, request expiry), `ttk-input-test` (E request after the
  stow, hint count, S adds Cross on a ladder, neutral directions in exits),
  `ttk-aim-test` PASS; Python 87 OK (2 skipped).
- The player's slot-12 file was copied, never loaded in place or written.

## Limits

- Only plain ladders (flag 0x200). Climbing walls (0x400) and the 0x100 family
  are untouched. Ladders whose top is not 300..800 below the floor next to them
  are not offered.
- The known ladders checked are the slot-12 sewer ladder and the first alley
  ladder. The second alley ladder (same type and layout) was not exercised.
- The mount motion is a host blend over the original transfer animation, not
  an original top-mount animation (the game has none for ladders). How natural
  it looks is for the playtest.
- On a ladder the camera stays the original ladder camera, as accepted in D08.
