# Job board history - September 2026

Dated session log entries from 2026-09-27 to 2026-09-30, moved verbatim from
`MODERNIZATION_JOBS.md` on 2026-11-08 to keep the board under the repository
file size limit. Job descriptions and statuses remain on the board.

## 2026-09-27 — D08G1 exact wording

Changed only success strings in `cheats.inc`. Private Xvfb/cards/profile run
`iteration40-wording-baseline` reviewed all ten result captures, including both
god/monster directions and dnstuff/dnitems. Added debugging-code and refusal
messages retain their distinct wording. D08G1 Done; D07C follows in selected order.

### D07C implementation — 2026-09-27

Reproduced held-aim facing separation and the rejected RPG caller before fixing them.
View input, supported weapon callers and the energy post-constructor now share
physical view aim. Base-family held/unheld, pitched RPG flights/explosions, sustained
streams and isolated upgraded-dispatch fixtures pass. Native input/controls/aim
checks and 57 Python checks pass. Needs playtest for feel and broader campaign
coverage; see [implementation evidence](40-feedback-implementation.md).

### D08H implementation — 2026-09-27

Reproduced concealed pickup, contact-jump failure, switch misses and small-drop
slowdown in isolated original apartment routes. Fixed animated-foot obstacle
clearance, bounded approach-speed inheritance, the concealed callback and the
original switch ray intersection. Fresh concealed/exposed pickup, repeated switch
angles, subsequent NPC conversation, furniture drops/jumps, release, large falls
and ladder/redraw checks pass. Needs playtest for subjective feel and broader
campaign coverage; original collision and secret progression remain authoritative.

### D19A and final verification — 2026-09-27

Done for the existing message-renderer scope. SHA-pinned local PNG assets generate
a deterministic pack with original palette/alpha, source credits and explicit
mapping/fallback for small and Atomic styles. All ten exact confirmations and
three window sizes reviewed in OpenGL and Software; transparent compositing,
integer glyph scaling, wrapping, expiry and post-resize gameplay pass. Vanilla
retains original presentation. 60 Python tests and five native suites pass;
missing/corrupt assets fall back. All private sessions closed; player files and
original archives unchanged. Current binary and detailed limits are recorded in
[implementation notes](40-feedback-implementation.md).

### 2026-09-27 — User playtest acceptance and next planning pass

D07C accepted for tested aiming/RPG behavior. D08H bed/couch entry and smooth
walk/run departure explicitly accepted; hidden pickup and switch/NPC human checks
remain unreported. Added D08G2, D08I, D08J, D08K and D08A1 for the next focused pass;
D19B is deferred with modern menus. Read-only EDuke32/local source investigation
confirms bracket cycling exists but lacks selection feedback. No runtime edits,
builds, gameplay input or settings/card writes this turn. See
[feedback, source evidence, acceptance and continuation prompt](41-playtest-follow-up-plan.md).

### 2026-09-27 — Selected traversal and inventory feedback delivered

D08G2, D08I, D08J and D08A1 move to **Needs playtest**. Silent centered cheat results
and a temporary inventory strip pass both-renderer/resize/input checks. Inventory
shares original menu selection and charge percentages; cycling never activates.
Run-start misses were traced to the first-stride gate and replayed successfully in
two 18-case sweeps. Separate small-edge grace is bounded to six input frames and
original clearance. Armed ladder failures were reproduced before fixes: E-owned
stow needed its final blend, and weapon-switch intent needed stable-state gating.
Pistol/shotgun first/second ladder, switching, redraw and ammo checks now pass.
Accepted bed/couch entry and departures were replayed, including actual airborne
release. Original contact, animation events and collision remain authoritative.

D08K moves to **Blocked** after inspection of original animation/root/skeleton
commands and collision paths: existing movement is full-body rolling, with standing
movement clearance and no validated low-gait override pipeline. No misleading slow
roll or sliding idle was substituted. See [audit](43-crouch-walk-audit.md).
D19B remains deferred with modern menus.

Normal regeneration/build and ordered patch stack, six native suites, 60 Python
tests (2 conditional skips) and the isolated 14-checkpoint Vanilla route pass.
All private games closed; all 11 player configuration/card/state hashes unchanged.
Original assets and unrelated local work preserved. See
[implementation, evidence and limits](44-traversal-inventory-implementation.md)
and [resumable handoff](42-traversal-feedback-progress.md).

### 2026-09-27 — Speech-to-text playtest accepted; next pass checkpointed

D08G2 and bounded D08J are Done by explicit user acceptance. D08I remains Needs
playtest for the close-bed Shift-preheld W+Space vertical-jump case; broader jump
responsiveness is praised. D08A1 design accepted, activation follow-up selected:
user explicitly requests contextual Enter use without pause, retaining ordinary
Enter pause elsewhere, U and custom bindings. D10A turning and D18A audible crackle
investigations added as Todo; neither cause nor regression is established.
Next order: D10A, D18A, D08I, D08A1. D08K blocked, D19B deferred. Documentation-only
checkpoint: no runtime changes, builds or game interaction. See [precise reports,
acceptance and restart state](45-playtest-turning-audio-follow-up.md).


## 2026-09-27 — Selected D10A / D18A / D08I / D08A1 follow-up

Worked in the selected order with private Xvfb displays/cards/preferences. Full
evidence and resumable checkpoints: [46](46-turning-audio-inventory-progress.md).

- D10A: 64 sweep comparisons (16 street, 24 mixed club/street, 16 focused
  interior and 8 interior while firing) preserved requested yaw, including Shift,
  bump states and held precision input.
  No supported turning repair established; physical mouse/compositor feel remains
  Needs playtest. Accepted aiming/collision paths unchanged.
- D18A: real PulseAudio monitor/runtime PCM captured. Club playback showed output
  starvation, worse during gunfire; full-scale output clipping was not observed.
  Earlier-stage clipping/source artifacts are not excluded. Apartment no-poll
  segments had no starvation; matched comparisons are recorded in note46. No
  speculative buffer/mixer/speed change. Investigation remains open.
- D08I: 19 valid close-bed comparisons retained the original rising-clearance
  handoff and landed forward. One non-contact attempt is excluded. No lost
  direction reproduced, and no traversal change made; Needs playtest with the
  reported exact hardware/geometry/timing. Six-frame edge grace unchanged.
- D08A1: reproduced visible-strip Enter opening pause. Implemented contextual
  Enter through existing item_use, retaining ownership until key release so
  timeout/repeat/focus cannot turn a held activation into pause. U/custom bindings
  and ordinary Enter elsewhere preserved; use hint displays the actual binding.
  Full GL/default and Software/O-P-L checks and native guards pass. Needs playtest.

Preserved accepted aiming, furniture, ladder transfers and silent cheats. D08K
remains Blocked; D19B remains deferred. No player settings/card, asset or unrelated
source edits. No commits. Release/test identities and remaining coverage are in46.

## 2026-09-27 — Playtest acceptance; D08A1 strip; D08J aim-held stow

Player accepted D08I bed jumps and D10A turning feel; Enter inventory use confirmed.
D18A remains In progress (no crackle heard this session; club starvation unrepaired).
Implemented silent empty inventory and tiles-only strip (D08A1 Needs playtest).
Reproduced armed second-ladder miss when right-aim was held across takeoff: flight
lease was skipped so E stow never finished; fixed by owning flight_valid while aim
is held without redirecting aimed jump arcs. Private aim-before-jump attach recovered.
Added backlog D08L / D11A / D18B. D08K blocked; D19B deferred. Evidence:
[documentation/47-playtest-iteration.md](47-playtest-iteration.md).
Binary `57380caedddaf28621378dcc241886a234bff85dc1dd97882f6c6d50e55365cf`. No commits.

## 2026-09-28 — D08J armed stow reliability; D08A1/D18A closed; F unbound

Player accepted inventory strip and closed audio crackle (reopen if it returns).
Armed ladder grabs still felt harder than holstered. E-owned stow now 4x on
ground and in flight; jump+E refreshes lease; shorter airborne holster pulse.
8x boost rejected after it skipped upper events. Private early-E and nominal
armed attaches passed. Default DisplayPerf/F binding removed; D26 console Todo.
Evidence: documentation/48-armed-ladder-reliability.md.
Binary 42eeaf3dc31a7f7a94ae9201e0c47f32cb23c1680edcfca994acb5cb20eccbab. No commits.

## 2026-09-28 — D08J Done; plan D27/D28/D08A2/D26

Player accepted armed second-ladder grabs. Documentation-only plan in
documentation/49-controls-hud-plan.md. Added D27 (RUN MODE + Shift clunk), D28
(Scroll Lock holster + WEAPON LOWERED/RAISED), D08A2 (EDuke32 bottom-left icon;
Auto not Otto). D26 remains FPS via backtick console; Scroll Lock is holster only.
No runtime/build this checkpoint.

## 2026-09-28 — Queue D27 → D28 → D08A2 → D26 delivered

Implemented the planned controls/HUD queue. D27: Caps Lock RUN MODE quotes;
Modernized gait soft-switch removes Shift-run restart clunk. D28: default holster
Scroll Lock + WEAPON LOWERED/RAISED; profile v8 migrates C; captured C no longer
hard-holsters. D08A2: bottom-left active gadget + green %-tiles 3010–3021 (Auto for
biomask). D26: backtick console with `fps`/`help`; F unbound. Native input/controls/
inventory tests and profile unit tests pass. All four **Needs playtest**. Evidence:
documentation/50-controls-hud-delivery.md. Binary
57c2b097dd98e28473fc644a20d9d3a2730deb32ecbca0db1d0465e8564646ea. No commits.

## 2026-09-28 — Hotfix stiff gait + inventory/%/kerning

User reported frozen stiff posture on walk→run (D27 soft-switch). Restored frame-0
gait restart. Inventory inset into 4:3 frame; % uses tile 3076 green-swapped inside
the icon; centered message advance tightened by 1. Clunk silence remains open.
Binary e3e517beb5606ac35e05fb69236cbde357f639b65a8568e2e9f518f9f5eaa964. No commits.

## 2026-09-28 — Inventory: strip + icons; drop bottom-left; M quote

User: restore switcher, remove bottom-left host gadget, M = medkit % message only.
Strip centered again with research/inv icons (2460/2461/2467/2468). D08A2 scope
revised. Binary 6d3cdd09073b895427d684459f1b6851796c2accc348f0d8ba4051f6c82f8dd9.
No commits.

## 2026-09-28 — Inventory strip polish + EDuke crosshair (session complete)

Player-tuned frame locked at 50×60 / frame_dy=-6; green palette-22 digits/%;
Enter=use / Escape=pause; direct keys skip switcher; CROSSHAIR tile 2523 with
alpha blend. D08A2 revised scope marked Done. Findings:
documentation/51-session-inventory-crosshair.md. Binary
f09b657e31ded01e987d4b39695d469c452bdda621f6f6aa68dd01ebf8edec97. No commits.

## 2026-09-28 — Late polish lock-in (session complete)

Quake drop-down console + Messages smallfont (quotes and console). Owned copy at
recomp/assets/fonts/Messages (no research/ build links for that art). I crosshair
toggle fixed (was outside weapon_previous edge loop); Right Shift remains Select
inventory. Findings: documentation/52-session-late-polish.md. Binary
2afe9956c6c06120cf840db2263ce7619e32827dd158c26dba14c25ee3362c78. No commits.

## 2026-09-28 — Console scrollback, swim bridge, inventory quotes

D26: console echo/scrollback; help multi-line; fps persistent top-left debug block;
no toast flashes for command feedback. Crosshair halved. D27: plant-aligned gait
switch (no +0x68 restart). D28 Done (user accepted). D08M interim underwater
WASD→D-pad. Findings: documentation/53-session-console-swim.md.
Binary f0c5ad2ab9a52e6d26353630916f3ba5272c7c964b3907f77230b8475da5c890. No commits.

## 2026-09-28 — Console/crosshair Done; Shift gait revert; clunk deferred

D26 Done (user signed off console). Half-size crosshair + I toggle accepted.
D27: restored immediate frame-0 walk↔run (plant-aligned delay rejected — lag +
clunk remained); Caps Lock quotes Done; Shift-run SFX silence deferred low-priority.
Bio Mask/scuba left for later user review.
Binary 076945b35e06edc1d7e978ac8b7876d856ebe74a3cf4855e5c511b217fc549a8. No commits.

## 2026-09-28 — D08M camera-relative swim redesign

