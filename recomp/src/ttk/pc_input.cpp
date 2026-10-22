#include "pc_input.h"
#include "modern_controls.h"
#include "host_osd.h"
#include "inventory_hud.h"
#include "weapon_aim.h"
#include "mod_plugins.h"
#include "game_menu.h"
#include "live_env.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <atomic>

extern "C" uint64_t s_frame_count;

namespace ttk {
static SDL_Window* window;
static CheatTyping cheat_typing;
static Cheat cheat_pending=Cheat::None;
static uint64_t cheat_deadline;
void input_notice(const char* message) {host_osd_push_centered(message,3000);std::fprintf(stderr,"[TTK cheat] %s\n",message);}
Cheat input_take_cheat() {
    auto result=cheat_pending;cheat_pending=Cheat::None;
    return result;
}
static bool modern, captured, focused, autorun;
// Keep ownership through clear()/focus/strip expiry until the physical release.
static bool inventory_enter_down;
static bool allow_capture = true;
static std::atomic<bool> capture_offer{false};
static std::atomic<uint64_t> capture_offer_at{0};
static constexpr uint64_t capture_offer_window=8;
static bool initial_capture, interaction_pending, interaction_started;
static uint64_t jump_deadline,menu_select_deadline,jump_pressed_at;
// Escape keeps the mouse free until the original pause has actually stopped
// gameplay offers: a quick tap that raced the pad poll used to recapture while
// the game kept running. The Start pulse makes a tap reach the pad poll.
static bool escape_hold;
// D19D: uncaptured mouse-wheel notches waiting to move a menu cursor.
static std::atomic<int> menu_wheel{0};
// D19H: menu key-down events (up, down, left, right, confirm, back, square,
// D19S delete) since the menu last read them; taken (and dropped when no menu
// screen owns the keys) on every pad read.
static std::atomic<int> menu_downs[8];
static void note_menu_down(int code) {
    int k=-1;
    switch(code) {
    case SDL_SCANCODE_UP: k=0;break; case SDL_SCANCODE_DOWN: k=1;break;
    case SDL_SCANCODE_LEFT: k=2;break; case SDL_SCANCODE_RIGHT: k=3;break;
    case SDL_SCANCODE_RETURN: case SDL_SCANCODE_X: k=4;break;
    case SDL_SCANCODE_ESCAPE: case SDL_SCANCODE_BACKSPACE: case SDL_SCANCODE_Z: k=5;break;
    case SDL_SCANCODE_V: k=6;break;
    case SDL_SCANCODE_DELETE: k=7;break;
    default: return;
    }
    if(menu_downs[k].load()<6)++menu_downs[k];
}
static uint64_t escape_start_deadline,escape_at;
static bool quick_press(int code,int mods); // D20
static bool airborne_interact;
static uint64_t interaction_deadline, interaction_pulse, capture_after, holster_pulse;
static uint64_t swim_redraw; // D08O3: E redraw in water pending until this frame
// Ownership belongs only to an E-triggered holster. Focus/pause clear held
// buttons but retain this intent; manual holster/selection and death cancel it.
static bool restore_owned;
static uint64_t restore_pulse, restore_settle;
static void cancel_restore() { restore_owned=false;restore_pulse=restore_settle=0; }
// D08T/D08T1 pushable objects (push.inc). Holding Grab / Manipulate while
// touching a pushable object requests the original grab: automatic holster if
// needed, no movement, and Cross only once Duke idles with the weapon stowed
// after at least two released ticks, so 0x80051890 sees a fresh press. While
// Grab stays held the grab latches: Cross stays held while the original
// grab/push/pull runs and W/S become original Up/Down along Duke's facing.
// Releasing Grab lets go. E is never a grab: push.inc masks E's Cross at the
// original idle grab, so E mantles a pushable object like any other.
static bool push_request, push_latched, push_released, push_letting_go;
static uint64_t push_request_at, push_deadline, push_holster_pulse;
static unsigned push_lost;
static bool push_touching;
// D08U: ladder-top hint once per arrival, twice per session.
static bool ladder_top_seen;
// D08J1: one host jump per E hold at an overhead ladder.
static bool ladder_leap_used;
static void push_reset() {push_request=push_latched=false;push_holster_pulse=0;push_lost=0;}
// Any end of a grab needs a fresh Grab press before the next one, and E's
// Cross waits until the original has actually let go.
static void push_let_go() {push_latched=push_request=false;push_released=push_letting_go=true;}
static bool keys[512], mouse[6];
static float dx, dy;
static double wheel_fraction, distance_total;
// D10 camera: recenter presses are counted per epoch (clear() resets them);
// the shoulder side survives capture changes and starts from the profile.
static uint64_t recenter_total,look_press_total;
static int shoulder;
static bool first_person;
static bool independent_camera() {
    const char* mode=ttk_live_env("DNTTK_CAMERA_MODE");
    return !mode || !std::strcmp(mode,"independent");
}
static void camera_action(int action) {
    if(!independent_camera())return;
    if(action==camera_recenter) {++recenter_total;return;}
    if(action==look_up || action==look_down) {++look_press_total;return;}
    if(action==camera_view) {
        first_person=!first_person;
        const char* text=first_person?"CAMERA: FIRST PERSON":"CAMERA: THIRD PERSON";
        host_osd_push_centered(text,1500);
        std::fprintf(stderr,"[TTK input] %s\n",text);
        return;
    }
    shoulder=shoulder==0?1:shoulder==1?-1:0;
    const char* text=shoulder>0?"CAMERA: RIGHT SHOULDER":shoulder<0?"CAMERA: LEFT SHOULDER":"CAMERA: CENTERED";
    host_osd_push_centered(text,1500);
    std::fprintf(stderr,"[TTK input] %s\n",text);
}
static int commands[32];
static unsigned command_count;
static uint64_t command_serial,command_deadline;
static uint64_t sequence, epoch;
// D08T2: hints follow every fresh contact or grab, at most once per cooldown
// (about five seconds), instead of only the first few of a session.
static constexpr uint64_t hint_cooldown=300;
static uint64_t push_notice_at, push_hint_at, ladder_hint_at;
static bool hint_due(uint64_t& at) {
    if(at && sequence-at<hint_cooldown)return false;
    at=sequence;return true;
}
static void command(int action) {
    // D08A5: the mission inventory is host-side, never a guest command.
    if(action==mission_previous || action==mission_next) {mission_browse_press(action==mission_next?1:-1);return;}
    if(command_count<32) {
        if(!command_count)command_deadline=sequence+8;
        commands[command_count++]=action;++command_serial;
    }
    if(action>=weapon_previous && action<=weapon_last || action>=weapon_group_1 && action<=weapon_group_0)cancel_restore();
}
static InputFrame frame, empty;
static double total_x, total_y;
// D17B: consumed mouse motion with its event time (presenter clock, ms), so a
// late camera can take the look as it was at a fixed time before each
// present: sampling "now" at irregular submission times made even turning
// uneven (2026-10-03, about half the presents at 180 Hz).
struct LookSample {double ms,x,y;};
static LookSample look_ring[512];
static unsigned look_n;
static double look_cx, look_cy;
static double event_ms(const SDL_Event& e) {
    // SDL event time on the SDL tick clock, moved to the performance clock.
    const double perf_ms=(double)SDL_GetPerformanceCounter()*1000.0/(double)SDL_GetPerformanceFrequency();
#if defined(PSX_SDL3)
    const double ticks_ms=(double)SDL_GetTicksNS()/1e6, at=(double)e.motion.timestamp/1e6;
#else
    const double ticks_ms=(double)SDL_GetTicks(), at=(double)e.motion.timestamp;
#endif
    return perf_ms-(ticks_ms-at);
}
static Device device;
static int binds[action_count] = {
#define TTK_ACTION(name, token, code, pad) code,
#include "input_bindings.def"
#undef TTK_ACTION
};
static const uint16_t pads[action_count] = {
#define TTK_ACTION(name, token, code, pad) pad,
#include "input_bindings.def"
#undef TTK_ACTION
};
// D19G: KEY BINDINGS waits for a key (scancode, mouse button -n).
static std::atomic<bool> key_capture{false};
static std::atomic<int> captured_key{0};
static constexpr int no_key=0x7fffffff;
static void clear() {
    cheat_typing.reset();cheat_pending=Cheat::None;menu_select_deadline=0;
    std::memset(keys, 0, sizeof keys);
    std::memset(mouse, 0, sizeof mouse);
    dx = dy = 0;wheel_fraction=distance_total=0;recenter_total=look_press_total=0;command_count=0;
    frame = {};jump_deadline=jump_pressed_at=0;airborne_interact=false;
    interaction_pending=false;interaction_pulse=0;interaction_started=false;holster_pulse=0;swim_redraw=0;
    push_reset();
    ++epoch; total_x = total_y = 0; look_n=0; look_cx=look_cy=0;
    device = Device::None;
}
// Cheat typing stops held movement keys without touching mouse look (D08G5).
static void release_keys() {
    std::memset(keys, 0, sizeof keys);
    jump_deadline=jump_pressed_at=0;
}
static bool relative(bool enabled) {
#if defined(PSX_SDL3)
    return SDL_SetWindowRelativeMouseMode(window, enabled);
#else
    return SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE) == 0;
#endif
}
// clear() forgets every key. A modifier or movement key that is physically
// held across F7 load, Escape/resume or inventory recapture used to stay dead
// until re-pressed (Shift held: Duke walked; Shift looked broken). Take the
// bound gameplay actions from SDL's live device state instead. Fixed menu keys
// (Escape/Enter/arrows/F-keys) are edge-driven and stay cleared on purpose.
static void resync_held() {
    if(!window)return;
    int count=0;
#if defined(PSX_SDL3)
    const auto* state=SDL_GetKeyboardState(&count);
    const auto buttons=SDL_GetMouseState(nullptr,nullptr);
#if defined(SDL_BUTTON_MASK)
#define TTK_BUTTON_MASK(b) SDL_BUTTON_MASK(b)
#else
#define TTK_BUTTON_MASK(b) SDL_BUTTON(b)
#endif
#else
    const auto* state=SDL_GetKeyboardState(&count);
    const Uint32 buttons=SDL_GetMouseState(nullptr,nullptr);
#define TTK_BUTTON_MASK(b) SDL_BUTTON(b)
#endif
    for(int i=0;i<action_count;++i) {
        const int code=binds[i];
        if(code>0 && code<512 && code<count && state) keys[code]=state[code]!=0;
        if(code==SDL_SCANCODE_LALT && SDL_SCANCODE_RALT<count && state) keys[SDL_SCANCODE_RALT]=state[SDL_SCANCODE_RALT]!=0;
        else if(code<0 && -code<6) mouse[-code]=(buttons & TTK_BUTTON_MASK(-code))!=0;
    }
#undef TTK_BUTTON_MASK
}
static bool capture_now() {
    clear();escape_hold=false;
    captured=relative(true);
    if(captured)resync_held();
    return captured;
}
// A savestate load replaced the guest state: host requests that described
// the old state (stow, weapon restore, push, queued jumps) must not act on it.
void input_state_loaded() {
    interaction_pending=false;interaction_pulse=0;interaction_started=false;holster_pulse=0;swim_redraw=0;
    airborne_interact=false;cancel_restore();push_reset();jump_deadline=0;jump_pressed_at=0;
}
void input_release() {
    if (window && captured) relative(false);
    captured = false;
    capture_after=sequence+12;
    clear();
}
void input_release_for_menu() {if(captured)input_release();}
void input_allow_capture(bool allow) {
    allow_capture = allow;
    if (!allow) input_release();
}
void input_pad_context(uint16_t pad) {
    if (captured && (pad & 9) != 9) {
        input_release();
        // Capture release clears held gameplay keys. Preserve the Select request
        // for six input frames so the original menu update can receive it.
        // Focus/other resets clear this pulse; it cannot repeat after expiry.
        if(!(pad&1))menu_select_deadline=sequence+6;
    }
}
bool input_wants_initial_capture() { return initial_capture && modern && !captured; }
void input_offer_gameplay_capture() { capture_offer_at.store(sequence); capture_offer.store(true); }
static bool allowed(int code) {
    return code == 0
        || (code >= 4 && code <= 39 && code != 9)
        || code == 71 || code == 224 || code == 226 || code == 47 || code == 48 || code == 51 || code == 52 || code == 44 || code == 54 || code == 55 || code == 225 || code == 74 || code == 75 || code == 77 || code == 78 || code == 79 || code == 80 || (code >= -5 && code <= -1)
        // D20: F4, F5, F6, F9 and F12 (F2 / F3 are SAVE / LOAD GAME; the rest are the framework's).
        || code == 61 || code == 62 || code == 63 || code == 66 || code == 69;
}
void input_init(SDL_Window* w) {
    window = w;inventory_enter_down=false;
    cancel_restore();
    autorun = false;
    initial_capture = !std::getenv("DNTTK_AUTO_CAPTURE") || std::strcmp(std::getenv("DNTTK_AUTO_CAPTURE"),"0");
    capture_offer=false;
    const char* mode = std::getenv("DNTTK_INPUT_MODE");
    modern = mode && std::strcmp(mode, "modernized") == 0;
    const char* side = std::getenv("DNTTK_CAMERA_SHOULDER");
    shoulder = side && !std::strcmp(side,"right") ? 1 : side && !std::strcmp(side,"left") ? -1 : 0;
    const char* view = std::getenv("DNTTK_CAMERA_VIEW");
    first_person = view && !std::strcmp(view,"first");
    focused = w && (SDL_GetWindowFlags(w) & SDL_WINDOW_INPUT_FOCUS);
    clear();
    static const int defaults[action_count] = {
#define TTK_ACTION(name, token, code, pad) code,
#include "input_bindings.def"
#undef TTK_ACTION
    };
    std::copy(defaults, defaults + action_count, binds);
    if (!modern) return;
    const char* spec = std::getenv("DNTTK_INPUT_BINDINGS");
    if (spec) {
        bool valid = std::strncmp(spec, "1:", 2) == 0;
        const char* cursor = spec + (valid ? 2 : 0);
        int parsed[action_count]{};
        for (int i = 0; valid && i < action_count; ++i) {
            char* end;
            long value = std::strtol(cursor, &end, 10);
            valid = end != cursor && value >= -5 && value <= 226 && allowed((int)value)
                && (value != 6 || i == holster)
                && *end == (i + 1 == action_count ? '\0' : ',');
            for (int j = 0; j < i; ++j) if (parsed[j] == value && value != 0) valid = false;
            parsed[i] = (int)value;
            cursor = end + (*end ? 1 : 0);
        }
        if (valid) std::copy(parsed, parsed + action_count, binds);
        else std::fprintf(stderr, "[TTK input] Invalid binding payload; using defaults.\n");
    }
    std::fprintf(stderr, "[TTK input] Modernized controls: automatic gameplay capture; F10 toggles; Escape pauses; Enter uses selected inventory. Unsupported states use original controls.\n");
}
bool input_modernized() { return modern; }
// D19G: bindings changed in the KEY BINDINGS page.
int input_binding_code(Action action) {return action>=0 && action<action_count ? binds[action] : 0;}
int input_default_code(Action action) {
    static const int defaults[action_count] = {
#define TTK_ACTION(name, token, code, pad) code,
#include "input_bindings.def"
#undef TTK_ACTION
    };
    return action>=0 && action<action_count ? defaults[action] : 0;
}
bool input_code_bindable(int code, Action action) {
    return allowed(code) && (code != SDL_SCANCODE_C || action == holster);
}
// Bindings change in the menu, which owns the keys meanwhile; held keys are
// read through binds[] on the next gameplay frame.
void input_set_binding_codes(const int* codes) {
    for (int i = 0; i < action_count; ++i) binds[i] = codes[i];
}
void input_capture_key(bool on) {captured_key.store(no_key);key_capture.store(on);}
int input_take_captured_key() {return captured_key.exchange(no_key);}
int input_no_key() {return no_key;}
// D19G: the view and shoulder the menu sets (P and H change them in game).
bool input_first_person() {return first_person;}
int input_shoulder() {return shoulder;}
void input_set_view(bool first) {first_person=first;}
void input_set_shoulder(int side) {shoulder=side>0?1:side<0?-1:0;}
const char* input_binding_name(Action action) {
    if(action<0 || action>=action_count)return "";
    const int code=binds[action];
    if(code==0)return "Unbound";
    static const char* mouse_names[]={"","Mouse1","Mouse3","Mouse2","Mouse4","Mouse5"};
    return code<0?mouse_names[-code]:SDL_GetScancodeName(SDL_Scancode(code));
}
// An Alt binding means either Alt key.
// Either Alt / either Shift satisfies a left-modifier binding.
static bool down(int code) { return code < 0 ? mouse[-code] : code==SDL_SCANCODE_LALT ? keys[code] || keys[SDL_SCANCODE_RALT] :
    code==SDL_SCANCODE_LSHIFT ? keys[code] || keys[SDL_SCANCODE_RSHIFT] : keys[code]; }
