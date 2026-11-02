# D08Q10 - Start the jetpack in water; the surface is a floor while flying

2026-11-02. Done, user-accepted (revision 10). User: "that's the one!! you absolutely got it. the feature is perfect now" and "i accept this job at hand" (2026-11-02), with closeout authorized.
The revision history below is the implementation record; earlier revisions
are superseded where later ones say so. Modern and EDuke32 jetpack schemes, Modernized only.

## Revision 10 - Fuel-out splash while moving (revision 9 incomplete)

User (2026-11-02): "i tried to replicate this and let the jetpack run out of
fuel while submerged, and i still heard the splash sound." A traced player
session (`DNTTK_SOUND_TRACE=1 DNTTK_JET_TRACE=1`, Modern, first person; local
copy `recomp/analysis/d08q10/player-session.log`) showed, while flying forward
(W) under water as fuel reached 0, within one update: the handler's cut-out
(pack shut-off `0x1c007`, pose 108), then the water-entry splash `0x3000`
from `800562bc`, and only afterwards revision 9's conversion.

Correction: the pre-state step `80055e80` (with its water entry) runs after
the mode-10 handler within the same player update, so revision 9's next-update
conversion was one step late. It went unnoticed because the revision 9 test
hovered: with no downward speed the native entry takes its silent branch.
The conversion now runs at that pre-state entry (`ra 0x8005a4f8`) straight
after this update's flight, before the water entry; the next-update check
stays as a fallback. Only the original pack shut-off sound plays (as on land).
`DNTTK_SOUND_TRACE` remains a diagnostic: every `8006bbd8`/`8006b270` request
with its caller and Duke's mode, pose and height.

Live, slot 12 copies, 180, first person, fuel nearly empty under water while
holding W, and W + Space: Modern converted at the pre-state entry; EDuke32
already swims from its own close; no `0x3000` in any of the four runs. Native:
the EDuke32 suite covers a native cut-out converted at the pre-state entry.
Modern full suite passes. Executable `3afecaf4bc648e19563c25c8af30f859a68e2ab2e0474fe6af09e9f75e7bd6f4`.

## Revision 9 - No splash when fuel runs out under water (incomplete, see revision 10)

User (2026-11-02): "when the jetpack runs out of fuel while underwater, it
plays a splash sound. as if duke just jumped into the water."

Cause: fuel-out did not use revision 7's under-water J-off path. EDuke32's
fuel-out calls the plain host close and Modern's is the native flight
handler's cut-out; both left the 108 fall, and the next update's native water
entry (mode 9 falling into water, `80056298`) took it as a jump into the water:
splash sound `0x3000`, splash rings and the entering-water actor flag
`0x04000000`.

Every switch-off with the root under the water level now goes straight to
native underwater swimming (state 5, pose 127, `+0x23e = -1`, as the native
entry sets them, without its splash): `jet_eduke_off` handles J and EDuke32
fuel-out, and `jetpack_update` converts a native cut-out (Modern fuel-out, a
blast) on the next update, before the pre-state water entry runs. Above the
water every switch-off still falls in with the native splash.

Live, slot 12 copies, 180, god mode and free fuel off on the copy, fuel set
nearly empty under water: Modern went flight -> one 108 cut-out frame ->
state 5, and EDuke32 flight -> state 5 directly; the entering-water flag was
never set. J off under water and at the bottom swims in place; J off above the
water still falls in (108 -> state 5). Native: the EDuke32 suite runs fuel-out
under water to state 5 / pose 127 with the pack closed. Modern full suite
passes. Two scripted runs dropped a short J tap (the test feed); the
deferred start itself was traced and accepted. Executable `7087ff4d49370e0ec2ca4f23e0aaaa325d739baaaa0d67e6acba216465291e8d`.

## Revision 8 - Surfacing sound

User (2026-11-02): "when duke surfaces from the water with the jetpack, it
should make the water surface sound just like when duke goes above water when
swimming".

Native surfacing (pre-state step, `80056200`..`8005623c`): when a swimming
Duke's head point (0x40 above the head) clears the water it plays the surface
splash `0x3003` and the breath `0x200e` through `8006bbd8`, and stops the
underwater ambience loop whose handle the underwater state keeps at `+0x268`
(`8006af44`). Flight never ran it, so it was silent and the ambience loop
also kept playing above the water.

`jet_surfacing` (in the post-handler bubble step) makes exactly those calls
once when host flight's head point crosses up through the surface. The state
resets at every pack start/stop, so a surface start (head already out) is
silent. `original_call` gained an optional a0 for these sound calls (Duke
remains the default).