Modern swim adapter (`swim.inc`): look-yaw W via face+D-pad bridge, underwater
look-pitch vertical, Space rise / Ctrl descend, surface horizontal clamp,
W+Space mantle (anim 96, weapon-agnostic, generous probes). Camera orbit kept in
water. ttk-controls-test + ttk-input-test PASS. Needs water-map playtest.
Findings: documentation/54-swim-redesign.md.
Binary 6edc846d4f7bede4d4b452a39a20d18ffa8a3dea62155dee33c0e1fc961cadc6. No commits.

## 2026-09-28 — D08M wade jump + deep swim root-cause fix

Wade: short Space → standing prep 96 (suppress Cross→176 dip); no host VY overlay.
Deep swim: D-pad bridge no longer blocked by camera lease; host look-relative
velocity; Space/Ctrl = VY only (no body flip); lip mantle tightened.
ttk-controls-test + ttk-input-test PASS. Needs subway/sewer playtest.
Findings: documentation/54-swim-redesign.md.
Binary 5bd15215ba61c4b7b3b9ffc4f0044853093ce4d69d12a0c2d091f2eba203f04a. No commits.

## 2026-09-28 — D08M swim locomotion redesign

Stopped host world-space root/ballistic fight with tank `8005201c`. Free swim:
face_view + WASD→D-pad horizontal; host +0x1f8 vertical only. Fixed deep/wade
classification (WASD bridge). Wade jump uses land-like anim 96 + 8003e2d0.
ttk-controls-test + ttk-input-test PASS. Needs subway/sewer playtest.
Findings: documentation/54-swim-redesign.md.
Binary 4304407bf979aaf9f03af20c32bfaeca2b58c9b399f6c3594c41604a6b14d075. No commits.

## 2026-09-28 — D08M swim playtest iteration (strafe/vertical/mantle)

Fixed Forward-inject killing A/D/S in free swim; Space/Ctrl via host Y step;
land-scale wade/mantle impulse; friendlier Space lip exit; removed SCUBA toast;
added backlog D08N (Duke3D scuba + HRP). ttk-controls/input tests PASS.
Findings: documentation/54-swim-redesign.md.
Binary f8ec18f8f7c10abb9be81c145c55dbe83bdafae7f4ee83209d80b76fd0f6a9db. No commits.

## 2026-09-28 — D08M swim playtest iteration 2 (class/strafe/jump)

Deep water misclassified as wade → Forward inject + dead vertical. Fixed free-swim
gate; airborne 98 + reasserted −11000 for wade/mantle; softer Space lip exit.
ttk-controls/input tests PASS. Needs sewer/shallow retest.
Findings: documentation/54-swim-redesign.md.
Binary a93d96a3b081525ab23c60cae4435b8490b72a55a065006d1496ac12cfc9d6a1. No commits.

## 2026-09-28 — D08M swim playtest iteration 3 (no leap-loop)

Reverted anim-98/−11000 exit hop that leap-looped deep water. Wade = land −9000;
Space alone rises; mantle = Space+forward+ledge. ttk-controls/input PASS.
Findings: documentation/54-swim-redesign.md.
Binary 758b90753c165855f2096535f9c3f43c90c9093ce37fc34f43a5adff41298119. No commits.

## 2026-09-28 — D08M swim research rewrite (Square / 8003e2d0)

Deep review: Space→Square is original swim thrust; host vertical fought it
(surface hop). Wade `8003e2d0(96)` invalid (index anim−97). Free swim keeps
Square; wade/mantle use 97|98. ttk-controls/input PASS. Needs playtest.
Findings: documentation/54-swim-redesign.md.
Binary 561a9357fcaa585fb083fe78533c9f4c3d977d9905ec0fe7b1ac714b822671e7. No commits.

## 2026-09-28 — D08M upright free swim (no Square flip)

Shallow jump accepted. Free swim suppresses Square (176 upside-down); host
Space/Ctrl vertical; ledge exit W+Space; wade directed XZ boost + water holdoff.
ttk-controls/input PASS. Needs deep-water + subway ledge retest.
Findings: documentation/54-swim-redesign.md.
Binary 359ad05710bd7b8542f76f4696ad2d57c2ad170b5d12de0d2b55f434a03f8d09. No commits.

## 2026-09-28 — D08M free-swim rewrite (kill hop loop)

False mantle: headroom counted as ledge + held Space+W → airborne 98 hops.
Pure Y vertical; mantle needs fresh press + standable ledge; Ctrl dives from
surface. Wade jump preserved. ttk-controls/input PASS. Needs deep-water retest.
Findings: documentation/54-swim-redesign.md.
Binary 4cdb813c6d0e50eead6aadbfb316d9379318e314bfee31a9faefcf9f76f61201. No commits.

## 2026-09-28 — D08M host-owned free swim + close ledge

D-pad tank swim dropped (no strafe / sliding idle). Host WASD+Space/Ctrl.
Shallow Space-facing-ledge hop. ttk-controls/input PASS. Needs retest.
Findings: documentation/54-swim-redesign.md.
Binary 13ec110b1af63f84b98357646bfb9713462c02d96cd158d967d061ec3de41def. No commits.

## 2026-09-28 — D08M water-state recovery (abort host position swim)

Host XZ/Y + forced water flag + clearing fall VY froze entry mid-air and left
oxygen/swim-anim above water. Restored original water ownership + D-pad/Square.
Wade ledge jump preserved. ttk-controls/input PASS. Needs deep-water retest.
Findings: documentation/54-swim-redesign.md.
Binary fa4f9ca8f2c85a089aeff6cc2653a4aa3723f27042ff24c7096e76bfe4937759. No commits.

## 2026-09-28 — D08M research artifact + root-bridge swim

Wrote documentation/55-swim-controls-research.md. Free swim: Square suppressed;
soft Y ascend/descend; D-pad+stick+root bridge; ledge anti-bounce. No world XYZ
or invented water flag. ttk-controls/input PASS. Needs subway/sewer playtest.
Findings: documentation/55-swim-controls-research.md, 54-swim-redesign.md.
Binary 47ead3263582cdcb6d7eb397686de924d4f628e46310619f286044754a2b20dc. No commits.

## 2026-09-28 — D08M closed / D08O next (docs only)

User playtest: ledge Done; Space↑ W/S E-mantle OK; A/D and Ctrl↓ fail; simplify
exit to mantle-only. Stood up 55/54 solidly; D08M → Done (foundation); carved
D08O Todo as NEXT PICKUP (strafe, Ctrl dive, mantle-only exit). No swim.inc
changes. Manual/status/handoff/matrix synced. No commits.


## 2026-09-28 — D08O implemented (Needs playtest)

Root-caused deep free-swim gaps on 8005201c: A/D were D-pad Left/Right (turn
71/70) and stick-X (turn); strafe is the layout's L2/R2 words (800d1a68/1b80 →
anims 88–93). Free swim now injects layout-resolved strafe bits and no longer
writes stick X. Ctrl dive gated on floor margin (+0x1c8−0x1e0) instead of
"already >0x100 under surface". Free-swim Space ledge exit removed (E/mantle
only); wade jump 8003e2d0(97|98) untouched. ttk-controls/input PASS; shallow
subway regression clean (W/S/A/D, Space 98, Ctrl 105, no turn). Deep water
unreachable from the savestate → Needs playtest. Binary
d72fa5f82575b6e45e929947795d3b0b616aab76a7466d301d1d8798c83c121e. No commits.

## 2026-09-28 — D08O.2/.3: native dive + wade ledge assist (Needs playtest)

Playtest: A/D strafe, Space↑ and mantle good; Ctrl still dead; subway shallow →
platform jump unreliable. (1) Ctrl: the original's native swim states (800455bc,
+0x22c 4/5) pin body Y to the surface (80044c18), so soft-Y can never dive; Ctrl
now calls the original dive 80045564 (state 5 / anim 133) and underwater the
host steers body yaw/pitch camera-relatively from WASD/Space/Ctrl and injects
Square for the original thrust; auto-surface unchanged. Verified in deep zone 35
by teleport (dive, 4-way thrust, look-up rise, Ctrl descend, Space surface).
(2) Ledge: measured 98 rise ~560 vs ledge 635; wall-touch cancel, 300–600 bump
107, only 700–1250 landed; probe reach ~350. New swim_ledge_ahead (standing probe
from 0/320/640/960 along view, XZ restored) + two-phase 97 → 98 switch when the
foot clears the ledge top, 97 VY kept via delayed_vertical, XZ speed scaled to
distance. 24/24 lab jumps landed (wall, 350–1300, run/stand, Space/W/A, ±53°).
Plain wall + W+Space → vertical hop instead of splash cancel. ttk-controls/input
PASS; shallow + deep regressions clean. Binary
71d2a1394ca4dad6c6a1e6fe9c1347cdb7eec8ecdacb3fbd54f9009a604edec5. No commits.

## 2026-09-28 — D08O.4: ledge jump back to the plain 98 arc (accepted → D08O Done)

Playtest accepted the swimming (Ctrl dive, underwater WASD/Space/Ctrl) and the
Bio Mask icon, but the 97 → 98 ledge hop looked far too high; user wants the
realistic simple jump of the previous cut, fixed if possible. Reworked: Space at
a ledge launches directed 98 as before, with VY −9000 via delayed_vertical at
the 8003ebf4 hook (98 integrates ≈VY/96 per tick → rise ≈840 vs 560), XZ owned
per tick by swim_ledge_ballistic: held at 0 until the foot clears the lip when
within one probe step (0/320) of the wall (98's first stride into the face was
the 107 bump / frame-one cancel), then 3400/4000; from further back 1900 +
3.5/unit (max 6400). Probe scan extended to 1280. Fixed a stale swim_ledge_pending
surviving a landing (now drops on anim ≠ 98 / state ≠ 9). Tuning trail: −5600
apex 90 short (0/9); −9000 + 2.7/unit 8/9 (oblique too far); +1280 scan 8/9
(1280 bucket under-speed); 3.5/unit 13/15 (near-oblique first-stride bump);
near-bucket hold 15/15. ttk-controls/input/inventory PASS; shallow (W/S/A/D,
Space 98, Ctrl 105) and deep (dive, thrust, rise/descend, auto-surface, surface
strafe) regressions unchanged. Binary
78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb. No commits.

## 2026-09-28 — D08O closed (playtest 3 accepted)

User: "everything here is confirmed working" — swimming (strafe, dive,
underwater controls, mantle exit), the round-3 ledge jump and the
`biomask-small.png` item-switcher icon. D08O → **Done**; docs 00/09/54/55 and
GAME_MANUAL updated to accepted. No code change; binary
78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb. No commits.

## 2026-09-28 — Bio Mask item-switcher icon (asset only)

Replaced the Duke3D scuba tile (2468) in slot 2 of `recomp/assets/ttk-inv-icons.pack`
with the user's own Bio Mask art `research/inv/biomask3.png` (17×14, cut from
`research/inv/biomask-asset/biomask1.png`); entry tile id 0 = custom art.
Provenance JSON updated. Pack format/loader unchanged; ttk-inventory-test PASS;
offline strip render shows the mask in the Bio Mask cell. No binary change
(runtime asset copied by the `ttk-inv-icon-assets` target). No commits.

Update (same day): slot 2 now carries the user's revised art
`research/inv/biomask-small.png` (16×13, tile id 0); provenance JSON updated;
ttk-inventory-test PASS; no binary change. Accepted by the user the same day. No commits.

## 2026-09-28 — D08N cancelled (scuba item out of scope)

User directed that a scuba device must not be added. D08N moved from Todo to
**Cancelled**. No code or asset change: there was no scuba prototype. TTK
underwater air stays original and automatic; Bio Mask stays the gas mask.
Player manual, status/handoff, swim note 55, and the D16A HRP note now say
scuba is not a future item. Historical work-log line that added D08N remains
as history. Binary unchanged
`78b68e42ab05ad831a6668c8b4a084413f014a449b08bd74bb09e776059e75bb`. No commits.

## 2026-09-28 — Playtest jobs accepted; D08P turret control recovery

User accepted all remaining Needs-playtest jobs from ongoing play: D07A, D07B,
D08A, D08H, and D10 distance. D08B/D10 broader work stays In progress. D08N
scuba stays cancelled.

## 2026-09-28 — D08P turret wade: depth gate, strafe fallback, Escape recapture

