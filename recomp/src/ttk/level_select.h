#pragma once

namespace ttk {
// D26A debug level select for the backtick console (runtime event-pump thread,
// between guest frames). Handles "levels" and "level N"; returns false for any
// other line. close is set when the console should close so play resumes.
bool level_console_command(const char* line, void (*say)(const char*), bool& close);
// D26E: "items" and "spawn <item>" (modern_controls.cpp, spawn.inc). Queues the
// request; the emulation thread creates the object. Returns false otherwise.
bool spawn_console_command(const char* line, void (*say)(const char*), bool& close);
bool earthquake_console_command(const char* line, void (*say)(const char*), bool& close);
bool lights_console_command(const char* line, void (*say)(const char*), bool& close);
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
// D17AV (developer): "smoothquads [on|off|default]" (near_clip.cpp), smooth
// quad shading in live frames for the session.
bool smoothquads_console_command(const char* line, void (*say)(const char*), bool& close);
// D17AR3 (developer): "redrawonly [on|off|default]" (frame_replay.cpp),
// redraw-only live frames for the session.
bool redraw_only_console_command(const char* line, void (*say)(const char*), bool& close);
// D40: "renderer [polymorph|classic]" (frame_replay.cpp), the world renderer
// in game for the session, as EDuke32's setrendermode.
bool renderer_console_command(const char* line, void (*say)(const char*), bool& close);
bool world_renderer_request(bool classic);
// D39C: "palfrom <intensity> [r] [g] [b]" and its alias "flash" (screen_flash.cpp).
bool flash_console_command(const char* line, void (*say)(const char*), bool& close);
// D10G1: "pointblank [lift [max]|default]" (weapon_aim.cpp), session-only lift of
// close third-person rounds toward the crosshair point.
bool point_blank_console_command(const char* line, void (*say)(const char*), bool& close);
// D10G3: "shotview [on|off|default]" (weapon_aim.cpp), session-only A/B of
// aiming shots from the view on screen versus the previous update's camera.
bool shot_view_console_command(const char* line, void (*say)(const char*), bool& close);
bool beam_follow_console_command(const char* line, void (*say)(const char*), bool& close);
// D10G4: "shootseen [on|off|default]" (weapon_aim.cpp), session-only A/B of
// testing the crosshair against enemies where they are drawn or where they are now.
bool shoot_seen_console_command(const char* line, void (*say)(const char*), bool& close);
// D10H3: "autoaim [cone|default]" (weapon_aim.cpp), the AUTO AIM EDUKE32 cone in
// 2048ths of a turn either side of the view, session only.
bool auto_aim_console_command(const char* line, void (*say)(const char*), bool& close);
// D10H2: "testmove <actor> <u/s> [span]|off" (weapon_aim.cpp), lab fixture that
// slides an actor across the view on the game thread, session only.
bool test_move_console_command(const char* line, void (*say)(const char*), bool& close);
// D10H2: "roundfollow [on|off|default]" (weapon_aim.cpp), session-only A/B of a
// round aimed at an enemy meeting it after the enemy's own update.
bool round_follow_console_command(const char* line, void (*say)(const char*), bool& close);
// D24B: the game's own name for level n (from guest RAM, "" when unavailable).
bool level_title(unsigned n, char* out, unsigned cap);
}
