# Weapon response and furniture investigation — 2026-09-27

D08D is Done: the user confirms natural lights-first access without conversation.
Automatic ladder-exit redraw is also human-confirmed; that does not close D08B.
Historical delivery below; D08E was subsequently accepted by the user.
Current terrain work is in [the furniture follow-up](37-furniture-traversal.md).
Build at this checkpoint:
`679b0410c3099f49d2202d802a9b5991b407c98bed80587c316dc89b62071306`.

## Delivered weapon behavior

In supported Modernized settled ground states, the bound fire action requests
original Circle to draw a holstered weapon. Cross is suppressed for that request
so it cannot trigger an unarmed interaction. Held fire then reaches the original
weapon handler; releasing before drawing finishes leaves the weapon drawn without
a delayed shot. C and E take priority; selected Boot zero retains its original
attack. Menus, ladders, interrupted equipment and foreign code cannot authorize draw.

Original upper-body equip/stow tracks 6/7/13/21/22/30/31/36/40 receive 1.5 times the
normal animation delta. This is applied once per authenticated `8005a210` update,
only at upper-track `80059db0` caller `8005a5a8`, a0=player+74, a1=player,
a2=entry stack-2e, a3=1. Remaining-delta callbacks cannot multiply repeatedly.
The original event runner and equipment changes remain in charge. Lower-body
movement, firing/recoil tracks and Vanilla receive no speed increase.

57 Python tests including owned-disc integration and all four native suites pass.
Input tests cover fire release, focus loss, attached exclusion and rebound G fire;
controls tests cover Boot/interaction exclusion, selected track deltas, duplicate
callbacks, wrong caller and unchanged firing track. The isolated live pistol replay
confirms C holster, held click draw/fire (200 to 192 ammo), then short-click draw
with unchanged 192 ammo after release. Sampled upper6 duration decreases from about
16 to about8 host frames; sampling cadence differs, so this is not an exact latency
benchmark. Native tests establish the exact 1.5 delta multiplier.
The final binary passes a 14-checkpoint Vanilla replay; firing, holster, draw,
jump and inventory captures were reviewed. Broader weapon/era transitions and
subjective speed remain human playtest work, not certified by the pistol route.

## Furniture findings — reproduced, no terrain fix delivered

The user reports close-contact jumps into the bed, inability to walk off the bed
or couch, and couch entry varying by approach. The isolated replay reproduces:

- At bed contact (8769,-11763,8597), W+jump enters preparation96 then vertical97.
  Forward directional98 is refused by original clearance before takeoff. This
  corrects the initial hypothesis that a later impact merely kills velocity.
- Stepping back to Z8732 allows directional98 and a landing on the bed at
  (8769,-12151,7489), using ordinary inputs and original collision.
- Holding normal walk stops on the bed at Z7985, Y-12150. Holding Shift to run
  subsequently leaves the bed without jumping and reaches the floor, Y-11788.

Static dispatch matches that observation: walk `80053404` uses `8007926c`; table
`800158f0` maps result6 (drop over256 units) to the stop branch `800534a8`.
Run `80053500` table `80015910` maps6 to `80053584`, calling original drop check
`800789a8` and fall handling. The independent camera adapter currently preserves
these original choices. Do not globally bypass clearance or convert a wall result
to a floor result. A follow-up needs a guarded original collision/fall transition
for walking and a separately verified ascending horizontal-motion policy after
initial forward clearance fails. Test bed/couch approaches, ordinary ledges,
walls, ceilings, release/capture, falls, E reach and Vanilla.
Couch behavior remains user evidence; it was not independently replayed here.

## Evidence limits and private fixtures

See [structured evidence](reports/d08e-response.json). Tests use private settings,
cards and X displays, dummy audio, logged player-health longevity fixtures, and
no player position/velocity/inventory grants or saves. Two enemy-health zero
writes were attempted to isolate a ladder obstruction. They did **not** remove
the enemies: flags remained1 and actors remained visible. Later ascent/redraw
succeeded, but this is not controlled proof of the prior stall cause. The earlier
commentary hypothesis was corrected. Keep the obstruction case unresolved.
Early navigation helpers overlapped requests and missed waypoints; exclude those
as route evidence. The later named weapon and bed checkpoints were checked against
state, screenshots and inputs. A harness invocation initially passed an extra
argument to the scene test; the correct single-RAM-fixture invocation passed.
A rebind test initially used a nonexistent environment key; the actual serialized
binding payload passes. Neither test-harness error was a product failure.

Recorded player settings/card/state hashes are unchanged; all sessions exited.
Launch: `python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.
Modernized captures automatically; F10 toggles capture.