Playtest of the first tank-WASD fallback: mouse dead, A/D turned, Escape
dropped Modernized, crosshair gone, water felt “too deep”. Isolated load of
host savestate slot 1 (player cards not written): depth `0x100`, anim 63,
normal camera, `+0x224` without `0x20000000`. Host free-swim used `0x200` and
treated anim 70 (D-pad turn) as underwater. Free swim now matches original
`≥0x281` / states 4–5; A/D fallback is L2/R2 strafe; Escape recapture from
`0x8005a210`; reticle follows camera-only locomotion. ttk-input-test,
ttk-aim-test, ttk-controls-test PASS. Needs playtest. Binary
`5cf66b4e2469b0045265e47135a1bdc305d2ae842edc3603caf283e8d78fed56`. Note:
documentation/56-scripted-camera-controls.md. No commits.

## 2026-09-28 — D08P F7 slot 2: recapture after savestate load

Playtest of iteration 2 from F7 UI slot 2 (turret around the corner): modern
controls gone entirely. Isolated dump of `state_800AB6FC_slot01.pst`: same
wade as before, apartment words already patched. F10 had cleared
`initial_capture`; F7 released capture and did not set it again; the F7 menu
could consume `capture_offer` while `allow_capture` was false. F7 now
requests recapture like Escape even if the mouse is already free; offers
are kept until capture is allowed; identity remaps the complete patched
LEVEL00 pair. ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Needs
playtest. Binary
`48e5c25f282d65741a94fd69eb58fc5409f7ac1fcf3282cbdf9bab9604e87336`. Note:
documentation/56-scripted-camera-controls.md. No commits.

## 2026-09-28 — D08P identity reader stall

Playtest of the slot-2 recapture binary: gameplay and audio ran slow.
Apartment pair classification ran on every guarded identity word, many
times per frame. The check is now only on `0x800cc57c` / `0x800cc580`.
F7 recapture and slot-2 overlay identity are unchanged. ttk-input-test,
ttk-aim-test, ttk-controls-test PASS. Needs playtest. Binary
`44abdda343cc0014f3bfd6602cb42211d8807e00f2f5d57d30a62eac672b5d4c`. No
commits.

## 2026-09-28 — D08P waist-deep water as land locomotion

Playtest after the identity-stall fix: speed was normal, but the turret
wade crawled, mouse look died in the water, and after leaving only walk
worked (no Shift). Isolated slot 2 already uses land run anim 78 on
forward; Modernized failed `state()` on tank turns 70/71 and injected
Walk. Waist-deep / mid water now keeps the land camera lease, faces the
view, converts 70/71 to the run gait, and does not inject Walk. True deep
swim unchanged. ttk-input-test, ttk-aim-test, ttk-controls-test PASS.
Needs playtest. Binary
`b642ab9fbfdac845a0e3c23157b3e2550f1fde520d4351b2b2585b4764414ecd`. No
commits.

## 2026-09-29 — D08P accepted

User playtested F7 slot 3 (ledge) with binary `79a3fd5b…`: "it really
works. perfectly." D08P → **Done**. Documentation committed and pushed.

## 2026-09-29 — D08P overlay scratch identity; mid-depth wade handler owned

User: the whole ledge area (F7 UI slot 3) loses run, mouse and jump for
good; depth is irrelevant. Their session log: `[TTK lease] inactive
(identity)` from the ledge on. Live per-guard diff on the isolated slot 3
copy: only `0x800ccf1c/20/24` changed — the LEVEL00 overlay's trailing
scratch vector, the output of the zone script's `0x8007177c` hit test
(overlay `0x800cb1a4`). Overlay guard trimmed to the 9652-byte code/table
body (`ttk_state_probe.py` too); controls test proves the vector write keeps
the lease and the last table word is still guarded. Same water is depth 512:
the resident dispatcher swaps every gait for clips 80/81 (Vanilla measured
8–14 units/frame), handler `0x800539f8`; the track advance writes the root
after the player-update hook, so the handler entry is hooked (added to
`mod_function_entry_funcs`, regenerated: one generated line; `build.py`
stops at the pre-existing runtime patch-stack drift, so generation ran
directly with the built recompiler). Root retargeted to camera-relative WASD
at the land run band; guards `0x800486b0`/260, `0x800539f8`/452,
`0x800788e0`/200; 80/81 join the wade-jump set. Live: 45–58 units/frame,
steering, strafe, jump 98, 0 refusals, turn/run/jump after the zone; slot 2
clean; wall stop while wading is the original handler (turn resumes).
ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Needs playtest. Binary
`79a3fd5b4cd7eb535d472089e680982516100fc65b1a00c3a5dd546b85ea527a`. No
commits.

## 2026-09-29 — D08P held keys across recapture; fresh capture offers

User: "still exactly the same issues" on iteration 4. Live Xvfb + xdotool
reproduction (isolated slot 2 copy; software and OpenGL; debug load and the
real F7 menu; the user's profile) shows the wade at dry-ground run speed
with mouse look, so the wade path is sound; slow headings beside the spawn
are the original wall slide. Reproduced: a Shift/W physically held through
F7 load, Escape/resume or F10 was wiped by `clear()` at capture → walk gait
with Walk pad (10.7 units/frame) until Shift was re-pressed. Also found:
Escape's release offered recapture in the same update, so the original
pause menu was captured (Enter/X swallowed). Capture resyncs bound
keys/mouse buttons from SDL device state; offers expire after 8 frames;
sustained tank fallback shows `ORIGINAL MOVEMENT (reason)` /
`MODERN MOVEMENT RESUMED`; `run.py` mirrors stderr into
`recomp/build-local/logs/session-*.log` (`--no-session-log` opts out).
Verified live after the change (Shift through F10: 76 at 50.6; Shift+W
through Escape/resume: paused uncaptured, 78 at 53.9 on resume; mouse
after resume). ttk-input-test, ttk-aim-test, ttk-controls-test PASS. Needs
playtest. Binary
`f8122f58e828a35b39fd11708195ccb5901b24e7e5207e593446b70b658564fb`. No
commits.

## 2026-09-29 — D08Q modern jetpack flight controls (Needs playtest)

User: in the jetpack "i cannot control it at all aside from spacebar to
ascend. wasd are blocked, modern controls fail here completely". New job
D08Q. Baseline probe on `79a3fd5b…` confirmed: original mode 10 (anims
163–170) had no lease, so no mouse; WASD only via tank fallback relative to
an unturnable body heading; idle gravity landed Duke in ~1–2 s; Ctrl inert.
Research (read-only): entry `8004aaf8` (Square + `358&3==3` + fuel), handler
`8004ade0` (body-space thrust, Square lift, Up/Down/L2/R2 thrust, hover
lock `0x08000000` with base `+0x860`, bit 31 = no gravity while any input,
`8003ea58` integration, `8004ac08` root apply with the floor probe), cut-out
→ anim 108 + `8003ff6c`. New `recomp/src/ttk/jetpack.inc` (host layer from
the `8005a210` hook): camera-only lease for mode 10 and the 108 fall after
cut-out, `face_view`, idle → hover lock at current height, Ctrl → lock off +
`+0x1f8 = 2800` (~48 units/update, lands via the original probe), WASD →
lock off + `+0x1e8 = 18` trim (level flight), `input_pad()` W/S → Up/Down,
A/D → layout strafe pads, `select_weapon` lets J toggle in flight. Tried and
rejected: driving the hover base `+0x860` (the pin only holds small offsets;
near a floor `8004ac08` pulls toward ~0x200 above it) and trim 29 (sank).
Guards for `8004aaf8`/`8004ac08`/`8004ade0`. Live Xvfb probes (isolated
cards, level 1 turret room + ledge): headings −33/154/−120/61° vs camera
−30°, level within ±25, speeds 25/29/24/14 units/frame, mouse `looks` 0→1,
Ctrl 489/30 frames → anim 105, J off → 108 fall with live mouse → ground
run at 45.7. ttk-input-test (new jetpack case), ttk-aim-test,
ttk-controls-test PASS (controls test needs an original overlay fixture,
e.g. `analysis/pc-input/d08-camera-final/`; the d08p-turret one is
Modernized-patched). Binary
`481dd2ec1c8c33cda1a77542c2a6f06bad845778ee62b1ff7d87e1d4a7a15692`. Docs
57/00/09 and GAME_MANUAL updated. No commits.

## 2026-09-29 — D23A Modernized frame budget: identity guard cost (Needs playtest)

User hit "that stuttering audio/slowness issue" before testing D08Q.
Isolated Xvfb probe with the runtime's own telemetry (`audio_stats`,
`phase_profile`, `phase_hot`, `frame`): Modernized 47.5–49.3 fps, 600–780
underruns per 20–30 s, output fill 17–34 ms (target 180); Vanilla 60.0 fps,
0 underruns, fill 264; Sep‑27 baseline binary 57.5 fps. Counters added to
`identity()`: 46 calls/frame × 108 µs = 5.0 ms/frame = 24 % of wall time —
every call re-read all 20,305 guarded words through `psx_mod_read_word`.
Fix in `code_identity.h` (memcmp on `g_psx_ram` + exact fallback),
`modern_controls.cpp` / `weapon_aim.cpp` (`IdentityMemo` per host frame ×
`g_dirty_ram_code_gen`), `pc_input` (`input_host_frame()`). After: 59.94
fps, 0 underruns, fill 267 ms, 1.00 check/frame at 28 µs (0.17 %); jetpack
flight 60.1 fps; jetpack behaviour probe unchanged. Tests model stores and
code pokes as generation bumps; new memo case. ttk-input-test,
ttk-aim-test, ttk-controls-test PASS. New job D23A (Needs playtest). Note
58. Binary
`81a4a9090fc25cb67a9d7be5c832ce36ebbab67d9c6070d6faa90f8981024d72`.
No commits.

## 2026-09-29 - D23B intro FMV stranded movie shard (Done)

User: stutter "only on the fmv at the beginning" after D23A; "make the
playback more robust ... if we can do it better, we should". Isolated probe
(started in the Cursor session, finished here): intro at ~52 fps with
continuous underruns; `overlay_loader_status` showed `OVERLAY CACHE HASH
MISMATCH` - the runtime read `gc15256f69` but the only shards were older
namespaces. The config hash includes `mod_function_entry` hooks, so the 28 Sep
hook additions stranded the 27 Sep shard. Rebuilt shard: 59.92 fps, 0
underruns, fill 267 ms, interpreter share 30 % -> 4 %; user confirmed smooth.
Hardening: `build_movie_overlay.py --if-ready --quiet` (skip when not
prepared, one-line result, rewrite capture.json only on change);
`ttk-movie-shard` ALL target after `psx-runtime`, added to the `local-dev`
build preset; `run.py` `ensure_movie_shard()` before launch (warns, still
launches); `fmv_poll.c` `note_movie_shard()` logs the decoder state on
change with `overlay_loader_last_msg()`. Verified headless with scratch cards:
current -> "active (19 functions)"; shard moved aside + direct launch ->
WARNING with the mismatch text; `run.py` -> "rebuilt for the current config"
-> active. Found and fixed: four `test_player_profiles.py` launch tests
patched `subprocess.call` while `run.py` now launches via `Popen`, so the
suite started the real game on `saves/local-play/` (cards had 0 occupied
blocks; no savestate touched). They now pass `--no-session-log` and stub
the shard check. 64 local Python tests OK. Binary
`3d370c02d4706e710eb3b1ef4f9e55930c49129fa8afc86e054c3157c39d0161`. No commits.

## 2026-09-29 - D08Q accepted (Done)

User playtest: "Jetpack works great! J to equip it, space to ascend, ctrl to descend, this is beautiful." D08Q moves to Done. Remaining notes in
57-jetpack-controls.md (abrupt stop on release, settle after Ctrl near a
floor, fuel drain while hovering, unprobed ceilings/water/damage) stay as
possible polish, not open acceptance items. D23A stays Needs playtest until
the user comments on in-game stutter.

## 2026-09-29 - D23A accepted (Done)

User playtest: "the in game stutter fix works fine". D23A moves to Done. D18A's historical club starvation
was not re-measured; if it recurs, check `identity_checks` and
`audio_stats.out.fill_ms` first (58-modernized-frame-budget.md).

## 2026-09-29 - Menu system backlogged; D08R narrowed to CLI

User direction: player-selectable options (starting with the D08R jetpack
scheme) should eventually live in a general Sonic 3 A.I.R.-style
customization menu built by hacking the original in-game menu, with a
responsiveness overhaul (D19B). Recorded as the D19 direction. D19 needs a
plan-mode design and a design artifact before implementation; not scheduled
yet. D08R scope revised to a persisted profile setting and `run.py` flag
only. Docs only; no code, build or launch.

## 2026-09-29 - D08B accepted (Done)

