#!/usr/bin/env python3
"""Build a deterministic local glyph pack from Duke message/menu PNG sprites.
Message glyphs prefer owned assets/fonts/Messages when --messages is set;
Atomic headings still come from the reviewed ZIP archive. Standard library only.
"""
import argparse,hashlib,json,struct,zlib,zipfile
from pathlib import Path
SOURCE_SHA='d716e22b2e49ff7adcd6cd15ba2f5001326f75dd7c9248a4c5d1153fc118d429'
MAGIC=b'TTKFONT1'

def png(data):
    if data[:8]!=b'\x89PNG\r\n\x1a\n':raise ValueError('PNG signature')
    p=8;compressed=b'';palette=None;alpha=b'';width=height=0
    while p<len(data):
        n=struct.unpack_from('>I',data,p)[0];kind=data[p+4:p+8];chunk=data[p+8:p+8+n]
        if p+12+n>len(data) or zlib.crc32(kind+chunk)!=struct.unpack_from('>I',data,p+8+n)[0]:raise ValueError('PNG chunk CRC')
        if kind==b'IHDR':
            width,height,depth,colour,compression,filtering,interlace=struct.unpack('>IIBBBBB',chunk)
            if not(0<width<=32 and 0<height<=32) or (depth,colour,compression,filtering,interlace)!=(8,3,0,0,0):raise ValueError('unsupported glyph PNG')
        elif kind==b'PLTE':palette=chunk
        elif kind==b'tRNS':alpha=chunk
        elif kind==b'IDAT':compressed+=chunk
        p+=n+12
        if kind==b'IEND':break
    if not palette or not width:raise ValueError('PNG palette/header')
    raw=zlib.decompress(compressed)
    if len(raw)!=(width+1)*height:raise ValueError('PNG raster size')
    rows=[];previous=[0]*width
    for y in range(height):
        f=raw[y*(width+1)];row=list(raw[y*(width+1)+1:(y+1)*(width+1)])
        for x in range(width):
            a=row[x-1] if x else 0;b=previous[x];c=previous[x-1] if x else 0
            if f==1:predict=a
            elif f==2:predict=b
            elif f==3:predict=(a+b)//2
            elif f==4:
                q=a+b-c;dist=[abs(q-a),abs(q-b),abs(q-c)];predict=[a,b,c][dist.index(min(dist))]
            elif f==0:predict=0
            else:raise ValueError('PNG filter')
            row[x]=(row[x]+predict)&255
        rows+=row;previous=row
    pixels=[]
    for index in rows:
        if index*3+3>len(palette):raise ValueError('PNG palette index')
        r,g,b=palette[index*3:index*3+3];a=alpha[index] if index<len(alpha) else 255
        pixels.append((a<<24)|(r<<16)|(g<<8)|b if a else 0)
    return width,height,pixels

def checksum(data):
    h=2166136261
    for v in data:h=((h^v)*16777619)&0xffffffff
    return h

def mapping(style,ch):
    if not style:return 2822+ord(ch)-ord('!')
    ch=ch.upper()
    if 'A'<=ch<='Z':return 2940+ord(ch)-65
    if '0'<=ch<='9':return 2930+ord(ch)-48
    return {'-':2929,'_':2929,'.':3002,',':3003,'!':3004,'?':3005,';':3006,':':3007,'/':3008,'\\':3008,'%':3009,"'":3022,'"':3022,'`':3022}.get(ch,3005)

def message_png(messages_dir,tile,z):
    """Load one Messages glyph. Prefer local extract; fall back to ZIP paths."""
    idx=tile-2816
    local_candidates=[]
    if messages_dir:
        if tile==2834:
            local_candidates.append(messages_dir/'Atomic Edition'/f'TILES011_{idx}.png')
        local_candidates.append(messages_dir/f'TILES011_{idx}.png')
    for path in local_candidates:
        if path.is_file():
            return str(path.as_posix()),png(path.read_bytes())
    zip_path=f'Font/Messages/Atomic Edition/TILES011_{idx}.png' if tile==2834 else f'Font/Messages/TILES011_{idx}.png'
    return zip_path,png(z.read(zip_path))

def build(archive,output,messages_dir=None):
    source=archive.read_bytes()
    if hashlib.sha256(source).hexdigest()!=SOURCE_SHA:raise ValueError('unreviewed font archive; expected supplied PNG ZIP')
    if messages_dir is not None:messages_dir=Path(messages_dir)
    records=bytearray();pixels=bytearray();manifest=[];table_size=2*95*8
    with zipfile.ZipFile(archive) as z:
        for style in range(2):
            for code in range(32,127):
                ch=chr(code);line_height=7 if style==0 else 15
                if ch==' ':width=height=0;values=[];advance=4 if style==0 else 8;path=None;y=0
                else:
                    tile=mapping(style,ch)
                    if style==0:
                        path,(width,height,values)=message_png(messages_dir,tile,z)
                    else:
                        path=f'Font/Menu/Atomic Edition/01. Normal (Palette 00)/TILES011_{tile-2816}.png'
                        width,height,values=png(z.read(path))
                    advance=width+1
                    if ch in "'\"`^~":y=0
                    elif ch in '-+=:*<>':y=max(0,(line_height-height)//2)
                    else:y=max(0,line_height-height)
                records+=struct.pack('<BBBBI',width,height,advance,y,table_size+len(pixels))
                pixels+=struct.pack('<'+'I'*len(values),*values)
                manifest.append(dict(style=['message','atomic'][style],character=ch,source=path,width=width,height=height,advance=advance,y=y))
    payload=bytes(records+pixels);pack=MAGIC+struct.pack('<II',len(payload),checksum(payload))+payload
    output.parent.mkdir(parents=True,exist_ok=True);output.write_bytes(pack)
    credits={}
    for name in ['DukeNukemSmallFont.pk3','DukeNukemAtomicFont.pk3']:
        with zipfile.ZipFile(archive.parent/name) as z:
            credits[name]={n:z.read(n).decode('utf-8',errors='replace') for n in z.namelist() if n.lower().split('.')[0] in ('credits','info','fontdefs')}
    output.with_suffix('.json').write_text(json.dumps(dict(
        source=archive.name,
        messages_dir=str(messages_dir) if messages_dir else None,
        source_sha256=SOURCE_SHA,
        pack_sha256=hashlib.sha256(pack).hexdigest(),
        credits=credits,
        policy='Local sprite conversion. Message glyphs prefer owned assets/fonts/Messages when supplied; Atomic menu glyphs remain from the reviewed ZIP. Original graphics: 3D Realms. PK3 metadata credited Jimmy as submitter. PNG palette/transparency retained. Ordinary text keeps logical case; Atomic folds lowercase to uppercase; missing glyphs use question mark. No redistribution permission added.',
        glyphs=manifest),indent=2)+'\n')
    return pack

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('archive',type=Path)
    p.add_argument('output',type=Path)
    p.add_argument('--messages',type=Path,default=None,help='owned Messages PNG dir (e.g. assets/fonts/Messages); preferred for style 0')
    a=p.parse_args()
    build(a.archive,a.output,a.messages)