// D08T1: with legacy (original) weapon aiming or the original camera the
// primary Grab input keeps its old precision-aim role; Grab (second) grabs.
static bool legacy_aim() {
    const char* mode=ttk_live_env("DNTTK_WEAPON_AIM");
    return !independent_camera() || !mode || std::strcmp(mode,"view");
}
static bool grab_down() {
    return (binds[grab] && !legacy_aim() && down(binds[grab])) || (binds[grab_alt] && down(binds[grab_alt]));
}
static bool aim_down() {
    return (binds[original_aim] && down(binds[original_aim])) || (binds[grab] && legacy_aim() && down(binds[grab]));
}
// The live input that grabs, for prompts: RMB, ALT, or the bound key's name.
static const char* grab_label() {
    const int code=binds[grab] && !legacy_aim() ? binds[grab] : binds[grab_alt];
    static char text[32];
    if(!code)return "GRAB";
    if(code<0) {static const char* names[]={"","LMB","MMB","RMB","MOUSE4","MOUSE5"};return names[-code];}
    if(code==SDL_SCANCODE_LALT)return "ALT";
    std::snprintf(text,sizeof text,"%s",SDL_GetScancodeName(SDL_Scancode(code)));
    for(char* c=text;*c;++c)*c=char(std::toupper((unsigned char)*c));
    return text;
}
// D08AA: Shift (or Caps Lock) decides walk/run in water as on land; shallow
// water no longer forces the run (D08AB: unless SWIMMING is original).
bool input_running() {
    if(wade_full_speed_ready())return true;
    return autorun != down(binds[walk]);
}
bool input_auto_stow_pending() {
    return modern && focused && captured && (interaction_pending || airborne_interact) && interaction_started && restore_owned && sequence<=interaction_deadline;
}
// D08X: E is held (or its press is still pending, or the airborne reach it
// started is live) during a jump: the host may arm the original reach early.
bool input_airborne_reach_held() {
    return modern && focused && captured && !push_latched && !push_request &&
        (down(binds[interact]) || airborne_interact || interaction_pending || ladder_leap_active());
}
bool input_jump_pending() {
    return modern && focused && captured && jump_deadline && sequence<=jump_deadline;
}
bool input_push_grab_owns_cross() {
    return modern && focused && captured && (push_request || push_latched);
}
bool input_take_swim_redraw() {
    if(!modern || !focused || !captured || !swim_redraw || sequence>swim_redraw) {swim_redraw=0;return false;}
    swim_redraw=0;return true;
}
bool input_take_jump() {
    if(!input_jump_pending())return false;
    jump_deadline=jump_pressed_at=0;return true;
}

