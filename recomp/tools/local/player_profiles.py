"""Versioned launcher preferences. No guest state or memory-card data lives here."""
import copy
import json
import math
import os
from pathlib import Path
import tempfile
import uuid
import pc_input

MODES = ('vanilla', 'modernized')
RENDERERS = ('opengl', 'software')
# D13 presentation. Internal scale renders the 3D scene at N times the original
# resolution (runtime supersampling, 1..4); display, window width and output
# filter only change how the finished image reaches the screen.
INTERNAL_SCALES = (1, 2, 3, 4)
DISPLAYS = ('windowed', 'borderless', 'exclusive')
FULLSCREEN_MODES = ('borderless', 'exclusive')
OUTPUT_FILTERS = ('linear', 'nearest')
PRECISION_MODES = ('original', 'corrected')
WINDOW_WIDTH_RANGE = (640, 7680)
AIM_MODES = ('view', 'original')
CAMERAS = ('independent', 'original')
JETPACK_SCHEMES = ('modern', 'classic')
SHOULDERS = ('center', 'right', 'left')
VIEWS = ('third', 'first')
# D14 widescreen (Modernized only; Vanilla stays 4:3). auto follows the window
# between 4:3 and 21:9, so borderless fullscreen matches the monitor.
WIDESCREEN_MODES = ('off', '16:9', '16:10', '21:9', 'auto')
# D08Y: emulated CPU speed in percent (Modernized only; Vanilla stays stock).
# The Modernized camera and widescreen show more of a level than the original
# camera, so busy views ran out of the PlayStation CPU budget and dropped to
# 20 fps (felt as freeze frames). 150 holds them at 30 without changing game
# speed; above that the game starts finishing frames in a single field.
CPU_OVERCLOCKS = (100, 125, 150, 175, 200)
# D23F emulated CPU timing model (Modernized only; Vanilla stays accurate).
# accurate is the framework's cycle model (pipeline, load delays, instruction
# cache) for every instruction; fast charges each recompiled game block once
# from its instruction count (calibrated to the same emulated speed) and
# checks interrupts only when they can matter, using far less host CPU. Menus,
# movies and loading stay on the accurate model either way. fast is the
# Modernized default since the user accepted it (2026-10-05, schema 26).
CPU_TIMINGS = ('accurate', 'fast')
# D08Z jump style (Modernized only). assisted keeps the original lip launch
# (a run jump pressed near a gap leaves from the edge) and the fixed arc;
# manual leaves on the press, with a short grace after running off an edge,
# bounded air steering and a quicker standing take-off.
JUMP_STYLES = ('assisted', 'manual')
# D08Z1 jump wall contact (Modernized only; Vanilla keeps the original). slide
# (EDuke32 style) only takes away the motion into a wall or ceiling, so Duke
# keeps his arc and slides along it; original bounces him off (107).
JUMP_WALLS = ('slide', 'original')
# D08Z3 landing (Modernized only; Vanilla keeps the original). modern lets a
# held direction or a jump press end the recovery pose 105 at once; original
# holds Duke still for about 0.6 s after a straight-up or released landing.
LANDINGS = ('modern', 'original')
# D08A12 dynamite (Modernized only; Vanilla keeps the original). modern: a
# drawn stick is unlit until fire is pressed, so it can be put away again;
# original: drawing it lights the fuse and it must be thrown or explode.
DYNAMITE_MODES = ('modern', 'original')
# D08AB swimming (Modernized only; Vanilla keeps the original). modern: one
# walk/run pace on land and in every water state, Shift decides (D08AA);
# original: the original water speeds (forced run in shallow water, fixed
# surface strokes and underwater thrust) with modern controls.
SWIMMING_MODES = ('modern', 'original')
# D08Z5 wall backflip (Modernized only; Vanilla keeps the original). modern:
# sliding off a face too steep to stand on (a car body, a steep slope) Duke
# drops where it ends; original: he is launched down the slope, backwards
# after the backflip-like slip pose.
WALL_BACKFLIPS = ('modern', 'original')
# D08K4 crouch (Modernized only; Vanilla keeps the original). modern: held
# crouch plus a direction walks low (camera-relative, Duke facing the view);
# original: crouch plus a direction rolls.
CROUCH_MODES = ('modern', 'original')
# D17 presentation rate (Modernized only; Vanilla presents at 60 as always).
# Game logic keeps its original timing at every value: display follows the
# monitor's refresh rate, 30 shows each new game image once, 60 is the
# original presentation, higher values and unlimited present more often.
FRAME_RATES = ('auto', 'display', '30', '60', '120', '144', '165', '180', '240', 'unlimited')
# Modernized first-person view bob (D17C): deliberate eye motion while walking.
VIEW_BOBS = ('off', 'subtle', 'on', 'strong')
# D17P draw distance (Modernized only; Vanilla keeps the original limits).
# extended doubles the far limits, closes the thin black seams the original
# leaves between distant sections and keeps thin distant surfaces with exact
# culling when geometry or texture precision is Corrected.
DRAW_DISTANCES = ('original', 'extended')
# D08A4 steroids (Modernized only; Vanilla keeps the original). portable stores
# a steroids pickup as one dose (EDuke32 style) that R takes; original starts
# the effect on pickup.
STEROIDS_MODES = ('portable', 'original')
# Alt-wheel boom range in game units; 0 keeps the original follow distance.
CAMERA_DISTANCE_RANGE = (768, 6144)
DEFAULT_CONTROLS = {'scheme': 'modern', 'camera': 'independent', 'mouse_sensitivity': 0.12, 'invert_y': False, 'weapon_aim': 'view', 'crosshair': True, 'red_dot': False, 'aim_assist': 'off', 'jetpack': 'modern', 'camera_distance': 0, 'shoulder': 'center', 'view': 'third', 'widescreen': '16:9', 'cpu_overclock': 150, 'cpu_timing': 'fast', 'jump': 'assisted', 'frame_rate': 'auto', 'view_bob': 'on', 'geometry_precision': 'original', 'texture_precision': 'original', 'draw_distance': 'extended', 'steroids': 'portable', 'jump_walls': 'slide', 'landing': 'modern', 'dynamite': 'modern', 'swimming': 'modern', 'wall_backflip': 'modern', 'crouch': 'modern'}
# display is how the game opens (the runtime reports the last state at exit);
# fullscreen_mode is what F11 enters from a window.
DEFAULT_PRESENTATION = {'renderer': 'opengl', 'internal_scale': 1, 'display': 'windowed', 'fullscreen_mode': 'borderless', 'window_width': 0, 'output_filter': 'linear'}
DEFAULT_PROFILE = {'presentation': DEFAULT_PRESENTATION, 'bindings': pc_input.DEFAULTS, 'controls': DEFAULT_CONTROLS}
# Modernized renders at four times the original resolution (user choice after
# the D13 playtest: 60 fps on a GTX 1080 Ti); Vanilla keeps the original.
MODE_PRESENTATION = {'vanilla': {}, 'modernized': {'internal_scale': 4}}
PREVIEW = ('Modernized first-map preview: captured WASD is camera-relative; mouse orbits the '
           'third-person camera. Unavailable hooks never switch the selected control scheme. '
           'View aiming defaults to unassisted shots for supported weapons; others retain original aiming. Gameplay captures automatically on first entry; Escape pauses and frees the mouse; resume captures automatically. OPTIONS > CONTROLS selects Modern or Legacy; F10 has no action.')


