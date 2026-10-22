# D12D - Upgraded first-person weapon framing

Status: Done (user-accepted 2026-10-22). User: "fully accepted!! lock it in,
and close us out". Implementation, tests and documentation commit/push
explicitly authorized.

## Cause and change

The original weapon resolver at `0x8003df40` maps upgraded records
9 -> 27 (HiTemp Flamethrower), 7 -> 28 (Laser Gatling), and
8 -> 29 (Incendiary RPG). Pistol, shotgun and energy upgrades retain
records 4, 5 and 10. Challenge Stage 4 directly equips record 29.
The baseline first-person offset, tilt, scale and firing-pose tables only
initialize the base records. Record 29 therefore falls through to zero
offset/tilt, scale 1 and an unseeded pose. Its launcher fills the centre
of the screen before the first shot; learning a pose does not supply the
missing framing.

`weapon_view_slot()` in `first_person.inc` now maps those three separate
records to their base presentation slots for every first-person consumer:
pose selection/recording, crouch-channel eligibility, offset, tilt and scale.
The existing base firing seeds apply from the first draw. Developer framing
overrides use the base slot too. There are no new offsets or pose guesses.

The held record itself is never changed. Original weapon selection, ammo,
upgrades, damage, projectiles and muzzle-flash calls still use their original
records. Base records and unknown/sentinel slots retain their old mapping.
The existing first-person draw gates keep third person and Vanilla outside
this code. No framework source, generated C, hook list or save format changes.

## Verification

Accepted executable:
`bb86f2da7265785ba692c3dadbbcc19e03e5d5f19e0a02d2cd85a57b28e390d0`.

Private lab: `recomp/analysis/d12d-upgrades`, port 9412, offscreen OpenGL,
dummy audio, private profile and copies of existing test savestates/cards.
No player saves or settings were written.

- Baseline and candidate Challenge Stage 4, 120 Hz: live held record 29;
  presented images before/after firing reproduce the centre obstruction and
  show the candidate using the low base-RPG framing from its first draw.
- Campaign level 0: `dnupgrade` sets mask 63 but ordinary weapon selection
  keeps the base held IDs. This path already uses the seeded base framing.
  Test selection must check the upgrade flags, not require held IDs 27-29.
- At 120 Hz, all six campaign upgrades were selected after `dnweapons` and
  `dnupgrade`; each base inventory record has upgrade bit 8 set. Presented
  captures cover idle, an attack, and crouch. Weapon models keep their base
  framing. Challenge stages 23 and 25 directly equip records 28 and 27;
  their idle and firing captures also retain the base framing. Original
  flame/projectile effects are still drawn.
- Challenge Stage 4 also passes the presented-image check at 60 Hz.
  Separate Modernized third-person and Vanilla smoke runs hold record 29
  with first-person blend 0 and zero first-person hand draws. The native
  exclusion tests, rather than the smoke runs, verify unchanged draw arguments.
- Player build and native input, aiming and controls suites pass. Controls
  include all 21 selectable overlay guards. New D12D assertions exercise
  the real hand and attached-draw hooks: 27/28/29 have byte-identical display
  matrices and equal mesh scales to 9/7/8, before and after muzzle-flash
  recoil, without modifying the held record or original joint matrices.
  All six upgraded records retain original hand/mesh/flash behavior in
  third person and Vanilla. Existing crouch-viewmodel checks also pass.

The diagnostic sweep initially used the wrong flamethrower key and then
expected resolved held IDs in campaign selection. Both were harness errors;
no gameplay changes were made in response.

## Player review

In Modernized, press P for first person and F10 if mouse capture is needed.
Check Challenge Stage 4 at rest and firing: the launcher should use the
same low framing as the normal RPG. In a campaign level, `dnweapons` followed
by `dnupgrade` provides the six upgraded weapons for checking keys 2, 3, 4,
5, 7 and 8. Compare standing, crouching, firing and switching. User visual
acceptance is complete; these bounded checks do not certify every
campaign encounter or costume.

Launch: `python3 recomp/tools/local/run.py`.

## Acceptance and closeout

The user fully accepted D12D and requested closeout. This executable is the
new regression baseline. No gameplay rebuild for closeout. Framework export
is unchanged; applying it to the clean pinned dependency reproduces all 50
patched files. No next job started.
