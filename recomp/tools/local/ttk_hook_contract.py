"""Fail-closed call-context checks for D03 read-only hook observations.

This is not a guest mutation API. Call sample() immediately before arming
traces, and again afterwards; these checks qualify trace ownership only.
Runtime movement/camera replacements must validate identity at invocation.
"""
from ttk_state_probe import CAMERA, PLAYER

# target: (permitted return addresses, required argument registers)
CONTEXTS = {
    0x80053500: ({0x80048664, 0x800486a8}, {'a0': PLAYER}),
    0x8007926c: ({0x80053548}, {'a0': PLAYER, 'a1': 4}),
    0x800598f0: ({0x8005a3e8}, {'a0': PLAYER+0x60, 'a1': PLAYER}),
    0x80057230: ({0x80041c3c}, {'a0': PLAYER}),
    0x8003aa48: ({0x8003aeb0}, {'a0': CAMERA}),
    0x8003ade4: ({0x80025ee8}, {'a0': CAMERA, 'a1': PLAYER}),
}


def qualifies(entry):
    """Reject other actors, callers, targets and malformed trace records."""
    try:
        number = lambda key: int(entry[key], 16)
        callers, arguments = CONTEXTS[number('target')]
        if number('target') == 0x8003aa48:
            # Caller passes its stack-local signed-word XYZ delta at sp+0x18.
            if not (0x80010000 <= number('sp') <= 0x801fffdc and
                    number('sp') % 4 == 0 and number('a1') == number('sp') + 0x18):
                return False
        return number('ra') in callers and all(number(k) == v for k, v in arguments.items())
    except (KeyError, TypeError, ValueError):
        return False
