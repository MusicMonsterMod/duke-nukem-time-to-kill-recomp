# Session wrap — console scrollback, swim bridge, inventory quotes

2026-09-28 (morning). Playtest pending for most items; D28 accepted.

## Build

Binary SHA256 `f0c5ad2ab9a52e6d26353630916f3ba5272c7c964b3907f77230b8475da5c890`.

```sh
python3 /home/spartacus/Desktop/dn-ttk/recomp/tools/local/run.py
```

No commits this session. Profile settings migrate to **version 9**.

## What landed

### D26 — EDuke-style console
- Command echo + multi-line replies in console scrollback (`help`, `fps`, `clear`,
  `quit`, unknown errors).
- No centered toast flashes for command feedback.
- `fps` enables a persistent top-left multi-line debug block (small font) via
  `host_osd_set_debug` / `host_osd_debug_image` (future `stat` can reuse).

### Crosshair
- On-screen size halved (1× tile × UI scale; was 2×).

### D27 — Shift-run clunk
- Walk↔run switches only at plant phase 0 without zeroing `+0x68/+0x6a`.
- Caps Lock quotes already accepted.

### D28 — Done
- User confirmed Scroll Lock holster quotes.

### Inventory / Bio Mask (D08A follow-up)
- Default **B** = Bio Mask with centered `BIO MASK ON/OFF` (TTK’s own gadget).
- Underwater air still works automatically (no scuba toast; scuba item cancelled).
- Also: `JETPACK ON/OFF`, `NIGHT VISION ON/OFF`. Profile settings **v10**.

### D08M — underwater interim
- When submerged and off other leases, WASD feeds original D-pad bits
  (tank-relative). Full camera-relative swim still open.

## Verification
- `ttk-input-test` PASS
- `ttk-controls-test` with LEVEL00 fixture PASS
- Python profile/binding unit tests PASS

## Constraints preserved
Vanilla, aiming, furniture, ladders, silent cheats, assets/saves, no hand-edit
of generated C, no commits unless asked.
