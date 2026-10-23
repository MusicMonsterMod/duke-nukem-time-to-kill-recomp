#pragma once
#include "psx_sdl.h"
#include <cstdint>
#include "cheat_codes.h"

namespace ttk {
enum Action {
#define TTK_ACTION(name, token, code, pad) name,
#include "input_bindings.def"
#undef TTK_ACTION
    action_count
};
enum class Context { Menu, Gameplay };
enum class Device { None, Keyboard, Mouse };
struct InputFrame {
    uint64_t sequence = 0, epoch = 0;
    double total_x = 0, total_y = 0;
    bool active = false;
    uint64_t command_serial = 0;
    int commands[32]{};
    unsigned command_count = 0;
    double distance_total = 0;
    // D10: recenter presses this epoch; shoulder side -1 left, 0 centered, 1 right.
    uint64_t recenter_total = 0;
    // D10C: Look Up / Down presses this epoch, so a tap between camera updates counts.
    uint64_t look_press_total = 0;
    int shoulder = 0;
    // Persistent view intent, including inactive frames. Active gates actions,
    // never this preference (the eye applies only in supported camera states).
    bool first_person = false;
    bool held[action_count]{};
    float move_x = 0, move_y = 0, look_x = 0, look_y = 0;
    // D08Q6: the fixed Up / Down arrows (up - down); jetpack flight treats
    // them as forward / back like W / S. D10D: Left / Right are turn keys.
    float arrow_y = 0;
    Device device = Device::None;
};
void input_init(SDL_Window* window);
// Called on the runtime event-pump thread, never from an SDL event watch.
void input_event(const SDL_Event& event);
void input_frame();
// Host frame counter advanced by input_frame() (main loop, once per presented
// frame). Identity verdicts are memoised per value of this.
uint64_t input_host_frame();
Cheat input_take_cheat();
void input_notice(const char* message);
// D08A18: start the normal E interaction (stow, then the original action), as
// if E were pressed; from the emulation thread.
void input_request_interaction();
// A guest update acknowledges accepted or rejected commands; bounded host expiry.
void input_ack_commands(uint64_t serial);
// A verified original normal-gameplay camera callback offers initial capture.
void input_offer_gameplay_capture();
bool input_wants_initial_capture();
// Menu has no gameplay actions. Fixed menu navigation is the pad path below.
const InputFrame& input_snapshot(Context context);
// D17B: look totals as of now for a late camera: the last frame's totals plus
// mouse motion pumped since and motion still queued in SDL (peeked, not
// consumed). False unless Modernized gameplay holds the captured mouse.
// at_ms >= 0: the look as it was at that time (performance clock, ms).
bool input_live_look(uint64_t& epoch, double& x, double& y, double at_ms=-1);
// The presenter clock input_live_look's at_ms uses (performance counter, ms).
double input_clock_ms();
bool input_modernized();
// Host preference; independent of capture, saves and guest control leases.
bool input_modern_controls();
// D19G: the KEY BINDINGS page. Codes are the DNTTK_INPUT_BINDINGS wire codes
// (SDL scancode, mouse button -n, 0 unbound).
int input_binding_code(Action action);
int input_default_code(Action action);
bool input_code_bindable(int code, Action action);
void input_set_binding_codes(const int* codes);
// While on, the next key or mouse button press goes to the menu only.
void input_capture_key(bool on);
int input_take_captured_key();   // input_no_key() when none
int input_no_key();
bool input_first_person();
int input_shoulder();
void input_set_view(bool first);
void input_set_shoulder(int side);
const char* input_binding_name(Action action);
bool input_running();
// Consume one bounded jump press after the controls hook verifies grounded ownership.
bool input_take_jump();
/* D08O3: one pending E redraw in water, taken by the weapon request path. */
bool input_take_swim_redraw();
bool input_jump_pending();
// Bounded E-owned stow, including its post-holster airborne blend.
bool input_auto_stow_pending();
// D08X: E asks for an airborne reach (held, pending or already reaching).
bool input_airborne_reach_held();
// D08T1: Grab / Manipulate is requesting or holding a pushable object, so any
// captured Cross is the grab's own, never E's interaction/mantle.
bool input_push_grab_owns_cross();
uint16_t input_pad();
// D19: the menu screens (game_menu.cpp) register their key handler here.
struct MenuKeys;
void input_set_menu_handler(bool (*handler)(const MenuKeys&));
// D20: EDuke32's quick save / quick load (bindable, F6 / F9; console
// quicksave / quickload) and its save / load menu keys (F2 / F3). game_menu.cpp
// answers what it did: nothing (not in a one-player game), done, the pause to
// open SAVE / LOAD GAME on, or a load under way.
enum class Quick {Save, Load, SaveMenu, LoadMenu};
enum class QuickResult {Ignored, Done, Pause, Load};
void input_set_quick_handler(QuickResult (*handler)(Quick));
// The handler's answer, with the mouse and pause it asks for applied.
QuickResult input_quick(Quick quick);
// Diagnostics: the last value input_pad() returned (no side effects).
uint16_t input_last_pad();
// D19O: our menu's keys while the game reads no pad (a demo scene's blocking
// load behind the title menu).
void input_menu_poll();
void input_release();
// Pause/death lists acknowledge the menu and release capture if held.
void input_release_for_menu();
void input_state_loaded();
void input_allow_capture(bool allow);
void input_pad_context(uint16_t pad);
// D17T (frontend_input.cpp): the player-1 pad word as the frontend should see
// it; a short Modernized press there is held until the 10 Hz tick can read it.
uint16_t frontend_pad_hold(uint16_t pad);
}
extern "C" const char* ttk_input_debug_json();
