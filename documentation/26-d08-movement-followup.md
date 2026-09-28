# D08 movement and traversal feedback

The user reports improved aiming and successful backward jumps, but intermittent
sideways jumps going forward, ladders stopping after a few steps, brick-wall
climbs needing Classic controls, occasional difficulty recapturing F10, and a
hard stop on landing. This pass implements a bounded Modernized follow-up;
Vanilla keeps its original behavior. D08's broader state matrix is not complete.

## Reproductions and changes

The original gait alternates between **103 and 104** for running jumps. The D07
adapter handled only 104. The baseline captured 103 launching in the facing
direction during a rightward run. Both variants now share the same guarded,
once-only horizontal-velocity correction. Gravity, swept collision, impact
response, fall damage and airborne integration remain original. Additional
owned-EXE guards authenticate both dispatch entries and the launch/landing code.

Held movement previously stopped emitting the game's locomotion request as soon
as ordinary walking/running ended. A separate input permission now retains that
request during an owned running jump and verified wall bumps. It does **not**
authorize movement, facing or camera writes in arbitrary states. The original
landing selector at `0x80054C04` can therefore choose its run continuation when
movement stays held, instead of its stationary recovery. Releasing movement
still permits the original stop. No synthetic position advance or air steering
is introduced.

The fenced car-park pillar reproduced the climb failure. W/E reached grab
animation 139 (state 8), then hung indefinitely at 149 (state 6); the original
Up/X buttons immediately completed pull-up 140. The new input handoff supplies
W/S/A/D as the original directional buttons during authenticated, holstered
attached traversal. E supplies the original action button there too. Opposing
directions cancel. Original attached-state dispatchers retain all animation,
facing, contact tests and camera ownership. Modern movement/facing hooks stay out.
The updated replay reaches the top using W/E alone and returns to normal state.

The alley ladder near the subway was then reached through ordinary movement.
The updated replay records mounting 185 (state 8), attached 186/188/189 (state 3),
and exit 190 (state 8), then normal standing on the platform. It climbs well past
the initial steps without switching to arrow controls. State 8's automatic ladder
mount/exit animations retain original behavior; the input handoff applies during
attached climbing. Ledge/mantle 134–142, attached state 3, and the original 6/7
hanging dispatches are guarded separately from normal camera-relative movement.
Not every attachment type or map has a real-input replay.

F10 capture remains explicit; focus/pause still releases held input. The observed
climb replays exercise recapture between actions, including supported climbing
without a normal-camera lease. The reported need for multiple F10 presses has
not been established as a separate SDL capture bug; do not claim a timing fix.

## Evidence boundaries

Research uses private displays, cards and preferences. The exploration harness
uses the original pause menu between commands, and all recorded movement runs
unpaused through original game simulation. A logged player-health-only fixture
sets `+0x32` to 30000 (spawn is 10000 with HUD 100) for long enemy-exposed routes;
no position, velocity, climb state, inventory or collision fixtures are used.
This is movement evidence, not damage-balancing or campaign acceptance.

Earlier harness experiments are retained but excluded: a mistaken 1000 health
fixture gave only HUD 10; an unpaused exploratory wait ended in death; stopping
the process between commands triggered the runtime's starvation watchdog; and a
replaced executable exposed a process-detection bug in the test helper. The
helper now recognizes Linux's ` (deleted)` executable suffix. Normal original
pause-menu input replaced process suspension. No watchdog setting or runtime
patch was changed for the player build. The later exploratory tail after the
successful ladder exit also ended in enemy-fire death and is not descent evidence.

Navigation reference only: [SPaul's walkthrough](https://gamefaqs.gamespot.com/ps/197177-duke-nukem-time-to-kill/faqs/3834)
identifies the alley ladder near the subway. Implementation addresses and behavior
come from the owned executable and local replays, not the walkthrough.

[The delivery report](reports/d08-movement-followup.json) records binary identity,
checks, accepted route segments and remaining limits. Six captured jumps verify
the corrected 103 direction in all four cardinal directions. Clear forward, left
and right landings return directly to run 78; the two obstacle impacts retain
107/108 followed by recovery 105. A separate backward route also returns directly
to 78. Its initial verifier failure came from sampling state 0/9 one frame before
the original animation changed from 103 to 78; the trace passes the corrected
verifier, while its failed harness exit remains recorded. The final continuation
route verifies 104 direction/camera ownership and original recovery when input is
released before landing. Both variants also have native coverage.

Short final timing windows measured approximately 60 guest fps at idle and during
look, with zero added dummy-output underruns. These are not listening evidence. Broader traversal, swimming, scripts, progression/security-card
interaction, campaign coverage and subjective movement feel remain required D08
work. The prior audio report is still not resolved by listening evidence.
