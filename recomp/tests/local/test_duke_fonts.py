import importlib.util,struct,sys,tempfile,unittest,zlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools/local'))
import build_duke_fonts as font

def chunk(kind,data):return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data))
class FontTests(unittest.TestCase):
    def test_indexed_palette_transparency_and_sub_filter(self):
        data=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',3,1,8,3,0,0,0))
        data+=chunk(b'PLTE',bytes([1,2,3,20,40,60,100,120,140]))+chunk(b'tRNS',bytes([0,255,128]))
        data+=chunk(b'IDAT',zlib.compress(bytes([1,1,1,254])))+chunk(b'IEND',b'')
        self.assertEqual(font.png(data),(3,1,[0xff14283c,0x8064788c,0]))
        broken=bytearray(data);broken[-1]^=1
        with self.assertRaises(ValueError):font.png(bytes(broken))
    def test_heading_case_and_missing_mapping(self):
        self.assertEqual(font.mapping(1,'a'),font.mapping(1,'A'))
        self.assertEqual(font.mapping(1,'['),font.mapping(1,'?'))
        self.assertEqual(font.mapping(0,'a'),2886)
        self.assertEqual(font.mapping(1,':'),3007)
    def test_unreviewed_archive_fails_before_unpacking(self):
        with tempfile.TemporaryDirectory() as d:
            source=Path(d)/'source.zip';source.write_bytes(b'bad')
            with self.assertRaisesRegex(ValueError,'unreviewed'):font.build(source,Path(d)/'fonts.pack')