def validate_controls(value):
    if not isinstance(value, dict) or set(value) != set(DEFAULT_CONTROLS):
        raise ValueError('Invalid camera preferences.')
    sensitivity = value['mouse_sensitivity']
    if (value['weapon_aim'] not in AIM_MODES or value['scheme'] not in ('modern', 'legacy') or value['camera'] not in CAMERAS or value['aim_assist'] not in ('off','original-lock') or value['jetpack'] not in JETPACK_SCHEMES or type(value['red_dot']) is not bool or type(value['crosshair']) is not bool or type(value['invert_y']) is not bool or
        type(sensitivity) not in (int, float) or not math.isfinite(sensitivity) or
        not 0.01 <= sensitivity <= 2 or not valid_camera_distance(value['camera_distance']) or value['shoulder'] not in SHOULDERS or value['view'] not in VIEWS or
        value['widescreen'] not in WIDESCREEN_MODES or type(value['cpu_overclock']) is not int or
        value['cpu_overclock'] not in CPU_OVERCLOCKS or value['cpu_timing'] not in CPU_TIMINGS or value['jump'] not in JUMP_STYLES or
        value['geometry_precision'] not in PRECISION_MODES or value['texture_precision'] not in PRECISION_MODES or
        value['frame_rate'] not in FRAME_RATES or value['view_bob'] not in VIEW_BOBS or
        value['draw_distance'] not in DRAW_DISTANCES or value['steroids'] not in STEROIDS_MODES or value['jump_walls'] not in JUMP_WALLS or value['landing'] not in LANDINGS or value['dynamite'] not in DYNAMITE_MODES or value['swimming'] not in SWIMMING_MODES or value['wall_backflip'] not in WALL_BACKFLIPS or value['crouch'] not in CROUCH_MODES):
        raise ValueError('Camera must be independent/original; sensitivity 0.01..2 degrees/count; invert_y boolean; jetpack modern/classic; '
                         'camera_distance 0 (original) or 768..6144; shoulder center/right/left; view third/first; '
                         'widescreen off/16:9/16:10/21:9/auto; cpu_overclock 100/125/150/175/200; cpu_timing accurate/fast; jump assisted/manual; '
                         'frame_rate ' + '/'.join(FRAME_RATES) + '; view_bob ' + '/'.join(VIEW_BOBS) + '; geometry_precision and texture_precision original/corrected; draw_distance original/extended; steroids portable/original; jump_walls slide/original; landing modern/original; dynamite modern/original; swimming modern/original; wall_backflip modern/original; crouch modern/original.')
    return dict(value)



def validate_presentation(value):
    if not isinstance(value, dict) or set(value) != set(DEFAULT_PRESENTATION):
        raise ValueError('Invalid presentation preferences.')
    width = value['window_width']
    if (value['renderer'] not in RENDERERS or type(value['internal_scale']) is not int or
            value['internal_scale'] not in INTERNAL_SCALES or value['display'] not in DISPLAYS or
            value['fullscreen_mode'] not in FULLSCREEN_MODES or
            value['output_filter'] not in OUTPUT_FILTERS or type(width) is not int or
            not (width == 0 or WINDOW_WIDTH_RANGE[0] <= width <= WINDOW_WIDTH_RANGE[1])):
        raise ValueError('Renderer opengl/software; internal scale 1..4; display windowed/borderless/exclusive; '
                         'fullscreen mode borderless/exclusive; '
                         'window width 0 (fit the display) or 640..7680; output filter linear/nearest.')
    return dict(value)


def default_profile(mode):
    profile = copy.deepcopy(DEFAULT_PROFILE)
    profile['presentation'].update(MODE_PRESENTATION[mode])
    if mode == 'vanilla':
        profile['controls']['frame_rate'] = '60'
    return profile


def effective_scale(presentation):
    """Software supersampling is CPU-bound (measured 29 fps at 2x against 60 at
    1x), so the software renderer always runs at the original resolution. The
    saved scale is kept for OpenGL."""
    return 1 if presentation['renderer'] == 'software' else presentation['internal_scale']


