#!/usr/bin/env python3
"""Serialize the selected Bio Mask atlas to the PS1 renderer's 15-bit texels."""
from pathlib import Path
import argparse
from PIL import Image
ROOT=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser()
parser.add_argument('--asset-dir',type=Path,default=ROOT/'assets/models/biomask')
parser.add_argument('--output',type=Path,default=ROOT/'src/ttk/bio_mask_atlas.h')
args=parser.parse_args()
im=Image.open(args.asset_dir/'biomask-atlas.png').convert('RGB')
assert im.size==(256,128)
texels=[max(1,(r>>3)|((g>>3)<<5)|((b>>3)<<10)) for r,g,b in im.getdata()]
text='// Bio Mask atlas; source and provenance selected by export_biomask_atlas.py --asset-dir\n#pragma once\n#include <cstdint>\ninline constexpr uint16_t bio_mask_atlas[256*128]={\n'
text+='\n'.join(','.join(map(str,texels[i:i+32]))+',' for i in range(0,len(texels),32))+'\n};\n'
args.output.write_text(text)
