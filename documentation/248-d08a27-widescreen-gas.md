# D08A27 - Widescreen toxic gas (accepted)

User accepted the candidate on 2026-10-25: "lock it in and close us out. perfect".
D08A27 is Done; implementation, documentation, commit and push are authorized.

## Implementation

`recomp/src/ttk/gas_widescreen.cpp` intercepts the completed native FT4 packet
at 0x8002bc18, caller 0x8002ffc0. Resident routine 0x8002fc38 computes the native
vertical gas boundary and animated material 5, then hardcodes X to +/-256.
Its sole caller at 0x800354cc checks the exposed camera owner. Both resident
code ranges are authenticated against the owned SLUS-00583 executable; these
are not level-overlay addresses.

The hook requires Modernized, one player, a positive live widescreen margin,
material 5, the expected packet format and exact original X coordinates. It
changes only the four X halves to +/-(256+margin). Native colour, blend mode,
UV animation, vertical boundary and packet ordering remain unchanged. The
same hook runs in replay workers. World projection, gas damage and Bio Mask
state/charge are untouched. Native 4:3 and Vanilla bypass the change.
`DNTTK_GAS_WIDE=0` is a developer-only baseline comparison.

## Verification

- Incremental player build passes. `ttk-gas-test disc/SLUS_005.83` verifies
  full packet preservation except X, multiple margins, idempotence, malformed
  packet/address rejection, unrelated caller/material rejection, altered code
  rejection, split screen, Vanilla and native 4:3 exclusions. Sky tests pass.
- Private copies of UI slot 11 reproduce the cutoff with the fix disabled.
  Full-width before/after captures show coverage at 16:9 and 21:9, plus auto
  resizing between 4:3, 16:9 and 21:9. Camera turning, first/third person,
  mask on/off, pause/resume and reload were exercised and captures inspected.
- Fixed 21:9 was checked at 60 FPS. 16:9 and auto used 120 target; auto rendered
  1017 replay captures with zero reported failures. Its trailing mean present
  interval was about 18 ms, so this is not sustained-120-FPS evidence.
- Separate shallow boundary routes pass repeated head entry/exit during held
  fire at both 60 and 120 target, including automatic mask removal. The earlier
  deep-pit backward movement did not exit gas and is not exit evidence.
- Native 4:3 and Vanilla load/present smoke checks pass. No writable test used
  the player's cards or settings. All diagnostic processes were stopped.

Evidence is local under `recomp/analysis/d08a27/`. User play acceptance does
not imply exhaustive campaign/cutscene coverage or a performance benchmark.
No framework changes were needed. The unchanged exported runtime patch matches
all 52 modified/new files when applied to the clean pinned dependency.

Accepted executable SHA-256:
`019d10de6a4977d6319f53811227121e8befc2a7b5057294b9ad8b13d65e741c`.
No post-acceptance gameplay edits, builds or launches. No next job started.

```sh
python3 recomp/tools/local/run.py
```
