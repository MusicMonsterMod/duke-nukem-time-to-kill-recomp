# 451 - D50B loads inside our guest calls; D50C the level 3 pit; D41O names

Date: 2026-11-12. Status: Done (D50B, D50C, D41O; user-accepted 2026-11-12: "the game no longer crashes at that point ... testing is solid"). Lab:
`recomp/analysis/perf/lab.py` cases `accept`, `d50b`, `loadloop`, `pit`,
`death`, `keys` (local; private card copies, the player's own profile copied
read-only, player files hashed before and after every run).

## 1. D50B - a save can end the game

### What the 2026-11-11 work left

Two symptoms: an exit (`execution completed, PC=0x00000000`) after a save,
and the guest stopping after a load that follows a save. The IRQ-return
guard of 2026-11-11 (a save taken at an IRQ return waits while a nested
dispatch or call unit is open) stayed in.

### Reproduction

The old loop (load the player's UI slot 9 state, save to slot 11, load the
player's state again) no longer failed: 125 rounds, then 60 rounds with every
save forced through the scheduler unwind, no stall. That loop never loads
the state it has just saved. The acceptance loop does (`accept`: save through
the debug port or F6, load that save back through the port or F9, check the
game runs), and it failed within 2 to 25 rounds on the current build, at 60
and 180 Hz: `execution completed, PC=0` right after the load, or the guest
stopped (0 fields a second), often after `[TTK load UI] fresh-frame wait
timed out` (the picture held for 1 s). Vanilla (`--vanilla`, the same loop
through the debug port): 50 rounds, 47 of them unwinding saves, no failure.
Loading a failing state in a fresh process: no failure (5 of 5).

### Cause

At the exit the function trace (`PSX_FNTRACE_ALL=1`, `PSX_EXIT_HALT=1`) shows
the camera render 0x8002e48c returning through a saved return address of 0
(its frame at 0x801fff08, `lw ra, 0x40(sp)`). Read straight from the saved
state file, that frame's return address and two saved registers were 0; the
player's own saves hold 0x80026278 there (the game's caller). A probe on the
camera render's prologue store never saw ra = 0, so the frame was overwritten
after it was built, and the save captured it.

Our Modernized code calls game functions from the host (13 calls: host
visibility 0x80031d10, pole visibility, crouch hull, kicks, cheats, the
gadget HUD, shortcuts, sleeping enemies, steroids, the damage flash). Each
copies the CPU state into a local `CPUState`, moves its stack 0x100 below the
current one, writes arguments and scratch into guest stack and scratchpad,
runs the game function on that copy with the return address 0x800000fc, then
puts the stack and scratchpad back. Two of our calls (the aim trace and the
menu text) tell the framework not to save or load meanwhile
(`psx_mod_host_call_begin/end`); these 13 only set our own `guest_busy` flag,
which the framework does not see.

Saves are already refused inside them (both admission paths need the
outermost dispatch). Loads are not: `savestate_poll` admits a staged load at
any compiled poll when no host call is declared, and applies the loaded
registers to the `CPUState` it was handed, which inside our call is our local
copy. It then unwinds to the scheduler and resumes the loaded PC with the
real CPU state, whose registers (stack pointer, return address, saved
registers) are still those of the moment our hook ran, against the loaded
RAM. The game then writes and returns through the wrong frames: zeroed saved
registers and return addresses (the camera render's frame in the failing
states), a return to PC 0, or a stop. Fresh-process loads never land inside
our calls at that moment, which is why the same files load cleanly there.

Established: the load path's behaviour (code), the zeroed live frame in the
failing states (file), Modernized only (Vanilla clean), and the fix's effect
(section 4). Not caught live: a load landing inside one of our calls (no
probe of it was run); the mechanism is the one the code allows and the fix
removes.

### Fix

