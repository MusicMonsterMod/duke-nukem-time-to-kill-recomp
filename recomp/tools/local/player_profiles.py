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
# D23W: 0 is AUTO, measured from the GPU at launch (gpu_auto.py).
INTERNAL_SCALES = (0, 1, 2, 3, 4)
DISPLAYS = ('windowed', 'borderless', 'exclusive')
FULLSCREEN_MODES = ('borderless', 'exclusive')
OUTPUT_FILTERS = ('linear', 'nearest')
PRECISION_MODES = ('original', 'corrected')
WINDOW_WIDTH_RANGE = (640, 7680)
AIM_MODES = ('view', 'original')
CAMERAS = ('independent', 'original')
JETPACK_SCHEMES = ('modern', 'classic', 'eduke32')
SHOULDERS = ('center', 'right', 'left')
VIEWS = ('third', 'first')
# D35A: action keys are added only with implemented eye paths. Their values
# differ from global VIEW: legacy-third means the Modernized orbit for an action.
ACTION_VIEWS = ('first', 'legacy-third')


def valid_enum(value, choices):
    return isinstance(value, str) and value in choices


def validate_action_view(value):
    if not valid_enum(value, ACTION_VIEWS):
        raise ValueError('Action view must be first or legacy-third.')
    return value

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
AUTO_BIO_MASK_MODES = ('on', 'off')
# D08A33 equip pickups (Modernized only; Vanilla keeps the original). if-new:
# a weapon Duke did not own is drawn and a picked-up gadget becomes the [ / ]
# selection (D08A9); never: neither, the original behaviour.
EQUIP_PICKUPS_MODES = ('if-new', 'never')
NIGHT_VISION_MODES = ('modern', 'original')
# D39C Screen Flashes (Modernized only; Vanilla never flashes). on: Duke3D
# palfrom-style coloured flashes as measured; reduced: half strength, at most
# a quarter; off: none drawn.
SCREEN_FLASHES_MODES = ('on', 'reduced', 'off')
# D08J7C (Modernized only): after a climb, hang or pull-up swapped in the
# sidearm, auto brings the selected weapon back 0.75 s after the last shot;
# never (the default since schema 65) keeps the sidearm until the player
# chooses a weapon.
SIDEARM_RETURN_MODES = ('auto', 'never')
# D12E1 (Modernized only): KICK in third person. quick (the default since
# schema 66) is the legs-only snap kick with any weapon, on the move; original
# is the holstered Boot's full-body kicks 112-115.
THIRD_PERSON_KICKS = ('quick', 'original')
# D10H4 (Modernized only): WEAPON FEEL. retro (the default since schema 67)
# gives the guns with a Duke Nukem 3D analog a close simulation of EDuke32's
# shot timing (every click fires, an earlier first shot, D10H6 held fire on
# EDuke32's clock); modern keeps the original timing. Its first build called
# retro eduke32; that saved value is read as retro.
WEAPON_FEELS = ('retro', 'modern')
# D44A (Modernized only): EARTHQUAKES. on (the default since schema 68) shakes
# the view map-wide like Duke Nukem 3D's earthquake when a TNT barrel or an
# explosive vest goes off; off keeps the original short camera jitter.
EARTHQUAKE_MODES = ('on', 'off')
# D41A (Modernized, Polymorph only): DYNAMIC LIGHTS. on (the default since
# schema 71) lets blasts and muzzle flashes light the world per pixel on top
# of the original vertex lighting; off draws only the original light.
# D41E: shown as LIGHTING. on = ENHANCED, off = ORIGINAL, relit = RELIT (the
# painted light dimmed per level and room so placed lamps, shadows and blasts
# carry the scene). Adding a value needs no migration.
DYNAMIC_LIGHTS_MODES = ('on', 'relit', 'off')
LIGHTING_NAMES = {'original': 'off', 'enhanced': 'on', 'relit': 'relit'}
# D53 (Modernized, Polymorph only): SECURITY MONITORS. duke3d (the default
# since schema 72) draws the monitors as larger Duke 3D-style TVs with a
# lit screen and frames camera views with corners, REC and a record light;
# ttk keeps the original monitors and views.
SECURITY_MONITORS_MODES = ('duke3d', 'ttk')
# D41F1 (Modernized, Polymorph with LIGHTING on): SHADOW EDGES. smooth (the
# default since schema 73) gives shadows the exact, anti-aliased shape of the
# occluding geometry; simple is the cheaper stepped shadow-map edge.
SHADOW_EDGES_MODES = ('smooth', 'simple')
# D20I OPTIONS > DISPLAY (Modernized only): LEVEL STATS, the time / kills /
# secrets lines in the bottom-left corner, and FPS, the console's statistics
# block (the `fps` command and this setting stay in step). Booleans; defaults
# on and off (DEFAULT_CONTROLS).
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
# D17AR6 world renderer (Modernized only; Vanilla always draws Classic).
# polymorph is unified geometry, host world visibility and redraw-only live
# frames; classic is the game's own pipeline with the near path that first
# person and widescreen need.
WORLD_RENDERERS = ('polymorph', 'classic')
# Alt-wheel boom range in game units; 0 keeps the original follow distance.
CAMERA_DISTANCE_RANGE = (768, 6144)
# D10H3: AUTO AIM, EDuke32's autoaim cone, never an NPC: eduke32 every
# view-aimed weapon but throws; hitscan (the default since schema 69) the
# bullet guns; off; original-lock. Schema 70 split eduke32 from hitscan: the
# first builds' duke3d and a schema 69 eduke32 both meant hitscan.
AIM_ASSISTS = ('eduke32', 'hitscan', 'off', 'original-lock')
DEFAULT_CONTROLS = {'status_bars': 'full', 'scheme': 'modern', 'camera': 'independent', 'mouse_sensitivity': 0.12, 'invert_y': False, 'weapon_aim': 'view', 'crosshair': True, 'red_dot': False, 'aim_assist': 'hitscan', 'jetpack': 'modern', 'camera_distance': 0, 'first_person_legs': 'on', 'first_person_fov': 90, 'shoulder': 'center', 'view': 'third', 'widescreen': '16:9', 'cpu_overclock': 150, 'cpu_timing': 'fast', 'jump': 'assisted', 'frame_rate': 'auto', 'view_bob': 'on', 'geometry_precision': 'original', 'texture_precision': 'original', 'draw_distance': 'extended', 'world_renderer': 'polymorph', 'steroids': 'portable', 'jump_walls': 'slide', 'landing': 'modern', 'dynamite': 'modern', 'auto_bio_mask': 'on', 'equip_pickups': 'if-new', 'screen_flashes': 'on', 'sidearm_return': 'never', 'third_person_kick': 'quick', 'weapon_feel': 'retro', 'earthquakes': 'on', 'dynamic_lights': 'on', 'security_monitors': 'duke3d', 'shadow_edges': 'smooth', 'level_stats': True, 'fps': False, 'night_vision': 'modern', 'swimming': 'modern', 'swim_view': 'first', 'jetpack_view': 'first', 'push_view': 'first', 'ladder_view': 'first', 'pole_view': 'first', 'mantle_view': 'first', 'overhead_view': 'first', 'wall_view': 'first', 'wall_backflip': 'modern', 'crouch': 'modern'}
# display is how the game opens (the runtime reports the last state at exit);
# fullscreen_mode is what F11 enters from a window.
SOUND_DEFAULTS = dict(sound_master=-1, sound_effects=-1, sound_music=-1, sound_speech=-1, sound_mode=-1)
CONTROLLER_DEFAULTS = dict(controller_speed=180, controller_deadzone=20, controller_invert_y=False, split_devices="two-pads")
DEFAULT_CONTROLS.update(SOUND_DEFAULTS)
DEFAULT_CONTROLS.update(CONTROLLER_DEFAULTS)

