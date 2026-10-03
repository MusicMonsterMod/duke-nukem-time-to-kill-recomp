# Current status - 2026-10-03

**D17A ACCEPTED. D17B ACCEPTED (2026-10-03).** The user completed natural
first-level play at 120 and 180 FPS and explicitly closed the parent jobs.
120 FPS is the primary quality/regression baseline; 180+ excellent high-refresh
support, 240+ robustness/compatibility, Unlimited stress/debug. No technical
ceiling or gameplay-speed change. Residuals are D17D-J and D18C, not reasons
to keep the accepted parent jobs open. No follow-up implementation started.

Accepted binary: `1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`.
Preserve mouse response, opening/club playability, apartment/culling fixes,
FMVs/audio, FPS reporting, idle stability and intentional PS1 motion.
[Acceptance, source baseline and reproduction ledger](86-d17-acceptance-and-regression-baseline.md).

The repository now tracks the actual `recomp/` implementation and complete
framework patch with pinned dependencies. The prior documentation-only exclusion
was explicitly revoked by the user. ROMs/generated game code/builds/saves/research
remain ignored. Original disc dump moved intact under `game/`; prepared runtime
copy retained. Older statuses and results below are historical and superseded.


**Latest: accepted playability baseline preserved; bounded polish investigation.**
The user reports the opening, strip club and apartment are now smooth, enjoyable
and substantially more stable. This acceptance applies to build `28554051...`.
The follow-up disables unnecessary per-batch GPU timing queries in ordinary
launches and adds opt-in pose-readiness tracing. No interpolation, culling,
physics, audio-buffer or mouse-sampling change is shipped.

Slot 12 Shift+W/S still exhausts usable interpolation history while presentation
intervals remain mostly regular; complete geometry arrives too late for some
planned movement frames. Larger buffering and stronger phase correction were
rejected. Slot 10 can starve audio production under the scripted stress route;
slot 1's crackle did not reproduce as an underrun. These remain open polish
items. Opening/dancer mouse regressions retain about 179-180 images/s, zero
underruns and no camera turn over one degree. D17D now holds isolated visual
artifacts (slot 9 also reproduces at 60 Hz). D17A/B remain Needs playtest.
See [85-movement-polish-and-isolated-artifacts.md](85-movement-polish-and-isolated-artifacts.md).
Current executable SHA-256:
`1c03b6a2c2c8fa6363376135d0ffa066dcdc0eb50e3f792efe198af1b9779287`.


**Latest: save-driven camera stability candidate, D17A/B Needs playtest.**
This supersedes note 83's performance conclusions. Five-field dancer updates
were incorrectly resetting camera yaw; expired replay jobs also hid their
cost from scheduling. Those causes are fixed, compatible opaque/translucent
batches reduce the slow states' main-thread submission cost, and X11 mouse
timestamps are repaired. Known scratch allocations now precede worker startup.
Final 180 Hz saved-state sweeps produce about 179-180 distinct images/s with
zero audio underruns and no adjacent camera turn over one degree. The fresh
opening has presentation p99 6.07 ms, max 7.83 ms from its first camera redraw.

Unlimited still exposes stale-camera episodes. Subway UI slot 3 did not
reproduce on the bounded 60/180 Hz capture route and remains open, alongside
earlier minor geometry pops. Player saves/cards/settings remain untouched.
Build SHA-256 `28554051d10f40fcf67c4f242998f02bc5ff960e570098dd9ae2a2fab3dbe264`.
See [84-warmup-and-camera-stability.md](84-warmup-and-camera-stability.md).
The next step is a player playtest of this bounded candidate, especially UI 5.


**Latest: opening/club responsiveness candidate, D17B Needs playtest.** This
supersedes the older performance results below. Final fresh boot at CPU 100%,
first person, 4x, 180 Hz: camera presentation p99 6.0 ms, maximum 8.3 ms from
the first valid redraw, zero audio underruns or failed/killed workers. Final
Match Display dancers turn: 179.5 distinct images/s, p99 6.18 ms, realtime
emulation and zero audio underruns; apartment comparison 179.8 images/s,
p99 5.98 ms. Mouse injection to traced camera presentation: median 17.85 ms
club / 12.48 ms apartment. These are bounded software measurements.

