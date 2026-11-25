"""D61 level atlas: read one Time to Kill level from a dump of the game's RAM.

Pure functions (no game, no files except the lamp text), so the reader can be
tested on synthetic memory. The layout is the one the runtime's own readers
use (src/ttk/light_shadows.inc, spawn.inc, level_select.cpp; notes 101, 111,
335, 517). Coordinates are the game's: x and z across, y down (a smaller y is
higher). See documentation/559-d61-level-atlas.md.
"""
import html
import math
import struct
from collections import Counter, defaultdict, deque
from pathlib import Path

BASE = 0x80000000
SIZE = 0x200000
SELECTABLE = [0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 21, 22, 23, 24, 25, 26, 27, 28, 29]
# The title cheat's skipped indices (note 101): 4 and 13 "XXX", 14 animation
# test, 15-20 two-player arenas, 30-31 empty / sound test.
SKIPPED = {4: 'XXX', 13: 'XXX', 14: 'animation test', 15: 'two-player arena', 16: 'two-player arena',
           17: 'two-player arena', 18: 'two-player arena', 19: 'two-player arena', 20: 'two-player arena',
           30: 'empty', 31: 'sound test'}
PLAYER = 0x800D7198
LEVEL_INDEX = 0x800BE570
ROOMS, ROOM_COUNT, ROOM_SIZE = 0x800DA454, 0x800DA47C, 88
TYPES, TYPE_SIZE, TYPE_COUNT = 0x800D2660, 28, 0x7428 // 28
MODELS = 0x800DDB18
SPAWNERS, SPAWNER_COUNT, SPAWNER_SIZE = 0x800DE720, 0x800C56A0, 48
LEVEL_OBJECTS = 0x800D2550           # 64 pointers: warp object N / goto object N
NAME_IDS, STRINGS = 0x800C3D2C, 0x800C60F4
LOCK_FLAG = 0x01000000                # type flag: a lock (note 517)
SECRET_ROOM = 0x4                     # room flags (+0xC): a secret area (FOUND A SECRET AREA; TIME TO KILL's 7 match its HUD S: 0/7)
STAND_HEIGHT = 0x500                  # headroom a standing Duke needs (note 185: 0x370 crouched)
GRID_XZ, GRID_Y = 1 << 10, 1 << 7     # world vertex quantization

# Pickup names (spawn.inc's table; mission slots 153/154 change name per level).
ITEMS = {
    153: 'key 1', 154: 'key 2', 761: 'generic crystal', 1761: 'red energy crystal', 2761: 'blue energy crystal',
    3761: 'green energy crystal', 542: 'skeleton key', 547: 'scrap of paper', 548: 'old note', 549: 'torn paper',
    854: 'family jewel', 855: 'family jewel 2', 856: 'family jewel 3', 638: 'steroids', 320: 'armor',
    473: 'health', 581: 'atomic health', 580: 'medkit', 560: 'biomask', 573: 'goggles', 753: 'jetpack',
    1047: 'invulnerability', 1045: 'invisibility', 1046: 'double duke',
    45: 'throwing knife', 1: 'throwing axe', 21: 'crossbow', 59: 'desert eagle', 66: 'combat shotgun',
    10: 'buffalo rifle', 41: 'gatling gun', 47: 'rpg', 37: 'flamethrower', 34: 'energy weapon', 641: 'freezer',
    58: 'pipe bomb', 608: 'holy hand grenade', 126: 'dynamite',
    609: 'crossbow ammo', 33: 'desert eagle ammo', 65: 'shotgun ammo', 124: 'buffalo rifle ammo',
    562: 'gatling ammo', 106: 'rpg ammo', 38: 'flamethrower ammo', 125: 'energy weapon ammo', 642: 'freezer ammo',
}
LEVEL_ITEMS = {(153, 0): 'subway security key', (154, 0): 'transport room id', (153, 5): 'warehouse key',
               (153, 7): 'gantry key', (154, 7): 'valve key', (153, 9): 'lab key', (154, 9): 'valve room key'}
MISSION = {153, 154, 155, 157, 542, 547, 548, 549, 744, 745, 746, 761, 854, 855, 856, 862, 863}
ENEMIES = {50: 'Necro'}


