# D08P — Crystal-2 turret and scripted-camera control recovery

## Report

After the first subway crystal, the green broken-bridge route jumps a ledge,
wades through water, and meets a ceiling-mounted turret. Playtest after the
first tank-WASD fallback: the room felt a bit more stable, but Duke returned
to walking, mouse look died, A/D turned him instead of strafing, Escape then
resume dropped Modernized capture entirely, and the crosshair vanished. The
water “registered as quite deeply underwater” even though it is only a wade.

Iteration 2 (depth gate, L2/R2 tank A/D, Escape-from-player-update) did **not**
restore play. Reloading **F7 UI slot 2** (the turret-corner save) then left
Modernized capture dead: no mouse, no WASD/strafe, Duke not controlling
properly. The turret is around the corner from that spawn.

## Isolated dump (F7 UI slot 2, not a memory card)

Player cards were not written. F7 UI slot 2 is OpenBIOS file
`saves/local-play/openbios/state_800AB6FC_slot01.pst` (runtime slot index is
UI slot − 1). Isolated copy: `recomp/analysis/d08p-slot2/`.

At that spawn (flooded room, turret around the dark doorway):

| Field | Value |
| --- | --- |
| anim | **63** idle |
| `+0x22c/+0x22d` | **0/0** |
| `+0x224` | `0x2` (run bit). **Not** `0x20000000`, **not** `0x200` |
| `+0x20c` | `0x33000` (water flagged) |
| depth `(+0x1c8)−(+0x834)` | **256 = 0x100** (original shallow band) |
| camera `+0xa4` | player `0x800d7198` (normal camera) |
| alt camera `0x800d7010` | unused |
| LEVEL00 `0x800cc57c` / `0x800cc580` | already Modernized apartment pair `0x24020000` / `0x30a30400` |

Walking the copy: depth stayed **0x100**; D-pad Left/Right played **anims 71/70**
(tank turns). Anim 70 is also the original “water idle” clip. Overlay raw SHA
differs from unpatched LEVEL00 by those two apartment words only.

## Cause

Iteration 2 fixed the wade/swim mis-classification. Slot 2 still lost all
modern controls because **F7 load released the mouse and never asked for it
back**:

1. **F10 opts out of automatic capture** (`initial_capture=false`). Only
   Escape previously set it again. F7 (and F7 after an already-free mouse)
   left `initial_capture` false, so `0x8005a210` never offered recapture.
2. **`capture_offer.exchange(false)` ate offers** while the F7 menu held
   `allow_capture` false. A later offer could recover only if
   `initial_capture` was still true.
3. **Identity hashed the live apartment words.** A Modernized savestate
   already carries the patched pair. Identity now treats the complete
   original pair and the complete patched pair as the authenticated LEVEL00
   overlay; a half-patched pair still fails.

Earlier wade mismatches (kept):

1. Host free-swim gate was `0x200`; original is `0x281`. Anim 70 in this
   room is a tank turn, not underwater idle.
2. Tank fallback A/D was D-pad Left/Right (anims 71/70). Now L2/R2 strafe.
3. Escape recapture required walk-only `state()`. Now offered from
   `0x8005a210` whenever gameplay context and identity match.

`0x80025e80` still branches to `0x8003B4F4` / `0x800D7010` when
`+0x224 & 0x20000000`. That bit was **clear** on the dumped checkpoint.
Hit reactions during the gun fight can still drop `state()`; the tank
strafe fallback remains. This is not the old security-card freeze.

## Iteration 5 — live Xvfb/xdotool reproduction (binary `b642ab9f…`)

The user reported "exactly the same issues" on iteration 4. Real SDL input
(Xvfb + xdotool, software **and** OpenGL renderers, the user's own
`player-profiles.json`, debug `savestate` load **and** the real F7 UI
`F7 → 2 → l`) against the isolated slot 2 copy, `/tmp/d08p_live_probe*.py`:

