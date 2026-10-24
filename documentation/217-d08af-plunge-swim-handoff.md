# D08AF - Plunge-to-swimming handoff

2026-10-24. Done, user-accepted.

User: "yep! its a winner, lock it in and close it out". Acceptance and
implementation/documentation commit/push explicitly authorized. No gameplay
changes, rebuild or new live test after acceptance; the evidence below retains
its original coverage limits.

## Finding and bounded change

Private copies of the current UI slots 9 (pool) and 2 (moat) reproduce a
horizontal motion dip near the end of water entry. The native swim handler
has already selected stroke 128, but flag 0x04000000 routes movement through
0x8003e4b0's residual-fall drag instead of the normal stroke tail. Drag reduces
horizontal travel until native completion or collision clears that ownership;
normal swim speed then resumes. This is movement evidence, not an animation
recovery diagnosis or proof about frame pacing in the user's earlier session.

The accepted change blends held horizontal swim intent into the final part of the
native plunge displacement, before the existing collision sweep reads it.
The blend starts below downward velocity 8192 and smoothly reaches full
intent at the native 1024 completion threshold. These are bounded
feel settings, not a claim of physically calibrated hydrodynamics. Desired
travel uses the existing walk/run swim pace, camera-relative direction and
view-pitch horizontal share. There is no persistent added velocity.

Vertical root and velocity, native drag accumulators, water/oxygen ownership,
animation, entry timing and collision resolution stay native. No held
horizontal direction means no change. Explicit surface dive 133, shallow
water, SWIMMING ORIGINAL and Vanilla are excluded. Space/Ctrl still use the
existing swim behavior; this change does not cancel the downward plunge.

`swim.inc` arms at the authenticated 0x800455bc swim-handler entry from
0x8004b5bc. The shared bone lookup 0x8003964c, called from 0x8007a9f0 in the
existing sweep, consumes the scope before horizontal root is read. It checks
actor, return address, the handler/sweep stack difference (0xb8), input epoch
and sequence, active swim lease and underwater state. Scope is consumed once.
The native integrator's 804 bytes now join the resident SHA256 guards; the
handler and sweep were already guarded. Overlay identity remains required.
No new generated hooks or framework changes are needed.

## Evidence and limits

Local evidence and private cards: `recomp/analysis/d08af/` (ignored).
Accepted baseline executable:
`6725addce03114878375d08d81331a4bd4032aa5169d21fff74a586602b73ea6`.

The first experimental callbacks targeted functions absent from generated
entry instrumentation. A second prototype incorrectly required fall flag
0x20; live tracing showed entry uses 0x04000000. Neither prototype exercised
the blend. Native mock success alone was insufficient. Final live diagnostic
lines (`ttk-plunge`) establish that the corrected existing-hook implementation
actually runs. No generated C was edited.

The first slot-9 live comparison preserved initial plunge depth samples while
raising horizontal travel in the 0.5-0.7-second interval after sampled entry
from about 1013 to 2557 world units/second. This wall-clock sample comparison
is illustrative, not an exact simulation-boundary measurement or universal
speed guarantee. Collision can still stop motion; deeper geometry changes
when horizontal travel differs.

Initial host checks found about 25 GiB available memory, negligible swap use,
and no current CPU/memory/I/O pressure or running Timeshift/rsync process.
Those observations do not establish conditions during the user's earlier
report. No host-performance cause is asserted.

User acceptance confirms the reported handoff feels successful. Wider
campaign geometry is not exhaustively covered. The entry-resistance fix
D08AE and first-person swimming D11E remain accepted baselines.

The accepted-binary rerun used the same private harness and inputs. Slot-9
running-jump entry depth was 585 in both. Released-input runs ended at the
identical XYZ [31217, 10761, 20255] with zero velocity in both builds. The
run-off route also showed a reduced late horizontal dip (about 821 to 2743
units/second in the same sampled interval). Held-input travel is intentionally
greater; unchanged depth after hitting different geometry is not promised.

The pre-acceptance entry matrix contains 24 routes: pool/moat, running jump/run-off,
held/released/reapplied movement, both cameras at 60/120 Hz. All reach native
swim mode; sampled entry depths span 443-630. The three additional six-route
camera/rate batches report zero identity refusals. First/third-person captures
were inspected. Run-offs and running jumps exercise different entry heights.

Separate private regressions cover shallow-water crouch/movement, surface
dive, underwater movement and fire (three shots), ascent to mode 4, oxygen
consumption underwater and recovery to 12000 at the surface. ORIGINAL and
Vanilla smoke routes also complete. These are bounded routes, not exhaustive
campaign or audible-effect verification. The existing native water tests
cover shallow/deep classification and jump gates; no new full shoreline
campaign survey was performed.

All 40 original player save/card files match their initial hashes. Test
profiles/cards are private; no player settings are changed.

## Final verification

Final player build and `ttk-controls-test` pass with the owned executable,
clean LEVEL00, LEVEL01 and all 21 level overlays. New cases exercise the blend
magnitude, preservation of native velocity/vertical root, release, one-shot
scope, caller/epoch/code rejection, shallow/surface/dive/stroke exclusions,
ORIGINAL and Vanilla. Input and aim suites pass. `git diff --check` passes.
Final executable (identical to the live-tested v4 build):
`710de20931fbebc816ef47e3b601b0b76afb381a6f1c4fafd4c84f7d22d1605c`.

All diagnostic game processes stopped; the candidate executable was restored
after the accepted-build comparison. Framework edits already present at intake
were preserved. At closeout the runtime patch was re-exported unchanged and
verified against clean pinned framework 08ec704a974b1f3a16335b4afeb340b9eff19926;
all 51 patched files match. Submodule pins are unchanged.

Launch from this workspace: `python3 recomp/tools/local/run.py`. Load UI slot
9, hold Shift + W and press Space into the pool. Judge the deepest part of the
plunge; also release and reapply W, and compare P first/third-person views.
Capture is automatic. No settings change is required with SWIMMING MODERN.