class Ram:
    def __init__(self, data):
        if len(data) < SIZE:
            data = bytes(data) + bytes(SIZE - len(data))
        self.m = data

    def ok(self, a, n=4):
        return BASE + 0x10000 <= a and a + n <= BASE + SIZE

    def w(self, a):
        return struct.unpack_from('<I', self.m, a - BASE)[0] if self.ok(a) else 0

    def i(self, a):
        return struct.unpack_from('<i', self.m, a - BASE)[0] if self.ok(a) else 0

    def h(self, a):
        return struct.unpack_from('<H', self.m, a - BASE)[0] if self.ok(a, 2) else 0

    def sh(self, a):
        return struct.unpack_from('<h', self.m, a - BASE)[0] if self.ok(a, 2) else 0

    def b(self, a):
        return self.m[a - BASE] if self.ok(a, 1) else 0

    def cstr(self, a, limit=96):
        if not self.ok(a, 1):
            return None
        end = self.m.find(b'\0', a - BASE, a - BASE + limit)
        if end < 0:
            return None
        raw = self.m[a - BASE:end]
        if not raw or any(c < 0x20 or c > 0x7e for c in raw):
            return None
        return raw.decode('ascii')


def read_lamps(path):
    lamps = []
    p = Path(path)
    if not p.is_file():
        return lamps
    for line in p.read_text().splitlines():
        body, _, note = line.partition('#')
        f = body.split()
        if len(f) >= 9:
            try:
                lamps.append(dict(pos=[int(float(f[0])), int(float(f[1])), int(float(f[2]))],
                                  rgb=[float(f[3]), float(f[4]), float(f[5])], radius=float(f[6]),
                                  strength=float(f[7]), style=f[8], note=note.strip()))
            except ValueError:
                pass
    return lamps


def string_table(r, limit=4096):
    """The game's string pointer table: id -> text (printable ASCII only)."""
    out = {}
    misses = 0
    for k in range(limit):
        s = r.cstr(r.w(STRINGS + 4 * k))
        if s is None:
            misses += 1
            if misses > 64:
                break
            continue
        misses = 0
        out[k] = s
    return out


def level_name(r, strings, level):
    sid = r.h(NAME_IDS + 2 * level)
    return strings.get(sid, ''), sid


def room_mesh(r, a):
    """World faces of one room: [(three points)], from its pieces."""
    org = [r.i(a), r.i(a + 4), r.i(a + 8)]
    tris = []
    n, lst = r.b(a + 0x32), r.w(a + 0x40)
    for k in range(n):
        piece = r.w(lst + 4 * k)
        if not r.ok(piece, 0x1c) or piece > 0x801F0000:
            continue
        count = r.h(piece + 0x14)
        if not count or count > 256 or not r.ok(piece + 0x1c, 8 * count):
            continue
        v = []
        for j in range(count):
            w0 = r.w(piece + 0x1c + 8 * j)
            v.append((org[0] + _s16(((w0 & 0xff) << 10) & 0xffff),
                      org[1] + _s16((((w0 >> 8) & 0xff) << 7) & 0xffff),
                      org[2] + _s16((((w0 >> 16) & 0xff) << 10) & 0xffff)))
        cur = piece + r.w(piece + 0x18)
        for _ in range(257):
            hd = r.w(cur)
            cur += 4
            t, m = hd & 0xffff, hd >> 16
            if t == 0xff or (t not in (0x60, 0x61)) or not m or m > 4096 or not r.ok(cur, 8 * m):
                break
            for _k in range(m):
                word = r.w(cur)
                cur += 8
                idx = [(word >> (8 * j)) & 0xff for j in range(3 if t == 0x60 else 4)]
                if any(i >= count for i in idx):
                    continue
                tris.append((v[idx[0]], v[idx[1]], v[idx[2]]))
                if t == 0x61:
                    tris.append((v[idx[1]], v[idx[2]], v[idx[3]]))
    return [t for t in tris if len({t[0], t[1], t[2]}) == 3]


def _s16(x):
    return x - 0x10000 if x >= 0x8000 else x


