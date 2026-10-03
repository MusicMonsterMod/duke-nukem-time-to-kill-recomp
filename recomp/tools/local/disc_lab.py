#!/usr/bin/env python3
"""Read-only dump analysis and guarded import for the locally inspected US disc.

Accepts the same player-facing image types psxrecomp mounts: Redump/raw
MODE2/2352 .cue+.bin, a lone .bin/.img of that layout, and the original
CloneCD .img+.ccd+.sub trio. The runtime already opens those formats; this
tool still pins USA SLUS-00583 identity before writing recomp/disc/.

ISO logical-file hashes use 2048 bytes per sector. Form 2 raw extents are hashed
separately: a logical hash is NOT a complete XA/audio integrity identity.
"""
import argparse
import collections
import configparser
import hashlib
import json
import mmap
import os
import re
import shutil
import struct
import sys
from pathlib import Path

SECTOR = 2352
# CloneCD dump used for local bring-up.
DISC_SHA256 = '230a34c2512c6db1708657f9452c42cc4b0db95caefd7ebd32035d8ba9933f5d'
# Redump USA MODE2/2352 (same SLUS_005.83; upstream probe MD5 82d6ef06...).
DISC_SHA256_REDUMP = '708c040436c4a8bfe0ef43379e934172d0b94131ee3db47e406767c9d306a8c1'
DISC_SHA256S = frozenset({DISC_SHA256, DISC_SHA256_REDUMP})
EXE_SHA256 = 'b5c3ba610074bff184f089a49e51a22a35455cfef08757bd673a54f4057d5a7a'
KNOWN_IMAGE_BYTES = 449772960
WORKSPACE = Path(__file__).resolve().parents[3]
GAME_DIR = WORKSPACE / 'game'
SKIP_DUMP_DIRS = frozenset({'.git', '.svn', 'ignore'})
GAME_DIR_README = """Drop your owned USA Duke Nukem: Time to Kill dump in this folder.

Required disc: USA SLUS-00583. Europe/PAL (SLES-01515) will not work.

Accepted layouts:
- Redump-style .cue + .bin (TRACK 01 MODE2/2352)
- CloneCD .img + .ccd + .sub
- a raw MODE2/2352 .bin or .img

You can drop the files here or inside a subfolder. Then run:

  python3 recomp/tools/local/run.py
"""

def u32(b, o=0):
    return struct.unpack_from('<I', b, o)[0]

