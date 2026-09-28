# Next iteration — user feedback and resume brief

**Historical selection brief — implemented in the requested order.** D08G1 and
D19A are Done; D07C and D08H are implemented / Needs playtest. Current evidence,
reproductions, limits and launch guidance are in
[the implementation notes](40-feedback-implementation.md). The original feedback
below is preserved verbatim as the acceptance reference.

2026-09-27. **Planning-only checkpoint.** The user asked to record the jobs and
supply a continuation prompt before clearing the chat. No code, build, gameplay
session or player preferences were changed in this turn. Canonical job definitions
and acceptance criteria are in [MODERNIZATION_JOBS.md](../MODERNIZATION_JOBS.md).

## Priority and user intent

1. **D08G1:** small confirmation-wording pass using the exact Duke3D strings.
2. **D07C:** highest-priority gameplay issue: consistent view aim, body/weapon
   presentation and pitched projectile trajectories, especially the RPG.
3. **D08H:** apartment contact jumps, fluid furniture drop/landing, premature
   pipe-bomb pickup and switch targeting.
4. **D19A:** supplied Duke fonts for cheat messages and shared modern UI text.

This is the proposed order encoded in the continuation prompt, not a claim that
implementation began. Do not start unrelated backlog jobs. Keep existing Vanilla
and explicit original options. Duke3D is the user's baseline for modern aiming
and interaction feel, not an instruction to replace every TTK weapon's physics or
turn the third-person build into a first-person-only game.

## Current build and acceptance

Current player binary SHA256:
`6efa111e9e838a00273aaef454ac3130563a49c1e03f9e84eb314d8e37e519b6`.
Earlier delivery evidence: [D08G report](reports/d08g-cheats.json). The user now says
“the cheats work fine”; D08G is Done for the delivered bounded feature set. That
acceptance does not verify hidden-enemy save/load, every boss/era or unsupported
codes. Show enemies before save/load; use private cards/settings for tests.
D08G1 tracks the requested label polish independently of core functionality.

D07's earlier bounded acceptance remains historical evidence. D07A/D07B are still
Needs playtest; the new report exposes specific gaps and must not be dismissed
because the pistol route previously passed. Likewise, a prior bed pickup sweep
that failed to reproduce the issue never established a pickup fix.

## Aiming report and first investigation

After `dnstuff` and unlimited-ammo testing, the user describes aiming/firing around
360 degrees while Duke keeps facing forwards, including shots behind his body.
They initially call the held action left-click, then identify right-click aim.
Record actual action bindings and test ordinary left-fire, right-aim, and combined
held fire/aim. With the RPG and no held aim, they report dead-horizontal rockets
despite attempted pitched aim. They question whether the white host crosshair has
any authority or the original red dot still controls the weapon.

**Source observations, not a proven root cause:**

- `recomp/src/ttk/weapon_aim.cpp` skips the modern shot hook while
  `input_snapshot(Context::Gameplay).held[original_aim]` is true.
- Its modern reticle also excludes original-aim-held states. The current
  `supported()` whitelist is weapon IDs 4–8 and 11. Map actual RPG/other weapon IDs
  and alternate shot callers before changing this list.
- `recomp/src/ttk/modern_controls.cpp`'s view-facing gate excludes original aim as
  well. Read `presentation_ready` and surrounding code before choosing integration.
- This may explain differing modes, but it does not prove the RPG failure or the
  exact 360-degree presentation problem. Reproduce and trace launch/impact paths.
- Existing diagnostics `ttk-launch`, `ttk-flight`, `ttk-impact`, aim/facing counters,
  original-camera/input hooks and isolated probe tools are reusable.

Read [D07 implementation](21-modern-weapon-aiming.md), [facing](22-view-facing.md),
[aim options](24-d07-controls-and-aim-options.md) and
[close-combat evidence](28-d08-close-combat.md). Avoid just drawing one reticle over
another: actual camera target, original acquisition, muzzle, projectile and model
must agree within physical cover, spread and weapon-specific arcs.

## Apartment report

See [furniture investigation](37-furniture-traversal.md). Still reported:
contact W+Space fails where a tiny step back works; walk/run departure and landing
retain a noticeable slowdown; switch E targeting is awkward. The newest report
again collects pipe bombs through the unopened bed from its end/pillow area.
Normal pickup after opening the bed worked in the previous test. Preserve the
accepted lights-before-NPC-conversation fix and ladder-exit weapon redraw.
D08H acceptance requires fresh-state/repeated routes and both concealed/exposed
pickup cases, not just one sweep or artificial inventory edits.

## Cheat wording