def presentation_args(presentation):
    """Runtime flags for the saved presentation (patched runtime CLI overrides)."""
    return ['--renderer', presentation['renderer'],
            '--internal-scale', str(effective_scale(presentation)),
            '--display', presentation['display'],
            '--fullscreen-mode', presentation['fullscreen_mode'],
            '--window-width', str(presentation['window_width']),
            '--output-filter', presentation['output_filter']]


def describe_presentation(presentation):
    scale = effective_scale(presentation)
    width = presentation['window_width']
    held = (f"; saved {presentation['internal_scale']}x applies with OpenGL"
            if scale != presentation['internal_scale'] else '')
    return (f"Internal resolution: {scale}x ({'original, ' if scale == 1 else ''}512x240 gameplay renders at "
            f"{512 * scale}x{240 * scale}{held}); display: {presentation['display']} "
            f"(F11: {presentation['fullscreen_mode']}); "
            f"window width: {width or 'fit the display'}; output filter: {presentation['output_filter']}")


def presentation_state_path(settings_path):
    """Side file the runtime writes at exit with the window state it was left in."""
    settings_path = Path(settings_path)
    return settings_path.with_name(settings_path.name + '.presentation-state')


def absorb_presentation_state(settings_path, settings, mode):
    """Fold the last session's display, fullscreen kind and window width into
    that profile. Returns True when it changed; the side file is consumed and an
    invalid one is discarded."""
    state = presentation_state_path(settings_path)
    try:
        raw = state.read_bytes()
    except FileNotFoundError:
        return False
    try:
        data = json.loads(raw)
        presentation = settings['profiles'][mode]['presentation']
        width = data['window_width']
        updated = validate_presentation({**presentation, 'display': data['display'],
                                         'fullscreen_mode': data['fullscreen_mode'],
                                         'window_width': min(width, WINDOW_WIDTH_RANGE[1]) if type(width) is int and width else 0})
    except (ValueError, TypeError, KeyError, UnicodeError):
        updated = None
    state.unlink(missing_ok=True)
    if updated is None or updated == settings['profiles'][mode]['presentation']:
        return False
    settings['profiles'][mode]['presentation'] = updated
    return True


def valid_camera_distance(value):
    return (type(value) in (int, float) and math.isfinite(value) and
            (value == 0 or CAMERA_DISTANCE_RANGE[0] <= value <= CAMERA_DISTANCE_RANGE[1]))


def camera_state_path(settings_path):
    """Side file the runtime writes when Alt-wheel distance, shoulder side or view changes."""
    settings_path = Path(settings_path)
    return settings_path.with_name(settings_path.name + '.camera-state')


def absorb_camera_state(settings_path, settings):
    """Merge the runtime's last camera distance/shoulder/view into Modernized controls.

    Returns True when the profile changed. The side file is consumed; an invalid
    one is discarded with the profile left unchanged."""
    state = camera_state_path(settings_path)
    try:
        raw = state.read_bytes()
    except FileNotFoundError:
        return False
    try:
        data = json.loads(raw)
        controls = dict(settings['profiles']['modernized']['controls'])
        distance = data['camera_distance']
        controls['camera_distance'] = round(distance) if distance else 0
        controls['shoulder'] = data['shoulder']
        # D11 view is optional so a side file from a v12 runtime is still accepted.
        if 'view' in data: controls['view'] = data['view']
        controls = validate_controls(controls)
    except (ValueError, TypeError, KeyError, UnicodeError):
        controls = None
    state.unlink(missing_ok=True)
    if controls is None or controls == settings['profiles']['modernized']['controls']:
        return False
    settings['profiles']['modernized']['controls'] = controls
    return True


def menu_settings_path(settings_path):
    """Side file the runtime writes when a setting is confirmed in the OPTIONS screens (D19)."""
    settings_path = Path(settings_path)
    return settings_path.with_name(settings_path.name + '.menu-settings')


def menu_presentation(presentation):
    """The Modernized display values the OPTIONS screens start from (DNTTK_MENU_PRESENTATION)."""
    return ','.join(f'{key}={presentation[key]}' for key in ('renderer', 'internal_scale', 'display', 'output_filter'))


def absorb_menu_settings(settings_path, settings):
    """Merge the settings last confirmed in the in-game OPTIONS screens (D19).

    The side file holds the menu's whole state: Modernized controls, the
    display values it offers and (D19G) the PC bindings set in KEY BINDINGS (its
    "active" field is always Modernized, D33). Returns True when the settings
    changed. The side file is consumed; an invalid one is discarded with the
    profile left unchanged."""
    state = menu_settings_path(settings_path)
    try:
        raw = state.read_bytes()
    except FileNotFoundError:
        return False
    try:
        data = json.loads(raw)
        if data.get('version') != 1 or data['active'] not in MODES:
            raise ValueError('menu settings version or profile')
        profile = settings['profiles']['modernized']
        controls = validate_controls({**profile['controls'], **data['controls']})
        presentation = validate_presentation({**profile['presentation'], **data['presentation']})
        # Files from before D19G carry no bindings: the profile's stay.
        bindings = pc_input.validate(dict(data['bindings'])) if 'bindings' in data else profile['bindings']
    except (ValueError, TypeError, KeyError, AttributeError, UnicodeError):
        controls = None
    state.unlink(missing_ok=True)
    if controls is None:
        return False
    profile = settings['profiles']['modernized']
    if controls == profile['controls'] and presentation == profile['presentation'] and bindings == profile['bindings']:
        return False
    profile['controls'], profile['presentation'], profile['bindings'] = controls, presentation, dict(bindings)
    return True


VANILLA_NOTICE = ('Modernized is now the default profile; your Vanilla settings are kept. '
                  'Launch with run.py --vanilla to play the original game.')


def defaults():
    return {'version': 40, 'active': 'modernized',
            'profiles': {mode: default_profile(mode) for mode in MODES}}


