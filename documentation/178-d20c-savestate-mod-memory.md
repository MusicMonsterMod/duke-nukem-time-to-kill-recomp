# D20C - Wrong colours after loading a savestate (and D20A)

Job: [D20C](../MODERNIZATION_JOBS.md). Status: Done (user-accepted 2026-10-21: "rock solid. it's working! the artifacting is no longer present upon load"; the user recreated the steps on their own saves). The same cause
explains [D20A](177-d20-quick-save.md#d20a-savestates-taken-shortly-after-a-load-framework-pre-existing),
fixed by the same change.

## Report

User, on accepting D20: "when loading, sometimes the colors are screwed up? i
cant explain, try loading several times. i was testing with my save in slot
one." Their UI slot 1 (file slot 00, sidecar `level 3`) is a first-person mine
scene, saved with F6 on 2026-10-21. Player settings: OpenGL 4x, 120 Hz,
corrected geometry and texture precision, first person.

## Reproduction (private copies only)

`recomp/analysis/d20c-colours` (local): a private level 3 mine save
(`d19k-load/cards-mine` slot 09 copied to slot 00), a private profile with the
player's Modernized controls and presentation, port 9381, offscreen OpenGL.

| Run | Route | Result |
| --- | --- | --- |
| `loop.py` (L), pre-fix | 12 debug loads, third person, no play in between | all 12 identical |
| `loop2.py` (Q), pre-fix | first person, F6 once, then 15 x (walk/turn 1 s, F9, shots) | load 4: correct for 0.1 s, then geometry missing and scrambled textures; load 7: wrong textures/colours on walls and the bench, persisting; load 8: `FATAL: GPU GP0 unknown command 0xF4` |

The wrong colours persist because the bad frame's garbage packets included
VRAM writes that overwrote texture pages and CLUTs; the level does not reload
them.

## Cause

Mods can allocate guest-addressable memory: Expansion 1 (`0x9F000000`) and a
1 MiB GPU-DMA aperture (`0x80F00000`) whose addresses survive 24-bit
ordering-table links. The near clip (D11B, D17R1) draws its host packets in a
four-region arena in that aperture and links them into the game's ordering
table. Savestates saved RAM, VRAM and every device but **not** these regions.

So a loaded state's RAM holds an ordering table that links into the arena,
but the arena holds whatever this process wrote there since: another frame's
packets, whose links run on into unrelated RAM. Walking it draws garbage,
uploads into VRAM (wrong textures and colours until the level reloads them) or
reaches a word that is no GP0 command (halt). Whether it hits depends on what
the arena held at load time, hence intermittent, and on first person (the near
clip's main user).

D20A fits exactly: its halting DMA list was at `0x80F40BD0`, region 2 of that
arena. In a fresh process the arena was not allocated yet, so the link folded
into main RAM (`0x140BD0`, the retail DMAC's 2 MiB folding) and the GPU read
`0xFA000045`.

## Fix

Framework (exported patch):

- `memory.c`: allocation records per region; savestate section
  `BS_SEC_MODMEM` (0x12, `boot_state.h`) with both regions' watermarks,
  records and bytes. On load, records equal to this process's (from the start)
  are restored; if this process's records are a prefix of the saved ones, the
  saved watermark is adopted too; a mismatch keeps this process's data from
  the first differing record on. Any of the aperture not restored is filled
  with end-of-list packet headers (`0x00FFFFFF`), so a stale link ends the
  list instead of drawing another frame's packets. States without the section
  (all saves before this build) get the same fill. A malformed section is
  treated as absent, never as a failed half-applied load.
- `boot_state.c`: writes the section (18 sections), applies it, fills when it
  is absent; the `load_timing` line reports `modmem=restored|partial|absent`.
- Neither header is in the codegen hash: the hash stays `0x8bab543c` and every
  existing savestate still loads.

Title (`near_clip.cpp`):

- The arena is reserved at startup (plugin constructor), not at the first
  packet, so the aperture is mapped before any load; otherwise an old save's
  stale links fold into RAM before anything can fill them.
- The arena's current region is mirrored in the word after the arena (saved
  with it). At the first frame after a load the near clip resumes from the
  loaded region and moves past it, so it never overwrites the region the
  loaded ordering table still links into. Render replay workers are excluded.

## Evidence

Executable `03bbe5f438a99b402b07e18ce25ade8685da787b6584d93f0950d8f711ebb0e5`,
offscreen OpenGL, 120 Hz, private profile/cards.

| Check | Result |
| --- | --- |
| `loop2.py` (R), new-format F6 save, first person, 20 x (walk/turn, F9, shots at 0.1 / 0.6 / 1.5 s) | 20 x `modmem=restored`, no FATAL, every shot matches (mean abs diff <= 2.5 of 765: torch flicker) |
| `loop3.py` (O), old-format save, 20 x (walk/turn, debug load) | 20 x `modmem=absent`, no FATAL, all clean |
| `loop2.py` (S), final build, 12 loads | 12 x restored, max diff 2.5 |
| D20A sweep (`sweep.py`): load, save after 0 / 0.3 / 0.5 / 0.7 / 1.0 / 1.2 / 1.5 / 2 / 3 s, then a fresh process loads each | Modernized and Vanilla, two source saves (level 3 mine, level 0 street): 36 / 36 load and run |
| D20A's recorded bad saves (`bad5.pst`, `z-0.3-q`, `z-0.7-q`, `z-0.3-`; old format, halted with GP0 0xFA before) in a fresh process | all 4 load and run, correct picture |
| Tests | `ttk-near-test` PASS, `ttk-input-test` PASS, Python 164 OK (2 skipped), framework `test_savestate_capture_boundary_guards.py` / `test_savestate_status_protocol.py` pass; pristine pinned framework + exported patch reproduces all 50 modified framework files; `check_repo.py` OK |

Before the startup reservation, D20A's bad saves still halted in a fresh
process (`modmem=absent`, aperture unmapped). That is why the arena is
reserved at startup.

## Limits

- Not run in a visible window, and not with the player's own slot 1 (their
  files are never used). The playtest is to load that save several times, as
  reported.
- Old-format saves cannot get their saved packets back. The fill means a
  stale link ends that frame's list, so the first frame after loading one may
  be partly drawn. Saving again stores the new format.
- No unit test for the section (memory.c links the whole runtime); the
  evidence is end to end.
- Vanilla: the aperture is now mapped from startup in both profiles (memory
  only; no retail code addresses it). Vanilla play is unchanged; the sweep
  passed in Vanilla.