Live, slot 12 copies, Modern 180, third and first person: rising out of the
water fired once (head 8146, water 8192) and both sounds started (non-zero
handles); the ambience handle went from `0x0d0e8ba4` to 0; descending onto
the surface floor did not repeat it; a surface start produced no event.
Native: the EDuke32 suite asserts splash, breath and loop stop exactly once
when the head clears the water. Modern full suite passes. Executable `33c58677d80610c453452db73ed56bbdedd322ba712c83ead091c6eff1a165d6`.

## Revision 7 - Surface start lifts like the ground; J off under water

User (2026-11-02): "when duke is on the surface of the water, and starts the
jetpack, it should lift him up no different to being on the ground."

- Surface swimming (state 4, also wading) now takes off with the ground
  start's accelerating lift, to the ground hover height above the surface
  floor (`water - 656 - 640`). Surface swimming sits about 1500 units below
  that, so the lift's distance and duration scale together (same curve and
  pace). Space/Ctrl take over at once, as on the ground. An underwater start
  (state 5) still holds in place.
- Found while testing: in Modern, J off under water was refused (the old
  native-path guard "jetpack cannot be switched in water"); that guard now
  only stops switching the pack on. Modern and EDuke32 switch off under water
  through the host toggle straight to native underwater swimming (state 5,
  pose 127, as the native entry sets it, without its splash). A 108 fall from
  rest there (for example at the pool bottom) chose surface swimming at depth,
  which the surface handler then pulled up to the surface. J off above the
  water still falls in and swims as before.

Live, slot 12 copies, 180: surface start 8391 -> 6896 (third person) and
about 6950 (first person) in about half a second; Ctrl stops at 7536; J off
over the water falls in to state 5. Under water, Modern and EDuke32 toggled
off/on/off in place (state 5 <-> flight), including at the bottom, and swim
controls work after. Modern full and EDuke32 native suites pass. Executable
`af3457e7685751cfbca81a62d2df41a00991c0563e4b0eb21566b5d30fdedf9c`.

## Revision 6 - Free flight under water; the surface is one-way

User (2026-11-02): "you should still be able to move downwards when the
jetpack is on while underwater. this really mimics how it works in eduke. you
simply just cant enter the surface of the water with the jetpack on".

The surface floor (water - 656) now applies only from above: when the root is
at or above it, a descent stops there and holds. Under it, Ctrl descends to
the bottom as normal flight (native floor clearance), and Space climbs out;
once above, Duke cannot re-enter. A first live run showed that the D08Q8
wall/step correction in the horizontal pass could still lower the root 121
units through the surface after the vertical clamp, so a final check after
every correction clamps the root to the floor and re-validates (or drops) the
horizontal route.

Live, slot 12 copies: Modern third person 180, from the bottom and from the
surface start, Ctrl descended under water to 10629 (pool bottom, still in
flight), Space climbed out to about 5700, Ctrl stopped at 7536, WASD over the
water held 7536. Along the surface in W, A, S, D and turning in both
directions, the root never went below 7536 (32-unit steps up at walls only).
EDuke32 first person 120 matched. Modern full and EDuke32 native suites pass.
Executable `350a78092d464584ad6eff90b2c6dff7634cd042b8cf4b763b8281ac8ef02db7`.

## Revision 5 - Air runs out under water

User (2026-11-02): "the oxygen meter just charges back up when you turn the
jetpack on. So obviously the game thinks duke is breathing, but he's not".

Cause: air is player `+0x236` (12000 full). Duke's update (`80041fd0`..
`800420b8`) spends it only in swim state 5 (`-= dt`, about 300 per second;
at 0 the native drowning damage unless god mode `0x800c3cc6`) and in every
other state refills it at 8 x dt, which looks instant. Flight is mode 10.

Fix: the Duke-update frame that keeps the pack open now carries a second
scoped patch, `80041fd8` (`bne` on state 5 -> `nop`), applied only while host
flight has the head joint under the water level. Air, drowning and god mode
then run natively; above the water the native refill returns. Both patches are
armed at the `800412a4` entry, restored at every hook and reapplied only while
that frame is on the stack and their own condition still holds.

Live, slot 12 copies, Modern 120: swimming 11690 -> 11390 in 1 s; after J,
hovering under water 11280 -> 10080 over 4 s (the same 300 per second); with
air forced to 0 and god mode off, health fell about 460 per 0.7 s; Space out
of the water stopped the damage and refilled air (1520 -> 3880 in 1 s). With
the player's god mode on, no damage, as when swimming. Surface start, Ctrl
floor and J off unchanged. Native: the EDuke32 suite checks that both words
are patched at the update entry with head and root under water, restored by
the next hook, and that only the pack word is armed with the head out. Modern
full suite passes. Executable `bbfd657c9a325f3bd5cb17685abade60cb9e03326f13614e20613f43cfc342a0`.

## Revision 4 - Bubbles under water

User (2026-11-02): "mechanically its perfect, but it needs some polish ...
the bubbles that duke produces when he's swimming ... when the jetpack is on.
currently there are no bubbles".

