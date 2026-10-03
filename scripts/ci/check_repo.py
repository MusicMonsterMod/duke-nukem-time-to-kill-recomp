#!/usr/bin/env python3
"""Fail CI if this GitHub tree contains retail media, dumps, or invalid JSON."""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MAX_BYTES = 2 * 1024 * 1024

FORBIDDEN_SUFFIXES = {
    ".img",
    ".sub",
    ".ccd",
    ".bin",
    ".cue",
    ".iso",
    ".chd",
    ".ecm",
    ".mds",
    ".mdf",
    ".nrg",
    ".pbp",
    ".grp",
    ".wad",
    ".pk3",
    ".rff",
    ".mcd",
    ".mcr",
    ".mcs",
    ".gme",
    ".vmp",
    ".exe",
    ".dll",
    ".so",
    ".dylib",
    ".pst",
    ".pack",
    ".rom",
    ".o",
    ".a",
}

FORBIDDEN_NAMES = {
    "duke3d.grp",
    "scph1001.bin",
    "scph5501.bin",
    "scph7001.bin",
}

FORBIDDEN_PREFIXES = (
    "research/",
    "recomp/disc/",
    "recomp/generated/",
    "recomp/analysis/",
    "recomp/saves/",
    "recomp/config/",
    "recomp/assets/fonts/",
    "duke nukem",
    "documentation/logs/",
)

GITIGNORE_NEEDLES = (
    "/research/",
    "/recomp/disc/",
    "/recomp/generated/",
    "/recomp/analysis/",
    "/game/*",
    "!/game/README.md",
    "/Duke Nukem",
)


def tracked_files() -> list[str]:
    result = subprocess.run(
        ["git", "ls-files", "-z"],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
    )
    return [path for path in result.stdout.decode().split("\0") if path]


def main() -> int:
    errors: list[str] = []
    gitignore = (ROOT / ".gitignore").read_text(encoding="utf-8")
    for needle in GITIGNORE_NEEDLES:
        if needle not in gitignore:
            errors.append(f".gitignore is missing required pattern: {needle}")

    if "/recomp/" in gitignore.splitlines():
        errors.append("blanket recomp exclusion would hide the implementation")
    files = tracked_files()
    required = ("recomp/src/ttk/frame_replay.cpp", "recomp/src/ttk/modern_controls.cpp",
                "recomp/tools/local/build.py", "recomp/tools/local/run.py",
                "recomp/patches/time-to-kill-accepted-source.patch", ".gitmodules")
    for relative in required:
        if relative not in files:
            errors.append(f"required implementation missing: {relative}")
    entries = subprocess.check_output(["git", "ls-files", "--stage"], cwd=ROOT, text=True)
    allowed_links = {"recomp/psxrecomp", "recomp/recomp-ui"}
    for entry in entries.splitlines():
        if entry.startswith("160000 ") and entry.split("\t", 1)[1] not in allowed_links:
            errors.append(f"unexpected nested-repository pointer: {entry}")
    if not files:
        errors.append("git ls-files returned no tracked files")

    for relative in files:
        lowered = relative.lower()
        path = ROOT / relative
        if any(lowered.startswith(prefix) for prefix in FORBIDDEN_PREFIXES):
            errors.append(f"forbidden path tracked: {relative}")
            continue
        if lowered.startswith("game/") and relative != "game/README.md":
            errors.append(f"player media directory tracked: {relative}")
            continue
        if lowered.startswith("recomp/build"):
            errors.append(f"build output tracked: {relative}")
            continue
        suffix = Path(lowered).suffix
        name = Path(lowered).name
        if suffix in FORBIDDEN_SUFFIXES or name in FORBIDDEN_NAMES or name.startswith("scph"):
            errors.append(f"forbidden media/binary tracked: {relative}")
            continue
        if not path.is_file():
            continue
        size = path.stat().st_size
        if size > MAX_BYTES:
            errors.append(f"oversized blob ({size} bytes): {relative}")
        if suffix == ".json":
            try:
                json.loads(path.read_text(encoding="utf-8"))
            except (OSError, json.JSONDecodeError) as exc:
                errors.append(f"invalid JSON {relative}: {exc}")

    if errors:
        print("Repository hygiene checks failed:")
        for error in errors:
            print(f"  - {error}")
        return 1

    json_count = sum(1 for relative in files if relative.lower().endswith(".json"))
    print(
        f"OK: {len(files)} tracked files, {json_count} JSON reports valid, "
        f"no retail media, no file over {MAX_BYTES} bytes."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