User direction: "d08b can be marked as completely done man, its fine. accept
it all", then "i have tested all of them!". D08B moves to Done on user
playtest. The user reports testing every previously recorded limit
(inconsistent contact jumps, residual bed run-off slowdown, early pipe-bomb
pickup from the unopened bed, outer-end/oblique couch entry, ladder ascent
stall with a pig cop on the landing) and accepting the results. No automated
replay or build was run for this change. D08L still covers inertial edge
run-off as a separate feature. Docs only; no code, build or launch.

## 2026-09-29 - D08R selectable jetpack scheme (Needs playtest)

Implementation: `player_profiles.py` schema 11 adds Modernized control
`jetpack` (`modern` | `classic`, v10 and older migrate to `modern` with a
backup); `run.py --jetpack` saves it and exports `DNTTK_JETPACK` (`modern` in
Vanilla); `describe()` prints the scheme. Runtime: `jetpack_classic()` gate;
Classic drops the `jet` camera lease clause, the host hover/descent/face_view
layer and the fall grace; `jetpack_classic_input_ready()` bridges W/S/A/D to
D-pad Up/Down/Left/Right in `input_pad()` (no tank fallback) and keeps the J
shortcut in flight. No terminal-menu entry (D19 owns in-game exposure).

Research: the original flight turns with D-pad Left/Right (live probe) even
though `8004ade0` never reads them; turning is not thrust, so gravity applies
unless Up/Down/Square is held. The original hover toggle table `800d1cf0`
resolves to physical L1 = Modernized walk (Shift).

Evidence (binary
`b588f926cf3cc300bdbf5873a3a0a4e7b78074c3fd0eccbe6acd060361e2da6d`,
isolated Xvfb, scratch copy of the D08Q cards, user's game closed): Classic W
along the body, A/D turn (yaw -421 / +391 in 30 frames), W identical to the
arrow-key D-pad Up, Shift hover holds (dy -28 / 60 frames), Ctrl inert, J cut-out
fall then ground lease, no tank fallback. Modern re-probed matching D08Q
numbers. 68 Python tests, `ttk-input-test` (new Classic pad case),
`ttk-controls-test`, `ttk-aim-test` PASS.

Limits: user playtest of Classic outstanding; fuel-out mid-flight, ceilings,
water, damage and controller not probed. The player's saved settings file
migrates to schema 11 (with a backup) on the next launch. No commits.

## 2026-09-29 - D08R revision 2: Classic keeps modern controls (Needs playtest)

User playtest of revision 1: "with jetpack classic, im having trouble turning
with the mouse ... the controls are slipping in and out of our non-modern
controls ... jetpack classic should basically be that, burst style jetpack with
space to boost ... but the modern controls, wasd, mouse look, should all be
retained". Classic now shares the Modern camera lease, `face_view`, WASD bridge,
J shortcut and cut-out fall grace; `jetpack_update()` skips only the host
vertical layer (hover on release, Ctrl descent, level trim). The D-pad
Left/Right bridge and the lease exclusion were removed. Isolated probe on
binary `1a8907148aadaa80902b34c644f61631c627845739cf6f525590666d197f03c2`:
lease held on every flight sample, mouse turns Duke in the air, W and D follow
the camera, release sinks with no host hover, Ctrl adds nothing, J fall keeps
the mouse, no tank-fallback messages. 68 Python tests and the three native
suites PASS. Classic playtest outstanding. No commits.

## 2026-09-29 - D08R accepted (Done); D08Q1 opened

User playtest of revision 2: "it's absolutely rock solid." D08R moves to Done.
Same message requests faster Modern Ctrl descent at the underwater dive speed;
scoped as D08Q1 (In progress).

## 2026-09-29 - D08Q1 faster Modern Ctrl descent (Needs playtest)

`jetpack.inc` `k_jet_descend_velocity` 2800 -> 5500 (Modern only; Classic
returns before the host vertical layer). Target: underwater Ctrl dive,
~29 units/frame from the D08O deep lab log (`/tmp/swimlab/deep4.log`,
y -1438 -> -888 over 19 frames). Isolated probe on binary
`9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`: steady
Ctrl 29.5 units/frame (5100 gave 27.0), Ctrl+W ~26-33, landing to anim 63 with
health 10000 -> 10000; Ctrl 30 frames dy 955 (D08Q 459). Modern hover, W/S/A/D
headings, mouse and Space re-probed unchanged; Classic re-probed unchanged
(0 lease losses, no host descent). `ttk-input-test`, `ttk-controls-test`,
`ttk-aim-test` PASS. User feel check outstanding. No commits.

## 2026-09-29 - D08Q1 accepted (Done)

User playtest: "verified working!!!" D08Q1 moves to Done. Binary
`9c01cae0183eb90824cc8fe56308871145010a2a243908e66c24a5801bebaf5f`.

## 2026-09-29 - D08S cancelled; D10 selected

User: scratch D08S ("i am so happy with our current jetpack controls now"),
may revisit. Nothing was implemented; the original scope is kept in the job text.
User selected D10. With no specific camera complaint, the user chose these
options: an optional shoulder offset (off by default), a recenter key (not
automatic), and a saved Alt+wheel distance.

## 2026-09-29 - D10 recenter, shoulder offset, saved distance (Needs playtest)

New rebindable actions `camera_recenter` (V) and `camera_shoulder` (H), handled
on press in `pc_input.cpp` and never queued as guest commands. `orbit_begin`
eases the orbit to Duke's heading and the rest pitch (the first seeded original
follow pitch). Any mouse motion cancels the swing. `orbit_constraint` moves the
requested eye along the view's right row by 0.22 x boom (192-640 units),
eased, before the original constraint solve. The look target, matrix and
collision are unchanged; view aim traces from the solved eye. Profile schema 12
adds controls `camera_distance`/`shoulder` and the two actions; migration never
takes a bound input. The runtime writes `<settings>.camera-state` 30 frames
after a change, and `run.py` merges it after exit and at the next launch. Flags:
`--camera-distance original|768..6144`, `--shoulder center|right|left`.

Evidence on binary
`84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`: 71 Python
tests; `ttk-input-test`, `ttk-controls-test` (new D10 case) and `ttk-aim-test`
PASS. Isolated Xvfb probes on the turret-room and ledge savestates: pitch
recenter to rest within about 40 frames, mouse cancel, right-shoulder lateral
637.5/640 (turret room), left pulled in to 544 by the original solve, and
screenshots with no geometry clipping. A right-shoulder shot fired with 0
rejections. Alt+wheel +576 saved; clean exit printed `Saved camera distance
3627, shoulder left.`; the relaunch restored radius 3627. The Vanilla
regression route exited 0. Finding: on the ground Duke's body already follows
the view (D07A/D07C), including while strafing and with original weapon aim.
V therefore mostly levels pitch; its yaw swing matters only where facing
differs. Needs playtest for feel (offset size, swing speed). The broader D10
doors/corners/tight rooms/vertical traversal acceptance remains open. No
commits.

## 2026-09-29 - D10 accepted (Done)

User after about five minutes of play on binary
`84669bb67650eb117aa042b3b12344192403a813618bb5adfe69e8b2c61c7c91`: "it's done,
fully accepted. this is amazing work." D10 moves to Done on explicit user
acceptance. The evidence is that play session plus the automated and
isolated-probe results above (two level-1 rooms). No per-location campaign
playtest of doors, corners, tight rooms or vertical traversal was recorded.
D11 is now ready (D08, D10 Done). No commits.

## 2026-09-29 - D11 first-person prototype (Needs playtest)

Modernized with the independent camera gets an optional eye-level view. **P**
(new action `camera_view`, after `camera_shoulder` so it is an edge, not a
guest command) toggles it; profile schema 13 adds control `view` (third by
default) and the action; `--view first|third`; the runtime side file now
carries `view`, merged by `run.py` like distance and shoulder. Vanilla and the
`original` camera option never use it.

Implementation (`recomp/src/ttk/first_person.inc`, wired from
`modern_controls.cpp`; hooks `0x800348D8`, `0x8002A038`, `0x800B4D9C` added to
`game.local.toml`, regenerated): the eye is Duke's neck joint (joint 9 of the
19-joint model at `[player+0x3c]`) plus 96 up, with only the neck's sway about
the root smoothed. It is fed through the D06 orbit anchor into the unchanged
original camera update. The original 768 minimum distance is relaxed only
inside the constraint call and restored before the update ends (a captured
relaxed value is repaired). The follow easing rates go to 1 so the eye does
not trail. Duke's head joint record gets the original skip bit only between
his draw and the next object-list step. The render's per-frame `SetGeomScreen`
call gets H 256 instead of 386 (`camera[+0x42]` is never written): at 386 a
wall Duke faced went black because the renderer drops vertices nearer than
H/2. A Space-only standing jump keeps a camera-only lease while the eye view
is live (no motion change). Swimming and jetpack flight blend back to third
person; unleased states switch to the original camera as before.

Evidence on binary
`dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5`: 74 Python
tests (new: v12->v13 migration without taking a custom P, optional side-file
view, `--view` env); `ttk-input-test` (P toggle), `ttk-controls-test` (new D11
case: anchor, boom relax/restore/repair, projection argument, head flag
window, lease loss, third person untouched) and `ttk-aim-test` PASS. Isolated
Xvfb route with fresh private cards (`recomp/analysis/d11-first-person/runs/
route-final`): all 14 checks pass - first-level spawn in first person, 7
view-aimed shots with actor hits and 0 rejections, club door waypoints,
doorway threshold sweep with street and club both rendered, entry hall and
main room sweeps, look up/down with no body in view, Space-only jump stays
active, crouch lowers the eye, P to third person and back with original
globals, no fallbacks. Earlier runs: eye within about 25 of the neck while
running; death falls back to the original camera. Frame budget in the club:
59.87 fps third person, 59.94 first person, 0 underruns. Vanilla regression
route `d11-vanilla` exit 0. Movie shard current (native decoder active).

Limits: steep-angle wall contact can still drop or tear the nearest wall
polygons (new job D11B); the original obstruction rays did not report the
touched wall; no hands or held weapon are drawn (D12); switching to an
unleased state is a hard cut to the original camera; field of view fixed at
about 64 degrees (FOV options are D14). Tested on the first map's street and
club interior and the turret-room corridor, not the campaign. Needs playtest
for feel (eye height, FOV, bob) and the user's own route. No commits.

## 2026-09-29 - D11 accepted (Done)

User after playing binary
`dd7b85b49eda500bf5646830dd7fddf4a061986bd3982dda7ae8533507d271c5` for a while:
"i have been playing it for a while, and im insanely happy with it. mark all as accepted. this is phenomenal, and it's like a dream come true." D11 moves to Done on explicit user acceptance, with the
automated route and test evidence above. The recorded limits stand: steep-angle
wall contact (D11B), no hands or held weapon (D12), fixed field of view (D14),
first-map coverage only. D11A and D12 are now ready.

## 2026-09-29 - D11A cancelled

User: "we can disregard D11A, the "p" button works so well for going to first
person mode." D11A moves to Cancelled. No code changed. D11B and D12 remain the
first-person follow-ups.

## 2026-09-29 - D11B / In progress (investigation only, no code changed)

The D11 note's cause was wrong: `0x8002ef90` (outcode bit `0x10` at
`0x8002f1e0`) is a horizontal grid surface renderer, not the walls. The GTE RTP
ring on the isolated club slot shows room geometry going through two
hand-written renderers:
- `0x80011020` world mesh renderer (vertices on a 1024 grid, types 0x60 GT3 and
  0x61 GT4, fog by DPCS, texture animation, light table). It drops a polygon if
  any vertex has SZ 0 (at or behind the eye). Vertices nearer than H/2
  saturate the GTE divide, so polygons get wrong screen coordinates (tears).
  If every vertex is under 0x2000 the polygon goes to a screen-space subdivision
  path (`0x80012960`), which does not fix the projection.
- `0x80010000` object mesh renderer (props); it flags vertices with SZ < H.
Callers `0x8003733c`, `0x80037c4c`, `0x80062db0` pass the render context
`0x800d67a8`, ordering table `0x800d27a0` (2048 buckets) and bitmap
`0x800d26a0`.

Planned fix (eye view only): hook `0x80011020`; for a mesh with unsafe
vertices, render only those polygons on the host with 3D frustum clipping and
subdivision (same colors, UVs, tpage/clut, OT rule `min(minSZ>>5,0x7ff)` and
packet allocation), and pass the original a copy of the mesh without them, so
all other polygons use the original code unchanged. Third person and Vanilla
stay on the original renderer.

## 2026-09-29 - D11B / Needs playtest