def save(path, settings):
    """Replace atomically in the same directory; do not leave a partial JSON file."""
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    fd, temporary = tempfile.mkstemp(prefix=path.name + '.', suffix='.tmp', dir=path.parent)
    try:
        with os.fdopen(fd, 'w') as stream:
            # Every launch starts Modernized; Vanilla is the one-launch
            # run.py --vanilla flag and is never remembered (D33).
            json.dump({**settings, 'active': 'modernized'}, stream, indent=2)
            stream.write('\n')
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def load(path):
    """Return settings and notices. Preserve invalid/legacy bytes before recovery.

    Version 0 is the documented single-profile interchange format
    {version:0, mode:..., renderer:...}; no historical runtime TOML is imported.
    Future versions fail closed so an older launcher cannot erase newer options.
    """
    path = Path(path)
    if not path.exists():
        return defaults(), []
    raw = path.read_bytes()
    notices = []
    try:
        data = json.loads(raw)
    except (ValueError, UnicodeError):
        data = None
    result = defaults()
    changed = False
    if not isinstance(data, dict):
        changed = True
        notices.append('Unreadable profile settings; restored Modernized defaults.')
    else:
        version = data.get('version')
        if type(version) is int and version > 40:
            raise ValueError(f'Profile settings version {version} is newer than this launcher; file left unchanged.')
        if type(version) is int and version == 0:
            mode = data.get('mode', 'modernized')
            renderer = data.get('renderer', 'opengl')
            if mode in MODES:
                if renderer in RENDERERS:
                    result['profiles'][mode]['presentation']['renderer'] = renderer
            changed = True
            notices.append('Migrated version 0 profile settings to version 40.')
        elif type(version) is int and version in (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40):
            if data.get('active') == 'vanilla':
                notices.append(VANILLA_NOTICE)
            profiles = data.get('profiles', {})
            for mode in MODES:
                profile = profiles.get(mode, {}) if isinstance(profiles, dict) else {}
                presentation = profile.get('presentation', {}) if isinstance(profile, dict) else {}
                renderer = presentation.get('renderer') if isinstance(presentation, dict) else None
                if renderer in RENDERERS:
                    result['profiles'][mode]['presentation']['renderer'] = renderer
                # v14 adds resolution/display keys; earlier files keep only the renderer.
                if version >= 14 and isinstance(presentation, dict):
                    if version == 14:
                        # v15 splits the F11 target from how the game opens.
                        kind = presentation.get('display')
                        presentation = {**presentation, 'fullscreen_mode': kind if kind in FULLSCREEN_MODES else 'borderless'}
                    try:
                        result['profiles'][mode]['presentation'] = validate_presentation(presentation)
                    except ValueError as exc:
                        notices.append(f'{mode}: {exc} Restored presentation defaults.')
                if isinstance(profile, dict) and isinstance(profile.get('controls'), dict):
                    try:
                        result['profiles'][mode]['controls'] = validate_controls({**({k:v for k,v in DEFAULT_CONTROLS.items() if k in ('crosshair','red_dot','aim_assist')} if version < 5 else {}), **({'weapon_aim':'view'} if version < 4 else {}), **({'jetpack':'modern'} if version < 11 else {}), **({'camera_distance':0,'shoulder':'center'} if version < 12 else {}), **({'view':'third'} if version < 13 else {}), **({'widescreen':'16:9'} if version < 18 else {}), **({'cpu_overclock':150} if version < 19 else {}), **({'jump':'assisted'} if version < 20 else {}), **({'frame_rate':'60'} if version < 21 else {}), **({'view_bob':'on'} if version < 22 else {}), **({'geometry_precision':'original','texture_precision':'original'} if version < 23 else {}), **({'draw_distance':'extended'} if version < 24 else {}), **({'cpu_timing':'fast'} if version < 25 else {}), **({'steroids':'portable'} if version < 29 else {}), **({'jump_walls':'slide'} if version < 30 else {}), **({'landing':'modern'} if version < 31 else {}), **({'dynamite':'modern'} if version < 32 else {}), **({'swimming':'modern'} if version < 33 else {}), **({'wall_backflip':'modern'} if version < 37 else {}), **({'crouch':'modern'} if version < 39 else {}), **({'scheme':'modern'} if version < 40 else {}), **profile['controls']})
                    except ValueError as exc:
                        notices.append(f'{mode}: {exc} Restored camera defaults.')
                # Schema 28 (D08A5): Comma/Period move from original strafe to the mission
                # inventory (add_missing_actions); schema 27's Backslash action is dropped.
                # Schema 34 (D10C): the look keys take PageUp/PageDown/Home/End when free.
                # Schema 35 (D10D): the turn keys take Left/Right when free.
                # Schema 36 (D19N): USE ITEM is Enter only; the old default U is unbound.
                # Schema 38 (D20): QUICK SAVE / QUICK LOAD take F6 / F9.
                if isinstance(profile, dict) and 'bindings' in profile:
                    try:
                        result['profiles'][mode]['bindings'] = dict(pc_input.migrate_bindings(profile['bindings']) if version < 8 else pc_input.migrate_grab(profile['bindings']) if version < 38 else pc_input.validate(profile['bindings']))
                    except ValueError as exc:
                        notices.append(f'{mode}: {exc} Restored action defaults.')
            if version < 6:
                for profile in result['profiles'].values():
                    if profile['bindings']['holster'] == 'H':
                        profile['bindings']['holster'] = 'C'
            if version < 8:
                for profile in result['profiles'].values():
                    if profile['bindings']['holster'] == 'C' and 'ScrollLock' not in profile['bindings'].values():
                        profile['bindings']['holster'] = 'ScrollLock'
            # v9 briefly unbound Bio Mask from B; v10 restores B when free.
            # Underwater air remains automatic (separate from Bio Mask).
            if version < 10:
                for profile in result['profiles'].values():
                    if profile['bindings'].get('biomask') == 'Unbound' and 'B' not in profile['bindings'].values():
                        profile['bindings']['biomask'] = 'B'
            # D07D: v16 hides the original red autoaim dot by default in
            # Modernized. Earlier files cannot distinguish a chosen 'on' from
            # the old default, so it is switched off once; from v16 on, the
            # saved value is the player's own choice and is kept.
            if version < 16 and result['profiles']['modernized']['controls']['red_dot']:
                result['profiles']['modernized']['controls']['red_dot'] = False
                notices.append('Modernized now hides the original red autoaim dot by default; '
                               'run.py --red-dot on restores it.')
            # D23F: schema 25 saved accurate as the default for a day; fast is
            # the accepted default from 26, so switch Modernized once. From 26
            # the saved value is the player's own choice and is kept.
            if version < 26 and result['profiles']['modernized']['controls']['cpu_timing'] == 'accurate':
                result['profiles']['modernized']['controls']['cpu_timing'] = 'fast'
                notices.append('Modernized now uses fast CPU timing by default; '
                               'run.py --cpu-timing accurate restores the full cycle model.')
            # D19N (user, 2026-10-20): Enter alone uses the item; U is no longer
            # its default. Only the old default is cleared, a U bound elsewhere stays.
            if version < 36:
                for profile in result['profiles'].values():
                    if profile['bindings'].get('item_use') == 'U':
                        profile['bindings']['item_use'] = 'Unbound'
            changed = result != data
            if changed and result != {**data, 'active': 'modernized'}:
                notices.append(f'Migrated version {version} profile settings to version 40; retained preferences and added control and presentation defaults.'
                               if version < 40 else
                               'Invalid or unsupported profile fields were reset; valid preferences were retained.')
        else:
            changed = True
            notices.append('Unsupported profile format; restored Modernized defaults.')
    if changed:
        backup = path.with_name(path.name + '.recovered-' + uuid.uuid4().hex)
        # Exclusive creation ensures earlier recovery evidence is never replaced.
        with backup.open('xb') as stream:
            stream.write(raw)
        save(path, result)
        notices.append(f'Original settings preserved at {backup}')
    return result, notices


