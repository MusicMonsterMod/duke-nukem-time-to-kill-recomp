#!/usr/bin/env python3
"""Serialize the licensed Photon atlas to the PS1 renderer's 15-bit texels."""
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[2]
im=Image.open(ROOT/'assets/models/biomask/biomask-atlas.png').convert('RGB')
assert im.size==(256,128)
texels=[max(1,(r>>3)|((g>>3)<<5)|((b>>3)<<10)) for r,g,b in im.getdata()]
text='// CC0 Poly Haven atlas, provenance: assets/models/biomask/provenance.json\n#pragma once\n#include <cstdint>\ninline constexpr uint16_t bio_mask_atlas[256*128]={\n'
text+='\n'.join(','.join(map(str,texels[i:i+32]))+',' for i in range(0,len(texels),32))+'\n};\n'
(ROOT/'src/ttk/bio_mask_atlas.h').write_text(text)