Systemic fixes cover timestamped scheduling, restored/worker code-state
coherence, an exactly guarded initialized math routine, GL batching, unnecessary
worker replacement for invalidated movie code, and a worker timeout race.
Vanilla and the accepted idle-eye behavior remain available. Minor closet and
furniture pops remain D17A playtest work. Existing player settings/saves/cards
are untouched. Build SHA-256
`ae4333a81c35dbeae2ccc5c5eb2b3ac2f3c99a71d84c36af35b8f950fce2da22`.
See [83-opening-responsiveness.md](83-opening-responsiveness.md) for evidence,
limits and reproduction. The next step is the user's fresh-game full playtest.

**2026-10-03 renderer quality pass:** D17A/B have a new stabilization
candidate, with replay color/depth isolation fixes, coherent eye setup for
room walks, transform lookup and vertex-provenance safeguards, and tail-aware
redraw scheduling. Final 2x isolation: 459 checks, zero mismatches. Normal
visual route: 24 sequences, no detected back-and-forth jumps. Performance is
still below the target in the club: the final 180-target run produced about
85 distinct images/s while turning; Unlimited about 100. Do not interpret the
older response-time/smoothness claims below as acceptance of this workload.
See [82-renderer-quality-pass.md](82-renderer-quality-pass.md) for profiles,
remaining endpoint uncertainty, regression evidence and build identity.

