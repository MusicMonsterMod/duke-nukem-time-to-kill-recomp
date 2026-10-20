# D08T4 - Pushable objects fixed by the level (slot 10 dumpster)

Status: **Done** (user-accepted 2026-10-20: "lock it in, close it out, another winner"). Modernized only; Vanilla unchanged.

## Report

In the player's save slot 10 (UI slot 10 = file `slot09`, level 5, written
2026-10-20 06:46, SHA-256
`ec705ac62ac548d022d069b5d053f387065e483f3af787fa3740db3167a0a720`), Duke at a
street dumpster got `HOLD RMB TO GRAB`, but holding Grab did not move it.

## Cause: fixed by design (case 1), our prompt ignored the original's floor gate

The dumpster is object `0x801d74a8`, **type 165, flags `0x081090e2`**, exactly
the same type and flags as the first map's alley dumpster that pushes. It sits
against the back wall of a recess (Duke cannot get behind it).

The original idle Action `0x80051890` grabs a pushable object only when all of
these hold (disassembly of the owned executable):

| Gate | Address | Slot 10 | Alley (control) |
| --- | --- | --- | --- |
| Fresh Cross (`word & 3 == 1`) | `0x800518e0` | yes | yes |
| `player+0x224 & 0x241 == 0` | `0x8005190c` | `0x2` (pass) | `0x2` (pass) |
| Type flag `0x08000000` | `0x80051920` | yes | yes |
| `player+0x17c == 0` (else switch/use path) | `0x80051950` | 0 | 0 |
| Holstered (`player+0x3b8 == 0`) | `0x80051960` | yes | yes |
| **Bit 0 of byte 4 of Duke's floor record `*(player+0x48)`** | `0x80051970` | **0 (fails)** | 1 |
| `LineUpWithObject(0x113)` (`0x8007ec4c`) | `0x8005198c` | not reached | passes |

The floor records are 7 bytes (pointers seen `0x80180814`, `0x80180822`,
`0x801806fc`, `0x8018070a`, `0x80180961`, `0x80180968`). Placing Duke on every
reachable side of the slot-10 dumpster and along the street (17 points, private
copy, diagnostics only) read bit 0 = 0 everywhere. The level marks this floor as
one where grabbing never starts, so the dumpster is a fixed step, not a
blocked pushable. Turning on the game's own debug print flag (`0x800c3d16`)
printed nothing, consistent with `LineUpWithObject` never being called.

Vanilla on the same save copy, holstered, still, original pad: Cross does
nothing (anim 63, `player+0x290` stays 0); Cross + Up climbs it (139/140,
mode 8). Not a Modernized regression.

Modernized went wrong only in the host: `push_contact_ready()` checked the
type flag alone, so the touch showed `HOLD RMB TO GRAB` and Grab started a
request (with the automatic holster) that the original then ignored.

## Fix (game-wide)

`recomp/src/ttk/push.inc`: `push_floor_allows()` reads the same bit the original
reads (`*(player+0x48)`, byte 4, bit 0; pointer range-checked, unaligned).
`push_contact_ready()` now also requires it. That single predicate drives both
the grab hint and the Grab request in `pc_input.cpp`, so at a fixed pushable
object no prompt appears and Grab neither holsters nor requests. E (with W)
still climbs it through the original `0x80051cf0`. No other predicate, hook or
generated code changed; Vanilla never reads it.

`LineUpWithObject` can still refuse a grab (about 24 degrees or more off square,
as before); that is a transient facing condition, not a property of the place,
so the prompt still shows there, unchanged.

## Evidence (executable `810c815782bce5874332bd762d9e69c66de4fe2e15ef4b9bc0b5a829a5a27df4`)

Private copies only, in `recomp/analysis/d08t4-20261020/` (`cards/` slot 10,
`cards-alley/` the D08T slot-5 alley state, `cards-medieval/` the D08T2 slot-1
medieval block). Scripts: `look.py`, `scan.py` (RAM scan for pushable types),
`v.py` and `gates.py` (Vanilla, original pad, gate readout), `ring.py` (floor
bit around the dumpster), `live.py` (Modernized, real xdotool keys and mouse on
a private X display). Port 9261.

| Check | Result |
| --- | --- |
| Slot 10, Modernized, W to contact, hold RMB, W, release (fix) | no hint, weapon never stowed, no grab, moved 0 |
| Same with the gate disabled (A/B, temporary build) | `HOLD RMB TO GRAB` shown, weapon stowed, no grab, moved 0 (the report) |
| Alley dumpster (fix) | hint, grab 121, pushed 1131, weapon redrawn |
| Medieval Duke-symbol block, type 924 (fix) | hint, grab 121, pushed 1129, weapon redrawn |
| Vanilla, alley and slot 10, before and after | identical: alley Cross grabs (121); slot 10 Cross nothing, Cross + Up climbs |
| `ttk-controls-test` (new floor-gate cases: bit set, clear, other bits only, null record, non-pushable), `ttk-input-test`, `ttk-aim-test` | PASS |

## Limits

- Only this dumpster is known to sit on a no-grab floor; the gate is the
  original's own, so every such placement in every level follows it.
- First person was not run separately; the predicate does not depend on the view.
- `scan.py` also found a second type-165 dumpster about 5800 units higher
  (`0x801d6cac`); it was not visited.
