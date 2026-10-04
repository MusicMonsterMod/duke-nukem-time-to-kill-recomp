# D22A - Portal transition keeps Modernized controls (LEVEL01)

**Accepted 2026-10-04.** User playtest: "Level 2 is working and is completely
playable. The teleport/level transition is functioning correctly and I can
proceed into the level and play normally." Executable
`9c9e2c3f3b07ddb2ad0dd9fea48b7adcf000037b03f2cd75295dfea7f95b34c0` is the
regression baseline. Issues found while playing LEVEL01 are separate jobs:
D11D (eye height), D12B (costume-aware kick), D17R (sky edges), plus D26A
(level select) and D22B (modern controls in every level and state).

## Report

User, 2026-10-03: load UI save slot 7 (file 06), walk into the portal and
start the next level. Duke then loses every Modernized control and the
first-person view.

## Reproduction

Private dated copy of the player's cards and savestates:
`recomp/analysis/d22a-20261004/cards` (slot 06 SHA-256
`b05b9437c0e15b298d54c7d5e118700efe2f625c7bbc6f5bc4c2521a16c07e3c`, equal to the
player's file at copy time). Offscreen harness on the real GPU, private
profile and port 9243 (`setup.py`, `route.py`, `repeat.py`).

Accepted baseline `a6f8c8cb...`: W into the portal, the level statistics
screen, Cross, and Duke stands in the Old West street in third person. Arrow
keys/original pad only; `ttk_input` reports `ready=false`, `fp.reason=lease`
and the refusal count climbs every frame. The log names the cause once:

```
[TTK identity] guard 119 (0x800ca968, 9652 bytes) changed at 0x800ca968: 0x00000004, expected 0x00000003
[TTK lease] inactive (identity) anim=63 state=0/0 ... ident=0 ctx=1
```

## Cause

The Modernized control lease authenticates its resident executable code and,
as guard 119, the 9652-byte code/table body of LEVEL00.OVR at `0x800ca968`.
That was a deliberate fail-closed boundary (documentation/18 and 20: "Other
maps must fail closed until their identities and ownership are verified").
The portal loads the next level's overlay over the same base, so the lease
correctly refused it. Nothing was lost from the profile: the first-person
request stayed set and returns as soon as the lease is live again.

A 2 MiB RAM snapshot in the new level, correlated against every owned-disc
overlay (`correlate_overlays.py`), shows `/D0/DB01/LEVEL01.OVR` resident at
`0x800ca968`: 12186 of 12196 bytes equal, every difference inside the final
16 bytes.

LEVEL01 has the same layout as LEVEL00. Every LEVELxx.OVR begins with its
level tag word (LEVEL00 = 3, LEVEL01 = 4, ... 0x1c), and LEVEL00/LEVEL01 end in
a 16-byte hit-position vector that the overlay itself passes to `0x8007177c`
(LEVEL00: `lui/addiu` at `0x800cb1a4`, call `0x800cb1ac`; LEVEL01: `0x800cbe48`,
call `0x800cbe50`, vector `0x800cd8fc`). A static scan of LEVEL01 for addresses
it forms inside its own range finds only debug strings, switch jump tables and
that vector (`selfwrites.py`).

## Fix

`recomp/src/ttk/control_guards.inc`:

- Guard 119 leaves the resident guard list.
- New `level_overlays[]`: tag plus code/table body digest for each
  authenticated level. LEVEL00 (tag 3, 9652 bytes, unchanged digest) and
  LEVEL01 (tag 4, 12180 bytes,
  `b3fa174b2bb6732dbd341725f59a853b8d75a0eee19ed180e2a0b841c1732445`).

`recomp/src/ttk/modern_controls.cpp` `identity()`:

- Reads the tag at `0x800ca968`, picks that level's body, and requires the
  resident guards **and** the complete body to match (same SHA-256
  authentication, per-frame memo and RAM-code-generation invalidation as
  before). An unknown tag, or a known tag over another level's bytes, fails
  closed. The log names the resident guard, unknown tag or level word that
  broke.
- The Modernized apartment pair remap (`apartment_identity_word`) applies to
  the LEVEL00 body only.
- `first_map()` (identity plus tag 3) now gates the three LEVEL00
  conveniences: the lights-off secret patch, light-switch targeting and the
  concealed bed pickup. They also kept their own `0x800be570 == 0` checks.

No hooks, addresses or guest behaviors changed; only which authenticated
overlay the existing lease accepts. Vanilla returns before any of this.

## Evidence

Native: `ttk-controls-test EXE LEVEL00_FIXTURE [LEVEL01.OVR]`. The optional
third argument (the owned-disc LEVEL01.OVR, extracted locally) adds the D22A
group: LEVEL01 identity keeps the lease, its tail vector is tolerated, its
last body word and an unknown/mismatched tag refuse, the apartment patch is
never written over LEVEL01, and restoring LEVEL00 restores the lease. All 31
earlier groups still pass. The test file also gained stubs for the D17N/D17P
renderer entry points (`pgxp_mesh_vertex`, `gte_nclip_culling_*`) that had
left the target unlinkable. `ttk-input-test`, `ttk-aim-test` and
`ttk-near-test` pass.

Offscreen routes (candidate build, 60 Hz, first person):

| Check after the portal (LEVEL01) | Result |
| --- | --- |
| Lease after arrival | `ready=true`, `fp=active`, blend 1.0, no identity refusal in the log |
| W / S | camera-relative movement (Modernized `moves` counter advances) |
| Mouse look | yaw follows mouse counts both ways |
| Space | jump; lease returns on landing |
| Fire (rebound to Q in the private profile only) | 27 view-aimed shots with flashes, including while turning |
| Start pause / resume | lease off while paused, back on and moving after resume |

Third person (`VIEW=third`), one session: portal, savestate back to LEVEL00
(slot 06), portal again. Both arrivals `ready=true` with zero identity
refusals, then a 90-second LEVEL01 wander (walk, run, strafe, turn, jump,
crouch, fire: 1153 moves, 93 shots, refusals 0). The only not-ready samples
were ordinary airborne/run-start states. A RAM dump after the wander still
differs from the disc LEVEL01 only inside the scratch tail.

Regression: all twelve private UI slots (LEVEL00, `0x800be570 == 0`) keep the
lease with zero refusals and first person active. Slots 8 and 9 start on a
ladder (state 3, anims 186-189, `ident=1`), which is the existing ladder
behavior. Vanilla on slot 06: no Modernized hook activity and no lease or
identity log lines. `0x800be570` reads 0 on LEVEL00 and 1 on LEVEL01, which
supports its use as the level index in the apartment checks.

Candidate executable SHA-256
`9c9e2c3f3b07ddb2ad0dd9fea48b7adcf000037b03f2cd75295dfea7f95b34c0`. Only
plugin sources changed; the framework patch and codegen hash are unchanged,
so existing savestates still load.

## Limits

- Only LEVEL00 and LEVEL01 are authenticated. Later levels still fail closed
  (original controls) until each is added after its layout and a route are
  verified (D22B). The level survey below shows other overlays' tails are not all
  the same scratch pattern, so do not add them by analogy.
- Offscreen scripted input cannot press Escape/mouse buttons; host capture
  release and Mouse1 were not exercised here (fire was tested through a
  private Q binding). The user's own playtest is the acceptance gate.
- This is one route, not campaign coverage (D22).

## Level overlay survey (owned disc, base 0x800ca968)

`levels.py`: LEVEL00 9668 bytes (tag 3), LEVEL01 12196 (4), LEVEL02 10780 (5),
LEVEL03 8808 (6), LEVEL04 1840 (7), LEVEL05 14148 (8), LEVEL06 8504 (9),
LEVEL07 8660 (0xa), LEVEL08 3700 (0xb), LEVEL09 12832 (0xc), LEVEL10 8266 (0xd),
LEVEL11 7704 (0xe), LEVEL12 13496 (0xf), LEVEL13 1960 (0x10), LEVEL14 604 (0x11),
LEVEL21-30 (0x13-0x1c). Only LEVEL00, LEVEL01, LEVEL03 and LEVEL09 end in 16
zero bytes; only LEVEL00 and LEVEL01 were confirmed to pass that tail to
`0x8007177c`.
