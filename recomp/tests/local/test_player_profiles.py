import copy
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

TOOLS = Path(__file__).resolve().parents[2] / 'tools/local'
sys.path.insert(0, str(TOOLS))
import player_profiles as profiles
import pc_input
import run


class ProfilesTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name) / 'profiles.json'

    def cli(self, *args):
        return subprocess.run([sys.executable, str(TOOLS / 'run.py'), '--settings-file', str(self.path),
                               '--show-settings', *args], capture_output=True, text=True, check=True).stdout

    def test_precision_migration_and_independent_choices(self):
        old = profiles.defaults()
        old['version'] = 22
        for mode in profiles.MODES:
            old['profiles'][mode]['controls'].pop('geometry_precision')
            old['profiles'][mode]['controls'].pop('texture_precision')
        old['profiles']['modernized']['controls']['mouse_sensitivity'] = 0.23
        self.path.write_text(json.dumps(old))
        loaded, notices = profiles.load(self.path)
        self.assertEqual(loaded['profiles']['modernized']['controls']['mouse_sensitivity'],0.23)
        self.assertEqual(loaded['profiles']['modernized']['controls']['geometry_precision'],'original')
        self.cli('--geometry-precision','corrected','--texture-precision','original')
        loaded, _ = profiles.load(self.path)
        self.assertEqual(loaded['profiles']['modernized']['controls']['geometry_precision'],'corrected')
        self.assertEqual(loaded['profiles']['modernized']['controls']['texture_precision'],'original')
        self.cli('--mode','vanilla')
        self.cli('--mode','modernized')
        self.assertIn('Mesh geometry: corrected; textures: original',self.cli())
        self.cli('--reset-profile','modernized')
        self.assertIn('Mesh geometry: original; textures: original',self.cli())

    def test_precision_launch_gates_and_menu(self):
        with patch.object(Path,'is_file',return_value=True), patch.object(run.subprocess,'call',return_value=0) as launch, patch.object(run,'ensure_movie_shard'):
            base=['--settings-file',str(self.path),'--no-session-log']
            run.main(base+['--geometry-precision','corrected','--texture-precision','corrected'])
            self.assertEqual(launch.call_args.kwargs['env']['DNTTK_GEOMETRY_PRECISION'],'corrected')
            self.assertEqual(launch.call_args.kwargs['env']['DNTTK_TEXTURE_PRECISION'],'corrected')
            run.main(base+['--renderer','software'])
            self.assertEqual(launch.call_args.kwargs['env']['DNTTK_GEOMETRY_PRECISION'],'original')
            self.assertEqual(launch.call_args.kwargs['env']['DNTTK_TEXTURE_PRECISION'],'original')
            run.main(base+['--renderer','opengl','--mode','vanilla'])
            self.assertEqual(launch.call_args.kwargs['env']['DNTTK_GEOMETRY_PRECISION'],'original')
            run.main(base+['--mode','modernized','--renderer','opengl'])
            self.assertEqual(launch.call_args.kwargs['env']['DNTTK_GEOMETRY_PRECISION'],'corrected')
        answers=iter(['g','corrected','original','5'])
        settings=profiles.menu(profiles.defaults(),read=lambda _:next(answers),write=lambda *_:None)
        self.assertEqual(settings['profiles']['modernized']['controls']['geometry_precision'],'corrected')
        self.assertEqual(settings['profiles']['modernized']['controls']['texture_precision'],'original')

    def test_defaults_and_independence(self):
        settings, notices = profiles.load(self.path)
        self.assertEqual(settings['active'], 'modernized')
        self.assertFalse(self.path.exists())
        self.assertEqual(notices, [])
        settings['profiles']['modernized']['presentation']['renderer'] = 'software'
        self.assertEqual(settings['profiles']['vanilla']['presentation']['renderer'], 'opengl')

    def test_cli_restart_switch_and_reset(self):
        self.cli('--mode', 'modernized', '--renderer', 'software')
        self.assertIn('Renderer: software', self.cli())
        self.assertIn('View aiming', self.cli())
        self.assertIn('Renderer: opengl', self.cli('--mode', 'vanilla'))
        self.assertIn('Renderer: software', self.cli('--mode', 'modernized'))
        self.cli('--reset-profile', 'vanilla')
        self.assertIn('Renderer: software', self.cli())
        self.cli('--reset-profile', 'modernized')
        self.assertIn('Renderer: opengl', self.cli())
        self.assertIn('Profile: Modernized', self.cli())

    def test_malformed_preserved_and_recovers(self):
        for raw in (b'{bad', b'\xff', b'[]'):
            self.path.write_bytes(raw)
            settings, notices = profiles.load(self.path)
            self.assertEqual(settings, profiles.defaults())
            self.assertTrue(notices)
            self.assertIn(raw, [p.read_bytes() for p in self.path.parent.glob('*.recovered-*')])
            self.assertEqual(json.loads(self.path.read_text()), settings)

    def test_invalid_fields_preserve_valid_other_profile(self):
        settings = profiles.defaults()
        settings['profiles']['vanilla']['presentation']['renderer'] = 'software'
        settings['profiles']['modernized'] = ['invalid']
        settings['active'] = {'invalid': 1}
        self.path.write_text(json.dumps(settings))
        loaded, notices = profiles.load(self.path)
        self.assertEqual(loaded['active'], 'modernized')
        self.assertEqual(loaded['profiles']['vanilla']['presentation']['renderer'], 'software')
        self.assertEqual(loaded['profiles']['modernized'], profiles.default_profile('modernized'))
        self.assertTrue(notices)

    def test_legacy_migration(self):
        raw = b'{"version":0,"mode":"modernized","renderer":"software"}'
        self.path.write_bytes(raw)
        loaded, notices = profiles.load(self.path)
        self.assertEqual(loaded['version'],24)
        self.assertEqual(loaded['active'], 'modernized')
        self.assertEqual(loaded['profiles']['modernized']['presentation']['renderer'], 'software')
        self.assertEqual(profiles.load(self.path), (loaded, []))
        self.assertEqual(next(self.path.parent.glob('*.recovered-*')).read_bytes(), raw)

    def test_future_version_not_overwritten(self):
        raw = b'{"version":25,"new_settings":[1,2]}'
        self.path.write_bytes(raw)
        with self.assertRaisesRegex(ValueError, 'newer'):
            profiles.load(self.path)
        self.assertEqual(self.path.read_bytes(), raw)
        self.assertEqual(len(list(self.path.parent.iterdir())), 1)

    def test_atomic_replace_failure_preserves_original(self):
        original = b'original'
        self.path.write_bytes(original)
        with patch.object(profiles.os, 'replace', side_effect=OSError('simulated disk error')):
            with self.assertRaises(OSError):
                profiles.save(self.path, profiles.defaults())
        self.assertEqual(self.path.read_bytes(), original)
        self.assertEqual(list(self.path.parent.iterdir()), [self.path])

    def test_menu_save_reset_and_cancel(self):
        settings = profiles.defaults()
        answers = iter(['2', '3', '2', '5'])
        edited = profiles.menu(settings, lambda _: next(answers), lambda _: None)
        self.assertEqual(edited['active'], 'modernized')
        self.assertEqual(edited['profiles']['modernized']['presentation']['renderer'], 'software')
        self.assertEqual(settings, profiles.defaults())
        answers = iter(['4', '5'])
        reset = profiles.menu(edited, lambda _: next(answers), lambda _: None)
        self.assertEqual(reset['profiles']['modernized'], profiles.default_profile('modernized'))
        before = copy.deepcopy(edited)
        answers = iter(['1', '4', '0'])
        self.assertIsNone(profiles.menu(edited, lambda _: next(answers), lambda _: None))
        self.assertEqual(edited, before)

    def test_launch_uses_profile_and_explicit_cards(self):
        cards = self.path.parent / 'test cards'
        with patch.object(Path, 'is_file', return_value=True), patch.object(run.subprocess, 'call', return_value=0) as launch, patch.object(run, 'ensure_movie_shard'):
            self.assertEqual(run.main(['--settings-file', str(self.path), '--no-session-log', '--mode', 'modernized',
                                       '--renderer', 'software', '--memcard-dir', str(cards), '--headless']), 0)
        argv = launch.call_args.args[0]
        self.assertEqual(argv[argv.index('--renderer') + 1], 'software')
        self.assertEqual(argv[argv.index('--memcard-dir') + 1], str(cards.resolve()))
        self.assertIn('--headless', argv)
        self.assertEqual(launch.call_args.kwargs['env']['DNTTK_INPUT_MODE'], 'modernized')
        self.assertTrue(launch.call_args.kwargs['env']['DNTTK_INPUT_BINDINGS'].startswith('1:'))


