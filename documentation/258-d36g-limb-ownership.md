# D36G - Shared limb presentation ownership

2026-10-26. Done, user-accepted. User: "lock it in and close it out".
Implementation, tests and documentation commit/push closeout are authorized.

## Scope and implementation

The accepted [D36 audit](257-d36-first-person-presentation-audit.md) remains the
source of evidence labels, ownership boundaries and later visual requirements.
This is its behavior-equivalent foundation, not D36H-L visual implementation.
D37A/D19Q3 remain Needs playtest and are included in the baseline candidate.

- `limb_presentation.h` describes the current semantic part, owner, transform
  space, depth domain, lighting correction and cleanup lifetime. Owners are
  native, native interaction, hidden arm, display hand and kick. Costume joint
  indices still pass through the existing verified semantic mapping.
- Existing hand/kick/native-arm hooks claim that descriptor after their original
  eligibility checks. The near clipper consumes its depth/admission policy.
  Attachment matrices, weapon framing, animation, native events and damage are
  unchanged. Display hands and kick legs keep the viewmodel depth domain;
  interaction arms keep native transforms, lighting and world occlusion.
- The joint-depth bridge remains traversal-only. Hidden armed mantle limbs stay
  excluded; the existing armed traversal hand bridge is retained until an
  intervening transform. Push/Use/Mirror gain no new clipping in this job.
- Joint/draw boundaries restore the existing fade scope and discard the owner.
  Actor entry also retires an interrupted primary scope before a reflected or
  foreign draw. World/load and replay replacement forget ownership without
  writing old scope data into the replacement RAM image. Replay workers rebuild
  ownership at their actual draw hooks.
- No new guest allocation, packet buffer, save field, replay snapshot field,
  preference, asset, hook address or framework change. Existing scratch reserve
  and fallback allocation paths are unchanged. The descriptor is a small host
  value, with no dynamic storage. No manual update is needed for this plumbing.

## Verification

**S - source/native tests:** player build succeeds. `ttk-controls-test` passes
with the original LEVEL00 guard fixture, owned LEVEL01 and all level overlays.
The suite retains weapon matrices/attachments, semantic costume mapping, kick
geometry/timing/hits, native interaction matrices, fade restoration, action/global
third-person, Vanilla, pause, world reset and replay checks. New checks cover the
owner/part/space/depth/light metadata, hidden-arm exclusion, traversal-only bridge,
interrupted actor/reflection boundaries, and replay/load token retirement.
The load test runs in a child process to keep its camera reset from contaminating
later pose comparisons in the shared fixture.

`ttk-input-test` passes under Xvfb; the offscreen SDL backend cannot provide the
relative-mouse assertion and was not used as the passing input result.
`ttk-aim-test` and `ttk-near-test` pass. The D37A native recovery/GPU-clearing and
animation-header checks pass via `test_animation_save_restore.py`.

**L - private runs:** local evidence is `recomp/analysis/d36g/`. `baseline-final`
is the preceding executable; `candidate-60` and `candidate-120` use separate
settings, copied test saves and private writable cards. Offscreen OpenGL, dummy
audio, 1280 window request, 2x internal scale, debug port 9487. Confirmed state
samples include slot-8 push/pull 121/120, slot-10 vending 277, slot-12 ladder 186,
pistol presentation, active kick and kick release. Slot-11 load/use was sampled
but did not establish a full held-item interaction sequence.

At 60 Hz, RGB comparisons are pixel-identical for the sampled slot-8 idle,
push/pull, slot-10 idle/vending, pistol, active kick and kick release images.
Slot-11 images differ by one red-channel unit in one pixel. Ladder images have
small differences (mean channel delta below one on the 0-255 scale); sparse
asynchronous captures do not establish their exact cause. Side-by-side visual
inspection found the same framing and limb policy. This preserves existing
floating/cut limb defects rather than certifying their quality.

The 120 Hz run started three replay workers and exercised the same named owners.
Visual comparison found consistent presentation, with sampling/animation variation.
This is not a measured performance benchmark or proof of 120 displayed frames/s.
Attempted live view-toggle probes did not change the reported view and are not
counted as live third-person coverage; global/action third-person is covered by
the native suite. The last supplementary view-probe run also missed the push
interaction; successful push coverage comes from `candidate-60`/`candidate-120`.

Player saves/cards/settings hashes and mtimes match the preservation manifest.
All private game processes stopped. Existing framework modifications were left
untouched; no framework patch export was required for title-source-only changes.

## Candidate and review

Baseline: `736c45ab5046f640a036e119a15d4401d312361d567445ed7e60776717e2fcbf`.
Candidate: `e2062e66f70eb0bbe20be9c8a7bb8bfbf9e5cf234107779d4e98dcd3c979f1d3`.

Please check that first-person weapons, Q kick, push/pull, ladders and close-use
interactions look and behave as before, including P view changes and pause/load.
This bounded check is not full campaign, costume or interruption certification.
D36H-L remain unstarted. No next job is selected automatically.

```sh
python3 recomp/tools/local/run.py
```

Modernized is default, gameplay capture is automatic, and P toggles first/third
person. `--vanilla` selects original play for one launch without saving it.


## Acceptance and closeout - 2026-10-26

User accepted D36G and explicitly authorized lock-in and closeout. The foundation
is Done; the verification above remains bounded to its recorded tests and runs.
Acceptance does not establish additional live view-toggle, costume or campaign
coverage. No new test, build or launch was needed for documentation closeout.
Authored title implementation, tests and documentation are included in the
MusicMonsterMod commit/push. Existing framework modifications stay untouched.
D36H-L are unstarted and D37A/D19Q3 retain their separate Needs playtest statuses.
