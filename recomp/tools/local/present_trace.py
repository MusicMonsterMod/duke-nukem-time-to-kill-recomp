#!/usr/bin/env python3
"""D17AY: measure presentation quantities that step at the game tick.

Reads `[present]` lines written by redraw workers with DNTTK_PRESENT_TRACE=1
(frame_replay.cpp `present_trace`): base frame serial, interpolation fraction
and a presented world point in the view of the draw that showed it. Redraws
are put in present order (serial + fraction) and each traced point is
projected to the screen (x right, y down) with the projection H, in pixels
of a display of the given width (native 320-wide pixels times width/320).

A point drawn smoothly moves by similar amounts from one redraw to the next.
A point that keeps the last game tick while the view moves between ticks
moves with the view inside a game frame and jumps when the next base frame
starts. Reported per name, as screen movement from the previous redraw per
game frame of elapsed time (base frame + fraction; redraws closer than 0.05
of a frame are skipped):

  boundary_px  mean rate into the first redraw of a new game frame
  interior_px  mean rate between redraws of the same game frame
  ratio        boundary / interior (near 1 when smooth; large when stepped)
  max_px       largest rate

--relative REF measures each point against another traced point on screen
(for example a hand against the object it holds); a steady hold then moves
by about 0 everywhere, and a tick-rate reader moves at the boundaries.

Usage: present_trace.py LOG [--h 160] [--width 2560] [--name NAME]
"""
import argparse, collections, math, re, sys

LINE = re.compile(r"\[present\] s=(\d+) a=([0-9.]+) body=(\d) n=(\S+) v=(-?[0-9.]+),(-?[0-9.]+),(-?[0-9.]+)")


STEP = re.compile(r"^\[step\] (\S+)")


def load(path, segment=None):
    rows = collections.defaultdict(dict)
    current = None
    with open(path, errors="replace") as f:
        for line in f:
            st = STEP.match(line)
            if st:
                current = st[1]
                continue
            if segment and (current is None or not re.fullmatch(segment, current)):
                continue
            m = LINE.search(line)
            if not m:
                continue
            s, a, body, name = int(m[1]), float(m[2]), int(m[3]), m[4]
            v = (float(m[5]), float(m[6]), float(m[7]))
            # A redraw drawn twice (cache refill) keeps its last sample.
            rows[name][(s, a)] = (body, v)
    return rows


def analyse(samples, h, width, ref=None, view=False):
    # The GTE projects view x/z and y/z by H into the 320-wide native screen;
    # scale native pixels to the display width.
    scale = h * width / 320.0
    keys = sorted(samples)
    pts = []
    for k in keys:
        _, (x, y, z) = samples[k]
        if view:
            # View-space units (a point near or behind the eye, or a scalar).
            q = (x, y, z)
            if ref is not None:
                r = ref.get(k)
                q = (x - r[1][0], y - r[1][1], z - r[1][2]) if r else None
            pts.append(q)
            continue
        if z <= 1:
            pts.append(None)
            continue
        q = (x / z * scale, y / z * scale)
        if ref is not None:
            r = ref.get(k)
            if not r or r[1][2] <= 1:
                pts.append(None)
                continue
            q = (q[0] - r[1][0] / r[1][2] * scale, q[1] - r[1][1] / r[1][2] * scale)
        pts.append(q)
    bnd, inn = [], []
    for i in range(1, len(keys)):
        a, b = pts[i - 1], pts[i]
        if not (a and b):
            continue
        # Only neighbouring base frames (a gap is a missed present, not a step).
        if keys[i][0] - keys[i - 1][0] > 1:
            continue
        # Movement per game frame: the redraw cadence varies (a busy machine
        # may show one redraw per frame), so divide by the time between them.
        dt = keys[i][0] + keys[i][1] - keys[i - 1][0] - keys[i - 1][1]
        if dt < 0.05:
            continue
        d = math.dist(a, b) / dt
        (bnd if keys[i][0] != keys[i - 1][0] else inn).append(d)
    def mean(v):
        return sum(v) / len(v) if v else float("nan")
    out = dict(redraws=len(keys), frames=len({k[0] for k in keys}),
               boundary_px=mean(bnd), interior_px=mean(inn),
               max_px=max(bnd + inn) if bnd or inn else float("nan"))
    out["ratio"] = out["boundary_px"] / out["interior_px"] if inn and out["interior_px"] > 1e-9 else float("inf")
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log")
    ap.add_argument("--h", type=float, default=160.0, help="projection H (160 = 90 degree FOV)")
    ap.add_argument("--width", type=int, default=2560)
    ap.add_argument("--name")
    ap.add_argument("--view", action="store_true", help="measure in view-space units instead of screen pixels")
    ap.add_argument("--segment", help="only lines after [step] markers matching this regex (probe logs)")
    ap.add_argument("--relative", help="measure each point on screen relative to this traced name")
    args = ap.parse_args()
    rows = load(args.log, args.segment)
    if not rows:
        print("no [present] lines (run with DNTTK_PRESENT_TRACE=1)")
        return 1
    for name in sorted(rows):
        if args.name and name != args.name:
            continue
        if name == args.relative:
            continue
        r = analyse(rows[name], args.h, args.width, rows.get(args.relative) if args.relative else None, args.view)
        print(f"{name:20s} redraws={r['redraws']:5d} frames={r['frames']:4d} "
              f"boundary_px={r['boundary_px']:7.3f} interior_px={r['interior_px']:7.3f} "
              f"ratio={r['ratio']:6.2f} max_px={r['max_px']:7.2f}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