if __name__ == '__main__':
    unittest.main()


class CameraPreferencesTest(unittest.TestCase):
    def test_v2_migration_preserves_bindings_and_renderer(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            data=profiles.defaults();data['version']=2;data['active']='modernized'
            for p in data['profiles'].values(): del p['controls']
            data['profiles']['modernized']['bindings']['jump']='J';data['profiles']['modernized']['bindings'].pop('jetpack')
            data['profiles']['modernized']['presentation']['renderer']='software'
            raw=json.dumps(data);path.write_text(raw)
            result,notes=profiles.load(path)
            self.assertEqual(result['version'],24)
            self.assertEqual(result['profiles']['modernized']['bindings']['jump'],'J')
            self.assertEqual(result['profiles']['modernized']['presentation']['renderer'],'software')
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_text(),raw)
            self.assertEqual(profiles.load(path),(result,[]))

    def test_camera_cli_persists_and_rejects_nan_atomically(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            cmd=[sys.executable,str(TOOLS/'run.py'),'--settings-file',str(path),'--show-settings']
            subprocess.run(cmd+['--mode','modernized','--camera','original','--mouse-sensitivity','0.25','--invert-y','on'],check=True,capture_output=True)
            raw=path.read_bytes()
            result,_=profiles.load(path)
            self.assertEqual(result['profiles']['modernized']['controls'],{'camera':'original','mouse_sensitivity':0.25,'invert_y':True,'weapon_aim':'view','crosshair':True,'red_dot':False,'aim_assist':'off','jetpack':'modern','camera_distance':0,'shoulder':'center','view':'third','widescreen':'16:9','cpu_overclock':150,'jump':'assisted','frame_rate':'60','view_bob':'on','geometry_precision':'original','texture_precision':'original','draw_distance':'extended'})
            invalid=subprocess.run(cmd+['--mouse-sensitivity','nan'],capture_output=True)
            self.assertNotEqual(invalid.returncode,0);self.assertEqual(path.read_bytes(),raw)
            subprocess.run(cmd+['--mode','vanilla'],check=True,capture_output=True)
            result,_=profiles.load(path)
            self.assertEqual(result['profiles']['modernized']['controls']['camera'],'original')

    def test_terminal_camera_edit(self):
        replies=iter(['2','7','independent 0.2 on','5'])
        edited=profiles.menu(profiles.defaults(),lambda _:next(replies),lambda _:None)
        self.assertEqual(edited['profiles']['modernized']['controls'],{'camera':'independent','mouse_sensitivity':0.2,'invert_y':True,'weapon_aim':'view','crosshair':True,'red_dot':False,'aim_assist':'off','jetpack':'modern','camera_distance':0,'shoulder':'center','view':'third','widescreen':'16:9','cpu_overclock':150,'jump':'assisted','frame_rate':'60','view_bob':'on','geometry_precision':'original','texture_precision':'original','draw_distance':'extended'})

    def test_view_bob_menu_and_migration(self):
        replies=iter(['2','b','subtle','b','wobbly','5'])
        edited=profiles.menu(profiles.defaults(),lambda _:next(replies),lambda _:None)
        self.assertEqual(edited['profiles']['modernized']['controls']['view_bob'],'subtle')
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            old=profiles.defaults(); old['version']=21
            del old['profiles']['modernized']['controls']['view_bob']
            path.write_text(json.dumps(old))
            loaded,notices=profiles.load(path)
            self.assertEqual(loaded['version'],24)
            self.assertTrue(any('version 24' in n for n in notices))
            self.assertEqual(loaded['profiles']['modernized']['controls']['view_bob'],'on')

class WeaponPreferencesTest(unittest.TestCase):
    def test_v3_migration_preserves_all_existing_preferences(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            old=profiles.defaults();old['version']=3;old['active']='modernized'
            for p in old['profiles'].values(): del p['controls']['weapon_aim']
            old['profiles']['modernized']['controls'].update(camera='original',mouse_sensitivity=0.37,invert_y=True)
            old['profiles']['modernized']['bindings']['jump']='J';old['profiles']['modernized']['bindings'].pop('jetpack')
            for mode in profiles.MODES:
                for action in ('camera_recenter','camera_shoulder'): old['profiles'][mode]['bindings'].pop(action)  # added in v12
            raw=json.dumps(old).encode();path.write_bytes(raw)
            result,_=profiles.load(path)
            for mode in profiles.MODES:
                migrated=result['profiles'][mode].copy();migrated['controls']=migrated['controls'].copy()
                self.assertEqual(migrated['controls'].pop('weapon_aim'),'view')
                expected=copy.deepcopy(old['profiles'][mode]);expected['bindings']=pc_input.migrate_bindings(expected['bindings'])
                self.assertEqual(migrated,expected)
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)
            self.assertEqual(profiles.load(path),(result,[]))

    def test_aim_menu_and_vanilla_launcher_contract(self):
        replies=iter(['2','8','original','7','independent 0.2 off','5'])
        edited=profiles.menu(profiles.defaults(),lambda _:next(replies),lambda _:None)
        self.assertEqual(edited['profiles']['modernized']['controls']['weapon_aim'],'original')
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';profiles.save(path,edited)
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','vanilla','--weapon-aim','view'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_WEAPON_AIM'],'original')
            self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['weapon_aim'],'view')

