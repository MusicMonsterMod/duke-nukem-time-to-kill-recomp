#!/usr/bin/env python3
"""Launch Time to Kill with persistent profiles and explicit memory-card paths."""
import argparse
import os
from pathlib import Path
import subprocess
import sys

import player_profiles as profiles
import pc_input

ROOT = Path(__file__).resolve().parents[2]


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--headless', action='store_true')
    p.add_argument('--diagnostics', action='store_true', help='retain dispatch and stopped guest diagnostics')
    p.add_argument('--debug-port', type=int, default=9123)
    p.add_argument('--renderer', choices=profiles.RENDERERS, help='save renderer for the selected profile')
    p.add_argument('--internal-scale', type=int, choices=profiles.INTERNAL_SCALES, help='save internal 3D resolution for the selected profile: 1 original, 2..4 times')
    p.add_argument('--display', choices=profiles.DISPLAYS, help='save how the selected profile opens; a fullscreen choice is also the F11 target')
    p.add_argument('--window-width', type=int, metavar='0|640..7680', help='save window width for the selected profile; 0 fits the display')
    p.add_argument('--output-filter', choices=profiles.OUTPUT_FILTERS, help='save how the finished image is scaled to the window')
    p.add_argument('--mode', choices=profiles.MODES, help='select and remember the launch profile')
    p.add_argument('--settings', action='store_true', help='open terminal settings, save and return without launching')
    p.add_argument('--show-settings', action='store_true', help='print active preferences without launching')
    p.add_argument('--reset-profile', choices=profiles.MODES, help='restore only the named profile defaults')
    p.add_argument('--settings-file', type=Path, default=ROOT / 'config/player-profiles.json')
    p.add_argument('--memcard-dir', type=Path, default=ROOT / 'saves/local-play')
    p.add_argument('--bind', action='append', default=[], metavar='ACTION=INPUT', help='rebind Modernized action; repeat to swap bindings atomically, then return')
    p.add_argument('--show-bindings', action='store_true', help='list selected profile PC bindings and return')
    p.add_argument('--camera', choices=profiles.CAMERAS, help='save Modernized camera selection')
    p.add_argument('--mouse-sensitivity', type=float, help='save Modernized degrees per mouse count, 0.01..2')
    p.add_argument('--invert-y', choices=('on', 'off'), help='save Modernized mouse Y inversion')
    p.add_argument('--weapon-aim', choices=profiles.AIM_MODES, help='save Modernized view aiming (auto-aim off) or original aiming')
    p.add_argument('--crosshair', choices=('on','off'), help='save Modernized crosshair visibility without changing aim behavior')
    p.add_argument('--aim-assist', choices=('off','original-lock'), help='save experimental Modernized view assistance')
    p.add_argument('--red-dot', choices=('on','off'), help='save original target marker visibility independently of assistance')
    p.add_argument('--jetpack', choices=profiles.JETPACK_SCHEMES, help='save Modernized jetpack scheme: modern flight or classic original flight with WASD')
    p.add_argument('--camera-distance', metavar='original|768..6144', help='save Modernized third-person distance (Alt+wheel also saves it)')
    p.add_argument('--view', choices=profiles.VIEWS, help='save Modernized camera view, third or first person (the camera_view key also toggles it)')
    p.add_argument('--shoulder', choices=profiles.SHOULDERS, help='save Modernized camera shoulder side (the camera_shoulder key also cycles it)')
    p.add_argument('--widescreen', choices=profiles.WIDESCREEN_MODES, help='save Modernized widescreen: off (4:3), 16:9, 16:10, 21:9 or auto (follows the window)')
    p.add_argument('--cpu-timing', choices=profiles.CPU_TIMINGS, help='save Modernized CPU timing model: fast (default; gameplay uses far less host CPU at the same emulated speed) or accurate (full cycle model); Vanilla always uses accurate')
    p.add_argument('--cpu-overclock', type=int, choices=profiles.CPU_OVERCLOCKS, help='save Modernized emulated CPU speed in percent (default 150 keeps busy views at 30 fps; 100 = original; Vanilla always uses 100)')
    p.add_argument('--jump', choices=profiles.JUMP_STYLES, help='save Modernized jump style: assisted (original lip launch, fixed arc) or manual (leaves on the press, edge grace, air steering)')
    p.add_argument('--steroids', choices=profiles.STEROIDS_MODES, help='save Modernized steroids: portable (default; a pickup is stored as one dose that R takes, EDuke32 style) or original (the effect starts on pickup); Vanilla always uses original')
    p.add_argument('--view-bob', choices=profiles.VIEW_BOBS, help='save Modernized first-person view bob while walking: off, subtle, on (default) or strong; standing still is steady at every setting')
    p.add_argument('--geometry-precision', choices=profiles.PRECISION_MODES, help='save Modernized mesh geometry: original (default) or corrected subpixel positions; OpenGL, next launch')
    p.add_argument('--texture-precision', choices=profiles.PRECISION_MODES, help='save Modernized mesh textures: original (default) or corrected perspective; OpenGL, next launch')
    p.add_argument('--draw-distance', choices=profiles.DRAW_DISTANCES, help='save Modernized draw distance: extended (default; twice the view limit, no distant black seams) or original; Vanilla always uses original')
    p.add_argument('--frame-rate', choices=profiles.FRAME_RATES, help="save Modernized frame rate: display (the monitor's refresh), 30, 60 (original), 120, 144, 165, 180, 240 or unlimited; game speed is unchanged, Vanilla always uses 60")
    p.add_argument('--no-session-log', action='store_true', help='do not mirror runtime stderr into build-local/logs/session-*.log')
    a = p.parse_args(argv)
    import disc_lab
    disc_lab.ensure_game_dir()
    if not 1024 <= a.debug_port <= 65535:
        p.error('debug port must be in 1024..65535')
    if a.settings and not sys.stdin.isatty():
        p.error('--settings requires a terminal; use --mode, --renderer and --show-settings in scripts')
    try:
        settings, notices = profiles.load(a.settings_file)
        for notice in notices:
            print(notice, file=sys.stderr)
        changed = profiles.absorb_camera_state(a.settings_file, settings)
        changed = profiles.absorb_presentation_state(a.settings_file, settings, settings['active']) or changed
        if a.mode:
            settings['active'] = a.mode
            changed = True
        if a.reset_profile:
            profiles.reset(settings, a.reset_profile)
            changed = True
        if a.renderer or a.internal_scale is not None or a.display or a.window_width is not None or a.output_filter:
            presentation = dict(settings['profiles'][settings['active']]['presentation'])
            if a.renderer: presentation['renderer'] = a.renderer
            if a.internal_scale is not None: presentation['internal_scale'] = a.internal_scale
            if a.display:
                presentation['display'] = a.display
                if a.display in profiles.FULLSCREEN_MODES: presentation['fullscreen_mode'] = a.display
            if a.window_width is not None: presentation['window_width'] = a.window_width
            if a.output_filter: presentation['output_filter'] = a.output_filter
            settings['profiles'][settings['active']]['presentation'] = profiles.validate_presentation(presentation)
            changed = True
        if a.bind:
            if settings['active'] != 'modernized':
                raise ValueError('--bind requires Modernized; Vanilla retains keybinds.ini.')
            profile = settings['profiles']['modernized']
            profile['bindings'] = pc_input.rebind(profile['bindings'], a.bind)
            changed = True
        if a.steroids is not None or a.draw_distance is not None or a.geometry_precision is not None or a.texture_precision is not None or a.view_bob is not None or a.frame_rate is not None or a.jump is not None or a.cpu_overclock is not None or a.cpu_timing is not None or a.widescreen is not None or a.camera_distance is not None or a.shoulder is not None or a.view is not None or a.jetpack is not None or a.aim_assist is not None or a.red_dot is not None or a.crosshair is not None or a.weapon_aim is not None or a.camera is not None or a.mouse_sensitivity is not None or a.invert_y is not None:
            controls = dict(settings['profiles']['modernized']['controls'])
            if a.aim_assist is not None: controls['aim_assist'] = a.aim_assist
            if a.jetpack is not None: controls['jetpack'] = a.jetpack
            if a.widescreen is not None: controls['widescreen'] = a.widescreen
            if a.cpu_overclock is not None: controls['cpu_overclock'] = a.cpu_overclock
            if a.cpu_timing is not None: controls['cpu_timing'] = a.cpu_timing
            if a.jump is not None: controls['jump'] = a.jump
            if a.steroids is not None: controls['steroids'] = a.steroids
            if a.frame_rate is not None: controls['frame_rate'] = a.frame_rate
            if a.geometry_precision is not None: controls['geometry_precision'] = a.geometry_precision
            if a.draw_distance is not None: controls['draw_distance'] = a.draw_distance
            if a.texture_precision is not None: controls['texture_precision'] = a.texture_precision
            if a.view_bob is not None: controls['view_bob'] = a.view_bob
            if a.shoulder is not None: controls['shoulder'] = a.shoulder
            if a.view is not None: controls['view'] = a.view
            if a.camera_distance is not None:
                try:
                    controls['camera_distance'] = 0 if a.camera_distance == 'original' else float(a.camera_distance)
                except ValueError:
                    raise ValueError('--camera-distance takes original or a number 768..6144.')
            if a.red_dot is not None: controls['red_dot'] = a.red_dot == 'on'
            if a.crosshair is not None: controls['crosshair'] = a.crosshair == 'on'
            if a.weapon_aim is not None: controls['weapon_aim'] = a.weapon_aim
            if a.camera is not None: controls['camera'] = a.camera
            if a.mouse_sensitivity is not None: controls['mouse_sensitivity'] = a.mouse_sensitivity
            if a.invert_y is not None: controls['invert_y'] = a.invert_y == 'on'
            settings['profiles']['modernized']['controls'] = profiles.validate_controls(controls)
            changed = True
        if a.settings:
            edited = profiles.menu(settings)
            if edited is None:
                return 0
            settings = edited
            changed = True
        if changed:
            profiles.save(a.settings_file, settings)
    except (OSError, ValueError) as exc:
        p.error(str(exc))
    except (EOFError, KeyboardInterrupt):
        print('\nSettings cancelled; no selection saved.')
        return 0
    print(profiles.describe(settings), flush=True)
    if a.show_bindings:
        if settings['active'] == 'vanilla':
            print('PC action bindings are inactive in Vanilla; it uses runtime keybinds.ini.')
        print(pc_input.describe(settings['profiles'][settings['active']]['bindings']))
    if a.settings or a.show_settings or a.reset_profile or a.bind or a.show_bindings:
        return 0
    exe = ROOT / 'build-local' / ('Duke_Nukem__Time_to_Kill_Recompiled' + ('.exe' if os.name == 'nt' else ''))
    if not exe.is_file():
        p.error('native candidate missing; run tools/local/build.py first')
    prepared = ROOT / 'disc/time-to-kill.cue'
    if not prepared.is_file():
        validator = ROOT / 'build-tools' / ('sector_check.exe' if os.name == 'nt' else 'sector_check')
        if not validator.is_file():
            p.error('prepared disc missing; run tools/local/build.py first')
        try:
            dump = disc_lab.find_valid_dump()
            disc_lab.import_disc(dump, ROOT / 'disc', validator)
        except ValueError as exc:
            print(str(exc), file=sys.stderr)
            return 1
    ensure_movie_shard()
    result = subprocess.run([sys.executable, str(ROOT / 'tools/local/build_math_overlay.py'),
                             '--if-ready', '--quiet'], cwd=ROOT)
    if result.returncode:
        print('Geometry math shard unavailable; using the guarded interpreter fallback.', file=sys.stderr)
    # Developer only: DNTTK_GAME_CONFIG points at a private config copy; normal
    # launches always use game.local.toml.
    game_config = Path(os.environ.get('DNTTK_GAME_CONFIG') or ROOT / 'game.local.toml')
    command = [str(exe), '--game', str(game_config),
               '--disc', str(ROOT / 'disc/time-to-kill.cue'),
               '--memcard-dir', str(a.memcard_dir.resolve()), '--debug-port', str(a.debug_port),
               *profiles.presentation_args(settings['profiles'][settings['active']]['presentation']),
               '--presentation-state', str(profiles.presentation_state_path(a.settings_file.resolve())),
               '--headless' if a.headless else '--no-launcher']
    env = os.environ.copy()
    # Per-batch GL timer queries are diagnostics, not normal presentation work.
    # Preserve an explicit override; profiling sessions can opt back in.
    env.setdefault('PSX_GL_PERF', '1' if env.get('PSX_REPLAY_PROFILE') == '1' else '0')
    env['DNTTK_INPUT_MODE'] = settings['active']
    env['DNTTK_INPUT_BINDINGS'] = pc_input.wire(settings['profiles'][settings['active']]['bindings'])
    controls = settings['profiles'][settings['active']]['controls']
    env['DNTTK_AIM_ASSIST'] = controls['aim_assist'] if settings['active'] == 'modernized' else 'off'
    env['DNTTK_RED_DOT'] = '0' if settings['active'] == 'modernized' and not controls['red_dot'] else '1'
    env['DNTTK_CROSSHAIR'] = '1' if controls['crosshair'] else '0'
    env['DNTTK_WEAPON_AIM'] = controls['weapon_aim'] if settings['active'] == 'modernized' else 'original'
    env['DNTTK_CAMERA_MODE'] = controls['camera'] if settings['active'] == 'modernized' else 'original'
    env['DNTTK_MOUSE_SENSITIVITY'] = str(controls['mouse_sensitivity'])
    env['DNTTK_MOUSE_INVERT_Y'] = '1' if controls['invert_y'] else '0'
    env['DNTTK_JETPACK'] = controls['jetpack'] if settings['active'] == 'modernized' else 'modern'
    modernized = settings['active'] == 'modernized'
    # D15 owns the player options; legacy screen-position lookup stays off.
    env['PSX_GEOMETRY_CORRECTION'] = '0'
    env['PSX_PERSPECTIVE_TEXTURING'] = '0'
    env['PSX_PGXP_CPU_MODE'] = '0'
    precision_supported = modernized and settings['profiles'][settings['active']]['presentation']['renderer'] == 'opengl'
    env['DNTTK_GEOMETRY_PRECISION'] = controls['geometry_precision'] if precision_supported else 'original'
    env['DNTTK_TEXTURE_PRECISION'] = controls['texture_precision'] if precision_supported else 'original'
    if modernized and not precision_supported and any(controls[k] == 'corrected' for k in ('geometry_precision','texture_precision')):
        print('Mesh precision requires OpenGL; this software-renderer session uses Original.', file=sys.stderr)
    # D17P: render-only view limits; Vanilla keeps the original.
    env['DNTTK_DRAW_DISTANCE'] = controls['draw_distance'] if modernized else 'original'
    env['DNTTK_CAMERA_DISTANCE'] = str(controls['camera_distance'] if modernized else 0)
    env['DNTTK_CAMERA_SHOULDER'] = controls['shoulder'] if modernized else 'center'
    env['DNTTK_CAMERA_VIEW'] = controls['view'] if modernized else 'third'
    env['DNTTK_WIDESCREEN'] = controls['widescreen'] if modernized else 'off'
    env['PSX_CPU_OVERCLOCK'] = str(controls['cpu_overclock'] if modernized else 100)
    env['DNTTK_CPU_TIMING'] = controls['cpu_timing'] if modernized else 'accurate'
    env['DNTTK_JUMP'] = controls['jump'] if modernized else 'assisted'
    env['DNTTK_STEROIDS'] = controls['steroids'] if modernized else 'original'
    # D17: presentation rate. Above/below 60 the runtime presents on its own
    # display deadlines; the guest keeps its original VBlank timing.
    env['DNTTK_FRAME_RATE'] = controls['frame_rate'] if modernized else '60'
    # D17B: ordered single-pass transparency batching, validated against the
    # isolated path pixel-for-pixel. Keep Vanilla/native-rate defaults intact.
    env.setdefault('PSX_GL_ORDERED_SEMI_BATCH', '1' if modernized and controls['frame_rate'] not in ('30', '60') else '0')
    env.setdefault('DNTTK_VIEW_BOB', controls['view_bob'] if modernized else 'off')
    env['DNTTK_CAMERA_STATE_FILE'] = str(profiles.camera_state_path(a.settings_file.resolve())) if modernized else ''
    if a.diagnostics:
        env['PSX_FNTRACE_ALL'] = '1'
        env['PSX_EXIT_HALT'] = '1'
    else:
        # Player sessions: keep the runtime's always-on forensic rings (per-store
        # traces, per-frame full-VRAM readback) quiet; they cost emulation time.
        env.setdefault('PSX_FORENSICS', '0')
    if a.no_session_log:
        status = subprocess.call(command, cwd=ROOT, env=env)
    else:
        status = run_with_session_log(command, env)
    save_camera_state(a.settings_file)
    save_presentation_state(a.settings_file, settings['active'])
    return status