Use the exact D08G1 table on the board. The user's
[InfoSuite reference](https://infosuite.duke4.net/index.php?page=references_cheats)
was successfully read this turn. `dnstuff` and `dnitems` share a confirmation;
hiding enemies says `Monsters: Off`, restoring them says `Monsters: On`.
Keep added debug codes' sensible distinct messages and honest TTK effect notes.
This is wording work, not an instruction to implement all of the reference's
warp/noclip/unlock/map/debug/joke codes in this iteration.

## Font assets inspected without modifying originals

`research/fonts/link.txt` records `https://www.realm667.com/repository/font-press/general`.
Both PK3 files open as ZIP archives and contain FONTDEFS, INFO, CREDITS and individual
`.lmp` glyphs under `graphics/`. Metadata explicitly says **Doom palette** and no
lower-case glyphs. Validate palette and case mapping rather than guessing RGB or
assuming the archives are scalable font files.

| Local file | Contents / metadata |
| --- | --- |
| `research/fonts/DukeNukemSmallFont.pk3` | 67 archive entries; template `DNSMR%03d`; 7px SmallFont; original Duke3D message font |
| `research/fonts/DukeNukemAtomicFont.pk3` | 68 entries; template `DNATM%03d`; 15px BigFont; Duke3D v1.4/1.5 menu font |
| `research/fonts/MS-DOS - Duke Nukem 3D - Miscellaneous - Fonts.zip` | 577 entries; indexed PNG glyph sprites in message, HUD and menu sets |

PK3 credits identify submitter Jimmy and author 3D Realms. Preserve these when
converting/packaging; inspect archive provenance terms before distribution.
PNG set counts: Messages 94 plus one Atomic override; HUD 68 each in blue/red/green/
yellow variants plus 12 miscellaneous sprites; menu v1.3 normal/disabled 45 each;
Atomic normal/disabled 46 each. These are sprite glyphs, not a single finished atlas.
No further upload is needed for the next investigation.

`DUKE3D.GRP` is present at the workspace root. Read-only directory inspection found
456 entries, the `KenSilverman` signature, and:

| Entry | Bytes | Offset | ART tile range |
| --- | ---: | ---: | --- |
| `PALETTE.DAT` | 82690 | 10689862 | — |
| `TILES011.ART` | 57815 | 32268840 | 2816–3071 |
| `TILES012.ART` | 1060417 | 32326655 | 3072–3327 |

Local primary EDuke32 source `research/eduke32/source/duke3d/src/names.h` defines
`STARTALPHANUM=2822`, `BIGALPHANUM=2940`, `MINIFONT=3072`, within those ranges.
Thus the owned GRP is another concrete font-art candidate. Verify glyph rendering,
coverage and palette before selecting it; source constants alone do not render art.
The current host message path is `input_notice` → `host_osd_push`; review the runtime
OSD renderer, CMake source definition and preserved
`recomp/patches/time-to-kill-zzz-cheat-osd.patch`. Do not hand-edit generated C or
lose the runtime submodule's pre-existing modifications.

Archive SHA256:

- Small PK3: `29e75ac7e55aae05eb2512d866856806f55560a8f2ef4c314389022ac3d1a370`
- Atomic PK3: `0f2718f980ffc0baf69b74893c90f0a4f3f361844f6473bfaca76816d2084370`
- PNG ZIP: `d716e22b2e49ff7adcd6cd15ba2f5001326f75dd7c9248a4c5d1153fc118d429`

## Continuation prompt

```text
$continue-duke-recomp
Read MODERNIZATION_JOBS.md, documentation/00-status.md, documentation/09-handoff.md
and documentation/39-next-iteration-brief.md in /home/spartacus/Desktop/dn-ttk.
Implement D08G1, D07C, D08H and D19A in that order, autonomously to a high standard.
The cheats work; match the original Duke3D confirmation wording. Use Duke3D as the
Modernized aiming baseline: unify actual shot direction, crosshair and Duke/body/gun
facing in ordinary and held-aim modes, with pitched RPG shots and weapon-family
coverage. Fix the apartment contact jumps/drop fluidity, premature hidden pipe-bomb
pickup and awkward switch targeting. Use the supplied Duke font assets for modern
messages/UI, starting with cheat confirmations. The brief contains inspected asset
locations and source leads; do not treat hypotheses as proven causes.
Preserve Vanilla, accepted controls, original assets and my saves/settings. Check
for an active game before testing; use isolated test state. Read existing evidence,
reproduce the reports, implement and verify each bounded change, update the board,
manual and handoff, and report remaining limits honestly. No need to ask me to
choose these jobs again. Finish with what to test and the current launch command.
```

Current launch (unchanged):
`python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py`.
Modernized captures automatically; F10 toggles capture.