Implementation (`recomp/src/ttk/near_clip.cpp`, new hooks `0x80010000` and
`0x80011020` in `game.local.toml`, regenerated: one generated line each;
`first_person_view_live()` and `first_person_duke_drawing()` added to
`modern_controls`; debug JSON `fp.near`; developer switch `DNTTK_NEAR_CLIP=0`).
While the eye view is live, each world or object mesh with a polygon that has an
unsafe corner (GTE divide saturated, IR clamped, projected beyond +-1000) or a
corner nearer than 1536 units has those polygons drawn by the host. They are
clipped in view space (near 16, guard band +-480 x +-240), cut along
whole-texel lines until small, and emitted as GT3/G3 packets. Colors, fog, light
table, texture animation, command bits, packet arena and OT slot follow each
renderer. The original routine then draws a copy of the mesh without those
polygons, so everything else stays on the original path. Object polygons with
every corner nearer than H stay dropped as in the original; Duke's own model is
left alone (D12). Guards: SHA-256 over both routines.

Evidence on binary
`d294c3d8d228a1ad6a3cdeff7aeac2b9ee2576c4df6a1e3fcc1f34902afc5aaa` (isolated
Xvfb, private route cards slot 1, Duke placed with debug position writes on
those cards only; captures in `recomp/analysis/d11b-near-clip/`):
- Club red-panel side wall: -90..+90 sweep. With clipping off, looking along the
  wall (+15..+45) the near wall is black and head-on the frame bends. With
  clipping on the wall is continuous with straight, perspective-correct texture
  lines.
- Club entry corridor side wall: off shows black gaps at -30..-15 and
  +15..+30; on is continuous.
- Stage block at spawn: floor near the eye now draws (off: missing).
- 0 refusals, 0 copy overflows. Frame budget in the club: first person 59.96
  fps on, 60.04 off; third person 60.03; 0 underruns.
- Third person (P toggle, live): 0 near-clip calls, projection 386 and minimum
  distance 768 intact. Vanilla route `d11b-vanilla` exit 0, captures normal.
- `ttk-near-test` (new), `ttk-controls-test` (D07 LEVEL00 fixture; the D08P
  fixtures fail an overlay-code assertion at `0x800cc57c` unrelated to this
  change), `ttk-input-test`, `ttk-aim-test` PASS; Python 74 OK (2 skipped);
  movie shard current.

Limits: club interior only (side wall, stage, entry corridor), not the rest of
the first level or the campaign; affine mapping remains inside each small
piece; object polygons entirely within H stay dropped; Duke's body is not
clipped (D12); host pieces ignore the runtime's widescreen X squash (the
project is 4:3; D14). Needs the user's own play near walls. No commits.

## 2026-09-29 - D11B / Needs playtest (second pass: apartment artifacts)

User after playing the first pass: "it looks very good, but i think there is a
kind of popping or artifacting ... inside the hooker's apartment ... the closet
... the back wall of the closet, which pops through the wall somehow" (the
closet door itself was fixed), and asked for a switch to compare. The switch
already existed: `DNTTK_NEAR_CLIP=0` on the launch command (run.py passes the
environment through).

Reproduced on fresh private cards (route street -> alley ladder -> apartment
window; debug state in `recomp/analysis/d11b-near-clip/apartment-cards`). The
wardrobe is a prop (object renderer) flush against a world wall; the first
pass sorted each host piece by its nearest corner, so wall and wardrobe pieces
interleaved (sawtooth). Changes: pieces sort by average depth; host prop pieces
4 OT slots nearer (actors excluded, tracked from `0x800348d8` to
`0x8001ca4c`); props taken over to 3072 units so original prop polygons do not
lose to host wall pieces; new corners round outward to close T-junction
hairlines; tighter depth split (1.25 / 16 px). A per-frame packet budget was
added after heavier splitting overran the 139,744-byte render ring (screen
garbage in an experiment): 64 KB host packets per frame and never past 60% of
the ring; otherwise pieces stop splitting or the mesh stays original.
Developer switches documented in `documentation/62-d11b-near-clip.md`
(`DNTTK_NEAR_CLIP=0|world|object`, `DNTTK_NEAR_TINT=1`, bias, sort).

Evidence on binary
`9029435a8bddb3fcc7ca4e572d13626fc222535f1f7f2c069405196c562b8f5e`: apartment
closet sweep, 24-frame walk to the wardrobe and five-spot room captures clean
(unclipped: torn/see-through wardrobe, black ceiling wedges); club side wall and
corridor unchanged; host packets peak 42-46 KB/frame, 0 skipped, 0 fallbacks;
59.92 fps clipped / 59.94 unclipped / 59.94 third person, 0 underruns; Vanilla
route `d11b-vanilla-2` exit 0; native tests and Python 74 OK; movie shard
current. Remaining: painter's sorting (a prop up to 128 units behind a nearby
wall piece could draw over it; actors excluded); tested in the club and the
apartment only. Needs the user's replay of the apartment. No commits.

## 2026-09-29 - D11B / Needs playtest (third pass: conservative default)

User at 1080p: black grid lines on floors outdoors and in the subway, faint
lines on objects, diagonal artifacts on the wardrobe, floor texture popping,
none in third person; asked whether the earlier state was better and what the
options are. Reproduced by capturing the presented window at 1920x1080
(`screenshot_file` reads the software raster and missed it): dotted dark lines
along host piece edges. The outward corner rounding caused most of them; the
subdivision seams and the -4 prop bias caused the rest. The default is now a
conservative mode: take over only polygons the original gets wrong (unsafe
corners, beyond the GPU's 1023x511 primitive limit, or props entirely within
H, which made the wardrobe see-through up close), clip them, emit without
subdivision, sort each as one polygon like the original, floor rounding. The
subdividing mode stays available as `DNTTK_NEAR_MODE=full`; `DNTTK_NEAR_CLIP=0`
is off.

Evidence on binary
`2b5670b5bcb0a4942901b16a5b6b04b3f2c653c2124dfce384ba174b3017209d`: 1080p
street without dark lines; apartment views, roam and the walk into the
wardrobe clean and solid where the unclipped build is torn or see-through;
club side wall and corridor gaps filled; 60.01 / 59.97 / 59.71 fps (on / off /
third person), 0 underruns, host packets at most about 1.2 KB per frame;
Vanilla `d11b-vanilla-3` exit 0; native tests, Python 74 OK; movie shard
current. Subway not tested here (no isolated subway state). Trade-off: close
surfaces keep the PS1 affine texture bend and whole-polygon sorting, as in
the original. No commits.

## 2026-09-29 - D11B / Needs playtest (user review; occluder fade in first person)

User after playing the conservative default: "the rendering that you've set for
default runs the very best it looks the very best"; peripheral glitching of the
Vanilla renderer in first person is gone; clip off (`DNTTK_NEAR_CLIP=0`) stays
as Vanilla rendering; full mode "is just no good" (black floor lines) - keep
for research only. Remaining: "occasionally the very smallest slight bit of
black line artefacting on the ground ... negligible", and the subway
card-reader door turns invisible when Duke walks up to it in first person.
The user's DuckStation screenshot (`research/vanilla.png`) shows the same door
translucent in the original game in third person.

Cause: the original occluder fade. The prop loop (`0x80031fa0..0x80032078`)
makes a prop semi-transparent and dims it by distance when it is nearer than
the fade distance (camera+0xa0, about 291) and its screen rectangle overlaps
Duke's (camera+0xa8), via `0x8002ee50` (rectangle overlap, single caller). In
the eye view Duke's rectangle covers the whole screen (-218,-16 466x579), so
any prop walked up to fades. Fix: new hook `0x8002EE50` (regenerated, one
generated line); in the eye view only, the call from `0x80032008` gets an empty
off-screen rectangle as a0, so the prop draws solid. Third person and Vanilla
keep the original fade. `DNTTK_FP_OCCLUDER_FADE=1` restores it; logged as a
D19 menu toggle at the user's request, together with the near-clip mode.