**2026-10-03 (later):** loss of control fixed (host Circle requests are taps;
held Circle entered the game's inventory hold, `+0x224 0x200`); camera
re-seeded after savestate loads; late camera rebuilt (one redraw per present,
fixed-lead mouse sampling, adaptive lead, pacing hysteresis).

**2026-10-03:** see-through outdoors and in the apartment fixed (host depth
cleared per frame); dancer jerk addressed (in-betweens planned per paced
present, overrun redraws, lead 16 ms); idle stability user-accepted.

**D17A / D17B / D17C - Needs playtest.** Late camera on with adaptive pacing:
every present uses the newest mouse look (step response about 17-19 ms, was
66-130 ms), a new image every 1st, 2nd or 3rd refresh as the PC keeps up. Near
geometry: host precise vertices (exact positions, depth, texture coordinates)
plus a depth buffer among them; correct perspective beside the eye, closet and
collar artifacts gone. First person: stable idle eye (no breathing world), PS1
wobble kept while moving, deliberate view bob option `view_bob`
off/subtle/on/strong. See documentation/81-d17a-instability.md.

**D17 high refresh rate - Done (user-accepted 2026-10-02).** Modernized
`--frame-rate display|30|60|120|144|165|180|240|unlimited` (default 60;
Vanilla always 60): the runtime presents at the display's rate and TTK's own
renderer draws interpolated in-between images in worker processes; game logic,
physics, animation timing and audio unchanged. The user saw about 180 FPS at
Match Display with very good stability, correct FMVs and an accurate fps
readout. Follow-ups: D17A (texture/geometry instability, black areas), D17B
(mouse responsiveness and input latency). Details:
documentation/80-d17-high-refresh-audit.md.

**D08J1 overhead ladder grab - Done (user-accepted).** New job from the user's
slot-6 report. That ladder's bottom rung is about 1040 above the floor, so
the original ground mount never sees it and walking into it stops at the
wall; jumps from the wrong distance bounce (107). Modernized now jumps for
Duke once per E hold when he heads at such a ladder inside the catching
window (run 750, walk 800, standing 450 from the panel), keeps the reach held
through the leap (an E tap at the wall works) and eases him along the panel
in the air; the original catch and climb do the rest. Private slot-6 copy:
all E approaches caught (first/third person, manual/assisted jump, armed or
holstered), none without E; D08U/D08X/D08Y/D08W regressions unchanged.
Binary `108d3a97a40b129d4f922d4aada3d9bdf20e7dd573293d52a55544f61b084607`. Details: documentation/79-d08j1-ladder-leap.md.

**D08Z manual jump style - Done (user-accepted).** Optional Modernized
`--jump manual` (assisted stays the default): jump on the press near gaps,
~0.15 s grace after running off an edge, WASD air steering up to the takeoff
speed (also during the E reach), standing/walking takeoff ~3x quicker. Slot-5
gap clearable by timing alone (last ~200 units or just after the edge; earlier
presses scramble up the low lip; E mantles from 1000 back). Assisted unchanged.
Binary `4f11af3a04d6987c99b0fea1ea7279c13c1c9e05144a0d61553f242f92d17a3a`. Details: documentation/78-d08z-manual-jump.md.

**D08Y gap jump - Done (user-accepted).** Jump, freezes and audio confirmed fixed. The overclock now applies only in
gameplay and backs off if emulation falls behind (fixed boot/loading audio
slowdowns; binary `0bdb53c328b12252c02edda0635950f9c1dbe11b6c93d380d887ed21d43508f2`). Round 4: Random 20 fps freezes were
the emulated CPU overrunning in views the Modernized camera/widescreen reveal;
Modernized now emulates the CPU at 150% (`--cpu-overclock`, Vanilla stock): 0
slow frames on the player's GPU (was 144/59). Binary `f4d22e958ce333f575aa977b094e2bbb43c1d36235497ae5bb59d6bd124cea4a`. Earlier: Reach freeze frames
and the jump-mantle pop fixed (binary `b7f038c03a042cfea9580270e639e1d8cc5e1ace3e79ca6a5d282ee14d61f9f9`). The gap was not a level-design
slip: the original launches a run jump pressed near a gap from the lip, and
Modernized had switched that off. Restored for real gaps (furniture keeps the
instant jump); a tapped Space works like the original's held button. Slot 5:
without E 8/10 clean landings (was 0/10), with E 14/14. The hitch after E jumps
(up to 27 ms per update) is down to ~6.5 ms. Mid-air mantles remain the safety
net. Binary `fdbaee0d01f0e8a24b128a8518ba6305a13bb0df924c3b79a3360ec3998e6379`. Details: documentation/77-d08y-gap-jump-research.md.

**D14 widescreen - Done (user-accepted).** Modernized opens in native 16:9 (also
16:10, 21:9, auto; `--widescreen off` for 4:3): wider view with the original
projection, no stretching, no holes at the widened edges (TTK portal root
rectangle widened; near clip in third person), HUD in the corners, movies and
2D menus 4:3. Vanilla unchanged. Binary
`3f726b9173ef236ca7c5f38b5c12b6713a877f85b6bbd5c0b2a8aca5e392af2c`. Unlike
DuckStation's widescreen hack, text, HUD and movies keep their original
proportions. Details: documentation/76-d14-widescreen-first-pass.md.

**D08U, D08W, D08A3, D08X - Done (user-accepted).** Ladder-top mount and full
descent; shallow-water jumps in the held direction; original TTK HUD icons in
the switcher; hold-E jump grabs and mantles (ledges and climbable objects at an
angle, extra reach, flush hang, stacked-crate release, identity-guard fix).
D14 widescreen: first pass only (inert 16:9 plugin; culling gaps open). Binary
`60161f81ab42a013ebfa326d6b80b97772b29b9d8fb051b598ddba45cbde369c`.

**Autonomous chain (user away): D08U, D08X, D08W, D08A3 Needs playtest; D14
In progress.** E at a ladder top climbs down onto it and S descends and steps
off (D08U); E held through a jump catches close ledges (D08X); shallow-water
jumps follow the held direction (D08W); the switcher uses TTK's own HUD item
icons rebuilt from /DATA/FONTS.RAW (D08A3); widescreen first pass found the
activation path and TTK's culling blocker, no player option yet (D14). Binary
`db5288c2a3431eb5d84f96ffc9cfcc00193442cd950a9ad1f420bbcb39323689`. Native
and Python suites pass. Details: documentation/72 to 76 and 09-handoff.md.

**D04A Escape frees the mouse - Done (user-accepted).** Escape now pulses Start so a
quick tap reaches the original pause poll, and automatic recapture waits until
gameplay has actually stopped (pause open) instead of 12 frames. Native input
test PASS; resume Escape no longer blocks recapture; binary
`198673f5f5643aed5f25c543b79ec69ed0216d69e2f211a075394f3b86ded68e`. User: "perfect, accept".

**D08V sewer mantle lease - Done (user-accepted).** Mantles, hangs and pull-ups,
the frames right after them and falls no jump owns now keep the mouse camera
and modern buttons (no `ORIGINAL MOVEMENT` flicker); the original traversal
routines are unchanged. Private slot-12 sweep: tank fallbacks 12 -> 0, mantles
identical. Current binary `123c7910b9ae049d0818de9cb85ea896a5242d6902ead311cf1f7f811d29693e`
(includes D07D and D08T1). Details: documentation/71-d08v-sewer-mantle-lease.md.

**D08T1 hold to grab, E always mantles - Done (user-accepted).** In Modernized, E
climbs a pushable object (the alley dumpster) exactly like any climbable
object; holding right mouse (or Alt) grabs it, W/S push/pull, releasing lets
go. With `original` weapon aiming right mouse stays precision aim and Alt
grabs. Profile schema 17. Live private-state checks in third and first person
and with legacy aiming pass; Vanilla push/pull unchanged. Binary
`66097cf8409830cba5ffdd54a830299b9fe13e480d371874d04b4b355bad3015`
(includes D07D). Details: documentation/70-d08t1-grab-manipulate.md.

**D07D red dot off by default in Modernized - Done (user-accepted).** Profile schema
16 turns the original red autoaim dot off in Modernized (older profiles are
migrated off once, with a backup; later choices are kept; `--red-dot on`
restores it). The hide hook now proves ownership at the marker's own enqueue,
so it no longer depends on the first-map camera/identity lease. Isolated
OpenGL route: hidden markers in third person (51), held aim (59) and first
person (65); none with the dot on. Vanilla unchanged. Binary
`db5bf9640893ae582acefa27fca8f98f27017bd6f1aa4c956d203cf31e6ec67b`.

**D13 higher internal resolution and display scaling - Done (user: "correct correct correct, D13 is good. I'd say let's approve it.").** Each
profile now saves an internal 3D resolution (1x original to 4x with OpenGL),
display mode (windowed, borderless or exclusive fullscreen), window width and
output filter (linear or nearest): `run.py --settings` choice R, or
`--internal-scale`, `--display`, `--window-width`, `--output-filter`.
Vanilla stays 1x; Modernized defaults to 4x (512x240 gameplay drawn at
2048x960; user: "holding 60 FPS the whole time it just looks amazing"). The software renderer always runs at 1x (29 fps at 2x). In an
isolated CPU-OpenGL instance 1x-3x held ~60 fps and 4x 50 fps; FMV framing,
HUD and pause menu are unchanged; settings survive a relaunch. Playtest
follow-up: **F11** is now the fullscreen key (replacing Alt+Enter/Ctrl+F,
which never worked because of a runtime modifier-matching bug, now fixed), and exclusive is now a real exclusive mode under SDL3.
The game now opens windowed by default and remembers
how it was left (windowed or fullscreen, exclusive or borderless, window
width). All accepted by the user. Separate backlog from the playtest:
D08U (top-of-ladder mount), D08V (sewer mantle flicker), D08W (subway
shallow-water sideways jumps), D10B (close-camera transparency, doors) and
D11C (the slot-12 headless Duke: a first-person head-hide flag saved inside
the savestate). Details: documentation/67-d13-resolution-display.md. Binary
`79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`.

**D11C slot-12 headless Duke - Done (user: "duke's head is back").** A first-person head-hide
bit captured in a savestate is now reclaimed at Duke's next draw (Modernized,
either view). The original never sets that bit (static check of the executable
and all overlays). Headless check on a private slot-12 copy: head shown in
third person, still hidden in first person. Binary
`d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`.