class Disc:
    def __init__(self, path):
        self.path = Path(path)
        if self.path.stat().st_size == 0 or self.path.stat().st_size % SECTOR:
            raise ValueError('image size is not a positive multiple of 2352')
        self.handle = self.path.open('rb')
        self.raw = mmap.mmap(self.handle.fileno(), 0, access=mmap.ACCESS_READ)
        self.count = len(self.raw) // SECTOR

    def close(self):
        self.raw.close()
        self.handle.close()

    def extent(self, lba, size):
        if lba < 0 or size < 0 or lba + (size + 2047)//2048 > self.count:
            raise ValueError('file extent outside image')
        return range(lba, lba + (size + 2047)//2048)

    def read(self, lba, size):
        return b''.join(self.raw[i*SECTOR+24:i*SECTOR+2072]
                        for i in self.extent(lba, size))[:size]

    def files(self):
        pvd = self.read(16, 2048)
        if pvd[:7] != b'\x01CD001\x01':
            raise ValueError('not an ISO9660 primary descriptor at Mode 2 offset 24')
        if u32(pvd, 80) != self.count:
            raise ValueError('ISO volume length does not match image')
        out, visited = [], set()
        def walk(lba, size, parent):
            if (lba, size) in visited:
                raise ValueError('repeated/cyclic directory extent')
            visited.add((lba, size))
            data = self.read(lba, size)
            pos = 0
            while pos < len(data):
                length = data[pos]
                if not length:
                    pos = (pos//2048+1)*2048
                    continue
                if length < 34 or pos+length > len(data) or pos%2048+length > 2048:
                    raise ValueError('invalid ISO directory record')
                r = data[pos:pos+length]; pos += length
                if r[32] > length-33:
                    raise ValueError('invalid ISO filename length')
                name = r[33:33+r[32]]
                if name in (b'\0', b'\1'):
                    continue
                name = name.decode('ascii')
                if '/' in name or '\\' in name or name in ('.', '..'):
                    raise ValueError('unsafe ISO filename')
                a, s = u32(r,2), u32(r,10)
                if a != int.from_bytes(r[6:10],'big') or s != int.from_bytes(r[14:18],'big'):
                    raise ValueError('ISO dual-endian fields disagree')
                self.extent(a,s)
                path = parent+'/'+name
                if r[25]&2:
                    walk(a,s,path)
                else:
                    out.append({'path':path, 'lba':a, 'size':s})
        r = pvd[156:190]
        walk(u32(r,2),u32(r,10),'')
        return out

def sha(b):
    return hashlib.sha256(b).hexdigest()

def inspect(path):
    disc = Disc(path)
    try:
        files = disc.files()
        hashes = {'sha256':sha(disc.raw), 'md5':hashlib.md5(disc.raw).hexdigest(),
                  'sha1':hashlib.sha1(disc.raw).hexdigest()}
        groups = collections.defaultdict(list)
        exe = None
        for item in files:
            data = disc.read(item['lba'],item['size'])
            item['logical_sha256'] = sha(data)
            sectors = disc.extent(item['lba'],item['size'])
            raw_hash = hashlib.sha256()
            modes = collections.Counter()
            for i in sectors:
                sector = disc.raw[i*SECTOR:(i+1)*SECTOR]
                raw_hash.update(sector)
                modes[str(sector[18])] += 1
            item['raw_extent_sha256'] = raw_hash.hexdigest()
            item['submodes'] = dict(modes)
            if item['path'].endswith('.OVR;1'):
                groups[sha(data)].append(item['path'])
                item['overlay_tag_candidate'] = u32(data) if len(data)>=4 else None
            if data.startswith(b'PS-X EXE'):
                if exe is not None:
                    raise ValueError('unexpected multiple PS-X EXE files')
                exe = {'path':item['path'], 'sha256':sha(data), 'size':len(data),
                       **{k:u32(data,o) for k,o in [('entry',16),('gp',20),('load',24),
                          ('payload_size',28),('bss',40),('bss_size',44),('stack',48)]}}
                payload=data[2048:]
        if exe is None:
            raise ValueError('no PS-X EXE found')
        if exe['payload_size'] != exe['size']-2048:
            raise ValueError('executable payload size mismatch')
        pairs=[]
        for item in files:
            if item['size'] and item['lba']>=23:
                pattern=struct.pack('<II',item['lba']-23,item['size'])
                hits=[exe['load']+m.start() for m in re.finditer(re.escape(pattern),payload) if m.start()%4==0]
                if hits:pairs.append({'path':item['path'],'addresses':hits})
        pvd = disc.read(16, 2048)
        volume_id = pvd[40:72].decode('ascii', 'replace').strip()
        return {'schema_version':1,'image_name':Path(path).name,'image_bytes':len(disc.raw),
                'sectors':disc.count,'hashes':hashes,'volume_id':volume_id,'executable':exe,'files':files,
                'overlay_groups':dict(groups),'extent_table_candidates':pairs,
                'notes':['Logical hashes omit Form 2 bytes after the first 2048; use raw hashes for media.',
                         'Extent pairs are static evidence, not proof of loader semantics.',
                         'Run sector_check separately for full EDC/ECC validation.']}
    finally:
        disc.close()

def parse_cue(cue_path):
    """Return the data-track BIN for a single-track MODE2/2352 cue."""
    cue_path = Path(cue_path).resolve()
    text = cue_path.read_text(encoding='utf-8', errors='replace')
    names = re.findall(r'FILE\s+"([^"]+)"\s+BINARY', text, flags=re.I)
    if not names:
        names = re.findall(r'FILE\s+(\S+)\s+BINARY', text, flags=re.I)
    if len(names) != 1:
        raise ValueError('only a single BINARY FILE cue is supported')
    modes = re.findall(r'TRACK\s+\d+\s+(\S+)', text, flags=re.I)
    if modes != ['MODE2/2352']:
        raise ValueError('only a single TRACK 01 MODE2/2352 cue is supported')
    if not re.search(r'INDEX\s+01\s+00:00:00', text, flags=re.I):
        raise ValueError('cue INDEX 01 must be 00:00:00')
    bin_path = Path(names[0])
    if not bin_path.is_absolute():
        bin_path = cue_path.parent / bin_path
    if not bin_path.is_file():
        raise ValueError(f'cue references missing bin: {bin_path}')
    return bin_path.resolve()


def sibling_with_suffix(path, suffix):
    cand = path.with_suffix(suffix)
    return cand if cand.is_file() else None


def resolve_image(path):
    """Map a player dump path to the raw MODE2/2352 image.

    .cue is preferred (psxrecomp's own picker). A .bin/.img with a sibling
    .cue uses that cue. CloneCD .ccd/.sub remain optional extras.
    """
    path = Path(path).resolve()
    suffix = path.suffix.lower()
    if suffix == '.cue':
        return parse_cue(path), 'cue'
    if suffix in ('.bin', '.img'):
        cue = sibling_with_suffix(path, '.cue')
        if cue is not None:
            data = parse_cue(cue)
            if data != path:
                raise ValueError(f'cue {cue.name} does not name {path.name}')
            return path, 'cue'
        return path, 'raw'
    raise ValueError('supported dumps are .cue, .bin, or .img (MODE2/2352)')


def require_clonecd_layout(ccd):
    conf = configparser.ConfigParser()
    conf.read(ccd)
    tracks = [s for s in conf.sections() if s.startswith('TRACK ')]
    if tracks != ['TRACK 1'] or conf.getint('TRACK 1', 'MODE') != 2 or conf.getint('TRACK 1', 'INDEX 1') != 0:
        raise ValueError('only verified single-track MODE2 INDEX 1=0 CloneCD layout supported')
    if conf.getint('Disc', 'DataTracksScrambled') != 0 or conf.getint('Disc', 'Sessions') != 1:
        raise ValueError('scrambled or multi-session CloneCD layout unsupported')


def identity_error(manifest):
    volume = manifest.get('volume_id') or 'unknown'
    digest = manifest['hashes']['sha256']
    return (
        f'unsupported disc identity (volume {volume!r}, sha256 {digest}). '
        'This build requires USA SLUS-00583 MODE2/2352. '
        'A Europe/PAL SLES dump will not run on the US recompilation.'
    )


def ensure_game_dir(path=None):
    """Create the player dump folder on first run."""
    directory = Path(path) if path is not None else GAME_DIR
    directory.mkdir(parents=True, exist_ok=True)
    readme = directory / 'README.md'
    if not readme.is_file():
        readme.write_text(GAME_DIR_README)
    return directory


def dump_search_roots(workspace=None):
    root = Path(workspace) if workspace is not None else WORKSPACE
    roots = [root / 'game']
    seen = {roots[0].resolve()} if roots[0].exists() else set()
    for extra in sorted(root.glob('Duke Nukem*')):
        if extra.is_dir() and extra.resolve() not in seen:
            roots.append(extra)
            seen.add(extra.resolve())
    return roots


def _iter_dump_files(root):
    if not root.is_dir():
        return
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [name for name in dirnames if name.lower() not in SKIP_DUMP_DIRS]
        for name in filenames:
            suffix = Path(name).suffix.lower()
            if suffix in {'.cue', '.bin', '.img'}:
                yield Path(dirpath) / name


def peek_volume(image):
    size = image.stat().st_size
    if size == 0 or size % SECTOR:
        return None
    with image.open('rb') as f:
        f.seek(16 * SECTOR + 24)
        pvd = f.read(72)
    if len(pvd) < 72 or pvd[:7] != b'\x01CD001\x01':
        return None
    return pvd[40:72].decode('ascii', 'replace').strip()


def file_sha256(path):
    with path.open('rb') as handle:
        return hashlib.file_digest(handle, 'sha256').hexdigest()


def classify_dump(path):
    """Return (status, detail) for a player dump path. status is valid, pal, mismatch, or skip."""
    try:
        image, kind = resolve_image(path)
    except (OSError, ValueError) as exc:
        return 'skip', str(exc)
    size = image.stat().st_size
    if size % SECTOR:
        return 'skip', f'{image.name}: not a raw MODE2/2352 image'
    volume = peek_volume(image)
    if volume is None:
        return 'skip', f'{image.name}: no PlayStation ISO header'
    serial = volume.replace('_', '').replace('.', '').replace('-', '')
    if serial.startswith('SLES') or serial.startswith('SCES'):
        return 'pal', f'{path.name}: Europe/PAL {volume} (need USA SLUS-00583)'
    if 'SLUS00583' not in serial:
        return 'skip', f'{path.name}: volume {volume!r} is not SLUS-00583'
    if size != KNOWN_IMAGE_BYTES:
        return 'skip', f'{path.name}: {size} bytes (need {KNOWN_IMAGE_BYTES})'
    digest = file_sha256(image)
    if digest in DISC_SHA256S:
        return 'valid', f'{path} ({kind}, {digest[:12]}...)'
    return 'mismatch', (
        f'{path.name}: USA SLUS-00583 image hash {digest} is not a known dump. '
        f'Accepted SHA-256: {" ".join(sorted(DISC_SHA256S))}'
    )


def find_valid_dump(workspace=None, create_game_dir=True):
    """Find one accepted USA dump, or raise ValueError with a console-ready report."""
    root = Path(workspace) if workspace is not None else WORKSPACE
    if create_game_dir:
        ensure_game_dir(root / 'game')
    notes = []
    valid = []
    for search in dump_search_roots(root):
        cues = []
        images = []
        for path in _iter_dump_files(search):
            if path.suffix.lower() == '.cue':
                cues.append(path)
            elif sibling_with_suffix(path, '.cue') is None:
                images.append(path)
        for path in sorted(cues) + sorted(images):
            status, detail = classify_dump(path)
            notes.append((status, detail))
            if status == 'valid':
                valid.append(path)
    if valid:
        chosen = valid[0]
        print(f'using dump: {chosen}', flush=True)
        return chosen.resolve()
    raise ValueError(format_dump_error(root, notes))


def format_dump_error(root, notes):
    game = root / 'game'
    lines = [
        f'error: no valid USA SLUS-00583 dump found.',
        f'  drop your owned USA disc in: {game}',
        '  required serial: SLUS-00583 (not Europe/PAL SLES-01515)',
        '  accepted SHA-256:',
    ]
    for digest in sorted(DISC_SHA256S):
        lines.append(f'    {digest}')
    if notes:
        lines.append('  found:')
        for status, detail in notes:
            lines.append(f'    [{status}] {detail}')
    else:
        lines.append('  found: (empty game folder)')
    return '\n'.join(lines)


def import_disc(path, output, integrity):
    path, output = Path(path).resolve(), Path(output).resolve()
    image, _kind = resolve_image(path)
    ccd = sibling_with_suffix(image, '.ccd')
    sub = sibling_with_suffix(image, '.sub')
    if ccd is not None:
        require_clonecd_layout(ccd)
    manifest = inspect(image)
    incoming = manifest['hashes']['sha256']
    if incoming not in DISC_SHA256S or manifest['executable']['sha256'] != EXE_SHA256:
        raise ValueError(identity_error(manifest))
    if sub is not None and sub.stat().st_size != manifest['sectors'] * 96:
        raise ValueError('subchannel length does not match sector count')
    # Always rerun the native validator against this exact input, never trust an unrelated report.
    import subprocess
    checked=subprocess.run([str(Path(integrity).resolve()),str(image)],capture_output=True,text=True,check=True)
    result=json.loads(checked.stdout)
    if result['sectors']!=manifest['sectors'] or result['bad_sectors'] or result['trailing_bytes']:
        raise ValueError('sector validation failed')
    output.mkdir(parents=True,exist_ok=True)
    dest=output/'time-to-kill.bin'
    if dest.exists():
        with dest.open('rb') as f:
            prepared=hashlib.file_digest(f,'sha256').hexdigest()
        if prepared not in DISC_SHA256S:
            raise ValueError('refusing to overwrite a different prepared image')
        if prepared != incoming:
            shutil.copyfile(image, dest)
    else:
        shutil.copyfile(image,dest)
    with dest.open('rb') as f:
        if hashlib.file_digest(f,'sha256').hexdigest()!=incoming:
            raise ValueError('prepared copy hash mismatch')
    cue='FILE "time-to-kill.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n'
    cue_path=output/'time-to-kill.cue'
    if cue_path.exists() and cue_path.read_text()!=cue:
        raise ValueError('refusing to overwrite a different CUE')
    cue_path.write_text(cue)
    disc=Disc(image)
    try:
        entry=next(f for f in manifest['files'] if f['path']==manifest['executable']['path'])
        exe_path=output/'SLUS_005.83'
        data=disc.read(entry['lba'],entry['size'])
        if exe_path.exists() and exe_path.read_bytes()!=data:
            raise ValueError('refusing to overwrite a different executable')
        exe_path.write_bytes(data)
    finally:disc.close()
    receipt={'sha256':incoming,'sector_integrity':result,
             'source':str(path),'image':str(image),
             'subchannels':'preserved in original when present; not consumed by CUE runtime'}
    if sub is not None:
        receipt['subchannel_sha256']=sha(sub.read_bytes())
    (output/'import-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
    return cue_path

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('action',choices=['inspect','import'])
    p.add_argument('image',type=Path,nargs='?')
    p.add_argument('--output',type=Path,required=True)
    p.add_argument('--validator',type=Path)
    args=p.parse_args()
    if args.action=='inspect':
        if args.image is None:
            p.error('inspect requires an image path')
        try:
            image,_kind=resolve_image(args.image)
        except ValueError as exc:
            p.error(str(exc))
        protected = {args.image.resolve(), image,
                     image.with_suffix('.ccd').resolve(), image.with_suffix('.sub').resolve(),
                     image.with_suffix('.cue').resolve()}
        if args.output.resolve() in protected:
            p.error('report output must not overwrite original media')
        report=inspect(image)
        args.output.parent.mkdir(parents=True,exist_ok=True)
        args.output.write_text(json.dumps(report,indent=2)+'\n')
        print(f"{len(report['files'])} files; {len(report['overlay_groups'])} distinct overlays; report: {args.output}")
        return
    if not args.validator:
        p.error('import requires --validator')
    try:
        source = args.image if args.image is not None else find_valid_dump()
        print(import_disc(source,args.output,args.validator))
    except ValueError as exc:
        print(str(exc), file=sys.stderr)
        raise SystemExit(1)
if __name__=='__main__':main()
