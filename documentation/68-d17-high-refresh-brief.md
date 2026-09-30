# D17 brief: arbitrary / high refresh rate support

User-supplied backlog brief (2026-09-30) for [D17](../MODERNIZATION_JOBS.md).
Backlog only: do not implement until the user selects D17. The audit and
technical plan at the end come before any change to the timing model.

## Goal

Extend the Duke Nukem: Time to Kill recomp so rendering can operate smoothly at
modern high refresh rates without changing gameplay behaviour, simulation speed,
physics, animation timing, or any other frame-dependent game logic.

The game currently runs correctly at **60 FPS** and looks excellent.

The next step is **not simply to make the game run faster**. The objective is to
properly separate presentation/render frequency from gameplay timing so the game
can render at arbitrary modern refresh rates while retaining identical gameplay
behaviour.

## User-facing frame-rate options

Add a frame-rate option with:

- **Match Display**
- **30 FPS**
- **60 FPS**
- **120 FPS**
- **144 FPS**
- **165 FPS**
- **180 FPS**
- **240 FPS**
- **Unlimited**

`Match Display` should be the preferred/default modern behaviour where practical.
It should detect the active display refresh rate and target that refresh rate
rather than relying on a hardcoded list.

The explicit presets remain useful for user preference, compatibility, debugging
and regression testing.

`Unlimited` should remove the intentional render FPS cap, subject to whatever
synchronization requirements the renderer/platform imposes.

## Core requirement: rendering must not control gameplay speed

Do **not** implement these modes by simply increasing the frequency at which
existing frame-dependent game logic executes.

The eventual architecture should conceptually allow:

    gameplay / simulation timing
                |
                v
        game state updates
                |
                v
      interpolation as required
                |
                v
         rendering timing
                |
                v
    30 / 60 / 120 / 144 / 165 /
    180 / 240 / display / unlimited

Rendering at 180 FPS must **not** cause gameplay systems to execute three times
faster than they do at 60 FPS. Likewise, changing from 180 FPS to 30 FPS must not
cause gameplay to slow down or otherwise behave differently.

## Behavioural requirement

Once implemented, the same gameplay sequence should behave equivalently at
**30, 60, 120, 144, 165, 180 and 240 FPS.**

Pay particular attention to anything that may currently be tied directly or
indirectly to rendered frames, including:

- Duke's movement
- acceleration/deceleration
- jumping and falling
- jetpack behaviour
- camera movement
- mouse/controller look
- player animation
- enemy movement and AI
- enemy animation
- weapon firing rates
- weapon animation
- projectile movement
- collision detection
- doors and moving geometry
- lifts/platforms
- pickups
- particles
- effects
- timers
- scripted sequences
- HUD animation
- menus
- audio triggers
- level transitions

Do not assume these systems are frame-independent merely because the game
currently behaves correctly at 60 FPS. Audit them.

## Interpolation

If the underlying game simulation cannot or should not execute at the same
frequency as the renderer, introduce interpolation where appropriate. A
high-frequency renderer should be capable of displaying intermediate positions
between simulation updates rather than requiring additional simulation steps
merely to produce additional visual frames.

Interpolation may be appropriate for:

- player/camera transforms
- actors
- projectiles
- moving sectors/platforms
- other visible world transforms

However, do not blindly interpolate state where doing so could alter gameplay
semantics. The priority is **identical gameplay + smoother presentation.**

## Input

High refresh rates should provide the expected improvement in perceived
input/camera responsiveness where the architecture allows it. Pay particular
attention to mouse look.

Avoid rendering at 180/240 FPS while camera input is perceptibly quantized to a
much lower rate unnecessarily. At the same time, input handling must not cause
movement speed or other gameplay values to become dependent on render FPS.

## Display synchronization

Investigate how the current renderer/platform layer handles:

- display refresh detection
- VSync
- VRR
- frame limiting
- swap/presentation intervals
- fullscreen vs windowed/borderless behaviour

`Match Display` should use the refresh rate of the display on which the game is
actually being presented where the platform APIs make this reliably available.
Consider what should happen if the window moves between monitors with different
refresh rates.

Do not hardcode 180 Hz merely because that is the current development monitor.

## Frame pacing

Average FPS alone is not sufficient. Frame delivery should be evenly paced:

- 60 FPS -> ~16.67 ms/frame
- 120 FPS -> ~8.33 ms/frame
- 144 FPS -> ~6.94 ms/frame
- 165 FPS -> ~6.06 ms/frame
- 180 FPS -> ~5.56 ms/frame
- 240 FPS -> ~4.17 ms/frame

A stable 180 FPS with consistent ~5.56 ms presentation is preferable to an
unstable nominal 180 FPS with poor pacing.

## Preserve 30 FPS

Keep **30 FPS** as an explicit option. It is valuable as:

1. a compatibility option,
2. a behavioural reference point,
3. a regression-testing mode,
4. a way of exposing logic that is accidentally tied to rendered frames.

The game should remain fully playable and logically correct at 30 FPS.

## Testing

Create a repeatable comparison procedure for at least **30 / 60 / 120 / 180 /
240 FPS**, comparing the same scenarios at each rate. Useful regression cases:

- standing/walking/running known distances,
- jumping,
- falling,
- jetpack ascent/descent/hover behaviour,
- camera rotation,
- firing automatic and semi-automatic weapons,
- projectile travel,
- enemy movement,
- doors/lifts/moving geometry,
- animations,
- timers and scripted events.

Where practical, measure behaviour numerically rather than relying exclusively on
visual inspection. For example, travelling the same distance for the same amount
of real time should produce equivalent results regardless of rendering FPS.

## Important constraint

**60 FPS is currently known-good behaviour.** Do not destabilize the existing
60 FPS implementation merely to add higher rendering rates. Treat current 60 FPS
gameplay as an important regression baseline while determining which parts of
the engine need to be decoupled or interpolated. If architectural changes are
necessary, make them deliberately and verify existing behaviour after each stage.

## Desired end state

Duke Nukem: Time to Kill runs with **original/correct gameplay timing +
arbitrary modern rendering frequency.** A 180 Hz display can present the game at
a properly paced **180 FPS** while Duke, enemies, weapons, physics, animations
and world systems behave identically to the established correct implementation.
The renderer is ultimately refresh-rate agnostic rather than engineered around
60, 120 or 180 FPS.

## First step: audit and technical plan

Before implementation, audit the existing timing architecture and produce a
short technical plan identifying:

1. what currently determines simulation/update frequency,
2. what currently determines rendering frequency,
3. which systems remain frame-dependent,
4. how display refresh is currently detected,
5. where interpolation or delta-time conversion is required,
6. the safest staged implementation path.

Do not begin modifying the timing model until that audit is complete.
