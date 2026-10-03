import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('disc_lab',ROOT/'tools/local/disc_lab.py')
lab=importlib.util.module_from_spec(spec);spec.loader.exec_module(lab)

def record(name,lba,size,flags=0):
    name=name if isinstance(name,bytes) else name.encode()
    r=bytearray(33+len(name)+(len(name)%2==0));r[0]=len(r)
    r[2:6]=lba.to_bytes(4,'little');r[6:10]=lba.to_bytes(4,'big')
    r[10:14]=size.to_bytes(4,'little');r[14:18]=size.to_bytes(4,'big')
    r[25]=flags;r[32]=len(name);r[33:33+len(name)]=name
    return r

def fixture(path,child=None):
    raw=bytearray(22*2352)
    p=bytearray(2048);p[:7]=b'\x01CD001\x01';p[80:84]=(22).to_bytes(4,'little')
    p[156:190]=record(b'\0',20,2048,2)
    raw[16*2352+24:16*2352+2072]=p
    entry=child if child is not None else record('FILE.BIN;1',21,4)
    raw[20*2352+24:20*2352+24+len(entry)]=entry
    raw[21*2352+24:21*2352+28]=b'TEST';path.write_bytes(raw)

class ParserTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
        self.path=Path(self.tmp.name)/'test.img'
    def open(self):
        d=lab.Disc(self.path);self.addCleanup(d.close);return d
    def test_read_and_inventory(self):
        fixture(self.path);d=self.open();self.assertEqual(d.read(21,4),b'TEST')
        self.assertEqual(d.files(),[{'path':'/FILE.BIN;1','lba':21,'size':4}])
    def test_truncated(self):
        self.path.write_bytes(b'x');self.assertRaises(ValueError,lab.Disc,self.path)
    def test_empty(self):
        self.path.write_bytes(b'');self.assertRaises(ValueError,lab.Disc,self.path)
    def test_bad_pvd(self):
        self.path.write_bytes(bytes(22*2352));self.assertRaises(ValueError,self.open().files)
    def test_out_of_bounds(self):
        fixture(self.path,record('FILE',22,2048));self.assertRaises(ValueError,self.open().files)
    def test_negative_extent(self):
        fixture(self.path);self.assertRaises(ValueError,self.open().read,-1,1)
    def test_unsafe_name(self):
        fixture(self.path,record('../x',21,1));self.assertRaises(ValueError,self.open().files)
    def test_directory_cycle(self):
        fixture(self.path,record('LOOP',20,2048,2));self.assertRaises(ValueError,self.open().files)
    def test_dual_endian_mismatch(self):
        r=record('FILE',21,4);r[9]=20;fixture(self.path,r);self.assertRaises(ValueError,self.open().files)
    def test_report_cannot_overwrite_input(self):
        fixture(self.path)
        before=self.path.read_bytes()
        import sys
        result=subprocess.run([sys.executable,str(ROOT/'tools/local/disc_lab.py'),'inspect',str(self.path),
                               '--output',str(self.path)],capture_output=True,text=True)
        self.assertNotEqual(result.returncode,0)
        self.assertIn('must not overwrite',result.stderr)
        self.assertEqual(self.path.read_bytes(),before)
    def test_unknown_identity_rejected(self):
        fixture(self.path);self.path.with_suffix('.ccd').write_text('[Disc]\nSessions=1\nDataTracksScrambled=0\n[TRACK 1]\nMODE=2\nINDEX 1=0\n')
        self.assertRaises(ValueError,lab.import_disc,self.path,Path(self.tmp.name)/'out','missing-validator')
    def test_cue_resolves_single_mode2_track(self):
        fixture(self.path)
        cue=Path(self.tmp.name)/'game.cue'
        bin_path=Path(self.tmp.name)/'game.bin'
        self.path.replace(bin_path)
        cue.write_text('FILE "game.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')
        self.assertEqual(lab.parse_cue(cue),bin_path.resolve())
        image,kind=lab.resolve_image(cue)
        self.assertEqual(image,bin_path.resolve())
        self.assertEqual(kind,'cue')
    def test_cue_import_does_not_require_ccd(self):
        fixture(self.path)
        cue=Path(self.tmp.name)/'game.cue'
        bin_path=Path(self.tmp.name)/'game.bin'
        self.path.replace(bin_path)
        cue.write_text('FILE "game.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')
        with self.assertRaises(ValueError) as raised:
            lab.import_disc(cue,Path(self.tmp.name)/'out','missing-validator')
        self.assertNotIn('CloneCD',str(raised.exception))
        self.assertNotIn('.ccd',str(raised.exception).lower())
    def test_ensure_game_dir_writes_readme(self):
        folder=Path(self.tmp.name)/'game'
        lab.ensure_game_dir(folder)
        self.assertTrue((folder/'README.md').is_file())
        self.assertIn('SLUS-00583',(folder/'README.md').read_text())
    def test_find_dump_reports_pal(self):
        workspace=Path(self.tmp.name)
        game=workspace/'game'
        game.mkdir()
        img=game/'europe.img'
        fixture(img)
        raw=bytearray(img.read_bytes())
        raw[16*2352+24+40:16*2352+24+51]=b'SLES01515  '
        img.write_bytes(raw)
        cue=game/'europe.cue'
        cue.write_text('FILE "europe.img" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')
        with self.assertRaises(ValueError) as raised:
            lab.find_valid_dump(workspace,create_game_dir=False)
        message=str(raised.exception)
        self.assertIn('error: no valid USA SLUS-00583 dump found.',message)
        self.assertIn('Europe/PAL',message)
        self.assertIn('SLES01515',message)

@unittest.skipUnless(os.getenv('DNTTK_IMAGE'),'set DNTTK_IMAGE for owned-disc integration tests')
class DiscIntegrationTests(unittest.TestCase):
    def test_identity_and_inventory(self):
        r=lab.inspect(os.environ['DNTTK_IMAGE'])
        self.assertIn(r['hashes']['sha256'],lab.DISC_SHA256S)
        self.assertEqual(r['executable']['sha256'],lab.EXE_SHA256)
        self.assertEqual(len(r['files']),548);self.assertEqual(len(r['overlay_groups']),30)
        self.assertEqual(len(r['extent_table_candidates']),453)
    def test_validator_good_and_corrupt(self):
        validator=ROOT/'build-tools'/('sector_check.exe' if os.name=='nt' else 'sector_check')
        self.assertTrue(validator.is_file(),'build sector_check first')
        with open(os.environ['DNTTK_IMAGE'],'rb') as f:
            f.seek(175345*2352);sector=f.read(2352)
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'sector.img';p.write_bytes(sector)
            r=subprocess.run([str(validator),str(p)],capture_output=True,text=True)
            self.assertEqual(r.returncode,0,r.stderr)
            self.assertEqual(json.loads(r.stdout)['bad_sectors'],0)
            for offset,key in [(100,'bad_edc'),(2076,'bad_ecc_p'),(2248,'bad_ecc_q')]:
                bad=bytearray(sector);bad[offset]^=1;p.write_bytes(bad)
                r=subprocess.run([str(validator),str(p)],capture_output=True,text=True)
                self.assertEqual(r.returncode,1);self.assertGreater(json.loads(r.stdout)[key],0)
            p.write_bytes(sector[:-1]);r=subprocess.run([str(validator),str(p)],capture_output=True,text=True)
            self.assertEqual(r.returncode,1)
if __name__=='__main__':unittest.main()
