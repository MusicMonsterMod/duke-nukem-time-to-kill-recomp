# D13 - Higher internal resolution and display scaling

**Done - 2026-09-30 (user-accepted):** "correct correct correct, D13 is good. I'd say
let's approve it." 4x held 60 fps on the user's GTX 1080 Ti ("it just looks
amazing").

## What the runtime already had

The pinned runtime (`psxrecomp`) already implements, but this build never exposed:

| Capability | Runtime variable | Notes |
| --- | --- | --- |
| Internal-resolution supersampling 1..4 | `g_video_scale` -> `gr_set_scale` | OpenGL: hr FBO at N times VRAM. Software: N times VRAM mirror. Clamped to `SW_MAX_INTERNAL_SCALE` (4). |
| Windowed / borderless / exclusive fullscreen | `g_fullscreen` (0/1/2) | Applied at window creation; Alt+Enter toggles the configured mode. |
| Window width | `g_video_win_w` | 0 = maximise to fit the display (default). |
| Output (present) filter | `g_video_aa` | Linear (default) or nearest when the finished image is scaled to the window. |

The only way to set them was `build-local/settings.toml` (written by the
disabled launcher UI) or `game.local.toml` (which also keys the movie-shard
cache). Neither is a per-profile preference.

## Implementation

- Runtime patch `recomp/patches/time-to-kill-zzzzzzzz-presentation-cli.patch`
  (main.cpp only): `--internal-scale 1..4`, `--display windowed|borderless|exclusive`,
  `--window-width 0|640..16384`, `--output-filter linear|nearest`. They are
  applied with the other CLI overrides (after game.toml and settings.toml),
  so the launcher's saved choice wins. Invalid values are ignored with a
  message. The log prints `psxrecomp: presentation (CLI): ...`.
- `player_profiles.py` schema 14: each profile's `presentation` now holds
  `renderer`, `internal_scale`, `display`, `window_width`, `output_filter`.
  Defaults: Vanilla 1x (original), Modernized 4x (the user's choice after the
  playtest; initially 2x); both windowed/fit/linear
  (the previous behavior). Schema 13 files migrate with a backup, keeping the
  renderer and every other preference. An invalid presentation block restores
  only presentation defaults.
- The software renderer always launches at 1x: its supersampling is CPU-bound
  (29 fps at 2x). The saved scale is kept and applies again with OpenGL.
- `run.py` flags `--internal-scale`, `--display`, `--window-width`,
  `--output-filter` save for the selected profile; `--settings` menu choice
  **R** edits all four.

Scene detail versus enlargement: **internal scale** rasterises polygons at
N times the PSX resolution (512x240 gameplay -> 1024x480 at 2x, 2048x960 at
4x), so model edges, thin geometry and distant detail gain real pixels.
Textures keep their original texels (filtering and replacement are D16).
**Window width, display mode and output filter** only enlarge the finished
image; at 1x a large window is a scaled 512x240 picture.

## Evidence

Isolated harness `recomp/analysis/d13-resolution/` (Xvfb :93, port 9193,
private profile `profiles.json`, copied cards `cards/` from D08L, state slot 1
= apartment). Xvfb uses Mesa's CPU OpenGL (llvmpipe) on the i7-5960X, so the
timings are a worst case, not the user's GTX 1080 Ti.

| Run | Renderer / scale | fps (10 s, apartment) | frame total ms | scene GPU ms |
| --- | --- | --- | --- | --- |
| `s1x` | OpenGL 1x | 59.92 | 16.7 | 13.4 |
| `s2x` | OpenGL 2x | 59.87 | 16.7 | 13.1 |
| `s3x` | OpenGL 3x | 59.49 | 16.9 | 13.5 |
| `s4x` | OpenGL 4x | 50.30 | 19.3 | 16.0 |
| `sw1x` | Software 1x | 60.38 | - | - |
| `sw2x` | Software 2x | 29.09 | - | - |

- Comparison captures: `runs/s1x/window.png` vs `runs/s4x/window.png`
  (same state, 1440x1080 window); crop `runs/compare-1x-4x-crop.png`. At 4x
  Duke's model, belt, boots and polygon edges are visibly sharper; texel size
  is unchanged. HUD position and size are identical.
- The runtime log confirms `GL GPU pipeline ready (internal scale Nx ...)` for
  2, 3 and 4. `screenshot_hires` reports scale 1 under OpenGL (it reads only
  the software mirror); window captures are the OpenGL evidence. Software 2x
  `screenshot_hires` is 1024x480.
- Intro FMV (`runs/boot1x`, `runs/boot4x`, no state load): the movie picture
  occupies exactly 1406x542 at +0+269 in both, i.e. identical framing. MDEC
  24-bit frames bypass supersampling by design.
- Pause menu at 4x with nearest filter (`runs/persist/pause.png`): correctly
  sized and placed.
- Persistence: preferences saved with `--show-settings`, then a launch with no
  presentation flags logged `internal scale 4x, display borderless, window
  width 1280, output filter nearest`; the window was 1280x960.
- Display modes under metacity on Xvfb (`runs/fs-*`): windowed opens maximised
  (1920x1043); borderless and exclusive report `_NET_WM_STATE_FULLSCREEN` at
  1920x1080 with a centered 1440x1080 4:3 image. Xvfb has one display mode, so
  a real exclusive mode change is not verified.
- Vanilla route `d13-vanilla-1`: exit 0, launched at 1x
  (`presentation (CLI): internal scale 1x`), captures reviewed normal.
- Python suite: 79 tests OK (2 skipped), including 6 new D13 tests. Native
  `ttk-input-test` PASS.

Initial binary `14fde30b76f3907effa6680603a367d28458dda0ed5aea69cb65c79711bee71d`; after the follow-up fixes `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`
(previous build `a16c13ca...`).

## Playtest follow-up fixes

- **Alt+Enter / Ctrl+F did nothing.** `host_keymap_match_event` compared
  modifiers exactly: an event with left Alt (`0x0100`) never equalled the
  stored `KMOD_ALT` (`0x0300`, both Alt keys). This is an upstream runtime bug
  that affected every modifier hotkey. Fixed by comparing Ctrl/Alt/Shift
  groups (`mod_groups`). New `test_host_keymap.c` cases cover left Alt, right
  Alt with Num Lock, left Ctrl+F, plain Enter (no toggle) and Alt+Shift+Enter
  (no toggle); the test passes. Traced with a temporary key log: Enter
  arrived with mod `0100`, and Ctrl+F with `0040`.
- **Exclusive was identical to borderless.** This build uses SDL3, where
  `SDL_WINDOW_FULLSCREEN_DESKTOP` is an alias of `SDL_WINDOW_FULLSCREEN`, and a
  window without a fullscreen display mode is borderless. Exclusive now sets
  the desktop's own display mode (`psx_apply_fullscreen_display_mode`) at
  window creation, on Alt+Enter and from the runtime menu. Borderless clears
  the mode. The log prints `exclusive fullscreen mode WxH @ Hz`.
- Verified under metacity on Xvfb (`runs/ae-exclusive`, `runs/ae-borderless`):
  Alt+Enter switches fullscreen to windowed (1440x1043) and back, Ctrl+F does
  the same, and plain Enter does not toggle. Vanilla route `d13-vanilla-2` exit
  0; Python 79 OK; `ttk-input-test` PASS.
- Reports that are not D13, checked on a private copy of slot 12: Duke
  headless and semi-transparent in the narrow sewer is identical at 1x, at 4x,
  and with the original camera (`runs/u12-compare.png`,
  `runs/u12-origcam/small.png`). The GPU's oversized-polygon rule runs on
  original coordinates before scaling (`gpu.c` 3521-3522), so the scale
  cannot change which door polygons are drawn. Backlogged as D10B. Correction: the missing head itself was
  a first-person hide flag stuck in the slot-12 savestate (D11C), not the
  camera. The
  controls flickering between modern and original while mantling is lease
  coverage (anims 139/140/142 and 107/108 in the session log). Backlogged as
  D08V.

## Windowed default and remembered display

- Windowed is the default for both profiles. `display` is how the game
  opens; `fullscreen_mode` (borderless/exclusive) is what F11 enters
  from a window (`--fullscreen-mode`). Profile schema 15; v14 files take
  `fullscreen_mode` from a fullscreen `display`, otherwise borderless.
- At exit (`shutdown_runtime`) the runtime writes `--presentation-state FILE`
  (`<profiles>.presentation-state`):
  `{"display", "fullscreen_mode", "window_width"}`. It records the real
  window state after any F11 toggle, the fullscreen kind (SDL3: whether a
  fullscreen display mode is set), and the last windowed width (0 when
  maximised), which window resize/maximise/restore events keep up to date.
  `run.py` absorbs it into the profile that was played and prints `Saved
  display ...`. An invalid file is discarded.
- Verified (`runs/st2`, `runs/st3`, metacity on Xvfb): see the work log. A
  forced X window destroy is not a close, so it writes nothing; closing with
  the window manager (Alt+F4) or the close button does.

## F11 fullscreen key

At the user's request, **F11** is the only default fullscreen key
(`host_keymap.c` default). Alt+Enter and Ctrl+F are no longer bound;
`config.ini [KeyMap] Fullscreen` can still rebind it. The modifier-group fix
stays in, covering rebound hotkeys such as `Alt+Return` or `Ctrl+F2`
(`test_host_keymap.c` now tests these through a config rebind). Verified
under metacity: F11 goes from the 1280x960 window into exclusive and back;
Alt+Enter does nothing. Vanilla route `d13-vanilla-4` exit 0; host keymap test,
Python 82 and `ttk-input-test` pass.

## Limits and notes

- 4x at 60 fps is confirmed by the user on a GTX 1080 Ti. The fixed Alt+Enter
  and the real exclusive mode still need confirmation on their display.
- 4:3 only. Widescreen, FOV and HUD anchoring are D14.
- Changes take effect on the next launch. The runtime F1 menu can also change
  video options and write `build-local/settings.toml`; the launcher's saved
  choice overrides those on the next launch.
- Pre-existing, not D13: `apply_runtime_patches.py` reports the installed
  stack as not matching because live `runtime/src/host_osd.c` has drifted from
  `time-to-kill-zzzzzz-inventory-strip.patch` (the reverse check fails at
  `host_osd.c:151`). This already failed before the D13 patch was added; the
  D13 patch itself reverses cleanly. A fresh `build.py` from pristine runtime
  source would need that patch regenerated. Tracked here for a separate fix.
