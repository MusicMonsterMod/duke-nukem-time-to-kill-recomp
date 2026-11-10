#!/usr/bin/env python3
"""Check the job board split (D58).

MODERNIZATION_JOBS.md holds the vision, the job table and the working rules.
Job descriptions and their work logs live in jobs/<category>.md, the dated
session log in jobs/log-YYYY-MM.md. This script checks that:

- every table ID is unique and links to the category file its family belongs
  to, at an anchor that exists there when the job has a description;
- every job description has a table row and lives in exactly one file, the
  file of its category;
- the log files hold only dated entries;
- every board file is under the repository size limit.

With --against REV it also compares the content with the single-file board at
that Git revision: every non-blank line of the old board and its history files
must still be present somewhere, apart from the known moved stubs and headings.
"""

from __future__ import annotations

import argparse
import collections
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BOARD = ROOT / "MODERNIZATION_JOBS.md"
JOBS = ROOT / "jobs"
MAX_BYTES = 2 * 1024 * 1024

# category file -> (title, scope written in the file header)
CATEGORIES = {
    "traversal": ("Traversal", "D08 movement, jumping, climbing, swimming, jetpack and pushing (D08A and its children are in items)"),
    "items": ("Items, inventory and HUD", "D08A and its children: weapon and item shortcuts, inventory, gadgets and HUD icons"),
    "controls": ("Controls", "D03-D07, D09, D10, D27 and D28: input, camera, aiming and controller"),
    "first-person": ("First person", "D11, D12, D14, D35, D36, D47 and D48"),
    "weapons-combat": ("Weapons and combat", "D38, D39, D44, D49 and D51"),
    "graphics": ("Graphics", "D13, D15, D16, D17, D40, D46, D52 and D53"),
    "lighting": ("Lighting", "D41 and its children, D54-D57"),
    "menus-ui": ("Menus and UI", "D19-D21, D26, D29, D42 and D43"),
    "campaign-performance": ("Campaign, audio and performance", "D18, D22, D23, D37, D45 and D50"),
    "project": ("Release and project", "D01, D02, D24, D25, D30-D34, D58 and later project jobs, R01, PRIV01"),
}

FAMILY = {}
for _cat, _nums in {
    "controls": [3, 4, 5, 6, 7, 9, 10, 27, 28],
    "first-person": [11, 12, 14, 35, 36, 47, 48],
    "weapons-combat": [38, 39, 44, 49, 51],
    "graphics": [13, 15, 16, 17, 40, 46, 52, 53],
    "lighting": [41, 54, 55, 56, 57],
    "menus-ui": [19, 20, 21, 26, 29, 42, 43],
    "campaign-performance": [18, 22, 23, 37, 45, 50],
    "project": [1, 2, 24, 25, 30, 31, 32, 33, 34, 58],
}.items():
    for _n in _nums:
        FAMILY[_n] = _cat

JOB_ID = r"(?:D\d+|PRIV\d+|R\d+)[A-Z0-9]*"
ID_HEADING = re.compile(rf"^### ({JOB_ID})\b")
DESC_HEADING = re.compile(rf"^### ({JOB_ID}) [-\u2014] ")
ROW = re.compile(rf"^\| (?:\[)?({JOB_ID})(?:\]\(([^)]*)\))? \|")


def category_of(job_id: str) -> str:
    """Category file for a job ID. New families must be added to FAMILY."""
    if not job_id.startswith("D"):
        return "project"
    if job_id.startswith("D08"):
        return "items" if re.fullmatch(r"D08A\d*", job_id) else "traversal"
    num = int(re.match(r"D(\d+)", job_id).group(1))
    if num not in FAMILY:
        raise KeyError(f"{job_id}: family D{num:02d} has no category in scripts/ci/check_jobs.py")
    return FAMILY[num]


def id_sort_key(job_id: str):
    m = re.match(r"([A-Z]+)(\d+)(.*)", job_id)
    key = [m.group(1) != "D", m.group(1), int(m.group(2))]
    for tok in re.findall(r"[A-Z]+|\d+", m.group(3)):
        key.append((0, len(tok), tok, 0) if tok.isalpha() else (1, 0, "", int(tok)))
    return key


def slugify(heading: str) -> str:
    """GitHub-style heading anchor."""
    text = re.sub(r"\[([^\]]*)\]\([^)]*\)", r"\1", heading)
    text = text.lower().replace("`", "").replace("*", "")
    text = re.sub(r"[^\w\- ]", "", text)
    return text.replace(" ", "-")


def anchors_of(lines: list[str]) -> set[str]:
    seen = collections.Counter()
    out = set()
    in_code = False
    for line in lines:
        if line.startswith("```"):
            in_code = not in_code
        if in_code:
            continue
        m = re.match(r"^#{1,6} (.*)", line)
        if m:
            base = slugify(m.group(1).strip())
            out.add(base if not seen[base] else f"{base}-{seen[base]}")
            seen[base] += 1
    return out


def read(p: Path) -> list[str]:
    return p.read_text(encoding="utf-8").split("\n")


