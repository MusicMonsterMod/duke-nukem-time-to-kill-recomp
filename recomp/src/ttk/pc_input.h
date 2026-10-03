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
    int shoulder = 0;
    // D11: player asked for the eye-level view (applies only in supported states).
    bool first_person = false;
    bool held[action_count]{};
    float move_x = 0, move_y = 0, look_x = 0, look_y = 0;
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
bool input_modernized();
const char* input_binding_name(Action action);
bool input_running();
// Consume one bounded jump press after the controls hook verifies grounded ownership.
bool input_take_jump();
bool input_jump_pending();
// Bounded E-owned stow, including its post-holster airborne blend.
bool input_auto_stow_pending();
// D08X: E asks for an airborne reach (held, pending or already reaching).
bool input_airborne_reach_held();
// D08T1: Grab / Manipulate is requesting or holding a pushable object, so any
// captured Cross is the grab's own, never E's interaction/mantle.
bool input_push_grab_owns_cross();
uint16_t input_pad();
void input_release();
void input_state_loaded();
void input_allow_capture(bool allow);
void input_pad_context(uint16_t pad);
}
extern "C" const char* ttk_input_debug_json();
