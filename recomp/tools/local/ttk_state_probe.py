"""Read-only SLUS-00583 state research. Candidate fields, never a gameplay hook."""
import hashlib
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
EXE_SHA256 = 'b5c3ba610074bff184f089a49e51a22a35455cfef08757bd673a54f4057d5a7a'
# Seed-bounded code ranges; a range may include additional unseeded routines.
GUARDS = ((0x8003accc, 0x8003ade4), (0x8003ade4, 0x8003b474),
          (0x80097c04, 0x80098204), (0x80053500, 0x800539e0),
          (0x80057230, 0x80057b90), (0x80056770, 0x8005690c),
          (0x800598f0, 0x80059db0), (0x8007926c, 0x800797f8),
          (0x80039c7c, 0x80039dd0), (0x8003aa48, 0x8003accc),
          (0x80025e80, 0x80025f10), (0x80048640, 0x800486b0))
LEVEL00_LOAD = 0x800ca968
# Code and tables only: the file's final 16 bytes (0x800ccf1c..) are a scratch
# hit-position vector the LEVEL00 zone script writes during play.
LEVEL00_SIZE = 9652
LEVEL00_SHA256 = '274d71ddd8aeb6e1ca12c0229eea5087c7750a39f442785b94f0efeec81a25b3'
CAMERA = 0x800d6eb0
PLAYER = 0x800d7198


def sample(call):
    exe = (ROOT / 'disc/SLUS_005.83').read_bytes()
    if hashlib.sha256(exe).hexdigest() != EXE_SHA256:
        raise ValueError('unsupported disc executable; state probe refused')
    first_frame = call('frame')['frame']
    guards = []
    for start, end in GUARDS:
        expected = exe[2048 + start - 0x80010000:2048 + end - 0x80010000]
        actual = bytes.fromhex(call('read_ram', addr=hex(start), len=end-start)['hex'])
        if actual != expected:
            raise ValueError(f'live code identity mismatch at {start:#x}; state probe refused')
        guards.append({'start': hex(start), 'end_exclusive': hex(end),
                       'sha256': hashlib.sha256(actual).hexdigest()})
    overlay = bytes.fromhex(call('read_ram', addr=hex(LEVEL00_LOAD), len=LEVEL00_SIZE)['hex'])
    if hashlib.sha256(overlay).hexdigest() != LEVEL00_SHA256:
        raise ValueError('LEVEL00 overlay identity mismatch; first-map state probe refused')
    data = bytes.fromhex(call('read_ram', addr=hex(CAMERA), len=PLAYER + 0x8a4 - CAMERA)['hex'])
    offset = PLAYER - CAMERA
    word = lambda at: struct.unpack_from('<I', data, at)[0]
    if word(0xa4) != PLAYER or word(offset + 0x7d4) != CAMERA:
        raise ValueError('camera/player back-reference mismatch; no state interpretation performed')
    return {'frame_before': first_frame, 'frame_after': call('frame')['frame'],
            'code_guards': guards, 'overlay_guard': {'address': hex(LEVEL00_LOAD), 'size': LEVEL00_SIZE, 'sha256': LEVEL00_SHA256}, 'player_base': hex(PLAYER), 'camera_base': hex(CAMERA),
            'player_position_candidate': struct.unpack_from('<iii', data, offset+4),
            'player_rotation_candidate': struct.unpack_from('<hhh', data, offset+0x1c),
            'player_target_rotation_candidate': struct.unpack_from('<hhh', data, offset+0x24),
            'player_delta_candidate': struct.unpack_from('<hhh', data, offset+0xfc),
            'player_flags': word(offset),
            'player_flags_224': word(offset+0x224),
            'equipment_state_3b8': data[offset+0x3b8],
            'weapon_slot_3b9': data[offset+0x3b9],
            'upper_animation_74': struct.unpack_from('<h', data, offset+0x74)[0],
            'player_animation': struct.unpack_from('<h', data, offset+0x60)[0],
            'player_turn_increment': struct.unpack_from('<h', data, offset+0x258)[0],
            'camera_matrix_q12': struct.unpack_from('<9h', data),
            'player_room_candidate': struct.unpack_from('<b', data, offset+0x2e)[0],
            'player_camera_anchor_candidate': struct.unpack_from('<iii', data, offset+0x7bc),
            'camera_14_vector': struct.unpack_from('<iii', data, 0x14),
            'camera_64_vector': struct.unpack_from('<iii', data, 0x64),
            'state_bytes_22c_233': list(data[offset+0x22c:offset+0x234]),
            'raw_hex': data.hex(),
            'interpretation': 'Read-only candidates. Units, axis conventions, collision and weapon aim are not yet verified. This does not authorize state writes.'}