def _normal(t):
    (ax, ay, az), (bx, by, bz), (cx, cy, cz) = t
    ux, uy, uz = bx - ax, by - ay, bz - az
    vx, vy, vz = cx - ax, cy - ay, cz - az
    nx, ny, nz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx
    ln = math.sqrt(nx * nx + ny * ny + nz * nz) or 1
    return nx / ln, ny / ln, nz / ln, ln / 2


def _inside_xz(t, x, z):
    (ax, _, az), (bx, _, bz), (cx, _, cz) = t
    d1 = (x - bx) * (az - bz) - (ax - bx) * (z - bz)
    d2 = (x - cx) * (bz - cz) - (bx - cx) * (z - cz)
    d3 = (x - ax) * (cz - az) - (cx - ax) * (z - az)
    neg = d1 < 0 or d2 < 0 or d3 < 0
    pos = d1 > 0 or d2 > 0 or d3 > 0
    return not (neg and pos)


def _height_at(t, x, z):
    (ax, ay, az), (bx, by, bz), (cx, cy, cz) = t
    det = (bz - cz) * (ax - cx) + (cx - bx) * (az - cz)
    if not det:
        return (ay + by + cy) / 3
    l1 = ((bz - cz) * (x - cx) + (cx - bx) * (z - cz)) / det
    l2 = ((cz - az) * (x - cx) + (ax - cx) * (z - cz)) / det
    return l1 * ay + l2 * by + (1 - l1 - l2) * cy


def horizontal(tris, slope=0.7):
    """Faces that are floors or ceilings, with the sign of their normal's y."""
    out = []
    for t in tris:
        nx, ny, nz, area = _normal(t)
        if abs(ny) >= slope and area >= 1:
            out.append((t, 1 if ny > 0 else -1, area))
    return out


def stand_surface(flat, x, z, top, bottom):
    """The lowest horizontal face at (x, z) inside [top, bottom] with
    STAND_HEIGHT clear above it (to the face above it or the room's top):
    where play happens. Winding says nothing about floor or ceiling here."""
    ys = sorted({round(_height_at(t, x, z)): a for t, _s, a in flat if _inside_xz(t, x, z)}.items())
    ys = [(y, a) for y, a in ys if top < y <= bottom]
    best = None
    prev = top
    for y, a in ys:
        if y - prev >= STAND_HEIGHT:
            best = (y, a, int(y - prev))
        prev = y
    return best


def standing_points(flat, box, limit=3, spacing=2048):
    lo, hi = box
    cands = []
    step = GRID_XZ
    x = lo[0] + step / 2
    while x < hi[0]:
        z = lo[2] + step / 2
        while z < hi[2]:
            hit = stand_surface(flat, x, z, lo[1], hi[1])
            if hit:
                y, area, head = hit
                if True:
                    cx, cz = (lo[0] + hi[0]) / 2, (lo[2] + hi[2]) / 2
                    cands.append((-(area) + 0.05 * ((x - cx) ** 2 + (z - cz) ** 2) ** 0.5, x, y, z, head, area))
            z += step
        x += step
    cands.sort()
    pts = []
    for _score, x, y, z, head, area in cands:
        if any((x - p['pos'][0]) ** 2 + (z - p['pos'][2]) ** 2 < spacing ** 2 for p in pts):
            continue
        pts.append(dict(pos=[int(round(x)), int(round(y)), int(round(z))], area=int(area), headroom=head))
        if len(pts) >= limit:
            break
    return pts


