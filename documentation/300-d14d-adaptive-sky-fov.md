# D14D - Sky backdrop coverage across FOV

2026-10-28. Done: user confirms "all working" and explicitly authorizes
lock-in and commit/push closeout.

## Finding and correction

User reports side artifacts at FOV 80/90 in current UI slots 2, 5 and 6.
Private current saves identify the medieval canal, Western outdoor street and
classical courtyard respectively. Slots are mutable; older mirror-slot notes
do not identify the current slot 5. Evidence is local under
`recomp/analysis/d14d/`.

Live matched slot-5 and slot-6 captures at 90 reproduce hard diagonal changes
in sky color toward both edges. D17R expands the camera-facing colored backdrop
for aspect ratio only. Its local X scale is `(256 + margin) / 256`, although
D14A reduces actual GTE H to 191 at 80 and 160 at 90. Clouds are full circular
bands and continue beyond that partial backdrop, exposing the fill behind it.
This is a coverage defect, not demonstrated texture corruption.

`sky_widescreen.cpp` now uses `(256 + margin) / min(H, 256)` for the authenticated
primary sky backdrop in Modernized, using actual GTE H from the rendering CPU.
That includes each replay frame's own projection, with no shared setting or new
replay state. H below 128 or above the correction range keeps the established
aspect correction. The existing saturating signed fixed-point writes remain.
4:3 is a valid adaptive path; invalid identity, split-screen or the developer
disable switch still prevent correction.

Only the backdrop's local X row changes. Its elevation, vertical gradient,
translation, horizon/cloud geometry, sky timer, textures, screen-space fill and
clip rectangle retain their previous behavior. H >= 256 preserves the accepted
geometry exactly, including default/60 and third person. Vanilla receives no
new FOV correction. No new assets, framework edits or generated C edits.
The accepted 60-90 player range is unchanged; this is not unlimited-FOV support.

## Verification

- Player build and owned-data `ttk-sky-test` / `ttk-near-test` pass. Sky tests
  cover H 160/191/256/277/386 at margins 0/85/192, unchanged non-X matrix fields
  and fill/clip coordinates, malformed H, Vanilla, unrelated callers/cameras,
  identity failure and split-screen, including the new 4:3 FOV path.
- Matched 16:9 slot-5 and slot-6 screenshots show the reported side seams
  removed at 90. Current slot 2 was loaded and inspected; its starting canal
  walls obscure most of the sky, so it is not an equally clear matched repro.
- Separate successful sweeps: slot 2 at 21:9/60, slot 5 at 4:3/60, slot 6 at
  16:9/120 setting. Each has default/60/80/90 samples (actual H
  256/277/191/160), followed by 12 yaw/pitch samples at 90. Native and presented
  captures are retained. Inspected views have continuous backdrop coverage.
- All three sweeps report zero mesh fallbacks and packet skips. Peak host packet
  usage: slot 2 71,032 bytes, slot 5 24,776, slot 6 29,024.
- Slot-6 replay: 609 surface-restoration checks, zero mismatches, zero failed
  worker jobs and zero worker deaths. This is correctness evidence, not sustained
  120 FPS: debug readbacks and offscreen diagnostics substantially slow the run.
- Separate third-person slot-6 capture retains H=386 and zero first-person blend.
  A synthetic view-toggle attempt stayed at H=160/blend=1, so it does not
  establish live view-transition coverage. This remains a limit of the automated evidence.
- Player saves/cards/settings retain their pre-run hashes and modification times.
  All writable test state is private. Native tests retain Vanilla exclusions;
  this job does not repeat a Vanilla campaign or exhaustive aspect/level coverage.

An initial baseline multi-load run, before code changes, halted with an unknown
GP0 command after reloading current slot 5. Subsequent captures from that halted
process are invalid. Separate-process baseline and candidate scene runs succeeded.
The cause of that load fault was not established and is not claimed fixed here.
Two overlapping diagnostic sweep attempts were discarded and rerun sequentially;
only the `verified2`, `verified5`, `verified6` sweeps support the results above.

## Acceptance and coverage limits

User accepts the result on 2026-10-28: "lock it in close it out! all working".
D14D is Done, with implementation, tests and documentation closeout authorized.
The bounded sweeps are not whole-campaign or every-angle proof.
Existing original sky art and intentional dark overhead remain unchanged.
No menu slider or new player setting is added; the manual's FOV commands stand.

Accepted executable SHA-256:
`2265f407ea7ee2b94fc2fdbfb06a854595c24f8edd28e286498576e2131c3cef`.

Launch from the workspace root: `python3 recomp/tools/local/run.py`.
