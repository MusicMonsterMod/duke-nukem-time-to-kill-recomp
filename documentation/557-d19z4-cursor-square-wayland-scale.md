# D19Z4 - the light square behind the pointer on a scaled Wayland screen

Job: [D19Z4](../jobs/menus-ui.md#d19z4---a-light-square-behind-the-mouse-cursor-citadel-wayland).
Machine: citadel, the TV PC (Ryzen 3 PRO 3200GE, Radeon Vega 8 / amdgpu
DCN 1.0, CachyOS kernel 7.3.4, KDE Plasma on Wayland, KWin 6.8.3, HDMI TV
1920x1080 at 60 Hz, KDE scale 145 %, overscan 40). Statuses below are
MEASURED (debugfs, protocol log, lab), USER (seen on the TV) or CODE.

## 1. Symptom (USER)

User, 2026-11-16: "whenever the mouse cursor is displayed on the screen,
there is an unusual lighter colored square that sits behind it with the
mouse cursor in the top left of this square". Never seen on the desktop or
the field laptop (both X11 at scale 1).

## 2. Who draws the pointer (MEASURED)

- The game runs **native Wayland** on citadel: SDL 3.4.10 picks it by
  default; the process holds one socket to `wayland-1` and none to X.
- The game sets no cursor image of its own. SDL asks KWin for its default
  pointer through the cursor-shape protocol
  (`wp_cursor_shape_device_v1.set_shape(serial, default)` in a
  `WAYLAND_DEBUG=client` log), so KWin draws the pointer from the theme.
  The game's in-game menus use relative mouse motion only.
- KWin reports scale 1.45 (`wp_fractional_scale_v1.preferred_scale(174)`),
  and the game's surface is 1324x745 logical. Before the fix the buffer was
  also 1324x745: the window was created without
  `SDL_WINDOW_HIGH_PIXEL_DENSITY`, so SDL used one buffer pixel per logical
  pixel.

## 3. What the display hardware shows (MEASURED, debugfs `dri/*/state`)

Fullscreen game, before the fix (KWin direct scanout):

| Plane | Buffer | On screen |
| --- | --- | --- |
| primary (plane-3) | the game's 1324x745 XR24 | scaled up to 1920x1080 |
| cursor (plane-5) | KWin's 256x256 AR24 cursor buffer | 256x256 at the pointer |

With a 1920x1080 game buffer (section 4) the primary plane is 1920x1080 to
1920x1080, unscaled, with the same cursor plane on top: the arrangement the
desktop has, where there is no square.

The square is consistent with amdgpu drawing the whole 256x256 cursor
buffer (the pointer image sits at its top-left) when the plane under the
cursor is hardware-scaled. The mechanism inside the driver is NOT
ESTABLISHED; the square is in the display engine's composition, which no
screenshot or readback shows (KWin's screenshots draw the pointer in
software and never show it).

## 4. A/B on the TV (USER, 2026-11-24, same session)

| Run | Game buffer | Square |
| --- | --- | --- |
| unchanged build, `SDL_VIDEO_WAYLAND_SCALE_TO_DISPLAY=1` | 1920x1080 | none ("No square") |
| unchanged build, no hint (as played on 2026-11-16) | 1324x745 scaled | "Square is back" |
| fixed build, normal launch | 1920x1080 | none ("Clean, no square") |

## 5. Fix (CODE)

`recomp/psxrecomp/runtime/src/main.cpp`, game window creation (SDL3
builds): `win_flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY`. On a scaled Wayland
screen the game now gets a drawable at the screen's real pixels (1920x1080
on citadel) and presents it unscaled. The renderer already sizes everything
from the drawable (`SDL_GL_GetDrawableSize` = `SDL_GetWindowSizeInPixels`);
window geometry (`SDL_GetWindowSize`) stays logical, which is what those
callers want (window width memory, adaptive widescreen ratio).

Side effect, wanted: the picture on scaled Wayland screens is no longer
upscaled from the logical size, so it is sharper. The internal render
resolution and GRAPHICS AUTO are unchanged (the GPU probe measures a fixed
1920x1080 target).

Scope: in the pinned SDL3 only the Wayland backend reads the flag (X11 and
Windows ignore it; Windows keeps its per-monitor DPI hints), so the desktop,
the field laptop and Vanilla are unchanged by construction. Exclusive
fullscreen on Wayland already used native pixels; borderless fullscreen
(the default) and windowed play were the affected modes.

## 6. Cost on citadel (MEASURED, real screen, interleaved)

Lab `recomp/analysis/d23ab/lab.py`, case `cad` (30 s walk, turn, fire), UI
slot 1, a copy of the player's profile, FRAME RATE 120 setting, native
Wayland, order old/new/new/old:

| Build | Game fps median / p10 | Pictures/s | Emulation thread | GPU busy |
| --- | --- | --- | --- | --- |
| old r1 | 60.0 / 59.2 | 60 | 55 % | 65 % |
| new r1 | 60.0 / 59.4 | 60 | 55 % | 70 % |
| new r2 | 60.0 / 59.1 | 60 | 55 % | 70 % |
| old r2 | 60.0 / 59.7 | 60 | 56 % | 68 % |

About +3.5 GPU points for the larger final picture; pacing unchanged
(98-100 % of intervals 13-21 ms in every run).

## 7. State

- User-accepted 2026-11-24 ("lock it in and close it out!"). Installed on
  citadel (its earlier build kept as `.citadel-1118`), launched from a
  desktop icon; the desktop's LTO+PGO player build rebuilt with the existing
  profile and installed.
- Not tested: other scaled Wayland setups (integer 200 %, other GPUs) and
  windowed mode on citadel.