def reset(settings, mode):
    settings['profiles'][mode] = default_profile(mode)


def describe(settings):
    active = settings['active']
    renderer = settings['profiles'][active]['presentation']['renderer']
    lines = [('Profile: Vanilla (this launch only)' if active == 'vanilla' else
              'Profile: Modernized (run.py --vanilla plays the original game for one launch)'), f'Renderer: {renderer}',
             describe_presentation(settings['profiles'][active]['presentation']),
             ('Gameplay: original movement, camera and aiming' if active == 'vanilla' else
              'Gameplay: first-map modern movement; ' + settings['profiles'][active]['controls']['camera'] + ' camera; ' + settings['profiles'][active]['controls']['weapon_aim'] + ' weapon aiming'),
             'Changes take effect on the next launch. Memory-card location is unchanged.']
    if active == 'modernized':
        controls = settings['profiles'][active]['controls']
        lines.append(f"Mouse: {controls['mouse_sensitivity']} degrees/count; invert Y: {controls['invert_y']}")
        lines.append(f"Aim assistance: {controls['aim_assist']}; red dot: {'on' if controls['red_dot'] else 'off'}")
        lines.append(f"Crosshair: {'on' if controls['crosshair'] else 'off'}")
        lines.append(f"Jetpack: {controls['jetpack']}" + (' (modern mouse/WASD controls; original burst flight: Space boosts, gravity otherwise, Shift hover toggle)' if controls['jetpack'] == 'classic' else ' (camera-relative WASD, Space up, Ctrl down, hover on release)'))
        distance = controls['camera_distance']
        lines.append(f"Camera distance: {'original' if not distance else round(distance)} (Alt+wheel in game, saved); shoulder: {controls['shoulder']} "
                     f"({pc_input_key(settings, 'camera_shoulder')} cycles, {pc_input_key(settings, 'camera_recenter')} recenters)")
        lines.append(f"Look keys: {pc_input_key(settings, 'look_up')}/{pc_input_key(settings, 'look_down')} look up/down while held, "
                     f"{pc_input_key(settings, 'lock_look_up')}/{pc_input_key(settings, 'lock_look_down')} lock the look up/down; "
                     f"{pc_input_key(settings, 'turn_left')}/{pc_input_key(settings, 'turn_right')} turn left/right")
        lines.append(f"Saves: {pc_input_key(settings, 'quick_save')} quick save, {pc_input_key(settings, 'quick_load')} quick load "
                     "(last slot used; F2 / F3 open SAVE GAME / LOAD GAME)")
        lines.append(f"View: {controls['view']} person ({pc_input_key(settings, 'camera_view')} toggles in game, saved; "
                     "first-person is a prototype: swimming and jetpack flight use third person)")
        lines.append(f"Widescreen: {describe_widescreen(controls['widescreen'])}")
        lines.append(f"Jump: {describe_jump(controls['jump'])}")
        lines.append(f"Jump wall contact: {describe_jump_walls(controls['jump_walls'])}")
        lines.append(f"Landing: {describe_landing(controls['landing'])}")
        lines.append(f"Wall backflip: {describe_wall_backflip(controls['wall_backflip'])}")
        lines.append(f"Control scheme: {controls['scheme']}")
        lines.append(f"Crouch: {describe_crouch(controls['crouch'])}")
        lines.append(f"Dynamite: {describe_dynamite(controls['dynamite'])}")
        lines.append(f"Swimming: {describe_swimming(controls['swimming'])}")
        lines.append(f"Mesh geometry: {controls['geometry_precision']}; textures: {controls['texture_precision']} (OpenGL; next launch)")
        lines.append(f"Frame rate: {describe_frame_rate(controls['frame_rate'])}")
        lines.append(f"Draw distance: {describe_draw_distance(controls['draw_distance'])}")
        lines.append(f"Steroids: {describe_steroids(controls['steroids'], pc_input_key(settings, 'steroids'))}")
        lines.append(f"View bob (first person): {controls['view_bob']} (the eye stays still while idle; off/subtle/on/strong while walking)")
        lines.append(f"Emulated CPU: {controls['cpu_overclock']}%" + (' (original speed)' if controls['cpu_overclock'] == 100 else ' (keeps busy views at 30 fps; game speed unchanged)'))
        lines.append(f"CPU timing: {describe_cpu_timing(controls['cpu_timing'])}")
        lines.append(PREVIEW)
    return '\n'.join(lines)


