"""D26L2: the console's one command table (src/ttk/console_syntax.cpp) stays in
step with the handlers that parse the commands, so the highlighter, help and
(later) tab completion never call a working command a typo."""
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
TTK = ROOT / "src" / "ttk"
SYNTAX = TTK / "console_syntax.cpp"
MAIN = ROOT / "psxrecomp" / "runtime" / "src" / "main.cpp"
# Parsers named `line` that read something other than the console.
NOT_CONSOLE = {("relit.inc", "ambient"), ("relit.inc", "relit")}  # the lamp file's ambient lines


def table():
    text = SYNTAX.read_text()
    body = text[text.index("const Command table[]={"):text.index("constexpr int table_n")]
    rows = {}
    for m in re.finditer(r'\{"([a-z]+)",((?:"[^"]*"\s*)+|toggle),', body):
        words = "on off default" if m.group(2) == "toggle" else "".join(re.findall(r'"([^"]*)"', m.group(2)))
        rows[m.group(1)] = set(words.split())
    return rows


def sources():
    files = [p for p in TTK.iterdir() if p.suffix in (".inc", ".cpp") and p.name != "console_syntax.cpp"]
    return files + [MAIN]


class ConsoleSyntaxTable(unittest.TestCase):
    def test_every_parsed_command_is_in_the_table(self):
        rows = table()
        missing = []
        for path in sources():
            for m in re.finditer(r'str(?:n)?cmp\((?:line|s_console_line), ?"([^"]*)"', path.read_text()):
                word = m.group(1).split(" ")[0]
                if word and (path.name, word) not in NOT_CONSOLE and word not in rows:
                    missing.append(f"{path.name}: {word}")
        self.assertEqual(missing, [], "add these commands to console_syntax.cpp's table")

    def test_every_table_command_is_parsed_somewhere(self):
        text = "\n".join(p.read_text() for p in sources())
        for name in table():
            self.assertRegex(text, r'"%s[" ]' % re.escape(name), f"{name}: no handler parses it")

    def test_fixed_name_lists_are_keywords(self):
        rows = table()
        def names(path, array):
            text = (TTK / path).read_text()
            block = text[text.index(array):]
            block = block[:block.index("};")]
            return set(re.findall(r'\{?"([a-z]+)"', block))
        self.assertLessEqual(names("level_lights.inc", "lamp_style_names[lamp_styles]={"), rows["lamp"])
        self.assertLessEqual(names("console_idle.inc", "fp_idle_gestures[]={"), rows["fpidle"])
        self.assertLessEqual(names("ladder_armed_hang.inc", "armed_hang_shapes[3]={"), rows["hangpose"])

    def test_old_help_list_is_gone(self):
        # help comes from the table now; a second hand-written list would drift.
        self.assertNotIn('debug_console_say("Available commands:")', MAIN.read_text())


if __name__ == "__main__":
    unittest.main()
