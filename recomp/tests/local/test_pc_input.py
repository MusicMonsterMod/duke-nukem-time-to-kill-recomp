import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

TOOLS = Path(__file__).resolve().parents[2] / 'tools/local'
sys.path.insert(0, str(TOOLS))
import pc_input
import player_profiles as profiles


class InputBindingsTest(unittest.TestCase):
    def test_hud_binding_migration_and_keypad(self):
        old = {k: v for k, v in pc_input.DEFAULTS.items() if k not in ('hud_less', 'hud_more')}
        old['camera_recenter'] = 'Minus'
        migrated = pc_input.add_missing_actions(old)
        self.assertEqual(migrated['camera_recenter'], 'Minus')
        self.assertEqual(migrated['hud_less'], 'Unbound')
        self.assertEqual(migrated['hud_more'], 'Equals')
        rebound = pc_input.rebind(migrated, ['hud_less=KPMinus', 'hud_more=KPPlus'])
        self.assertTrue(pc_input.wire(rebound).endswith(',86,87'))

    def test_defaults_wire_and_separate_movement(self):
        self.assertEqual(len(pc_input.validate(pc_input.DEFAULTS)), 55)
        self.assertTrue(pc_input.wire(pc_input.DEFAULTS).startswith('1:26,22,4,7,'))
        # D08A5: Comma/Period browse the mission inventory; original strafe is unbound.
        self.assertTrue(pc_input.wire(pc_input.DEFAULTS).endswith(',54,55,75,78,74,77,80,79,63,66,6,-2,45,46'))  # D10C look keys, D10D turn keys, D20 quick keys last
        self.assertEqual((pc_input.DEFAULTS['original_strafe_left'], pc_input.DEFAULTS['original_strafe_right']),
                         ('Unbound', 'Unbound'))
        old = {k: v for k, v in pc_input.DEFAULTS.items() if not k.startswith('mission_')}
        old.update(original_strafe_left='Comma', original_strafe_right='Period', mission_browse='Backslash')
        new = pc_input.add_missing_actions(old)
        self.assertEqual((new['mission_previous'], new['mission_next'], new['original_strafe_left'],
                          new['original_strafe_right']), ('Comma', 'Period', 'Unbound', 'Unbound'))
        self.assertNotIn('mission_browse', new)
        old.update(original_strafe_left='Q', original_strafe_right='Period', quick_kick='Comma')
        new = pc_input.add_missing_actions(old)  # customized keys stay; the new action is unbound
        self.assertEqual((new['quick_kick'], new['original_strafe_left'], new['mission_previous'], new['mission_next']),
                         ('Comma', 'Q', 'Unbound', 'Period'))
        self.assertTrue(all(int(pad) == 0 for name, _, _, pad in pc_input.ROWS if name.startswith('move_')))

    def test_free_look_migration_preserves_custom_middle_button(self):
        old = dict(pc_input.DEFAULTS)
        del old['camera_free_look']
        self.assertEqual(pc_input.add_missing_actions(old)['camera_free_look'], 'Mouse3')
        old['fire'] = 'Mouse3'
        migrated = pc_input.add_missing_actions(old)
        self.assertEqual(migrated['fire'], 'Mouse3')
        self.assertEqual(migrated['camera_free_look'], 'Unbound')

    def test_crouch_secondary_migration(self):
        old = dict(pc_input.DEFAULTS)
        del old['crouch_alt']
        self.assertEqual(pc_input.add_missing_actions(old)['crouch_alt'], 'C')
        old['holster'] = 'C'
        migrated = pc_input.add_missing_actions(old)
        self.assertEqual(migrated['holster'], 'C')
        self.assertEqual(migrated['crouch_alt'], 'Unbound')
        swapped = pc_input.rebind(pc_input.DEFAULTS, ['crouch_alt=Unbound', 'crouch=C'])
        self.assertEqual(swapped['crouch'], 'C')

    def test_grab_defaults_and_migration(self):
        # D08T1: Mouse2 moves from precision aim to Grab; Alt is the second Grab.
        self.assertEqual((pc_input.DEFAULTS['grab'], pc_input.DEFAULTS['grab_alt'], pc_input.DEFAULTS['original_aim']),
                         ('Mouse2', 'Alt', 'Unbound'))
        old = {k: v for k, v in pc_input.DEFAULTS.items() if k not in ('grab', 'grab_alt', 'camera_free_look')}
        old['original_aim'] = 'Mouse2'
        migrated = pc_input.migrate_grab(old)
        self.assertEqual((migrated['grab'], migrated['grab_alt'], migrated['original_aim']), ('Mouse2', 'Alt', 'Unbound'))
        custom = dict(old, original_aim='Mouse3')
        migrated = pc_input.migrate_grab(custom)
        self.assertEqual((migrated['grab'], migrated['original_aim']), ('Mouse2', 'Mouse3'))
        taken = dict(old, original_aim='Mouse3', interact='Mouse2')
        migrated = pc_input.migrate_grab(taken)
        self.assertNotEqual(migrated['grab'], 'Mouse2')
        self.assertEqual(migrated['interact'], 'Mouse2')
        self.assertTrue(pc_input.wire(pc_input.DEFAULTS).split(',')[17:19] == ['-3', '226'])

    def test_conflict_is_atomic_and_swap_works(self):
        before = dict(pc_input.DEFAULTS)
        with self.assertRaisesRegex(ValueError, 'conflicts'):
            pc_input.rebind(before, ['jump=W'])
        self.assertEqual(before, pc_input.DEFAULTS)
        swapped = pc_input.rebind(before, ['jump=W', 'move_forward=Space'])
        self.assertEqual(swapped['move_forward'], 'Space')
        self.assertEqual(swapped['jump'], 'W')

    def test_reserved_malformed_and_unknown_inputs(self):
        for value in ['Escape', 'F10', 'Tab', 'X', 'C', 'F', 'Left', 'Ctrl+C', 'Mouse6', '999', '']:
            with self.assertRaises(ValueError):
                pc_input.rebind(pc_input.DEFAULTS, ['fire=' + value])
        with self.assertRaises(ValueError):
            pc_input.rebind(pc_input.DEFAULTS, ['missing=G'])

    def test_v1_migration_retains_profile(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'profiles.json'
            data = profiles.defaults()
            data['version'] = 1
            data['active'] = 'modernized'
            for p in data['profiles'].values():
                del p['bindings']
            data['profiles']['modernized']['presentation']['renderer'] = 'software'
            raw = json.dumps(data)
            path.write_text(raw)
            loaded, notices = profiles.load(path)
            self.assertEqual(loaded['version'], profiles.defaults()['version'])
            self.assertEqual(loaded['active'], 'modernized')
            self.assertEqual(loaded['profiles']['modernized']['presentation']['renderer'], 'software')
            self.assertEqual(loaded['profiles']['modernized']['bindings'], pc_input.DEFAULTS)
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_text(), raw)
            self.assertTrue(notices)

    def test_invalid_saved_bindings_recover_with_visible_conflict(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'profiles.json'
            data = profiles.defaults()
            data['profiles']['modernized']['bindings']['fire'] = 'W'
            profiles.save(path, data)
            loaded, notices = profiles.load(path)
            self.assertIn('conflicts', '\n'.join(notices))
            self.assertEqual(loaded['profiles']['modernized']['bindings'], pc_input.DEFAULTS)

    def test_cli_persists_rebinding_and_rejects_conflict_without_write(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'profiles.json'
            cmd = [sys.executable, str(TOOLS / 'run.py'), '--settings-file', str(path)]
            subprocess.run(cmd + ['--bind', 'jump=G'], check=True, capture_output=True)
            raw = path.read_bytes()
            failed = subprocess.run(cmd + ['--bind', 'fire=G'], capture_output=True, text=True)
            self.assertNotEqual(failed.returncode, 0)
            self.assertIn('conflicts', failed.stderr)
            self.assertEqual(path.read_bytes(), raw)
            shown = subprocess.check_output(cmd + ['--show-bindings'], text=True)
            self.assertIn('jump=G', shown)
            subprocess.run(cmd + ['--vanilla', '--show-settings'], check=True, capture_output=True)
            loaded, _ = profiles.load(path)
            self.assertEqual(loaded['profiles']['modernized']['bindings']['jump'], 'G')

    def test_menu_conflict_then_correction(self):
        settings = profiles.defaults()
        replies = iter(['6', 'jump=W', '6', 'jump=G', '5'])
        output = []
        edited = profiles.menu(settings, lambda _: next(replies), output.append)
        self.assertIn('conflicts', '\n'.join(output))
        self.assertEqual(edited['profiles']['modernized']['bindings']['jump'], 'G')
        self.assertEqual(settings, profiles.defaults())


class FeedbackMigrationTest(unittest.TestCase):
    def test_old_defaults_and_custom_keys(self):
        old={k:v for k,v in pc_input.DEFAULTS.items() if k in list(pc_input.DEFAULTS)[:13]}
        old.update(original_strafe_left='Q',original_strafe_right='E',walk='R')
        new=pc_input.migrate_bindings(old)
        self.assertEqual(new['interact'],'E')
        self.assertEqual(new['walk'],'R')
        # D08A5: the Q/E -> Comma/Period strafe migration now hands those keys
        # on to the mission inventory.
        self.assertEqual((new['original_strafe_left'],new['mission_previous']),('Unbound','Comma'))
        self.assertEqual((new['original_strafe_right'],new['mission_next']),('Unbound','Period'))
        old.update(fire='E',original_strafe_right='Mouse1')
        new=pc_input.migrate_bindings(old)
        self.assertEqual(new['fire'],'E')
        self.assertEqual(new['original_strafe_right'],'Mouse1')
        self.assertNotEqual(new['interact'],'E')

class ControlsMigrationTest(unittest.TestCase):
    def test_v6_defaults_and_customizations(self):
        old={k:v for k,v in pc_input.DEFAULTS.items() if k in list(pc_input.DEFAULTS)[:14]}
        old['crouch']='V'
        new=pc_input.migrate_bindings(old)
        self.assertEqual(new['crouch'],'LCtrl')
        self.assertEqual(new['medkit'],'M')
        old['crouch']='M';old['jump']='J'
        new=pc_input.migrate_bindings(old)
        self.assertEqual(new['crouch'],'M');self.assertEqual(new['jump'],'J')
        self.assertNotEqual(new['medkit'],'M');self.assertNotEqual(new['jetpack'],'J')
        pc_input.validate(new)

class MenuKeyNamesTest(unittest.TestCase):
    # D19G: KEY BINDINGS writes the profile's tokens itself (game_menu.cpp).
    def test_menu_tokens_match_profile_tokens(self):
        import re
        source = (TOOLS.parents[1] / 'src/ttk/game_menu.cpp').read_text()
        named = {int(code): token for code, token in re.findall(r'\{(-?\d+),"(\w+)","[^"]*"\}', source)}
        menu = dict(named)
        menu.update({4 + i: chr(65 + i) for i in range(26)})
        menu.update({29 + i: str(i) for i in range(1, 10)}); menu[39] = '0'
        bindable = {code: token for token, code in pc_input.TOKENS.items()}
        for code, token in bindable.items():
            self.assertEqual(menu.get(code), token, code)
        for code, token in named.items():
            self.assertEqual(pc_input.TOKENS.get(token), code, token)

class D21ProfileTest(unittest.TestCase):
    def test_schema53_adds_free_look_without_stealing_middle_mouse(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'profile.json'
            old = profiles.defaults()
            old['version'] = 53
            for profile in old['profiles'].values():
                del profile['bindings']['camera_free_look']
                profile['bindings']['fire'] = 'Mouse3'
            profiles.save(path, old)
            loaded, _ = profiles.load(path)
            self.assertEqual(loaded['version'], 60)
            for profile in loaded['profiles'].values():
                self.assertEqual(profile['bindings']['fire'], 'Mouse3')
                self.assertEqual(profile['bindings']['camera_free_look'], 'Unbound')

    def test_schema52_preserves_custom_bindings_and_adds_sound(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'profile.json'
            old = profiles.defaults()
            old['version'] = 52
            for p in old['profiles'].values():
                p['bindings'].pop('crouch_alt')
                p['bindings']['holster'] = 'C'
                p['bindings']['crouch'] = 'G'
                for k in profiles.SOUND_DEFAULTS:
                    p['controls'].pop(k)
            profiles.save(path, old)
            loaded, _ = profiles.load(path)
            p = loaded['profiles']['modernized']
            self.assertEqual((p['bindings']['holster'], p['bindings']['crouch'], p['bindings']['crouch_alt']), ('C','G','Unbound'))
            self.assertEqual({k:p['controls'][k] for k in profiles.SOUND_DEFAULTS}, profiles.SOUND_DEFAULTS)

    def test_sound_sidefile_roundtrip_and_invalid_atomicity(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / 'profile.json'
            settings = profiles.defaults()
            values = dict(sound_master=0, sound_effects=50, sound_music=100, sound_speech=25, sound_mode=1)
            data = dict(version=1, active='modernized', controls=values, presentation={})
            profiles.menu_settings_path(path).write_text(json.dumps(data))
            self.assertTrue(profiles.absorb_menu_settings(path, settings))
            profiles.save(path, settings)
            loaded, _ = profiles.load(path)
            self.assertEqual({k:loaded['profiles']['modernized']['controls'][k] for k in values}, values)
            before = copy.deepcopy(loaded)
            data['controls']['sound_master'] = 101
            profiles.menu_settings_path(path).write_text(json.dumps(data))
            self.assertFalse(profiles.absorb_menu_settings(path, loaded))
            self.assertEqual(loaded, before)