def read_level(data, lamps=()):
    r = Ram(data)
    strings = string_table(r)
    level = r.w(LEVEL_INDEX)
    name, name_id = level_name(r, strings, level)
    duke = [r.i(PLAYER + 4), r.i(PLAYER + 8), r.i(PLAYER + 12)]
    duke_feet = r.i(PLAYER + 0x1c8)
    duke_room = r.b(PLAYER + 0x2e)
    table, count = r.w(ROOMS), r.w(ROOM_COUNT)
    if not (0 < count <= 256 and r.ok(table, count * ROOM_SIZE)):
        raise ValueError(f'no room table (table {table:#x}, count {count})')
    types = r.w(TYPES)
    models = r.w(MODELS)

    def tinfo(t):
        a = types + TYPE_SIZE * t
        return r.w(a), r.b(a + 6), r.h(a + 16)

    def model_loaded(t):
        if not (0 < t < TYPE_COUNT):
            return False
        flags, _c, model = tinfo(t)
        if not flags & 2:
            return None
        return r.ok(r.w(models + 4 * model))

    rooms, flat, objects, seen = [], {}, [], {}
    for k in range(count):
        a = table + ROOM_SIZE * k
        lo = [r.i(a), r.i(a + 4), r.i(a + 8)]
        hi = [r.i(a + 0x20), r.i(a + 0x24), r.i(a + 0x28)]
        pc, pl = r.b(a + 0x30), r.w(a + 0x38)
        portals = []
        for p in range(pc if r.ok(pl, 36 * pc) else 0):
            q = pl + 36 * p
            corners = [[lo[c] + r.sh(q + 8 * j + 2 * c) for c in range(3)] for j in range(4)]   # room-relative in memory
            portals.append(dict(to=r.sh(q + 14), flags=r.sh(q + 6), corners=corners))
        tris = room_mesh(r, a)
        flat[k] = horizontal(tris)
        rooms.append(dict(index=k, min=lo, max=hi, flags=r.w(a + 0xC), secret=bool(r.w(a + 0xC) & SECRET_ROOM),
                          portals=portals, faces=len(tris),
                          pieces=r.b(a + 0x32)))
        for head in (0x48, 0x4c):
            node = r.w(a + head)
            for _ in range(2048):
                if not node or not r.ok(node, 12):
                    break
                o, node = r.w(node + 8), r.w(node + 4)
                if o in seen or not r.ok(o, 0x60):
                    continue
                seen[o] = k
                t = r.h(o + 0x2c)
                objects.append(dict(addr=o, room=k, list=head, type=t, kind=r.b(o + 0x14), sub=r.b(o + 0x34),
                                    dropped=r.b(o + 0x35), contents=r.h(o + 0x36), yaw=r.sh(o + 0x1c) & 4095,
                                    pos=[r.i(o + 4), r.i(o + 8), r.i(o + 12)]))
    level_objects = [r.w(LEVEL_OBJECTS + 4 * n) for n in range(64)]
    for ob in objects:
        t = ob['type']
        flags, cls, _m = tinfo(t) if 0 < t < TYPE_COUNT else (0, 0, 0)
        ob.update(type_flags=flags, lock=bool(flags & LOCK_FLAG), model=model_loaded(t),
                  name=LEVEL_ITEMS.get((t, level)) or ITEMS.get(t) or ENEMIES.get(t),
                  table=level_objects.index(ob['addr']) if ob['addr'] in level_objects else None)
        ob['addr'] = f"{ob['addr']:08x}"
        ob['outside'] = not any(all(rm['min'][i] <= ob['pos'][i] <= rm['max'][i] for i in range(3)) for rm in rooms)

    spawners = []
    st, sn = r.w(SPAWNERS), r.w(SPAWNER_COUNT)
    for k in range(sn if 0 < sn <= 512 and r.ok(st, sn * SPAWNER_SIZE) else 0):
        q = st + SPAWNER_SIZE * k
        t = r.w(q)
        if not t:
            continue
        flags, cls, _m = tinfo(t) if 0 < t < TYPE_COUNT else (0, 0, 0)
        pos = [r.i(q + 8), r.i(q + 12), r.i(q + 16)]
        spawners.append(dict(index=k, type=t, creature_class=cls, flags=r.w(q + 0x14), contents=r.h(q + 0x2c),
                             live=r.w(q + 4) != 0, model=model_loaded(t), pos=pos, name=ENEMIES.get(t),
                             room=_room_of(rooms, pos)))

    for rm in rooms:
        rm['stand'] = standing_points(flat[rm['index']], (rm['min'], rm['max']))
        rm['levels'] = sorted({int(round(sum(p[1] for p in t) / 3)) for t, _s, _a in flat[rm['index']]})[:24]

    # Model use: every model this level loaded, and the types that use one.
    used_types = {o['type'] for o in objects} | {s['type'] for s in spawners} | \
        {o['contents'] for o in objects if o['contents']} | {s['contents'] for s in spawners if s['contents']}
    by_model = defaultdict(list)
    for t in range(1, TYPE_COUNT):
        flags, cls, model = tinfo(t)
        if flags & 2 and r.ok(r.w(models + 4 * model)):
            by_model[model].append(t)
    used_models = {tinfo(t)[2] for t in used_types if 0 < t < TYPE_COUNT and tinfo(t)[0] & 2}

    a = dict(level=level, name=name, name_id=name_id, duke=duke, duke_feet=duke_feet, duke_room=duke_room,
             rooms=rooms, objects=objects, spawners=spawners, lamps=list(lamps),
             level_objects=[f'{x:08x}' if x else None for x in level_objects],
             models={m: ts for m, ts in sorted(by_model.items())}, used_models=sorted(used_models))
    a['reach'] = reachability(rooms, duke_room)
    a['flags'] = cut_flags(a)
    return a