def describe_steroids(value, key='R'):
    if value == 'portable':
        return f'portable (a pickup is stored as one dose; {key} takes it)'
    return 'original (the effect starts on pickup)'


def describe_frame_rate(value):
    if value == 'auto':
        return 'auto (display-aware rate up to 120 FPS; game speed unchanged)'
    if value == '60':
        return '60 (original presentation)'
    if value == 'display':
        return "display (the monitor's refresh rate; game speed unchanged)"
    if value == '30':
        return '30 (each new game image shown once; game speed unchanged)'
    if value == 'unlimited':
        return 'unlimited (present as often as possible; game speed unchanged)'
    return f'{value} (game speed unchanged)'


def describe_cpu_timing(value):
    if value == 'fast':
        return 'fast (gameplay uses far less host CPU, so high refresh rates and the overclock keep up; same emulated speed)'
    return 'accurate (full cycle model for every instruction)'


def describe_draw_distance(value):
    if value == 'original':
        return 'original (the original view limit: the far end of long views is black)'
    return 'extended (twice the original view limit; no black seams between distant sections)'


def describe_jump(value):
    if value == 'manual':
        return 'manual (leaves on the press, short grace after an edge, steer in the air)'
    return 'assisted (original: a run jump near a gap leaves from the edge, fixed arc)'


def describe_jump_walls(value):
    if value == 'slide':
        return 'slide (EDuke32 style: a wall or ceiling only stops the motion into it; Duke keeps his arc and slides along)'
    return 'original (a wall bounces Duke off and the jump is lost)'


def describe_crouch(value):
    if value == 'modern':
        return 'modern (hold crouch and move to walk low; Duke faces the view)'
    return 'original (crouch and a direction roll)'


def describe_wall_backflip(value):
    if value == 'modern':
        return 'modern (sliding off a steep car body or slope, Duke drops where it ends)'
    return 'original (sliding off one, Duke is launched backwards after a backflip-like slip)'


def describe_landing(value):
    if value == 'modern':
        return 'modern (a direction or jump pressed on landing moves Duke at once)'
    return 'original (after some landings Duke stands still for about 0.6 s)'


def describe_dynamite(value):
    if value == 'modern':
        return 'modern (a drawn stick is unlit until you fire; switch away freely)'
    return 'original (drawing it lights the fuse; throw it or it explodes in hand)'


def describe_swimming(value):
    if value == 'modern':
        return 'modern (Shift walks or runs in water at the land speeds)'
    return 'original (shallow water always runs; the original swim and underwater speeds)'


def describe_widescreen(value):
    if value == 'off':
        return 'off (original 4:3 picture)'
    if value == 'auto':
        return 'auto (follows the window from 4:3 to 21:9; wider view, HUD in the corners)'
    return f'{value} (wider view, HUD in the corners; movies and menus stay 4:3)'


def pc_input_key(settings, action):
    return settings['profiles'][settings['active']]['bindings'].get(action, 'Unbound')


