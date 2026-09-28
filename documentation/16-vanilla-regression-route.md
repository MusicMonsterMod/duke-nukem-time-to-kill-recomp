# Vanilla regression route — D01

This route targets the local US SLUS-00583 Linux candidate, original controls,
4:3 presentation, OpenBIOS and native movie cache. It is a baseline for later
profile/control changes, not campaign acceptance. The automated section records
inputs and images; **a successful driver exit does not assert gameplay success**.

## Reproduce the bounded input check

Close any existing game yourself before starting. From the workspace root:

```sh
python3 recomp/tools/local/vanilla_regression.py --name baseline-YYYYMMDD-01
```

Use a new name on every run. The runner refuses an existing run directory or
running game and checks that its loopback port is available. It launches only
its own child, with fresh cards at
`recomp/analysis/vanilla-regression/<name>/cards`, and records in that same run
directory. It never copies the player's cards. This Linux-only runner uses the
existing candidate without rebuilding or modifying its configuration.

`report.json` records binary/configuration/cache/BIOS/patch hashes, repository
heads and dirty status, relevant inherited environment variables, launch argv,
input button words, actual guest frames and wall times, diagnostics, card hashes
and process exit. `runtime.log` and PNG checkpoints remain local under ignored
`analysis/`. Effective next-to-executable TOML/INI files are hashed too; compare
them before calling two runs equivalent. Headless execution does not measure
real-time playback, audible output, controller hardware or window behavior.

The runner waits for guest frame 900, captures the intro, presses Start for 12
frames to skip it, and allows 6000 guest frames for the remaining startup/title
sequence. It then performs this sequence. Waits are measured from the request's
starting frame; the report records actual overshoot. Timing is specific to this
candidate; if scene review fails, stop treating later captures as gameplay.

| Checkpoint | Input / hold / wait (guest frames) | Required visual review |
| --- | --- | --- |
| main-menu | Start / 12 / 120 | One Player selected; not PRESS START attract footage |
| difficulty | Cross / 12 / 120 | GET SOME selected |
| spawn | Cross / 12 / 900 | Duke in starting park, health 100, pistol ammo 200 |
| fire | Cross / 60 / 65 | Shot animation and ammunition reduction; pistol starts drawn |
| holster | Circle / 12 / 60 | Weapon holstered |
| draw | Circle / 12 / 60 | Pistol visibly drawn again |
| jump | Square / 12 / 35 | Duke above ground; distinguish takeoff/landing from crouch |
| land | Released / 1 / 120 | Back on ground |
| inventory | Select / 12 / 60 | Weapons inventory opens |
| inventory-close | Select / 12 / 60 | Inventory closes |
| forward | Up / 45 / 60 | Duke advances toward street |
| turn | Right / 30 / 45 | Facing/view changes |

Every press is active-low PS1 input with a bounded duration. No guest memory
writes, cheats or synthetic pause/step are used. A debug `quit` is followed by
waiting for process exit 0. The server may return an error or close its response
while exiting; process status is the shutdown evidence. This does not replace
the window-close test below.

The first two timing experiments are retained as failed semantic routes:
`d01-baseline-01` reached attract mode and `d01-baseline-02` issued Start while
startup was still finishing. Neither counts as a passing regression route.

## Extended progression, sound, save and window route

This section requires a displayed, audible playthrough and checkpoint review.
It is specified for repeat testing but is not yet fully verified in one recorded
run. Do not fill missing evidence with earlier reports.

Choose a new test directory, then launch:

```sh
python3 recomp/tools/local/run.py --diagnostics --debug-port 9153 --memcard-dir recomp/analysis/vanilla-manual-YYYYMMDD-01/cards
```

1. Let the opening movie play, recording wall duration, picture/audio sync and
   any underruns. Repeat in a second fresh run with Enter to skip. Confirm
   title music replaces the movie and menu navigation works. Select One Player
   and GET SOME with X. Keep the in-game controller layout at its default.
2. At the park, record a capture and HUD. Fire with X (use S only if holstered), jump with Z,
   open/close inventory with Right Shift, and move/turn with the arrows. Record
   damage from enemies and subsequent control response. Confirm speech finishes
   and music continues without an old voice sample repeating. Record exact
   observed words/times privately if needed; do not infer audible success from
   CD diagnostics.