DEFAULT_PRESENTATION = {'renderer': 'opengl', 'internal_scale': 1, 'display': 'windowed', 'fullscreen_mode': 'borderless', 'window_width': 0, 'output_filter': 'linear'}
DEFAULT_PROFILE = {'presentation': DEFAULT_PRESENTATION, 'bindings': pc_input.DEFAULTS, 'controls': DEFAULT_CONTROLS}
# Modernized renders at four times the original resolution (user choice after
# the D13 playtest: 60 fps on a GTX 1080 Ti); Vanilla keeps the original.
MODE_PRESENTATION = {'vanilla': {}, 'modernized': {'internal_scale': 0}}
PREVIEW = ('Modernized first-map preview: captured WASD is camera-relative; mouse orbits the '
           'third-person camera. Unavailable hooks never switch the selected control scheme. '
           'View aiming covers supported weapons, with AUTO AIM HITSCAN for the Desert Eagle, Combat Shotgun, Buffalo Rifle and Gatling; others retain original aiming. Gameplay captures automatically on first entry; Escape pauses and frees the mouse; resume captures automatically. OPTIONS > CONTROLS selects Modern or Legacy; F10 has no action.')


def validate_controls(value):
    # D19V: discard retired overrides, including stale menu sidecar values.
    # Keep other preferences even when an obsolete value is malformed.
    if isinstance(value, dict):
        value = {k: v for k, v in value.items() if k not in ('use_view', 'mirror_view')}
        # D10H4: WEAPON FEEL's first build saved retro as eduke32.
        if value.get('weapon_feel') == 'eduke32':
            value['weapon_feel'] = 'retro'
        # D10H3: AUTO AIM's first build saved hitscan as duke3d.
        if value.get('aim_assist') == 'duke3d':
            value['aim_assist'] = 'hitscan'
    if isinstance(value, dict):
        # D19X: recover this additive field without losing other preferences.
        fov = value.get('first_person_fov', 90)
        if type(fov) not in (int, float) or not math.isfinite(fov) or not 60 <= fov <= 120:
            fov = 90
        value = {**value, 'status_bars': value.get('status_bars') if valid_enum(value.get('status_bars'), ('full', 'borderless', 'hidden')) else 'full', 'first_person_fov': round(fov),
                 'first_person_legs': value.get('first_person_legs') if valid_enum(value.get('first_person_legs'), ('on', 'off')) else 'on',
                 'world_renderer': value.get('world_renderer') if valid_enum(value.get('world_renderer'), WORLD_RENDERERS) else 'polymorph'}
    if not isinstance(value, dict) or set(value) != set(DEFAULT_CONTROLS):
        raise ValueError('Invalid camera preferences.')
    validate_action_view(value['swim_view'])
    validate_action_view(value['jetpack_view'])
    validate_action_view(value['push_view'])
    validate_action_view(value['ladder_view'])
    validate_action_view(value['pole_view'])
    validate_action_view(value['mantle_view'])
    validate_action_view(value['overhead_view'])
    validate_action_view(value['wall_view'])
    for key in SOUND_DEFAULTS:
        if type(value[key]) is not int or not -1 <= value[key] <= (1 if key == 'sound_mode' else 100):
            raise ValueError('Sound levels must be -1 (existing setting) or 0..100; mode -1/0/1.')
    if (type(value['controller_speed']) is not int or not 30 <= value['controller_speed'] <= 360 or
        type(value['controller_deadzone']) is not int or not 5 <= value['controller_deadzone'] <= 50 or
        type(value['controller_invert_y']) is not bool):
        raise ValueError('Controller speed 30..360, deadzone 5..50 percent, inversion boolean.')
    if not valid_enum(value['split_devices'], ('two-pads', 'keyboard-pad')):
        raise ValueError('Split devices must be two-pads or keyboard-pad.')
    sensitivity = value['mouse_sensitivity']
    if (value['weapon_aim'] not in AIM_MODES or value['scheme'] not in ('modern', 'legacy') or value['camera'] not in CAMERAS or value['aim_assist'] not in AIM_ASSISTS or value['jetpack'] not in JETPACK_SCHEMES or type(value['red_dot']) is not bool or type(value['crosshair']) is not bool or type(value['invert_y']) is not bool or
        type(sensitivity) not in (int, float) or not math.isfinite(sensitivity) or
        not 0.01 <= sensitivity <= 2 or not valid_camera_distance(value['camera_distance']) or value['shoulder'] not in SHOULDERS or not valid_enum(value['view'], VIEWS) or
        value['widescreen'] not in WIDESCREEN_MODES or type(value['cpu_overclock']) is not int or
        value['cpu_overclock'] not in CPU_OVERCLOCKS or value['cpu_timing'] not in CPU_TIMINGS or value['jump'] not in JUMP_STYLES or
        value['geometry_precision'] not in PRECISION_MODES or value['texture_precision'] not in PRECISION_MODES or
        value['frame_rate'] not in FRAME_RATES or value['view_bob'] not in VIEW_BOBS or
        value['draw_distance'] not in DRAW_DISTANCES or value['steroids'] not in STEROIDS_MODES or value['jump_walls'] not in JUMP_WALLS or value['landing'] not in LANDINGS or value['auto_bio_mask'] not in AUTO_BIO_MASK_MODES or value['equip_pickups'] not in EQUIP_PICKUPS_MODES or value['screen_flashes'] not in SCREEN_FLASHES_MODES or value['sidearm_return'] not in SIDEARM_RETURN_MODES or value['third_person_kick'] not in THIRD_PERSON_KICKS or value['weapon_feel'] not in WEAPON_FEELS or value['earthquakes'] not in EARTHQUAKE_MODES or value['dynamic_lights'] not in DYNAMIC_LIGHTS_MODES or value['security_monitors'] not in SECURITY_MONITORS_MODES or value['shadow_edges'] not in SHADOW_EDGES_MODES or type(value['level_stats']) is not bool or type(value['fps']) is not bool or value['night_vision'] not in NIGHT_VISION_MODES or value['dynamite'] not in DYNAMITE_MODES or value['swimming'] not in SWIMMING_MODES or value['wall_backflip'] not in WALL_BACKFLIPS or value['crouch'] not in CROUCH_MODES):
        raise ValueError('Camera must be independent/original; sensitivity 0.01..2 degrees/count; invert_y boolean; jetpack modern/classic/eduke32; '
                         'camera_distance 0 (original) or 768..6144; shoulder center/right/left; view third/first; '
                         'widescreen off/16:9/16:10/21:9/auto; cpu_overclock 100/125/150/175/200; cpu_timing accurate/fast; jump assisted/manual; '
                         'frame_rate ' + '/'.join(FRAME_RATES) + '; view_bob ' + '/'.join(VIEW_BOBS) + '; geometry_precision and texture_precision original/corrected; draw_distance original/extended; steroids portable/original; jump_walls slide/original; landing modern/original; auto_bio_mask on/off; equip_pickups if-new/never; screen_flashes on/reduced/off; sidearm_return auto/never; third_person_kick quick/original; weapon_feel retro/modern; earthquakes on/off; dynamic_lights on/relit/off; security_monitors duke3d/ttk; shadow_edges smooth/simple; level_stats and fps booleans; night_vision modern/original; dynamite modern/original; swimming modern/original; wall_backflip modern/original; crouch modern/original.')
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
        raise ValueError('Renderer opengl/software; internal scale 0 (auto) or 1..4; display windowed/borderless/exclusive; '
                         'fullscreen mode borderless/exclusive; '
                         'window width 0 (fit the display) or 640..7680; output filter linear/nearest.')
    return dict(value)