void input_event(const SDL_Event& e) {
    if (!window) return;
#if defined(PSX_SDL3)
    if ((e.type == SDL_EVENT_WINDOW_FOCUS_LOST || e.type == SDL_EVENT_WINDOW_MINIMIZED)
        && e.window.windowID == SDL_GetWindowID(window)) {
        focused = false; input_release(); return;
    }
    if (e.type == SDL_EVENT_WINDOW_FOCUS_GAINED && e.window.windowID == SDL_GetWindowID(window)) {
        focused = true; clear(); return;
    }
#else
    if (e.type == SDL_WINDOWEVENT && e.window.windowID == SDL_GetWindowID(window)) {
        if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST || e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
            focused = false; input_release(); return;
        }
        if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) { focused = true; clear(); return; }
    }
#endif
    if (e.type == SDL_QUIT) { input_release(); return; }
    // SDL can deliver the owned key release after focus has been lost.
    if(e.type==SDL_KEYUP && e.key.windowID==SDL_GetWindowID(window)) {
#if defined(PSX_SDL3)
        if(e.key.scancode==SDL_SCANCODE_RETURN)inventory_enter_down=false;
#else
        if(e.key.keysym.scancode==SDL_SCANCODE_RETURN)inventory_enter_down=false;
#endif
    }
    if (!modern || !focused) return;
    // D19G: the KEY BINDINGS page waits for one key or mouse button; it goes
    // to the menu alone (no menu taps, cheats, capture or gameplay action).
    if (key_capture.load()) {
        if (e.type == SDL_KEYDOWN && e.key.windowID == SDL_GetWindowID(window) && !e.key.repeat) {
#if defined(PSX_SDL3)
            captured_key.store((int)e.key.scancode);
#else
            captured_key.store((int)e.key.keysym.scancode);
#endif
            return;
        }
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.windowID == SDL_GetWindowID(window)) {
            if (e.button.button >= 1 && e.button.button < 6) captured_key.store(-int(e.button.button));
            return;
        }
        if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEWHEEL) return;
    }
    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
        if (e.key.windowID != SDL_GetWindowID(window)) return;
#if defined(PSX_SDL3)
        int code = e.key.scancode;
        int mods = e.key.mod;
#else
        int code = e.key.keysym.scancode;
        int mods = e.key.keysym.mod;