Evidence on binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73` (private copy
of the user's F7 slot 4 savestate in
`recomp/analysis/d11b-near-clip/subway-cards`, player files untouched):
walking into the street sign pole, original fade shows it ghostly translucent,
the new default draws it solid (41 tests intercepted, 0 in third person);
`recomp/analysis/d11b-near-clip/third-pass/pole.png`. 59.87 fps first person,
59.78 third, 0 underruns; Vanilla `d11b-vanilla-4` exit 0; native and Python
tests pass; movie shard current. The subway door itself was not reached in the
isolated run (it is past the station guards); needs the user's replay there.
The faint ground-line residue in conservative mode was not reproduced at 1080p
street captures. No commits.

## 2026-09-29 - Ad hoc: Jetpack inventory icon replaced

At the user's request the Modernized inventory strip's Jetpack icon (item 1,
was Duke3D tile 2467) is now the user's own `research/inv/jetpack-icon.png`
(16x12). `recomp/assets/ttk-inv-icons.pack` rebuilt with only that entry
changed (backup `recomp/analysis/d11b-near-clip/ttk-inv-icons.pack.before-jetpack`),
provenance JSON updated, redeployed beside the executable (no rebuild of code
needed). Verified in the inventory strip (`]`) on an isolated run. No commits.

## 2026-09-29 - D11B accepted (Done)

User after playing binary
`84168b78fb49318312a6acf586ab0b8aef98606caf7bbc16598376bd9ace3b73`: "its awesome!!! now it doesnt peek through the doors. amazing work." (the new jetpack sprite "is also available").
D11B moves to Done: conservative near clipping is the default, props no longer
fade see-through in the eye view, `DNTTK_NEAR_CLIP=0` remains Vanilla-style
rendering, `DNTTK_NEAR_MODE=full` is kept for research only. Both switches are
logged as D19 menu toggles. Remaining notes: occasional faint ground-line
residue the user called negligible; first-map areas only, not the campaign.
The jetpack inventory sprite change is confirmed in game. D12 (hands and
weapon) is the next first-person job.

## 2026-09-29 - D12 first-person weapons (Needs playtest)

User asked to continue at the recommended point (D12) and, mid-session: "if we
can ACTUALLY use the real hands and weapons from this game, that will be the
most perfect way to go for it". Delivered with the game's own assets only.

Research: the actor draw `0x800348d8` draws the weapon (`0x80033e40`), muzzle
flash (`0x800341e4`) and held item (`0x80033f5c`) from the hand joint (model
byte `+0x33`, joint 7) after `0x800292a0`; the hand's near cull (depth < H)
skipped all of it in the eye view. Joint matrices are stored world-to-joint
(transposed by `0x80010d60` before camera `+0x20` times it). `duke_drawing`
turned out to be cleared by the draw's own callees before the joint loop, so
D12 bounds Duke's draw separately.

Implementation (`recomp/src/ttk/first_person.inc`, `near_clip.cpp`,
`modern_controls.cpp`; hooks `0x800292A0`, `0x80033E40`, `0x800341E4`,
`0x80033F5C` added to `game.local.toml` and regenerated with the recompiler
directly): the hand transform and its attached draws get a guest-allocated
matrix (Duke's joints never written); placement anchored in view space; a
ready pose from each weapon's firing pose (torso-relative, seeds for slots
4-11 surveyed in this build); flash kick; viewmodel packets appended to
ordering-table slot 0 when Duke's draw ends so walls and Duke's chest cannot
cover them; arm joints 2-6 culled at the eye; close-camera fade neutralized for
the viewmodel only; SHA-256 guards for `0x800348d8` and `0x800341e4`;
`ttk_input` JSON buffer 2048 -> 8192.

Evidence on binary
`1f232bc162e6354f8e3aa2d87994401e11410cf38bd653236f0ac2167a125ccc`: 13
weapons (slots 1, 2, 4-14) captured at rest and firing, all drawn; pitch
sweep 0/+-35/+-60 stable for pistol and shotgun with the flash on the muzzle;
walk, run, crouch, jump, fast turns and wall contact keep the weapon in view
and on top; third person unchanged; 59.95 fps first person / 59.96 third,
0 underruns; Vanilla route `d12-vanilla` exit 0; native tests
(`ttk-controls-test` with a new D12 case, aim, near, scene, input) PASS;
Python 74 OK. Limits: instant holster/draw, right hand only, ready pose
replaces reload/pump motion, projectiles leave from the real hand, slot 8/9
large, first-map street only. Needs the user's playtest for placement, size
and feel. No commits.

## 2026-09-29 - D12 twin cannons framed like the Devastator (Needs playtest)

User: "This is an amazing start". Asked for weapon key 5 (slot 8, the twin
cannon launcher) to show "the two Canons visible at the bottom" like Duke 3D's
Devastator (reference images in `research/`), using the strip-club mirror to
see how Duke holds it. Delivered: slot 8 offset (-20, 240, -100), 42 degrees
muzzle-up, scale 1.2 via the original weapon draw's scale argument; the
cannons rise from the bottom corners and the joining block stays a strip at
the bottom edge (the model's middle shares the cannons' top plane, so it
cannot be framed out entirely). The viewmodel moved from ordering-table slot
0 to slot 1 so the HUD draws over it (the right cannon had covered the ammo
box). New developer overrides per slot for offset, tilt and scale. Club
mirror checked: the reflection keeps Duke's head, arms and real weapon pose.
Binary `b756d71f9558ce7a3ce5c68e12d3283eeff3e702ef11d0e6ca6df433da93a81e`;
native tests and Python 74 OK. No commits.

## 2026-09-29 - D12 weapon framing tweaks (Needs playtest); D12A queued

User: "it's amazing!!" Pistol, Devastator (slot 8), flamethrower (slot 10),
special shotgun (slot 6), crossbow, rocket launcher, Python, dynamite and Holy
Hand Grenade accepted as they look. Requested tweaks delivered as per-weapon
display tilt/offset defaults (no pose or asset changes): shotgun (slot 5,
key 3) offset (40, 0, 0), tilt -10 pitch / -30 yaw, so the barrel and pump
show; gatling (slot 7, key 4) offset (30, -10, 330), tilt -10 / -25 / roll 10,
right of centre and pointing in toward the crosshair; throwing blades slot 1
tilt -30 / -20 / roll 40 and slot 2 tilt -40 so they read as blades instead of
a vertical line. Captured at rest and firing (flash on the gatling muzzle).
New backlog job D12A (kick stays in first person), queued next by the user.

## 2026-09-29 - D12 framing pass 2 (Needs playtest)

User accepted knife, axe, pistol, Devastator, pipe bomb/dynamite/Holy Hand
Grenade, flamethrower (slot 9, key 8), double barrel (slot 6, key 9) and
crossbow as they look; "this is all polish, everything is working". Retuned:
shotgun (slot 5, key 3) less sideways, offset (30, -15, 0), tilt 3 / -15;
gatling (slot 7, key 4) at the bottom edge pointing up toward the crosshair,
offset (30, 55, 290), tilt 8 / -22 / roll 10; energy weapon (slot 10, key 7)
and freezer (slot 11, key 0) scaled 1.4, raised 20, tilted -12 (freezer yaw
-10) so more barrel and tip show. Bringing them closer made them vanish (the
hand joint fell inside the near cull), so scale is used instead. Captured at
rest and firing (freezer effect at its tip). Binary
`29a31eb40642108e1c8b9b1f319e17e81fcb6b2139ed45a9d8ef2a728429146e`. Next job remains D12A.

## 2026-09-29 - D12 framing pass 3 (Needs playtest)

User: shotgun (key 3) and number 4 should have the grip lower and the muzzle
raised so they point at the crosshair; number 9 a slight version of the same;
5, 6, 7 and 8 fine. Pitch tilts: slot 5 -12 (yaw -15), slot 7 -8 (yaw -22,
roll 10), slot 6 -6. Negative pitch raises the muzzle (the code comment had
the sign backwards; corrected). Captured at rest and firing (flash at the
barrel tips). Binary
`f56bc9a570014a30a386efcc31e9f955872508f5c58e26d512b5325d1f8f7b3d`. Next job remains D12A.

## 2026-09-30 - D12 framing pass 4: hide the floating hand (Needs playtest)

User: pass 3 "absolutely phenomenal"; asked whether positioning alone can
hide the gap where Duke's arm is missing (a floating hand), for example by
lowering the pistol toward the bottom of the screen as the gatling already
is. All weapons reviewed: the pistol (slot 4) and knife (slot 1) showed a cut
wrist; the others already run off the bottom edge. Pistol offset (-10, 65, 0),
tilt -10 (muzzle raised so it still points at the crosshair); knife offset
(-60, 60, 0). The glove now enters from the bottom edge; pistol flash stays
on the muzzle, stable looking down. Binary
`08af082dc6d5bb750120a7927ede9757d6233c5c1c4cbcb4544fade3c6bf5741`. Next job remains D12A.

## 2026-09-30 - D12 framing pass 5 (Needs playtest)

User: numbers 3 and 4 a bit lower, tilted very slightly to the right, and
re-aimed at the crosshair. Shotgun (slot 5) offset (30, 10, 0), tilt -16 /
-15 / roll 6; gatling (slot 7) offset (30, 80, 290), tilt -12 / -22 / roll 16.
A yaw-only alternative (muzzle turned 4 degrees right instead of a lean) was
captured and left as an override. Captured at rest and firing. Binary
`cc33b1e015057ae21b84052d9e9bb3fa0c90935f4fabcb4e1f6a2c5fdb28f7c6`. Next job remains D12A.

## 2026-09-30 - D12 framing pass 6 (Needs playtest)

User accepted the shotgun (number 3). Number 4 (gatling, slot 7) moved about
"two inches to the right" at 1080p (view x 30 -> 110) and re-aimed at the
crosshair (yaw -22 -> -26); a -31 yaw variant turned too far across.
Captured at rest and firing. Binary
`65226a9d8b4335f67b955b35af1172fb6a42357adcbd0027c97ebd5744612798`. Next job remains D12A.

## 2026-09-30 - D12 accepted (Done)

User: "finally, we can mark this as accepted!!" D12 moves to Done on explicit
user acceptance after six framing passes, with the automated evidence in the
earlier entries. Recorded limits stand: instant holster/draw, right hand only,
reload/pump motion replaced by the flash kick, projectiles leave from Duke's
real hand, first-map street and club coverage only. Binary
`65226a9d8b4335f67b955b35af1172fb6a42357adcbd0027c97ebd5744612798`. Next: D12A (kick stays in first person).

## 2026-09-30 - D12A first-person quick kick (Needs playtest)

User: in first person the kick's "duke-fu spin around and crazy moves" are
unnecessary; wants a Duke Nukem 3D style kick using his leg, "more accurate to
aim". Implemented (`recomp/src/ttk/kick.inc`, no new hook addresses, no
generated-C change):

- In the Modernized eye view, **Q** is a 0.45 s quick kick with any weapon,
  holstered, walking, running, crouched or jumping. Weapon stays drawn.
- The original boot attack (`0x800517a4`, random animation 112..115) is still
  reachable with Boot selected (Q, E at idle, held attack); in the eye view its
  request is converted at the lower-body initializer `0x800493a4` into the
  quick kick, and the eye view keeps its lease for that one update.
- The hit is the original sphere `0x800a979c` (radius 96, type 10, damage
  `[0x800d21fc] * 20`, x4 with steroids, attacker Duke) placed 340 along the
  view, on each player update of the extension (about 4), as the original does
  for the foot on every update of 112..115. The original kick sound
  (`0x8006b270` id `0x1000`) plays as the leg snaps out.
- Duke's own right-leg joints 14..17 are drawn in front of the eye from three
  poses recorded from the original straight front kick 115 (rest, chamber,
  extension), on top of walls, rising from the lower left with the boot just
  under the crosshair.

Evidence (isolated private instance, fresh private cards, god mode, pig cop
on the first street): pistol Q 3750 -> 2250 per kick, three kicks kill;
Boot + E and Boot + held attack convert (animation stays 63) and hit; running
and jumping kicks keep first person (0 fallbacks); third person unchanged
(armed Q inert, Boot + Q plays original 114/115); 59.89 / 59.93 fps without /
with repeated kicks, 0 underruns; Vanilla route `d12a-vanilla` exit 0;
native suites PASS with a new D12A case; Python tests OK. Limits: no
knockback (the original's reaction 84 did not occur with the quick kick's
hits at any tested height or reach), 1500 per quick kick versus up to a kill
for a point-blank original kick that lasts 2.5 times longer, right leg only,
first street and pig cops only. Needs the user's playtest for feel, framing
and balance. Binary
`93b08bf7f89d1183dae2afb1fa8118a27774a2fd77a59776fe715d194e6cfb27`.

## 2026-09-30 - D12A follow-up: crosshair hits, Boot left-click while moving, thigh (Needs playtest)

User after the first playtest: "The kick looks amazing"; no knockback and the
lower damage per kick are fine; garbage bags (hit only when crouching) and the
pallet by the subway entrance should be kicked where the crosshair is;
left-click with key 1 should kick while moving; the missing thigh needs tuning
and options. Changes: the kick sphere goes to where the original query-only
crosshair trace (`0x8006d980`) meets a surface or actor within the foot's
reach (480 horizontally), else 340 along the view; a held attack with the Boot
selected starts the quick kick in first person while moving; the thigh is
drawn end for end from the knee (its hip origin is inside the loop's near
cull), `DNTTK_FP_KICK_THIGH=0` for the old look. Evidence: alley garbage bag
struck from about 520 with the crosshair on it (private copy of the user's
slot-5 state); running Boot left-click kicks twice and keeps running; pig cop
1500 per kick; 59.95 fps; Vanilla route `d12a-vanilla-2` exit 0; native and
Python tests PASS. Not located: the subway pallet. Thigh framing is still open
for the user. Binary
`e48dc442889800d962ace3274f2c4b2f019f50928cddb06a9aff5e20e05e37c3`.

## 2026-09-30 - D12A thigh nudge (Needs playtest)

User: "incredibly fun"; nudge only the thigh down by 10% and test. The thigh
matrix alone moves down the screen by 10% of the screen height at its
knee-end depth (`DNTTK_FP_KICK_THIGH_DROP`, default 0.10); calf and boot
unchanged. Captured at frames 7 and 13 before/after
(`recomp/analysis/d12a-kick/runs/thigh-drop-sheet.png`); `ttk-controls-test`
PASS (thigh offset is along the view's down axis only). Binary
`1a915e56d629844357cb36cc2d19ababa363134b1aea7b8e5c52a3db2a3bc358`.

## 2026-09-30 - D12A: thigh 0.13, E never kicks, Q chains (Needs playtest)

User: thigh drop 0.13 is the sweet spot; E into a wall could kick (only Q
should); Q should chain freely with any weapon while running, like the Boot's
held left-click. Changes: thigh default 0.13; in the eye view an original
Boot kick request converts to the quick kick only when the attack is held
(E's Action request restarts idle with no kick); a Q during a kick queues the
next, and a held Q repeats. Evidence: Boot + E no kick (3 suppressed); pistol
into a wall + E no kick; running pistol Q mash chains, held Q 7 kicks in 3 s;
native suites, Python and Vanilla route `d12a-vanilla-3` PASS. Binary
`1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`.

## 2026-09-30 - D12A accepted (Done)

User: "its done! accepted. ... this has been yet another amazing feat of engineering." D12A moves to Done on explicit user acceptance after three playtest
passes (crosshair hits, Boot left-click while moving, thigh drawn and tuned to
0.13, E never kicks, Q chains), with the automated evidence in the earlier
entries. Recorded limits stand and were accepted: no knockback, lower damage
per kick than a point-blank original kick, right leg only, first street and
alley coverage only (the subway pallet was not located). Binary
`1452391c97b4eb59df0e7482a939d48673e1270cacae2b52a51d9a54e115e956`.

## 2026-09-30 - D08L inertial edge run-off (Needs playtest)

Failing comparison first (isolated instance, private apartment-state copies,
real keys): the apartment bed run-off and walk-off were already continuous
(~47.5 and ~9.5 per frame), but running off the fire-escape platform outside
the apartment window (drop >= 2048) fell at 3058 (16.5 per frame, a third of
run speed) with the mouse camera frozen, because the D08F short-fall lease
only covered 256-768 drops. Also found: the stride estimate can spike to its
14000 clamp at a run start, so 1 of 8 bed run-offs left faster than a running
jump (same on the pre-change binary). Change, Modernized only: the lease now
also covers a running fall over 768 that directly follows a ground stride;
every departure is capped at 10000 running (running jump 10085) or 2048
walking; edge-jump grace stays 768-only. After: platform run-off 9172 through
the fall, 2700 units of air travel, running landing; mouse look live in third
and first person; running jump unchanged; Space mid-fall gives no jump; walking
still stops at the edge; bed spike capped at 10000, other samples unchanged.
Native suites (new run-off cases), Python (74, 2 skipped) and Vanilla route
`d08l-vanilla-1` exit 0. Limits: feel unconfirmed; only this platform and the
bed measured; very high falls and wall-adjacent ledges not swept. Details:
[D08L note](66-d08l-edge-run-off.md). Binary
`a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

## 2026-09-30 - D08L accepted (Done)