Cause: the native breathing bubbles belong to swim state 5 (`80047084`..
`800471bc`): timer `+0x85c` counts down by the animation dt (about 280 per
second), then `80044fa4(actor, count, position)` releases 1-2 bubbles at the
head joint (layout `+0x34`) and the timer restarts at 300 + rand % 600. Mode
10 never runs it. The limb loop at `80046d60` is only the water-entry burst.

`jet_bubbles` (after the mode-10 handler, host flight only) keeps the native
cadence and spawner at the head joint while the head is more than 32 units
under the surface, and adds a pack trail between the head and the root:
about every 0.15-0.3 s while moving or climbing, 0.5-1 s while hovering.
The position is passed on the isolated `original_call` stack window and
restored. Host-side random numbers leave the guest RNG untouched. Nothing
spawns once the head and pack are above the water.

Live, slot 12 copies, Modern 180: third person shows a rising column from
Duke and the pack while hovering, moving and climbing; first person spawns
the same (13-16 bubbles per 3.5 s run; they rise from the eye and pack, as
native first-person swimming). Native: the EDuke32 suite asserts bubbles with
the head under water (breathing at the head, trail below it) and none above;
the guest-call stub checks the actor, count and position window. Modern full
suite passes. Executable `f61f3807f6be51a7e67f881982567317b50aadbcbbbe9f7f48c05e5d6c8f338a`.

## Revision 3 - Hold in place (EDuke32 behavior)

User (2026-11-02), after revision 2: Duke was still carried completely out of
the water, even from the bottom; "the true behaviour in eduke32 is that duke
doesnt even move". The EDuke32 reference
(`research/screencaps/Video_2026-11-02_11-39-22.mp4`) shows JETPACK ON
underwater with the view unchanged for several seconds, then Duke flying
above the water and descending onto the surface, where he hovers.

The automatic rise is removed. J in or under water opens the pack and holds
Duke exactly at the start height (as a midair start does). Space climbs out;
WASD flies; under the surface floor Ctrl cannot take him deeper; once above
it the surface stays a floor. The two native vetoes stay armed while the root
is under the water level, so the hover underwater keeps the pack open.

Live, slot 12 copies, Modern first person 180: J at the bottom (y 10213) and
on the surface (y 8389) held the exact height for 3 s, through Ctrl and W;
W moves him under water; Space climbs out; Ctrl then stops on the surface
floor (7536); J off swims. Modern full and EDuke32 native suites pass (the
D08Q10 cases now assert no vertical velocity until Space). Executable
`d2bf41d61db7de9cba35a09cbc5c7166d382708dea643402593c9a06a1cdd151`. Revision 2 and the first-pass evidence below are superseded where they
describe a rise.

## Revision 2 - No launch out of the water (superseded by revision 3)

User (2026-11-02): "great!! the only thing is that the jetpack shouldnt launch
duke out of the water. otherwise thats fine and a superb first pass". A water
start no longer adds the ground start's 640-unit lift: Duke comes up to the
surface floor (water - 656) and hovers there, feet just above the water.
Space climbs from there as usual. Live, slot 12 copies, Modern first person
180: surface start 8391 -> 7544 and underwater start (dived to about 10270)
-> 7544, both settling within about 0.7 s; Ctrl holds at the floor; Space
climbs. Modern full and EDuke32 native suites pass. Executable
`04aadaae7d07ed1594993101d6b745c086e3469c7c56d585791627f3735d6974`. The evidence below is the first pass.

## Requested behavior

EDuke32 / Duke 3D feel (reference `research/screencaps/water-jetpack.mp4`):
J while swimming on the surface or underwater starts the jetpack and Duke flies
up and out. With the pack on he cannot go below the water surface; Ctrl stops
there and he hovers. J off over water drops him back in to swim as before.

## Original-code findings (owned SLUS-00583, read-only)

Three separate native rules kept the pack out of the water:

- `800402d0` (pack open) refuses swim states 4/5 and a root at or below the
  water level (`player+8 >= player+0x834`).
- `80055e80` (pre-state step, reached from the player update at
  `ra 0x8005a4f8`) recomputes the water level and depth every update
  (`8005b85c`), then forces surface swimming when the head/root are in the
  water band, in any mode except 4/5/8 (`80056240` is that branch; mode 9
  with downward velocity takes the splash/underwater path instead).
- Duke's update `800412a4` closes an open pack whose root is under the water
  level (`800425ec`..`80042614` -> `8003ff6c(player,1)`), after its splash
  and water-mask block (`800422a4`..`80042474`).

The mode-10 handler itself never reads the water level; its root apply
`8004ac08` only reaches `80054c04` (landing into water) on a floor hit.
J is read inside the native swim handler's entry (`800455bc`) while swimming;
that handler keeps running its state on the same update, so a start made there
was undone (anim 122, pack closed).

