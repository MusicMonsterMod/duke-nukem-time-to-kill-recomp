# D35J4 - Slot-9 time-crystal perspective

2026-10-25. Done (user-accepted). User selected D35J4 after
requesting backlog intake for five reports. The other four remain Todo.

## Accepted closeout

User explicitly instructed "lock it in and close it out!". The delivered
D35J4 candidate is accepted, with implementation, regression tests, manual
and documentation included in the authorized MusicMonsterMod commit/push.
No post-acceptance gameplay changes, rebuilds or launches. The evidence and
coverage limits below remain; no other backlog job is started.

## Reproduction and cause

The current UI slot 9 differs from the earlier D35J crystal-use fixture.
Private natural input reaches the holder at 0x801d5dc4, type 182/kind 17,
near (25079, -7069, 48130). Holding E runs lower/upper pose 268, followed
by item handling and pose 267. Both poses set committed-action bit 0x40,
retain mode/history 0/0 and normal reciprocal camera ownership, but the
pre-fix accepted build reports unsupported-camera and first-person blend 0.

The earlier note 233/234 crystal case covered pose 266, placing an item at
a different structure. It did not establish coverage of this holder's take
and return sequence. Resident selector 0x80051608 selects 267 for interaction
kind 3 and otherwise 268 on land (131 underwater); 0x80051730 installs the
selected pose. The crystal-taking routine is 0x80091cec, also documented in
[note 117](117-d26f-crystals.md). No underwater variant is added here.

## Change

The existing presentation-only use predicate now includes the two reproduced
poses, 267 and 268, alongside 118/266/277. It retains the native action-bit,
mode/history, selected-view, identity, death/script and camera-owner guards.
USE INTERACTION VIEW controls these poses. Existing stable eye and native
interaction hands are reused; native callbacks, inventory events, timing,
movement restrictions and Vanilla are unchanged.

Native regression coverage now exercises both poses through the existing
transient-action contracts: presentation without movement permission, first
and third person, action preference, pause, save-load reseeding, replay state,
death, script and foreign camera ownership. Adjacent unsupported pose 269
remains excluded. No framework, generated-C, dependency-pin or schema changes.

## Evidence

Player build and native controls/input/aim/near-clipping suites pass. Controls
include the owned executable and all 21 selectable level-overlay fixtures.
Private slot-9 sequences at 60 and 120 presentation settings each sampled
39 frames in poses 267/268, all at first-person blend 1. Both actions complete,
return to idle and redraw the weapon. The full take/return cycle leaves the
mission inventory bytes unchanged, as expected for returning the item.
The action Third Person comparison sampled 41 action frames at blend 0,
then recovered the selected global first-person view. Global Third Person
remained third person, including all 42 sampled action frames.

Local scripts, traces, captures and private saves are under
`recomp/analysis/d35j4/`. The initial fixed-duration 120 route did not reach
the holder and failed the activation assertion; it is not a passing test.
The successful repeat steers by observed position with ordinary key input.
Earlier standing-use/empty-holder attempts did not activate the target either.
No teleport, animation forcing, object callback injection or guest RAM patch.
Live samples are sequential reads, not atomic frame snapshots or benchmarks.

Accepted executable SHA-256:
`7da56adc98ba7b4bde4b92591a06cd35953da1a7b117450607363684264b5663`.

Pause/resume during collection retains first person. A private mid-action
save resumes pose 268 and completes in first person. The first immediate-load
sample reports world-change/blend 0 during normal invalidation; all subsequent
stable samples retain blend 1. The initial overly strict load assertion included
that invalidation and failed; the repeat explicitly verifies the stable action
after it. A Vanilla load of the same native action completes at blend 0.
First-person captures of both poses were inspected. Diagnostic sessions stopped.
The player save/card source hashes and mtimes are unchanged; diagnostic runs
used private settings/cards. No player settings were written.

## Limits

User accepted the delivered eye/hand presentation. Recorded testing verifies the
current slot-9 holder sequence, not every crystal, underwater collection,
scripted camera or campaign interaction. No new menu preference is needed.
Implementation/documentation commit and push are explicitly authorized.

Launch: `python3 recomp/tools/local/run.py`. Select global First Person with
P and leave USE INTERACTION VIEW on First Person; gameplay captures
automatically. Test current UI slot 9's crystal holder.