| Check | Result |
| --- | --- |
| Auto-capture after F7 UI load | captured within ~1 s, `ready=1`, refusals 0 |
| Mouse yaw in the wade | 3758 → 449 for 600 px; body faces view |
| W / Shift+W in open water, 11 headings | **48–54 units/frame, anims 76/78** (Vanilla D-pad wade: 53) |
| Headings with a +x component from spawn | 13–30 units/frame, `x` frozen: the spawn is beside the east wall; the original collision slides along it. Facing the wall: idle |
| Leaving the water (SW grate, N barrel platform, W walkway) | lease kept; walls give 94/95 then idle with W held (same as original D-pad) |
| Dry ground W / Shift+W | 12.7 (walk 72/74) / 37.5–50 (run 76) |
| Escape → resume (held ≥3 frames) | recaptured; mouse yaw moves |
| Explored | flooded room x 37172–45850, z 100200–108500; S doorway → bridge corridor → 6500-unit drop into deep water (127, state 5); N barrel platform → W walkway dead end. No `0x20000000` camera, no depth ≥ `0x200` reached |

So the wade itself runs at dry-ground speed with mouse look. The one
reproducible defect matching "W only walks, Shift does nothing, after the
water": **`clear()` at every recapture forgot physically held keys.** Holding
Shift through F10/F7/Escape-resume, then pressing W: **72/74 at 10.7
units/frame with the Walk pad (L1) injected**; releasing and re-pressing
Shift: 78 at 48.7. A player who keeps the run modifier down across the F7
load or a pause sees exactly "walking only, Shift broken" until Shift is
re-pressed. Slow wading toward a corridor wall is the original slide.

Second defect found on the way: the Escape release offered recapture in the
same frame (gameplay still running for one update), so the mouse was
recaptured **inside the original pause menu**, where Enter/X are swallowed
by the inventory/cheat handling.

### Delivery (iteration 5)

- Capture (`F10`, automatic) now resyncs every bound gameplay key and mouse
  button from `SDL_GetKeyboardState` / `SDL_GetMouseState` after `clear()`.
  Fixed menu keys (Escape/Enter/arrows/F-keys) stay cleared.
- Capture offers carry the input sequence and must be ≤8 frames old. The
  player update re-offers every frame while gameplay runs, so a pause menu
  (no offers) cannot be captured; host overlays do not advance the sequence.
- Tank fallback that lasts ≥45 frames shows `ORIGINAL MOVEMENT (reason)`
  (reason from the last `[TTK lease]` refusal: `state`/`identity`/`context`/
  `released`) and `MODERN MOVEMENT RESUMED`; stderr carries the same lines.
- `run.py` mirrors runtime stderr into `recomp/build-local/logs/session-*.log`
  (last five; `--no-session-log`). stdout is untouched; nothing under `saves/`.
- Live checks after the change: Shift held through F10 recapture → 76 at
  50.6; Shift+W held through Escape/resume → not captured while paused,
  78 at 53.9 on resume; mouse yaw 914 → 1386 after resume.

## Iteration 6 — the whole area: overlay identity (binary `79a3fd5b…`)

The user was right that depth was not the story: from **F7 UI slot 3** (ledge
on the right of the flooded corridor) running forward lost run, mouse and
jump for good. The user's own `session-20260929-005059.log` showed it:
`[TTK lease] inactive (identity) … ident=0` from the moment Duke reached the
ledge and forever after, in every animation, including idle 63 back on land.

Reproduced on the isolated slot 3 copy while diffing **every guard range**
each 15 frames (`/tmp/d08p_live_probe17.py`): the only change was the
**last three words of the LEVEL00 overlay guard**, `0x800ccf1c/20/24`,
`00000000 → a3750000 87e6ffff 7ea80100` = position `(30115, -6521, 108670)`,
right where Duke was. Overlay code at `0x800cb1a4` passes `0x800ccf1c` as the
output vector of the resident trace routine `0x8007177c` (the zone script's
hit test). The 9668-byte guard covered the file's trailing 16-byte scratch
vector, so the first time that script fired, `identity()` failed and — since
identity is re-checked every hook — never recovered.

**Fix:** the overlay guard is `0x800ca968` × **9652** bytes (code and tables,
`274d71dd…`); the scratch vector is level state. `ttk_state_probe.py` uses
the same range. The controls test writes the vector (and a foreign fourth
word) and requires the lease to hold, and flips the last table word before
it and requires refusal. Live: 0 identity refusals across the ledge, turn
after it 3115 → 85, standing jump 98, run resumes.

