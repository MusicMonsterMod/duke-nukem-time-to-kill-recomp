# D08J5 - Climb down chains and poles

Status: **Accepted** (2026-10-07, user: "that definitely works, and i accept it ... it works perfectly"). Modernized only; Vanilla unchanged.

User request (2026-10-07): "the ability to climb down chains. Slot 3 is a great
one for testing this with as theres a chain right in front of us ready to
attempt climbind down."

Test location: the user's UI slot 3 (savestate file 02), SHA-256
`b4a5d740570335bca3ee41951d3462b53d684e3fc028a0a1c7fcffc8e551acf8`. Private copy
in `recomp/analysis/d08j5-20261007/cards` (the player's file was only read).

## The slot-3 chain

- Duke stands on an attic platform (feet Y -3581). In front of him hangs chain
  object `0x801dd3a4`, type 842, flags `0x100412`, at (-4049, -1024, 31248). Its
  box is a 138 x 138 column from Y -3072 to 1024: the top is 509 below the
  platform floor, the axis about 350 units beyond the point where walking stops
  at the edge. A walkway at Y about 510 surrounds its lower part; the pit floor
  is at about 5,700.
- With the current build, every way down from the platform missed the chain:
  a running jump (W + E + Space) sails over it into the pit, walking stops at
  the edge, a standing hop lands back on the platform, running off falls past it.

## Original rules (disassembly and Vanilla fixtures)

- Poles and chains are type flag `0x400` objects. The airborne catch
  (`0x800475c0..0x8004765c`) attaches them with transfer 154 (`0x100` objects
  get 155, plain ladders 156/207), then the hang-climb 192..195.
- During 154 and the hang-climb the original places Duke 225 units from the
  axis along his heading every update (a fixture that moved him off that circle
  was put back on it the next frame). His heading alone decides his side.
- There is **no top mount** for a pole or chain (as for plain ladders, D08U).
- Rest (192/193, `0x8004421c`): Up asks `0x8007d65c(player, 0, 1)` and exits
  at the top with 196 when the exit test `0x8007d240` passes; Down asks
  `0x8007d65c(player, 1, 1)`. Stepping (194/195, `0x80044398`) asks the same
  probe every update in the direction of the step flag `+0x6a`.
- Down probe result 4 = "a floor within 360 below Duke's lowest bone"
  (`0x8007d8d4..0x8007d8f4`, also writing `+0x1c8`); the handler then steps him
  off onto it with 191 (mode 8). The test is signed and the floor is the level
  query `0x8007cf28` around Duke, stored at the probe's `sp+0x4c`. Near the
  top, on the platform side of the chain, it returned -3072 (the chain-top
  level) while Duke's feet were at -2805 or -2299, so Down started 191 and
  lifted Duke **back onto the platform**. On the far side the descent was
  clean.
- Vanilla fixture (attachment written as the catch does, raw D-pad): Down from
  1,500 below the top descends 194/195 and steps off onto the walkway (191) on
  either side; Down from 400 or 900 below on the platform side climbs back out.
  Square lets go. Modernized S already sent Down and descended the same way.

## Change (Modernized)

New `recomp/src/ttk/pole_climb.inc`, wired through `ladder_top.inc`,
`modern_controls.cpp` and `pc_input.cpp`:

- **E at the top of a pole or chain.** `ladder_top_available()` now also finds
  a pole or chain (flags `0x400`, not `0x100`/`0x200`; a column box with depth
  on both axes, at most 512 across and at least 1,024 tall) in Duke's cell list
  whose top is 300..800 below his feet with its axis 120..640 away. The D08U
  request path (E stow first, settled ground, the `E TO CLIMB DOWN` hint) is
  shared.
- **Mount.** The host writes the catch's attachment (`+0x17c`, `+0x180` = 0,
  `+0x1c4` = 0, mode 3/3, anim 154) and over 12 player updates lowers Duke to
  400 below the chain top and turns his heading half a turn (smoothstep) from
  facing the chain. Because the original places him from his heading, he swings
  round the chain to the far side and ends facing back toward the platform;
  then the original hang-climb owns him.
