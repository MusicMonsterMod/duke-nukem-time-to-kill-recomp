# Time to Kill jobs - Release and project

Job descriptions and work logs for D01, D02, D24, D25, D30-D34, D58 and later
project jobs, R01, PRIV01. Status and dependencies are on [the job
board](../MODERNIZATION_JOBS.md); the dated session log is in the
`log-YYYY-MM.md` files beside this one. Moved verbatim from the board on
2026-11-10 (D58), one section per job in ID order.

## Jobs

### D01 — Establish the Vanilla regression route

Create a repeatable route from boot and FMV through first-level combat, inventory, security-card insertion and the newly accessible area, then save/load and window close. Record build identity, settings, input steps, expected behavior and timings. Preserve test memory cards separately from the player's cards. Include the voice/music transition and diagnostics from the previous bugs.

**Acceptance:** another session can repeat the route from documented starting conditions; current results and known gaps are recorded. Include both observed user feedback and reproducible checks without treating either as full-campaign proof.

### D02 — Persistent Vanilla / Modernized profiles

Introduce versioned, persistent settings with explicit defaults and a minimal usable mode selector. Separate original gameplay behavior from optional presentation settings. Keep Vanilla as the initial default until the modern preset is playable; label incomplete Modernized features honestly. Document when changes take effect.

**Acceptance:** selection survives restart; restoring each preset works; malformed or older settings recover sensibly; switching back restores Vanilla behavior and preserves user customizations predictably. No advertised toggle silently does nothing.

### D24 — Linux / Windows player build and disc import

Create reproducible player builds with a simple launch flow, settings/save locations and clear owned-disc import errors. Verify Windows independently rather than extrapolating from Linux. Package permitted runtime components; keep original disc assets and personal saves out of redistributable artifacts.

**Acceptance:** a clean installation on each claimed platform imports the supported disc, launches, saves/loads and closes successfully. Document dependencies, licenses, build identity and known limitations in the player instructions.

### D24A - Public clone gives the full experience (assets out of research/)

**Accepted 2026-10-07** (user: "now working significantly better"). **No Duke Nukem 3D art remains** ([note 112](documentation/112-d24a-open-assets.md)). The user confirms the look; the fresh-clone test is re-run after commit. Route (a) chosen, all Duke 3D art removed.
Art direction (2026-10-06): the user will hand-make original, openly licensed
icons in the TTK style for mission items, keys/key cards and steroids. The disc
has no 2D art for those. Fonts, digits, buttons and HUD sprites remain disc-derived.
User: "We are going fully open, so the duke fonts must be removed. we need to
replace them with 1:1 analogues." Font picker built; waiting for the user's
font/palette choices per use (see work log).

**User request, 2026-10-06.** "the font assets etc need to be moved into
a proper director, not in the research. i need to make this whole project
available online for anyone to download and play!"

**Evidence (2026-10-06 fresh-clone test).** A clean `git clone
--recurse-submodules` of `main` (5f9e705), plus an owned USA disc in `game/`,
built with `build.py` (exit 0). All 69 generated files were byte-identical to
the working copy, as were `recomp/src` and the patched framework. Jetpack,
fire, landing, fuel and Vanilla checks gave the same results as the working
build. Missing from the fresh build (present only in the ignored
`recomp/assets/` of the working copy, built from local `research/`):
- `ttk-inv-icons.pack` (+ provenance): D08A3 inventory icons, extracted from
  the TTK disc's `/DATA/FONTS.RAW` by `build_ttk_inv_icons.py`.
  `build.py` never runs that step.
- `ttk-inv-digits.pack` (+ provenance): the D08A2 green inventory digits and
  %, from Duke Nukem 3D art tiles under `research/inv/font/`.
- `fonts/Messages` (+ provenance): the D19A Duke message font, from Duke Nukem
  3D / Atomic Edition tiles under `research/`. Without it the generic host
  font is used.

**Scope.**
1. Disc-derived assets: `build.py` extracts `ttk-inv-icons` from the user's
   own disc (`/DATA/FONTS.RAW`) automatically and deterministically. The output
   is ignored build/asset output, never committed, and gets a clear error if
   the disc lacks it.
2. Fonts and digits: move every build input out of `research/` into a proper,
   documented project location that the build consumes. **Licensing
   constraint:** the current glyphs and digits are Duke Nukem 3D art, and
   AGENTS.md forbids committing extracted retail assets. Decide (with the
   user) before committing any of them:
   - (a) derive equivalents from the TTK disc at build time (TTK's own fonts
     in `FONTS.RAW`), so every owner gets them;
   - (b) an original, freely licensed font/digit set made for this project and
     tracked in the repo;
   - (c) an optional "supply your own Duke Nukem 3D files" import with a
     documented path, falling back to (a) or (b).
   Do not publish Duke 3D-derived art without a licence that permits it.
3. `check_repo.py` and the docs updated so the build never reads `research/`,
   and the README states exactly what a clone plus a disc gives.
4. Re-run the fresh-clone test (empty folder, README steps only, owned disc)
   and confirm that fonts, icons and digits are present and match the
   accepted look.

**Acceptance:** a fresh clone, plus the README steps and an owned USA disc,
produces a build with the message font, inventory icons and digits (by the
chosen route) and no dependency on `research/`. No retail or third-party art
is committed without a permitting licence. The user confirms the look.

### D24B - Savestate menu (F7) dressed in the TTK fonts and disc art

**Accepted (2026-10-06): "im happy with all progress tonight"; built to the approved mockup** ([note 113](documentation/113-d24b-savestate-menu.md)). User: "i was also wondering if we
could dress up our save screen while we have the fonts open too". Asked which
screen; the user chose the recomp savestate slots (F7).

