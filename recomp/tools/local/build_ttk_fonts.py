#!/usr/bin/env python3
"""D24A: build the Modernized text glyph pack from Time to Kill's own fonts.

Replaces the Duke Nukem 3D message/Atomic glyph pack (build_duke_fonts.py). All
glyph pixels come from the player's own disc at build time; nothing is tracked.

Sources (owned USA SLUS-00583, read only, hashes pinned):
  /DATA/FONTS.RAW  64x256 16-bit VRAM image at (960, 0): TTK Big Italic (17 px,
                   rows 0..70), TTK Medium Italic (11 px, rows 72..106) and the
                   16-colour CLUTs in column x = 1008.
  SLUS_005.83      the PSY-Q system font (FntPrint) at 0x800c7d70, 128x32,
                   ASCII 0x20..0x5f; the fallback for characters the italic
                   fonts lack (& + and others; D24C draws its own !,
                   D24D ( ), D24E % and D19T the double quotes).
Palettes: disc CLUT 225 (gold, the pause-menu text) and "Console steel", the
project's own palette matching the old console font.

Choices (user, 2026-10-06, D24A font picker; sets 3-7 from the approved D24B
savestate panel mockup):
  set 0 messages  (styles 0 and 2): TTK Big Italic, Console steel, 1x, spacing 1, drop shadow, all caps
  set 1 headings  (style 1):        TTK Big Italic, Gold (CLUT 225), 2x, spacing 1, all caps
  set 2 console   (style 3):        TTK Medium Italic, Console steel, 1x, spacing 0, drop shadow, all caps
Missing glyphs fall back to the system 8x8 font in the set's palette, except
'!' (D24C): the italic fonts have none, so one is drawn from their own 'I' (a
stem cut short and tapered, closed with the I's bottom edge) and '.' (top rows
trimmed so the stem stays the taller part); '(' and ')' (D24D), the two sides of
the font's own 'O'; and '%' (D24E), './.' from its own '/' and two trimmed
periods; and the double quotes (D19T), two of its own apostrophes: '"' the
closing quote, glyph 127 the opening one (the pair turned half round; '`'
before D26L). The text renderer draws every other '"' of a string as glyph
127 (ttk_font.cpp).

It also writes ttk-ui.pack (TTKUI1): the disc's button sprites and radiation
emblem for the D24B savestate panel.

D26L: every other printable character the fonts lack (# $ & * + [ backslash ] ^ _ { | }
~ and a real backquote) is built in each italic font's style by full_ascii();
the 8x8 font gets real [ ] _ and ` { | } ~; nothing falls back any more.

Pack TTKFONT3 (little endian): 'TTKFONT3', u32 payload size, u32 FNV-1a of the
payload. Payload: u32 set count; per set 8 bytes (u8 line height, u8 scale,
u8 shadow, u8 0, u32 shadow ARGB) and 96 glyph records for ASCII 32..126 and
127 (the opening double quote)
(u8 w, u8 h, u8 advance, u8 y, u32 pixel offset in the payload); then ARGB
pixels (u32, 0 = transparent).
"""
import argparse, hashlib, json, struct, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from disc_lab import Disc  # noqa: E402

ROOT = HERE.parents[1]
FONTS_PATH = '/DATA/FONTS.RAW;1'
FONTS_SHA256 = '54e16c1af6b63edcc5a7d92eefe45220b9921ebd0a41fa8047c92ebb5f18e1be'
SYSTEM_FONT = 0x800c7d70
EXE_LOAD = 0x80010000
STEEL = ['000000', '0c0c18', '14182c', '202840', '405080', '5c70a0', '7088b4', '84a4cc', '94b0d8',
         'a4c4e4', 'b4d4f4', 'bcdcfc', 'd0e6fc', 'e4f0ff', 'f4faff', '000000']
SHADOW = 0xff0c0c18  # Console steel's darkest navy
# D43: the level title's red (Duke 3D's level name), the steel ramp's steps in red.
RED = ['000000', '1c0000', '300000', '440404', '600808', '780c0c', '901010', 'a81414', 'bc1818', 'cc2020',
       'dc2c2c', 'e83c3c', 'f45050', 'fc6c6c', 'fc8c8c', '000000']
# D20I: LEVEL STATS, matched to EDuke32's level stats as measured on its 720p
# frame (clean room: colours read off the screen): the letters' orange-red
# (#8c200c..#d83414) and the values' light gold-beige (#986444..#ecd8b0), each
# with a dark outline (step 1) in its own hue. Medium Italic uses steps 1 and 4-13.
STATS_RED = ['000000', '280400', '180400', '3c0800', '601404', '78180a', '8c200c', '9c240c', 'ac280c', 'b82c10',
             'c83010', 'd83414', 'e8442a', 'f45c40', '000000', '000000']
STATS_BEIGE = ['000000', '28140c', '180c04', '3c2014', '6c4428', '805234', '986444', 'a87450', 'bc8e66', 'ccac80',
               'dcc298', 'ecd8b0', 'f4e4c4', 'fcf0dc', '000000', '000000']

