from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'tools/local'))
from ttk_hook_contract import qualifies

class HookContractTest(unittest.TestCase):
    def test_camera_pair_and_caller(self):
        good = dict(target='0x8003ade4', ra='0x80025ee8', a0='0x800d6eb0', a1='0x800d7198')
        self.assertTrue(qualifies(good))
        for key in good:
            self.assertFalse(qualifies({**good, key: '0'}))
    def test_shared_delta_producer_rejects_enemy(self):
        good = dict(target='0x800598f0', ra='0x8005a3e8', a0='0x800d71f8', a1='0x800d7198')
        self.assertTrue(qualifies(good))
        self.assertFalse(qualifies({**good, 'a0': '0x801d1654', 'a1': '0x801d15f4'}))
    def test_collision_other_modes_and_callers_refused(self):
        good = dict(target='0x8007926c', ra='0x80053548', a0='0x800d7198', a1='4')
        self.assertTrue(qualifies(good))
        self.assertFalse(qualifies({**good, 'a1': '6'}))
        self.assertFalse(qualifies({**good, 'ra': '0x80052fd0'}))
    def test_camera_constraint_stack_vector(self):
        good = dict(target='0x8003aa48', ra='0x8003aeb0', a0='0x800d6eb0', sp='0x801ffe98', a1='0x801ffeb0')
        self.assertTrue(qualifies(good))
        self.assertFalse(qualifies({**good, 'a1': '0x800d719c'}))
        self.assertFalse(qualifies({**good, 'sp': '0x801ffff8', 'a1': '0x80200010'}))

    def test_malformed_refused(self):
        for value in ({}, {'target': None}, {'target': 'bad!'}):
            self.assertFalse(qualifies(value))
