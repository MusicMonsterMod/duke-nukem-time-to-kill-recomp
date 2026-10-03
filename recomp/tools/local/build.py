#!/usr/bin/env python3
"""Build a local playable candidate from the validated CloneCD dump, on Linux/Windows."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[2]

def run(args,env):
    print('+', ' '.join(map(str,args)),flush=True)
    subprocess.run(list(map(str,args)),cwd=ROOT,env=env,check=True)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--image',type=Path,help='dump .cue/.bin/.img; default: search game/')
    p.add_argument('--jobs',type=int,default=4)
    p.add_argument('--configure-only',action='store_true')
    p.add_argument('--skip-movie-overlay',action='store_true',
                   help='omit the GCC movie shard (slower intro playback)')
    args=p.parse_args()
    if args.jobs<1:p.error('--jobs must be positive')
    env=os.environ.copy()
    local_venv=ROOT.parent/'.venv'/('Scripts' if os.name=='nt' else 'bin')
    if local_venv.is_dir():env['PATH']=str(local_venv)+os.pathsep+env.get('PATH','')
    for tool in ['cmake','ninja']:
        if not shutil.which(tool,path=env['PATH']):p.error(f'{tool} missing from PATH; see documentation/04-build-and-run.md')
    sys.path.insert(0, str(ROOT/'tools/local'))
    import disc_lab
    disc_lab.ensure_game_dir()
    try:
        image=(args.image.resolve() if args.image is not None else disc_lab.find_valid_dump())
    except ValueError as exc:
        print(str(exc), file=sys.stderr)
        return 1
    exe='.exe' if os.name=='nt' else ''
    run([sys.executable,'tools/local/apply_runtime_patches.py'],env)
    run(['cmake','-S','tools/local','-B','build-tools','-G','Ninja','-DCMAKE_BUILD_TYPE=Release'],env)
    run(['cmake','--build','build-tools','--parallel',args.jobs],env)
    run([sys.executable,'tools/local/disc_lab.py','import',str(image),'--output','disc',
         '--validator',f'build-tools/sector_check{exe}'],env)
    run(['cmake','-S','psxrecomp/recompiler','-B','build-recompiler','-G','Ninja','-DCMAKE_BUILD_TYPE=Release'],env)
    run(['cmake','--build','build-recompiler','--target','psxrecomp-game','psxrecomp-bios','--parallel',args.jobs],env)
    env['PSXRECOMP_GAME']=str(ROOT/'build-recompiler'/('psxrecomp-game'+exe))
    env['PSXRECOMP_BIOS']=str(ROOT/'build-recompiler'/('psxrecomp-bios'+exe))
    run([sys.executable,'psxrecomp/psxrecomp_cli.py','generate','--config','game.local.toml',
         '--project-root',ROOT,'--disc','disc/time-to-kill.cue'],env)
    run(['cmake','--preset','local-dev'],env)
    if not args.configure_only:
        run(['cmake','--build','--preset','local-dev','--parallel',args.jobs],env)
        if not args.skip_movie_overlay:
            run([sys.executable,'tools/local/build_movie_overlay.py','--jobs',args.jobs],env)
if __name__=='__main__':
    raise SystemExit(main() or 0)