Proposal: the font picker's "Savestate menu (F7) redesign - mockup".
- Header: "SAVE STATES" in TTK Big Italic gold.
- Slot cards: thumbnail; slot title and level name in TTK Medium Italic gold
  (blue when selected); date and level line in System 8x8.
- Prompts: the disc's button sprites with Medium Italic labels.
- Backdrop: dark steel with the disc's radiation emblem.
- Text drawn at 1x with the navy shadow.
The panel is the framework's `psx_savestate_menu.c` (640x480). The plan is a
game-specific render hook (the framework panel stays the fallback, and Vanilla
gets the same look or keeps the original, to be decided), plus reading the level
index from each savestate for its name.

**Acceptance:** F7 shows the approved design with correct slot data, load/save
and navigation unchanged; the fallback works without the font pack.

### D24C - TTK-font exclamation mark and `>` console prompt

**Done (user-accepted, 2026-10-08): "all accepted".** User: "in the case of cheats like dnstuff which
gives the message "giving everything!" we dont have an exclaimation mark in this
font ... I'm thinking we could fake an exclaimation mark with the I character and
the period. I want the console prompt character itself to be a > rather than a |
also." Two small, related font jobs done together at the user's request.

- The TTK Big and Medium Italic fonts have no `!`, so it fell back to the 8x8
  system font. `build_ttk_fonts.py` now draws one from each font's own `I` and
  `.`: the I's top as the stem, tapered towards the bottom (Big 3 px, Medium
  2 px) and closed with the I's own bottom edge moved up the slant; a clear row;
  then the period minus its top rows (Big 2, Medium 1) on the baseline. Every
  set built from those fonts (messages, headings, console, savestate panel
  titles and slots) gets it.
- The console prompt and the echoed command lines use `>` (a native Medium
  Italic glyph) instead of `]`, which the font lacked and fell back to a
  thin system-font glyph that read as `|`.

**Acceptance:** `dnstuff` shows GIVING EVERYTHING! with an exclamation mark in the
message font; the console prompt and command echoes start with `>`.

### D24D - `<` and `>` in messages use the system-font fallback

**Done (user-accepted, 2026-10-20): "its perfect! lock it in close it out".** Originally Todo. User report, 2026-10-08:** "Message: original movement <state> but the
< and > are in fallback font despite that we have those characters
available." The message font set (built by `build_ttk_fonts.py`) appears to
lack `<`/`>` even though the TTK fonts include them (the console already uses
the native Medium Italic `>`, D24C). Find which set and style the message uses,
map both glyphs from the disc fonts, and check every other character that
still falls back.

**Acceptance:** the reported message shows `<` and `>` in the TTK font; no
other message-font fallback for characters the disc fonts have; the user
confirms.

**Work log, 2026-10-20 (Needs playtest).** `<` and `>` were already native in
every italic set. The message is `ORIGINAL MOVEMENT (%s)`, and its parentheses
were the system-font glyphs, because the disc fonts have no `(` or `)`.
`build_ttk_fonts.py` now builds `(` and `)` from each font's own `O` (the side
strokes, with half the top and bottom bars following the slant). A scan of our
UI strings found no other italic-set fallback in use except `%` (D24E). Font
tests pass. An offscreen console echo `(PAREN) 50%` draws them in the TTK
font. Only the font pack changed; executable `bbfaf117...` is unchanged. See
[note 165](documentation/165-d24d-d24e-font-glyphs.md). Awaiting the user's
check.

### D24E - TTK-font percent sign from period and slash

**Done (user-accepted, 2026-10-20): "its perfect! lock it in close it out".** Originally Todo. User request, 2026-10-08:** "We need to fake the percent symbol for our
fonts in exactly the same way we constructed the exclamation mark. i think
period, and forward slash might work like ./. with the first one being
positioned at the top. This is especially necessary in "medkit 39%" or
something at the top when using it."

The TTK Big and Medium Italic fonts have `.` and `/` but no `%`, so `%` falls
back to the system font (`MEDKIT %u%%`, `recomp/src/ttk/shortcuts.inc`). Build
it in `recomp/tools/local/build_ttk_fonts.py` next to D24C's `exclamation()`:
the font's own `/`, a period near the top-left on the slant and one on the
baseline at the right, sized for each font. Preview it in the local font picker
first. Every set built from these fonts (messages, headings, console, savestate
panel) gets it.

**Acceptance:** MEDKIT 39% (and any other `%` message) shows the built `%` in
the message font; Big and Medium both have it; it reads as a percent sign at
1x and 2x; font tests pass; the user confirms.

**Work log, 2026-10-20 (Needs playtest).** `percent()` in
`build_ttk_fonts.py` draws `./.` from each font's own `/` and two periods
(trimmed as for `!`). One sits at the top left, the other on the baseline at
the right, each one clear pixel from the slash. Big and Medium both have it,
and previews at 1x and 2x read as `%`. An offscreen `dnstuff` + M run shows
MEDKIT 100% with the built glyph. Font tests pass (new
`test_parentheses_and_percent_from_own_font`). See
[note 165](documentation/165-d24d-d24e-font-glyphs.md). Awaiting the user's
check.

### D25 — Modernized edition release acceptance

Run the documented campaign and regression checks against both presets in a frozen candidate build. Reconcile manual, settings, supported platforms and feature claims. Choose release scope explicitly; optional first-person or HD packs need their own completed acceptance work to be advertised.

**Acceptance:** no known release-blocking progression, input, save or playback issue in the claimed scope; comparison evidence shows Vanilla preserved and Modernized usable; reproducible build and player instructions are complete. Record remaining issues visibly.