3. For the security-card checkpoint, head toward the wrecked police car, turn
   left and descend into the subway. Defeat the guards and collect the blue
   card; check the guard at the locked end door if necessary. Face the reader,
   holster with S and use X. Capture inventory/card state before use, door state
   after use, and Duke moving beyond the doorway. This landmark route is based
   on [SPaul's level-one guide](https://gamefaqs.gamespot.com/ps/197177-duke-nukem-time-to-kill/faqs/3834),
   not a newly observed local replay. Input durations and completion time here
   remain to be recorded.
4. If the guest stops, retain the diagnostic process for inspection. Read
   [the existing investigation](15-security-key-and-window-close.md) before
   collecting its register/RAM/dispatch evidence. The old freeze's cause is
   unknown; a successful replay does not establish a repair mechanism.
5. Complete the level using the linked guide: restore subway power, activate
   the train from the ticket booth, then follow the cleared tunnel and collect
   the transporter-room key. Gather the crystals from the purple chain-platform
   route, green broken-bridge route and valve-controlled water route. Return
   to the transporter and use the crystals. These landmarks are guide-derived
   and remain unverified here. At the game's normal save offer, record completion
   time, save-menu choices, slot, filename and visible success. Close normally;
   hash only these test cards. Relaunch with exactly this test directory, choose
   Load Game and the saved slot, and verify the displayed continuation/level,
   inventory and functioning controls against the save checkpoint. Record
   before/after card hashes. Merely creating blank cards is not save/load proof.
6. Exercise death and restart in a separate fresh-card run so that the main
   progression/save comparison is not contaminated. Record the restart choice,
   destination and HUD rather than assuming original restart rules.
7. Close using the window × during gameplay and, in another run, during movie
   playback. Record process exit, elapsed close time and exit trace origin.
   `test_window_close.py` is the existing automated healthy/halted/idle-client
   window test; run it only when no player session is active.

At gameplay entry, before/after the card, and before closing, collect `frame`,
`audio_stats`, `cdrom_command_history`, `dispatch_stats` and
`overlay_loader_status` using `debug_client.py --port 9153 ... --output ...`.
Capture with `screenshot_file` to an absolute path inside the test directory.
Run `check_cd_seeks.py` on command history: zero checked seeks is inconclusive,
not a pass. Ring histories are bounded; collect near transitions, not only at
the end of a long level. Never use `pause` (unsupported by this runtime).

## Evidence boundaries

- Earlier user report: card inserted, next area reached, window × closed;
  exit origin `sdl_window_close`, guest frame 12144. This was not an input-recorded
  playthrough and supplies no save/load evidence.
- Earlier movie measurements: approximately 59.94 guest frames/s and no new
  underruns in two measured intro intervals; these do not establish 60 unique
  images/s or full movie coverage.
- New exploratory headless run: inventory visible, forward motion, turning,
  pistol ammo 200 → 195, health 100 → 92 and a jump capture. Exact cold-start
  reproduction is evaluated separately by the bounded runner.
- Still required: one recorded extended card route, audible transition check,
  death/restart, original save/load round trip and displayed-window integration
  of this route. No full campaign or Windows claim follows from D01.


## 2026-09-26 bounded replay results

Runs `d01-baseline-05` and `d01-baseline-06` both passed visual review of the
bounded input sequence. Their fresh-card spawn captures show health 100 and
ammo 200; firing reduces ammo to 195, enemy damage later reduces health to 70.
Holster/draw, airborne/landed poses, inventory open/close, movement and turning
are visible. Both exited with code 0 through debug quit. Exact checkpoint
frames and elapsed times are in [the summary](reports/d01-regression.json).
Runs 03/04 were partial checks: they reached gameplay, but the test holstered
the initially drawn pistol before attempting to fire. They do not pass firing.

The final CD history contains no eligible explicit seek/read pair, so the seek
checker is **inconclusive**, not a fresh voice-stream fix verification. Existing
21-transition seek evidence remains separate. Python compilation and Git
whitespace checks passed; no rebuild was needed because runtime/game code did
not change. The active-game guard was exercised and refused a second launch.
No test game remained running after the final replay.

D01 remains **Needs playtest** for the extended procedure above. The next
implementation job is D02 once that prerequisite is complete; the user has
already authorized continuing in order, so another job-selection prompt is
unnecessary.


## User acceptance — 2026-09-26

After the recorded probes, the user confirmed that playtesting showed the game
working to a very high Vanilla standard and authorized continuing. D01 is closed
on this user acceptance plus the repeatable route/evidence above. The user's
statement is an overall playtest confirmation, not a new instrumented result
for each item or proof of campaign/Windows completion. Historical Needs playtest
notes above describe the state before that confirmation.
