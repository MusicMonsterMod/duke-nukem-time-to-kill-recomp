# D07 follow-up — controls, aiming options and timing

> Superseded behavior note (2026-09-27): D07C/D08H implementation and current
> coverage are in [feedback implementation](40-feedback-implementation.md).
> Earlier held-aim exclusions, weapon-family limits, and apartment non-reproduction
> below are historical evidence, not the current implementation.


This pass follows the user's D07A playtest: aiming feels much improved, with a
new report of slight audio choppiness. It retains the accepted D01–D07 milestones
and their coverage limits. D07A/D07B still require gameplay acceptance of the new
candidate; neither synthetic fixtures nor a successful build certify a campaign.

## Input and preferences

Schema 5 adds independent `aim_assist` (`off`, `original-lock`), `crosshair` and
`red_dot` preferences, plus `interact`. Migration backs up the exact prior JSON,
keeps renderer/camera/sensitivity/aim choice and customized bindings. Old default
Q/E sidesteps move to Comma/Period when those keys are free. Interaction defaults
to E; if a custom binding occupies E, migration chooses an available letter and
the binding list shows it. Vanilla retains its original input/aim/marker path.
Player profiles are migrated on the next launcher invocation, not by tests.

E emits the game's Cross action only in supported, holstered normal locomotion.
It never fires a drawn weapon. C remains the fixed original draw/holster key;
H remains its rebindable default alias. Required holstering for traversal stays
part of the game. This does not make arbitrary inventory items usable with E.

Number keys 1–9, 0 request base slots 4–13, respectively, only in supported normal
locomotion. An explicit custom action binding on a number takes precedence.
Requests are edge-triggered, expire after four input frames, and are discarded in unsupported states. Owned inventory,
weapon-table flags and ammunition (including mapped upgraded ammo slots) are
checked. Only the original selected slot (`+0x3BA`) and equipment-change request
(`+0x224 & 4`) change: original draw/holster/equipment logic owns the equipped
slot, model and animation. There are no inventory/ammo grants in the player path.
Acquisition, era variants and the remaining slots require further coverage.

## Speed and running jumps

The speed policy remains walk on launch, Shift reverses speed, Caps Lock toggles
autorun. The ordinary gait pairs 72/76 and 74/78 now change at the guarded original
locomotion dispatcher `0x80048410`, RA `0x8004B634`, instead of waiting for the
current gait cycle to finish. The selected gait restarts its original animation
counters, matching the original transition convention. Root-motion displacement,
vertical motion, collision routines and simulation clocks remain original.
Stop, jump, precision-aim and unsupported animations are excluded.

The baseline reproduced backward run -> forward running jump. Original running
jump state is **9/9**, animation **104**, not normal locomotion's 0/0. The first
attempt correctly failed closed on this mismatch; its failed replay is retained.
At original ballistic update `0x8003EBF4`, RA `0x80055934`, the adapter latches the
last supported movement direction within four host input frames and the same
capture epoch. It rotates only horizontal ballistic velocity `+0x1F4/+0x1FC`
once. Original gravity, swept collision and integration execute. Later impacts
retain their own velocities; the adapter supplies no air steering or reinjection.
Capture/focus changes invalidate ownership. Verified running jump/landing 104/105
may retain the normal camera only with that same jump lease and state 9/9.

Walking jump, standing jump, crouch, swimming, ledges, ladders, scripts and other
jump animations are not added to a broad whitelist. The walking jump input in
this route did not enter airborne animation; this is an explicit D08 observation.

## Aiming and marker separation

View aiming stays unassisted by default. `original-lock` is an experimental opt-in
for the already supported base shot paths (4–8 and 11). It considers only a live
original game target (`+0x288`, `+0x2B0`), within six degrees of the view and the
existing 16384-unit view-ray range. Both the camera-to-target and physical-muzzle-
to-target queries must identify that same actor. World cover, a different actor,
invalid/absent lock, dead target or an out-of-cone target keeps free view aiming.
Thin-cover muzzle checks still run first. Original spread, projectile integration
and damage handling stay in place. The original in-game autoaim option must be on
for original target acquisition. This is not a replacement acquisition system.

