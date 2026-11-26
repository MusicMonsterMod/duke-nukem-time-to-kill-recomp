"""D61B: the hidden levels' sound and look packs (synthetic banks; disc runs skip in CI)."""
import struct, subprocess, sys, tempfile, unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
IMAGE = ROOT / 'disc/time-to-kill.bin'
sys.path.insert(0, str(ROOT / 'tools/local'))
import build_ttk_sound as snd  # noqa: E402


def bank(groups, samples, used=None):
    """A synthetic SND.DAT: groups {g: [entry bytes]}, samples [bytes] (index k = sample k)."""
    counts = [len(groups.get(g, [])) for g in range(snd.GROUPS)]
    body, ptrs, at = b'', [], snd.ENTRIES
    for g in range(snd.GROUPS):
        ptrs.append(0x400000 + at)
        for e in groups.get(g, []):
            body += e
            at += 16
    table_size = snd.ENTRIES + len(body)
    sizes = [0] + [len(s) // 8 for s in samples] + [0] * (255 - len(samples))
    vh = bytearray(0x800)
    vh[:4] = b'pBAV'
    struct.pack_into('<H', vh, 0x16, len(samples) if used is None else used)
    vh[-0x200:] = struct.pack('<256H', *sizes)
    vb = b''.join(samples if used is None else samples[:used])
    head = struct.pack('<III', table_size, len(vh), len(vb))
    head += b''.join(struct.pack('<II', c, p) for c, p in zip(counts, ptrs))
    return snd.Bank(head + body + bytes(vh) + vb)


def entry(k, disabled=False):
    return bytes([k >> 4, k & 15, 10, 0, 0x80 if disabled else 6, 0x40, 0x46, 0x34]) + bytes(8)


class SoundPatch(unittest.TestCase):
    def test_missing_gadget_group_and_disabled_ids(self):
        s = [bytes([i]) * 16 for i in range(6)]
        donor = bank({0: [entry(0)], 1: [entry(1), entry(2)], 28: [entry(3), entry(4, True), entry(5)]}, s)
        hidden = bank({0: [entry(0)], 1: [entry(1, True), entry(1)]}, s[:2])
        entries, samples = snd.patch(hidden, donor)
        ids = [i for i, _ in entries]
        # the menu group, the disabled id 0x1000, the whole gadget group; not 0x1001 (own variant)
        self.assertEqual(ids, [0x0, 0x1000, 0x1c000, 0x1c001, 0x1c002])
        # each donor sample once, at the next free index
        self.assertEqual([k for k, _ in samples], [2, 3, 4, 5])
        by_id = dict(entries)
        self.assertEqual(snd.index(by_id[0x1c000]), 4)        # donor sample 3 -> new index 4
        self.assertTrue(snd.disabled(by_id[0x1c001]))          # a disabled donor entry stays disabled
        self.assertEqual(dict(samples)[4], s[3])

    def test_stale_sizes_past_the_sample_count(self):
        s = [bytes([i]) * 16 for i in range(3)]
        b = bank({0: [entry(0)]}, s, used=2)   # the VB holds 2 samples, the size table 3
        self.assertEqual(b.used, 2)


@unittest.skipUnless(IMAGE.exists(), 'prepared owned disc required')
class DiscPacks(unittest.TestCase):
    def test_build(self):
        with tempfile.TemporaryDirectory() as tmp:
            for tool, magic in (('build_ttk_sound.py', b'TTKSND1\0'), ('build_ttk_look.py', b'TTKLOOK2')):
                out = Path(tmp) / tool
                subprocess.run([sys.executable, str(ROOT / 'tools/local' / tool), '--image', str(IMAGE),
                                '--output', str(out)], check=True, capture_output=True)
                self.assertEqual(out.read_bytes()[:8], magic)


if __name__ == '__main__':
    unittest.main()