**D08L inertial edge run-off - Done (user: "im happy with that!").** Running off a large ledge in
Modernized no longer brakes: on the fire-escape platform outside the apartment
window the original fall cut Duke from ~47 to 16.5 units per frame (velocity
3058) and froze the mouse camera; he now keeps his running speed (9172) and
the camera through the fall and lands into his run. Walking still stops at
large edges (original), the running jump is unchanged and the late-jump grace
still applies to small drops only. Every edge departure is capped below the
running jump (a run-start stride spike could launch the bed run-off at 14000).
Vanilla route exit 0; native suites and Python PASS. Limit: only this
platform and the bed were measured. Details:
documentation/66-d08l-edge-run-off.md. Binary
`a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

**D12A first-person quick kick - Done (user: "its done! accepted. ... this has been yet another amazing feat of engineering.").** In the Modernized eye
view, **Q** is now a short Duke 3D style kick: Duke's own right leg (original
meshes, posed from the original straight front kick) snaps out in front of the
eye with any weapon still drawn, standing, moving or in a jump, and the view
never leaves first person. The hit uses the original kick's damage sphere,
radius and damage where the crosshair ray meets something within reach
(about 4 contacts, 1500 to a pig cop; three kicks kill; low props like the
alley garbage bags are hit without crouching). With the Boot selected a held
left-click kicks while moving; the thigh is now drawn. With the Boot selected, the original spinning kicks (Q, E,
held attack) become the quick kick in first person. Third person and Vanilla
keep the original kick. Checked in an isolated instance against pig cops on
the first street; 59.9 fps; Vanilla route exit 0; native suites PASS. Limit:
no knockback and lower damage per kick (both accepted by the user); thigh
13% lower on screen (user's choice). Only Q or the attack kicks (never E);
Q chains while running with any weapon. Details:
documentation/65-d12a-first-person-kick.md. Binary
`1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`.

**D08T pushable objects - Done (user: "it works so much better than the original now. this is it rock solid. confidence level is very high.").** User report: the alley dumpster
could not be grabbed in Modernized; E climbed it or did nothing. In
Modernized, **E** now grabs any original pushable object (even with W held
into it), **W/S** push/pull relative to the camera, **E** lets go, and
**Space** climbs a climbable one. Mouse look stays live while grabbing, and
first person blends to the orbit during the grab. The original grab, push/pull
and climb animations run unchanged; Vanilla keeps the original Action rules.
Checked with real keys from a private copy of the user's slot-5 state (third
and first person); Vanilla route exit 0; native suites PASS. Only the dumpster
was checked. Details: documentation/64-d08t-pushable-objects.md. Binary
`941593c077bae11e441ce8a89832f2292f97934681648eba08df4b7c36b1e0ac`.

**D12 first-person weapons - Done (user: "finally, we can mark this as accepted!!").** Next: D12A (kick stays in first person). In the Modernized eye view
Duke's own right hand and the original weapon mesh are drawn in front of the
eye (no new assets), held in each weapon's firing pose, always on top of
walls, with the original muzzle flash and a short kick. All 13 weapons on the
first map checked; pitch-stable; 59.95 fps; third person and Vanilla
unchanged. The twin cannons (key 5) are framed like Duke 3D's Devastator;
the HUD stays on top. Limits: instant holster/draw, no left hand, no reload
motion. Details: documentation/63-d12-first-person-weapons.md. Binary
`65226a9d8b4335f67b955b35af1172fb6a42357adcbd0027c97ebd5744612798` (after framing pass 6).

**D11B first-person near-wall clipping - Done (user: "its awesome!!! now it doesnt peek through the doors. amazing work." (the new jetpack sprite "is also available")).** In the Modernized
eye view, walls, floors and props beside the eye are clipped and drawn by the
host instead of tearing or turning black; everything else stays on the
original renderers (`0x80011020` world, `0x80010000` object). Club side wall
and entry corridor sweeps render continuous walls where the unclipped build
shows black gaps; first person 59.96 fps, third person and Vanilla unchanged.
Third pass: after the user's 1080p report of black floor lines, the default
is a conservative mode (clip only what the original gets wrong, no
subdivision). Compare with `DNTTK_NEAR_CLIP=0`; the subdividing mode is
`DNTTK_NEAR_MODE=full` (research only). User: conservative "looks the very
best". Props no longer fade see-through in the eye view (original occluder
fade; `DNTTK_FP_OCCLUDER_FADE=1` restores). Both are future D19 menu toggles.
Details: documentation/62-d11b-near-clip.md. Binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`.