def check(errors: list[str]) -> dict:
    board = read(BOARD)
    rows = {}
    for n, line in enumerate(board, 1):
        m = ROW.match(line)
        if not m:
            continue
        jid, link = m.group(1), m.group(2)
        if jid in rows:
            errors.append(f"MODERNIZATION_JOBS.md:{n}: {jid} has two table rows")
        rows[jid] = (n, link)
        if ID_HEADING.match(line):
            errors.append(f"MODERNIZATION_JOBS.md:{n}: job description left on the board")
    for n, line in enumerate(board, 1):
        if ID_HEADING.match(line):
            errors.append(f"MODERNIZATION_JOBS.md:{n}: job description {line[4:30]} belongs in jobs/")

    files = {p.stem: read(p) for p in sorted(JOBS.glob("*.md"))}
    expected = set(CATEGORIES) | {s for s in files if re.fullmatch(r"log-\d{4}-\d\d", s)}
    for stem in files:
        if stem not in expected:
            errors.append(f"jobs/{stem}.md: not a category or monthly log file")
    for cat in CATEGORIES:
        if cat not in files:
            errors.append(f"jobs/{cat}.md: missing")

    where = collections.defaultdict(set)
    for stem, lines in files.items():
        for n, line in enumerate(lines, 1):
            m = ID_HEADING.match(line)
            if not m:
                continue
            jid = m.group(1)
            if stem.startswith("log-"):
                # Work-log subheadings ("### D07C implementation - date") may stay;
                # a description heading ("### D07C - Name") may not.
                if DESC_HEADING.match(line):
                    errors.append(f"jobs/{stem}.md:{n}: job description {jid} in a log file")
                continue
            where[jid].add(stem)
            try:
                cat = category_of(jid)
            except KeyError as exc:
                errors.append(str(exc))
                continue
            if cat != stem:
                errors.append(f"jobs/{stem}.md:{n}: {jid} belongs in jobs/{cat}.md")
        if stem.startswith("log-"):
            for n, line in enumerate(lines, 1):
                if line.startswith("## ") and not line.startswith(f"## {stem[4:]}-"):
                    errors.append(f"jobs/{stem}.md:{n}: log entry not dated in {stem[4:]}")
    for jid, stems in where.items():
        if len(stems) > 1:
            errors.append(f"{jid}: descriptions in several files: {sorted(stems)}")
        if jid not in rows:
            errors.append(f"{jid}: has a description but no row in the job table")

    anchors = {stem: anchors_of(lines) for stem, lines in files.items()}
    for jid, (n, link) in rows.items():
        try:
            cat = category_of(jid)
        except KeyError as exc:
            errors.append(str(exc))
            continue
        target = f"jobs/{cat}.md"
        if not link:
            errors.append(f"MODERNIZATION_JOBS.md:{n}: {jid} does not link to {target}")
            continue
        path, _, frag = link.partition("#")
        if path != target:
            errors.append(f"MODERNIZATION_JOBS.md:{n}: {jid} links to {path}, expected {target}")
        if jid in where and not frag:
            errors.append(f"MODERNIZATION_JOBS.md:{n}: {jid} link has no anchor to its description")
        if frag and frag not in anchors.get(cat, set()):
            errors.append(f"MODERNIZATION_JOBS.md:{n}: {jid} anchor #{frag} not found in {target}")

    for p in [BOARD, *sorted(JOBS.glob("*.md"))]:
        size = p.stat().st_size
        if size > MAX_BYTES:
            errors.append(f"{p.relative_to(ROOT)}: {size} bytes, over the {MAX_BYTES} limit")

    return {"rows": rows, "where": where, "files": files}


def git_show(rev: str, path: str) -> list[str] | None:
    r = subprocess.run(["git", "-C", str(ROOT), "show", f"{rev}:{path}"],
                       capture_output=True, text=True)
    return r.stdout.split("\n") if r.returncode == 0 else None


ALLOWED_LOSS = [
    re.compile(r"^\| (?:D\d+|PRIV\d+|R\d+)[A-Z0-9]* \|"),       # table rows gained links
    re.compile(r"^## 2026-09-27 to 2026-09-30 - session log moved$"),
    re.compile(r"^## 2026-10-01 - session log moved$"),
    re.compile(r"^These entries are in \[the "),
    re.compile(r"^## (Foundation and controls|Graphics and playback|Player features and release work"
               r"|First-person presentation quality audit intake - 2026-10-26"
               r"|2026-10-25 - Gadget modernization and slot-5 jump artifact intake"
               r"|First-person action roadmap \(D35 research\))$"),  # empty section headings
    re.compile(r"^## D17AA - "),                                  # now a ### job heading
    re.compile(r"^# Job board history - "),                       # history file headers
    re.compile(r"^Dated session log entries from 2026-09-2|^Dated session log entries from|^`MODERNIZATION_JOBS.md` on 2026-11-|^file size limit\. Job descriptions|^limit\. Job descriptions"),
]


def compare(rev: str, errors: list[str]) -> None:
    old = []
    for path in ["MODERNIZATION_JOBS.md", "documentation/jobs-history-2026-09.md",
                 "documentation/jobs-history-2026-10-01.md"]:
        lines = git_show(rev, path)
        if lines is None:
            errors.append(f"--against {rev}: {path} not found")
            return
        old += lines
    new = read(BOARD)
    for p in sorted(JOBS.glob("*.md")):
        new += read(p)
    have = collections.Counter(line.rstrip() for line in new if line.strip())
    lost = []
    for line in old:
        line = line.rstrip()
        if not line.strip():
            continue
        if have[line] > 0:
            have[line] -= 1
        elif not any(p.search(line) for p in ALLOWED_LOSS):
            lost.append(line)
    for line in lost[:40]:
        errors.append(f"lost since {rev}: {line[:120]}")
    if len(lost) > 40:
        errors.append(f"... {len(lost) - 40} more lost lines")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--against", metavar="REV",
                    help="also check that no text of the single-file board at REV was lost")
    args = ap.parse_args()
    errors: list[str] = []
    info = check(errors)
    if args.against:
        compare(args.against, errors)
    for e in errors:
        print(e)
    rows, where = info["rows"], info["where"]
    print(f"{len(rows)} table rows, {len(where)} with descriptions, "
          f"{len(info['files'])} files in jobs/: {'FAILED' if errors else 'ok'}")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
