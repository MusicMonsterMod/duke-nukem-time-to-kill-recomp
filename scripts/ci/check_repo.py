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
}

FORBIDDEN_NAMES = {
    "duke3d.grp",
    "scph1001.bin",
    "scph5501.bin",
    "scph7001.bin",
}

FORBIDDEN_PREFIXES = (
    "research/",
    "recomp/",
    "duke nukem",
    "documentation/logs/",
)

GITIGNORE_NEEDLES = (
    "/research/",
    "/recomp/",
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

    files = tracked_files()
    if not files:
        errors.append("git ls-files returned no tracked files")

    for relative in files:
        lowered = relative.lower()
        path = ROOT / relative
        if any(lowered.startswith(prefix) for prefix in FORBIDDEN_PREFIXES):
            errors.append(f"forbidden path tracked: {relative}")
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