**D11 first-person prototype - Done (user: "this is phenomenal, and it's like a dream come true").** Modernized (independent
camera): **P** toggles an eye-level view with a wider field of view (about 64
degrees); saved in profile schema 13 (`--view first|third`, default third).
The eye follows Duke's neck through the original camera solve; his head is
hidden only during his own draw; mouse look, camera-relative WASD, view aiming,
crouch and jumps all work in it. Swimming and jetpack flight return to third
person. An isolated first-level route (street, club door fight, doorway,
entry hall, main room) passed all 14 checks at 59.9 fps with 0 underruns;
Vanilla route exit 0. Known limit: steep-angle wall contact can drop wall
polygons (new job D11B). Hands and weapon: D12. Details:
documentation/61-d11-first-person.md. Binary
`dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5`.

**D10 Done (user: "it's done, fully accepted"); D08S cancelled (may revisit).** Modernized
camera: **V** recenters (smooth swing to the original follow angle; the mouse
cancels it), **H** cycles centered / right / left shoulder (off by default),
and Alt+wheel distance plus shoulder side are now saved in the profile (schema
12; `--camera-distance`, `--shoulder`). Shots still meet the crosshair and walls
still constrain the camera. On foot Duke already faces the view, so V mostly
levels pitch. Details: documentation/60-d10-camera-polish.md. Binary
`84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`.

**D08R Done (user-accepted: "it's absolutely rock solid").** **D08Q1 Done
(user: "verified working!!!"):** Modern jetpack Ctrl descent 2800 -> 5500, steady 29.5
units/frame to match the underwater Ctrl dive; soft landings, no damage.
Binary `9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`.