All 13 calls now bracket the game call with `psx_mod_host_call_begin/end`, as
the other two do: a load (or save) requested during one waits for the next
poll (microseconds later). No change to what the calls do. Rule for new code:
every host-to-guest call (`psx_dispatch_call` on a copied `CPUState`) must be
bracketed; `guest_busy` alone is not enough.

## 2. D50C - walking into the level 3 pit

Walking forward from UI slot 9 drops Duke about 7000 units at the end of the
corridor. He dies there (flags 0x802, health 0); the game's own outcome is its
death view and the PRESS E TO OPEN MENU prompt. Before, the emulation halted
on `GPU GP0 unknown command 0xFE` or `unimplemented command 0x1F`, or slowed
until the starvation watchdog stopped it.

GP0 ring at the halts (`PSX_FORENSICS=1`): with Duke dead outside the level
the game builds broken packets. One node had a corrupt word count, so the DMA
sent the following packets' link headers to the GPU as data, then followed a
garbage link into game data; another link was 0xA805E5 (unaligned, bit 23
set). The same family halted a third-person load of UI slot 9 (GP0 0xFE from
a link into 0x801f0030, game records rather than packets).

The console's GPU ignores a command word it does not know (DuckStation and
Mednafen consume it as a one-word NOP), and GP0(1Fh) only requests IRQ1. The
framework halted instead. Changes (framework, `gpu.c`, `dma_gpu_ll.c`):

- Unknown GP0 opcodes are one-word NOPs; GP0(1Fh) sets GPUSTAT.24 and raises
  IRQ1 (masked by the game). The first four and then every 4096th are logged
  (`gpu: GP0 0x.. ... ignored as the console does`).
- The DMA linked-list walker ends a list at a bit-23 link that is not
  allocated mod packet memory (main RAM is 2 MB; only our packet windows live
  above it), as the replay walker already did, instead of folding it into RAM
  and walking game data. Logged four times (`dma: GPU list link ... the list
  ends there`). New case in `test_dma_gpu_linked_list_timing.c`.

## 3. D41O - the shadow setting's name

OPTIONS > DISPLAY > SHADOW EDGES is now LIVE SHADOWS, SHARP (was SMOOTH) or
SIMPLE, help "BLASTS AND FLICKERING LAMPS. STEADY LAMPS ARE BAKED". Saved
values stay `smooth`/`simple`; console `light shadows sharp` works. Verified
in the menu (offscreen shot, local).

## 4. Results

Offscreen, private card copies, player profile copied read-only.

| Run | Before the fix | After |
| --- | --- | --- |
| Save, load it back (port), 60 Hz | exit or stop within 2-25 rounds (5 runs) | 60/60 |
| Same, 180 Hz, F6/F9 and port alternating | exit within 3 rounds | 50/50 (25 F6), lowest 60 fields/s |
| Same, 180 Hz, first run | - | 35/35, then one round at 8-13 fields/s during a Timeshift backup's tail (08:00-08:26); the state loads at full speed fresh, and the 50/50 run above came after |
| Same, third person, port, 60 Hz | - | 30/30 |
| Vanilla, port, 60 Hz | 50/50 | 25/25 |
| Third-person load of UI slot 9, 60 Hz | GP0 0xFE halt on the first load | 10/10 |
| Pit walk (180 Hz x2, 60 Hz first and third person) | GP0 halt or watchdog stop | 4/4 end in the death prompt |

GP0 words ignored per pit run: 0-4 (logged); list ends at foreign links: 0-4.
Tests: `dma_gpu_linked_list_timing_test` (new case), `mod_gpu_dma_aperture_test`,
`test_player_profiles` (136) pass. The exported framework patch applies to a
pristine pinned checkout and reproduces the changed files.

## 5. Remaining

- D50B: the user's playtest (F6 / F9 and SAVE GAME / LOAD GAME at 180 Hz).
- D50C: after the death in the pit the game ran at 53-60 of 60 fields a
  second (the dead camera far outside the level is heavier); not a halt.
- D41O: the wording is a proposal for the user to accept or change.