#endif
        bool pressed = e.type == SDL_KEYDOWN;
        if (pressed && e.key.repeat) return;
        if(pressed)note_menu_down(code);
        // Enter uses the selected gadget while captured (EDuke Inventory). Escape pauses.
        if(code==SDL_SCANCODE_RETURN && pressed) {
            if(inventory_enter_down)return;
            if(captured && !(mods&(KMOD_CTRL|KMOD_ALT|KMOD_GUI))) {
                inventory_enter_down=true;command(item_use);return;
            }
        }
        // F10 that frees the mouse opts out of automatic capture; F10 that
        // captures opts back in, so console travel and later releases recapture
        // as before (D22C: a stale opt-out left every next level uncaptured).
        if (pressed && code == SDL_SCANCODE_F10) {
            initial_capture=!captured;
            if (captured) input_release();
            else if (allow_capture) capture_now();
            std::fprintf(stderr, "[TTK input] Mouse %s%s\n", captured ? "captured" : "released",
                         captured ? " (F10 releases)" : "");
            return;
        }
        // D08G5: typing only owns the keyboard. Mouse look stays live, and a key
        // that breaks the sequence acts at once as its normal gameplay key.
        if(captured && !(mods&(KMOD_CTRL|KMOD_ALT|KMOD_GUI))) {
            const bool typing=cheat_typing.active();
            if(pressed && code>=SDL_SCANCODE_A && code<=SDL_SCANCODE_Z) {
                auto result=cheat_typing.feed(char('a'+code-SDL_SCANCODE_A),sequence);
                if(cheat_typing.active() || result!=Cheat::None) {
                    if(!typing)release_keys();
                    if(result!=Cheat::None) {cheat_pending=result;cheat_deadline=sequence+120;}
                    return;
                }
            } else if(typing && pressed) {
                cheat_typing.reset();
            } else if(typing) {
                return;
            }
        }
        if(pressed && code!=0 && quick_press(code,mods)) return;
        // Escape, F7 savestate and other host overlays return to menu input.
        // F10 opts out of auto-capture. F7 must request recapture even if the
        // mouse is already free: otherwise loading slot 2 leaves WASD/mouse
        // dead. Enter never pauses.
        if (pressed && (code == SDL_SCANCODE_ESCAPE
            || (code >= SDL_SCANCODE_F1 && code <= SDL_SCANCODE_F12)
            || (mods & KMOD_GUI) || (code==SDL_SCANCODE_TAB && (mods & KMOD_ALT)))) {
            if(code!=SDL_SCANCODE_F10)initial_capture=true;
            const bool was_captured=captured;
            input_release();
            // Only an Escape from running gameplay (captured, or offers still
            // arriving) holds recapture. Escape in the pause menu resumes and
            // must let the resumed offers recapture at once.
            if(code==SDL_SCANCODE_ESCAPE && allow_capture) {
                escape_start_deadline=sequence+6;
                if(was_captured || sequence-capture_offer_at.load()<=capture_offer_window) {
                    escape_hold=true;escape_at=sequence;
                }
                if(was_captured)std::fprintf(stderr,"[TTK input] Mouse released (Escape)\n");
            }
        }
        if (pressed && captured && code == SDL_SCANCODE_CAPSLOCK) {
            autorun = !autorun;
            input_notice(autorun ? "RUN MODE ON" : "RUN MODE OFF");
            std::fprintf(stderr, "[TTK input] Autorun %s\n", autorun ? "on" : "off");
        }
        // Inventory sits before weapon_previous in Action order, so it is not in the
        // edge-command loop below. I toggles the crosshair; RShift still opens Select.
        if(pressed && captured && code==binds[inventory] &&
           !(mods&(KMOD_CTRL|KMOD_ALT|KMOD_GUI))) {
            ttk_aim_toggle_crosshair();
            input_notice(ttk_aim_crosshair_enabled()?"CROSSHAIR ON":"CROSSHAIR OFF");
        }
        // D10C look keys (after mission_next) are held, never commands.
        if(pressed && captured)for(int i=weapon_previous;i<=mission_next;++i)
            if(code==binds[i])command(i);
        if(pressed && captured)for(int i:{camera_recenter,camera_shoulder,camera_view,look_up,look_down})
            if(code==binds[i])camera_action(i);
        if(code==SDL_SCANCODE_LALT || code==SDL_SCANCODE_RALT)wheel_fraction=0;
        if (code >= 0 && code < 512) keys[code] = pressed;
        if (pressed) device = Device::Keyboard;
        // Space is always jump; while Duke holds an object it is ignored.
        if(pressed && captured && code==binds[jump] && !push_latched) {
            jump_deadline=sequence+8;jump_pressed_at=sequence;
            // Armed run-jumps often press E during flight. If E is already held
            // at takeoff, keep/refresh the stow lease so the weapon clears early.
            if(down(binds[interact]) || interaction_pending) {
                interaction_pending=true;interaction_deadline=sequence+120;
            }
        }
        // E is always the normal interaction/mantle; during a grab it lets go first.
        if(pressed && captured && code==binds[interact]) {
            if(push_latched || push_request)push_let_go();
            interaction_pending=true;interaction_deadline=sequence+120;interaction_pulse=0;interaction_started=false;holster_pulse=0;
        }
        if(pressed && captured && code==binds[holster]) {
            cancel_restore();airborne_interact=false;interaction_pending=false;interaction_pulse=0;interaction_started=false;holster_pulse=0;swim_redraw=0;
            // Deliberate holster key only — E auto-stow never publishes these quotes.
            if(weapon_drawn())input_notice("WEAPON LOWERED");
            else if(weapon_holstered() || fire_draw_ready())input_notice("WEAPON RAISED");
        }
    } else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.windowID != SDL_GetWindowID(window)) return;
        if(e.type==SDL_MOUSEBUTTONDOWN)cheat_typing.reset();
        if(e.type==SDL_MOUSEBUTTONDOWN && e.button.button<6 && quick_press(-int(e.button.button),(int)SDL_GetModState())) return;
        if (captured && e.button.button < 6) {
            mouse[e.button.button] = e.type == SDL_MOUSEBUTTONDOWN;
            if(e.type==SDL_MOUSEBUTTONDOWN && binds[inventory]==-int(e.button.button)) {
                ttk_aim_toggle_crosshair();
                input_notice(ttk_aim_crosshair_enabled()?"CROSSHAIR ON":"CROSSHAIR OFF");
            }
            if(e.type==SDL_MOUSEBUTTONDOWN)for(int i=weapon_previous;i<=mission_next;++i)
                if(binds[i]==-int(e.button.button))command(i);
            if(e.type==SDL_MOUSEBUTTONDOWN)for(int i:{camera_recenter,camera_shoulder,camera_view,look_up,look_down})
                if(binds[i]==-int(e.button.button))camera_action(i);
            device = Device::Mouse;
            if(e.type==SDL_MOUSEBUTTONDOWN && binds[jump]==-int(e.button.button) && !push_latched) {
                jump_deadline=sequence+8;
                if(down(binds[interact]) || interaction_pending) {
                    interaction_pending=true;interaction_deadline=sequence+120;
                }
            }
            if(e.type==SDL_MOUSEBUTTONDOWN && binds[interact]==-int(e.button.button)) {
                if(push_latched || push_request)push_let_go();
                interaction_pending=true;interaction_deadline=sequence+120;interaction_pulse=0;interaction_started=false;holster_pulse=0;
            }
            if(e.type==SDL_MOUSEBUTTONDOWN && binds[holster]==-int(e.button.button)) {
                cancel_restore();airborne_interact=false;
                interaction_pending=false;interaction_pulse=0;interaction_started=false;holster_pulse=0;swim_redraw=0;
                if(weapon_drawn())input_notice("WEAPON LOWERED");
                else if(weapon_holstered() || fire_draw_ready())input_notice("WEAPON RAISED");
            }
        }
    } else if (e.type == SDL_MOUSEWHEEL && !captured && e.wheel.windowID == SDL_GetWindowID(window)) {
        // D19D: uncaptured, the wheel moves a menu cursor (input_pad()).
        double amount=e.wheel.y;
        if(e.wheel.direction==SDL_MOUSEWHEEL_FLIPPED)amount=-amount;
        if(!std::isfinite(amount) || !amount)return;
        const int step=amount>0?1:-1, pending=menu_wheel.load();
        if(pending*step<0)menu_wheel.store(step);
        else if(std::abs(pending)<6)menu_wheel.store(pending+step);
    } else if (e.type == SDL_MOUSEWHEEL && captured && e.wheel.windowID == SDL_GetWindowID(window)) {
        double amount=e.wheel.y;
        if(e.wheel.direction==SDL_MOUSEWHEEL_FLIPPED)amount=-amount;
        if(!std::isfinite(amount))return;
        amount=std::clamp(amount,-32.0,32.0);
        if(keys[SDL_SCANCODE_LALT] || keys[SDL_SCANCODE_RALT]) {
            wheel_fraction=0;distance_total+=amount;
        } else {
            wheel_fraction+=amount;
            int steps=std::clamp(int(wheel_fraction),-32,32);wheel_fraction-=steps;
            for(int i=0;i<std::abs(steps);++i)command(steps>0?weapon_previous:weapon_next);
        }
    } else if (e.type == SDL_MOUSEMOTION && captured && e.motion.windowID == SDL_GetWindowID(window)) {
        dx = std::clamp(dx + (float)e.motion.xrel, -32768.f, 32768.f);
        look_cx+=e.motion.xrel; look_cy+=e.motion.yrel;
        look_ring[look_n++%512]={event_ms(e),look_cx,look_cy};
        dy = std::clamp(dy + (float)e.motion.yrel, -32768.f, 32768.f);
        device = Device::Mouse;
    }
}
uint64_t input_host_frame() { return sequence; }
static std::atomic<bool> interaction_request{false};
void input_request_interaction() {interaction_request.store(true);}
void input_frame() {
    // D08A18: Enter on a mission item at its lock: the same pending interaction an E press starts.
    if(interaction_request.exchange(false) && modern && focused && captured && !push_latched && !push_request) {
        interaction_pending=true;interaction_deadline=sequence+120;interaction_pulse=0;interaction_started=false;holster_pulse=0;
    }
    if(sequence>cheat_typing.deadline)cheat_typing.reset();
    if(sequence>cheat_deadline)cheat_pending=Cheat::None;
    // Offers repeat every player update while gameplay runs; one that stopped
    // arriving (original pause menu opened right after Escape released the
    // mouse) must not capture inside that menu, where Enter/X would be eaten.
    // Host overlays (F7) do not advance sequence, so their offer stays fresh.
    // Escape's hold ends once offers stop (pause menu open); resume recaptures.
    // No offers arrive while captured, so the gap counts from the Escape press.
    if(escape_hold && (!allow_capture ||
       sequence-std::max<uint64_t>(capture_offer_at.load(),escape_at)>capture_offer_window))escape_hold=false;
    if (initial_capture && !escape_hold && sequence>=capture_after && modern && focused && allow_capture && !captured
        && capture_offer.load() && sequence-capture_offer_at.load()<=capture_offer_window) {
        capture_offer.store(false);
        if(capture_now()) {std::fprintf(stderr,"[TTK input] Gameplay captured automatically\n");}
    }
    if(!(modern && focused && captured)) {push_reset();push_released=push_letting_go=false;}
    else {
        const bool grabbed=push_grab_ready();
        const bool want=grab_down();
        if(push_latched) {
            // Releasing Grab lets go. Hit reactions, a blocked object or the
            // original letting go end it too; a short gap covers the
            // grab/push/pull transitions. Any end needs a fresh Grab press.
            if(!want)push_let_go();
            else if(grabbed)push_lost=0;
            else if(++push_lost>12)push_let_go();
        }
        if(push_request && !want)push_let_go();
        if(push_request && grabbed) {
            push_latched=true;push_request=false;push_lost=0;
            if(hint_due(push_notice_at)) {
                char text[64];
                std::snprintf(text,sizeof text,"W/S PUSH/PULL - RELEASE %s TO LET GO",grab_label());
                host_osd_push_centered(text,2500);
                std::fprintf(stderr,"[TTK input] Hint: %s\n",text);
            }
            std::fprintf(stderr,"[TTK input] Pushable object grabbed\n");
        }
        if(push_request && sequence>push_deadline)push_let_go();
        const bool touching=push_contact_ready();
        if(!push_released && !push_latched && !push_request && want && touching) {
            push_request=true;push_request_at=sequence;push_deadline=sequence+90;push_holster_pulse=0;
            if(interaction_holster_ready()) {
                // Only a stowed Duke can grab; redraw after, as for E interactions.
                push_holster_pulse=sequence+4;restore_owned=true;restore_pulse=restore_settle=0;
            }
        }
        // Discoverability: a fresh touch names the grab input.
        if(touching && !push_touching && !want && !push_latched && hint_due(push_hint_at)) {
            char text[48];
            std::snprintf(text,sizeof text,"HOLD %s TO GRAB",grab_label());
            host_osd_push_centered(text,2000);
            std::fprintf(stderr,"[TTK input] Hint: %s\n",text);
        }
        push_touching=touching;
        if(!want)push_released=false;
        if(!grabbed)push_letting_go=false;
        // D08U discoverability: a ladder top names E (D08T2 cooldown).
        const bool ladder_top=ladder_top_available();
        if(ladder_top && !ladder_top_seen && hint_due(ladder_hint_at)) {
            char text[48];
            std::snprintf(text,sizeof text,"%s TO CLIMB DOWN",input_binding_name(interact));
            host_osd_push_centered(text,2000);
            std::fprintf(stderr,"[TTK input] Hint: %s\n",text);
        }
        ladder_top_seen=ladder_top;
    }
    if(interaction_pending && (sequence>interaction_deadline || down(binds[fire])))interaction_pending=false;
    if(interaction_pending && !interaction_started && interaction_holster_ready()) {
        // Shorter Circle pulse once jumping/airborne so stow begins inside the
        // ladder contact window; grounded approach keeps a slightly longer tap.
        const bool urgent=airborne_input_ready() || (jump_deadline && sequence<=jump_deadline);
        holster_pulse=sequence+(urgent?2:4);interaction_started=true;restore_owned=true;restore_pulse=restore_settle=0;
    }
    // D08J1: E held (or just pressed) heading at a ladder that hangs out of
    // reach: press the original jump once, as Space would; the E reach catches.
    if(!down(binds[interact]) && !interaction_pending)ladder_leap_used=false;
    else if(captured && focused && !ladder_leap_used && !push_latched && !push_request &&
            !input_jump_pending() && ladder_leap_ready()) {
        ladder_leap_used=true;ladder_leap_note();
        jump_deadline=sequence+8;jump_pressed_at=sequence;
        interaction_pending=true;interaction_deadline=sequence+120;
    }
    if(airborne_interact && (!airborne_input_ready() || down(binds[fire])))airborne_interact=false;
    if(airborne_input_ready() && interaction_ready() &&
       (down(binds[interact]) || (interaction_pulse && sequence<=interaction_pulse)))airborne_interact=true;
    // D08U: E at the top of a ladder climbs down onto it (after any E stow).
    if(interaction_pending && interaction_ready() && ladder_top_available()) {
        ladder_top_request();interaction_pending=false;interaction_pulse=0;airborne_interact=false;
    }
    if(interaction_pending && interaction_ready()) {
        airborne_interact=airborne_input_ready();
        interaction_pulse=sequence+8;interaction_pending=false;
    }
    if(restore_owned && !interaction_alive())cancel_restore();
    if(restore_owned && captured && focused) {
        if(restore_pulse) {
            // One short original draw request, never a held/repeated toggle.
            if(sequence>restore_pulse)cancel_restore();
        } else if(!interaction_pending && !push_request && !push_latched && sequence>interaction_pulse &&
                  !down(binds[interact]) && (interaction_restore_ready() || interaction_swim_restore_ready())) {
            if(!restore_settle)restore_settle=sequence;
            // D08O3: in water the draw is the original weapon request instead.
            if(sequence-restore_settle>=6 && interaction_swim_restore_ready()) {swim_redraw=sequence+30;cancel_restore();}
            else if(sequence-restore_settle>=6)restore_pulse=sequence+6;
        } else restore_settle=0;
    }
    if(command_count && sequence>command_deadline)command_count=0;
    // Performance diagnostics only (DNTTK_TEST_DRIVE=<mouse counts per frame>):
    // act focused and captured without a window (SDL offscreen on the real
    // GPU) and hold Shift + W with a constant mouse turn, so a captured
    // Modernized route can be timed reproducibly. Never set by run.py.
    static const char* test_drive=std::getenv("DNTTK_TEST_DRIVE");
    if(test_drive && modern) {
        focused=captured=true;keys[SDL_SCANCODE_W]=true;keys[SDL_SCANCODE_LSHIFT]=true;
        dx+=std::atof(test_drive);
        // DNTTK_TEST_KICK=<frames>: also tap the quick kick that often.
        static const long test_kick=[]{const char* t=std::getenv("DNTTK_TEST_KICK");return t?std::atol(t):0L;}();
        if(test_kick>0 && sequence%(uint64_t)test_kick==0 && command_count<32){
            if(!command_count)command_deadline=sequence+8;
            commands[command_count++]=quick_kick;++command_serial;}
    }
    // Diagnostics only (DNTTK_TEST_INPUT=<file>): a script steers the
    // offscreen game. The file is re-read every input frame: held key names
    // (w a s d shift ctrl space e q up down enter left right x z c esc v backspace
    // pgup pgdn home end delete),
    // cheat=<word>, act=<action> (D07E) and quick=<save/load/savemenu/loadmenu>
    // (D20; once per change of word) and
    // dx=<counts> dy=<counts>
    // per frame.
    // Never set by run.py.
    static const char* test_input=std::getenv("DNTTK_TEST_INPUT");
    if(test_input && modern) {
        // DNTTK_TEST_UNCAPTURED: menus as a player sees them (X/Z/C are pad buttons).
        // D20: a file without a quick= word ends the last one (the same word again acts again).
        static char quick_last[16];bool quick_seen=false;
        static const bool uncaptured=std::getenv("DNTTK_TEST_UNCAPTURED")!=nullptr;
        focused=true; captured=!uncaptured;
        static const struct {const char* name;SDL_Scancode code;} named[]={{"w",SDL_SCANCODE_W},{"a",SDL_SCANCODE_A},
            {"s",SDL_SCANCODE_S},{"d",SDL_SCANCODE_D},{"shift",SDL_SCANCODE_LSHIFT},{"ctrl",SDL_SCANCODE_LCTRL},
            {"space",SDL_SCANCODE_SPACE},{"e",SDL_SCANCODE_E},{"q",SDL_SCANCODE_Q},
            {"up",SDL_SCANCODE_UP},{"down",SDL_SCANCODE_DOWN},{"enter",SDL_SCANCODE_RETURN},
            {"left",SDL_SCANCODE_LEFT},{"right",SDL_SCANCODE_RIGHT},{"x",SDL_SCANCODE_X},{"z",SDL_SCANCODE_Z},
            {"c",SDL_SCANCODE_C},{"esc",SDL_SCANCODE_ESCAPE},{"v",SDL_SCANCODE_V},{"backspace",SDL_SCANCODE_BACKSPACE},
            {"pgup",SDL_SCANCODE_PAGEUP},{"pgdn",SDL_SCANCODE_PAGEDOWN},{"home",SDL_SCANCODE_HOME},{"end",SDL_SCANCODE_END},{"delete",SDL_SCANCODE_DELETE}};
        bool was[sizeof named/sizeof named[0]];
        for(size_t i=0;i<sizeof named/sizeof named[0];++i){was[i]=keys[named[i].code];keys[named[i].code]=false;}
        if(FILE* f=std::fopen(test_input,"r")) {
            char word[64];
            while(std::fscanf(f,"%63s",word)==1) {
                if(!std::strncmp(word,"cheat=",6)) {
                    static char last[24];
                    if(std::strcmp(last,word+6)) {
                        std::snprintf(last,sizeof last,"%s",word+6);
                        for(auto code:cheat_codes) if(!std::strcmp(last,code.text)) {cheat_pending=code.action;cheat_deadline=sequence+120;}
                    }
                }
                else if(!std::strncmp(word,"act=",4)) {
                    static const char* const names[action_count]={
#define TTK_ACTION(name, token, code, pad) #name,
#include "input_bindings.def"
#undef TTK_ACTION
                    };
                    static char last[32];
                    if(std::strcmp(last,word+4)) {
                        std::snprintf(last,sizeof last,"%s",word+4);
                        for(int i=0;i<action_count;++i) if(!std::strcmp(last,names[i]))command(i);
                    }
                }
                // D20: quick=save|load|savemenu|loadmenu, once per change (F6 / F9 / F2 / F3).
                else if(!std::strncmp(word,"quick=",6)) {
                    quick_seen=true;
                    if(std::strcmp(quick_last,word+6)) {
                        std::snprintf(quick_last,sizeof quick_last,"%s",word+6);
                        if(!std::strcmp(quick_last,"save")) input_quick(Quick::Save);
                        else if(!std::strcmp(quick_last,"load")) input_quick(Quick::Load);
                        else if(!std::strcmp(quick_last,"savemenu")) input_quick(Quick::SaveMenu);
                        else if(!std::strcmp(quick_last,"loadmenu")) input_quick(Quick::LoadMenu);
                    }
                }
                // D19G: "play" is captured gameplay for this frame (menus run uncaptured).
                else if(!std::strcmp(word,"play")) captured=true;
                else if(!std::strncmp(word,"dx=",3)) dx+=std::atof(word+3);
                else if(!std::strncmp(word,"dy=",3)) dy+=std::atof(word+3);
                else for(const auto& n:named) if(!std::strcmp(word,n.name)) keys[n.code]=true;
            }
            std::fclose(f);
            if(!quick_seen) quick_last[0]=0;
        }
        // DNTTK_TEST_EDGES: keys change only when the file does, as a real
        // keyboard's do (a held key that the input layer clears stays clear).
        {
            static const bool edges=std::getenv("DNTTK_TEST_EDGES")!=nullptr;
            static bool wanted[sizeof named/sizeof named[0]];
            bool changed=false;
            for(size_t i=0;i<sizeof named/sizeof named[0];++i) if(wanted[i]!=keys[named[i].code]) {changed=true;wanted[i]=keys[named[i].code];}
            if(edges && !changed) for(size_t i=0;i<sizeof named/sizeof named[0];++i) keys[named[i].code]=was[i];
        }
        // D19D: "wheelup" / "wheeldown" scroll one notch when they appear.
        {
            static bool wheel_was[2];bool wheel_now[2]{};
            if(FILE* f=std::fopen(test_input,"r")) {
                char word[64];
                while(std::fscanf(f,"%63s",word)==1) {
                    if(!std::strcmp(word,"wheelup"))wheel_now[0]=true;
                    if(!std::strcmp(word,"wheeldown"))wheel_now[1]=true;
                }
                std::fclose(f);
            }
            for(int i=0;i<2;++i) if(wheel_now[i] && !wheel_was[i] && window) {
                SDL_Event e{};e.type=SDL_MOUSEWHEEL;e.wheel.windowID=SDL_GetWindowID(window);e.wheel.y=i?-1:1;
                SDL_PushEvent(&e);
            }
            wheel_was[0]=wheel_now[0];wheel_was[1]=wheel_now[1];
        }
        // Edges become key events, so press-driven actions (E) run as for a
        // real keyboard (queued; handled by the next event pump).
        for(size_t i=0;i<sizeof named/sizeof named[0];++i) if(was[i]!=keys[named[i].code] && window) {
            SDL_Event e{};e.type=keys[named[i].code]?SDL_KEYDOWN:SDL_KEYUP;
            e.key.windowID=SDL_GetWindowID(window);
#if defined(PSX_SDL3)
            e.key.scancode=named[i].code;
#else
            e.key.keysym.scancode=named[i].code;
#endif
            SDL_PushEvent(&e);
        }
    }
    frame = {};
    frame.sequence = ++sequence; frame.epoch = epoch;
    if (modern && focused && captured) {
        frame.active = true;
        frame.command_serial=command_serial;frame.command_count=command_count;
        std::copy(commands,commands+command_count,frame.commands);frame.distance_total=distance_total;
        frame.recenter_total=recenter_total;frame.look_press_total=look_press_total;frame.shoulder=shoulder;frame.first_person=first_person;
        total_x += dx; total_y += dy;
        frame.total_x = total_x; frame.total_y = total_y;
        for (int i = 0; i < action_count; ++i) frame.held[i] = down(binds[i]);
        frame.held[original_aim]=aim_down();frame.held[grab]=grab_down();frame.held[grab_alt]=false;
        frame.move_x = float(frame.held[move_right]) - float(frame.held[move_left]);
        frame.move_y = float(frame.held[move_forward]) - float(frame.held[move_back]);
        float magnitude = std::hypot(frame.move_x, frame.move_y);
        if (magnitude > 1) { frame.move_x /= magnitude; frame.move_y /= magnitude; }
        frame.arrow_y = float(keys[SDL_SCANCODE_UP]) - float(keys[SDL_SCANCODE_DOWN]);
        frame.look_x = dx; frame.look_y = dy;
        frame.device = device;
        // In guarded view-aim locomotion Mouse2 keeps the same view policy.
        // Resolve the action before gameplay consumers see it; explicit original
        // aim/camera, unsupported states and Vanilla retain the precision input.
        if(view_aim_input_ready())frame.held[original_aim]=false;
    }
    dx = dy = 0;
}
double input_clock_ms() {
    return (double)SDL_GetPerformanceCounter()*1000.0/(double)SDL_GetPerformanceFrequency();
}
bool input_live_look(uint64_t& out_epoch, double& x, double& y, double at_ms) {
    if(!modern || !focused || !captured || !window) return false;
    double px=dx,py=dy;
    // Samples: consumed motion, then the peeked queue continuing the sums.
    static LookSample s[640];
    int ns=0;
    const unsigned first=look_n>512?look_n-512:0;
    for(unsigned i=first;i<look_n;++i) s[ns++]=look_ring[i%512];
    double cx=look_cx, cy=look_cy;
    // Motion arrives with the runtime's event pump once per guest field; peek
    // at what the OS has delivered since, so a late camera sees it now. The
    // events stay queued and reach input_event() at the next pump as usual.
    static const bool peek=[]{const char* t=std::getenv("DNTTK_LATE_PEEK");return !(t && t[0]=='0' && !t[1]);}();
    if(peek) {
        SDL_PumpEvents();
        SDL_Event events[128];
        const int n=SDL_PeepEvents(events,128,SDL_PEEKEVENT,SDL_MOUSEMOTION,SDL_MOUSEMOTION);
        const auto id=SDL_GetWindowID(window);
        for(int i=0;i<n;++i) if(events[i].motion.windowID==id) {
            px+=events[i].motion.xrel;py+=events[i].motion.yrel;
            cx+=events[i].motion.xrel;cy+=events[i].motion.yrel;
            if(ns<640) s[ns++]={event_ms(events[i]),cx,cy};
        }
    }
    out_epoch=epoch; x=total_x+px; y=total_y+py;
    if(at_ms<0 || !ns) return true;
    // The look at at_ms: the totals after the last event at or before it,
    // plus the share of the next event's step (spread over at most 8 ms
    // before that event, so a move after a pause does not start early).
    double ax=cx, ay=cy;
    int k=ns;
    while(k>0 && s[k-1].ms>at_ms) --k;
    if(k<ns) {
        const LookSample& b=s[k];
        // Before the first kept sample: 0 at the epoch start, else unknown
        // (ring overflow) and taken as that sample.
        const LookSample a=k>0 ? s[k-1] : first==0 ? LookSample{b.ms-8.0,0.0,0.0} : b;
        const double from=std::max(a.ms,b.ms-8.0);
        const double w=b.ms>from ? std::clamp((at_ms-from)/(b.ms-from),0.0,1.0) : 0.0;
        ax=a.x+(b.x-a.x)*w; ay=a.y+(b.y-a.y)*w;
    }
    x-=cx-ax; y-=cy-ay;
    return true;
}
void input_ack_commands(uint64_t serial) {
    if(!serial || serial>command_serial)return;
    unsigned remaining=unsigned(std::min<uint64_t>(command_count,command_serial-serial));
    std::move(commands+command_count-remaining,commands+command_count,commands);
    command_count=remaining;
}
const InputFrame& input_snapshot(Context context) {
    return context == Context::Gameplay ? frame : empty;
}
static uint16_t last_pad=0xffff;
// D08V1: the identity fallback's mouse turn. Horizontal mouse counts bank up
// and are paid out as the original tank turn (D-pad Right 32 / Left 128), one
// input frame of turn per fallback_turn_counts counts, so a flick turns about
// as far as it would turn the modern camera at the default sensitivity.
static constexpr double fallback_turn_counts=11, fallback_turn_cap=fallback_turn_counts*30;
static uint16_t fallback_turn_pad() {
    static double pending;static uint64_t seen;static uint16_t pad;
    if(frame.sequence==seen)return pad;
    if(seen+1!=frame.sequence)pending=0;
    seen=frame.sequence;
    pending=std::clamp(pending+frame.look_x,-fallback_turn_cap,fallback_turn_cap);
    pad=0;
    if(std::abs(pending)>=fallback_turn_counts*0.5) {
        pad=pending>0?32:128;
        pending-=std::copysign(std::min(std::abs(pending),fallback_turn_counts),pending);
    }
    return pad;
}
// D19D: the original menus as a PC player expects them (Modernized only).
// Escape (and Backspace) goes back one level, Enter confirms, the mouse wheel
// moves the cursor and V reaches Square. The PS1 buttons per screen:
// - mode 0x800bcbb0 0, frontend state 0x800d24e4 5 = PRESS START: only Start
//   opens the menu; Enter is Start, Escape nothing (it used to open the menu).
// - mode 0, states 7-10/12 (main menu, difficulty, two player, LOAD GAME,
//   OPTIONS and its screens 0x800d24d2): Cross/Start confirm, Triangle
//   cancels (does nothing on the main menu). Escape is Triangle, Enter Cross.
// - mode 1, pause 0x800be56a 12 (RESUME / LOAD / OPTIONS / QUIT): Triangle is
//   QUIT, so Escape stays Start (resume, D04A) and Enter is Cross (RESUME).
//   OPTIONS (2), SOUND (7), GAME SETTINGS (11), LOAD (5) and the QUIT prompt
//   (12 with 0x800dd1c4 set) cancel with Triangle: Escape is Triangle there.
// - mode 1, death screen (pause 15: CONTINUE / LOAD / RESTART / QUIT): Triangle
//   is QUIT and Start CONTINUE, so Escape does nothing; Enter is Cross. Its
//   confirmations (0x800dd1c6: 1 CONTINUE, 2 QUIT, 3 LOAD, 4 RESTART; 0 on
//   the choice, reset when the screen opens; a stale 4 is left outside it)
//   accept Start as well as Cross, so Escape on REALLY QUIT? used to quit to
//   the game over screen; it is Triangle (CANCEL) now.
// - mode 3, game over (CONTINUE): Enter Cross, Escape Triangle (nothing).
// - CONFIGURE CONTROLLER (options screen 4, in the frontend and the pause
//   menu) assigns whatever button is pressed to the highlighted action until
//   Start ("PRESS START TO CONTINUE") opens its CONFIRM / CANCEL prompt
//   (0x800dd276 set). Escape and Enter are Start there, then Triangle
//   (cancel) and Cross (confirm) in the prompt.
// - mode 2, statistics and SAVE GAME (0x800d2672): Cross confirms, Triangle
//   cancels the save (nothing on the statistics page).
// - anything else (intro FMV mode 0xffff, gameplay): Escape and Enter
//   stay Start as before (skip the intro, pause), Backspace does nothing.
// Each key keeps the button it was pressed as until it is released, so a
// screen that changes under the press (Triangle back to the pause menu) never
// turns it into another button (Start would resume).
enum class MenuScreen {None, PressStart, Menu, PauseTop, DeathTop, ControllerAssign};
static MenuScreen options_screen(uint16_t screen) {
    if(screen==4)return psx_mod_read_half(0x800dd276) ? MenuScreen::Menu : MenuScreen::ControllerAssign;
    return MenuScreen::Menu;
}
static MenuScreen menu_screen() {
    const uint16_t mode=psx_mod_read_half(0x800bcbb0);
    if(mode==0) {
        const uint16_t state=psx_mod_read_half(0x800d24e4);
        if(state==5)return MenuScreen::PressStart;
        if(state==12)return options_screen(psx_mod_read_half(0x800d24d2));
        return state>=7 && state<=10 ? MenuScreen::Menu : MenuScreen::None;
    }
    if(mode==1) {
        const uint16_t pause=psx_mod_read_half(0x800be56a);
        if(pause==12)return psx_mod_read_half(0x800dd1c4) ? MenuScreen::Menu : MenuScreen::PauseTop;
        if(pause==15)return psx_mod_read_half(0x800dd1c6) ? MenuScreen::Menu : MenuScreen::DeathTop;
        if(pause==4)return options_screen(pause);
        return pause==2 || pause==5 || pause==7 || pause==11 ? MenuScreen::Menu : MenuScreen::None;
    }
    return mode==2 || mode==3 ? MenuScreen::Menu : MenuScreen::None;
}
enum class MenuKey {Back, Backspace, Confirm};
static uint16_t menu_key_button(MenuKey key) {
    constexpr uint16_t start=8, triangle=4096, cross=16384;
    switch(menu_screen()) {
    case MenuScreen::PressStart: return key==MenuKey::Confirm ? start : 0;
    case MenuScreen::DeathTop: return key==MenuKey::Confirm ? cross : 0;
    case MenuScreen::Menu: return key==MenuKey::Confirm ? cross : triangle;
    case MenuScreen::PauseTop: return key==MenuKey::Confirm ? cross : start;
    case MenuScreen::ControllerAssign: return start;
    default: return key==MenuKey::Backspace ? 0 : start;
    }
}
struct MenuLatch {bool held=false;uint16_t button=0;};
static MenuLatch latch_escape, latch_backspace, latch_enter;
static uint16_t latched(MenuLatch& latch, bool held, MenuKey key) {
    if(!held) {latch.held=false;return 0;}
    if(!latch.held) {latch.held=true;latch.button=menu_key_button(key);}
    return latch.button;
}
// Wheel notches (menu_wheel, from input_event()) become Up (+) / Down (-)
// taps of menu_tap_fields pressed and released each (video fields; the pause
// and statistics screens read the pad every 2 fields, the frontend hold
// covers its 6).
static constexpr uint64_t menu_tap_fields=4;
static uint16_t menu_wheel_pad() {
    static int dir;static uint64_t until;
    const uint64_t field=s_frame_count;
    if(dir && field<until)return dir>0?16:64;
    if(dir) {dir=0;until=field+menu_tap_fields;}
    if(field<until)return 0;
    const int pending=menu_wheel.load();
    if(!pending)return 0;
    // Plain menu screens only (not PRESS START, the pause menu or the
    // controller assignment view).
    if(menu_screen()!=MenuScreen::Menu) {menu_wheel.store(0);return 0;}
    dir=pending>0?1:-1;menu_wheel.fetch_sub(dir);until=field+menu_tap_fields;
    return dir>0?16:64;
}
// D19: our OPTIONS screens (game_menu.cpp) take the menu keys while they are
// up; the pad then reads as nothing pressed. Keys still held when they hand
// back to an original screen stay released until pressed again, so the press
// that opened SOUND is never SOUND's own Cross.
// D20: what the pause key does, without an Escape: the Start pulse that
// opens the pause menu (here on SAVE / LOAD GAME) and the mouse it frees.
static void pause_from_play() {
    initial_capture=true;
    const bool was_captured=captured;
    input_release();
    escape_start_deadline=sequence+6;
    if(allow_capture && (was_captured || sequence-capture_offer_at.load()<=capture_offer_window)) {
        escape_hold=true;escape_at=sequence;
    }
}
static QuickResult (*quick_handler)(Quick);
void input_set_quick_handler(QuickResult (*handler)(Quick)) {quick_handler=handler;}
QuickResult input_quick(Quick quick) {
    const QuickResult r=modern && quick_handler ? quick_handler(quick) : QuickResult::Ignored;
    // A load recaptures once play resumes, as after an F7 load.
    if(r==QuickResult::Load) {initial_capture=true;input_release();}
    else if(r==QuickResult::Pause) pause_from_play();
    return r;
}
// A press of a quick key (scancode, mouse button -n); true when it was taken.
static bool quick_press(int code,int mods) {
    if(mods&(KMOD_CTRL|KMOD_ALT|KMOD_GUI)) return false;
    Quick q;
    if(code==binds[quick_save]) q=Quick::Save;
    else if(code==binds[quick_load]) q=Quick::Load;
    else if(code==SDL_SCANCODE_F2) q=Quick::SaveMenu;
    else if(code==SDL_SCANCODE_F3) q=Quick::LoadMenu;
    else return false;
    return input_quick(q)!=QuickResult::Ignored;
}
static bool (*menu_handler)(const MenuKeys&);
static bool menu_had_keys;
void input_set_menu_handler(bool (*handler)(const MenuKeys&)) {menu_handler=handler;}
static bool menu_owns_keys() {
    if(!menu_handler)return false;
    MenuKeys k;
    k.up=keys[SDL_SCANCODE_UP];k.down=keys[SDL_SCANCODE_DOWN];
    k.left=keys[SDL_SCANCODE_LEFT];k.right=keys[SDL_SCANCODE_RIGHT];
    k.confirm=keys[SDL_SCANCODE_RETURN] || keys[SDL_SCANCODE_X];
    k.back=keys[SDL_SCANCODE_ESCAPE] || keys[SDL_SCANCODE_BACKSPACE] || keys[SDL_SCANCODE_Z];
    k.square=keys[SDL_SCANCODE_V];
    k.wheel=captured?0:menu_wheel.load();
    int* const taps[8]={&k.up_taps,&k.down_taps,&k.left_taps,&k.right_taps,&k.confirm_taps,&k.back_taps,&k.square_taps,&k.delete_taps};
    for(int i=0;i<8;++i)*taps[i]=captured?0:menu_downs[i].exchange(0);
    if(captured)for(auto& d:menu_downs)d.store(0);
    const bool owns=!captured && menu_handler(k);
    if(owns) {
        if(k.wheel)menu_wheel.fetch_sub(k.wheel);
    } else if(menu_had_keys) {
        for(int code:{SDL_SCANCODE_UP,SDL_SCANCODE_DOWN,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT,SDL_SCANCODE_RETURN,
                      SDL_SCANCODE_X,SDL_SCANCODE_ESCAPE,SDL_SCANCODE_BACKSPACE,SDL_SCANCODE_Z,SDL_SCANCODE_V})keys[code]=false;
        escape_start_deadline=0;
    }
    menu_had_keys=owns;
    return owns;
}
void input_menu_poll() {
    if(cheat_typing.active() || !modern || !focused) return;
    (void)menu_owns_keys();
}
uint16_t input_pad() {
    if(cheat_typing.active())return 65535;
    uint16_t value = 0xffff;
    if (!modern || !focused) return value;
    if(menu_owns_keys())return value;
    if(modern && focused && menu_select_deadline && sequence<=menu_select_deadline)value &= ~1u;
    // D19D: Escape (its Start pulse included), Backspace and Enter as menu keys.
    value &= ~latched(latch_escape,keys[SDL_SCANCODE_ESCAPE] || (escape_start_deadline && sequence<=escape_start_deadline),MenuKey::Back);
    value &= ~latched(latch_backspace,!captured && keys[SDL_SCANCODE_BACKSPACE],MenuKey::Backspace);
    value &= ~latched(latch_enter,keys[SDL_SCANCODE_RETURN],MenuKey::Confirm);
    if(!captured)value &= ~menu_wheel_pad();
    // Unrebindable escape hatch for menus and the original tank movement preview.
    const int fixed[] = {SDL_SCANCODE_UP, SDL_SCANCODE_RIGHT, SDL_SCANCODE_DOWN,
        SDL_SCANCODE_LEFT, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_RSHIFT, SDL_SCANCODE_Z, SDL_SCANCODE_V};
    const uint16_t bits[] = {16, 32, 64, 128, 16384, 8192, 1, 4096, 32768};
    // D08Q6: captured, Right Shift is Shift (above), never the escape hatch's
    // Select: Select reaching input_pad_context() silently dropped capture, so
    // the host layer stopped mid-flight (hover lock left on, WASD dead).
    // D19D: V (Square: pause OPTIONS, statistics SAVE) is a menu key only.
    // D10D: captured, Left / Right are keyboard turning, never the D-pad turn.
    for (int i = 0; i < 9; ++i) if (!(captured && (fixed[i]==SDL_SCANCODE_LEFT || fixed[i]==SDL_SCANCODE_RIGHT || fixed[i]==SDL_SCANCODE_X || fixed[i]==SDL_SCANCODE_Z || fixed[i]==SDL_SCANCODE_C || fixed[i]==SDL_SCANCODE_RSHIFT || fixed[i]==SDL_SCANCODE_V)) && keys[fixed[i]]) value &= ~bits[i];
    // Retain a brief tap across host polling and a short original transition.
    // Grounded locomotion, and host-owned shallow-water jumps.
    if(captured && jump_deadline && sequence<=jump_deadline &&
       (movement_ready() || swim_host_owns_jump())) {
        // Wade / free swim: do not feed Square — host owns jump / vertical / mantle.
        // Square in deep water selects anim 176 and flips the body.
        if(!swim_host_owns_jump())value &= ~pads[jump];
    }
    // D08Y: a run jump the original queued for the coming edge launches from the
    // lip only while the button stays held; hold it for a recent press (about
    // the 1024-unit look-ahead at running speed, with margin).
    if(captured && jump_pressed_at && sequence-jump_pressed_at<=40 && edge_jump_queued())value &= ~pads[jump];
    const bool locomotion = captured && (movement_ready() || locomotion_input_ready());
    const bool jet = captured && jetpack_input_ready();
    const bool push_grab = captured && push_latched && push_grab_ready();
    const bool push_wait = captured && push_request && !push_grab;
    const bool push_owned = push_grab || push_wait;
    // After letting go, an original push/pull cycle already under way runs to
    // its end; directions stay neutral until the original has let go, so a held
    // W cannot keep feeding it (and E's Cross waits, below).
    const bool push_releasing = captured && !push_owned && push_letting_go && push_grab_ready();
    // D08V: mouse camera kept through a mantle/hang/pull-up or unowned fall.
    const bool traversal_camera = captured && traversal_camera_ready();
    const bool modern_lease = locomotion || jet || push_owned || push_releasing || traversal_camera || (captured && (traversal_input_ready() ||
        swim_input_ready() || swim_thrust_input_ready()));
    if (captured) for (int i = 0; i < action_count; ++i)
        if (!(locomotion && i == walk) && !(jet && i == walk && !jetpack_classic_input_ready()) &&
            !(i==original_aim && view_aim_input_ready()) &&
            !(i==jump && (swim_host_owns_jump() || push_owned)) && down(binds[i])) value &= ~pads[i];
    // Legacy aiming: the primary Grab input is original precision aim (R1).
    if (captured && aim_down() && !view_aim_input_ready()) value &= ~pads[original_aim];
    // The original ignores the ladder let-go while Cross is held, so a held E
    // (interact = Cross) must not keep Duke on the last rung.
    bool ladder_let_go=false;
    if(captured && traversal_input_ready() && !ladder_exit_ready()) {
        const bool mount_finishing=ladder_mount_finishing();
        if(down(binds[move_forward]) && !down(binds[move_back]) && !mount_finishing) value &= ~16;
        // D08U1: S at a ladder that ends above the floor lets go from the
        // bottom-rung hang (original Square) instead of Down, which only
        // flips the hang poses; W still climbs back up.
        // At the last rung of such a ladder, S lets go from the climbing pose
        // (the original's own probe decides), so Duke never swings into it.
        const bool descending=down(binds[move_back]) && !down(binds[move_forward]) && !mount_finishing;
        const bool end_drop=descending && ladder_end_below();
        const int bottom_hang=descending && !end_drop ? ladder_bottom_hang() : 0;
        if(descending && !bottom_hang && !end_drop) value &= ~64;
        if(bottom_hang==2 || end_drop) value &= ~pads[jump];
        ladder_let_go=bottom_hang==2 || end_drop;
        // D08U: original Down alone stops at the lowest rung; Down + Cross
        // climbs on down and steps off onto the floor (185 reversed).
        if(descending && !end_drop && ladder_descent_ready()) value &= ~16384;
        // D08X: a stalled object hang lets go through the original Square.
        if(object_hang_release_ready()) value &= ~pads[jump];
        // D08J3: unswapped again when the camera looks at Duke's front.
        const bool pole=pole_sidestep_ready() &&
            !pole_view_from_front(down(binds[move_left]) || down(binds[move_right]));
        if(down(binds[move_left]) && !down(binds[move_right])) value &= pole?~32:~128;
        if(down(binds[move_right]) && !down(binds[move_left])) value &= pole?~128:~32;
        // D08J5: Ctrl lets go of a pole or chain, as under a ceiling; so does S
        // resting at its end with nothing to step onto (original Square).
        if(pole_sidestep_ready() && (down(binds[crouch]) || (descending && pole_let_go_ready()))) {
            value &= ~pads[jump];value |= 64;ladder_let_go=true;
        }
    }
    // D08J4: under a climbable ceiling the host steers Duke's yaw from WASD and
    // the camera; the original only gets Up to carry him, or Square for Ctrl.
    if(captured && ceiling_hang_ready()) {
        value |= 16|32|64|128;
        if(ceiling_hang_drop()) value &= ~pads[jump];
        else if(ceiling_hang_advance()) value &= ~16;
    }
    // D08V: an unowned fall keeps the original fall buttons the tank fallback
    // used to give (Up/Down and the strafe pads, never D-pad turns).
    // D22B: a dodge roll or slope slide gets none; it ends in idle and the
    // lease resumes.
    if(traversal_camera && !traversal_input_ready() && !committed_camera_ready()) {
        uint16_t strafe_left=0,strafe_right=0;
        swim_strafe_pads(strafe_left,strafe_right);
        if(down(binds[move_forward]) && !down(binds[move_back])) value &= ~16;
        if(down(binds[move_back]) && !down(binds[move_forward])) value &= ~64;
        if(down(binds[move_left]) && !down(binds[move_right])) value &= ~strafe_left;
        if(down(binds[move_right]) && !down(binds[move_left])) value &= ~strafe_right;
    }
    // Free swim: W/S → D-pad Up/Down for 8005201c; A/D → the layout's strafe
    // buttons (L2/R2 by default). D-pad Left/Right would turn (anims 71/70).
    // Space suppressed (host soft ascend). Forward magnitude word is written
    // from swim.inc.
    if(captured && swim_input_ready()) {
        uint16_t strafe_left=0,strafe_right=0;
        swim_strafe_pads(strafe_left,strafe_right);
        if(down(binds[move_forward]) && !down(binds[move_back])) value &= ~16;
        if(down(binds[move_back]) && !down(binds[move_forward])) value &= ~64;
        if(down(binds[move_left]) && !down(binds[move_right])) value &= ~strafe_left;
        if(down(binds[move_right]) && !down(binds[move_left])) value &= ~strafe_right;
    }
    // D08Q jetpack flight (original mode 10): the body faces the camera, so
    // original Up/Down thrust and the layout's strafe pads are camera-relative;
    // WASD and Up / Down drive them (D-pad Left/Right turns would be
    // overridden by face_view), Space is Square. Classic (D08R) flies on
    // them. Modern (D08Q6) feeds them for the original's flame, lean poses,
    // thrust state and fuel while the host owns the velocity (jetpack.inc
    // cancels their thrust); Shift (L1, the original hover toggle) is withheld
    // there because hovering is host-owned.
    if(jet) {
        uint16_t strafe_left=0,strafe_right=0;
        swim_strafe_pads(strafe_left,strafe_right);
        value |= 16|32|64|128;
        const bool forward=down(binds[move_forward]) || keys[SDL_SCANCODE_UP];
        const bool back=down(binds[move_back]) || keys[SDL_SCANCODE_DOWN];
        const bool left=down(binds[move_left]);
        const bool right=down(binds[move_right]);
        if(forward && !back) value &= ~16;
        if(back && !forward) value &= ~64;
        if(left && !right) value &= ~strafe_left;
        if(right && !left) value &= ~strafe_right;
        if(!jetpack_classic_input_ready()) {
            value |= pads[walk];
            // Space and Ctrl together hold height on the host: no lift.
            if(down(binds[crouch])) value |= pads[jump];
        }
    }
    // Underwater (original state 5): Square is thrust along body yaw/pitch,
    // which swim.inc steers from WASD / Space / Ctrl. D-pad would pitch.
    if(captured && swim_thrust_input_ready() &&
       (down(binds[move_forward]) || down(binds[move_back]) || down(binds[move_left]) ||
        down(binds[move_right]) || down(binds[jump]) || down(binds[crouch]))) value &= ~0x8000u;
    // E can never put armed Cross on SIO. The original Circle sequence owns
    // holstering; a short queued pulse makes a tap interact after it completes.
    // Host Circle requests (E stow, weapon restore, fire draw, push stow) are
    // taps: see circle_tap below.
    bool circle_request=captured && interaction_pending && holster_pulse && sequence<=holster_pulse && interaction_holster_ready();
    // After a grab ends, E's Cross waits until the original has let go.
    if(captured && !push_owned && !(push_letting_go && push_grab_ready()) && (down(binds[interact]) || airborne_interact || ladder_leap_active() || (interaction_pulse && sequence<=interaction_pulse)) && interaction_ready() && !ladder_let_go) value &= ~16384;
    if(captured && restore_owned && restore_pulse && sequence<=restore_pulse &&
       !down(binds[interact]) && interaction_restore_ready())circle_request=true;
    // Bound fire draws a settled holstered weapon through original Circle.
    // Suppress unarmed Cross for this request so it cannot activate an object.
    // Once drawing starts the readiness gate closes; held fire then reaches the
    // original weapon handler. No shot is buffered after release or focus loss.
    if(captured && down(binds[fire]) && !down(binds[holster]) &&
       !down(binds[interact]) && fire_draw_ready()) {
        value |= 16384;
        circle_request=true;
    }
    // Keep the persisted action ID/binding. In supported Modernized locomotion
    // it is the speed modifier: default walk, Shift run, Caps Lock reverses it.
    if (locomotion && !short_fall_input_ready() && !wade_full_speed_ready() &&
        !(autorun != down(binds[walk]))) value &= ~pads[walk];
    // Camera-relative ground locomotion injects Forward for any WASD so the
    // original gait runs; free swim must keep distinct D-pad bits (A/D/S).
    if(captured && directional_takeoff_ready())value &= ~16;
    if (captured && !swim_input_ready() && !push_owned && !push_releasing &&
        (down(binds[move_forward]) != down(binds[move_back]) ||
         down(binds[move_right]) != down(binds[move_left])) && locomotion) value &= ~16;
    // D08K4: crouched with Modern crouch, the host walks Duke; any direction
    // reaching the original crouch handler would start a roll.
    if(captured && crouch_walk_input_ready()) value |= 16|32|64|128;
    // D08T/D08T1: the host owns every direction, Cross, Circle and Square while
    // Grab requests or holds a pushable object; the original grab/push/pull
    // run. Fire, Space and precision aim wait until Duke lets go.
    if(push_owned) {
        value |= 16|32|64|128|16384|8192|32768|2048;
        if(push_grab) value &= ~(16384|push_pad(frame.move_x,frame.move_y));
        else {
            if(push_holster_pulse && sequence<=push_holster_pulse && interaction_holster_ready()) circle_request=true;
            if(sequence>=push_request_at+6 && push_idle_ready()) value &= ~16384;
        }
    }
    // Circle is the original tap-or-hold button: a tap holsters, draws or
    // uses; held past about 100 ticks of game-frame time it opens the
    // hold-to-select inventory mode (+0x224 0x200), which stops Duke until
    // the release. Host requests used to hold Circle for 4-6 input frames, or
    // for as long as fire was held to draw a holstered weapon; at 15-20 game
    // fps that crossed the threshold: Duke froze while fire was held and an
    // item was used on release (2026-10-03 playtest, "lost all movement").
    // A request is now one tap: Circle until the game has sampled it once,
    // then released until the request ends. A bound holster key stays raw.
    // (8 input frames at most if the history cannot be read.)
    static bool circle_tapping, circle_tap_done;
    static uint64_t circle_tap_from;
    if(!circle_request) circle_tapping=circle_tap_done=false;
    else if(!circle_tap_done) {
        if(!circle_tapping) {circle_tapping=true;circle_tap_from=sequence;}
        if(sequence>circle_tap_from && (hold_button_sampled() || sequence-circle_tap_from>=8)) circle_tap_done=true;
        else value &= ~8192;
    }
    if(push_releasing) value |= 16|32|64|128|16384|8192|32768|2048;
    // WASD actions have pad 0: camera-relative play injects Forward only while
    // the modern camera lease is live. A scripted/alternate camera (turret,
    // crystal-2 ceiling gun) drops that lease and used to leave captured WASD
    // inert. Feed original forward/back plus L2/R2 strafe until the lease
    // returns. D-pad Left/Right are tank *turns* (anims 71/70) and must not
    // be A/D — that is what spun Duke in the turret wade.
    static bool tank_fallback, tank_notified;
    static uint64_t tank_since;
    if (captured && !modern_lease) {
        const bool tank_move=down(binds[move_forward])!=down(binds[move_back]) ||
            down(binds[move_left])!=down(binds[move_right]);
        if (tank_move && !tank_fallback) {
            tank_fallback = true;tank_since=sequence;tank_notified=false;
            std::fprintf(stderr, "[TTK input] WASD tank fallback (modern camera lease inactive: %s)\n", lease_refusal_reason());
        }
        // Hit reactions and wall bumps pass in a moment; only a sustained loss
        // is announced, with the lease's own reason, so a playtest can name it.
        // D08V1: an identity refusal (the guarded original code no longer
        // matches) has no camera to steer by, so it says so and the mouse turns
        // Duke through the original D-pad turn (the turn keys still work too).
        const bool identity_lost=!std::strcmp(lease_refusal_reason(),"identity");
        if (tank_fallback && !tank_notified && sequence-tank_since>=45) {
            tank_notified=true;
            char text[96];
            if(identity_lost)std::snprintf(text,sizeof text,"MODERN CONTROLS PAUSED - MOUSE TURNS, WASD MOVES");
            else std::snprintf(text,sizeof text,"ORIGINAL MOVEMENT (%s)",lease_refusal_reason());
            host_osd_push_centered(text,identity_lost?4000:2500);
            std::fprintf(stderr, "[TTK input] %s (%s)\n", text, lease_refusal_reason());
        }
        uint16_t strafe_left=0,strafe_right=0;
        swim_strafe_pads(strafe_left,strafe_right);
        if(down(binds[move_forward]) && !down(binds[move_back])) value &= ~16;
        if(down(binds[move_back]) && !down(binds[move_forward])) value &= ~64;
        if(down(binds[move_left]) && !down(binds[move_right])) value &= ~strafe_left;
        if(down(binds[move_right]) && !down(binds[move_left])) value &= ~strafe_right;
        if(identity_lost)value &= ~fallback_turn_pad();
        // D10D: without the modern camera the turn keys turn Duke through the
        // original D-pad turn.
        if(down(binds[turn_left]) && !down(binds[turn_right])) value &= ~128;
        if(down(binds[turn_right]) && !down(binds[turn_left])) value &= ~32;
    } else if (captured && tank_fallback) {
        tank_fallback = false;
        std::fprintf(stderr, "[TTK input] Modern movement lease resumed\n");
        if (tank_notified) host_osd_push_centered("MODERN MOVEMENT RESUMED",1500);
        tank_notified=false;
    }
    last_pad=value;
    return value;
}
uint16_t input_last_pad() {return last_pad;}
}
extern "C" const char* ttk_fast_timing_json(void);
extern "C" const char* ttk_input_debug_json() {
    static char buffer[10240];
    const auto& f = ttk::input_snapshot(ttk::Context::Gameplay);
    // D19G: the live binding codes, in action order.
    static char binds_text[512];
    {size_t n=0;for(int i=0;i<ttk::action_count && n<sizeof binds_text-8;++i)
        n+=std::snprintf(binds_text+n,sizeof binds_text-n,"%s%d",i?",":"",ttk::binds[i]);}
    std::snprintf(buffer, sizeof buffer,
        "{\"binds\":[%s],\"sequence\":%llu,\"jump_remaining\":%llu,\"modernized\":%s,\"focused\":%s,\"captured\":%s,\"pad\":%u,\"move_x\":%.5f,\"move_y\":%.5f,\"look_x\":%.1f,\"look_y\":%.1f,\"device\":%d,\"controls\":%s,\"cpu_timing\":%s}",
        binds_text, (unsigned long long)ttk::sequence, (unsigned long long)(ttk::input_jump_pending()?ttk::jump_deadline-ttk::sequence+1:0),
        ttk::modern ? "true" : "false", ttk::focused ? "true" : "false", ttk::captured ? "true" : "false",
        ttk::input_pad(), f.move_x, f.move_y, f.look_x, f.look_y, (int)f.device, ttk::controls_debug_json(), ttk_fast_timing_json());
    return buffer;
}
