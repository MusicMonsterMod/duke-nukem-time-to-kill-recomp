"""D04 action schema shared with native input; no game memory or pad emulation."""
from pathlib import Path
import re

DEFINITIONS = (Path(__file__).resolve().parents[2] / 'src/ttk/input_bindings.def')
ROWS = re.findall(r'TTK_ACTION\((\w+), (\w+), (-?\d+), (\d+)\)', DEFINITIONS.read_text())
DEFAULTS = {name: token for name, token, _, _ in ROWS}
# Function keys, Tab and Escape are reserved; Ctrl is bindable. Alt (either Alt
# key) is bindable as a held action; Alt + wheel still sets camera distance.
# Fixed menu controls stay available even after every action is rebound.
TOKENS = {chr(65+i): 4+i for i in range(26) if chr(65+i) not in 'CF'}
TOKENS['C'] = 6  # Only holster may share the fixed original Circle key.
TOKENS.update({str(i): 29+i for i in range(1, 10)})
TOKENS.update({'LCtrl':224, 'LBracket':47, 'RBracket':48, 'Semicolon':51, 'Apostrophe':52, 'Comma': 54, 'Period': 55, '0': 39, 'Space': 44, 'LShift': 225, 'ScrollLock': 71, 'Unbound': 0, 'Mouse1': -1,
               'Mouse2': -3, 'Mouse3': -2, 'Mouse4': -4, 'Mouse5': -5, 'Alt': 226})


def validate(bindings):
    if not isinstance(bindings, dict) or set(bindings) != set(DEFAULTS):
        raise ValueError('Input bindings must contain every listed action exactly once.')
    used = {}
    for action, token in bindings.items():
        if not isinstance(token, str) or token not in TOKENS:
            raise ValueError(f'{action}: unsupported or reserved input {token!r}.')
        if token == 'C' and action != 'holster':
            raise ValueError('C is reserved for holster.')
        if token != 'Unbound' and token in used:
            raise ValueError(f'{token} conflicts: {used[token]} and {action}; bindings unchanged.')
        if token != 'Unbound':
            used[token] = action
    return bindings


def rebind(bindings, assignments):
    candidate = dict(bindings)
    for assignment in assignments:
        action, sep, token = assignment.partition('=')
        if not sep or action not in DEFAULTS:
            raise ValueError('Use ACTION=INPUT; list actions with --show-bindings.')
        candidate[action] = token
    return validate(candidate)


def wire(bindings):
    validate(bindings)
    return '1:' + ','.join(str(TOKENS[bindings[name]]) for name in DEFAULTS)


NOTES = {'walk': ' (speed modifier; Caps Lock toggles autorun)',
         'grab': ' (hold to grab and push/pull; precision aim with original weapon aiming or camera)',
         'grab_alt': ' (second hold-to-grab input; the only grab with original weapon aiming or camera)',
         'original_aim': ' (original precision aim; unbound by default in Modernized)'}


def describe(bindings):
    return '\n'.join(f'{name}={bindings[name]}' + NOTES.get(name, '') for name in DEFAULTS)


def migrate_bindings(bindings):
    """Migrate old defaults and add actions without stealing customized bindings."""
    if not isinstance(bindings, dict):
        raise ValueError('Invalid action bindings.')
    result = dict(bindings)
    for action, old, new in [('original_strafe_left','Q','Comma'),('original_strafe_right','E','Period')]:
        if result.get(action) == old and new not in result.values(): result[action] = new
    if 'interact' not in result:
        available = next((key for key in ['E','R','T','G','B','N','M','U','O','P'] if key not in result.values()), None)
        if available is None:
            raise ValueError('No free interaction default; configure bindings.')
        result['interact'] = available
    if result.get('crouch') == 'V' and 'LCtrl' not in result.values():
        result['crouch']='LCtrl'
    return migrate_grab(result)


def migrate_grab(bindings):
    """D08T1: Mouse2 moves from original precision aim to Grab / Manipulate.
    A customized precision-aim input is kept; Grab then takes Mouse2 only if free."""
    if not isinstance(bindings, dict):
        raise ValueError('Invalid action bindings.')
    result = dict(bindings)
    if 'grab' not in result and result.get('original_aim') == 'Mouse2':
        result['original_aim'] = 'Unbound'
    return add_missing_actions(result)


RETIRED = {'mission_browse'}  # D08A5's first Backslash binding, replaced by Comma/Period


def add_missing_actions(bindings):
    """Add newly defined actions. Existing custom bindings win; a new action takes
    its default input when free, otherwise an unused valid input."""
    if not isinstance(bindings, dict):
        raise ValueError('Invalid action bindings.')
    bindings = {action: token for action, token in bindings.items() if action not in RETIRED}
    result = {action: token for action, token in bindings.items() if action in DEFAULTS}
    if len(result) != len(bindings):
        raise ValueError('Unknown action in bindings.')
    # D08A5: the mission inventory takes Comma/Period from original strafe when
    # those still hold them; a customized key stays and the new action is unbound.
    for strafe, browse, key in [('original_strafe_left', 'mission_previous', 'Comma'),
                                ('original_strafe_right', 'mission_next', 'Period')]:
        if browse not in result:
            if result.get(strafe) == key:
                result[strafe] = 'Unbound'
            result[browse] = key if key not in result.values() else 'Unbound'
    for action, token in DEFAULTS.items():
        if action not in result:
            free = next((key for key in [token, *(k for k in TOKENS if k not in DEFAULTS.values()), *TOKENS] if key not in result.values() and key != 'C'), None)
            if free is None: raise ValueError('No free input for new action.')
            result[action]=free
    return validate(result)
