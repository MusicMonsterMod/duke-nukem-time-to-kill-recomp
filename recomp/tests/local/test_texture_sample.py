"""D16 specimen wire and exact-identity contract, without retail inputs or GL."""
import importlib.util
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('texture_sample', ROOT/'tools/local/texture_sample.py')
sample = importlib.util.module_from_spec(spec)
spec.loader.exec_module(sample)


class TextureSampleTests(unittest.TestCase):
    def fixture(self, depth=0):
        m=dict(depth=depth,page_x=640,page_y=0,u=0,v=16,w=8,h=8,cx=0,cy=300,iw=32,ih=32)
        ppw,pn=sample.validate(m)
        words=[0x1234]*(8//ppw*8)
        palette=list(range(pn))
        return m,words,palette,bytes([80,120,160])*32*32

    def test_region_validation_and_indexed_decode(self):
        m,w,p,rgb=self.fixture()
        self.assertEqual(sample.decode(m,w,p)[:6],bytes([32,0,0,24,0,0]))
        for change in ({'u':1},{'w':0},{'w':257},{'depth':3},{'cx':1020},{'page_x':1},
                       {'iw':2048},{'iw':31},{'ih':64},{'page_y':1},{'page_x':960,'depth':2,'w':128}):
            with self.subTest(change=change),self.assertRaises(ValueError):
                sample.validate(dict(m,**change))

    @unittest.skipUnless(shutil.which('cc'), 'C compiler required')
    def test_native_parser_identity_and_invalidation(self):
        code=r'''
#include "ttk_texture_sample.h"
#include <assert.h>
int main(int argc,char **argv) {
    TtkTextureSample s={0};
    int expected=atoi(argv[2]);
    assert(ttk_texture_sample_load(argv[1],&s)==expected);
    if(!expected) {assert(!s.source);return 0;}
    assert(s.iw==32&&s.ih==32);
    assert(ttk_texture_sample_equal(&s,s.source,s.palette));
    size_t n=s.words_w*s.h;
    uint16_t *changed=malloc(n*2);memcpy(changed,s.source,n*2);changed[n-1]^=1;
    assert(!ttk_texture_sample_equal(&s,changed,s.palette));free(changed);
    if(s.palette_n) {
        uint16_t p[256];memcpy(p,s.palette,s.palette_n*2);p[s.palette_n-1]^=0x8000;
        assert(!ttk_texture_sample_equal(&s,s.source,p));
        assert(ttk_texture_sample_intersects(&s,s.cx,s.cy,s.cx,s.cy));
    }
    assert(ttk_texture_sample_intersects(&s,s.x,s.y,s.x,s.y));
    assert(ttk_texture_sample_intersects(&s,s.x+s.words_w-1,s.y+s.h-1,s.x+s.words_w,s.y+s.h));
    assert(!ttk_texture_sample_intersects(&s,s.x+s.words_w,s.y,s.x+s.words_w,s.y));
    assert(!ttk_texture_sample_intersects(&s,0,0,10,10));
    ttk_texture_sample_free(&s);assert(!s.source&&!s.rgb);
    return 0;
}
'''
        with tempfile.TemporaryDirectory() as d:
            d=Path(d);(d/'test.c').write_text(code)
            subprocess.run(['cc','-std=c11','-Wall','-Werror','-Wno-unused-parameter',
                            '-I',str(ROOT/'psxrecomp/runtime/src'),str(d/'test.c'),'-o',str(d/'test')],check=True)
            for depth in range(3):
                m,w,p,rgb=self.fixture(depth)
                data=sample.build(m,w,p,rgb)
                for name,payload,expected in [('valid',data,1),('short',data[:-1],0),('extra',data+b'x',0),
                                              ('magic',b'BADMAGIC'+data[8:],0),('huge',data[:44]+struct.pack('<I',0xffffffff)+data[48:],0)]:
                    with self.subTest(depth=depth,case=name):
                        (d/'sample').write_bytes(payload)
                        subprocess.run([str(d/'test'),str(d/'sample'),str(expected)],check=True)


if __name__=='__main__':
    unittest.main()
