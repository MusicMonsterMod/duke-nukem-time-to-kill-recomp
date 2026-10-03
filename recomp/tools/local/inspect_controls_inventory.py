#!/usr/bin/env python3
"""Read-only SLUS-00583 identity evidence, without redistributing retail code."""
import argparse, hashlib, json, struct
from pathlib import Path
from disc_lab import EXE_SHA256

def inspect(path):
    data=Path(path).read_bytes()
    if hashlib.sha256(data).hexdigest()!=EXE_SHA256:raise ValueError('Unsupported executable')
    def offset(address):
        pos=address-0x80010000+2048
        if not 2048<=pos<len(data):raise ValueError('Bad executable address')
        return pos
    def half(address):return struct.unpack_from('<H',data,offset(address))[0]
    def word(address):return struct.unpack_from('<I',data,offset(address))[0]
    def string(address):return data[offset(address):].split(b'\0',1)[0].decode('ascii')
    rows=[]
    for slot in [*range(15),27,28,29]:
        label=half(0x800c4592+44*slot)
        rows.append({'slot':slot,'name':string(word(0x800c60f4+4*label)),'selectable':bool(half(0x800c4590+44*slot)&1)})
    assert rows[12]['name']=='PIPE BOMB' and rows[14]['name']=='DYNAMITE'
    assert rows[0]['name']=='MIGHTY BOOT' and rows[4]['name']=='DESERT EAGLE'
    return {'executable_sha256':EXE_SHA256,'weapons':rows,
        'evidence':'Weapon record +2 string ID through 800c60f4; original selector range 0..14; upgrades resolved at 8003df40',
        'items':{'1':'Jetpack','2':'Bio Mask','3':'Goggles','4':'Steroids (pickup-activated, hidden inventory label)','5':'Portable medkit'},
        'item_capacity_table':'800c2710 is runtime-initialized, not immutable identity material',
        'not_proven':'Natural ownership/acquisition, all eras, full campaign or human gameplay acceptance'}
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('exe',type=Path);p.add_argument('--output',type=Path)
    a=p.parse_args();text=json.dumps(inspect(a.exe),indent=2)+'\n'
    if a.output:a.output.write_text(text)
    else:print(text,end='')