def _room_of(rooms, p):
    best, vol = None, None
    for rm in rooms:
        if all(rm['min'][i] <= p[i] <= rm['max'][i] for i in range(3)):
            v = 1
            for i in range(3):
                v *= rm['max'][i] - rm['min'][i] + 1
            if vol is None or v < vol:
                best, vol = rm['index'], v
    return best


def reachability(rooms, start):
    """Rooms the portal graph joins to Duke's start room (portals both ways)."""
    adj = defaultdict(set)
    n = len(rooms)
    for rm in rooms:
        for p in rm['portals']:
            if 0 <= p['to'] < n and p['to'] != rm['index']:
                adj[rm['index']].add(p['to'])
                adj[p['to']].add(rm['index'])
    seen = {start} if 0 <= start < n else set()
    q = deque(seen)
    while q:
        k = q.popleft()
        for j in adj[k]:
            if j not in seen:
                seen.add(j)
                q.append(j)
    groups = []
    rest = set(range(n)) - seen
    while rest:
        k = min(rest)
        g = {k}
        q = deque([k])
        while q:
            x = q.popleft()
            for j in adj[x]:
                if j not in g:
                    g.add(j)
                    q.append(j)
        groups.append(sorted(g))
        rest -= g
    return dict(start=start, joined=sorted(seen), islands=groups)


def cut_flags(a):
    """Leads for cut or unused content. Each is a lead, not a find: the
    tour and the original game decide (documentation/559)."""
    out = []
    for g in a['reach']['islands']:
        objs = [o for o in a['objects'] if o['room'] in g]
        out.append(dict(kind='island', rooms=g, objects=len(objs),
                        text=f"rooms {g} share no portal path with the start room ({len(objs)} objects)"))
    for o in a['objects']:
        if o['outside']:
            out.append(dict(kind='outside', object=o['addr'], type=o['type'],
                            text=f"object {o['addr']} type {o['type']} {o['name'] or ''} lies outside every room at {o['pos']}"))
    for s in a['spawners']:
        if s['model'] is False:
            out.append(dict(kind='spawner-no-model', spawner=s['index'], type=s['type'],
                            text=f"spawner {s['index']} type {s['type']} has no model loaded (can never appear)"))
    locks = [o for o in a['objects'] if o['lock']]
    keys = [o for o in a['objects'] if o['type'] in MISSION or o['contents'] in MISSION] + \
        [s for s in a['spawners'] if s['contents'] in MISSION]
    if locks and not keys:
        out.append(dict(kind='lock-without-key', locks=[o['addr'] for o in locks],
                        text=f"{len(locks)} lock objects but no mission item placed or carried"))
    return out


OVERLAY = 0x800CA968     # the level overlay's load address (note 101)


def code_refs(data, values, lo=0x80010000, hi=0x800E0000):
    """Instructions that load a constant: addiu/ori rt, zero, imm (li rt, imm).
    -> {value: {'exe': n, 'overlay': n}}. A type the code never names is not
    created at run time by an immediate (it may still come from a table)."""
    want = set(values)
    out = {v: {'exe': 0, 'overlay': 0} for v in want}
    for a in range(lo, hi, 4):
        w = struct.unpack_from('<I', data, a - BASE)[0]
        op, rs, imm = w >> 26, (w >> 21) & 31, w & 0xffff
        if op in (0x09, 0x0d) and rs == 0 and imm in want:
            out[imm]['overlay' if a >= OVERLAY else 'exe'] += 1
    return out