BIG_ROWS = [(0, ',-./0123456789A'), (18, 'BCDEFGHIJKLM'), (36, 'NOPQRSTUVWXY'), (54, "Z:;<>=?@'")]
MEDIUM_ROWS = [(72, ',-./0123456789ABCDEFG'), (84, 'HIJKLMNOPQRSTUVWXY'), (96, "Z:;<>=?@©™'")]
SETS = [  # name, font, palette or '#rrggbb' tint (system font), scale, spacing, shadow (True: SHADOW, or its ARGB)
    ('messages', 'big', 'steel', 1, 1, True),
    ('headings', 'big', 225, 2, 1, False),
    ('console', 'medium', 'steel', 1, 0, True),
    # D24B savestate panel (approved mockup): title, slot titles, selected slot, details.
    ('panel_title', 'big', 225, 1, 1, True),
    ('panel_slot', 'medium', 225, 1, 1, True),
    ('panel_slot_selected', 'medium', 226, 1, 1, True),
    ('panel_text', 'system', '#dedede', 1, 0, False),
    ('panel_dim', 'system', '#848484', 1, 0, False),
    # D08A5 mission row and item card (approved design E): FOUND, then the 2x
    # Microfont "MISSION" label and found/total count (all found: green).
    ('mission_found', 'system', '#5fd35f', 1, 0, False),
    ('mission_label', 'micro', '#848484', 2, 1, False),
    ('mission_count', 'micro', '#989c58', 2, 1, False),
    ('mission_complete', 'micro', '#5fd35f', 2, 1, False),
    # D08A19: USED on the item card, in the gold of the user's mockup.
    ('mission_used', 'system', '#ffdc30', 1, 0, False),
    # D43: the level title card (user, 2026-11-03): TTK Big Italic, 1x, spacing -1,
    # in red over a near-black shadow; drawn in the game's screen pixels like the menu.
    ('level_title', 'big', 'red', 1, -1, 0xff140000),
    # D20I LEVEL STATS: TTK Medium Italic, 1x, in EDuke32's colours: the letters
    # (and a finished count) red, the values beige, each with a shadow in its hue.
    ('level_stats_label', 'medium', 'stats_red', 1, 0, 0xff180400),
    ('level_stats_value', 'medium', 'stats_beige', 1, 0, 0xff180c04),
]
FONT_INFO = {'big': (BIG_ROWS, 17, 8), 'medium': (MEDIUM_ROWS, 11, 5), 'system': (None, 8, 8), 'micro': (None, 5, 4)}  # rows, height, space
EXCLAMATION = {'big': (2, 3), 'medium': (1, 2)}  # D24C: period rows trimmed, stem taper (px)
QUOTE_STEP = {'big': 6, 'medium': 5}  # D19T: px between the two apostrophes (as the menu font's)
MICROFONT = ROOT / 'assets/ui/fonts/microfont/3x5-Microfont_1D.png'  # CC0, tracked
GLYPHS = 96  # D26L: ASCII 32..126, then 127 for the opening double quote
OPEN_QUOTE = 'open"'  # D19T's opening quote (glyph 127; '`' was its slot before D26L)