User: "im happy with that!" D08L moves to Done on explicit user acceptance of
the inertial run-off, with the automated before/after evidence in the previous
entry. Recorded limits stand: only the fire-escape platform and the apartment
bed were measured; very high falls and wall-adjacent ledges were not swept.
Binary `a16c13ca3bd9cba5e864213fdc6ba57526a976cbe2be8df7301f85b0bd697eb8`.

## 2026-09-30 - D13 higher internal resolution and display scaling (Needs playtest)

Scope and decisions: exposed the runtime's existing supersampling (1-4x),
fullscreen tri-state, window width and present filter as per-profile launcher
preferences instead of adding a renderer feature. A reviewed runtime patch
(`time-to-kill-zzzzzzzz-presentation-cli.patch`) adds `--internal-scale`,
`--display`, `--window-width` and `--output-filter` CLI overrides; profile
schema 14 stores them (schema 13 migrates with a backup). Vanilla keeps 1x
(original presentation); Modernized defaults to 2x. The software renderer
always runs at 1x because its supersampling halves the frame rate (29 fps at
2x); the saved scale applies again with OpenGL.
Files or build identity: `recomp/tools/local/player_profiles.py`, `run.py`,
tests; binary `14fde30b76f3907effa6680603a367d28458dda0ed5aea69cb65c79711bee71d`.
Verification and evidence: isolated Xvfb instance (Mesa CPU OpenGL, worst
case): OpenGL 1x/2x/3x/4x 59.9/59.9/59.5/50.3 fps in the apartment; software
1x 60.4, 2x 29.1. 1x vs 4x window captures show sharper geometry and model
edges, same texels, identical HUD. Intro FMV framing identical at 1x and 4x;
pause menu correct at 4x/nearest; saved 4x/borderless/1280/nearest survived a
relaunch with no flags; borderless/exclusive went fullscreen under metacity
with a centered 4:3 image. Vanilla route `d13-vanilla-1` exit 0 at 1x; Python
79 OK (2 skipped); native `ttk-input-test` PASS.
Remaining limitations / next action: user playtest on the real desktop and
GPU (fps at 2x-4x, exclusive fullscreen mode change, visual preference).
4:3 only (widescreen is D14). Pre-existing unrelated issue recorded: the
runtime patch stack check fails on `host_osd.c` drift from the
inventory-strip patch. Details:
[D13 note](67-d13-resolution-display.md).

## 2026-09-30 - D08U backlogged (Todo)

The user reported the problem during the D13 playtest: there is no way to latch
onto a ladder from the platform at its top and climb down. Reproduction: the
user's savestate slot 12. Added D08U (depends on D08, D08J) with investigation
and acceptance criteria. No code changed; D13 remains Needs playtest.

## 2026-09-30 - D13 playtest follow-up (Needs playtest)

User: "I have just been playing at 4x and it's holding 60 FPS the whole time it
just looks amazing." They asked for 4x as the Modernized default, which it now
is. Alt+Enter did not leave fullscreen. Cause: the runtime keymap compared
modifiers exactly, so a real left-Alt event (0x0100) never matched the stored
KMOD_ALT (0x0300). Ctrl+F had the same problem. Fixed by comparing Ctrl/Alt/
Shift groups (`host_keymap.c`, with new unit cases). A related finding: in
this SDL3 build "exclusive" was identical to borderless. Exclusive now sets
the desktop display mode explicitly (`psx_apply_fullscreen_display_mode`).
Both fixes are in the D13 runtime patch. Verified under metacity on Xvfb:
Alt+Enter and Ctrl+F toggle windowed/fullscreen in both modes, plain Enter
does not. Vanilla route `d13-vanilla-2` exit 0, Python 79 OK, host keymap and
native input tests PASS. Binary
`ecc9328065b8a6c3311423e1640936b1835e1517d35b923605609595e2f3a95d`.
The user's other reports are not caused by D13 and were backlogged: D10B
(headless Duke and see-through doors, same at 1x and with the original camera)
and D08V (sewer mantle lease flicker). Next: the user confirms Alt+Enter and
exclusive on their desktop.

## 2026-09-30 - D13 windowed default and remembered display (Needs playtest)

User: "make windowed the default rather than fullscreen ... it should also save
the last known config". Windowed was already the built-in default. The
user's profile opened fullscreen because it had saved `exclusive`, so it is
now set to windowed (the v14 file was backed up). New: the runtime writes the
window state at exit (`--presentation-state` file: windowed or fullscreen
after any Alt+Enter, borderless or exclusive, last windowed width, 0 when
maximised). The launcher folds it into the profile that was played. Profile
schema 15 separates `display` (how the game opens) from `fullscreen_mode`
(what Alt+Enter enters, passed as `--fullscreen-mode`); v14 files migrate with
a backup. Verified under metacity: exclusive -> Alt+Enter -> resize to 1280
-> close saved `windowed / exclusive / 1280`; the relaunch opened windowed at
1280x960 and Alt+Enter entered exclusive; quitting fullscreen saved
`exclusive`. Python 82 OK (3 new), Vanilla route `d13-vanilla-3` exit 0,
`ttk-input-test` PASS, patch checked forward and reverse. Binary
`7bd2a001a295330397d45c73ca9bc2ca0bc575c0d889bab3259d06778b3a3d1c`.

## 2026-09-30 - D13 F11 fullscreen key (Needs playtest)

User: "make f11 the key to go fullscreen instead of this alt enter thing".
F11 is now the only default fullscreen key (runtime `host_keymap.c` default,
in the D13 patch); Alt+Enter and Ctrl+F are unbound. The modifier-matching
fix stays for rebound hotkeys, with keymap tests moved to a config rebind.
Launcher text, the manual and the docs now say F11. Verified under metacity:
F11 toggles window <-> exclusive, Alt+Enter does nothing. Keymap test PASS,
Python 82 OK, `ttk-input-test` PASS, Vanilla route `d13-vanilla-4` exit 0.
Binary `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`.

## 2026-09-30 - D11C backlogged; D10B corrected

The user noticed that the headless Duke happens only in slot 12. Confirmed on
a private copy: the head-joint hide bit that first person sets while drawing
was saved inside the savestate (flag `01`; slot 1 `00`). Nothing clears it
after a load, because `first_person_draw` never takes ownership of a bit that
is already set. Added D11C (depends on D11). D10B no longer claims the headless
head; it keeps the close-camera transparency and the doors. No code changed.

## 2026-09-30 - D13 accepted (Done)

User: "correct correct correct, D13 is good. I'd say let's approve it." D13 is
Done: per-profile internal resolution (Modernized 4x at 60 fps on the user's
GTX 1080 Ti, Vanilla 1x), windowed by default with the last display state
remembered, F11 fullscreen with a real exclusive mode, and linear/nearest
output. Evidence is in the preceding D13 entries and
[the D13 note](67-d13-resolution-display.md). Remaining,
recorded: 4:3 only (widescreen is D14); the runtime patch stack check still
fails on the pre-existing `host_osd.c` drift. Binary `79e8cc5c579e7afa50f13b312e253d5104da5cdf82a51535245b171a20f1086b`.

## 2026-09-30 - D08W backlogged (Todo)

The user reported that in the first subway area's shallow water, A/D (with
Shift) plus Space always jumps forward, so sideways jumps are impossible.
Added D08W (depends on D08, D08C) to fix later. No code changed.

## 2026-09-30 - D16A cancelled; D08A3 backlogged (Todo)

User: the work done with the original assets "is the way to go"; HRP is
"probably unnecessary, considering how far we've taken it". D16A is Cancelled.
As its only replacement, D08A3 (depends on D08A2) will extract the original TTK
medkit, biomask, jetpack, steroids (and goggles) art to PNG and swap it into the
inventory switcher in place of the Duke3D tiles. No code changed.

## 2026-09-30 - D17 scope expanded: high refresh rate rendering (Todo)

The user supplied a backlog brief for arbitrary / high refresh rate support.
It overlaps D17 (presentation smoothness without faster simulation), so D17 was
expanded rather than duplicated: Match Display / 30-240 / Unlimited frame-rate
option, render timing decoupled from gameplay, interpolation, mouse-look
responsiveness, pacing, and a numeric 30/60/120/180/240 comparison. The full
brief is [documentation/68-d17-high-refresh-brief.md](68-d17-high-refresh-brief.md).
Audit and plan come first when selected. No code changed.

## 2026-09-30 - D11C stale head-hide reclaim (Needs playtest)

Chose the load-side option: it also repairs saves that already carry the bit
(slot 12) and rewind snapshots, needs no runtime patch and no save-time write.
`first_person_draw` (`recomp/src/ttk/first_person.inc`) now takes Duke's draw
(`duke`, identity checked) separately from the eye view (`eye`). At Duke's
draw entry, after releasing our own hide, a bit 0 still set on Duke's joint 9
record is unowned and is cleared, keeping the record's other flag bits; in
first person the normal hide then takes ownership as before. New debug counter
`fp.head_reclaims`. Vanilla is unchanged (the hook returns before any
Modernized work).

Original-game check (static, owned disc): in `SLUS_005.83` the only joint
record flag writes are `ori 4` at `0x8009d974` and `0x800a33c8` (bit 4). None
of the 30 unique `.OVR` payloads stores a byte at `+0x44` or sets bit 0 on a
joint record; the executable's five byte `|1` read-modify-writes are other
structures (GPU primitive code byte `+7`, `+3`, `+0x27e`). The head joint is
never hidden by the game itself, so clearing an unowned bit cannot undo a real
hide. Dynamic tracing of every level was not done.

Evidence:
- `ttk-controls-test` PASS with new cases: in third person a record of `0x11`
  becomes `0x10` at Duke's draw (other actors and the other draw loop leave it
  alone; counter +1); in first person a stale bit is adopted for that draw and
  released at the list step. `ttk-input-test`, `ttk-aim-test` PASS; Python 82
  OK (2 skipped).
- Private copy of slot 12 (`recomp/analysis/d13-resolution/cards`, identical
  to the player's file, which was not touched), run `d11c-fixed`, Xvfb :93,
  Modernized third person: after load Duke's joint flags are all `00`,
  `head_reclaims` 1, and the head is drawn
  (`runs/d11c-fixed-third.png`). P into first person: head hidden, 29 hides,
  no further reclaims, bit clear between frames; back to third person, head
  shown. Six saves taken in first person on private slots 4-9 all load with
  the bit clear (none happened to land inside the draw window).
- Player binary `d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`.

Remaining: the user's own confirmation that slot 12 shows Duke's head. A save
still stores the bit if taken inside the window; it is harmless only with this
build, and loading such a save in Vanilla (no hooks) would still show a
headless Duke. The slot-12 file is not rewritten.

## 2026-09-30 - D11C accepted (Done)

User, after loading slot 12: "duke's head is back, mark as complete! well
done". D11C is Done. Remaining, recorded: a save taken inside the draw window
still stores the bit (cleared on load in Modernized; Vanilla would still show
it headless), and the slot-12 file is not rewritten. Binary `d14b04f062dc88271fa9072288f0b37a170637563309920f2c92e139eb908f58`.

## 2026-09-30 - D07D red dot off by default in Modernized (Needs playtest)

Profile: schema 16 (`player_profiles.py`). `DEFAULT_CONTROLS['red_dot']` is
now `False`. A file older than v16 whose Modernized profile still says
`red_dot: true` is switched off once, with the usual recovery backup and a
notice ("run.py --red-dot on restores it"). How an explicit choice is told
apart from the old default: it cannot be for pre-v16 files (the old default and
a deliberate "on" were stored identically), so every pre-v16 "on" is treated as
the old default, per the user's request. From v16 on, the stored value is the
player's own choice and later loads keep it. Vanilla still launches with
`DNTTK_RED_DOT=1` whatever is stored.

Hook (`weapon_aim.cpp` `marker_hook`): the old code needed an entry lease at
`0x80033AF8` plus `player_identity_ready()` (gameplay context, camera<->player
back-links and the full LEVEL00 control identity), and stepped aside in the
runtime's precise-interpreter slices. Any of those could let the dot through:
scripted/detached cameras, other maps, or a frame whose entry hook was skipped.
(The "precision aim" in the brief was `g_precise_mode`, the runtime's
interpreter slice, not right-mouse aim; held Mouse2 was already covered.) Now
ownership is proven at the single enqueue `0x8002BC18` with RA `0x80033DB0`:
S4 is Duke (`0x800D7198`; the routine never saves or writes S4, and its
only caller `0x80035434` passes the actor there), the marker frame's saved RA
at `sp+0x40` is `0x8003543C`, the primitive is the same textured quad, and the
weapon/marker code identity (`aim_guards`, all main executable) matches.
Static check on the owned disc: `jal 0x80033AF8` occurs once in the
executable and in none of the 30 unique overlays. Lockstep verification, call
bail and Vanilla still step aside. The quad is collapsed exactly as before; no
aim, target, option or damage state is touched.

Evidence:
- `ttk-aim-test` PASS with new cases: hides without an entry lease or player
  readiness and during `g_precise_mode`; no write for another actor, a foreign
  frame RA, lockstep, Vanilla, or changed marker code; red dot on leaves the
  quad. Earlier assist/aim-vector assertions unchanged.
- Python 85 OK (2 skipped), including new schema-16 migration/launch tests.
- New route `pc_input_probe.py --controls red-dot` (`red_dot_probe.py`):
  natural pistol scans, OpenGL, Xvfb. `d07d-off-gl`: hidden markers third 51,
  held Mouse2 59, first person 65. `d07d-on-gl`: 0 in every segment; targets
  acquired in both runs. The captures do not show the dot clearly either way
  (muzzle flash, red lighting), so they are not visual proof.
- The old `--controls aim-options` route now fails in its state sampler
  (LEVEL00 full-hash check does not accept the D08D apartment patch) and the
  menu D04 diagonal check; both predate this job and were not changed.
- Player binary `db5bf9640893ae582acefa27fca8f98f27017bd6f1aa4c956d203cf31e6ec67b`.

Remaining: the user's own look at play with and without targets in third and
first person, held aim, jetpack, swimming and a scripted camera; the first
launch migrates the saved profile (backup kept).

## 2026-09-30 - D08T1 hold to grab, E always mantles (Needs playtest)

Binding model taken from the brief's proposal (the user asked for autonomous
work): `grab` Mouse2 and `grab_alt` Alt (either key), both rebindable;
`original_aim` Unbound in Modernized; with `original` weapon aiming or original
camera the `grab` input is precision aim (R1) and only Alt grabs. Profile schema
17 moves Mouse2 from `original_aim` to `grab` (custom precision-aim inputs kept).