### Mid-depth wade (0x200–0x281) owned at the original handler

With identity fixed the ledge water still wades at the original pace. That
water is depth **512**: every gait transition in the resident dispatcher
(`800486c8` and ~50 sibling checks) swaps land gaits for clips **80/81**,
state **1/1**, whose handler `0x800539f8` (called from the dispatcher's wade
cases `0x8004870c/3c/6c/9c`) runs an `0x800788e0` clearance probe and adds
the animation root to the position. Vanilla D-pad in this zone: **8–14
units/frame** (measured). The clip root is written by the track advance
(`80059d18..80059d88`, rotated in place by `80066d94`) **after** the
player-update hook and **before** the handler, so the iteration-4 early
rewrite could never take (write trace: extraction → rotate → handler).

Delivery: hook `0x800539f8` (added to `mod_function_entry_funcs`, regenerated
through the normal generator; one line changed in generated C). With the
camera-only lease live, mode 1, clips 80/81, depth `≥0x200` and a direction
held, the world-space root is retargeted to camera-relative WASD at the land
run's per-tick band (clip 6–44 → gain 4, floor 80, ceiling 120; walk band
40–60 if the modifier ever applies there). No direction, original aim, hurt
or camera flags, foreign caller or land anim: untouched. Guards added for
the dispatcher wade cases (`0x800486b0`/260), the handler (`0x800539f8`/452)
and its probe step (`0x800788e0`/200). Space while mid-wading is a normal
wade jump (80/81 joined the standing set).

Live on slot 3 (`/tmp/d08p_live_probe30/31/34.py`): mid-wade **45–58
units/frame** in 80/81 (Vanilla 12); mouse turn while wading changes the
heading (yaw 85 → 7°, moved 7°); A strafes at 45; Space in the wade → 98;
0 identity refusals; turn after the zone 3115 → 1152; run back 58.4;
standing jump 98. Slot 2 (turret corridor) regression: 0 refusals, turn,
jump and run resume.

Known original behaviour kept: pushing into a wall while wading makes the
handler idle (63) with W held — the original probe refuses, there is no
wall slide in that handler. Turning the view away (mouse) resumes at once
(probe: +200 units into the wall stays; −200 moves at 25 → 78 land run).

## Delivery

- Free swim only for original swim states 4/5 or depth **`≥0x281`**.
- Escape **and F7** (and other host F-keys except F10) request recapture
  even if the mouse is already free. F10 still opts out.
- Gameplay capture offers are kept until `allow_capture` is true; the F7
  menu cannot swallow them.
- Identity remaps the complete Modernized apartment pair so F7 slot 2
  keeps the LEVEL00 camera lease. The pair check runs only on those two
  overlay words; doing it on every guarded word stalled gameplay and audio.
- Waist-deep / mid water (`<0x281`, not swim states 4/5) uses the **land**
  camera lease: mouse look, body faces the view, original run gait (no
  default Walk pad), and tank turns 70/71 convert to 76/72. This is not a
  swim path.
- Crosshair / view-aim accept `locomotion_input_ready()`.
- Lease-dead tank fallback: W/S D-pad Up/Down; **A/D are L2/R2**, never turn.

Vanilla is unchanged. stderr tank/lease lines remain.

## Acceptance

**Accepted 2026-09-29** — user playtest of F7 UI slot 3 on binary `79a3fd5b…`:
"it really works. perfectly." D08P is Done. The original playtest brief follows
for reference.

Playtest from **F7 UI slot 3** (right-hand ledge; run forward along
it and past it: run, mouse and jump must all survive, and the mid-depth
water there must move at run pace with mouse steering) and from **F7 UI
slot 2** (turret around the corner) or the same
in-game spot: run-speed camera-relative WASD and mouse look in that wade;
Shift-run and mouse still work after stepping out, including when Shift was
already held during the load or a pause; crosshair while armed; F7 load
recaptures Modernized without a manual F10; Escape → resume recaptures and
the pause menu itself is not captured; true deep water (`≥0x281` / swim
states 4–5) still uses the D08O swim path. If movement still degrades, note
the on-screen `ORIGINAL MOVEMENT (reason)` text and send
`recomp/build-local/logs/session-*.log` from that launch.