The crosshair shows view center; assistance can correct a shot within its stated
cone. The original red marker remains the game's indication, not a laser or a
claim that assistance was accepted. Hiding either marker leaves aiming unchanged.

For the red marker, guarded `0x80033AF8`, RA `0x8003543C`, establishes player/stack
ownership. At its specific OT enqueue `0x8002BC18`, RA `0x80033DB0`, hiding collapses
only the verified textured quad to zero area. No target field, texture, saved game
option or global rendering behavior is changed. Unknown code/overlays fail closed.
Original aiming remains available independently of both Modernized marker choices.

## Timing and audio

Repeated SHA-256 guards were replaced with authenticated reference-byte storage.
Every invocation still compares every live word, including the complete LEVEL00
identity range; no frame-cached permission survives a loader write. Reference
storage is thread-local. Mismatch immediately refuses the hook, and restored
bytes can recover. Same-frame corruption/restoration is covered by native fixtures.

The guard-only comparison measured idle/look at 59.97/60.02 guest fps, versus
59.91/58.53 in the previous binary's short samples. Both recorded zero added
output underrun samples with SDL's dummy audio device. The combined controls
candidate measured 59.91/59.98, also with zero added output underruns. These are
bounded timing observations, not listening evidence or proof that the user's
audio report is resolved. Host audio, voice transitions, FMV and longer play still
need listening. No audio-buffer size, playback rate or simulation speed was changed.

## Verification

See [the final evidence report](reports/d07-continuation.json) for binary identity, native/Python results, real SDL
routes, screenshots reviewed and failed experiments. Tests own private displays,
cards, profiles and processes, and refuse a pre-existing game. The player cards
and preferences and the pre-existing runtime patch diff are preserved.

## Actor-contact evidence added in this pass

A real-input pistol scan acquired original target `0x801D1328`. Three assisted
shots were followed by original actor-contact function `0x8006F444`, RA
`0x8006FD64`, with that actor as A0 and player-owned projectile slots as A1.
The projectile slots also appear in the read-only player-projectile flight log.
This closes a bounded pistol-to-this-actor contact observation; it does not
establish quantified damage, moving-target reliability, all weapon families or
the cause of D07's earlier actor-selected wall-hit discrepancy. A sampled actor
`+0x204` word was -9216 and is **not** promoted to a health field. Actor layouts
must be verified separately; a player-specific offset is not an enemy contract.

Visual QA rejected the first delivery-hash controls replay: its speed and airborne
backpedal checks passed, but enemy fire killed Duke by `run-land` (death flag,
animation 227). The report is retained and explicitly rejected as a complete
route. The corrected driver rejects any dead capture, shortens its idle timing
windows and removes the redundant walking-jump observation from acceptance runs.
Earlier walking-jump research remains recorded separately.

The shortened accepted delivery replay measured 59.68/59.59 guest fps in its
idle/look windows, with zero added dummy-device output underrun samples. All
recorded captures stayed alive, ordinary gait switched 72 -> 76 -> 72, and the
running sequence reached 104 -> 105 with backward displacement and continued
camera callbacks. Its final composed image shows Duke alive at health 40.

The final Software route with both markers hidden recorded two assisted shots and
two player-projectile contacts with the acquired actor. The OpenGL route with
crosshair visible/red dot hidden recorded three each. These are separate natural
encounters, not deterministic image or shot-count comparisons between renderers.

The final evidence report accepts seven bounded delivery-binary gameplay routes.
Original-aim samples keep modern shot/assist/facing/arm counters at zero. Vanilla
captures show original firing and turning. Its quit RPC returns `emu busy or
frozen` despite process exit 0, so graceful shutdown remains inconclusive. This
behavior was also present in the historical regression record.
