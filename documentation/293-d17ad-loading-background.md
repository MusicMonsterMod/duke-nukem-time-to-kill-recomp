# D17AD - Tiled widescreen loading background

Status: Done (user-accepted 2026-10-28).

User: "great work! lock it in and close us out". Implementation, tests, manual
and documentation commit/push closeout are authorized as MusicMonsterMod.
Acceptance does not add new agent-observed gameplay evidence; the bounded
verification and title/demo capture limit below remain explicit.

## Cause and change

The supplied local `research/screencaps/loading-4-3.png` shows the tiled
radiation-panel backdrop ending at the original 4:3 edges. Resident function
`0x800220f4` constructs a texture-window command, one FT4 spanning
(-256,-120)..(256,120), and a texture-window reset. The separate spinner and
its accepted D17U replay are described in [note 141](141-d17u-loading-screens.md).

`recomp/src/ttk/loading_widescreen.cpp` uses the existing `0x8002bc18` entry
hook at return `0x80022270`. After the original quad is linked, it inserts
copies translated by -512 and +512 before the texture-window reset. Texture,
UVs, palette, colour and vertical coordinates are copied exactly. The centre
quad's geometry and material stay unchanged; the viewport clips the extra
coverage. There is no horizontal artwork scaling, new asset or spinner change.

The hook requires Modernized, the exact caller/arguments, the complete expected
packet shape/UVs/window commands, and the resident function's SHA-256 identity.
Fixed 4:3 and Vanilla take the original path. Adaptive 4:3 retains clipped side
copies: a loading replay captured at 4:3 must already contain them if the window
widens while that image is held. This corrects a resize case found in the first
candidate. The supported margin range is 0..512 native pixels; the launcher's
adaptive range remains 4:3..21:9.

A startup-reserved 336-byte GPU DMA allocation holds four pairs of packets and
a saved region cursor. Copies do not consume the game's packet arena. The
cursor advances with the saved/replayed allocation, and loading replay captures
the completed chain before drawing the spinner. Existing save-allocation
recovery is relevant because this adds an allocation before lazy animation
buffers. No save format, framework source, generated C or hook list changed.

## Verification

- Player build and `ttk-loading-test` pass. The native test uses the owned
  executable and checks material/centre preservation, translated coverage,
  texture-window chain ordering, ring reuse, adaptive 4:3, idempotence, identity
  rejection, malformed packets, wrong callers and Vanilla exclusion.
- `python3 recomp/tools/local/test_animation_save_restore.py` passes the actual
  runtime memory recovery, GPU clearing, startup reservation and animation
  metadata contracts.
- Private offscreen OpenGL captures cover fixed 4:3, 16:9, 16:10 and 21:9,
  Modernized 60/120 and Vanilla 60. Reviewed captures show repeated artwork
  across the wide frame, without uncovered columns or visible joining seams.
  The 120 Hz samples show 120-122 distinct presentations across roughly one
  second, zero worker failures and the normal approximately 33.4 ms loading
  image interval. These are bounded observations, not full performance tests.
- Xvfb/X11 adaptive resize covers 1280x720, 1000x700, 1440x900, 1600x700 and
  800x600, including returning from 4:3 to 16:9. The revised candidate retains
  coverage after that return. Resize events are asynchronous; a capture taken
  before its new margin is observed is not evidence for the requested shape.
  The final route waits for the expected margin (18, 51, 183, 0, 85) before
  capturing; every row on both outer edges contains backdrop pixels.
- With `PSX_REPLAY_COW_CHECK=1`, a separate diagnostic sample records 1,419
  restoration checks and zero mismatches. These checks are intentionally
  expensive and are not used to measure normal playback speed.
- Historical private saves and a private copy of current UI slot 9 load.
  Subsequent repeated/cross-save samples recover valid TTKA v1 animation
  headers. The first early sample during the expensive checking run had not
  yet recovered its table and is excluded from passing recovery evidence.
  A final route waits for recovery and passes current UI slot 9, historical
  private slot 3, then current UI slot 9 again.

- Title/main-menu text was inspected over the native demo. A subsequent
  75-second title/demo route did not reach a loading image, so that specific
  logo-plus-title/menu composition is not independently certified by this run.
  It uses the same guarded backdrop routine; verify those transitions in the
  player playtest rather than treating source inspection as visual acceptance.

Local captures and diagnostic data: `recomp/analysis/d17ad-loading/`.
Player settings/cards were not used as writable test destinations. No new
gameplay changes, build or launch were needed for the accepted closeout.

Candidate executable SHA-256:
`71328ba1be20f605acc1d598d49d25cf86b46e811c64e9018730920cb9bacb95`.

## Player review

Launch from the workspace root:

```sh
python3 recomp/tools/local/run.py
```

Check the spinning-nuke background during level loads and menu/demo transitions.
It should fill the selected widescreen picture with repeated panels while the
logo and menu text retain their shape. With WIDESCREEN set to AUTO, resize
through 4:3 and back to a wide window during a load. User acceptance is recorded above; no additional agent-observed full campaign
or physical-display test is claimed.
