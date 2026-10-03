import json
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools/local'))
from summarize_weapon_aim import summarize

class WeaponEvidenceTest(unittest.TestCase):
    def test_correlates_actual_impact_and_excludes_unsupported_beam(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d);(p/'report.json').write_text(json.dumps({'binary_sha256':'fixture','exit_code':0}))
            (p/'runtime.log').write_text('''ttk-aim-query kind=1
 tt ignored
 tt ignored
'''.replace(' tt ignored\n','')+'''ttk-aim-query kind=0
ttk-aim-shot weapon=4 target=0,0,1000 direction=0,0,4096 cover=0
ttk-launch type=0 origin=0,0,0 direction=0,0,4096
ttk-flight projectile=800daa38 type=0 from=0,0,0 to=0,0,2000 direction=0,0,4096
ttk-impact projectile=800daa38 type=0 point=0,0,1000
ttk-launch type=15 origin=0,0,0 direction=0,0,4096
ttk-flight projectile=800daa98 type=15 from=0,0,0 to=0,0,2000 direction=0,0,4096
ttk-impact projectile=800daa98 type=15 point=0,0,1000
''')
            r=summarize(p)
            self.assertEqual(r['impacts'][0]['distance_to_view_target'],0)
            self.assertEqual(r['impacts'][0]['perpendicular_launch_error'],0)
            self.assertIsNone(r['impacts'][1]['launch']['aim'])
            self.assertNotIn('distance_to_view_target',r['impacts'][1])

    def test_unmatched_impact_is_not_claimed_as_aim_verification(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d);(p/'report.json').write_text(json.dumps({'binary_sha256':'fixture'}))
            (p/'runtime.log').write_text('ttk-impact projectile=800daa38 type=0 point=1,2,3\n')
            r=summarize(p)
            self.assertNotIn('launch',r['impacts'][0])
            self.assertNotIn('distance_to_view_target',r['impacts'][0])
