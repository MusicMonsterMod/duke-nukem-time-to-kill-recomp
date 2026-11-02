# D26K - No weapon holster while the console is open

Status: Done (user-accepted 2026-11-02: "excellent work! lock it in, close it out"). Board: `MODERNIZATION_JOBS.md` D26K.

## Cause (confirmed)

The holster is the original's own idle sequence, not a Modernized holster
path. The native idle routine `0x80053038` (called from the player update,
return address `0x800532b8`) resets `player+0x240` to 3000 when
`0x8001e070` reports pad activity, otherwise counts it down by the frame step
`0x800d21fc`. At zero, with an armed upper-body animation (flag 8 in the
`0x800c2824` table), it calls `0x8004d4a4` (the only caller of that holster
routine), then later expiries pick fidgets 64-69.

Whether the timer runs depends on the speed modifier. In Modernized
locomotion the Walk bit is on the guest pad in walk mode (the default), so the
timer stays at 3000 and there is no idle holster. In run mode (Caps Lock, or
Shift held) the neutral pad has no Walk bit and the timer runs: the weapon goes
away after about 14 s standing still. While the console holds input (D26J,
active frame with no new input), Caps Lock run mode stays on, so the same
countdown ran with the console open.

Reproduced on the D26J executable
(`d28b1a8e8cbc23c2bfc4933d4e48d9d4050d12011c778956a03cd6219d5b34bf`), private
copy of file slot 10, Caps Lock run mode: console closed, holster after 13.4 s
(upper animation 5 -> 6, equipment 2 -> 0); console open, holster after
15.6 s with `console_hold` true and the timer counting.
An earlier lead was wrong: in file slot 11 (UI slot 12) Duke takes damage
while standing (health 92 -> 62) and dies at about 108 s, which first looked
like a holster (animation 215/217/219, equipment 0). D08A33's 25 s check in
that slot was in walk mode, which is why it did not reproduce.

## Change

`recomp/src/ttk/console_idle.inc`, `console_idle_hold()`, called from the
authenticated player update (`0x8005a210` from `0x80041b34`) beside the
other per-update owners. While `input_console_holding()` (new in
`pc_input.h`; true only for the D26J hold over captured Modernized gameplay)
and `player_identity_ready()`, it records `player+0x240` and writes it back
whenever the native countdown lowers it. A reset upward (activity, 3000) is
accepted and kept from then on. The record is per input epoch. Closing the
console stops the hold; the countdown resumes from the kept value. A holster
or fidget already under way when the console opens finishes as the original's.
No hook, codegen or framework change; Vanilla never has a console hold.

## Evidence (executable `e0714bcbd5b17717dca8c78688ef74e40f0dba42fdb9040bdfeef2c109d625e7`)

Probe `recomp/analysis/d26k-console-idle/probe.py` (private card copy, Xvfb,
debug port, the real backtick console, Caps Lock sent as an X key event; player
saves, cards and profile hashes unchanged after every run), file slot 10:

- Console closed, run mode: holster after 14.1 s (13.4 s before the change),
  then fidgets 69 and 66 as before.
- Console open 65 s with W, A, S, D, C, Space, E, 1 and a click typed, third
  person, weapon 4: timer held at 2890-2910, equipment 2 and the ready loop
  unchanged throughout; after close the first click fired (upper 5 -> 16).
- Same in first person after `weapon_next` (weapon 5): timer held at
  2380-2390, no change; first click after close fired (20 -> 24).
- Fists (`weapon_group_1`), 40 s open: no fidget (lower 63 throughout); the
  first click after close punched (113).
- Resume: 5 s idle, 65 s in the console (timer 1900-1910), closed: the holster
  came 8.7 s after close, i.e. the remaining time.
- Under Xvfb the game ran at about 32-48 frames per second while the console
  was open, so 65 s of wall time was about 34 s of game time, still more than
  twice the 14 s idle time.
- Native: `ttk-controls-test` new D26K group (held for 600 modelled native
  steps, reset kept, close resumes, never expires at a low value, new epoch
  re-records, Vanilla unchanged). Fails without the change, passes with it;
  123 groups pass. `ttk-input-test` and `test_pc_input.py` pass.

## Remaining

Accepted by the user. Vanilla is covered by construction and the native group; the scripted input
driver does not run in Vanilla. With the console closed, Modernized run mode
still holsters after about 14 s idle and walk mode never does; that is the
original timer and is unchanged here.