**D08R selectable jetpack scheme (Done).** `run.py --jetpack
classic|modern` (persisted, Modernized, default modern). Classic (revision 2,
after user feedback) keeps all modern controls - mouse look, camera-relative
WASD, J - with the original burst physics: Space boost, gravity, no host
hover or Ctrl descent. Details: documentation/57-jetpack-controls.md (D08R
section). Binary
`1a8907148aadaa80902b34c644f61631c627845739cf6f525590666d197f03c2`.

**D23B Done - intro FMV stutter was a stranded native movie shard.** The
shard's cache folder is keyed by the `game.local.toml` hook list; the 28 Sep
hook additions moved it, so the intro decoder ran interpreted (~52 fps,
continuous underruns). Rebuilt: 59.92 fps, 0 underruns; the user confirmed
smooth playback. The build preset and `run.py` now re-check the shard on every
build/launch (~0.2 s), and the session log says `ttk-fmv: native movie
decoder active` or warns. Details: documentation/59-fmv-shard-namespace.md.
Binary `3d370c02d4706e710eb3b1ef4f9e55930c49129fa8afc86e054c3157c39d0161`.

**D23A Done (user-accepted) - the Modernized stutter/slowness had a measured
cause and is fixed in the current build.** Isolated probe: Modernized ran
at 47.5–49.3 fps with continuous audio underruns (output fill 17–34 ms
against a 180 ms target) while Vanilla ran 60 fps clean. `ttk::identity()`
re-read all 106 guards (81 KB) word-by-word through the guest bus on every
call — 46 calls per frame, 108 µs each, 5 ms of every 16.7 ms frame (24 %
of wall time). It now memcmps the runtime RAM image once per host frame
(re-verifying immediately on any RAM-code generation change) and costs
28 µs per frame. Result: 59.94 fps, 0 underruns, fill 267 ms; jetpack flight
60.1 fps. Details: documentation/58-modernized-frame-budget.md.

**D08Q Done (user-accepted) - modern jetpack flight.** "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful." User: in the jetpack only
Space worked, WASD were "blocked", mouse dead. Original mode 10 had no lease;
WASD reached it only through the tank fallback relative to an unturnable body,
and its idle gravity landed Duke within seconds. New host layer
(`recomp/src/ttk/jetpack.inc`) over the unchanged original handler: mouse
orbit with the body facing the view (so the original body-relative thrust is
camera-relative), W/S/A/D → original Up/Down + L2/R2, Space climb (original),
release everything → hover (original hover lock at the current height), Ctrl
→ steady descent to a soft landing, J in the air → original cut-out fall with
the camera still live. Probes: headings within 4° of the camera, level flight
±25 units, 25–29 units/frame forward/back, Ctrl 489 units/30 frames then
landing, ground controls resume. Known: stop-on-release is immediate; after
Ctrl release Duke settles ~130–300 units more near a floor; fuel drains while
hovering (original rule). Details: documentation/57-jetpack-controls.md.

Binary `81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`.

**D08P Done (user-accepted) — area-wide control loss was overlay identity;
mid-depth wade now runs at land pace.** The user's session log showed `ident=0` from the
flooded-corridor ledge onward. The LEVEL00 zone script writes a hit position
into the overlay's trailing 16-byte scratch vector (`0x800ccf1c`), which the
9668-byte overlay guard covered, so identity failed permanently the first time
that script fired. The guard is now the 9652-byte code/table body. Separately,
that water is depth 512: the original swaps every gait for wade clips 80/81
(Vanilla 8–14 units/frame, forward only). The wade handler `0x800539f8` is
now hooked (new generated entry) and its root retargeted to camera-relative
WASD at the land run band: 45–58 units/frame, mouse steering, strafe, Space
jump; 0 identity refusals across and past the ledge. Wading straight into a
wall idles as in the original; turn the view to resume. Details:
documentation/56-scripted-camera-controls.md (iteration 6).

Pending playtest jobs D07A, D07B, D08A, D08H and D10 distance were **Done** on
user acceptance of ongoing play. D08N scuba remains cancelled. D10 broader work
stays In progress. D08B is Done on 2026-09-29 after the user playtested all
of its recorded limits (see the job board work log).

Previous binary `79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`
(D08P acceptance).

```sh
python3 /home/spartacus/CODE/duke-nukem-time-to-kill-recomp/recomp/tools/local/run.py
```