- **No step-off upward.** New function-entry hook `0x8003964C` (the bone
  lookup the down probe calls at `0x8007d7f4`, return address `0x8007d7fc`).
  Only when the probe's own saved return address is the hang-climb's
  (`0x80044340` rest, `0x80044434` stepping), its direction register s3 is 1,
  the attached object is a pole/chain and the floor at `sp+0x4c` is more than
  100 above Duke's feet, the host replaces that floor with one far below, so
  the original's 360 test does not step him off and the descent continues. A
  floor below is untouched: the original 191 still steps him off at the
  bottom.
- **Let go.** Ctrl on a pole or chain sends the original Square (as on
  ceilings, D08J4). S resting at a chain's end for 8 player updates without
  the original starting a step (nothing to step onto) also sends Square.
- **Exits.** The pole/chain top exit 196 and step-off 191 (mode 8 after the
  climb) join the D08U ladder exits: camera-only lease, neutral directions, so
  a held S no longer reaches the landing as an original backstep (82).
- Code identity guards: the hang-climb cases `0x8004421c`/748 and the probe
  `0x8007d65c`/1784.
- Codegen: `0x8003964C` added to `game.local.toml` `mod_function_entry_funcs`
  and regenerated (one generated line in `SLUS_005.83_full_13.c`). The codegen
  hash stays `0x8bab543c`; the player's slot copies still load.
- Counters: `ttk_input` -> `controls.pole` (`mount_starts`, `mounts`,
  `mount_aborts`, `floor_rejects`, `stall_drops`, `mounting`).

## Evidence (binary `3c0ca76e96fd7577a3875b8e7dce1d0c31fbeb0af27ff9591eddde7abd11d6be`)

Private Xvfb runs, real keys and mouse, dummy audio, isolated profile and card
copies (`recomp/analysis/d08j5-20261007/`):

| Check | Result |
| --- | --- |
| Walk to the edge, E (`top.py`, `final.py`) | hint `E TO CLIMB DOWN`; 63 -> 154 -> 192 in 27 frames; heading 791 -> 2839; Duke from the edge to (-3839, -2672, 31327), the far side; position steps under 250 units |
| Then S (third person) | 195/194 all the way down, 191 at Y 111..277, standing on the walkway at Y about 507; S neutral during 191 (no backstep) |
| Then S (first-person profile) | same mount and descent, 191 step-off |
| Mount strip (`strip.py`, camera pitched down) | Duke steps off, raises his hands to the chain, swings round it and ends facing the platform |
| Then W | top exit 196 back onto the platform |
| Then Ctrl / Space | let go (108), land on the walkway |
| Then A / D | screen-left -119 / screen-right +94, still on the chain |
| Platform side near the top (`mfix.py`, fixture at 400 and 900 below), S | descends to the walkway instead of climbing back out; log `floor -3072 above Duke (-2804) is not a step-off` |
| Vanilla fixture, Down from 900 on the platform side | original 191 back onto the platform (unchanged) |
| Vanilla Cross at the edge (`van.py`) | no mount, pole counters 0 |
| Ladder regression (`regress.py`, UI slot 5, third and first) | W+E climb and top exit past the LARD (`actor_exits` 1), E top mount, S descent and step-off: same states and end positions as the D08J3 baseline |
| Suites | `ttk-controls-test` (new D08J5 group: gates, Vanilla refusal, 154 attach, half-turn swing, probe floor rule incl. callers/direction/Vanilla, S-at-end let-go, exits keep the lease; 38 PASS groups with LEVEL01 and all levels), `ttk-input-test` (new D08J5 case: Ctrl and S-at-end send Square, S elsewhere Down), `ttk-aim-test`, `ttk-near-test` PASS; Python 121 OK (7 skipped) |

## Limits

- One chain played (slot 3, both chains there end over a walkway or crate).
  Other poles and chains share the flag, box and state rules, not
  individually played. The S-at-the-end let-go is only covered by the native
  test: no chain ending over nothing was found.
- The mount starts with Duke on the near side of the chain (the original's
  225-unit circle), up to about 400 units from where he stood; the swing then
  takes him round. Judge in the playtest whether it reads well.
- The mount needs the chain's top 300..800 below the floor and its axis within
  640; chains whose top hangs well below a ledge are not offered.
- Climbing up (W) is unchanged: from the far side near the top the original
  still exits onto the platform with 196.
