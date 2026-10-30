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
// D08K2 (debug): "crouchwalk <fwd> [side] [hull]", "crouchgo <dx> <dz> [hull]" (world units),
// "crouchwalk off" and "crouchscan [radius] [grid]".
bool crouch_console_command(const char* line, void (*say)(const char*), bool& close);
// D14A: "fov [60..120|default]", session-only first-person projection.
bool fov_console_command(const char* line, void (*say)(const char*), bool& close);
// D17AR2 (developer): "hostvis [on|off|default]" (modern_controls.cpp,
// host_visibility.inc), the host world visibility switch for the session.
bool hostvis_console_command(const char* line, void (*say)(const char*), bool& close);
// D17AR3 (developer): "redrawonly [on|off|default]" (frame_replay.cpp),
// redraw-only live frames for the session.
bool redraw_only_console_command(const char* line, void (*say)(const char*), bool& close);
// D24B: the game's own name for level n (from guest RAM, "" when unavailable).
bool level_title(unsigned n, char* out, unsigned cap);
}