class Sheet:
    def __init__(self, raw):
        self.raw = raw

    def word(self, vx, vy):
        return struct.unpack_from('<H', self.raw, (vy * 64 + (vx - 960)) * 2)[0]

    def texel(self, px, py):
        return (self.word(960 + px // 4, py) >> (4 * (px % 4))) & 15

    def segments(self, y0, y1):
        cols = [any(self.texel(x, y) for y in range(y0, y1 + 1)) for x in range(256)]
        out, start = [], None
        for i, lit in enumerate(cols + [False]):
            if lit and start is None:
                start = i
            if not lit and start is not None:
                out.append((start, i - 1)); start = None
        return out

    def font(self, rows, height):
        glyphs = {}
        for y0, chars in rows:
            segs = self.segments(y0, y0 + height - 1)
            if len(segs) != len(chars):
                raise SystemExit(f'FONTS.RAW row {y0}: {len(segs)} glyphs for {len(chars)} characters')
            for ch, (a, b) in zip(chars, segs):
                glyphs[ch] = (b - a + 1, height, [self.texel(x, y) for y in range(y0, y0 + height) for x in range(a, b + 1)])
        return glyphs

    def palette(self, clut_y):
        out = []
        for k in range(16):
            v = self.word(1008 + k, clut_y)
            r, g, b = v & 31, (v >> 5) & 31, (v >> 10) & 31
            out.append((r * 255 // 31, g * 255 // 31, b * 255 // 31))
        return out


def exclamation(glyphs, trim, taper):
    """D24C: '!' from the font's own 'I' and '.', both on the italic slant.

    The stem is the I's top, narrowing by `taper` px from the right towards the
    bottom, closed by the I's own bottom edge rows moved up along the slant. The
    dot is the period minus its top `trim` rows, kept on the baseline, with one
    clear row above it."""
    w, h, stem = glyphs['I']
    pw, _, dot = glyphs['.']
    rows = [list(stem[y * w:(y + 1) * w]) for y in range(h)]
    period = [dot[y * pw:(y + 1) * pw] for y in range(h)]
    lit = [y for y in range(h) if any(period[y])]
    dot_rows = period[lit[0] + trim:lit[-1] + 1]
    top = lit[-1] + 1 - len(dot_rows)
    edge = 2 if h >= 17 else 1  # the I's dark bottom edge
    last = max(y for y in range(h) if any(rows[y]))
    cut = top - 1 - edge
    left = lambda r: next(x for x, v in enumerate(r) if v)
    right = lambda r: max(x for x, v in enumerate(r) if v)
    out = [[0] * w for _ in range(h)]
    for y in range(cut):
        r, n = rows[y], round(taper * y / (cut - 1))
        if n:
            end = right(r); r[end - n] = r[end]
            for x in range(end - n + 1, end + 1):
                r[x] = 0
        out[y] = r
    shift, limit = left(out[cut - 1]) - left(rows[last - edge + 1]), right(out[cut - 1])
    for k in range(edge):
        for x, v in enumerate(rows[last - edge + 1 + k]):
            if v and 0 <= x + shift <= min(w - 1, limit + k):
                out[cut + k][x + shift] = v
    for i, r in enumerate(dot_rows):
        for x, v in enumerate(r):
            if v:
                out[top + i][x] = v
    return w, h, [v for r in out for v in r]


def crop(rows):
    """Trim clear columns from a list of rows; returns (w, h, texels)."""
    cols = [x for x in range(len(rows[0])) if any(r[x] for r in rows)]
    a, b = cols[0], cols[-1]
    return b - a + 1, len(rows), [v for r in rows for v in r[a:b + 1]]


def parenthesis(glyphs, right):
    """D24D: '(' or ')' from the font's own 'O': the stroke on that side of the
    hole, and on the closed top and bottom rows the half of the bar on that side
    of the row's own centre (so the halves follow the italic slant)."""
    w, h, texels = glyphs['O']
    out = []
    for y in range(h):
        r, keep = texels[y * w:(y + 1) * w], [0] * w
        runs, start = [], None
        for x, v in enumerate(list(r) + [0]):
            if v and start is None:
                start = x
            if not v and start is not None:
                runs.append((start, x - 1)); start = None
        if len(runs) >= 2:
            a, b = runs[-1] if right else runs[0]
        elif runs:
            a, b = runs[0][0], runs[-1][1]
            mid = (a + b) // 2
            a, b = (mid + 1, b) if right else (a, mid)
        else:
            a, b = 0, -1
        keep[a:b + 1] = r[a:b + 1]
        out.append(keep)
    return crop(out)


def percent(glyphs, trim):
    """D24E: '%' as './.' from the font's own '/' and '.': one period (minus its
    top `trim` rows, as D24C's '!') at the top left of the slash, one on the
    baseline at the right, each one clear pixel from the slash on every row."""
    sw, h, slash = glyphs['/']
    pw, _, dot = glyphs['.']
    period = [dot[y * pw:(y + 1) * pw] for y in range(h)]
    lit = [y for y in range(h) if any(period[y])]
    dot_rows = period[lit[0] + trim:lit[-1] + 1]
    dw, dh, dot_px = crop(dot_rows)
    dot_rows = [dot_px[y * dw:(y + 1) * dw] for y in range(dh)]
    rows = [slash[y * sw:(y + 1) * sw] for y in range(h)]
    span = lambda y: [x for x, v in enumerate(rows[y]) if v]
    top = min(y for y in range(h) if span(y))
    base = lit[-1] + 1 - dh
    # Leftmost x for the bottom dot / rightmost x for the top dot, 1 px clear.
    upper = min(min(span(top + i)) - 2 - max(x for x, v in enumerate(r) if v)
                for i, r in enumerate(dot_rows) if span(top + i))
    lower = max(max(span(base + i)) + 2 - min(x for x, v in enumerate(r) if v)
                for i, r in enumerate(dot_rows) if span(base + i))
    pad = max(0, -upper)
    width = max(sw, lower + dw) + pad
    out = [[0] * width for _ in range(h)]
    for y in range(h):
        for x, v in enumerate(rows[y]):
            out[y][x + pad] = v
    for oy, ox in ((top, upper + pad), (base, lower + pad)):
        for i, r in enumerate(dot_rows):
            for x, v in enumerate(r):
                if v:
                    out[oy + i][ox + x] = v
    return crop(out)


def quotes(glyphs, step):
    """D19T: closing and opening double quotes from the font's own apostrophe
    (its shape is a closing quote): two apostrophes `step` px apart, the second
    over the first; the opening one is that pair turned half round over the
    apostrophe's ink rows, so it stays at the top of the line. As the menu
    font's quotes (game_menu.cpp)."""
    w, h, texels = glyphs["'"]
    rows = [[0] * (w + step) for _ in range(h)]
    for k in (0, step):
        for y in range(h):
            for x in range(w):
                if texels[y * w + x]:
                    rows[y][x + k] = texels[y * w + x]
    lit = [y for y in range(h) if any(rows[y])]
    a, b = lit[0], lit[-1]
    opening = [list(r) for r in rows]
    for y in range(a, b + 1):
        opening[y] = rows[a + b - y][::-1]
    return crop(rows), crop(opening)


# ---------------------------------------------------------------------------
# D26L: the rest of printable ASCII in the italic fonts. The disc has none of
# # $ & * + [ \ ] ^ _ { | } ~ or a single backquote; before D26L the first ten
# fell back to the 8x8 system font (whose own [ ] are one bare bar and _ is
# blank) and the last five drew '?'. Each is built from the font's own strokes,
# as D24C-D24E: the pipe from a narrowed I, _ + ~ from the hyphen, [ ] from the
# L, { } from the parentheses, # from = and narrowed slashes, $ from the S, ^
# and \ from the slash. Only & and * have no part to borrow: they are drawn as
# silhouettes (AMPERSAND, ASTERISK; ours) and lit by style_shade(), which
# reads its colours off the font's own glyphs.
SLANT = {'big': 4 / 16, 'medium': 3 / 10}  # the I's lean: px right per row up
FONT_GLYPHS = {}  # the italic fonts being built, for leaned()
NARROW = {'big': 4, 'medium': 3}  # D26L: px kept of a stroke for | # $ + (the I is 12 / 8)


def as_rows(glyph):
    w, h, texels = glyph
    return [list(texels[y * w:(y + 1) * w]) for y in range(h)]


def blank(w, h):
    return [[0] * w for _ in range(h)]


def paste(dst, rows, ox, oy):
    """Draws rows into dst at (ox, oy); clear texels leave dst as it is."""
    for y, r in enumerate(rows):
        for x, v in enumerate(r):
            if v and 0 <= oy + y < len(dst) and 0 <= ox + x < len(dst[0]):
                dst[oy + y][ox + x] = v


def runs(row):
    out, start = [], None
    for x, v in enumerate(list(row) + [0]):
        if v and start is None:
            start = x
        if not v and start is not None:
            out.append((start, x - 1)); start = None
    return out


def narrowed(rows, keep):
    """Each row's stroke cut to `keep` px: its first keep-1 texels (the lit
    edge) and its last (the dark edge), so the cut stroke is lit as the font."""
    out = []
    for r in rows:
        new = [0] * len(r)
        for a, b in runs(r):
            part = r[a:b + 1]
            if len(part) > keep:
                part = part[:keep - 1] + part[-1:]
            new[a:a + len(part)] = part
        out.append(new)
    return out


def squeezed(rows, width):
    """Rows `width` wide by dropping their middle columns (both ends kept)."""
    w = len(rows[0])
    if width >= w:
        return [list(r) for r in rows]
    left = width // 2
    keep = list(range(left)) + list(range(w - (width - left), w))
    return [[r[x] for x in keep] for r in rows]


def mirrored(rows, slant=0.0):
    """Each stroke run moved to the mirror position in its row, its texels kept
    in order (so it stays lit from the left). With `slant`, mirrored about the
    upright letter and leaned again (for ] } from [ {)."""
    h, w = len(rows), len(rows[0])
    shift = [round(2 * slant * (h - 1 - y)) for y in range(h)]
    pad = max(shift)
    out = blank(w + pad, h)
    for y, r in enumerate(rows):
        for a, b in runs(r):
            x0 = w - 1 - b + shift[y]
            out[y][x0:x0 + b - a + 1] = r[a:b + 1]
    return out


def retoned(rows, glyphs, dst_y):
    """Rows moved to dst_y re-lit for their new height: lit texels (4 and up)
    shifted by the difference in the I's mean lit texel between the rows."""
    ref = [r for r in as_rows(glyphs['I']) if any(t >= 4 for t in r)]
    mean = lambda y: (lambda v: sum(v) / len(v))([t for t in ref[max(0, min(y, len(ref) - 1))] if t >= 4])
    out = []
    for y, r in enumerate(rows):
        src = len(ref) - len(rows) + y + 1  # rows taken from the bottom
        d = round(mean(dst_y + y) - mean(src))
        out.append([v if v < 4 else max(4, min(14, v + d)) for v in r])
    return out


def ink_rows(rows):
    lit = [y for y, r in enumerate(rows) if any(r)]
    return lit[0], lit[-1]


def stem(glyphs, font, height, keep=None):
    """A narrowed I stem `height` rows tall, leaning as the font, lit from the
    I's own rows (sampled over its height) and closed by the I's bottom edge."""
    ref = narrowed(as_rows(glyphs['I']), keep or NARROW[font])
    h = len(ref)
    edge = 2 if h >= 17 else 1
    body = ref[:h - edge]
    width = len(ref[0]) + round(SLANT[font] * height)
    out = blank(width + 2, height)
    left = lambda r: next(x for x, v in enumerate(r) if v)
    for y in range(height):
        src = body[min(len(body) - 1, y * len(body) // max(1, height - edge))] if y < height - edge else ref[h - edge + y - (height - edge)]
        part = [v for v in src[left(src):] if v] if any(src) else []
        x0 = round(SLANT[font] * (height - 1 - y))
        out[y][x0:x0 + len(part)] = part
    return out


def pipe(glyphs, font):
    """'|': a narrowed I stem one row taller than the capitals (into the line
    gap), so it is not read as an I."""
    return crop(stem(glyphs, font, len(as_rows(glyphs['I'])) + 1))


def underscore(glyphs, font):
    """'_': the hyphen's lower rows (Medium 2, Big 4, the dark edge last)
    moved below the baseline, its dark edge on the row under the capitals'
    outline, so its drop shadow stays inside the line; re-lit for the height."""
    rows = as_rows(glyphs['-'])
    h = len(rows)
    a, b = ink_rows(rows)
    n = {'big': 4, 'medium': 2}[font]
    bar = retoned(rows[b + 1 - n:b + 1], glyphs, h + 1 - n)
    out = blank(len(rows[0]) + 4, h + 1)
    paste(out, bar, 0, h + 1 - n)
    return crop(out)


def backslash(glyphs):
    """'\\': the font's own slash with each row's stroke moved to the mirror
    position (still lit from the left)."""
    return crop(mirrored(as_rows(glyphs['/'])))


def bracket(glyphs, font, right):
    """'[' from the font's own L: its stem and foot, the foot shortened to a
    bracket's width and copied to the top (re-lit, leaning with the stem).
    ']' is '[' mirrored about the upright letter (Medium's ']' is drawn in ART
    instead since D26L5: the mirrored L stem came out ragged)."""
    rows = as_rows(glyphs['L'])
    h = len(rows)
    foot = {'big': 7, 'medium': 3}[font]
    width = {'big': 11, 'medium': 7}[font]
    shape = squeezed(rows, width + 1)
    out = blank(len(shape[0]) + 4, h)
    paste(out, shape, 0, 0)
    top = retoned(shape[h - foot:], glyphs, 0)
    if font == 'medium':
        top = top[:2]  # the foot's own dark under-edge stays at the bottom only
    a = min(x for x, v in enumerate(shape[0]) if v)
    b = min(x for x, v in enumerate(top[0]) if v) if any(top[0]) else 0
    for y in range(len(top)):
        out[y] = [0] * len(out[y])
    paste(out, top, a - b, 0)
    glyph = crop(out)
    return crop(mirrored(as_rows(glyph), SLANT[font])) if right else glyph


def backquote(glyphs):
    """'`': the font's own apostrophe turned half round over its ink rows (the
    single form of D19T's opening quote)."""
    rows = as_rows(glyphs["'"])
    a, b = ink_rows(rows)
    out = [list(r) for r in rows]
    for y in range(a, b + 1):
        out[y] = rows[a + b - y][::-1]
    return crop(out)


# D26L: our own pixel art for the glyphs with no disc part to cut them from,
# per font: (silhouette, top row, disc part drawn over it or None[, upright]).
# Silhouettes are drawn upright and leaned as the font's I when built (pass
# False for one drawn in place over a disc part). In a
# silhouette '#' is a texel style_shade() lights from the font's own colours,
# 'o' the font's dark line, '.' clear; the part is (glyph, x, y, rows left out)
# and stays on top. A string in place of a silhouette mirrors that glyph's
# silhouette about the upright letter (} from {). Leaning as the I.
ART = {
    'big': {
        '+': ([
            '.....######.....',
            '.....######.....',
            '.....######.....',
            '.....######.....',
            '################',
            '################',
            '################',
            '################',
            '################',
            '################',
            '.....######.....',
            '.....######.....',
            '.....######.....',
            '.....######.....',
        ], 1, None),
        '#': ([
            '...#####....#####..',
            '...#####....#####..',
            '...#####....#####..',
            '...#####....#####..',
            '###################',
            '###################',
            '###################',
            '...#####....#####..',
            '...#####....#####..',
            '...#####....#####..',
            '###################',
            '###################',
            '###################',
            '...#####....#####..',
            '...#####....#####..',
            '...#####....#####..',
            '...#####....#####..',
        ], 0, None),
        '[': ([
            '###########',
            '###########',
            '###########',
            '###########',
        ] + ['######.....'] * 9 + [
            '###########',
            '###########',
            '###########',
            '###########',
        ], 0, None),
        ']': ('[', 0, None),
        '|': (['######'] * 18, 0, None),  # one row taller than the capitals
        '$': ([
            '..........++++#',
            '..........++++#',
            '', '', '', '', '', '', '', '', '', '', '', '', '', '', '',
            '.....++++',
        ], 0, ('S', 0, 2, (4, 12)), False),
        '^': ([  # D26L5: 8-texel arms meeting in a solid apex (bold, same width)
            '........#####',
            '.......#######',
            '......#########',
            '.....###########',
            '....#############',
            '...###############',
            '..########.########',
            '.########...########',
            '########.....########',
        ], 0, None),
        '~': ([  # D26L5: every column 6 rows, the hyphen's thickness
            '...#####',
            '.########',
            '###########',
            '#############......##',
            '#####################',
            '#####################',
            '###.....#############',
            '#........############',
            '...........##########',
            '.............######',
        ], 3, None),
        '*': ([
            '.....+++#.....',
            '.++#.+++#.++#.',
            '..+++++++++#..',
            '...+++++++#...',
            '+++++++++++++#',
            '...+++++++#...',
            '..+++++++++#..',
            '.++#.+++#.++#.',
            '.....+++#.....',
        ], 1, None),
        '&': ([  # painted texel by texel after the font's own 8, as Medium's
            '........111111.......',
            '......2256665211.....',
            '....1469aa9987511....',
            '...147bab98987721....',
            '...25bca931687721....',
            '...47caa522688611....',
            '...48b9951577633.....',
            '...15a9987866223..1341',
            '..1358999886431..15871',
            '.138a99545766411688741',
            '.16999511466689998741.',
            '138988222466799885421.',
            '1499862222269887542 1..'.replace(' ', '1'),
            '139877552215998754321.',
            '116887765569988765441.',
            '.11465654655656654461',
            '..11111222222112222221',
        ], 0, None, False),
        '{': ([
            '.....########',
            '....#########',
            '....#########',
            '....#######..',
            '....#######..',
            '....#######..',
            '...#######...',
            '.########....',
            '########.....',
            '.########....',
            '...#######...',
            '....#######..',
            '....#######..',
            '....#######..',
            '....#########',
            '....#########',
            '.....########',
        ], 0, None),
        '}': ('{', 0, None),
    },
    'medium': {
        '+': ([
            '...####...',
            '...####...',
            '...####...',
            '##########',
            '##########',
            '##########',
            '...####...',
            '...####...',
            '...####...',
        ], 2, None),
        '#': ([
            '..###..###..',
            '..###..###..',
            '..###..###..',
            '############',
            '############',
            '..###..###..',
            '..###..###..',
            '############',
            '############',
            '..###..###..',
            '..###..###..',
        ], 0, None),
        '$': ([
            '......###',
            '', '', '', '', '', '', '', '', '', '',
            '....+++',
        ], 0, ('S', 0, 1, (5,)), False),
        '^': ([  # D26L5: 5-texel arms meeting in a solid apex (bold, same width);
            '.....###',  # '+' keeps the right arm's inner texel lit
            '....#####',
            '...#######',
            '..#########',
            '.#####.+####',
            '#####...#####',
        ], 0, None),
        '~': ([  # D26L5: 3-4 rows a column, the hyphen's weight
            '.####',
            '#######.....#',
            '#############',
            '.############',
            '.....########',
            '........####',
        ], 3, None),
        ']': ([  # D26L5: drawn, not '[' mirrored (its stem came out ragged)
            '########',
            '########',
            '########',
        ] + ['....####'] * 5 + [
            '########',
            '########',
            '########',
        ], 0, None),
        '*': ([
            '....+#....',
            '.+#.+#.+#.',
            '..+++++#..',
            '+++++++++#',
            '..+++++#..',
            '.+#.+#.+#.',
            '....+#....',
        ], 1, None),
        '&': ([  # painted texel by texel after the font's own 8: its lower bowl
            '....556677....',  # opened into a tail up to the right and a kicked foot
            '...55667788...',
            '..5566118881..',
            '..5661188811..',
            '...667888111.6',
            '.66678888117781',
            '.667118899881..',
            '66781..18891...',
            '6778889988991..',
            '.78889991.999a1',
            '..11111111.1111',
        ], 0, None, False),
        '{': ([
            '...######',
            '..#######',
            '..####...',
            '..####...',
            '.####....',
            '####.....',
            '.####....',
            '..####...',
            '..####...',
            '..#######',
            '...######',
        ], 0, None),
        '}': ('{', 0, None),
    },
}


def edge_runs(mask, x, y):
    """Lit texels to a texel's left, right, top and bottom in its silhouette."""
    h, w = len(mask), len(mask[0])

    def run(dx, dy):
        n, xx, yy = 0, x + dx, y + dy
        while 0 <= xx < w and 0 <= yy < h and mask[yy][xx]:
            n += 1; xx += dx; yy += dy
        return n
    return run(-1, 0), run(1, 0), run(0, -1), run(0, 1)


def is_edge(font, l, r, u, d, y, drawn=False):
    """The font's dark outline: Medium Italic's lies along the right and bottom
    of a stroke (not on its top two rows), Big Italic's all round it. On a
    drawn Medium glyph the right edge is dark only after 3 lit texels, so its
    thinner diagonals (^ ~ { }) keep their weight (D26L, user 2026-11-27)."""
    if font == 'medium':
        return d == 0 or (r == 0 and y >= 2 and (not drawn or l >= 3))
    return min(l, r, u, d) == 0


def style_model(glyphs, font):
    """D26L: the font's colours, fitted on its letters and digits: the mean lit
    texel of each row, then mean offsets by distance (capped at 4) from the
    stroke's left and right edges; and its commonest outline texel."""
    rows, left, right, edge = {}, {}, {}, []
    samples = []
    for ch, (w, h, texels) in glyphs.items():
        if not ch.isalnum():
            continue
        mask = [[texels[y * w + x] != 0 for x in range(w)] for y in range(h)]
        for y in range(h):
            for x in range(w):
                v = texels[y * w + x]
                if not v:
                    continue
                l, r, u, d = edge_runs(mask, x, y)
                if is_edge(font, l, r, u, d, y):
                    edge.append(v)
                elif v >= 4:
                    rows.setdefault(y, []).append(v)
                    samples.append((y, min(l, 4), min(r, 4), v))
    mean = lambda v: sum(v) / len(v)
    row = {y: mean(v) for y, v in rows.items()}
    for y, l, r, v in samples:
        left.setdefault(l, []).append(v - row[y])
    lo = {k: mean(v) for k, v in left.items()}
    for y, l, r, v in samples:
        right.setdefault(r, []).append(v - row[y] - lo[l])
    ro = {k: mean(v) for k, v in right.items()}
    return row, lo, ro, max(set(edge), key=edge.count)  # the commonest outline texel


def style_shade(art, glyphs, font):
    """Lights a silhouette with the font's colours: '#' as the font would ('+'
    always as a stroke's inside, never its dark edge), 'o' the dark line, a
    hex digit that texel painted by hand."""
    row, lo, ro, edge = style_model(glyphs, font)
    h, w = len(art), max(len(r) for r in art)
    grid = [r.ljust(w, '.') for r in art]
    mask = [[c != '.' for c in r] for r in grid]
    out = blank(w, h)
    for y in range(h):
        for x in range(w):
            if grid[y][x] == '.':
                continue
            if grid[y][x] in '123456789abcde':  # a texel painted by hand
                out[y][x] = int(grid[y][x], 16)
                continue
            l, r, u, d = edge_runs(mask, x, y)
            if grid[y][x] == 'o' or (grid[y][x] == '#' and is_edge(font, l, r, u, d, y, drawn=True)):
                out[y][x] = 1 if grid[y][x] == 'o' else edge
            else:
                base = row.get(y, row[max(k for k in row if k <= y)] if any(k <= y for k in row) else 6)
                out[y][x] = max(4, min(14, round(base + lo.get(min(l, 4), 0) + ro.get(min(r, 4), 0))))
    return out


def leaned(art, top, font):
    """An upright silhouette leaned as the font's I: each row moved right by
    the slant times its height above the baseline."""
    h = len(as_rows(FONT_GLYPHS[font]['I']))
    shift = [max(0, round(SLANT[font] * (h - 1 - (top + y)))) for y in range(len(art))]
    return [('.' * shift[y] + r) if r else '' for y, r in enumerate(art)]


def drawn(glyphs, font, ch):
    """D26L: a glyph from ART: its silhouette lit by style_shade() on the union
    with the disc part it is drawn over (if any), the disc part on top."""
    art, top, part, *rest = ART[font][ch]
    upright = rest[0] if rest else True
    if isinstance(art, str):  # the mirror of another glyph's upright silhouette
        src = ART[font][art][0]
        w = max(len(r) for r in src)
        art = [r.ljust(w, '.')[::-1] for r in src]
    if upright:
        art = leaned(art, top, font)
    h = len(as_rows(glyphs['I']))
    tall = max(h, top + len(art))
    width = max(len(r) for r in art) + 4
    base = blank(width, tall)
    if part:
        src, dx, dy, drop = part
        rows = [r for y, r in enumerate(as_rows(glyphs[src])) if y not in drop]
        width = max(width, len(rows[0]) + dx + 4)
        base = blank(width, tall)
        paste(base, rows, dx, dy)
    grid = [['.'] * width for _ in range(tall)]
    for y, r in enumerate(art):
        for x, c in enumerate(r):
            grid[top + y][x] = c
    for y in range(tall):
        for x in range(width):
            if grid[y][x] == '-':  # cut out of the disc part
                base[y][x] = 0
                grid[y][x] = '.'
            elif base[y][x] and grid[y][x] == '.':
                grid[y][x] = '#'
    lit = style_shade([''.join(r) for r in grid], glyphs, font)
    out = blank(width, tall)
    for y in range(tall):
        for x in range(width):
            out[y][x] = base[y][x] or (lit[y][x] if art_at(art, top, x, y) else 0)
    return crop(out)


def art_at(art, top, x, y):
    return 0 <= y - top < len(art) and x < len(art[y - top]) and art[y - top][x] not in '.-'


def full_ascii(fonts, name):
    """D26L: every printable character the italic font lacks, in its style."""
    g = fonts[name]
    FONT_GLYPHS[name] = g
    g['|'] = pipe(g, name)
    g['_'] = underscore(g, name)
    g['\\'] = backslash(g)
    if '[' not in ART.get(name, {}):
        g['['] = bracket(g, name, False)
        g[']'] = bracket(g, name, True)
    for ch in ART.get(name, {}):
        g[ch] = drawn(g, name, ch)
    g['`'] = backquote(g)


# D26L: the PSY-Q 8x8 font's own '[' and ']' are one bare bar each (the same
# glyph) and its '_' is blank; it has no '`', '{', '|', '}' or '~'. Ours, in
# its 1-px style (the system sets draw them; the pipe one row taller than I).
SYSTEM_EXTRA = {
    '[': ['........', '...###..', '...#....', '...#....', '...#....', '...#....', '...#....', '...###..'],
    ']': ['........', '..###...', '....#...', '....#...', '....#...', '....#...', '....#...', '..###...'],
    '_': ['........', '........', '........', '........', '........', '........', '........', '........', '.######.'],
    '`': ['........', '..##....', '...##...', '........', '........', '........', '........', '........'],
    '{': ['........', '....##..', '...#....', '...#....', '..#.....', '...#....', '...#....', '....##..'],
    '|': ['....#...', '....#...', '....#...', '....#...', '....#...', '....#...', '....#...', '....#...', '....#...'],
    '}': ['........', '..##....', '....#...', '....#...', '.....#..', '....#...', '....#...', '..##....'],
    '~': ['........', '........', '........', '..##..#.', '.#..##..', '........', '........', '........'],
}


def micro_font(path=MICROFONT):
    """The open 3x5 Microfont (1D sheet, 3 px per ASCII code), 1-bit glyphs."""
    from build_ttk_inv_icons import read_rgba_png
    w, h, rgba = read_rgba_png(path)
    if (w, h) != (384, 5):
        raise SystemExit(f'{path}: expected the 384x5 Microfont 1D sheet, got {w}x{h}')
    return {chr(code): (3, 5, [int(rgba[(y * w + 3 * code + x) * 4 + 3] >= 128) for y in range(5) for x in range(3)])
            for code in range(0x21, 0x7f)}  # D26L: to '~' (lower case is drawn as capitals)


def system_font(exe):
    data = exe[SYSTEM_FONT - EXE_LOAD + 2048:SYSTEM_FONT - EXE_LOAD + 2048 + 2048]
    glyphs = {}
    for code in range(0x20, 0x60):
        i = code - 0x20
        gx, gy = (i % 16) * 8, (i // 16) * 8
        bits = []
        for y in range(8):
            for x in range(8):
                px = gx + x
                bits.append((data[(gy + y) * 64 + px // 2] >> (4 * (px % 2))) & 15)
        glyphs[chr(code)] = (8, 8, bits)
    for ch, art in SYSTEM_EXTRA.items():  # D26L
        glyphs[ch] = (8, len(art), [int(c == '#') for r in art for c in r])
    return glyphs


# D24B UI sprites from the HUD sprite table (16-byte records: CLUT id, w, h,
# VRAM x, VRAM y), cells and CLUTs in FONTS.RAW. Pack TTKUI1: 'TTKUI1\0\0', u32
# count, then per sprite u16 id, u16 w, u16 h and ARGB u32 pixels.
UI_SPRITES = [(1, 'Up/Down', 0x800c4474), (2, 'Cross', 0x800c4414), (3, 'Square', 0x800c4424),
              (4, 'Circle', 0x800c4404), (5, 'Triangle', 0x800c43f4), (6, 'Radiation disc', 0x800c4454)]


def ui_pack(raw, exe):
    sheet, out = Sheet(raw), b''
    for ident, _, address in UI_SPRITES:
        clut, wh, x, z1, y, z2 = struct.unpack_from('<HHHHHH', exe, address - EXE_LOAD + 2048)
        if z1 or z2 or (clut & 0x3f) != 0x3f:
            raise SystemExit(f'unexpected sprite record at {address:#x}')
        w, h, cy = wh & 255, wh >> 8, clut >> 6
        pixels = []
        for yy in range(h):
            for xx in range(w):
                t = (sheet.word(x + xx // 4, y + yy) >> (4 * (xx % 4))) & 15
                v = sheet.word(1008 + t, cy)
                pixels.append(0 if v == 0 else argb(((v & 31) * 255 // 31, ((v >> 5) & 31) * 255 // 31, ((v >> 10) & 31) * 255 // 31)))
        out += struct.pack('<HHH', ident, w, h) + struct.pack(f'<{len(pixels)}I', *pixels)
    return b'TTKUI1\0\0' + struct.pack('<I', len(UI_SPRITES)) + out


def argb(rgb):
    return 0xff000000 | (rgb[0] << 16) | (rgb[1] << 8) | rgb[2]


def build(raw, exe):
    sheet = Sheet(raw)
    fonts = {name: sheet.font(rows, height) for name, (rows, height, _) in FONT_INFO.items() if rows}
    for name, (trim, taper) in EXCLAMATION.items():
        fonts[name]['!'] = exclamation(fonts[name], trim, taper)
        fonts[name]['('] = parenthesis(fonts[name], False)
        fonts[name][')'] = parenthesis(fonts[name], True)
        fonts[name]['%'] = percent(fonts[name], trim)
        fonts[name]['"'], fonts[name][OPEN_QUOTE] = quotes(fonts[name], QUOTE_STEP[name])  # D19T
        full_ascii(fonts, name)  # D26L
    system = system_font(exe)
    fonts['system'] = system
    fonts['micro'] = micro_font()
    palettes = {225: sheet.palette(225), 226: sheet.palette(226),
                'steel': [tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) for h in STEEL],
                'red': [tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) for h in RED],
                'stats_red': [tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) for h in STATS_RED],
                'stats_beige': [tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) for h in STATS_BEIGE]}
    headers, pixels, records, fallbacks = b'', b'', [], {}
    base = 4 + len(SETS) * (8 + GLYPHS * 8)
    for name, font, pal, scale, spacing, shadow in SETS:
        _, height, space = FONT_INFO[font]
        if isinstance(pal, str) and pal.startswith('#'):
            tint = tuple(int(pal[i:i + 2], 16) for i in (1, 3, 5))
            colours = [tint] * 16  # 1-bit system glyphs: every lit texel takes the tint
        else:
            colours = palettes[pal]
        shadow_colour = (SHADOW if shadow is True else shadow) if shadow else 0
        headers += struct.pack('<BBBBI', height + 2, scale, int(bool(shadow)), 0, shadow_colour)
        used = []
        for code in range(32, 32 + GLYPHS):
            ch = OPEN_QUOTE if code == 127 else chr(code).upper()  # all caps
            if ch == OPEN_QUOTE and OPEN_QUOTE not in fonts[font]:
                ch = '"'  # the 8x8 and 3x5 fonts' quote is the same both ways
            if ch == ' ':
                records.append(struct.pack('<BBBBI', 1, 1, space, 0, base + len(pixels)))
                pixels += struct.pack('<I', 0)
                continue
            if ch in fonts[font]:
                w, h, texels = fonts[font][ch]
                y, advance = 0, w + spacing
                data = [0 if t == 0 else argb(colours[t]) for t in texels]
            elif ch in system:
                w, h, bits = system[ch]
                y, advance = max(0, height - h - (1 if height >= 11 else 0)), w
                data = [argb(colours[11]) if b else 0 for b in bits]
                used.append(ch)
            else:
                ch = '?'
                w, h, texels = fonts[font]['?']
                y, advance = 0, w + spacing
                data = [0 if t == 0 else argb(colours[t]) for t in texels]
            records.append(struct.pack('<BBBBI', w, h, advance, y, base + len(pixels)))
            pixels += struct.pack(f'<{len(data)}I', *data)
        fallbacks[name] = ''.join(sorted(set(used)))
    glyph_block = b''
    for s in range(len(SETS)):
        glyph_block += headers[8 * s:8 * s + 8] + b''.join(records[GLYPHS * s:GLYPHS * (s + 1)])
    payload = struct.pack('<I', len(SETS)) + glyph_block + pixels
    assert len(struct.pack('<I', len(SETS)) + glyph_block) == base
    h = 2166136261
    for byte in payload:
        h = ((h ^ byte) * 16777619) & 0xffffffff
    return b'TTKFONT3' + struct.pack('<II', len(payload), h) + payload, fallbacks


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--image', default=str(ROOT / 'disc/time-to-kill.bin'), help='prepared MODE2/2352 image')
    ap.add_argument('--exe', default=str(ROOT / 'disc/SLUS_005.83'))
    ap.add_argument('--output', default=str(ROOT / 'build-local/ttk-fonts.pack'))
    ap.add_argument('--ui-output', help='UI sprite pack (default: ttk-ui.pack beside --output)')
    args = ap.parse_args()
    disc = Disc(args.image)
    try:
        entry = next((f for f in disc.files() if f['path'] == FONTS_PATH), None)
        if not entry or entry['size'] != 32768:
            raise SystemExit(f'{FONTS_PATH} not found with the expected size: is this the owned USA SLUS-00583 disc?')
        raw = disc.read(entry['lba'], entry['size'])
    finally:
        disc.close()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != FONTS_SHA256:
        raise SystemExit(f'{FONTS_PATH} sha256 {digest} does not match the owned USA disc')
    exe = Path(args.exe).read_bytes()
    pack, fallbacks = build(raw, exe)
    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(pack)
    ui = Path(args.ui_output) if args.ui_output else out.with_name('ttk-ui.pack')
    ui.write_bytes(ui_pack(raw, exe))
    out.with_suffix('.json').write_text(json.dumps({
        'source': f'{FONTS_PATH} (sha256 {digest}) and the SLUS_005.83 system font at {SYSTEM_FONT:#x}; extracted from the player\'s disc at build time',
        'sets': [{'set': i, 'use': name, 'font': font,
                  'palette': 'Console steel (project)' if pal == 'steel' else pal if isinstance(pal, str) else f'CLUT {pal}',
                  'scale': scale, 'spacing': spacing, 'shadow': shadow, 'system_font_fallback': fallbacks[name]}
                 for i, (name, font, pal, scale, spacing, shadow) in enumerate(SETS)],
        'format': 'TTKFONT3; see tools/local/build_ttk_fonts.py',
        'note': 'Retail-derived build output: never commit or distribute.'}, indent=2) + '\n')
    print(out)


if __name__ == '__main__':
    main()
