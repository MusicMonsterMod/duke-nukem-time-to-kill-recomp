# D07B / D08 — close combat and temporary interaction holstering

> Superseded behavior note (2026-09-27): D07C/D08H implementation and current
> coverage are in [feedback implementation](40-feedback-implementation.md).
> Earlier held-aim exclusions, weapon-family limits, and apartment non-reproduction
> below are historical evidence, not the current implementation.


The user reports good movement feel, an apparently ineffective point-blank pistol
encounter in the first-map apartment, and a weapon left holstered after climbing.
E remains strictly interaction-only; C remains deliberate draw/holster.

## Interaction ownership

An E-triggered original holster now acquires a temporary restoration intent. Once
the unarmed action completes, E is released, and authenticated normal locomotion
has settled, the adapter requests one short original Circle draw pulse. It never
writes equipment, inventory, animation or player position to force a draw.
The existing original draw animation remains; this removes the manual holstering
chore rather than removing animation/state requirements from the original engine.

Mounting, attached climbing, stepping off, airborne/impact recovery and an active
upper-body action cannot grant redraw permission. Normal ownership requires a
recent authenticated camera lease, equipment 0, no equipment-transition flag,
and the upper animation matching the normal lower animation. Six stable input
frames separate the completed action from the draw request. Holding E continues
the action and delays redraw until release.

Focus and original pause clear held inputs but preserve restoration intent.
Manual C/custom holster, numbered weapon selection, death or lost authenticated
player identity cancels it. A weapon already holstered before E stays holstered.
The intent is acquired only for automatic holstering; Vanilla stays unchanged.
Every interaction Cross pulse still requires holstered ownership, including when
E is held while drawing or returning from traversal. E cannot fire an armed gun.

## Close-range aiming

The saved user profile had assistance **off**. This pass does not enable it,
change enemy health or damage, or alter projectile speed/spread/fire rate.

The previous adapter could aim backwards in a reproducible overlap case: the
screen ray hits an enemy's front surface, while the offset muzzle enters farther
inside its collision volume. Retracting only eight units from that second entry
can leave the muzzle beyond the chosen target. A sphere fixture produces a
negative forward direction (-255 in Q12) with the previous production source;
the corrected adapter produces +4096 and starts on Duke's side of the overlap.
This is a geometric regression test, separate from actual retail geometry proof.

The view ray now starts on the same screen ray at Duke's depth. Obstacles between
the follow camera and Duke therefore cannot choose a target behind the character.
A body-to-muzzle actor intersection retracts the origin to the body-side endpoint
at muzzle height. A target less than 64 units forward of an offset muzzle also
retracts that origin, with a small forward convergence floor for degenerate contact.
World obstruction retains the existing body-to-muzzle check and safe-side clamp.
The original projectile sweep still decides all actual world/actor impacts and
damage; camera visibility never permits a projectile to cross physical cover.
No new guest callback, state whitelist or generated-C edit was introduced.

## Gameplay evidence

[Delivery report](reports/d08-close-combat.json) records the exact binary, private
cards, logged player-health longevity fixtures, native/Python results, traces and
screenshots. The exploration harness uses original SDL movement/climb inputs.
A research helper reads enemy coordinates to steer mouse aim, then holds the
original firing input; it is automated evidence, not a claim about human aiming
feel. Enemy health, traversal, equipment and player position are not fixture-written.

The final route reaches the first ladder armed, holds W/E to mount, releases E,
uses W to ascend, completes original mount/climb/exit states, and automatically
redraws on the platform. Original pause between operations preserves restoration
intent. Ammo remains 200 and the modern shot counter remains zero throughout.
Duke then enters the apartment armed without a C press.

Both apartment pig cops are killed by original player projectile contacts with
assistance off. Actor `0x801d0ac4` falls from 3750 through 2813, 1876, 939, 2 to
0 health, with five original projectile-to-actor contacts. Those health changes
occur at approximately 727–732 raw units of separation; the new close retraction
counter activates during the second hit. Actor `0x801d0d90` also reaches zero
with five recorded contacts, at approximately 2033–2035 units during the damaging
portion. Its initial close approach/misses are not claimed as hits. Screenshots
show both original enemy deaths and a living Duke in the apartment.

In the cleared room, armed E temporarily holsters then redraws with unchanged
ammo/shot counts. Manually holstering with C, then using E, leaves the gun holstered;
C draws it again. Native checks additionally cover held E, traversal ownership,
manual cancellation, focus/pause, death, upper-body transition guards, and original
fallbacks. The existing parallax, near-wall, assistance visibility and identity
checks remain passing. The same delivery binary passes the bounded Vanilla
boot/fire/draw/holster/jump/inventory/movement route (14 checkpoints); the final
turn image was reviewed with Duke alive and original presentation.

Earlier exploratory runs are retained: the baseline was interrupted, and a later
candidate died on the ladder. Neither is complete ladder-exit acceptance. Street
research confirmed normal enemy damage and a lamppost correctly blocking bullets.
Only the final route establishes the apartment and automatic ladder redraw results.

D07B/D08 remain Needs playtest for feel and broader weapon/era, progression,
scripted-camera and campaign coverage. The adapter edge case is proven; it does
not establish the exact cause of every miss in the user's earlier playthrough.
The earlier audio report still lacks new listening acceptance.