def default_profile(mode):
    profile = copy.deepcopy(DEFAULT_PROFILE)
    profile['presentation'].update(MODE_PRESENTATION[mode])
    if mode == 'vanilla':
        profile['controls']['frame_rate'] = '60'
    return profile


def effective_scale(presentation, auto_scale=None):
    """Software supersampling is CPU-bound (measured 29 fps at 2x against 60 at
    1x), so the software renderer always runs at the original resolution. The
    saved scale is kept for OpenGL. D23W: AUTO (0) takes the measured scale
    (gpu_auto.resolve), or gpu_auto.FALLBACK_SCALE when none is given."""
    if presentation['renderer'] == 'software':
        return 1
    if presentation['internal_scale'] == 0:
        import gpu_auto
        return auto_scale or gpu_auto.FALLBACK_SCALE
    return presentation['internal_scale']


def presentation_args(presentation, auto_scale=None):
    """Runtime flags for the saved presentation (patched runtime CLI overrides)."""
    return ['--renderer', presentation['renderer'],
            '--internal-scale', str(effective_scale(presentation, auto_scale)),
            '--display', presentation['display'],
            '--fullscreen-mode', presentation['fullscreen_mode'],
            '--window-width', str(presentation['window_width']),
            '--output-filter', presentation['output_filter']]


