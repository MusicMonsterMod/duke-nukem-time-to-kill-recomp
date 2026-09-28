# Playtest iteration 48 — armed ladder reliability — 2026-09-28

## Player feedback

- Second-ladder run-jump+E works but still fails often while armed; holstered
  feels instant. Wants armed grabs to feel the same, including ground-ladder
  run-jump approaches.
- Inventory strip silence + tiles: **accepted** (D08A1 Done).
- Audio crackle: **none heard; close D18A**, reopen if it returns.
- Aiming: still happy.
- F key should not toggle FPS; reserve F for gameplay. Prefer a backtick debug
  console (fps command now; menu option later under D19/D21).

## Implementation

### D08J armed stow reliability

E-owned stow now boosts at **4x on the ground as well as in flight** (previously
only airborne got 4x; grounded E approaches were stuck at 1.5x). Jump with E
held refreshes the stow lease. Holster Circle pulse is shorter once jumping /
airborne (2 frames vs 4 on ground).

Tried 8x airborne boost; it skipped original upper events (upper stuck on jump
pose 104) and missed grabs — reverted.

Private evidence (dummy audio, fresh cards):

- `iteration48-armed-window2` early-E armed: stow mid-flight, attach 186/[3,3].
- `iteration48-nominal-armed` late-transfer armed: attach 207/[3,3], E at X5401.

### FPS key

Default `DisplayPerf` binding removed from `host_keymap.c` (F unbound). Patch:
`recomp/patches/time-to-kill-zzzzzzz-unbind-fps-f.patch`. FPS remains available via
`PSX_FPS_TELEMETRY=1` or `config.ini` `[KeyMap] DisplayPerf=...`.

### Backlog

**D26** — backtick debug console (`fps`, future helpers); migrate FPS readout
there and into modern settings later.

## Status

| Job | Status |
| --- | --- |
| D08A1 | Done (player accepted) |
| D18A | Done (player closed; reopen if crackle returns) |
| D08J | Needs playtest (faster grounded+air E stow) |
| D26 | Todo (console) |
| D08K | Blocked |
| D19B | Deferred |

Binary SHA256:
`42eeaf3dc31a7f7a94ae9201e0c47f32cb23c1680edcfca994acb5cb20eccbab`.

Native controls/input suites passed. No commits. Player profile/card identities
preserved (writable-state.log may change from live play). Original assets
untouched. Did not attach to an active player session.

## Please test

1. Close any old binary; launch the command below.
2. Armed pistol: run-jump+E to the second exterior ladder repeatedly — should
   feel closer to holstered.
3. Armed run-jump onto the ground/apartment approach ladder.
4. Confirm F no longer toggles the FPS title readout.
5. Inventory/audio smoke only if convenient.

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```

Modernized captures automatically; F10 toggles capture.