def menu(settings, read=input, write=print):
    """Small terminal selector; cancellation leaves the caller's settings intact."""
    edited = copy.deepcopy(settings)
    while True:
        write('\n' + describe(edited))
        write('3 Renderer  |  4 Restore this profile  |  R Resolution and display\n'
              'G Geometry and texture precision (Modernized)  |  D Draw distance (Modernized)  |  W Widescreen (Modernized)  |  J Jump style (Modernized)  |  K Jump wall contact (Modernized)  |  P Wall backflip (Modernized)  |  C Crouch (Modernized)  |  L Landing (Modernized)  |  T Dynamite (Modernized)  |  U Swimming (Modernized)  |  F Frame rate (Modernized)  |  B View bob (Modernized)  |  S Steroids (Modernized)  |  5 Save and return  |  6 PC bindings (Modernized)  |  7 Camera (Modernized)  |  8 Weapon aiming (Modernized)  |  9 Aiming display / assistance (Modernized)  |  0 Cancel')
        choice = read('Choice: ').strip()
        if choice == '3':
            write('1 OpenGL  |  2 Software')
            renderer = read('Renderer: ').strip()
            if renderer in ('1', '2'):
                edited['profiles'][edited['active']]['presentation']['renderer'] = RENDERERS[int(renderer) - 1]
            else:
                write('Choose 1 or 2; renderer unchanged.')
        elif choice == '4':
            reset(edited, edited['active'])
        elif choice.lower() == 'r':
            presentation = edited['profiles'][edited['active']]['presentation']
            write(describe_presentation(presentation))
            write('Internal scale renders the 3D scene at 1x (original 512x240), 2x, 3x or 4x resolution with OpenGL '
                  '(the software renderer always uses 1x). '
                  'Display, window width (0 fits the display) and output filter only change how the image is shown.')
            values = read('scale display width filter (e.g. 2 windowed 0 linear; blank cancels): ').split()
            if not values:
                continue
            try:
                if len(values) != 4:
                    raise ValueError('Enter scale, display, width and filter.')
                edited['profiles'][edited['active']]['presentation'] = validate_presentation(
                    {**presentation, 'internal_scale': int(values[0]), 'display': values[1],
                     'fullscreen_mode': values[1] if values[1] in FULLSCREEN_MODES else presentation['fullscreen_mode'],
                     'window_width': int(values[2]), 'output_filter': values[3]})
            except ValueError as exc:
                write(f'{exc} Presentation unchanged.')
        elif choice.lower() == 'w':
            if edited['active'] != 'modernized':
                write('Vanilla always keeps the original 4:3 picture; select Modernized for widescreen.')
                continue
            write('Widescreen shows more of the world at the sides; the HUD moves to the screen corners. Movies and menus stay 4:3.')
            value = read('Widescreen (' + '/'.join(WIDESCREEN_MODES) + '; blank cancels): ').strip()
            if value in WIDESCREEN_MODES:
                edited['profiles']['modernized']['controls']['widescreen'] = value
            elif value:
                write('Choose one of the listed values; widescreen unchanged.')
        elif choice.lower() == 'j':
            if edited['active'] != 'modernized':
                write('Vanilla always keeps the original jump; select Modernized for the jump style.')
                continue
            write('assisted: original jump (a run jump pressed near a gap waits for the edge; fixed arc). '
                  'manual: Duke leaves on the press, can still jump just after running off an edge, and steers in the air.')
            value = read('Jump style (' + '/'.join(JUMP_STYLES) + '; blank cancels): ').strip()
            if value in JUMP_STYLES:
                edited['profiles']['modernized']['controls']['jump'] = value
            elif value:
                write('Choose one of the listed values; jump style unchanged.')
        elif choice.lower() == 'k':
            if edited['active'] != 'modernized':
                write('Vanilla always keeps the original wall bounce; select Modernized for jump wall contact.')
                continue
            write('slide: in a jump, a wall or ceiling only takes away the motion into it; Duke keeps his height, arc '
                  'and the motion along it (EDuke32 style). original: a wall bounces him off and the jump is lost.')
            value = read('Jump wall contact (' + '/'.join(JUMP_WALLS) + '; blank cancels): ').strip()
            if value in JUMP_WALLS:
                edited['profiles']['modernized']['controls']['jump_walls'] = value
            elif value:
                write('Choose one of the listed values; jump wall contact unchanged.')
        elif choice.lower() == 'p':
            if edited['active'] != 'modernized':
                write('Vanilla always keeps the original wall backflip; select Modernized for the wall backflip.')
                continue
            write('modern: when Duke lands on a face too steep to stand on (a car body, a steep slope) he slides down it '
                  'and drops where it ends. original: he slips back in a backflip-like pose and is launched down the slope.')
            value = read('Wall backflip (' + '/'.join(WALL_BACKFLIPS) + '; blank cancels): ').strip()
            if value in WALL_BACKFLIPS:
                edited['profiles']['modernized']['controls']['wall_backflip'] = value
            elif value:
                write('Choose one of the listed values; wall backflip unchanged.')
        elif choice.lower() == 'c':
            if edited['active'] != 'modernized':
                write('Vanilla always keeps the original crouch roll; select Modernized for the crouch walk.')
                continue
            write('modern: hold crouch and move to walk low in any direction, Duke facing the view (aim and fire as usual). '
                  'original: crouch and a direction roll.')
            value = read('Crouch (' + '/'.join(CROUCH_MODES) + '; blank cancels): ').strip()
            if value in CROUCH_MODES:
                edited['profiles']['modernized']['controls']['crouch'] = value
            elif value:
                write('Choose one of the listed values; crouch unchanged.')
        elif choice.lower() == 'l':
            if edited['active'] != 'modernized':
                write('Vanilla always keeps the original landing; select Modernized for the landing.')
                continue
            write('modern: after a jump, a direction or jump pressed as Duke lands moves him at once. '
                  'original: after a straight-up jump, or one whose direction was let go, he stands still for about 0.6 s.')
            value = read('Landing (' + '/'.join(LANDINGS) + '; blank cancels): ').strip()
            if value in LANDINGS:
                edited['profiles']['modernized']['controls']['landing'] = value
            elif value:
                write('Choose one of the listed values; landing unchanged.')
        elif choice.lower() == 't':
            if edited['active'] != 'modernized':
                write('Vanilla always lights dynamite on drawing it; select Modernized for the dynamite behaviour.')
                continue
            write('modern: selecting dynamite only draws it, unlit; fire lights and throws it, and you can switch away first. '
                  'original: drawing it lights the fuse, so it must be thrown or it explodes in Duke\'s hand.')
            value = read('Dynamite (' + '/'.join(DYNAMITE_MODES) + '; blank cancels): ').strip()
            if value in DYNAMITE_MODES:
                edited['profiles']['modernized']['controls']['dynamite'] = value
            elif value:
                write('Choose one of the listed values; dynamite unchanged.')
        elif choice.lower() == 'u':
            if edited['active'] != 'modernized':
                write('Vanilla always keeps the original swimming speeds; select Modernized for the swimming pace.')
                continue
            write('modern: in shallow water, wading, on the surface and underwater Duke walks, or runs with Shift, at the land speeds. '
                  'original: shallow water always runs, and the surface strokes and underwater thrust keep their original fixed speeds.')
            value = read('Swimming (' + '/'.join(SWIMMING_MODES) + '; blank cancels): ').strip()
            if value in SWIMMING_MODES:
                edited['profiles']['modernized']['controls']['swimming'] = value
            elif value:
                write('Choose one of the listed values; swimming unchanged.')
        elif choice.lower() == 's':
            if edited['active'] != 'modernized':
                write('Vanilla always starts steroids on pickup; select Modernized for portable steroids.')
                continue
            write('portable: a steroids pickup is stored as one dose (EDuke32 style) and the steroids key takes it; '
                  'with a dose held, more steroids stay where they are. original: the effect starts on pickup.')
            value = read('Steroids (' + '/'.join(STEROIDS_MODES) + '; blank cancels): ').strip()
            if value in STEROIDS_MODES:
                edited['profiles']['modernized']['controls']['steroids'] = value
            elif value:
                write('Choose one of the listed values; steroids unchanged.')
        elif choice.lower() == 'g':
            if edited['active'] != 'modernized':
                write('Select Modernized to change mesh precision. Vanilla keeps Original.')
                continue
            write('OpenGL: corrected geometry reduces mesh snapping; corrected textures reduce perspective warp. '
                  'Original retains the accepted presentation and existing near-wall fixes. '
                  'Some subdivided geometry/effects retain original rendering.')
            for field, label in (('geometry_precision','Geometry'),('texture_precision','Textures')):
                value = read(label + ' (original/corrected; blank keeps current): ').strip()
                if value in PRECISION_MODES:
                    edited['profiles']['modernized']['controls'][field] = value
                elif value:
                    write('Choose original or corrected; setting unchanged.')
        elif choice.lower() == 'd':
            if edited['active'] != 'modernized':
                write('Vanilla keeps the original view limit; select Modernized for the draw distance.')
                continue
            write('extended draws twice as far, so long corridors end in their real walls instead of black, '
                  'and closes thin black seams between distant sections. original keeps the original limit.')
            value = read('Draw distance (' + '/'.join(DRAW_DISTANCES) + '; blank cancels): ').strip()
            if value in DRAW_DISTANCES:
                edited['profiles']['modernized']['controls']['draw_distance'] = value
            elif value:
                write('Choose one of the listed values; draw distance unchanged.')
        elif choice.lower() == 'f':
            if edited['active'] != 'modernized':
                write('Vanilla always presents at the original 60; select Modernized for the frame rate.')
                continue
            write('Frame rate changes how often the picture is shown; game logic keeps its original speed. '
                  'display follows the monitor; 30 shows each new game image once; 60 is the original.')
            value = read('Frame rate (' + '/'.join(FRAME_RATES) + '; blank cancels): ').strip()
            if value in FRAME_RATES:
                edited['profiles']['modernized']['controls']['frame_rate'] = value
            elif value:
                write('Choose one of the listed values; frame rate unchanged.')
        elif choice.lower() == 'b':
            if edited['active'] != 'modernized':
                write('Vanilla keeps the original camera; select Modernized for view bob.')
                continue
            write('View bob moves the first-person eye gently while Duke walks; standing still stays steady at every setting.')
            value = read('View bob (' + '/'.join(VIEW_BOBS) + '; blank cancels): ').strip()
            if value in VIEW_BOBS:
                edited['profiles']['modernized']['controls']['view_bob'] = value
            elif value:
                write('Choose one of the listed values; view bob unchanged.')
        elif choice == '6':
            if edited['active'] != 'modernized':
                write('Vanilla preserves runtime keybinds.ini; select Modernized for PC actions.')
                continue
            profile = edited['profiles']['modernized']
            write(pc_input.describe(profile['bindings']))
            write('Inputs: ' + ', '.join(pc_input.TOKENS))
            write('Menu keys and F10/Escape are reserved. Supply multiple assignments to swap inputs.')
            assignments = read('ACTION=INPUT (space separated; blank cancels): ').split()
            try:
                profile['bindings'] = pc_input.rebind(profile['bindings'], assignments)
            except ValueError as exc:
                write(str(exc))
        elif choice == '7':
            if edited['active'] != 'modernized':
                write('Select Modernized to edit its camera; Vanilla always uses the original camera.')
                continue
            profile = edited['profiles']['modernized']
            write(str(profile['controls']))
            values = read('camera sensitivity invert-y (e.g. independent 0.12 off; blank cancels): ').split()
            if not values:
                continue
            try:
                if len(values) != 3 or values[2] not in ('on', 'off'):
                    raise ValueError('Enter camera, sensitivity and on/off.')
                profile['controls'] = validate_controls({**profile['controls'], 'camera': values[0], 'mouse_sensitivity': float(values[1]), 'invert_y': values[2] == 'on'})
            except ValueError as exc:
                write(str(exc))
        elif choice == '8':
            if edited['active'] != 'modernized':
                write('Vanilla always retains original aiming.')
                continue
            write('view: view-directed shots with optional assistance (supported weapons only); display/assistance options are under 9.')
            write('original: original weapon directions and in-game auto-aim setting; no modern crosshair.')
            value = read('Weapon aiming (view/original; blank cancels): ').strip()
            if value in AIM_MODES:
                edited['profiles']['modernized']['controls']['weapon_aim'] = value
            elif value:
                write('Choose view or original; aiming unchanged.')
        elif choice == '9':
            if edited['active'] != 'modernized':
                write('Select Modernized to edit its aiming display and assistance; Vanilla retains original behavior.')
                continue
            controls = edited['profiles']['modernized']['controls']
            write('Assistance is experimental: original-lock uses the game lock within 6 degrees, with camera/muzzle visibility checks. Original game autoaim must be enabled. Applies to view aiming only.')
            values = read('assist crosshair red-dot (e.g. original-lock on off; blank cancels): ').split()
            if not values: continue
            if len(values) != 3 or values[0] not in ('off','original-lock') or any(v not in ('on','off') for v in values[1:]):
                write('Enter off/original-lock, on/off, on/off; unchanged.')
                continue
            controls.update(aim_assist=values[0], crosshair=values[1]=='on', red_dot=values[2]=='on')
        elif choice == '5':
            return edited
        elif choice == '0':
            return None
        else:
            write('Choose a listed number.')
