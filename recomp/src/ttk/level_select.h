#pragma once

namespace ttk {
// D26A debug level select for the backtick console (runtime event-pump thread,
// between guest frames). Handles "levels" and "level N"; returns false for any
// other line. close is set when the console should close so play resumes.
bool level_console_command(const char* line, void (*say)(const char*), bool& close);
// D26E: "items" and "spawn <item>" (modern_controls.cpp, spawn.inc). Queues the
// request; the emulation thread creates the object. Returns false otherwise.
bool spawn_console_command(const char* line, void (*say)(const char*), bool& close);
// D08A10: "sfx <id>" plays a game sound at Duke (debug audition).
bool beat_console_command(const char* line, void (*say)(const char*), bool& close);
// D24B: the game's own name for level n (from guest RAM, "" when unavailable).
bool level_title(unsigned n, char* out, unsigned cap);
}