def save_presentation_state(settings_file, mode):
    """Remember the window as the session left it (windowed/fullscreen, width)."""
    try:
        settings, _ = profiles.load(settings_file)
        if profiles.absorb_presentation_state(settings_file, settings, mode):
            profiles.save(settings_file, settings)
            presentation = settings['profiles'][mode]['presentation']
            print(f"Saved display {presentation['display']} (F11: {presentation['fullscreen_mode']}), "
                  f"window width {presentation['window_width'] or 'fit the display'}.", flush=True)
    except (OSError, ValueError) as exc:
        print(f'Display preferences not saved: {exc}', file=sys.stderr, flush=True)


def save_camera_state(settings_file):
    """Fold the session's Alt-wheel distance, shoulder side and view into the profile."""
    try:
        settings, _ = profiles.load(settings_file)
        if profiles.absorb_camera_state(settings_file, settings):
            profiles.save(settings_file, settings)
            controls = settings['profiles']['modernized']['controls']
            print(f"Saved camera distance {controls['camera_distance'] or 'original'}, shoulder {controls['shoulder']}, view {controls['view']} person.", flush=True)
    except (OSError, ValueError) as exc:
        print(f'Camera preferences not saved: {exc}', file=sys.stderr, flush=True)


def ensure_movie_shard():
    """Rebuild the native intro-FMV shard if game.local.toml moved its cache
    namespace since the last build (~0.2 s when current). A failure only warns:
    the game still runs, with the intro decoded by the interpreter."""
    result = subprocess.run([sys.executable, str(ROOT / 'tools/local/build_movie_overlay.py'),
                             '--if-ready', '--quiet'], cwd=ROOT)
    if result.returncode != 0:
        print('WARNING: native movie shard unavailable; the intro FMV may stutter. '
              'Rebuild with: python3 tools/local/build_movie_overlay.py', file=sys.stderr, flush=True)


def run_with_session_log(command, env, keep=5):
    """Mirror the runtime's stderr ([TTK input]/[TTK lease] diagnostics) into
    build-local/logs so a playtest report can be checked afterwards. stdout stays
    on the terminal untouched; nothing under saves/ is written."""
    import datetime
    log_dir = ROOT / 'build-local' / 'logs'
    log_dir.mkdir(parents=True, exist_ok=True)
    for stale in sorted(log_dir.glob('session-*.log'))[:-(keep - 1) or None]:
        try:
            stale.unlink()
        except OSError:
            pass
    path = log_dir / datetime.datetime.now().strftime('session-%Y%m%d-%H%M%S.log')
    print(f'session log: {path}', flush=True)
    proc = subprocess.Popen(command, cwd=ROOT, env=env, stderr=subprocess.PIPE)
    try:
        with path.open('wb') as log:
            for line in proc.stderr:
                sys.stderr.buffer.write(line)
                sys.stderr.flush()
                log.write(line)
                log.flush()
    except KeyboardInterrupt:
        pass
    finally:
        try:
            return proc.wait()
        except KeyboardInterrupt:
            proc.terminate()
            return proc.wait()


if __name__ == '__main__':
    raise SystemExit(main())
