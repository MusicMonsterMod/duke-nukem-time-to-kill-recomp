#!/usr/bin/env python3
"""Export the complete authored framework delta without changing its real index."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]

def main():
    framework = ROOT / 'psxrecomp'
    target = ROOT / 'patches/time-to-kill-accepted-source.patch'
    with tempfile.TemporaryDirectory(prefix='ttk-export-index-') as directory:
        env = dict(os.environ, GIT_INDEX_FILE=str(Path(directory) / 'index'))
        def git(*args):
            return subprocess.check_output(['git', '-C', str(framework), *args], env=env)
        git('read-tree', 'HEAD')
        git('add', '-u')
        extra = git('ls-files', '--others', '--exclude-standard', '-z').decode().split('\0')
        extra = [p for p in extra if p]
        for path in extra:
            if not path.startswith(('runtime/', 'recompiler/')) or Path(path).suffix not in {'.c', '.h', '.cpp', '.py', '.cmake'}:
                raise SystemExit(f'Review unexpected untracked framework file before export: {path}')
        if extra:
            git('add', '--', *extra)
        target.write_bytes(git('diff', '--cached', '--binary', 'HEAD'))
    print(target)

if __name__ == '__main__':
    main()
