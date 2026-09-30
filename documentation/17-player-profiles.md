# D02 — Persistent launch profiles

The local `run.py` launcher owns a versioned JSON file at
`recomp/config/player-profiles.json`. Modernized is the initial selection for new
settings; existing saved mode choices remain intact. Both profiles retain their
own preferences, and Vanilla keeps the original gameplay/input path.

The current format is **schema 7**. Migration backs up old bytes, moves the old
default crouch V to Left Ctrl, and adds named weapon/item actions. Custom bindings
win when defaults conflict; new actions receive unused keys. Earlier H→C holster,
interaction, aiming and renderer migrations remain in place. See
[current controls and migration details](33-controls-shortcuts.md) and the
[manual](../GAME_MANUAL.md). Earlier schema milestones below are historical.

**Schema 17 (D08T1, 2026-09-30)** is the current launcher format: it adds the
`grab` (Mouse2) and `grab_alt` (Alt) actions and moves Mouse2 off
`original_aim` (now Unbound by default); a customized precision-aim input is kept.

**Schema 16 (D07D, 2026-09-30)**: Modernized
`red_dot` defaults to off. A pre-16 file whose Modernized profile holds
`red_dot: true` is switched off once (backup plus notice), because older files
cannot tell a chosen "on" from the old default; from 16 on the stored value is
the player's choice. Vanilla always launches with the original dot.

## Player entry points

```sh
python3 recomp/tools/local/run.py --settings
python3 recomp/tools/local/run.py --mode vanilla
python3 recomp/tools/local/run.py --mode modernized
python3 recomp/tools/local/run.py --show-settings
python3 recomp/tools/local/run.py --reset-profile modernized
```

The terminal settings menu selects profiles/renderers and restores defaults.
Save and return exits without launching; a subsequent ordinary `run.py` uses the
saved choice. Cancel abandons menu edits. Command-line selection and renderer
changes persist. `--show-settings` combines with change flags for scripted edits
without launching. Reset restores only the named profile's preferences and does
not change the active selection or the other profile. Both renderer defaults
are OpenGL, retaining the existing launcher default. Changes take effect at the
next process launch; there is no mid-game profile switch.

Memory cards retain the existing `saves/local-play` location unless explicitly
overridden. Switching profiles does not copy, delete or rewrite cards. Existing runtime keyboard/controller settings remain shared for Vanilla and
physical controllers; Modernized PC action bindings are separate. Display options other than renderer are not controlled
by these profiles yet. Runtime `settings.toml` remains untouched. The explicit
`--renderer` argument is applied after the runtime loads TOML, so its value is
not silently overridden by next-to-executable preferences.

## File and recovery contract

Schema 2 contains `version`, `active`, and `profiles`, with each profile's
`presentation.renderer` and `bindings`. Only implemented fields are accepted. Invalid fields
fall back to defaults while valid preferences in the other profile survive.
Malformed files are copied verbatim to a uniquely named `.recovered-*` file
before defaults are written. All writes use a temporary file in the destination
directory, flush/fsync and atomic replacement; a failed replace leaves the
previous settings intact. I/O errors stop launch rather than pretending to save.
Future schema versions fail closed without changing the file.

Version 0 is a documented migration/interchange format with `mode` and `renderer`
at the top level. Its selected profile retains that renderer on migration to
schema 2. Version 1 migrates while preserving selection/renderers and adding
action defaults; original bytes are backed up. It is not a claim that an older shipped profile feature existed, and
no upstream runtime TOML is imported or rewritten. `--settings-file` allows
isolated tests; personal settings and recovery copies are ignored by Git.

## Verification

`python3 -m unittest discover -s recomp/tests/local -p test_player_profiles.py -v`
checks nine cases: defaults/independence, real CLI process restart and switching,
per-profile reset, malformed recovery and byte-preserved backups, partial-field
recovery, legacy migration, future-version refusal, atomic-write failure,
terminal save/reset/cancel, and launcher argv/card isolation (some cases combine
related checks). Runtime replay results are recorded in the work log/report.

Use `vanilla_regression.py --name UNIQUE --mode modernized --renderer software`
to exercise the actual launcher with fresh cards and an isolated profile file.
The report retains the selected profile and captures; it does not use personal
preferences. Native Windows remains untested.