def cross_flags(atlases, rare=4, dumps=None):
    """Models a level loads, that nothing it places uses, and that at most
    `rare` levels load at all: a level-specific model with no user (the
    generic pickups and Duke's weapons, loaded everywhere, are left out)."""
    loads = Counter()
    for a in atlases.values():
        loads.update(a['models'].keys())
    for n, a in atlases.items():
        used = set(a['used_models'])
        cands = [(m, ts) for m, ts in a['models'].items() if m not in used and loads[m] <= rare]
        refs = code_refs(dumps[n], {t for _m, ts in cands for t in ts}) if dumps and n in dumps else {}
        for m, ts in cands:
            names = [ITEMS.get(t) for t in ts if ITEMS.get(t)]
            r = {t: refs[t] for t in ts if t in refs}
            named = sum(v['exe'] + v['overlay'] for v in r.values())
            a['flags'].append(dict(kind='unused-model' if named else 'unused-model-unnamed', model=m, types=ts[:12],
                                   levels=loads[m], code_refs=r,
                                   text=f"model {m} (types {ts[:12]}{' ' + ', '.join(names) if names else ''}) "
                                        f"is loaded here and in {loads[m] - 1} other level(s), nothing placed uses it, "
                                        + (f"code loads the type {named} time(s)" if r else "code refs not checked")
                                        if named or not r else
                                        f"model {m} (types {ts[:12]}{' ' + ', '.join(names) if names else ''}) "
                                        f"is loaded here and in {loads[m] - 1} other level(s), nothing placed uses it "
                                        f"and no instruction loads its type number"))


def summary(a):
    return dict(level=a['level'], name=a['name'], rooms=len(a['rooms']), objects=len(a['objects']),
                spawners=len(a['spawners']), lamps=len(a['lamps']),
                stand=sum(len(r['stand']) for r in a['rooms']),
                secrets=[r['index'] for r in a['rooms'] if r.get('secret')],
                islands=len(a['reach']['islands']), flags=Counter(f['kind'] for f in a['flags']))


def findings_md(atlases):
    lines = ['# Level atlas findings (local, generated)', '',
             'Leads for cut or unused content, per level. Each is checked by visiting',
             'it (atlas.py shot / tour) and on the original game before it is called a find.', '']
    for n, a in sorted(atlases.items()):
        lines.append(f"## Level {n} {a['name']}")
        lines.append('')
        if not a['flags']:
            lines.append('- no leads')
        for f in a['flags']:
            lines.append(f"- {f['kind']}: {f['text']}")
        lines.append('')
    return '\n'.join(lines)


# ---- top-down plan -------------------------------------------------------

def _ramp(t):
    t = max(0.0, min(1.0, t))
    stops = [(0.0, (38, 70, 140)), (0.5, (60, 150, 130)), (1.0, (230, 200, 90))]
    for (t0, c0), (t1, c1) in zip(stops, stops[1:]):
        if t <= t1:
            f = (t - t0) / (t1 - t0)
            return '#%02x%02x%02x' % tuple(int(c0[i] + (c1[i] - c0[i]) * f) for i in range(3))
    return '#e6c85a'


