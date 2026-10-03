#!/usr/bin/env python3
"""Apply the reviewed Time to Kill fixes to the pinned runtime, idempotently."""
from pathlib import Path
import subprocess
import shutil
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    patches = sorted((ROOT / 'patches').glob('time-to-kill-*.patch'))
    # Later seams can overlap earlier fixes. Verify the complete installed
    # stack by reversing copies, never by temporarily undoing live source.
    if installed_stack(patches):
        print('Reviewed runtime patch stack already applied')
        return
    for patch in patches:
        apply(patch)


def installed_stack(patches):
    if not patches:
        return True
    with tempfile.TemporaryDirectory(prefix='ttk-patch-check-') as temporary:
        scratch = Path(temporary)
        paths = {line[6:].split('\t')[0] for patch in patches
                 for line in patch.read_text().splitlines() if line.startswith('+++ b/')}
        for relative in paths:
            source = ROOT / 'psxrecomp' / relative
            if not source.is_file():
                return False
            target = scratch / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)
        for patch in reversed(patches):
            result = subprocess.run(['git', '-C', str(scratch), 'apply', '--reverse', str(patch)],
                                    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            if result.returncode:
                return False
    return True


def apply(patch):
    command = ['git', '-C', str(ROOT / 'psxrecomp'), 'apply']
    # Only accept pristine old code or the exact already-applied patch. Preserve
    # unrelated local edits and fail on conflicting runtime revisions.
    if subprocess.run(command + ['--reverse', '--check', str(patch)],
                      stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0:
        print(f'{patch.name} already applied')
        return
    subprocess.run(command + ['--check', str(patch)], check=True)
    subprocess.run(command + [str(patch)], check=True)
    print(f'Applied {patch.name}')


if __name__ == '__main__':
    main()