class AimingDisplayPreferencesTest(unittest.TestCase):
    def test_v4_migration_retains_preferences_and_backs_up(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            old=profiles.defaults();old['version']=4
            for p in old['profiles'].values():
                for key in ('crosshair','red_dot','aim_assist'): p['controls'].pop(key)
                p['bindings']={k:v for k,v in p['bindings'].items() if k in list(pc_input.DEFAULTS)[:13]}
                p['bindings'].update(original_strafe_left='Q',original_strafe_right='E',walk='R')
                p['controls'].update(camera='original',weapon_aim='original',mouse_sensitivity=.33)
            raw=json.dumps(old).encode();path.write_bytes(raw)
            migrated,notes=profiles.load(path)
            for p in migrated['profiles'].values():
                self.assertEqual(p['controls']['camera'],'original')
                self.assertEqual(p['controls']['weapon_aim'],'original')
                self.assertEqual(p['controls']['mouse_sensitivity'],.33)
                self.assertEqual(p['bindings']['walk'],'R')
                self.assertEqual(p['bindings']['interact'],'E')
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)
            self.assertEqual(profiles.load(path),(migrated,[]))

    def test_independent_cli_and_vanilla_override(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized','--aim-assist','original-lock','--crosshair','off','--red-dot','off'])
                env=launch.call_args.kwargs['env']
                self.assertEqual((env['DNTTK_AIM_ASSIST'],env['DNTTK_CROSSHAIR'],env['DNTTK_RED_DOT']),('original-lock','0','0'))
                run.main(['--settings-file',str(path),'--no-session-log','--mode','vanilla'])
                env=launch.call_args.kwargs['env']
                self.assertEqual((env['DNTTK_AIM_ASSIST'],env['DNTTK_RED_DOT']),('off','1'))
                p=profiles.load(path)[0]['profiles']['modernized']['controls']
                self.assertEqual((p['aim_assist'],p['crosshair'],p['red_dot']),('original-lock',False,False))

class GrabBindingTest(unittest.TestCase):
    # D08T1: schema 17 moves Mouse2 from precision aim to Grab (plus Alt).
    def test_v16_migration_moves_mouse2_to_grab(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            old=profiles.defaults();old['version']=16
            for p in old['profiles'].values():
                p['bindings']={k:v for k,v in p['bindings'].items() if k not in ('grab','grab_alt')}
                p['bindings']['original_aim']='Mouse2'
            old['profiles']['modernized']['bindings']['jump']='J'
            old['profiles']['modernized']['bindings']['jetpack']='Space'
            raw=json.dumps(old).encode();path.write_bytes(raw)
            migrated,notes=profiles.load(path)
            b=migrated['profiles']['modernized']['bindings']
            self.assertEqual((b['grab'],b['grab_alt'],b['original_aim'],b['jump'],b['jetpack']),('Mouse2','Alt','Unbound','J','Space'))
            self.assertEqual(migrated['version'],24)
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)
            self.assertEqual(profiles.load(path),(migrated,[]))

class RedDotDefaultTest(unittest.TestCase):
    # D07D: the original red autoaim dot is off by default in Modernized (schema 16).
    def test_v15_migration_turns_old_default_off_once_and_keeps_later_choice(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            old=profiles.defaults();old['version']=15
            for p in old['profiles'].values(): p['controls']['red_dot']=True
            old['profiles']['modernized']['controls'].update(crosshair=False,aim_assist='original-lock')
            raw=json.dumps(old).encode();path.write_bytes(raw)
            migrated,notes=profiles.load(path)
            controls=migrated['profiles']['modernized']['controls']
            self.assertEqual((controls['red_dot'],controls['crosshair'],controls['aim_assist']),(False,False,'original-lock'))
            self.assertEqual(migrated['version'],24)
            self.assertTrue(any('red autoaim dot' in n for n in notes))
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)
            # An explicit v16 choice survives later loads.
            migrated['profiles']['modernized']['controls']['red_dot']=True
            profiles.save(path,migrated)
            self.assertEqual(profiles.load(path),(migrated,[]))

    def test_v15_file_already_off_has_no_red_dot_notice(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            old=profiles.defaults();old['version']=15
            path.write_text(json.dumps(old))
            migrated,notes=profiles.load(path)
            self.assertFalse(migrated['profiles']['modernized']['controls']['red_dot'])
            self.assertFalse(any('red autoaim dot' in n for n in notes))

    def test_defaults_launch_hides_dot_in_modernized_only(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_RED_DOT'],'0')
                run.main(['--settings-file',str(path),'--no-session-log','--mode','vanilla'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_RED_DOT'],'1')
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized','--red-dot','on'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_RED_DOT'],'1')

class JetpackSchemeTest(unittest.TestCase):
    # D08R: Modernized jetpack scheme, CLI-selected and persisted (schema 11).
    def test_v10_migration_adds_modern_and_retains_preferences(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';old=profiles.defaults();old['version']=10;old['active']='vanilla'
            for p in old['profiles'].values():
                p['controls'].pop('jetpack')
                p['controls'].update(camera='original',mouse_sensitivity=.4,aim_assist='original-lock')
                p['bindings']['jetpack']='K'
            raw=json.dumps(old).encode();path.write_bytes(raw)
            migrated,notes=profiles.load(path)
            self.assertEqual(migrated['version'],24)
            self.assertEqual(migrated['active'],'vanilla')
            for p in migrated['profiles'].values():
                self.assertEqual(p['controls']['jetpack'],'modern')
                self.assertEqual((p['controls']['camera'],p['controls']['mouse_sensitivity'],p['controls']['aim_assist']),('original',.4,'original-lock'))
                self.assertEqual(p['bindings']['jetpack'],'K')
            self.assertTrue(any('version 24' in n for n in notes))
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)
            self.assertEqual(profiles.load(path),(migrated,[]))

    def test_invalid_scheme_restores_control_defaults(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';bad=profiles.defaults()
            bad['profiles']['modernized']['controls']['jetpack']='duke3d'
            path.write_text(json.dumps(bad))
            loaded,notes=profiles.load(path)
            self.assertEqual(loaded['profiles']['modernized']['controls']['jetpack'],'modern')
            self.assertTrue(any('jetpack modern/classic' in n for n in notes))

    def test_cli_switch_persists_and_reaches_runtime_env(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized','--jetpack','classic'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_JETPACK'],'classic')
                self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['jetpack'],'classic')
                run.main(['--settings-file',str(path),'--no-session-log'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_JETPACK'],'classic')
                run.main(['--settings-file',str(path),'--no-session-log','--mode','vanilla'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_JETPACK'],'modern')
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized','--jetpack','modern'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_JETPACK'],'modern')
            self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['jetpack'],'modern')

    def test_show_settings_saves_without_launch(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            out=subprocess.run([sys.executable,str(TOOLS/'run.py'),'--settings-file',str(path),'--mode','modernized','--jetpack','classic','--show-settings'],
                               capture_output=True,text=True,check=True).stdout
            self.assertIn('Jetpack: classic',out)
            self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['jetpack'],'classic')

class WidescreenTest(unittest.TestCase):
    # D14: Modernized widescreen, saved per profile (schema 18); Vanilla stays 4:3.
    def test_v17_migration_adds_16_9_and_retains_preferences(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';old=profiles.defaults();old['version']=17
            for p in old['profiles'].values():
                p['controls'].pop('widescreen')
                p['controls'].update(jetpack='classic',view='first')
            raw=json.dumps(old).encode();path.write_bytes(raw)
            migrated,notes=profiles.load(path)
            self.assertEqual(migrated['version'],24)
            for p in migrated['profiles'].values():
                self.assertEqual(p['controls']['widescreen'],'16:9')
                self.assertEqual((p['controls']['jetpack'],p['controls']['view']),('classic','first'))
            self.assertTrue(any('version 24' in n for n in notes))
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)
            self.assertEqual(profiles.load(path),(migrated,[]))

    def test_invalid_value_restores_control_defaults(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';bad=profiles.defaults()
            bad['profiles']['modernized']['controls']['widescreen']='32:9'
            path.write_text(json.dumps(bad))
            loaded,notes=profiles.load(path)
            self.assertEqual(loaded['profiles']['modernized']['controls']['widescreen'],'16:9')
            self.assertTrue(any('widescreen off/16:9/16:10/21:9/auto' in n for n in notes))

    def test_cli_persists_and_vanilla_never_widens(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_WIDESCREEN'],'16:9')
                for value in ('21:9','auto','16:10','off'):
                    run.main(['--settings-file',str(path),'--no-session-log','--widescreen',value])
                    self.assertEqual(launch.call_args.kwargs['env']['DNTTK_WIDESCREEN'],value)
                    self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['widescreen'],value)
                run.main(['--settings-file',str(path),'--no-session-log','--widescreen','21:9','--mode','vanilla'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_WIDESCREEN'],'off')
            self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['widescreen'],'21:9')

    def test_show_settings_and_terminal_menu(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            out=subprocess.run([sys.executable,str(TOOLS/'run.py'),'--settings-file',str(path),'--mode','modernized','--widescreen','off','--show-settings'],
                               capture_output=True,text=True,check=True).stdout
            self.assertIn('Widescreen: off (original 4:3 picture)',out)
        settings=profiles.defaults()
        answers=iter(['w','21:9','5'])
        edited=profiles.menu(settings,read=lambda _:next(answers),write=lambda *_:None)
        self.assertEqual(edited['profiles']['modernized']['controls']['widescreen'],'21:9')
        settings['active']='vanilla'
        answers=iter(['w','5'])
        edited=profiles.menu(settings,read=lambda _:next(answers),write=lambda *_:None)
        self.assertEqual(edited['profiles']['modernized']['controls']['widescreen'],'16:9')

class CameraPolishTest(unittest.TestCase):
    # D10: saved third-person distance, shoulder side and camera keys (schema 12).
    def test_v11_migration_adds_camera_keys_without_stealing_custom_bindings(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';old=profiles.defaults();old['version']=11
            for p in old['profiles'].values():
                for key in ('camera_distance','shoulder'): p['controls'].pop(key)
                for action in ('camera_recenter','camera_shoulder'): p['bindings'].pop(action)
            old['profiles']['modernized']['bindings']['holster']='H'
            old['profiles']['modernized']['controls']['jetpack']='classic'
            raw=json.dumps(old).encode();path.write_bytes(raw)
            migrated,notes=profiles.load(path)
            self.assertEqual(migrated['version'],24)
            modern=migrated['profiles']['modernized']
            self.assertEqual((modern['controls']['camera_distance'],modern['controls']['shoulder'],modern['controls']['jetpack']),(0,'center','classic'))
            self.assertEqual((modern['bindings']['holster'],modern['bindings']['camera_recenter']),('H','V'))
            self.assertNotIn(modern['bindings']['camera_shoulder'],('H','C','Unbound'))
            self.assertEqual(migrated['profiles']['vanilla']['bindings']['camera_shoulder'],'H')
            self.assertTrue(any('version 24' in n for n in notes))
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)

    def test_runtime_side_file_is_absorbed_once_and_validated(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';state=profiles.camera_state_path(path)
            settings=profiles.defaults()
            self.assertFalse(profiles.absorb_camera_state(path,settings))
            state.write_text('{"camera_distance": 2304, "shoulder": "left"}')
            self.assertTrue(profiles.absorb_camera_state(path,settings))
            self.assertEqual((settings['profiles']['modernized']['controls']['camera_distance'],settings['profiles']['modernized']['controls']['shoulder']),(2304,'left'))
            self.assertFalse(state.exists())
            for bad in ['{"camera_distance": 99999, "shoulder": "left"}','{"camera_distance": 2304, "shoulder": "up"}','not json','{"shoulder":"right"}']:
                state.write_text(bad)
                self.assertFalse(profiles.absorb_camera_state(path,settings))
                self.assertFalse(state.exists())
            self.assertEqual(settings['profiles']['modernized']['controls']['camera_distance'],2304)
            state.write_text('{"camera_distance": 0, "shoulder": "center"}')
            self.assertTrue(profiles.absorb_camera_state(path,settings))
            self.assertEqual(settings['profiles']['modernized']['controls']['camera_distance'],0)

    def test_cli_and_launch_carry_camera_preferences(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';state=profiles.camera_state_path(path)
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized','--camera-distance','3000','--shoulder','right'])
                env=launch.call_args.kwargs['env']
                self.assertEqual((env['DNTTK_CAMERA_DISTANCE'],env['DNTTK_CAMERA_SHOULDER']),('3000.0','right'))
                self.assertEqual(env['DNTTK_CAMERA_STATE_FILE'],str(profiles.camera_state_path(path.resolve())))
                # A session that changed the camera leaves a side file; the launcher folds it in after exit.
                launch.side_effect=lambda *a,**k: (state.write_text('{"camera_distance": 1536, "shoulder": "center"}'),0)[1]
                run.main(['--settings-file',str(path),'--no-session-log'])
                launch.side_effect=None
                controls=profiles.load(path)[0]['profiles']['modernized']['controls']
                self.assertEqual((controls['camera_distance'],controls['shoulder']),(1536,'center'))
                self.assertFalse(state.exists())
                run.main(['--settings-file',str(path),'--no-session-log'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_CAMERA_DISTANCE'],'1536')
                run.main(['--settings-file',str(path),'--no-session-log','--mode','vanilla'])
                env=launch.call_args.kwargs['env']
                self.assertEqual((env['DNTTK_CAMERA_DISTANCE'],env['DNTTK_CAMERA_SHOULDER'],env['DNTTK_CAMERA_STATE_FILE']),('0','center',''))
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized','--camera-distance','original'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_CAMERA_DISTANCE'],'0')
            for bad in ['100','nan','far']:
                with self.assertRaises(SystemExit):
                    run.main(['--settings-file',str(path),'--camera-distance',bad,'--show-settings'])
            self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['camera_distance'],0)


class FirstPersonViewTest(unittest.TestCase):
    # D11: saved first/third-person view and the camera_view key (schema 13).
    def v12(self,path,modern_bindings=None):
        old=profiles.defaults();old['version']=12
        for p in old['profiles'].values():
            p['controls'].pop('view');p['bindings'].pop('camera_view')
        old['profiles']['modernized']['bindings'].update(modern_bindings or {})
        raw=json.dumps(old).encode();path.write_bytes(raw)
        return raw

    def test_v12_migration_adds_view_and_key_without_stealing(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            raw=self.v12(path,{'medkit':'P'})
            migrated,notes=profiles.load(path)
            self.assertEqual(migrated['version'],24)
            modern=migrated['profiles']['modernized']
            self.assertEqual(modern['controls']['view'],'third')
            self.assertEqual(modern['bindings']['medkit'],'P')
            self.assertNotIn(modern['bindings']['camera_view'],('P','C','Unbound'))
            self.assertEqual(migrated['profiles']['vanilla']['bindings']['camera_view'],'P')
            self.assertTrue(any('version 24' in n for n in notes))
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)
            pc_input.wire(modern['bindings'])

    def test_side_file_view_is_optional_and_validated(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';state=profiles.camera_state_path(path)
            settings=profiles.defaults()
            state.write_text('{"camera_distance": 0, "shoulder": "center", "view": "first"}')
            self.assertTrue(profiles.absorb_camera_state(path,settings))
            self.assertEqual(settings['profiles']['modernized']['controls']['view'],'first')
            # A side file from the v12 runtime has no view; the saved view stays.
            state.write_text('{"camera_distance": 2304, "shoulder": "left"}')
            self.assertTrue(profiles.absorb_camera_state(path,settings))
            self.assertEqual(settings['profiles']['modernized']['controls']['view'],'first')
            state.write_text('{"camera_distance": 0, "shoulder": "center", "view": "fourth"}')
            self.assertFalse(profiles.absorb_camera_state(path,settings))
            self.assertFalse(state.exists())
            self.assertEqual(settings['profiles']['modernized']['controls']['view'],'first')

    def test_cli_launch_and_vanilla_view(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized','--view','first'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_CAMERA_VIEW'],'first')
                run.main(['--settings-file',str(path),'--no-session-log','--mode','vanilla'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_CAMERA_VIEW'],'third')
            self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['view'],'first')
            out=subprocess.run([sys.executable,str(TOOLS/'run.py'),'--settings-file',str(path),'--mode','modernized','--show-settings'],
                               capture_output=True,text=True,check=True).stdout
            self.assertIn('View: first person (P toggles',out)
            with self.assertRaises(SystemExit):
                run.main(['--settings-file',str(path),'--view','second','--show-settings'])


class PolishMigrationTest(unittest.TestCase):
    def test_v5_changes_default_h_only_and_preserves_vanilla_choice(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';old=profiles.defaults();old['version']=5;old['active']='vanilla'
            old['profiles']['modernized']['bindings']['holster']='H'
            old['profiles']['vanilla']['bindings']['holster']='G'
            old['profiles']['modernized']['bindings']['jump']='J';old['profiles']['modernized']['bindings'].pop('jetpack')
            for mode in profiles.MODES:
                for action in ('camera_recenter','camera_shoulder'): old['profiles'][mode]['bindings'].pop(action)  # added in v12
            profiles.save(path,old);raw=path.read_bytes();new,_=profiles.load(path)
            self.assertEqual(new['active'],'vanilla')
            self.assertEqual(new['profiles']['modernized']['bindings']['holster'],'ScrollLock')
            self.assertEqual(new['profiles']['vanilla']['bindings']['holster'],'G')
            self.assertEqual(new['profiles']['modernized']['bindings']['jump'],'J')
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)


class PresentationTest(unittest.TestCase):
    # D13: internal resolution, display mode, window width and output filter (schema 14).
    def test_v13_migration_keeps_renderer_and_adds_mode_defaults(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';old=profiles.defaults();old['version']=13;old['active']='vanilla'
            for p in old['profiles'].values(): p['presentation']={'renderer':'software'}
            old['profiles']['modernized']['controls']['view']='first'
            raw=json.dumps(old).encode();path.write_bytes(raw)
            migrated,notes=profiles.load(path)
            self.assertEqual(migrated['version'],24)
            self.assertEqual(migrated['active'],'vanilla')
            self.assertEqual(migrated['profiles']['vanilla']['presentation'],{**profiles.DEFAULT_PRESENTATION,'renderer':'software'})
            self.assertEqual(migrated['profiles']['modernized']['presentation'],{**profiles.DEFAULT_PRESENTATION,'renderer':'software','internal_scale':4})
            self.assertEqual(migrated['profiles']['modernized']['controls']['view'],'first')
            self.assertTrue(any('version 24' in n for n in notes))
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)
            self.assertEqual(profiles.load(path),(migrated,[]))

    def test_invalid_presentation_restores_only_presentation(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            for bad in ({'internal_scale':8},{'internal_scale':2.0},{'display':'kiosk'},{'window_width':320},{'window_width':True},{'output_filter':'bicubic'}):
                data=profiles.defaults();data['profiles']['modernized']['controls']['jetpack']='classic'
                data['profiles']['modernized']['presentation'].update(bad)
                path.write_text(json.dumps(data))
                loaded,notes=profiles.load(path)
                self.assertEqual(loaded['profiles']['modernized']['presentation'],profiles.default_profile('modernized')['presentation'])
                self.assertEqual(loaded['profiles']['modernized']['controls']['jetpack'],'classic')
                self.assertTrue(any('Restored presentation defaults' in n for n in notes))

    def test_cli_saves_per_profile_and_reaches_runtime_argv(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            def flags(argv):
                return {k:argv[argv.index(k)+1] for k in ('--renderer','--internal-scale','--display','--window-width','--output-filter')}
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized'])
                self.assertEqual(flags(launch.call_args.args[0]),{'--renderer':'opengl','--internal-scale':'4','--display':'windowed','--window-width':'0','--output-filter':'linear'})
                run.main(['--settings-file',str(path),'--no-session-log','--internal-scale','4','--display','borderless','--window-width','1920','--output-filter','nearest'])
                self.assertEqual(flags(launch.call_args.args[0]),{'--renderer':'opengl','--internal-scale':'4','--display':'borderless','--window-width':'1920','--output-filter':'nearest'})
                run.main(['--settings-file',str(path),'--no-session-log','--mode','vanilla'])
                self.assertEqual(flags(launch.call_args.args[0])['--internal-scale'],'1')
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized'])
                self.assertEqual(flags(launch.call_args.args[0])['--internal-scale'],'4')
            raw=path.read_bytes()
            for bad in (['--internal-scale','5'],['--window-width','100'],['--display','kiosk']):
                with self.assertRaises(SystemExit):
                    run.main(['--settings-file',str(path),*bad,'--show-settings'])
            self.assertEqual(path.read_bytes(),raw)

    def test_menu_resolution_edit_and_rejection(self):
        replies=iter(['1','r','3 exclusive 1600 nearest','5'])
        edited=profiles.menu(profiles.defaults(),lambda _:next(replies),lambda _:None)
        self.assertEqual(edited['profiles']['vanilla']['presentation'],{'renderer':'opengl','internal_scale':3,'display':'exclusive','fullscreen_mode':'exclusive','window_width':1600,'output_filter':'nearest'})
        self.assertEqual(edited['profiles']['modernized']['presentation']['internal_scale'],4)
        replies=iter(['R','9 windowed 0 linear','R','two windowed 0 linear','5'])
        edited=profiles.menu(profiles.defaults(),lambda _:next(replies),lambda _:None)
        self.assertEqual(edited['profiles']['modernized']['presentation'],profiles.default_profile('modernized')['presentation'])

    def test_software_renderer_runs_at_original_scale_and_keeps_choice(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized','--internal-scale','3','--renderer','software'])
                argv=launch.call_args.args[0]
                self.assertEqual(argv[argv.index('--internal-scale')+1],'1')
                run.main(['--settings-file',str(path),'--no-session-log','--renderer','opengl'])
                argv=launch.call_args.args[0]
                self.assertEqual(argv[argv.index('--internal-scale')+1],'3')
            self.assertIn('saved 3x applies with OpenGL',profiles.describe_presentation({**profiles.DEFAULT_PRESENTATION,'renderer':'software','internal_scale':3}))


class DisplayStateTest(unittest.TestCase):
    # D13 follow-up: windowed by default, last display state remembered (schema 15).
    def test_v14_migration_derives_alt_enter_target(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';old=profiles.defaults();old['version']=14
            for p in old['profiles'].values(): p['presentation'].pop('fullscreen_mode')
            old['profiles']['modernized']['presentation'].update(display='exclusive',internal_scale=4)
            raw=json.dumps(old).encode();path.write_bytes(raw)
            migrated,notes=profiles.load(path)
            self.assertEqual(migrated['version'],24)
            self.assertEqual(migrated['profiles']['modernized']['presentation'],{**profiles.DEFAULT_PRESENTATION,'display':'exclusive','fullscreen_mode':'exclusive','internal_scale':4})
            self.assertEqual(migrated['profiles']['vanilla']['presentation'],profiles.DEFAULT_PRESENTATION)
            self.assertEqual(profiles.DEFAULT_PRESENTATION['display'],'windowed')
            self.assertEqual(next(Path(d).glob('*.recovered-*')).read_bytes(),raw)
            self.assertEqual(profiles.load(path),(migrated,[]))

    def test_exit_state_is_absorbed_into_launched_profile_once(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';state=profiles.presentation_state_path(path)
            settings=profiles.defaults()
            self.assertFalse(profiles.absorb_presentation_state(path,settings,'modernized'))
            state.write_text('{"display": "windowed", "fullscreen_mode": "exclusive", "window_width": 1600}')
            self.assertTrue(profiles.absorb_presentation_state(path,settings,'modernized'))
            p=settings['profiles']['modernized']['presentation']
            self.assertEqual((p['display'],p['fullscreen_mode'],p['window_width'],p['internal_scale']),('windowed','exclusive',1600,4))
            self.assertEqual(settings['profiles']['vanilla']['presentation'],profiles.DEFAULT_PRESENTATION)
            self.assertFalse(state.exists())
            for bad in ['{"display": "kiosk", "fullscreen_mode": "exclusive", "window_width": 0}','{"display": "windowed", "fullscreen_mode": "exclusive", "window_width": 100}','nope','{}']:
                state.write_text(bad)
                self.assertFalse(profiles.absorb_presentation_state(path,settings,'modernized'))
                self.assertFalse(state.exists())
            self.assertEqual(settings['profiles']['modernized']['presentation']['window_width'],1600)

    def test_launch_passes_state_file_and_saves_what_the_session_left(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json';state=profiles.presentation_state_path(path.resolve())
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized','--display','exclusive'])
                argv=launch.call_args.args[0]
                self.assertEqual(argv[argv.index('--presentation-state')+1],str(state))
                self.assertEqual((argv[argv.index('--display')+1],argv[argv.index('--fullscreen-mode')+1]),('exclusive','exclusive'))
                # The player pressed F11 and quit windowed.
                launch.side_effect=lambda *a,**k:(state.write_text('{"display": "windowed", "fullscreen_mode": "exclusive", "window_width": 0}'),0)[1]
                run.main(['--settings-file',str(path),'--no-session-log'])
                launch.side_effect=None
                run.main(['--settings-file',str(path),'--no-session-log'])
                argv=launch.call_args.args[0]
                self.assertEqual((argv[argv.index('--display')+1],argv[argv.index('--fullscreen-mode')+1]),('windowed','exclusive'))
                run.main(['--settings-file',str(path),'--no-session-log','--display','windowed'])
                argv=launch.call_args.args[0]
                self.assertEqual(argv[argv.index('--fullscreen-mode')+1],'exclusive')
            self.assertEqual(profiles.load(path)[0]['profiles']['vanilla']['presentation']['display'],'windowed')


class CpuOverclockTest(unittest.TestCase):
    # D08Y: Modernized emulated CPU speed (schema 19); Vanilla always stock.
    def test_migration_cli_env_and_validation(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            data=profiles.defaults()
            data['version']=18
            for p in data['profiles'].values():
                p['controls'].pop('cpu_overclock')
            path.write_text(json.dumps(data))
            migrated,notes=profiles.load(path)
            self.assertEqual(migrated['version'],24)
            for p in migrated['profiles'].values():
                self.assertEqual(p['controls']['cpu_overclock'],150)
            self.assertTrue(any('version 24' in n for n in notes))
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized'])
                self.assertEqual(launch.call_args.kwargs['env']['PSX_CPU_OVERCLOCK'],'150')
                for value in (100,125,175,200):
                    run.main(['--settings-file',str(path),'--no-session-log','--cpu-overclock',str(value)])
                    self.assertEqual(launch.call_args.kwargs['env']['PSX_CPU_OVERCLOCK'],str(value))
                    self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['cpu_overclock'],value)
                run.main(['--settings-file',str(path),'--no-session-log','--mode','vanilla'])
                self.assertEqual(launch.call_args.kwargs['env']['PSX_CPU_OVERCLOCK'],'100')
            with self.assertRaises(SystemExit),patch('sys.stderr'):
                run.main(['--settings-file',str(path),'--no-session-log','--cpu-overclock','300'])
            bad=profiles.load(path)[0]
            bad['profiles']['modernized']['controls']['cpu_overclock']=130
            path.write_text(json.dumps(bad))
            loaded,notes=profiles.load(path)
            self.assertEqual(loaded['profiles']['modernized']['controls']['cpu_overclock'],150)



class JumpStyleTest(unittest.TestCase):
    # D08Z: Modernized jump style (schema 20); Vanilla always the original jump.
    def test_migration_cli_env_and_validation(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            data=profiles.defaults()
            data['version']=19
            for p in data['profiles'].values():
                p['controls'].pop('jump')
            path.write_text(json.dumps(data))
            migrated,notes=profiles.load(path)
            self.assertEqual(migrated['version'],24)
            for p in migrated['profiles'].values():
                self.assertEqual(p['controls']['jump'],'assisted')
            self.assertTrue(any('version 24' in n for n in notes))
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_JUMP'],'assisted')
                run.main(['--settings-file',str(path),'--no-session-log','--jump','manual'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_JUMP'],'manual')
                self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['jump'],'manual')
                run.main(['--settings-file',str(path),'--no-session-log','--mode','vanilla'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_JUMP'],'assisted')
            with self.assertRaises(SystemExit),patch('sys.stderr'):
                run.main(['--settings-file',str(path),'--no-session-log','--jump','floaty'])
            bad=profiles.load(path)[0]
            bad['profiles']['modernized']['controls']['jump']='floaty'
            path.write_text(json.dumps(bad))
            loaded,notes=profiles.load(path)
            self.assertEqual(loaded['profiles']['modernized']['controls']['jump'],'assisted')

    def test_menu_edits_jump_style(self):
        settings=profiles.defaults()
        answers=iter(['j','manual','5'])
        edited=profiles.menu(settings,read=lambda _: next(answers),write=lambda *_: None)
        self.assertEqual(edited['profiles']['modernized']['controls']['jump'],'manual')
        self.assertIn('Jump: manual',profiles.describe(edited))


class FrameRateTest(unittest.TestCase):
    # D17: Modernized frame rate (schema 21); Vanilla always presents at 60.
    def test_migration_cli_env_and_validation(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            data=profiles.defaults()
            data['version']=20
            for p in data['profiles'].values():
                p['controls'].pop('frame_rate')
            path.write_text(json.dumps(data))
            migrated,notes=profiles.load(path)
            self.assertEqual(migrated['version'],24)
            for p in migrated['profiles'].values():
                self.assertEqual(p['controls']['frame_rate'],'60')
            self.assertTrue(any('version 24' in n for n in notes))
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized'])
                env=launch.call_args.kwargs['env']
                self.assertEqual(env['DNTTK_FRAME_RATE'],'60')
                run.main(['--settings-file',str(path),'--no-session-log','--frame-rate','display'])
                env=launch.call_args.kwargs['env']
                self.assertEqual(env['DNTTK_FRAME_RATE'],'display')
                self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['frame_rate'],'display')
                run.main(['--settings-file',str(path),'--no-session-log','--mode','vanilla'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_FRAME_RATE'],'60')
            with self.assertRaises(SystemExit),patch('sys.stderr'):
                run.main(['--settings-file',str(path),'--no-session-log','--frame-rate','75'])
            bad=profiles.load(path)[0]
            bad['profiles']['modernized']['controls']['frame_rate']='75'
            path.write_text(json.dumps(bad))
            loaded,notes=profiles.load(path)
            self.assertEqual(loaded['profiles']['modernized']['controls']['frame_rate'],'60')

    def test_menu_edits_frame_rate(self):
        settings=profiles.defaults()
        answers=iter(['f','180','5'])
        edited=profiles.menu(settings,read=lambda _: next(answers),write=lambda *_: None)
        self.assertEqual(edited['profiles']['modernized']['controls']['frame_rate'],'180')
        self.assertIn('Frame rate: 180',profiles.describe(edited))


class DrawDistanceTest(unittest.TestCase):
    # D17P: Modernized draw distance (schema 24); Vanilla keeps the original limits.
    def test_migration_cli_env_vanilla_and_validation(self):
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'profiles.json'
            data=profiles.defaults()
            data['version']=23
            for p in data['profiles'].values():
                p['controls'].pop('draw_distance')
            path.write_text(json.dumps(data))
            migrated,notes=profiles.load(path)
            self.assertEqual(migrated['version'],24)
            for p in migrated['profiles'].values():
                self.assertEqual(p['controls']['draw_distance'],'extended')
            self.assertTrue(any('version 24' in n for n in notes))
            with patch.object(Path,'is_file',return_value=True),patch.object(run.subprocess,'call',return_value=0) as launch,patch.object(run,'ensure_movie_shard'):
                run.main(['--settings-file',str(path),'--no-session-log','--mode','modernized'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_DRAW_DISTANCE'],'extended')
                run.main(['--settings-file',str(path),'--no-session-log','--draw-distance','original'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_DRAW_DISTANCE'],'original')
                self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['draw_distance'],'original')
                run.main(['--settings-file',str(path),'--no-session-log','--draw-distance','extended','--mode','vanilla'])
                self.assertEqual(launch.call_args.kwargs['env']['DNTTK_DRAW_DISTANCE'],'original')
                self.assertEqual(profiles.load(path)[0]['profiles']['modernized']['controls']['draw_distance'],'extended')
            with self.assertRaises(SystemExit),patch('sys.stderr'):
                run.main(['--settings-file',str(path),'--no-session-log','--draw-distance','far'])
            bad=profiles.load(path)[0]
            bad['profiles']['modernized']['controls']['draw_distance']='far'
            path.write_text(json.dumps(bad))
            loaded,_=profiles.load(path)
            self.assertEqual(loaded['profiles']['modernized']['controls']['draw_distance'],'extended')

    def test_menu_edits_draw_distance(self):
        answers=iter(['d','original','d','far','5'])
        edited=profiles.menu(profiles.defaults(),read=lambda _: next(answers),write=lambda *_: None)
        self.assertEqual(edited['profiles']['modernized']['controls']['draw_distance'],'original')
        self.assertIn('Draw distance: original',profiles.describe(edited))
        answers=iter(['1','d','original','5'])
        vanilla=profiles.menu(profiles.defaults(),read=lambda _: next(answers),write=lambda *_: None)
        self.assertEqual(vanilla['profiles']['modernized']['controls']['draw_distance'],'extended')