def describe_presentation(presentation, auto_scale=None):
    width = presentation['window_width']
    tail = (f"display: {presentation['display']} (F11: {presentation['fullscreen_mode']}); "
            f"window width: {width or 'fit the display'}; output filter: {presentation['output_filter']}")
    if presentation['internal_scale'] == 0 and presentation['renderer'] != 'software':
        if auto_scale is None:
            import gpu_auto
            auto_scale = gpu_auto.cached_scale()
        if not auto_scale:
            return f"Internal resolution: auto (measured from the GPU at the next launch); {tail}"
        return (f"Internal resolution: auto, {auto_scale}x on this GPU ({'original, ' if auto_scale == 1 else ''}512x240 "
                f"gameplay renders at {512 * auto_scale}x{240 * auto_scale}); {tail}")
    scale = effective_scale(presentation, auto_scale)
    saved = 'auto' if presentation['internal_scale'] == 0 else f"{presentation['internal_scale']}x"
    held = f"; saved {saved} applies with OpenGL" if scale != presentation['internal_scale'] else ''
    return (f"Internal resolution: {scale}x ({'original, ' if scale == 1 else ''}512x240 gameplay renders at "
            f"{512 * scale}x{240 * scale}{held}); {tail}")


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
        bindings = dict(data['bindings']) if 'bindings' in data else profile['bindings']
        # Preserve a pending schema-59 menu confirmation when adding HUD keys.
        if set(bindings) == set(pc_input.DEFAULTS) - {'hud_less', 'hud_more'}:
            bindings = pc_input.add_missing_actions(bindings)
        else:
            bindings = pc_input.validate(bindings)
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
    return {'version': 73, 'active': 'modernized',
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
        if type(version) is int and version > 73:
            raise ValueError(f'Profile settings version {version} is newer than this launcher; file left unchanged.')
        if type(version) is int and version == 0:
            mode = data.get('mode', 'modernized')
            renderer = data.get('renderer', 'opengl')
            if mode in MODES:
                if renderer in RENDERERS:
                    result['profiles'][mode]['presentation']['renderer'] = renderer
            changed = True
            notices.append('Migrated version 0 profile settings to version 73.')
        elif type(version) is int and version in (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73):
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
                        result['profiles'][mode]['controls'] = validate_controls({**({"split_devices":"two-pads"} if version < 56 else {}), **(CONTROLLER_DEFAULTS if version < 55 else {}), **(SOUND_DEFAULTS if version < 53 else {}), **({k:v for k,v in DEFAULT_CONTROLS.items() if k in ('crosshair','red_dot','aim_assist')} if version < 5 else {}), **({'weapon_aim':'view'} if version < 4 else {}), **({'jetpack':'modern'} if version < 11 else {}), **({'camera_distance':0,'shoulder':'center'} if version < 12 else {}), **({'view':'third'} if version < 13 else {}), **({'widescreen':'16:9'} if version < 18 else {}), **({'cpu_overclock':150} if version < 19 else {}), **({'jump':'assisted'} if version < 20 else {}), **({'frame_rate':'60'} if version < 21 else {}), **({'view_bob':'on'} if version < 22 else {}), **({'geometry_precision':'original','texture_precision':'original'} if version < 23 else {}), **({'draw_distance':'extended'} if version < 24 else {}), **({'cpu_timing':'fast'} if version < 25 else {}), **({'steroids':'portable'} if version < 29 else {}), **({'jump_walls':'slide'} if version < 30 else {}), **({'landing':'modern'} if version < 31 else {}), **({'dynamite':'modern'} if version < 32 else {}), **({'swimming':'modern'} if version < 33 else {}), **({'wall_backflip':'modern'} if version < 37 else {}), **({'crouch':'modern'} if version < 39 else {}), **({'scheme':'modern'} if version < 40 else {}), **({'swim_view':'first'} if version < 41 else {}), **({'jetpack_view':'first'} if version < 42 else {}), **({'push_view':'first'} if version < 43 else {}), **({'ladder_view':'first'} if version < 44 else {}), **({'pole_view':'first'} if version < 45 else {}), **({'mantle_view':'first'} if version < 46 else {}), **({'overhead_view':'first'} if version < 47 else {}), **({'wall_view':'first'} if version < 48 else {}), **({'night_vision':'modern'} if version < 51 else {}), **({'auto_bio_mask':'on'} if version < 52 else {}), **({'equip_pickups':'if-new'} if version < 61 else {}), **({'screen_flashes':'on'} if version < 62 else {}), **({'level_stats':True,'fps':False} if version < 63 else {}), **({'sidearm_return':'never'} if version < 64 else {}), **({'third_person_kick':'quick'} if version < 66 else {}), **({'weapon_feel':'retro'} if version < 67 else {}), **({'earthquakes':'on'} if version < 68 else {}), **({'dynamic_lights':'on'} if version < 71 else {}), **({'security_monitors':'duke3d'} if version < 72 else {}), **({'shadow_edges':'smooth'} if version < 73 else {}), **profile['controls'], **({'sidearm_return':'never'} if version == 64 else {}), **({'aim_assist':'hitscan'} if version < 69 and mode == 'modernized' and profile['controls'].get('aim_assist','off') == 'off' else {}), **({'aim_assist':'hitscan'} if version == 69 and profile['controls'].get('aim_assist') in ('eduke32','duke3d') else {})})
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
                        result['profiles'][mode]['bindings'] = dict(pc_input.migrate_bindings(profile['bindings']) if version < 8 else pc_input.migrate_grab(profile['bindings']) if version < 53 else pc_input.add_missing_actions(profile['bindings']) if version < 60 else pc_input.validate(profile['bindings']))
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
            # D10H4: the first WEAPON FEEL build saved retro as eduke32. A
            # profile that differs only by that is renamed in place, quietly.
            renamed = json.loads(json.dumps(data))
            for profile in renamed.get('profiles', {}).values():
                if isinstance(profile, dict) and isinstance(profile.get('controls'), dict) and profile['controls'].get('weapon_feel') == 'eduke32':
                    profile['controls']['weapon_feel'] = 'retro'
            if renamed != data and result == renamed:
                save(path, result)
                notices.append('WEAPON FEEL EDUKE32 is now called RETRO; your choice is kept.')
                return result, notices
            changed = result != data
            if changed and result != {**data, 'active': 'modernized'}:
                notices.append(f'Migrated version {version} profile settings to version 73; retained preferences and added control and presentation defaults.'
                               if version < 73 else
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
        lines.append(f"Jetpack: {controls['jetpack']}" + (' (modern mouse/WASD controls; original burst flight: Space boosts, gravity otherwise, Shift hover toggle)' if controls['jetpack'] == 'classic' else ' (J toggles instant flight, including midair; 61 seconds of fuel; Space up, Ctrl down)' if controls['jetpack'] == 'eduke32' else ' (J starts with a short lift; camera-relative WASD, Space up, Ctrl down, hover on release)'))
        distance = controls['camera_distance']
        lines.append(f"Camera distance: {'original' if not distance else round(distance)} (Alt+wheel in game, saved); shoulder: {controls['shoulder']} "
                     f"({pc_input_key(settings, 'camera_shoulder')} cycles, {pc_input_key(settings, 'camera_recenter')} recenters)")
        lines.append(f"Look keys: {pc_input_key(settings, 'look_up')}/{pc_input_key(settings, 'look_down')} look up/down while held, "
                     f"{pc_input_key(settings, 'lock_look_up')}/{pc_input_key(settings, 'lock_look_down')} lock the look up/down; "
                     f"{pc_input_key(settings, 'turn_left')}/{pc_input_key(settings, 'turn_right')} turn left/right")
        lines.append(f"Saves: {pc_input_key(settings, 'quick_save')} quick save, {pc_input_key(settings, 'quick_load')} quick load "
                     "(last slot used; F2 / F3 open SAVE GAME / LOAD GAME)")
        lines.append(f"View: {controls['view']} person ({pc_input_key(settings, 'camera_view')} toggles in game, saved; "
                     "first-person supports swimming and jetpack flight; push/pull and attached traversal use third person)")
        lines.append(f"Widescreen: {describe_widescreen(controls['widescreen'])}")
        lines.append(f"Jump: {describe_jump(controls['jump'])}")
        lines.append(f"Jump wall contact: {describe_jump_walls(controls['jump_walls'])}")
        lines.append(f"Landing: {describe_landing(controls['landing'])}")
        lines.append(f"Wall backflip: {describe_wall_backflip(controls['wall_backflip'])}")
        lines.append(f"Control scheme: {controls['scheme']}")
        lines.append(f"Crouch: {describe_crouch(controls['crouch'])}")
        lines.append(f"Auto Bio Mask: {controls['auto_bio_mask']} (instant use; automatic gas protection)")
        lines.append(f"Equip pickups: {controls['equip_pickups']} (if-new draws a new weapon and selects a new item)")
        lines.append(f"Weapon feel: {controls['weapon_feel']} (retro: every click fires, the first shot comes sooner and held fire keeps a steady rate, close to EDuke32; modern: the original timing)")
        lines.append(f"Earthquakes: {controls['earthquakes']} (on: TNT barrels and explosive vests shake the whole level, Duke Nukem 3D style)")
        lines.append(f"Lighting: {controls['dynamic_lights']} (on: Enhanced, real lights from blasts, guns, lamps and signs on the original light; relit: Relit, the original light dimmed so those lights carry the scene; off: Original; Polymorph renderer only)")
        lines.append(f"Live shadows: {'sharp' if controls['shadow_edges'] == 'smooth' else 'simple'} (shadow edges of blasts and flickering or breakable lamps; sharp: the exact shape of the geometry, anti-aliased; simple: faster, stepped edges; steady lamps are baked; Lighting Enhanced or Relit only)")
        lines.append(f"Security monitors: {controls['security_monitors']} (duke3d: larger lit TV screens and a camera frame with REC, Duke 3D style; ttk: the original monitors; Polymorph renderer only)")
        lines.append(f"Third-person kick: {controls['third_person_kick']} (quick: a fast legs-only kick with any weapon, on the move; original: the holstered Boot's full-body kicks)")
        lines.append(f"Sidearm return: {controls['sidearm_return']} (never: keep a climb sidearm until you choose; auto: your weapon comes back 0.75 s after the last shot)")
        lines.append(f"Screen flashes: {controls['screen_flashes']} (Duke3D-style coloured flashes; reduced is half strength, at most a quarter)")
        lines.append(f"Level stats: {'on' if controls['level_stats'] else 'off'} (time, kills and secrets in the bottom-left corner)")
        lines.append(f"FPS display: {'on' if controls['fps'] else 'off'} (the console's statistics block, top left)")
        lines.append(f"Night vision: {controls['night_vision']} (Modern toggles instantly without interrupting fire)")
        lines.append(f"Dynamite: {describe_dynamite(controls['dynamite'])}")
        lines.append(f"Swimming: {describe_swimming(controls['swimming'])}")
        lines.append(f"Wall climb view: {controls['wall_view']} (when global view is first person)")
        lines.append(f"Overhead view: {controls['overhead_view']} (when global view is first person)")
        lines.append(f"Mantle view: {controls['mantle_view']} (when global view is first person)")
        lines.append(f"Chain/pole view: {controls['pole_view']} (when global view is first person)")
        lines.append(f"Ladder view: {controls['ladder_view']} (when global view is first person)")
        lines.append(f"Push/pull view: {controls['push_view']} (when global view is first person)")
        lines.append(f"Jetpack view: {controls['jetpack_view']} (when global view is first person)")
        lines.append(f"Swimming view: {controls['swim_view']} (when global view is first person)")
        lines.append(f"Mesh geometry: {controls['geometry_precision']}; textures: {controls['texture_precision']} (OpenGL; next launch)")
        lines.append(f"Frame rate: {describe_frame_rate(controls['frame_rate'])}")
        lines.append(f"World renderer: {describe_world_renderer(controls['world_renderer'])}")
        lines.append(f"Draw distance: {describe_draw_distance(controls['draw_distance'])}")
        lines.append(f"Steroids: {describe_steroids(controls['steroids'], pc_input_key(settings, 'steroids'))}")
        lines.append(f"First-person legs: {controls['first_person_legs']} (Options > Advanced; torso stays hidden)")
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
        return 'auto (your display\'s refresh up to 240 FPS, half or a third where a scene cannot keep up; game speed unchanged)'
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


def describe_world_renderer(value):
    if value == 'classic':
        return 'Classic (the game\'s own drawing order and visibility; next launch)'
    return 'Polymorph (one geometry path, host world visibility, redraw-only frames at high refresh; next launch)'


def describe_draw_distance(value):
    if value == 'original':
        return 'original (the original view limit: the far end of long views is black)'
    return 'extended (the maximum view limit, over outdoor walls from the jetpack, distant enemies in place but dormant until you are near or hurt them; no black seams between distant sections)'


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
              'G Geometry and texture precision (Modernized)  |  D Draw distance (Modernized)  |  W Widescreen (Modernized)  |  J Jump style (Modernized)  |  K Jump wall contact (Modernized)  |  P Wall backflip (Modernized)  |  C Crouch (Modernized)  |  L Landing (Modernized)  |  M Auto Bio Mask (Modernized)  |  E Equip pickups (Modernized)  |  X Screen flashes (Modernized)  |  A Sidearm return (Modernized)  |  Q Third-person kick (Modernized)  |  H Weapon feel (Modernized)  |  Z Earthquakes (Modernized)  |  I Lighting (Modernized)  |  V Level stats (Modernized)  |  Y FPS display (Modernized)  |  N Night vision (Modernized)  |  T Dynamite (Modernized)  |  U Swimming (Modernized)  |  F Frame rate (Modernized)  |  B View bob (Modernized)  |  S Steroids (Modernized)  |  5 Save and return  |  6 PC bindings (Modernized)  |  7 Camera (Modernized)  |  8 Weapon aiming (Modernized)  |  9 Aiming display / assistance (Modernized)  |  0 Cancel')
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
                  '(the software renderer always uses 1x); 0 is AUTO, measured from the GPU at launch. '
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
        elif choice.lower() == 'e':
            if edited['active'] != 'modernized':
                write('Vanilla keeps the original pickups: nothing is equipped or selected.')
                continue
            value = read('Equip pickups (' + '/'.join(EQUIP_PICKUPS_MODES) + '; blank cancels): ').strip()
            if value in EQUIP_PICKUPS_MODES:
                edited['profiles']['modernized']['controls']['equip_pickups'] = value
            elif value:
                write('Choose if-new or never; equip pickups unchanged.')
        elif choice.lower() == 'q':
            if edited['active'] != 'modernized':
                write('Vanilla keeps the original third-person kick.')
                continue
            value = read('Third-person kick (' + '/'.join(THIRD_PERSON_KICKS) + '; blank cancels): ').strip()
            if value in THIRD_PERSON_KICKS:
                edited['profiles']['modernized']['controls']['third_person_kick'] = value
            elif value:
                write('Choose quick or original; third-person kick unchanged.')
        elif choice.lower() == 'h':
            if edited['active'] != 'modernized':
                write('Vanilla keeps the original weapon timing.')
                continue
            value = read('Weapon feel (' + '/'.join(WEAPON_FEELS) + '; blank cancels): ').strip()
            if value in WEAPON_FEELS:
                edited['profiles']['modernized']['controls']['weapon_feel'] = value
            elif value:
                write('Choose retro or modern; weapon feel unchanged.')
        elif choice.lower() == 'z':
            if edited['active'] != 'modernized':
                write('Vanilla keeps the original camera jitter.')
                continue
            value = read('Earthquakes (' + '/'.join(EARTHQUAKE_MODES) + '; blank cancels): ').strip()
            if value in EARTHQUAKE_MODES:
                edited['profiles']['modernized']['controls']['earthquakes'] = value
            elif value:
                write('Choose on or off; earthquakes unchanged.')
        elif choice.lower() == 'i':
            if edited['active'] != 'modernized':
                write('Vanilla keeps the original lighting.')
                continue
            value = read('Lighting (' + '/'.join(DYNAMIC_LIGHTS_MODES) + ', or original/enhanced; blank cancels): ').strip()
            value = LIGHTING_NAMES.get(value, value)
            if value in DYNAMIC_LIGHTS_MODES:
                edited['profiles']['modernized']['controls']['dynamic_lights'] = value
            elif value:
                write('Choose on, relit or off; lighting unchanged.')
        elif choice.lower() == 'a':
            if edited['active'] != 'modernized':
                write('Vanilla has no climbing sidearm swap.')
                continue
            value = read('Sidearm return (' + '/'.join(SIDEARM_RETURN_MODES) + '; blank cancels): ').strip()
            if value in SIDEARM_RETURN_MODES:
                edited['profiles']['modernized']['controls']['sidearm_return'] = value
            elif value:
                write('Choose auto or never; sidearm return unchanged.')
        elif choice.lower() == 'x':
            if edited['active'] != 'modernized':
                write('Vanilla never flashes the screen.')
                continue
            value = read('Screen flashes (' + '/'.join(SCREEN_FLASHES_MODES) + '; blank cancels): ').strip()
            if value in SCREEN_FLASHES_MODES:
                edited['profiles']['modernized']['controls']['screen_flashes'] = value
            elif value:
                write('Choose on, reduced or off; screen flashes unchanged.')
        elif choice.lower() in ('v', 'y'):
            key, name = ('level_stats', 'Level stats') if choice.lower() == 'v' else ('fps', 'FPS display')
            if edited['active'] != 'modernized':
                write(f'{name} is a Modernized setting.')
                continue
            value = read(f'{name} (on/off; blank cancels): ').strip()
            if value in ('on', 'off'):
                edited['profiles']['modernized']['controls'][key] = value == 'on'
            elif value:
                write(f'Choose on or off; {name.lower()} unchanged.')
        elif choice.lower() == 'm':
            if edited['active'] != 'modernized':
                write('Vanilla keeps original manual Bio Mask use.')
                continue
            value = read('Auto Bio Mask (on/off; instant use when on; blank cancels): ').strip()
            if value in AUTO_BIO_MASK_MODES:
                edited['profiles']['modernized']['controls']['auto_bio_mask'] = value
            elif value:
                write('Choose on or off; Auto Bio Mask unchanged.')
        elif choice.lower() == 'n':
            if edited['active'] != 'modernized':
                write('Vanilla keeps the original goggles equip animation.')
                continue
            write('Modern toggles night vision instantly without interrupting movement or fire. Original keeps the equip animation.')
            value = read('Night vision (' + '/'.join(NIGHT_VISION_MODES) + '; blank cancels): ').strip()
            if value in NIGHT_VISION_MODES:
                edited['profiles']['modernized']['controls']['night_vision'] = value
            elif value:
                write('Choose one of the listed values; night vision unchanged.')
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
            write('extended draws as far as the PlayStation depth range allows, so long corridors end in their real walls '
                  'instead of black, shows outdoor areas over the walls when you fly above them, shows distant enemies standing '
                  'still until you come near or hurt them and closes thin black seams between distant sections. original keeps the original limit.')
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
            write('hitscan (default): Desert Eagle, Combat Shotgun, Buffalo Rifle and Gatling rounds go at the nearest visible enemy (never an NPC) within about 8.4 degrees of the crosshair, as EDuke32 autoaim on hitscan weapons; eduke32: every weapon but throws. original-lock follows the game lock within 6 degrees. Applies to view aiming only.')
            values = read('assist crosshair red-dot (e.g. hitscan on off; blank cancels): ').split()
            if not values: continue
            if len(values) != 3 or values[0] not in AIM_ASSISTS or any(v not in ('on','off') for v in values[1:]):
                write('Enter eduke32/hitscan/off/original-lock, on/off, on/off; unchanged.')
                continue
            controls.update(aim_assist=values[0], crosshair=values[1]=='on', red_dot=values[2]=='on')
        elif choice == '5':
            return edited
        elif choice == '0':
            return None
        else:
            write('Choose a listed number.')