### D30 - List the project on recomp.board (`.recomp.json`)

**Done 2026-10-18. Opened the same day at the user's request.** Project visibility, not
gameplay. No game code, build or launch is involved.

[recomp.board](https://recomp.fyi/) ("who's recompiling what") lists game
recompilation, decomp and source-port projects. Route A: a `.recomp.json`
at the repository root owns the project's row; the board reads it daily
(the file, never anything else, and never writes to the repo), and whoever
can commit there is the proof of ownership. Format and field list:
[recomp.fyi/add.html](https://recomp.fyi/add.html), schema
`https://recomp.fyi/schema/v1.json`.

**Current state (checked 2026-10-18).** The board's daily scan already
found this repository and lists it, **unclaimed** ("Claimed: no"):

- Table row `duke-nukem-time-to-kill-musicmonstermod`: PS1, recomp; Runs on,
  Toolchain, Status and Version empty; last commit 2026-10-09 (stale).
- Game page [recomp.fyi/game/duke-nukem-time-to-kill-ps1](https://recomp.fyi/game/duke-nukem-time-to-kill-ps1):
  Maintainer empty; Description taken from the GitHub repository
  description; **Works from: "SLUS-00583 (USA), SLES-01515 (Europe)" - wrong**,
  the scan misread the README's warning that PAL SLES-01515 does *not* work;
  Started 2026-09-27; MIT; C++.
- A second, unrelated row on the same page: `alexbeavs-ps1-ports/duke-nukem-time-to-kill-recomp`
  (Psxrecomp, Windows/Linux/macOS, v0.3.6, GPL-3.0, also unclaimed). Not
  ours; leave it alone.

**How claiming works.** A committed `.recomp.json` on the default branch
claims the row: it loses the unclaimed mark, only the file (or a form from
the owning GitHub account) can edit it, and the periodic scan stops touching
it. The file is re-read daily, so editing it keeps the row current. Deleting
it releases the claim (the row keeps status, targets, method and toolchain;
the rest is cleared). The alternative, Route B, is a GitHub issue form on
[neurocontrarian/recomp-board](https://github.com/neurocontrarian/recomp-board)
sent from the MusicMonsterMod account, but it snapshots status and notes on
the day it is sent; the file is the better fit for a moving project. The
game page offers a starter file (choose project and status, Copy the file);
for our row it is only:

```json
{
  "$schema": "https://recomp.fyi/schema/v1.json",
  "game": "Duke Nukem: Time to Kill",
  "wikidata": "Q2666099",
  "system": "PS1",
  "type": "recomp"
}
```

**Work:**

1. Re-fetch the starter file, the field list and the schema
   (`https://recomp.fyi/schema/v1.json`, `additionalProperties: false`, so
   only listed fields and `x-` extensions) before writing; they may change.
2. Write `.recomp.json` at the repository root from the starter, under
   8 KB, ASCII hyphen-minus only, and fill what the scan left empty or got
   wrong. Proposed content, for the user to approve:
   - keep `game`, `wikidata` "Q2666099", `system` "PS1", `type` "recomp";
   - `original`: **only** `{ "region": "USA", "serial": "SLUS-00583" }`, to
     replace the wrong Europe entry; a `sha1` only if the user wants it and
     it is computed locally from their own dump (never a file, link or
     mention of an ISO/ROM or where to get one, or the whole file is refused);
   - `toolchain` "Psxrecomp" (the pinned `recomp/psxrecomp` upstream);
   - `status` "in-progress" (not "playable" until a full-campaign pass is
     established, D22);
   - `targets` ["Linux"]; add "Windows" only once D24 delivers it;
   - `project` name (for example "Duke Nukem: Time to Kill Recompiled"; user
     choice), `approach` one line (static recompilation of the PS1
     executable and overlays; faithful Vanilla plus an optional Modernized
     profile), `maintainers` MusicMonsterMod with `https://github.com/MusicMonsterMod`;
   - `links`: game manual and job board in this repo; optional `help`
     (for example controller support, Windows build) and `notes`
     (600 characters max).
3. Validate the file against the schema locally (for example the `jsonschema`
   package in `.venv`), run `scripts/ci/check_repo.py`, and check
   `.gitignore` does not exclude it.
4. With the user's closeout authorization, commit and push as
   MusicMonsterMod to `main`. Optionally submit the manifest form so the row
   updates at once instead of at the next daily read.
5. Optional: mention the listing in `README.md`.

**Acceptance:** the user approves the file content; it is committed and
pushed as MusicMonsterMod (only when the user authorizes the closeout); the
board row and game page show **claimed**, the maintainer, our status,
toolchain and targets, and Works from shows only SLUS-00583 (USA). Keep the
file current when status or targets change (for example after D22, D24 or
D25).

### D31 - Eye-catching README (hero banner, icon, social preview, art credits)

**Todo, design locked (2026-10-20). Opened 2026-10-20 at the user's request;
reworked the same day to an own-assets approach (user: "i'd rather go legit
about this").** Project presentation, not gameplay. No game code changes.

**Locked design (user, 2026-10-20: "this is the design but it's going to
remain in the backlog").** Concept version 7 of the artifact below is the
approved README; a saved copy is
`research/artwork/readme-drafts/readme-concept-7.html`. It fixes:

- Hero: our LA capture with Duke facing the camera firing, the "KaBoom"
  explosion behind him, Anton "DUKE NUKEM: TIME TO KILL" / gold
  "RECOMPILED" (weathered: hard chisel bevel, flat faces, grungy pitting,
  dents and rust, concept 9) / tagline;
  `readme-hero.jpg` cut from `hero16-grit.png` at
  (0,62)-(1707,773), 1600x666; `social-preview.jpg` 1280x640.
- Icon: the in-engine Photon portrait (`ttk-recomp-icon.psd`,
  `icon-ingame-*.png`) for the README; the game executable and window use
  it through D32.
- Vanilla vs Modernized comparison and table; screenshots (third person LA,
  Wild West, first person at the Time-Space Warp, first person Wild West
  street); the "First person: gadgets and power-ups" set (`shots/fp-*.jpg`:
  steroids, `[ / ]` switcher, Double Duke, invincibility plus Double Duke,
  night vision, mission items); the eras strip with Rome and Dark Ages
  brightened and captioned as such; quick start, credits and disclaimer.

**Asset rules (user decision 2026-10-20):**

- Commit no commercial key art: no poster, no edits of it, no Wikipedia logo
  (its page says the trademark exception does not extend to reuse), no
  advert slogans. They stay in `research/artwork/` for reference only.
- Name the game in plain text in our own type; never imitate the official logo.
- Screenshots: a few of our own captures from this build, captioned with the
  feature they show; no FMV frames, box art or menu art; compressed, modest
  resolution. Link videos rather than hosting them.
- Extras only when public domain or open licence (U.S. government photos,
  SIL OFL fonts), always credited.
- Disclaimer: unofficial, not affiliated (Gearbox, 3D Realms, n-Space, GT
  Interactive, Sony); images removed on a rights holder's request.
- Icon: our own in-engine portrait composited in Photon (user, 2026-10-20:
  "we can then safely say we own this as an icon"). The poster-cut icon is
  retired to `research/`. Duke's character and likeness remain the trademark
  owner's, as in every screenshot; the composite itself is our work.

**Concept 2 (2026-10-20).** Private artifact
https://claude.ai/artifact/GodtFaGHo1o8mztw3hhs1a (version 2): GitHub-dark
README mock, the rules, the asset kit and open questions. Local only (all
under ignored `research/` and `recomp/analysis/`):

- `research/artwork/d31-photon/`: `build_hero.py` and `ph.py` (headless
  Photon build: our capture, HUD removed by content-aware fill, the "KaBoom"
  explosion masked behind Duke with a linear-dodge glow, shading, live Anton
  type with gradient/stroke/glow), `ttk-recomp-hero.psd` (layered master),
  `readme-hero.jpg` (1600x666, 164 KB), `social-preview.jpg` (1280x640),
  `icon-poster-kept.jpg` (retired), `build_icon.py` and `ttk-recomp-icon.psd`
  (in-engine icon: low-angle capture cut out with `layer.removeBackground`
  and `layer.mask.apply`, fire rim light, two "KaBoom" bursts, clouds-filter
  sky, smoke, vignette), `icon-ingame-1024/512/256/128/64/32.png`, `shots/` (1280x720
  gameplay set, `vanilla-vs-modernized.jpg`, `eras.jpg` costume strip).
- `research/artwork/stock/`: public-domain photos with `sources.txt`
  ("KaBoom" Flickr 8248032542, U.S. Marine Corps; USAF 080620-F-9876D-295,
  Staff Sgt. Patrick Dixon, unused).
- `research/artwork/fonts-ofl/`: Anton and Black Ops One with their OFL
  texts (also copied to `~/.local/share/fonts` so Photon can use them).
- `recomp/analysis/d31-readme/`: capture lab (`shot.py`, `ctl.py`,
  `survey.py`, private profile, card and savestate copies, port 9371).
  Offscreen real GPU, `screenshot` gives the 4x internal render (2728x960,
  scale to 16:9); `present_shot` is only the 1024x576 window. Facing-camera
  portraits: `run_shot.py` (private launcher copy) honours
  `D31_WEAPON_AIM=none` so Modernized stops turning Duke with the camera;
  `front(off, pitch)` orbits to his front; Fire (bound to Q) shoots toward
  the lens. Levels via debug `level N`, `cheat=dnkroz` for the shoot.

**Work:**

1. Design approved (concept 7); no further picks needed.
2. Export the locked banner, social preview, screenshots and icon sizes
   (1024 master, 512/256/64) as they stand in concept 7.
3. Commit only final exports to `.github/readme/` (about 1.3 MB total),
   with an explicit allowlist for that folder in `scripts/ci/check_repo.py`;
   masters and sources stay under `research/`.
4. Rewrite the README top: banner, one-line pitch, badges, nav row,
   no-game-data note, Vanilla vs Modernized comparison and table, a few
   captioned screenshots, the eras strip, quick start, credits (images,
   explosion photo, Anton OFL, icon) and the disclaimer.
5. The user uploads the social preview in GitHub Settings.
6. Check rendering on github.com in light and dark and at phone width.

**Open questions:** final asset folder (`.github/readme/` proposed); a short
Modernized GIF or a link to a user video. Settled: hero frame (the LA shot),
icon role (README now, executable and window in D32).

**Acceptance:** the user approves the images and README text; it renders
well on GitHub in light and dark and on a phone; every image is our capture
or public domain/open licence and credited;
`scripts/ci/check_repo.py` passes.

### D32 - Game icon from the D31 in-engine portrait

**Todo. Opened 2026-10-20 at the user's request** ("this will use the
in-engine icon also to replace that psxrecomp.ico etc, and that's going to be
part of a separate job"). Depends on the locked D31 icon design
(`research/artwork/d31-photon/ttk-recomp-icon.psd`, `icon-ingame-*.png`).

**Today:** `recomp/assets/psxrecomp.ico` (6 sizes up to 256x256),
`psxrecomp.png` and `psxrecomp.svg` are the framework's default pad icon.
`recomp/CMakeLists.txt` passes `APP_ICON .../assets/psxrecomp.ico`;
`psxrecomp_add_game_runtime` (`recomp/psxrecomp/runtime/runtime.cmake`)
embeds it on Windows through an `.rc` and stages `psxrecomp.png` beside the
executable (`assets/psxrecomp.png`), which `psx_window_icon.cpp` loads for
`SDL_SetWindowIcon` on Linux, Windows and macOS. The recomp-ui launcher
reads the host's `window_icon_path` (`recomp-ui/src/recomp_launcher.h`).

**Work:**

1. From the 1024 master, export a multi-size `.ico` (16, 24, 32, 48, 64,
   128, 256; PNG-compressed 256) and a 256 or 512 PNG; consider a tighter
   head crop for 16-48 px so the face reads, per D31's note.
2. Replace `recomp/assets/psxrecomp.ico` and `psxrecomp.png` in our tree
   (keep the file names, or rename and update `APP_ICON`); the framework's
   own copy in the `recomp/psxrecomp` submodule stays untouched. Decide the
   `.svg` (drop it or replace it with a PNG-based fallback; a raster
   portrait has no honest SVG form).
3. Check Linux desktop integration if any (`.desktop`/AppImage icon) and the
   recomp-ui launcher window.
4. Build; confirm the window/taskbar icon on Linux, and the embedded PE icon
   on a Windows build when one is available.
5. Credit the icon (our capture and Photon composite, public-domain
   "KaBoom" photo) where D31 credits it.

**Acceptance:** the game window, taskbar and launcher show the new icon at
small and large sizes, the Windows executable embeds it, the icon files are
committed only as our own exports (no poster-derived art), and
`scripts/ci/check_repo.py` passes.

### D33 - Review: Modernized by default, Vanilla behind a flag

**Done (user-accepted 2026-10-20).** Opened 2026-10-20 at the user's request
("modernized btw, should be the default profile of the game. --vanilla i
think should be the special flag ... dont take action now"); selected the
same day. Decided (user): plain launch always Modernized; `--vanilla` one
launch, not saved; `--mode` removed; in-game PROFILE row removed; a saved
Vanilla moves to Modernized once with a notice. Implemented and verified, see
[note 158](documentation/158-d33-default-profile.md); executable
`fa415f5e183167b3b8a64b54959a9d3333f247a823527c7451553e2445d3dff2`.

**Found when opening (read only, not yet verified by a run):**

- `recomp/tools/local/player_profiles.py` `defaults()` already sets
  `active: modernized`, so a new settings file starts Modernized; an
  unreadable file restores Modernized defaults. The version 0 migration
  falls back to `vanilla` when the old file has no mode.
- `run.py --mode vanilla|modernized` selects **and remembers** the profile,
  so one `--mode vanilla` launch keeps later plain launches in Vanilla. There
  is no `--vanilla` / `--modernized` flag.
- Wording still frames Modernized as optional: `AGENTS.md` ("Vanilla stays
  available. Modernized is an optional profile."), the
  `continue-duke-recomp` skills (Claude Code and Codex), `GAME_MANUAL.md`
  (`--mode` examples, line ~380), `README.md` and `documentation/` notes.

**Review questions:**

1. Should a plain launch always mean Modernized, or the remembered profile
   (today's behaviour)? Should `--vanilla` be one-shot (not saved) or
   saved like `--mode`?
2. Keep `--mode` as an alias, deprecate it, or remove it; does a matching
   `--modernized` exist for symmetry?
3. The `--settings` menu and the in-game/recomp-ui launcher: which profile
   they open on and how Vanilla is picked there.
4. Existing players whose saved `active` is `vanilla`: leave it, or move
   them to Modernized once with a notice.
5. Version 0 migration default (`vanilla` today).
6. Tests and diagnostics that assume `--mode` (`tests/local`, analysis
   scripts, the debug ports, `DNTTK_*` env passing) and CI.
7. Docs and rules: AGENTS.md and the skills' "Modernized is optional"
   wording needs the user's approval to change; Vanilla must still be fully
   available and untouched in behaviour.

**Acceptance:** the user approves the decided behaviour; launcher, profiles,
tests and docs agree with it; Vanilla remains reachable and unchanged in
play; `scripts/ci/check_repo.py` and the launcher tests pass.

### D34 - Multiplayer investigation: modern split-screen, online play and self-hosting

**Done (research accepted 2026-10-20: "amazing read").** Follow-ups D34A-D34G
stay proposed in note 167 for a later session. Opened 2026-10-20 at the
user's request. Research and planning only: no gameplay, engine, framework or
build change. Research note:
[note 167](documentation/167-d34-multiplayer-research.md). Shareable page:
[TTK Multiplayer Research (D34)](https://claude.ai/code/artifact/07cd1540-1c0c-43ef-8d1b-d4b5428ba81f).

**Problem (user playtest 2026-10-20, first time in two-player):** TWO
PLAYER, MESA, Red Duke and Blue Duke, START: the loading spinner stops while
music plays; Enter opens a pause/options screen; the split screen flickers on
the left and right; after RESUME the controls are the original legacy scheme
and the frame rate is about 30-60 instead of 120. Leaving goes through the
original results screen and back out through the two-player setup pages. The
whole flow is disconnected from the modern experience.

**Vision:** keep the original local two-player game, modernised (controllers,
independent movement / camera / aim per player, our renderer, smooth
pacing), and build towards online play where each player has a full-screen
view on their own machine, with self-hosted matches (LAN, direct IP, a
self-hostable server), no central service or accounts. More than two players
only if the engine allows it.

**Scope:**

1. How the original two-player mode works (simulation, players, input,
   cameras, viewports, rendering, arenas, characters, rules) and why it
   bypasses the modern input, rendering and frame rate.
2. Modern local split-screen options.
3. Online feasibility: architecture, replication or synchronisation,
   prediction, latency, combat, match lifecycle, timing changes, player
   count.
4. Hosting, connectivity, security and trust model.
5. Technology comparison (including the framework's own netplay libraries).
6. A phased roadmap and the smallest proof of concept.

**Deliverables:** this job entry and note 167 (sections 0-12), with
verified findings, hypotheses and recommendations kept apart and source
references.

**Key findings (see the note for evidence):**

- One shared simulation with two players (`0x800d7198`, stride `0x8a4`) and
  two cameras (`0x800d6eb0`, stride `0xb0`). The composition `0x80026164`
  draws one player's view per game frame and alternates (`0x800d2654`).
  Verified from the executable.
- Our build has `MAX_PLAYERS 1` (no pad on port 2). Modern controls are bound
  to player 1. The arenas (levels 15-20) are outside the control identity
  guard. D17 replay captures gameplay only for one player
  (`frame_replay.cpp:867`). Together these explain the legacy controls and
  the frame rate. The frozen spinner and the flicker are probably the same
  family (hypotheses to prove in Phase A).
- psxrecomp already ships MIT-licensed input-sync netplay (`recomp-net`,
  `retcomm-rbengine`: delay-sync, rollback, LAN, direct IP, ICE, digests). It
  is disabled in our build and has no encryption.
- Recommendation: deterministic input-synchronised play of the game's own
  two-player mode, with per-peer presentation through D17 replay (own view
  full screen). Modern controls become a synced per-tick intent record. Use a
  self-hostable input relay and authenticated encryption for internet
  matches. More than two players is not feasible on this route without major
  engine work.
- Proposed follow-ups D34A-D34G (not yet on the board): discovery runs, two
  input seats, modern split-screen, determinism and LAN prototype, online
  gameplay, hosting and security, expansion study.

**Acceptance:** the user reviews note 167 and either accepts it or asks for
changes. Then the user chooses which follow-up jobs to open. No
implementation starts under D34.

**Decided (user, 2026-10-20):** online and LAN play are full screen, each
player's own view on their own machine, never split screen. Split screen is
for local play only.

### D34B - Classic split-screen with two controllers

**Closed for now (user request 2026-10-27; physical-controller acceptance deferred to D09).** Opened 2026-10-20 at the user's request: "fix the split screen
multiplayer so it can work just oldskool with 2 controllers and thats it,
nothing particularly fancy there yet". The local, no-frills first step of
[note 167](documentation/167-d34-multiplayer-research.md) phase B1. Modern
split-screen (per-player modern controls, high refresh per viewport) stays
in the proposed D34C; online and LAN play (full screen per player) stay in
D34D onwards.

**Problem (user playtest 2026-10-20, causes in note 167 section 2):**
player 2 has no input (`MAX_PLAYERS 1`: no pad on port 2); the loading
spinner appears to freeze while the arena already runs; Enter opens a pause
screen; the split screen flickers left and right; the frame rate is about
30-60; exit walks back through the setup pages.

**Scope:**

1. Two input seats: build with `MAX_PLAYERS 2`; each SDL controller drives
   its own PS1 port with the original controls. Keyboard and mouse may be
   player 1 with one controller as player 2, or two controllers; decide the
   routing and document it.
2. CHOOSE CHARACTER read from each player's own pad, as the original does
   (replacing D19J's one-keyboard workaround in two-player only).
3. In a two-player match the modern layer steps aside cleanly: original
   controls for both players, original presentation (no stale replay or
   loading image), widescreen off or a correct 4:3 picture, no flicker
   caused by our presentation.
4. Start, pause (either player), results screen and exit return cleanly to
   our menus.
5. Check first whether `MAX_PLAYERS 2` changes the savestate format; if it
   would reject existing savestates, stop and ask the user before going on.

**Out of scope:** per-player modern controls, high refresh in two-player,
gamepad modern controls (D09), online or LAN play.

**Acceptance:** two controllers play a full two-player match (character
choice, arena, kills, respawn, time or kill limit, results, exit) with no
frozen loading image and no flicker from our presentation. Single-player
Modernized and Vanilla unchanged (regression checks as for other jobs);
existing savestates still load, or the user approved the change. The user
playtests on the real display.

**Work log (2026-10-27, Needs playtest).** Two independent SDL pad seats and
character choices, native 4:3 split presentation, single-player replay/capture
isolation, and results Quit to main menu implemented. Old one-seat SIO snapshots
load without migration; missing P2 starts neutral. Build, SIO compatibility,
input, controls and aim tests pass. Private virtual SDL routes cover both split
orientations, movement isolation, kill/respawn, pause from either pad, save/load
and a natural five-minute match. Campaign Modernized/Vanilla smoke checks pass.
Physical controllers and real-display acceptance remain; no pre-change visual
A/B or broad arena coverage claimed. See [note 288](documentation/288-d34b-classic-split-screen.md).
No commit/push or acceptance closeout performed.

**Work log (2026-10-27, Closed for now at user request).** User reports stuck
Red Duke / Blue Duke selection and explicitly requests no code changes and
closeout. Retain the existing implementation; carry physical-controller
selection, full-match and real-display acceptance into D09. Cause undiagnosed;
no claim of full acceptance. Existing authored implementation/tests and docs
are authorized for commit/push as MusicMonsterMod. No next job started.

### D34C - Modern local split-screen controls and presentation

**Status: Closed for now (2026-10-28).** Current original-style two-player
baseline and ownership foundation accepted; modern gameplay/presentation below
remain deferred, not Done. See [note 291](documentation/291-d34c-local-multiplayer-foundation.md).
Dependencies:
D09 and D09A (Done), retained D34B implementation. This opens the existing D34C
proposal from [note 167](documentation/167-d34-multiplayer-research.md), using
accepted D09A as the input-seat foundation. User confirms the current mode works
but is fundamental; the next step is modernization, not another seat-routing job.

**Scope and implementation order:**

1. Audit current single-player ownership in input/actions, movement, cameras,
   weapon aiming, traversal, HUD and replay. Reverify arena identity and active
   player/camera addresses against the owned executable and runtime. Old research
   assumptions are a starting point, not proof that current hooks cover arenas.
2. Give each player independent movement, camera and weapon-aim state. Keyboard/
   mouse P1 uses camera-relative movement and mouse look; gamepad P2 uses left
   stick movement and right stick look/aim. Support two gamepads with the same
   per-player independence. Start with third person, including camera collision
   and recenter; do not call keyboard remapping alone modern controls. Scope
   first-person presentation separately after the third-person foundation works.
3. Extend applicable Modernized actions to both actors without sharing toggles,
   weapon selection, aim targets, camera settings or traversal leases. Keep
   original match scoring, arena rules, spawning, damage and time/kill limits.
   Retain an explicit classic local mode and unchanged Vanilla/single-player play.
4. Modernize presentation per viewport: independent camera/render history,
   correctly owned HUD/reticle and aspect handling, clean loading/pause/results
   transitions, and both horizontal and vertical layouts. Target 120 Hz player
   presentation using measured frame budgets; report simulation cadence and each
   viewport's actual updates separately. A 60 Hz fallback must be explicit and
   measured, not presented as 120 Hz acceptance.
5. Preserve clear seat assignment/confirmation, device-loss handling, mouse
   capture/focus behavior, save/load, rematch and exit. Existing textual device
   labels can support this job; full live prompt-icon switching stays in D19F1
   and is not a prerequisite for the control implementation.

**Acceptance:** both players simultaneously move, look and fire independently;
one player's camera/actions never steer the other. Verify keyboard/mouse plus
physical gamepad and two physical gamepads, including disconnect/reconnect and
focus changes. Play complete matches through kills, death/respawn, results,
rematch and exit; test private save/load and both split orientations. Cover all
six original arenas for hook eligibility and basic play, with representative
collision/traversal/aim routes. Capture full-display evidence and measure both
viewports at 60/120 presentation, including busy combat, with no frozen loading
image or presentation-induced flicker. Regress accepted campaign controls,
classic split-screen and Vanilla. Record unsupported cases and performance
limits explicitly; require user acceptance before Done.

**Out of scope:** online/LAN play, servers, more than two players, new arenas,
new match rules and a first-person body/weapon art overhaul. Those remain separate
jobs. No gameplay edits, build or game launch authorized merely by opening this
entry; implement only after the user selects D34C.

### D58 - Split the job board into category files

**Status: Done (user-accepted 2026-11-10: "lock it in and close it out").** Dependencies: none.

`MODERNIZATION_JOBS.md` reached the 2 MiB per-file limit of
`scripts/ci/check_repo.py` (a guard against binaries such as disc dumps).
About 730 KB is the dated session log, about 1.2 MB job descriptions and work
logs (D08 alone about 430 KB), about 70 KB the job table. User: split it per
category. Scope:

- `MODERNIZATION_JOBS.md` keeps the vision, the full job table (every ID and
  status, each row linking to its category file) and the working rules.
- `jobs/<category>.md` holds the job descriptions and their work logs,
  moved verbatim. Proposed categories (confirm with the user first):
  controls (D03-D10, D27, D28), first-person (D11, D12, D35, D36, D47, D48),
  weapons and combat (D39, D44, D49-D51), graphics (D13-D17, D40, D46, D52,
  D53), lighting (D41 and children, D54-D57), menus and UI (D19-D21, D26,
  D29, D42, D43), campaign and performance (D18, D22, D23, D37, D45), release
  and project (D01, D02, D24, D25, D30-D34). Consider D08 traversal on its own.
- The dated session log goes to `jobs/log-YYYY-MM.md` per month; fold in
  `documentation/jobs-history-2026-09.md` and `jobs-history-2026-10-01.md`.
- Update the `continue-duke-recomp` skill (Claude and Codex copies),
  AGENTS.md and README pointers. Links from `documentation/` notes to the
  board stay valid.
- A check script: every table ID has exactly one description, no text lost
  (content compared before and after), every file under the limit.

Acceptance: the board and the category files read as before, the skill lists
jobs from the table, `check_repo.py` passes, user review.

#### 2026-11-10 - Split done (Needs review)

Categories confirmed by the user: the ticket's list with D08 split into
`traversal` (D08 without D08A) and `items` (D08A and its children), D14 in
first person, D38 in weapons, D50 in performance. The user also chose to move
the 2026-09-25 to 09-27 work log out of the working rules into the September
log, and to keep undated intake sections with their category file.

- `MODERNIZATION_JOBS.md` (116 KB, was 2.09 MB) keeps the title, a list of
  the category and log files, the vision, the job table and the working rules
  with the work log format. Every table ID links to its category file, at the
  anchor of its description where one exists (509 of 547).
- `jobs/<category>.md`: 10 files, 31-309 KB. Each holds its intake sections
  (`## Background` or the original `##` intake headings) and then `## Jobs`,
  one block per job in ID order (D17, D17A ... D17Z, D17AA ...). A job with
  several blocks keeps its description first, then the others in board order
  (`D08G9 update`, `D08K9 follow-up`). Blocks are verbatim; the `## D17AA`
  description became `### D17AA`.
- `jobs/log-2026-09.md`, `log-2026-10.md`, `log-2026-11.md` (154, 507 and
  249 KB): every dated `##` entry from the top and bottom of the board plus
  both history files (now deleted), newest first. Job descriptions that sat
  inside dated intake entries moved to their category file; the rest of each
  entry stayed in the log. Work-log subheadings such as `### D07C
  implementation - 2026-09-27` stayed in the log.
- Dropped: the empty headings `Foundation and controls`, `Graphics and
  playback`, `Player features and release work`, three intake headings with
  no text left after their jobs moved, and the two "session log moved" stubs.
- 21 jobs had a description but no table row (D38, D35I1, D36H4-H7, D36J1-J5,
  D36K1, D14D, D19V, D19W, D09B, D17AB-AD, D08A31, PRIV01). They now have rows
  with the status and dependencies from their description. D19W (separate
  keyboard and gamepad bindings pages) is Todo and was missing from job
  listings because of this.
- 38 table rows have no description of their own (for example D17AM1-AM4,
  D39A-G, D44A-C): their work is described under the parent job or only in
  the session log. They link to the category file without an anchor.
- 17 anchor links in `documentation/` notes pointed into the old board; they
  now point to the category files. Plain links to `MODERNIZATION_JOBS.md`
  stay valid.
- New `scripts/ci/check_jobs.py`: unique table IDs, every row linked to the
  file of its category and to an existing anchor, every description with a
  row and in exactly one file (the right one), only dated entries in the logs,
  every file under 2 MiB. `--against REV` compares with the single-file board
  and history files at REV: every non-blank old line must still be present,
  apart from a fixed list of dropped headings and stubs.
- Updated the `continue-duke-recomp` skill (Claude and Codex copies; the
  Codex copy is local, outside the repo), AGENTS.md, README.md and the board's
  working rules: descriptions and work logs go to the category file, new
  dated entries to the top of the month's log file, and a new job family
  needs a category in `check_jobs.py`.

Evidence: `check_jobs.py --against HEAD` (HEAD 4019fce) reports two lines
only, both edited on purpose: the D58 status line and the board's intro
sentence that pointed to "the job's work log below". Of 639 added non-blank lines, 547
are the linked table rows, the rest the file headers and this status.
`check_repo.py` passes.

### PRIV01 - Remove local usernames from repository content

**Status: Done (user-accepted; closeout, commit and push authorized 2026-10-27).**
Scope: scrub personal home paths from all tracked content, including reports
and embedded asset metadata; update Codex, Claude and Cursor guidance; enforce
portable paths in repository hygiene checks. Acceptance: no local username
matches in tracked working-tree bytes, valid reports/assets, and a passing
privacy guard. User chose to retain existing Git history after reviewing the
limited privacy risk of a local username; no history rewrite is requested.


**Latest closeout (2026-10-26): D36H2 is Done. User accepted both Buffalo grip fixes and explicitly authorized lock-in/closeout. [Note 277](documentation/277-d36h2-buffalo-grip.md). No next job started.**

**Latest closeout (2026-10-26): D07G is Done. User accepted crosshair readability and authorized lock-in/closeout. [Note 276](documentation/276-d07g-crosshair-scaling.md). No next job started.**

**Latest closeout (2026-10-26): D35J5 is Done. User accepted frozen first-person blue vision and explicitly authorized lock-in/closeout. [Note 275](documentation/275-d35j5-frozen-first-person.md) retains verification limits. No next job started.**

### R01 - DisruptorRecomp architecture and modernization reference research

Research-only job selected 2026-10-04. Inspect a pinned, read-only upstream
checkout under ignored `research/`; compare geometry precision, gap handling,
perspective textures, sky, interpolation, input latency, CPU scheduling,
widescreen and supporting systems with current TTK. No source import, gameplay
edit, build, launch or replacement of accepted D17/D17B/D17O work.

Acceptance: record acquisition/revisions and ignore verification; produce a
source/function-cited architecture review with applicability classifications,
backlog mapping, licensing/provenance and ranked bounded experiments; recommend
whether to retain the checkout and identify the top three experiments.

Work log (2026-10-04, In progress): verified `/research/` ignore rule before
cloning; existing psxrecomp working-tree modifications are preserved.

Work log (2026-10-04, Done): completed the [source-cited comparative review](documentation/93-disruptor-reference-research.md)
at Disruptor revision `408f214d3109dbc6cbdded7edd128cbf8de6466a` and framework
`193a60b805e1eaa853129d6ccf63022440d4b143`. Retained the clean reference under
ignored `research/DisruptorRecomp`; verified real-file/nested ignore behavior
and no tracked research files. Findings distinguish image blending from TTK
worker redraws, gap-covering rims from culling/depth repairs, panorama sky from
TTK sky meshes, and guest overclock from host execution cost. Top experiments:
primitive/provenance census, optional D15 geometry/texture precision, then
bounded timing-overhead attribution. Recorded license text discrepancy before
any future source adaptation. No code import, build, launch, gameplay/renderer
change, media/save/card/preferences write or new implementation job. Existing
framework modifications preserved. Research acceptance met by static review;
upstream performance claims were not independently benchmarked.

User direction (2026-10-04): retain Disruptor as research only. Learn techniques
and write purpose-built TTK implementations; no 1:1 copying or mechanical
translation of third-party source. This direction does not start an experiment.
