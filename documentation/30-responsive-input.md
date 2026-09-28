# D08: Escape / Start and responsive input

The user reports that Escape should skip intros and open the game menu without
opting out of Modernized controls, and that short-platform running jumps and
midair ladder grabs remain finicky. This pass changes the input adapter only.

Escape and Enter both emit original Start in Modernized, including uncaptured
intro/menu input. They release relative mouse capture for menus. Pausing from
captured gameplay retains automatic capture intent; a fresh verified normal
camera offer restores capture after the debounce. F10 still explicitly opts out.
Focus and menu release clear held actions and the jump buffer. Vanilla is unchanged.

Jump previously forwarded only the instantaneous held state. A press/release
between host event polling and guest pad sampling could disappear. Keyboard and
rebound mouse jump presses now retain an eight-input-frame request (~133 ms).
Replay requires the existing identity-checked normal locomotion lease. It does
not authorize midair jumps, override attached traversal, write position/velocity,
or bypass original jump selection/collision. Key repeat cannot renew the request.
Ordinary held input remains available to original unsupported-state controls.
This is input buffering, not coyote-time: a press after leaving the edge still
requires a supported grounded state to be replayed.

A queued unarmed E tap during owned airborne locomotion now lasts eighteen input
frames (~300 ms), instead of eight. Grounded taps keep eight frames so ordinary
interactions/redraw do not gain extra delay. The equipment check is evaluated on
every pad composition: E cannot send armed Cross even if equipment changes during
the window. Existing temporary-holster ownership and manual C semantics remain.

Local reference: `/home/spartacus/Games/build-eduke32-vanilla/settings.cfg`
confirms E=Open, Space=Jump, LShift=Run, Caps Lock=AutoRun, WASD forward/back/strafe.
Its other bindings are not copied over the user's explicit C holster preference.
The nearby Duke-RT source is a different engine and was not treated as TTK's
physics or state contract.

## Verification and remaining limits

All four native harnesses pass: input, controls/camera, aiming, and scene lifetime
(the latter uses the preserved private crash RAM). Python: 56 run, 54 passed,
two environment skips. Native input coverage includes a complete tap between
polls, timeout/repeat, flight rejection, focus cancellation, Escape's Start bit,
automatic recapture, F10 opt-out, and midair E remaining unarmed-only.

The private OpenGL delivery run skips intro with Escape and reaches gameplay.
Escape pause remains uncaptured; Escape resume restores capture without F10.
Six real SDL brief jump taps (one guest frame plus scheduling overhead), with
left/right/forward/back run-ups, all reach original running-jump animation 103
with correct world direction. Four clear landings continue running; two obstacle
impacts retain original recovery. Final screenshot reviewed; exit code 0.
The driver uses private cards/profiles and an explicitly logged player-health
longevity fixture; it never writes movement, animation or enemy health.

The exact short-platform-to-second-ladder sequence is **not** reproduced here.
The input fix is bounded and tested, not proof that every missed jump shares this
cause. Midair E's timing is native-tested; the exact ladder catch and perceived
heaviness still need user playtesting. Original takeoff/landing/collision remain;
no full-campaign or audible-output acceptance is claimed. D08 remains Needs playtest.
See [recorded evidence](reports/d08-responsive-input.json).
