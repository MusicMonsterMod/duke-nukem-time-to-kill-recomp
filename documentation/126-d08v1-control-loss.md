# D08V1 - Modernized controls lost after a chain top exit

Status: **Accepted** (2026-10-07, user: "you can probably see my playtest log, i'm very happy with how it played, everything felt comfortable replaying that area"). Playtest log `session-20261007-202257.log`: two chain top exits (196), no `[TTK identity]` line, no identity lease loss. Modernized only; Vanilla unchanged.

User report (2026-10-07): "where i missed a jump, and somehow duke never went
back into modern controls mode, and ive lost the ability to control him ... we
must basically make sure that modern controls take precidence over everything".

Evidence log: `recomp/analysis/control-loss-20261007/session-20261007-195012.log`
(binary `3c0ca76e...`). Private test runs: `recomp/analysis/d08v1-20261007/`
(copy of UI slot 3, savestate file 02, SHA-256 `b4a5d740...` verified; the
player's file was only read).

## Cause

- The Modernized lease checks SHA-256 guards over the original code it relies
  on. One guard pair covered the upper-body animation flag table
  `0x800c2824 + 4 * anim` (280 entries), minus 149..153, because the ledge hang
  (`0x8004c768/7f0/8b0`) sets and the object hang (`0x80055438`) clears bit
  0x40 of 149/152/153.
- The pole and hang side probe `0x800439e4` (reached from the hang-climb at
  `0x80044388/98`) also sets or clears bit 0x40, on the entry of **Duke's
  current animation** (`lh v1,0(s2)` with `s2 = player + 0x60`, store at
  `0x80043bec`). On the chain's top exit that is 196, `0x61 -> 0x21`.
- The guard read that game-state write as changed code. `identity()` refused
  the lease every frame from then on, since the byte never returns, so
  Modernized was off until the game restarted. The earlier static review missed
  this writer: its `lui` sits in a branch delay slot and the store is indexed.
- It reproduced on the first private run: E mount on the slot-3 chain, W to
  the top, top exit 196, `[TTK identity] guard 52 ... 0x800c2b34: 0x00000021,
  expected 0x00000061`, and every later line `inactive (identity)`. The user's
  sequence (a mid-air catch 148 -> 154, then the top exit) reaches the same
  exit. It was not caught in D08J5 because its runs did not check the lease
  after the W exit.
- With the lease off, the WASD tank fallback gave forward/back and L2/R2
  strafes but no turn (the arrow keys did turn, as an unlabelled escape hatch).
  In the private repro W stood still against the platform, so the player was
  effectively stuck.

## Change

- `code_identity.h`: `masked_identity()` for original data that the game itself
  rewrites in place. A `MaskedGuard {address, size, mask, digest}` hashes and
  compares `word & mask`, so the bits the game writes are state and every other
  bit stays authenticated.
- `control_guards.inc`: the flag table moved from the code guards to
  `state_guards`. All 280 entries are covered (149..153 too, which were
  previously unguarded), with bit 0x40 masked on every entry and the low
  halfword of entry 265 masked: the bonus levels' `BONUS.OVR` (DB21..DB26)
  stores it at `0x800cf9a8` and `0x800cfc70`, so levels 21..26 would have hit
  the same lockout.
- `modern_controls.cpp`: `identity()` checks `state_guards` and names a
  state-table mismatch in the log. The verdict is still recomputed every frame,
  so bytes that return bring the lease back.
- `pc_input.cpp`: when the lease is refused for `identity` (a real code change),
  the fallback now pays horizontal mouse movement out as the original D-pad turn
  (11 counts per input frame, banked and capped). That is about 45 degrees for
  400 counts, near the modern camera's default 48. The screen shows
  `MODERN CONTROLS PAUSED - MOUSE TURNS, WASD MOVES`. Other refusals (scripted
  cameras, state) keep the existing fallback and message.
- `tools/local/guard_writer_audit.py`: a static audit of the owned disc. It scans
  the EXE and every overlay for stores whose `lui` base lands in a guarded range,
  carrying `lui` values through branch delay slots into the branch target and
  reporting indexed stores. It fails on any store into a code guard or an
  unreviewed state writer. With `--port` it diffs a running game's guarded
  words against the disc, after applying the masks.

## Audit (owned disc)

| Range | Writers found | Verdict |
| --- | --- | --- |
| Flag table bit 0x40 | `0x8004c778..8d8` (9 stores), `0x80055444..45c` (3), `0x80043bec` (indexed, any entry) | state, masked |
| Flag entry 265 low half | `BONUS.OVR` `0x800cf9a8`, `0x800cfc70` (DB21..DB26) | state, masked |
| `0x800bcbb4` (64 bytes, handler table) | `0x8001bd14` in setter `0x8001bcf8` | unreachable: no `jal` or pointer to it anywhere on the disc |
| Every other controls guard, and the aim, near clip, scene, level select, sky and draw distance guard sets | none | code |

The static scan follows only `lui`-formed bases. Pointer-held writes are covered
by the live mode: after mount, descent, top exit, A/D, a catch jump and a climb
out, 0 guarded words differed from the disc.

## Evidence (executable `cf00c0826e85cea9d555ebca0bec650d17837ffd9e7b5dad3cdf1522068a4e43`)

Private Xvfb runs, real keys and mouse, isolated profile and card copies:

| Check | Result |
| --- | --- |
| Baseline `3c0ca76e` (`repro.py`) | E mount, W, top exit 196: entry 196 `0x21`, lease off for good; fallback W did not move Duke and the mouse could not turn him |
| Fixed: E mount, W top exit, W, S (`repro.py fixed`) | entry 196 `0x21`, lease on throughout, W/S camera-relative after the exit, no `[TTK identity]` line |
| Catch jump x2 (`jump.py catch`: run, Space, W+E held) | 154 catch, 196 top exit, back on the platform, lease on |
| Missed jump (`jump.py miss`) | fall 98 -> landing 106 in the pit, then W runs with the lease on |
| Live guard audit after climbs and exits (`live.py`) | 0 guarded words differ (entries 149/152/153/194/195/196 all `0x21`/`0x18002`, state) |
| Forced identity loss (`force.py`: one byte of a debug string in the authenticated LEVEL09 body) | `[TTK identity] level 9 overlay changed`, `MODERN CONTROLS PAUSED - MOUSE TURNS, WASD MOVES`; mouse +400 turns Duke +491, -400 turns him -516; W moves him; restoring the byte logs `Modern movement lease resumed`, lease on at 63 |
| Vanilla (`van.py`, D-pad route) | Duke driven by the original pad, host moves/cameras/orbits 0, no lease or identity lines |
| Suites | `ttk-controls-test` (guard group now: bit 0x40 on 149/152/153/196/148/0/279 and entry 265's low half keep the lease; bits 0x1/0x20/0x80/0x10000 on 148 and 196, and 265's high half, still fail and recover when restored; 38 PASS groups), `ttk-input-test` (new identity mouse-turn case), `ttk-aim-test`, `ttk-near-test` PASS; Python 126 OK (7 skipped), including the new `test_guard_writer_audit.py` |

## Limits

- The bonus-level (`BONUS.OVR`) mask comes from the static audit only. No bonus
  level was played.
- A real code mismatch still turns the lease off (fail closed). The fallback is
  now steerable and labelled, and it recovers when the bytes return. It is still
  the original tank movement with the original camera, not Modernized.
- The `PAUSED` message appears after about a second of WASD in the fallback,
  not on mouse movement alone.
- The fallback turn rate is tuned to the default sensitivity and the game's
  frame rate. It does not follow the sensitivity setting.
