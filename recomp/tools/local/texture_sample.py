#!/usr/bin/env python3
"""D16 local specimen: inspect/export a VRAM region, then compile edited RGB art.

Retail-derived captures and specimens belong in ignored analysis/, never Git.
This is a developer input, not a mod pack format. Renderer contract: one region,
exact source and CLUT identity, nearest RGB replacement, native cutout/STP.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

FIELDS = ('depth', 'page_x', 'page_y', 'u', 'v', 'w', 'h', 'cx', 'cy', 'iw', 'ih')
MAGIC = b'TTKTX01\0'


def validate(m):
    for key in FIELDS:
        if type(m.get(key)) is not int or m[key] < 0:
            raise ValueError(f'{key}: nonnegative integer required')
    d, px, py, u, v, w, h, cx, cy, iw, ih = (m[k] for k in FIELDS)
    if d > 2 or px > 960 or px % 64 or py not in (0, 256):
        raise ValueError('invalid texture page/depth')
    if not (0 <= u < 256 and 0 <= v < 256 and 1 <= w <= 256-u and 1 <= h <= 256-v):
        raise ValueError('region must fit one non-wrapping texture page')
    ppw = 4 >> d
    pn = (16, 256, 0)[d]
    if u % ppw or w % ppw or px + (u+w)//ppw > 1024 or py+v+h > 512:
        raise ValueError('region must be word-aligned and fit VRAM')
    if cx > 1023 or cy > 511 or cx+pn > 1024:
        raise ValueError('CLUT must fit one VRAM row')
    if not (1 <= iw <= 1024 and 1 <= ih <= 1024) or iw % w or ih % h or iw//w != ih//h:
        raise ValueError('replacement needs uniform integer scale, at most 1024x1024')
    return ppw, pn


def decode(m, words, palette):
    ppw, pn = validate(m)
    if len(words) != m['w']//ppw*m['h'] or len(palette) != pn:
        raise ValueError('source/CLUT word count mismatch')
    rgb = bytearray()
    for y in range(m['h']):
        for x in range(m['w']):
            word = words[y*(m['w']//ppw)+x//ppw]
            raw = palette[(word >> ((x % ppw)*(16//ppw))) & ((1 << (16//ppw))-1)] if pn else word
            rgb.extend(((raw >> shift) & 31)*255//31 for shift in (0, 5, 10))
    return bytes(rgb)


def build(m, words, palette, rgb):
    validate(m)
    decode(m, words, palette)  # validates source sizes too
    if len(rgb) != m['iw']*m['ih']*3:
        raise ValueError('replacement RGB size mismatch')
    return MAGIC + struct.pack('<11I', *(m[k] for k in FIELDS)) + struct.pack(f'<{len(words)+len(palette)}H', *words, *palette) + rgb


def main():
    p = argparse.ArgumentParser(description=__doc__)
    sub = p.add_subparsers(dest='command', required=True)
    cap = sub.add_parser('capture')
    cap.add_argument('--port', type=int, required=True)
    cap.add_argument('--region', type=int, nargs=9, required=True,
                     metavar=('DEPTH','PAGE_X','PAGE_Y','U','V','W','H','CLUT_X','CLUT_Y'))
    cap.add_argument('--output', type=Path, required=True)
    compile_p = sub.add_parser('compile')
    compile_p.add_argument('capture', type=Path)
    compile_p.add_argument('image', type=Path)
    compile_p.add_argument('output', type=Path)
    a = p.parse_args()
    from PIL import Image
    if a.command == 'capture':
        from debug_client import request
        m = dict(zip(FIELDS[:9], a.region))
        m.update(iw=m['w'], ih=m['h'])
        ppw, pn = validate(m)
        def read(x,y,w,h):
            result = []
            for yy in range(y,y+h):
                for xx in range(x,x+w,128):
                    r=request(dict(cmd='gl_fbo_peek',x=xx,y=yy,w=min(128,x+w-xx),h=1), a.port)
                    if not r.get('ok'): raise RuntimeError(r)
                    result.extend(int(r['hex'][i:i+4],16) for i in range(0,len(r['hex']),4))
            return result
        words=read(m['page_x']+m['u']//ppw,m['page_y']+m['v'],m['w']//ppw,m['h'])
        palette=read(m['cx'],m['cy'],pn,1) if pn else []
        if words != read(m['page_x']+m['u']//ppw,m['page_y']+m['v'],m['w']//ppw,m['h']) or (pn and palette != read(m['cx'],m['cy'],pn,1)):
            raise ValueError('texture changed during capture; choose a stable state and retry')
        pixels=decode(m,words,palette)
        a.output.mkdir(parents=True,exist_ok=True)
        Image.frombytes('RGB',(m['w'],m['h']),pixels).save(a.output/'original.png')
        m.update(source_words=words,palette_words=palette,
                 source_sha256=hashlib.sha256(struct.pack(f'<{len(words)+len(palette)}H',*words,*palette)).hexdigest())
        (a.output/'capture.json').write_text(json.dumps(m,indent=2)+'\n')
        print(a.output/'capture.json')
    else:
        m=json.loads(a.capture.read_text())
        with Image.open(a.image) as image:
            if 'A' in image.getbands() and image.getchannel('A').getextrema() != (255,255):
                raise ValueError('RGB-only foundation: original game controls transparency; export opaque RGB art')
            m.update(iw=image.width,ih=image.height)
            validate(m)
            data=build(m,m['source_words'],m['palette_words'],image.convert('RGB').tobytes())
        a.output.write_bytes(data)
        print(f'{a.output}: {len(data)} bytes; {m["w"]}x{m["h"]} -> {m["iw"]}x{m["ih"]}')


if __name__ == '__main__':
    main()
