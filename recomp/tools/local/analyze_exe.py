#!/usr/bin/env python3
"""Emit conservative executable research metadata, never automatic hook ownership."""
import argparse
import json
import re
import struct
from pathlib import Path
from disc_lab import Disc, EXE_SHA256, sha

def analyze(image,seeds):
    disc=Disc(image)
    try:
        f=next(f for f in disc.files() if f['path']=='/SLUS_005.83;1')
        exe=disc.read(f['lba'],f['size'])
    finally:disc.close()
    if sha(exe)!=EXE_SHA256:raise ValueError('unsupported executable')
    data=exe[2048:];base=0x80010000
    words=struct.unpack('<'+'I'*(len(data)//4),data)
    strings=[]
    for m in re.finditer(rb'[ -~]{7,}',data):
        text=m.group().decode()
        if not re.search(r'camera|UpdateDukeMatrices|LineUpWith|Launchpoint Malloc|CD Error|Disk error',text,re.I):continue
        address=base+m.start(); refs=[]
        for i,w in enumerate(words):
            if w>>26!=15:continue
            reg=(w>>16)&31
            for j in range(i+1,min(i+7,len(words))):
                v=words[j];op=v>>26
                if (v>>21)&31==reg and op in (9,13):
                    lo=v&65535
                    value=((w&65535)<<16)+(lo if op==13 or lo<32768 else lo-65536)
                    if value==address:refs.append(hex(base+4*i))
        strings.append({'address':hex(address),'text':text,'candidate_lui_references':refs})
    seed_list=[int(s,16) for s in Path(seeds).read_text().splitlines() if s.startswith('0x')]
    targets={0x80000000|((w&0x3ffffff)<<2) for w in words if w>>26==3}
    report={'executable_sha256':EXE_SHA256,'seed_count':len(seed_list),
            'seed_outside_payload':[hex(s) for s in seed_list if not base<=s<base+len(data)],
            'seed_unaligned':[hex(s) for s in seed_list if s%4],
            'seed_not_raw_jal_target':[hex(s) for s in seed_list if s not in targets],
            'strings':strings,'warning':'Linear instruction and string-reference scans are candidates, not CFG verification.'}
    try:
        from capstone import Cs, CS_ARCH_MIPS, CS_MODE_MIPS32, CS_MODE_LITTLE_ENDIAN
        cs=Cs(CS_ARCH_MIPS,CS_MODE_MIPS32|CS_MODE_LITTLE_ENDIAN)
        report['startup_disassembly']=[{'address':hex(i.address),'mnemonic':i.mnemonic,'operands':i.op_str}
            for i in cs.disasm(data[0xab6fc-0x10000:0xab6fc-0x10000+128],0x800ab6fc)]
    except ImportError:report['startup_disassembly']='Install optional capstone==5.0.9 for decoded startup.'
    return report
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('image',type=Path)
    p.add_argument('--seeds',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();a.output.parent.mkdir(parents=True,exist_ok=True)
    a.output.write_text(json.dumps(analyze(a.image,a.seeds),indent=2)+'\n')
