# D07G - Windowed crosshair scaling

Status: Done (user-accepted 2026-10-26).

User: "perfect, lock it in and close it out". Readability accepted; implementation,
manual and documentation commit/push explicitly authorized.

## Finding and change

The OpenGL crosshair used integer-floored drawable height / 480. A 9x9
source therefore stayed 9 pixels wide through many ordinary window sizes,
then jumped by nine pixels. The SDL/software path instead used logical
render height, so internal resolution could affect its displayed proportions.

Both supported SDL3 renderer paths now use the fitted game picture's output
height. Each bitmap extent is rounded from `texels * max(2, picture_height / 480)`.
The existing lime-green 9x9 artwork is unchanged, with nearest sampling and
an 18-pixel minimum. Above 960 picture lines, size grows fractionally rather
than jumping at whole scale factors. At 1080 lines it is 20 pixels; at 1440
lines the formula retains the previous 27 pixels. Letterboxing does not enlarge
it based on unused window height. Placement uses the fitted picture center;
integer raster placement can differ by a pixel between renderer paths.

The SDL3 draw temporarily uses output coordinates and restores logical
presentation, scale, viewport and clipping. Visibility, input, aim rays,
weapon behavior and Vanilla gates were not changed. The SDL2 fallback uses
the shared extent rule in its existing logical space; this build uses SDL3,
and SDL2 was not validated.

## Verification

- Incremental player build passed.
- Native `ttk-aim-test` passed, including crosshair preference, guarded fallback,
  parallax and muzzle obstruction. Ten UI art tests passed; existing unclosed
  image ResourceWarnings remain.
- Private Xvfb runs used copied cards and settings, private port 9496, UI slot 1.
  Actual presented-window captures covered OpenGL and software, 4:3 and 16:9,
  at 480x360, 640x480, 800x600, 960x720, 1280x720, 1200x900,
  1440x1080 and 1920x1080. All 32 captures contained the centered crosshair.
  Measured bounds were 18x18 through the smaller fitted pictures and 20x20
  at 1080 picture lines. Captures include letterboxing and pillarboxing.
- Real I-key input hid the crosshair on both paths. Earlier diagnostic `i`
  steering tokens did not send a supported key; those initial hidden captures
  are not counted as toggle evidence.
- Additional 4x internal-resolution checks on both renderers at 800x600 and
  1920x1080 matched the 1x crosshair bounds (36 measured window captures total).
- F11 exclusive fullscreen checked on the private 1920x1080 X display.
- Framework patch exported and checked against the clean pinned dependency;
  installed patch-stack verification passed.

Local evidence: `recomp/analysis/d07g/matrix.py`, per-renderer captures,
`results.json` and logs. Screenshots were inspected for crispness and centering.
This is bounded Linux/Xvfb presentation verification, not physical monitor DPI
coverage or full campaign certification. The user accepted the readability and fullscreen proportions after playtest.

Candidate executable SHA-256:
`9f32726f7b6edd0dc31216bb84f729dfcb142365421e6f59c8a74e37e03ff69c`.

Accepted and closed; no new build or game launch during closeout. No next job
started. Launch with the player's settings:

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```

Compare a small window with fullscreen (F11), resize through intermediate sizes,
and toggle I off/on. No new setting is required; existing Crosshair and Weapon
Aim preferences still govern visibility.