## Implementation

Host code only (`recomp/src/ttk/`); no generated C, framework, hook list or
codegen change, so existing savestates load.

- `shortcuts.inc`: Modern also routes J/item-use to the instant toggle in
  swim states 4/5 (EDuke32 already routed every state).
- `jetpack.inc`, `jet_instant_toggle`: a swim-state J becomes a request,
  executed at Duke's next update entry (`800412a4`, `jet_water_start`) before
  the pre-state step and any handler; a new capture/load epoch drops it.
  The start presents `800402d0` an airborne state with no water for that one
  call, restores the water level, clears the swim pitch (`+0x26`), the
  underwater flag bit `0x100` and the water mask, then uses the accepted D08S
  native pack/model endpoints and flight pose. The old submerged refusal for
  ground/air starts is removed.
- Surface floor: `water level - 656`, the same clearance D08Q8 gives a floor
  contact (flight envelope feet at root + 624, held 32 above). In the D08Q8
  clearance step a descent that would cross it is clamped and held there
  (Modern/EDuke32; Classic is excluded). Below it (a water start, or later
  submersion) `jetpack_update` rises toward it with an eased, capped climb
  (`k_jet_water_gain`, `k_jet_climb`), through the same native collision; Ctrl
  cannot hold Duke under and Space climbs as usual. J from water rises to the
  floor and holds there (revision 2; the first pass added the 640-unit lift).
- `grounded_water.inc`: two scoped native vetoes in the existing
  grounded-water patch framework, only for host-owned Modern/EDuke32 flight
  (`jetpack_flying_early` and `jetpack_input_ready`):
  - kind 5 reuses the D08AE `80056240` branch patch inside the authenticated
    `80055e80` frame, at any depth, so mode 10 is never turned into swimming.
  - `8004260c` (pack close under water) is armed at the `800412a4` entry only
    while the root is under the water level (rising out), so the runtime
    selects its guarded interpreter; every hook restores it, and it is
    reapplied only while that frame (0x58, ra at -4) is on the stack. The model
    build `800411b8` (`ra 0x80042634`), called right after the test, ends it.
    Identity checks see the original word through the existing substitution
    table. Fuel, damage and every other close remain native.

Splash rings and the water mask still run natively as Duke leaves the water
(the mask goes from `0x7ffff` to `0` during the rise).

## Evidence

Private copies only: the player's UI slot 12 savestate copied into
`recomp/analysis/d08q10/cards` (hash verified identical before and after),
private profile, blank private cards, offscreen OpenGL, port 9461.

- Before: J on the surface was refused; Ctrl dives (state 5).
- Surface start, Modern, first person, 120: J -> mode 10 / anim 163, pack on,
  root 8391 -> hover 6904 (water 8192, floor 7536, +640 lift) in about 0.8 s;
  Ctrl descends and stops at 7536 (feet about 70 above the water); W flies
  over the water at 7536; J off falls in and swims (state 5), as before.
- Underwater start (dived to 10268): Modern first person 120 and EDuke32 third
  person 180 both rise through the surface to 6904, Ctrl stops at 7536,
  Space climbs, J off swims. Screenshots show the normal first-person weapon
  presentation and the third-person pack and flame.
- Modern first person 180: 178 presents/s and 60 guest fields/s during the
  rise, the same as idle (the interpreted Duke update lasts only the rise).
- Classic, slot 12: J on the surface and underwater still refused.
- Native: `ttk-controls-test` full suite (LEVEL00 fixture, LEVEL01, all
  levels) passes with Modern; the EDuke32 suite passes with the D08S "water
  veto" case replaced by D08Q10 cases (submerged fall start rising without
  Space and with Ctrl held; surface and underwater swim starts deferred to the
  update entry, epoch-dropped, pitch/flag cleared).

Executable `04c94ce34a2bbef95a3923017bbeab51677ee5c145be1cdc8e9b9b17db45809a`.

## Limitations

- Feel (rise speed, hover height over water) needs the user's playtest
  against the EDuke32 reference. Presentation smoothness at the surface was
  judged from guest-update motion and the frame rate, not by eye.
- Not live-tested: J while wading in shallow water or while falling into
  water (native fixture only), flying sideways into a higher water volume
  (the rise rule covers it), other levels' water. Vanilla and Classic are
  unchanged by code gates.

## Playtest

```sh
python3 recomp/tools/local/run.py
```

Load slot 12 (Duke on the water surface). Press J on the surface; dive with
Ctrl and press J underwater; with the pack on, hold Ctrl over the water, fly
around with WASD, then J off over the water. Try first and third person (P)
and the EDuke32 scheme (`--jetpack eduke32`, saved; `--jetpack modern`
restores it).
