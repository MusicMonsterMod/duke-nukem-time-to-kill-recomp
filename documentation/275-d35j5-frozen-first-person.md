# D35J5 - Frozen first-person blue vision

2026-10-26. Status: Done - user accepted and explicitly authorized closeout.
User: "awesome!!! lock it in and close us out".

User selected D35J5, supplied `research/screencaps/freezethrow.mp4`, and prepared
UI slot 11. Keep its aim unchanged and shoot. Tests use private copies of saves,
cards and settings under `recomp/analysis/d35j5/`; original saves are not written.
D36 presentation ownership and evidence rules remain applicable (note 257).

## Evidence and implementation

- U: User requested first-person frozen blue vision with the EDuke video as reference.
- L: Reflected Freezethrower shots from slot 11 set actor flags `0x22000`.
  Earlier eligibility rejected `0x20000`, causing the third-person cut.
- S: Native damage code at `0x800a43c4` sets freeze and timer +0x214 to 3000.
  Native update at `0x8004157c` drains the timer and clears freeze/ice flags.
  Added an executable guard for that update slice; existing damage guard remains.
- I: Presentation alone may retain the selected first-person eye while alive,
  frozen and timed. Script/owner/animation guards remain. Movement and fire do
  not gain this exception. Mouse/recenter/key look and late view stay locked;
  quick kick explicitly refuses frozen/dead actors. Native damage, freeze timer
  and shatter code are not modified.
- I: A half-blended blue pair of quads covers the viewport after world, weapon
  and mask, before HUD. It uses the existing presentation arena, restores texture
  page/blend state, and supports widescreen without exceeding primitive width.
  Tint follows live native state rather than a persistent host latch.
- L/I: At 60 Hz the isolated OpenGL path lost health/armor icons during freeze,
  including with tint disabled. The existing mixed ordered path preserves them.
  Launcher now selects that path at all Modernized rates (previously only rates
  other than 30/60). Vanilla and explicit environment overrides are unchanged.
  This is a workaround, not a demonstrated root-cause fix of the isolated path.
  It expands renderer-default scope beyond frozen scenes; broader scene testing
  remains a regression consideration. No framework source was edited for this job.

## Verification

Final player build and controls/aim/near native suites pass; 36 profile tests pass.
Controls cover freeze presentation vs gameplay, dead/zero-timer/script/owner
rejections, view choice, inactive input, render state save/load and Vanilla gate.
Packet tests cover blend modes, page restoration, HUD order and aspect margins
0/85/256/427. These are automated contracts, not full gameplay acceptance.

L: `lifecycle60b` and `lifecycle120` pass private slot-11 reflected-shot tests:
frozen eye retained, position/yaw stable while W/Space/Q/mouse are held, no active
kick, third/first toggles, loading a frozen save, native thaw and loading a normal
save. Screenshots show blue world/arms with readable health and armor, clear
colour on thaw, and mask mesh still visible. Goggles and mask activation bits
were observed with the corresponding item inputs. At 120 Hz the worker rendered
1626 jobs, showed 1587 frames and reported zero failures in the sampled sequence.
Local logs, RAM samples and PNGs are under `recomp/analysis/d35j5/`.

Limits: Natural frozen death/shattering/continue, underwater/gas freeze and broad
campaign scenes have not been live-tested. Death eligibility is tested by native
fixtures only. User accepted the presentation after the candidate handoff; this does not
establish exact visual parity with EDuke. Some right-hand item/ammo icons were absent in sampled normal/thaw
frames; this has not been established as a freeze regression or fully explained.
Do not claim complete HUD regression coverage from the passing frozen captures.
Vanilla is covered by native/profile guards, not a new full live playthrough.

Accepted executable SHA-256: `2f19bae74d85d29e5b6074ada0c3f886862a11aaed7a4f1ed10bc8b06fc6af3f`.

Launch `python3 recomp/tools/local/run.py`, select first person if necessary,
load UI slot 11 and simply shoot. Implementation, tests, manual and documentation included in authorized closeout.
No new build, game launch, framework edits or next job during closeout.