Before the change (documented in `documentation/70-d08t1-grab-manipulate.md`):
the `0x80051CF0` mask stopped E's Cross from ever climbing a pushable object,
E started the grab and let go, and Space ran a host climb. Now: the
`0x80051CF0` mask applies only while Grab owns Cross, so W + E climbs the
dumpster like any climbable object; a new generated entry hook on the original
idle Action `0x80051890` (callers `0x800467F4`/`0x80052C14`) masks E's fresh
Cross at a pushable-only object, so E never grabs. Grab is held: request with
automatic holster, latch while held, W/S camera-relative push/pull (original
motion), release/E/blocked/hit/pause/focus end it, a fresh press is needed to
grab again, Space/fire/Circle/R1 wait. Directions stay neutral until the
original leaves 119..121 after letting go (a live run showed a held W feeding
the running push cycle through the tank fallback). Space climb code removed.
Prompts: `HOLD RMB TO GRAB` (first two touches) and `W/S PUSH/PULL - RELEASE
RMB TO LET GO` (first three grabs), with the live binding name.

Mouse2 audit: Modernized with `view` aiming loses R1 where view aim was not
ready (airborne, unsupported weapons), the held-RMB original jump arc and
held-RMB jetpack facing hold; all remain reachable by binding `original_aim`.

Evidence (binary `66097cf8409830cba5ffdd54a830299b9fe13e480d371874d04b4b355bad3015`):
- Live, private copy of the D08T slot-5 alley state
  (`recomp/analysis/d08t1-grab/grab.py`), Xvfb real keys/mouse, third + view,
  first + view, third + original (Alt): W + E climbs to the top and never
  grabs; E alone does nothing (16-24 idle masks); hold Grab grabs, pushes
  2582 -> 3539 (shove runs on to 3795, original lets go, no regrab while held),
  fresh grab + S pulls 2582 -> 1639 with mouse look live; release then W + E
  climbs; Grab while walking in grabs, never climbs; W + E while grabbing lets
  go then climbs; pause ends the grab; Grab against other things does nothing
  and sends no R1; with `original` aiming RMB is R1 and does not grab. First
  person blends to the orbit while grabbing.
- Vanilla original pad, gapless route (`vanilla.py`): grab 121, push +1185,
  pull -923, no climb (D08T measured +1202 / -928).
- `ttk-input-test`, `ttk-controls-test`, `ttk-aim-test` PASS; Python 87 OK
  (2 skipped). D07D red-dot route rerun on this binary: hidden 53/58/63.
- `build.py` stops in its runtime patch-stack check (reviewed patches no longer
  apply cleanly over live runtime edits; not changed); generation was run
  directly with the same `psxrecomp_cli.py generate` command.

Remaining: user playtest of the feel (hold vs. release timing, Alt in their
desktop environment, prompts), other pushable objects, and a pushable object
that is also a switch (keeps original precedence; none known).

## 2026-09-30 - D08V sewer mantle lease (Needs playtest)

Reproduced on a private copy of slot 12 (`recomp/analysis/d08v-sewer`; the
player's file untouched): 180 degrees from the save, W + E mantles a ledge
(139/140, mode 8). Refusals matched the report: mantle 8/0 and 8/8 (camera
lease never covered attached traversal; pad lease needed mode == previous),
gait 0/8 and 0/9 just after, falls 107/108 9/9. Change: camera-only lease for
mantle/hang/pull-up (mode 8 134..142, mode 6 147..153, mode 7 149..153, entry
frames included), unowned 107/108 falls (not the owned short fall) and gait
while previous is 8/9; locomotion excludes the mantle/fall states (no facing or
gait writes); traversal pad lease accepts mantle/hang entry frames; new
`traversal_camera_ready()` counts as a modern lease (fall keeps Up/Down and
strafe pads); first person blends to the orbit there.

Evidence (binary `123c7910b9ae049d0818de9cb85ea896a5242d6902ead311cf1f7f811d29693e`):
mantle timeline identical before/after with the orbit camera live throughout;
16-case sweep tank fallbacks 12 -> 0, no mantle lost; mid-mantle captures third
and first person; native and Python suites PASS; D08T1 route unchanged.
Remaining: user playtest in the sewers (other hangs were not reached), the
unconfirmed "mantling felt harder" (not reproduced; original routine unchanged).

## 2026-09-30 - D07D, D08T1, D08V accepted (Done)

User, after the three-job session: "awesome. accept". D07D, D08T1 and D08V are
Done on binary `123c7910b9ae049d0818de9cb85ea896a5242d6902ead311cf1f7f811d29693e`.
Recorded limits stay: D07D captures were not visual proof; D08T1 verified on
the alley dumpster only (a started shove finishes; switch-flagged pushables
keep original precedence); D08V exercised one sewer ledge (other hangs follow
the same rule). `build.py` still stops in its stale runtime patch-stack check.
D08X is now unblocked by D08V.

## 2026-09-30 - D08U top-of-ladder mount (Needs playtest)

Research on private copies of slot 12 (sewer) and the alley state: the slot-12
ladder is a plain ladder (type 308, flag 0x200), a flat panel whose top is 507
below the walkway, bottom at a floor near Y 510. The original has no top mount
for plain ladders (only 185 from below and the airborne catch -> 156 -> 186);
Vanilla edge inputs, backing off the edge and a forced airborne reach all end
in the water. A holstered fixture writing the catch's fields at the top gave a
complete original climb, bottom and top exit. Change (Modernized only): E at a
ladder top (panel top 300..800 below Duke, within 420 of the panel and its
width + 120, settled ground; E stows first) writes the catch's attachment
(+0x17c, +0x180, +0x1c4, mode 3, anim 156) and blends Duke over the 12 updates
of 156 to the far-face climbing line, facing the ladder; S on a plain ladder
adds Cross so Duke climbs down and steps off; the original 190 top exit and
185 bottom step-off join the D08V camera-only lease with neutral directions
(no tank fallback or `ORIGINAL MOVEMENT`, no backstep 82 on landing);
`E TO CLIMB DOWN` hint twice a session; guards for 156's end, its table entry
and `0x80073ae8`.

Evidence (binary `6129f2ab76e533ec9670bf17eb1a9c4f08471fa2d0fa7d883fd82a11da945683`):
slot-12 mount and full descent in third and first person with pistol stow and
redraw; alley ladder climb up (exit leased), E mount at its top, descent and
redraw; walk/run-off, strafe past and E away never mount; Vanilla edge routes
identical to baseline; native controls (new D08U fixture), input and aim
tests PASS; Python 87 OK. Player's slot file untouched.
Remaining: user playtest of the feel of the mount blend (a host blend over the
original transfer, since the game has no top-mount animation), the second
alley ladder and other ladders not exercised.

## 2026-09-30 - D08X hold-E airborne ledge grab (Needs playtest)

Traced the original reach: `0x800557f8` enters 109 only after an animation
event arms it (`+0x224 |= 0x800000`), then `0x80055208` acquisition runs before
the flight collision. A standing jump at a close wall with a ledge in reach
(slot-12 walkway wall, 1024 high) hits it first (107) and bounces, in Vanilla
as well as Modernized. Change: while E asks for a reach during an original
jump (98/103/104, mode 9), the owned ballistic hook sets the arming bit at
once (not over blocking bits 0x241, never with precision aim or in Vanilla);
the original then enters 109 on the held Cross and decides the catch.

Evidence (binary `bd153dd540359b1fa5eafc84a9ea04492099abd20e7e8479d511b33d36b065b3`):
W + E + Space at the slot-12 wall now grabs (148, mode 6) and pulls up (140)
3/3 armed and 2/2 holstered with late E, first person 2/2; without E still
bounces 3/3; pit, walkway-end and tall-wall jumps unchanged; alley running jump
reaches at takeoff but the original does not catch that low object (same end
as before); D08 alley ladder transfer attaches 2/2; Vanilla bounce identical;
native controls (new D08X fixture) and input tests PASS.
Remaining: user playtest; apartment exterior and crystal-2 ledges not reached
(no private state); armed late-E in a short jump still loses to the stow.

## 2026-09-30 - D08W shallow-water jump direction (Needs playtest)

Reproduced on a private copy of UI slot 1 (subway tunnel, ankle-deep water):
A, D, W, S and run-then-strafe jumps all went forward (about 1600 forward,
10 sideways). Cause: `swim_boost_horizontal` rescaled the original launch
velocity (aimed along Duke's facing) instead of aiming it along the held
direction; the D08O ledge/climb assists also caught strafes. Fix (`swim.inc`):
aim along the held direction at max(original speed, wade minimum); assists
only for Space alone or W. Evidence (binary `7ca63455...3328`): subway A
(10, -1612), run-then-A (10, -1612), S (-1650, 17), run-then-D (-12, 1238);
crystal-2 wade (UI slots 2 and 3) follows the held direction too; suites PASS.
Remaining: user playtest; the platform ledge assist was not triggered here.

## 2026-09-30 - D08A3 TTK inventory icons (Needs playtest)

The original inventory screen has no icons; the game's own item art is the HUD
indicator set (jetpack, Bio Mask, goggles, steroids, health cross) in
`/DATA/FONTS.RAW` (raw VRAM image at (960, 0), 4bpp cells with their CLUTs),
indexed by the HUD sprite table at `0x800c44b4..0x800c4504`. New
`recomp/tools/local/build_ttk_inv_icons.py` rebuilds the pack from the disc
(pinned hash), keys out the cell background, keeps the selection frame, and
writes provenance and review PNGs (local only). Medkit uses the health cross
(no medkit icon exists); steroids extracted only. Live capture shows the new
icons with green % and frame unchanged; missing-pack fallback and a new
reproducibility test pass. Remaining: user confirms the look in play.

## 2026-09-30 - D14 widescreen first pass (In progress)

The framework clamps PSX widescreen to 4:3 unless a trusted activation plugin
requests an aspect. Added plugin `ttk.widescreen` (acts only for Modernized
with `DNTTK_WIDESCREEN=16:9`), a default-enabled plugin-only preloaded package,
and CMake staging for `recomp/mods/preloaded/packages`. run.py sets nothing,
so the player build stays 4:3; a default launch is pixel-identical to the
baseline. Private 16:9 experiment: the wider view renders, but TTK's 4:3
screen-space culling leaves black gaps on the widened right side, and the
ammo box is not anchored. Next: TTK cull sites in `[widescreen.cull]` plus
regeneration, right-HUD anchoring, then a `--widescreen` profile option.
Native D14 activation test PASS.

