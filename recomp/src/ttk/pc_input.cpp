#include "pc_input.h"
#include "modern_controls.h"
#include "host_osd.h"
#include "inventory_hud.h"
#include "weapon_aim.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <atomic>

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
static uint64_t escape_start_deadline,escape_at;
static bool airborne_interact;
static uint64_t interaction_deadline, interaction_pulse, capture_after, holster_pulse;
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
static uint64_t recenter_total;
static int shoulder;
static bool first_person;
static bool independent_camera() {
    const char* mode=std::getenv("DNTTK_CAMERA_MODE");
    return !mode || !std::strcmp(mode,"independent");
}
static void camera_action(int action) {
    if(!independent_camera())return;
    if(action==camera_recenter) {++recenter_total;return;}
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
    if(command_count<32) {
        if(!command_count)command_deadline=sequence+8;
        commands[command_count++]=action;++command_serial;
    }
    if(action>=weapon_previous && action<=weapon_last || action>=weapon_group_1)cancel_restore();
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
static void clear() {
    cheat_typing.reset();cheat_pending=Cheat::None;menu_select_deadline=0;
    std::memset(keys, 0, sizeof keys);
    std::memset(mouse, 0, sizeof mouse);
    dx = dy = 0;wheel_fraction=distance_total=0;recenter_total=0;command_count=0;
    frame = {};jump_deadline=jump_pressed_at=0;airborne_interact=false;
    interaction_pending=false;interaction_pulse=0;interaction_started=false;holster_pulse=0;
    push_reset();
    ++epoch; total_x = total_y = 0; look_n=0; look_cx=look_cy=0;
    device = Device::None;
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
    interaction_pending=false;interaction_pulse=0;interaction_started=false;holster_pulse=0;
    airborne_interact=false;cancel_restore();push_reset();jump_deadline=0;jump_pressed_at=0;
}
void input_release() {
    if (window && captured) relative(false);
    captured = false;
    capture_after=sequence+12;
    clear();
}
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
        || code == 71 || code == 224 || code == 226 || code == 47 || code == 48 || code == 51 || code == 52 || code == 44 || code == 54 || code == 55 || code == 225 || (code >= -5 && code <= -1);
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
    const char* mode=std::getenv("DNTTK_WEAPON_AIM");
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
        if(captured && !(mods&(KMOD_CTRL|KMOD_ALT|KMOD_GUI))) {
            const bool typing=cheat_typing.active();
            if(pressed && code>=SDL_SCANCODE_A && code<=SDL_SCANCODE_Z) {
                auto result=cheat_typing.feed(char('a'+code-SDL_SCANCODE_A),sequence);
                if(typing || cheat_typing.active() || result!=Cheat::None) {
                    if(!typing) {auto parser=cheat_typing;clear();cheat_typing=parser;}
                    if(result!=Cheat::None) {cheat_pending=result;cheat_deadline=sequence+120;}
                    return;
                }
            } else if(typing && code!=SDL_SCANCODE_ESCAPE && code!=SDL_SCANCODE_F10) {
                if(pressed) clear();
                return;
            }
        }
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
        if(pressed && captured)for(int i=weapon_previous;i<action_count;++i)
            if(code==binds[i])command(i);
        if(pressed && captured)for(int i:{camera_recenter,camera_shoulder,camera_view})
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
            cancel_restore();airborne_interact=false;interaction_pending=false;interaction_pulse=0;interaction_started=false;holster_pulse=0;
            // Deliberate holster key only — E auto-stow never publishes these quotes.
            if(weapon_drawn())input_notice("WEAPON LOWERED");
            else if(weapon_holstered() || fire_draw_ready())input_notice("WEAPON RAISED");
        }
    } else if (cheat_typing.active()) {
        return;
    } else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        if (e.button.windowID != SDL_GetWindowID(window)) return;
        if (captured && e.button.button < 6) {
            mouse[e.button.button] = e.type == SDL_MOUSEBUTTONDOWN;
            if(e.type==SDL_MOUSEBUTTONDOWN && binds[inventory]==-int(e.button.button)) {
                ttk_aim_toggle_crosshair();
                input_notice(ttk_aim_crosshair_enabled()?"CROSSHAIR ON":"CROSSHAIR OFF");
            }
            if(e.type==SDL_MOUSEBUTTONDOWN)for(int i=weapon_previous;i<action_count;++i)
                if(binds[i]==-int(e.button.button))command(i);
            if(e.type==SDL_MOUSEBUTTONDOWN)for(int i:{camera_recenter,camera_shoulder,camera_view})
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
                interaction_pending=false;interaction_pulse=0;interaction_started=false;holster_pulse=0;
                if(weapon_drawn())input_notice("WEAPON LOWERED");
                else if(weapon_holstered() || fire_draw_ready())input_notice("WEAPON RAISED");
            }
        }
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
void input_frame() {
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
                  !down(binds[interact]) && interaction_restore_ready()) {
            if(!restore_settle)restore_settle=sequence;
            if(sequence-restore_settle>=6)restore_pulse=sequence+6;
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
    // (w a s d shift ctrl space e q) and dx=<counts> dy=<counts> per frame.
    // Never set by run.py.
    static const char* test_input=std::getenv("DNTTK_TEST_INPUT");
    if(test_input && modern) {
        focused=captured=true;
        static const struct {const char* name;SDL_Scancode code;} named[]={{"w",SDL_SCANCODE_W},{"a",SDL_SCANCODE_A},
            {"s",SDL_SCANCODE_S},{"d",SDL_SCANCODE_D},{"shift",SDL_SCANCODE_LSHIFT},{"ctrl",SDL_SCANCODE_LCTRL},
            {"space",SDL_SCANCODE_SPACE},{"e",SDL_SCANCODE_E},{"q",SDL_SCANCODE_Q}};
        bool was[sizeof named/sizeof named[0]];
        for(size_t i=0;i<sizeof named/sizeof named[0];++i){was[i]=keys[named[i].code];keys[named[i].code]=false;}
        if(FILE* f=std::fopen(test_input,"r")) {
            char word[64];
            while(std::fscanf(f,"%63s",word)==1) {
                if(!std::strncmp(word,"dx=",3)) dx+=std::atof(word+3);
                else if(!std::strncmp(word,"dy=",3)) dy+=std::atof(word+3);
                else for(const auto& n:named) if(!std::strcmp(word,n.name)) keys[n.code]=true;
            }
            std::fclose(f);
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
        frame.recenter_total=recenter_total;frame.shoulder=shoulder;frame.first_person=first_person;
        total_x += dx; total_y += dy;
        frame.total_x = total_x; frame.total_y = total_y;
        for (int i = 0; i < action_count; ++i) frame.held[i] = down(binds[i]);
        frame.held[original_aim]=aim_down();frame.held[grab]=grab_down();frame.held[grab_alt]=false;
        frame.move_x = float(frame.held[move_right]) - float(frame.held[move_left]);
        frame.move_y = float(frame.held[move_forward]) - float(frame.held[move_back]);
        float magnitude = std::hypot(frame.move_x, frame.move_y);
        if (magnitude > 1) { frame.move_x /= magnitude; frame.move_y /= magnitude; }
        frame.arrow_x = float(keys[SDL_SCANCODE_RIGHT]) - float(keys[SDL_SCANCODE_LEFT]);
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
uint16_t input_pad() {
    if(cheat_typing.active())return 65535;
    uint16_t value = 0xffff;
    if (!modern || !focused) return value;
    if(modern && focused && menu_select_deadline && sequence<=menu_select_deadline)value &= ~1u;
    if(escape_start_deadline && sequence<=escape_start_deadline)value &= ~8u;
    // Unrebindable escape hatch for menus and the original tank movement preview.
    const int fixed[] = {SDL_SCANCODE_UP, SDL_SCANCODE_RIGHT, SDL_SCANCODE_DOWN,
        SDL_SCANCODE_LEFT, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_RETURN, SDL_SCANCODE_RSHIFT, SDL_SCANCODE_Z, SDL_SCANCODE_ESCAPE};
    const uint16_t bits[] = {16, 32, 64, 128, 16384, 8192, 8, 1, 4096, 8};
    // D08Q6: captured, Right Shift is Shift (above), never the escape hatch's
    // Select: Select reaching input_pad_context() silently dropped capture, so
    // the host layer stopped mid-flight (hover lock left on, WASD dead).
    for (int i = 0; i < 10; ++i) if (!(captured && (fixed[i]==SDL_SCANCODE_X || fixed[i]==SDL_SCANCODE_Z || fixed[i]==SDL_SCANCODE_C || fixed[i]==SDL_SCANCODE_RSHIFT)) && keys[fixed[i]]) value &= ~bits[i];
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
        if(down(binds[move_left]) && !down(binds[move_right])) value &= ~128;
        if(down(binds[move_right]) && !down(binds[move_left])) value &= ~32;
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
    // WASD and the arrow keys both drive them (D-pad Left/Right turns would be
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
        const bool left=down(binds[move_left]) || keys[SDL_SCANCODE_LEFT];
        const bool right=down(binds[move_right]) || keys[SDL_SCANCODE_RIGHT];
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
        if (tank_fallback && !tank_notified && sequence-tank_since>=45) {
            tank_notified=true;
            char text[96];
            std::snprintf(text,sizeof text,"ORIGINAL MOVEMENT (%s)",lease_refusal_reason());
            host_osd_push_centered(text,2500);
            std::fprintf(stderr, "[TTK input] %s\n", text);
        }
        uint16_t strafe_left=0,strafe_right=0;
        swim_strafe_pads(strafe_left,strafe_right);
        if(down(binds[move_forward]) && !down(binds[move_back])) value &= ~16;
        if(down(binds[move_back]) && !down(binds[move_forward])) value &= ~64;
        if(down(binds[move_left]) && !down(binds[move_right])) value &= ~strafe_left;
        if(down(binds[move_right]) && !down(binds[move_left])) value &= ~strafe_right;
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
    std::snprintf(buffer, sizeof buffer,
        "{\"sequence\":%llu,\"jump_remaining\":%llu,\"modernized\":%s,\"focused\":%s,\"captured\":%s,\"pad\":%u,\"move_x\":%.5f,\"move_y\":%.5f,\"look_x\":%.1f,\"look_y\":%.1f,\"device\":%d,\"controls\":%s,\"cpu_timing\":%s}",
        (unsigned long long)ttk::sequence, (unsigned long long)(ttk::input_jump_pending()?ttk::jump_deadline-ttk::sequence+1:0),
        ttk::modern ? "true" : "false", ttk::focused ? "true" : "false", ttk::captured ? "true" : "false",
        ttk::input_pad(), f.move_x, f.move_y, f.look_x, f.look_y, (int)f.device, ttk::controls_debug_json(), ttk_fast_timing_json());
    return buffer;
}