def plan_svg(a, width=1600, floors=None):
    rooms = a['rooms']
    xs = [v for rm in rooms for v in (rm['min'][0], rm['max'][0])]
    zs = [v for rm in rooms for v in (rm['min'][2], rm['max'][2])]
    if not xs:
        return '<svg xmlns="http://www.w3.org/2000/svg"/>'
    x0, x1, z0, z1 = min(xs), max(xs), min(zs), max(zs)
    pad = 40
    s = (width - 2 * pad) / max(1, x1 - x0)
    height = int((z1 - z0) * s + 2 * pad + 60)

    def P(x, z):
        return pad + (x - x0) * s, pad + (z1 - z) * s     # +z up the page

    ys = [p['pos'][1] for rm in rooms for p in rm['stand']] or [0]
    ylo, yhi = min(ys), max(ys)
    out = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" font-family="monospace">',
           f'<rect width="{width}" height="{height}" fill="#101418"/>']
    islands = {k for g in a['reach']['islands'] for k in g}
    for rm in rooms:
        ax, az = P(rm['min'][0], rm['max'][2])
        bx, bz = P(rm['max'][0], rm['min'][2])
        col = '#d9533b' if rm['index'] in islands else '#ffd23c' if rm.get('secret') else '#5d6b7a'
        out.append(f'<rect x="{ax:.1f}" y="{az:.1f}" width="{bx - ax:.1f}" height="{bz - az:.1f}" fill="none" '
                   f'stroke="{col}" stroke-width="1" opacity="0.8"><title>room {rm["index"]}</title></rect>')
    for rm in rooms:
        for p in rm['stand']:
            x, z = P(p['pos'][0], p['pos'][2])
            c = _ramp((yhi - p['pos'][1]) / max(1, yhi - ylo))
            out.append(f'<circle cx="{x:.1f}" cy="{z:.1f}" r="7" fill="{c}" stroke="#0b0e11" stroke-width="1.5"><title>room {rm["index"]} '
                       f'stand {p["pos"]} headroom {p["headroom"]}</title></circle>')
        if rm['stand']:
            x, z = P(rm['stand'][0]['pos'][0], rm['stand'][0]['pos'][2])
            out.append(f'<text x="{x + 6:.1f}" y="{z - 6:.1f}" font-size="11" fill="#c8d2dc">{rm["index"]}</text>')
    for o in a['objects']:
        x, z = P(o['pos'][0], o['pos'][2])
        if o['lock']:
            c = '#e05ad2'
        elif o['type'] in MISSION:
            c = '#ffd23c'
        elif o['name']:
            c = '#5ad27a'
        else:
            c = '#8a96a3'
        label = html.escape(f"{o['type']} {o['name'] or ''} room {o['room']} table {o['table']}")
        out.append(f'<rect x="{x - 2:.1f}" y="{z - 2:.1f}" width="4" height="4" fill="{c}"><title>{label}</title></rect>')
    for sp in a['spawners']:
        x, z = P(sp['pos'][0], sp['pos'][2])
        out.append(f'<path d="M{x:.1f} {z - 5:.1f}L{x + 5:.1f} {z + 4:.1f}L{x - 5:.1f} {z + 4:.1f}Z" fill="#f08a3c">'
                   f'<title>spawner {sp["index"]} type {sp["type"]}</title></path>')
    dx, dz = P(a['duke'][0], a['duke'][2])
    out.append(f'<circle cx="{dx:.1f}" cy="{dz:.1f}" r="8" fill="none" stroke="#ffffff" stroke-width="2"/>')
    ty = height - 22
    out.append(f'<text x="{pad}" y="{ty}" font-size="16" fill="#e6edf3">Level {a["level"]} {html.escape(a["name"])} - '
               f'{len(rooms)} rooms, {len(a["objects"])} objects, {len(a["spawners"])} spawners '
               f'(dots: standing points, blue low to yellow high; yellow boxes: secret areas; red boxes: islands; white ring: start)</text>')
    out.append('</svg>')
    return '\n'.join(out)


def contact_sheet(a, results, folder, views):
    rows = []
    for r in results:
        cells = ''.join(f'<img src="{html.escape(s)}" loading="lazy">' for s in r['shots'])
        warn = f'<b>{html.escape("; ".join(r["hazard"]))}</b>' if r.get('hazard') else ''
        rows.append(f'<section><h2>room {r["room"]}.{r.get("point", 0)} - {html.escape(r.get("target", ""))}</h2>'
                    f'<p>{html.escape(r["status"])} {warn}</p><div>{cells}</div></section>')
    page = f"""<!doctype html><meta charset="utf-8"><title>Level {a['level']} tour</title>
<style>body{{background:#111;color:#ddd;font:14px monospace;margin:16px}}section{{margin:0 0 18px}}
div{{display:grid;grid-template-columns:repeat({views},1fr);gap:4px}}img{{width:100%}}h2{{margin:4px 0;font-size:15px}}
p{{margin:0 0 4px;color:#999}}b{{color:#ff7a5c}}</style>
<h1>Level {a['level']} {html.escape(a['name'])}: {len(results)} stops</h1>
<p><a href="../../level{a['level']:02d}.svg">plan</a></p>
{''.join(rows)}"""
    (Path(folder) / 'sheet.html').write_text(page)
