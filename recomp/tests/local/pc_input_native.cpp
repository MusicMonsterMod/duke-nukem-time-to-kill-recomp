// SDL integration of production input module; no guest/disc/card dependencies.
#include "pc_input.h"
#include "live_env.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#define CHECK(expr) do { if (!(expr)) { std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expr); return 1; } } while (0)
static unsigned notices;
static int crosshair_forced=-1;
extern "C" void host_osd_push_centered(const char*,int) {++notices;}
extern "C" int ttk_aim_crosshair_enabled(void) {
    if(crosshair_forced>=0)return crosshair_forced;
    return 1;
}
extern "C" const char* ttk_fast_timing_json(void) { return "{}"; }
extern "C" void ttk_aim_toggle_crosshair(void) {
    crosshair_forced=ttk_aim_crosshair_enabled()?0:1;
}
// D19D: guest halfwords the menu keys read (game mode 0x800bcbb0 = 1, gameplay, by default).
#include <map>
static std::map<uint32_t,uint16_t> guest{{0x800bcbb0u,1}};
extern "C" uint16_t psx_mod_read_half(uint32_t address) {auto it=guest.find(address);return it==guest.end()?0:it->second;}
extern "C" uint32_t psx_mod_read_word(uint32_t address) {return psx_mod_read_half(address) | ((uint32_t)psx_mod_read_half(address+2)<<16);}
extern "C" uint64_t s_frame_count=0;
static SDL_Window* win;
static bool inventory_visible;
namespace ttk {
bool hold_button_sampled(){return false;}
bool inventory_visible(){return ::inventory_visible;}
unsigned mission_presses;int mission_steps;void mission_browse_press(int d){++mission_presses;mission_steps+=d;}}
static bool crouch_walk; // D08K4
static bool wade_full_speed; // D08K12 input policy regression
static bool d21_swimming,d21_underwater;
static bool ready, holstered, flight, traversal, preparing, shortfall, switching, jet, jet_classic, edge_queued;
static const char* refusal="state";
static bool alive=true,view_aim;
static bool push_grab,push_contact,push_idle;
static bool trav_camera, roll_camera;
static bool ledge_hang;
static bool ladder_top,ladder_on,ladder_exit,ladder_end,pole,pole_front,pole_end,ceiling,ceiling_adv,ceiling_let_go;static int ladder_hang;
static unsigned ladder_requests;
static bool ladder_leap,leap_on;static unsigned ladder_leaps;
static ttk::Quick last_quick;static ttk::QuickResult quick_answer;static int quick_calls;
static ttk::QuickResult quick_handler(ttk::Quick q) {last_quick=q;++quick_calls;return quick_answer;}
static void key(SDL_Scancode code, bool down, bool repeat = false) {
    SDL_Event e{};
    e.type = down ? SDL_KEYDOWN : SDL_KEYUP;
    e.key.windowID = SDL_GetWindowID(win);
    e.key.scancode = code;
    e.key.repeat = repeat;
    ttk::input_event(e);
}
static void focus(bool gained) {
    SDL_Event e{};
    e.type = gained ? SDL_EVENT_WINDOW_FOCUS_GAINED : SDL_EVENT_WINDOW_FOCUS_LOST;
    e.window.windowID = SDL_GetWindowID(win);
    ttk::input_event(e);
}
static void motion(float x, float y) {
    SDL_Event e{};
    e.type = SDL_MOUSEMOTION;
    e.motion.windowID = SDL_GetWindowID(win);
    e.motion.xrel = x; e.motion.yrel = y;
    ttk::input_event(e);
}
static const ttk::InputFrame& tick() {
    ttk::input_frame();
    return ttk::input_snapshot(ttk::Context::Gameplay);
}
// Establish capture through the production gameplay-offer path. Older tests
// used F10 as setup; it is deliberately not a capture API any more.
static void capture_fixture() {
    if(SDL_GetWindowRelativeMouseMode(win)) {ttk::input_release();return;}
    ttk::input_release_for_menu();
    for(int i=0;i<13;++i) {ttk::input_offer_gameplay_capture();tick();}
}
int main() {
    CHECK(SDL_Init(SDL_INIT_VIDEO) == 0);
    win = SDL_CreateWindow("D04 native test", 0, 0, 320, 240, 0);
    CHECK(win);
    SDL_setenv_unsafe("DNTTK_INPUT_MODE", "modernized", 1);
    ttk::input_init(win);
    CHECK(std::strcmp(ttk::input_binding_name(ttk::original_aim),"Unbound")==0);
    CHECK(std::strcmp(ttk::input_binding_name(ttk::grab),"Mouse2")==0);
    CHECK(std::strcmp(ttk::input_binding_name(ttk::grab_alt),"Left Alt")==0);
    CHECK(std::strcmp(ttk::input_binding_name(ttk::item_use),"Unbound")==0);
    focus(true);
    key(SDL_SCANCODE_W, true);
    CHECK(tick().move_y == 0); // menu context never leaks movement
    capture_fixture();
    CHECK(SDL_GetWindowRelativeMouseMode(win));
    // D34B: entering a native two-player match releases modern capture;
    // returning to single player recovers the saved Modern scheme unaided.
    guest[0x800c27bc]=2;
    CHECK(!tick().active && !SDL_GetWindowRelativeMouseMode(win));
    CHECK(!ttk::input_modern_controls());
    SDL_setenv_unsafe("DNTTK_SPLIT_DEVICES","keyboard-pad",1);
    CHECK(ttk::input_split_keyboard());
    key(SDL_SCANCODE_W,true);key(SDL_SCANCODE_SPACE,true);
    CHECK((ttk::input_pad()&(16|32768))==0);
    key(SDL_SCANCODE_W,false);key(SDL_SCANCODE_SPACE,false);
    SDL_Event split_mouse{};split_mouse.type=SDL_MOUSEBUTTONDOWN;
    split_mouse.button.windowID=SDL_GetWindowID(win);split_mouse.button.button=1;
    ttk::input_event(split_mouse);CHECK(!(ttk::input_pad()&16384));
    split_mouse.type=SDL_MOUSEBUTTONUP;ttk::input_event(split_mouse);CHECK(ttk::input_pad()&16384);
    CHECK((ttk::input_pad()&(16|32768))==(16|32768));
    focus(false);key(SDL_SCANCODE_W,true);CHECK(ttk::input_pad()==0xffff);
    focus(true);key(SDL_SCANCODE_W,false);
    guest[0x800bcbb0]=0;CHECK(ttk::input_modern_controls());
    guest[0x800bdea4]=1;CHECK(!ttk::input_split_keyboard());
    guest[0x800bdea4]=2;CHECK(ttk::input_split_keyboard());
    SDL_unsetenv_unsafe("DNTTK_SPLIT_DEVICES");CHECK(!ttk::input_split_keyboard());
    guest[0x800bcbb0]=1;guest[0x800c27bc]=1;
    capture_fixture();CHECK(tick().active && ttk::input_modern_controls());
    // D09: actual SDL gamepad sampling, radial deadzone, independent actions,
    // time-based look, focus loss, disconnect and original-mode exclusion.
    CHECK(SDL_InitSubSystem(SDL_INIT_GAMEPAD)==0);
    SDL_VirtualJoystickDesc desc{};SDL_INIT_INTERFACE(&desc);
    desc.type=SDL_JOYSTICK_TYPE_GAMEPAD;desc.naxes=6;desc.nbuttons=SDL_GAMEPAD_BUTTON_COUNT;
    desc.axis_mask=63;desc.button_mask=(1u<<SDL_GAMEPAD_BUTTON_COUNT)-1;
    desc.name="TTK controller regression";
    const auto id=SDL_AttachVirtualJoystick(&desc);CHECK(id);
    auto* joystick=SDL_OpenJoystick(id);auto* pad=SDL_OpenGamepad(id);CHECK(joystick && pad);
    SDL_SetJoystickVirtualAxis(joystick,4,-32768);SDL_SetJoystickVirtualAxis(joystick,5,-32768);
    auto sample=[&](){SDL_UpdateJoysticks();ttk::input_controller_sample(pad);return tick();};
    SDL_SetJoystickVirtualAxis(joystick,0,3000);CHECK(sample().move_x==0 && !sample().held[ttk::fire]);
    SDL_SetJoystickVirtualAxis(joystick,0,20000);auto cf=sample();
    CHECK(cf.move_x>0.4f && cf.move_x<0.7f && cf.move_y==0 && cf.device==ttk::Device::Controller);
    SDL_SetJoystickVirtualAxis(joystick,1,-32768);cf=sample();CHECK(cf.move_x>0 && cf.move_y>0 && std::hypot(cf.move_x,cf.move_y)<=1.001f);
    SDL_SetJoystickVirtualAxis(joystick,2,32767);sample();SDL_Delay(20);cf=sample();CHECK(cf.look_x>0 && cf.look_y==0);
    SDL_SetJoystickVirtualAxis(joystick,2,0);SDL_SetJoystickVirtualAxis(joystick,3,24000);
    SDL_Delay(10);CHECK(sample().look_y>0);
    SDL_setenv_unsafe("DNTTK_CONTROLLER_INVERT_Y","1",1);SDL_Delay(10);CHECK(sample().look_y<0);
    SDL_setenv_unsafe("DNTTK_MOUSE_INVERT_Y","1",1);SDL_Delay(10);CHECK(sample().look_y>0);
    SDL_unsetenv_unsafe("DNTTK_CONTROLLER_INVERT_Y");SDL_unsetenv_unsafe("DNTTK_MOUSE_INVERT_Y");
    SDL_SetJoystickVirtualAxis(joystick,3,0);
    SDL_SetJoystickVirtualAxis(joystick,2,0);SDL_SetJoystickVirtualAxis(joystick,0,0);SDL_SetJoystickVirtualAxis(joystick,1,0);
    SDL_SetJoystickVirtualAxis(joystick,5,32767);cf=sample();CHECK(cf.held[ttk::fire]);
    SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,true);cf=sample();CHECK(cf.held[ttk::jump] && ttk::input_jump_pending());
    SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,false);sample();
    SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,true);cf=sample();
    CHECK(cf.command_count && cf.commands[cf.command_count-1]==ttk::weapon_previous);
    ttk::input_ack_commands(cf.command_serial);cf=sample();CHECK(cf.command_count==0); // no held-button repeats
    focus(false);cf=sample();CHECK(!cf.active && ttk::input_pad()==0xffff);
    focus(true);capture_fixture();CHECK(!sample().held[ttk::fire]); // held through recapture needs release
    SDL_SetJoystickVirtualAxis(joystick,5,-32768);sample();
    SDL_SetJoystickVirtualAxis(joystick,5,32767);CHECK(sample().held[ttk::fire]);
    // D34C: sample both actual SDL assignments without merging either seat.
    const auto id2=SDL_AttachVirtualJoystick(&desc);CHECK(id2);
    auto* joystick2=SDL_OpenJoystick(id2);auto* pad2=SDL_OpenGamepad(id2);
    CHECK(joystick2 && pad2);
    SDL_SetJoystickVirtualAxis(joystick2,0,-24000);
    SDL_SetJoystickVirtualAxis(joystick2,2,16000);
    SDL_SetJoystickVirtualButton(joystick2,SDL_GAMEPAD_BUTTON_EAST,true);
    SDL_UpdateJoysticks();ttk::input_controller_sample(pad,pad2);
    const auto first=ttk::input_controller_sample_at(0);
    const auto second=ttk::input_controller_sample_at(1);
    CHECK(first.connected && second.connected && ttk::input_controller_connections()==3);
    CHECK(first.axes[0]==0 && first.axes[2]==0);
    CHECK(second.axes[0]==-24000 && second.axes[2]==16000);
    CHECK(!(first.buttons&(1u<<SDL_GAMEPAD_BUTTON_EAST)));
    CHECK(second.buttons&(1u<<SDL_GAMEPAD_BUTTON_EAST));
    CHECK(!ttk::input_controller_sample_at(2).connected);
    CHECK(!ttk::input_controller_sample_at(999).buttons);
    SDL_UpdateJoysticks();ttk::input_controller_sample(pad,pad2);
    CHECK(ttk::input_controller_sample_at(1).generation==second.generation);
    // Removing P1 must not promote P2; its held input and generation persist.
    ttk::input_controller_sample(nullptr,pad2);
    CHECK(!ttk::input_controller_sample_at(0).connected);
    CHECK(ttk::input_controller_sample_at(0).buttons==0);
    CHECK(ttk::input_controller_sample_at(0).axes[5]==0);
    CHECK(ttk::input_controller_sample_at(0).generation>first.generation);
    CHECK(ttk::input_controller_sample_at(1).generation==second.generation);
    CHECK(ttk::input_controller_sample_at(1).axes[0]==-24000);
    ttk::input_controller_sample(pad,pad2);
    CHECK(ttk::input_controller_sample_at(0).generation>first.generation);
    SDL_DetachVirtualJoystick(id2);SDL_UpdateJoysticks();
    ttk::input_controller_sample(pad,pad2);
    CHECK(!ttk::input_controller_sample_at(1).connected);
    CHECK(!ttk::input_controller_sample_at(1).buttons);
    CHECK(ttk::input_controller_sample_at(1).axes[0]==0);
    CHECK(ttk::input_controller_sample_at(1).generation>second.generation);
    SDL_CloseGamepad(pad2);SDL_CloseJoystick(joystick2);
    SDL_DetachVirtualJoystick(id);SDL_UpdateJoysticks();ttk::input_controller_sample(pad);cf=tick();CHECK(!cf.held[ttk::fire] && !cf.held[ttk::jump] && !ttk::input_jump_pending());
    SDL_CloseGamepad(pad);SDL_CloseJoystick(joystick);
    ttk::input_release();ttk::input_state_loaded();capture_fixture();
    // D10E: SDL's middle button is Mouse3, host-only, rebindable and held.
    CHECK(ttk::input_binding_code(ttk::camera_free_look)==-2);
    SDL_Event inspect{};inspect.type=SDL_MOUSEBUTTONDOWN;inspect.button.windowID=SDL_GetWindowID(win);inspect.button.button=2;
    ttk::input_event(inspect);CHECK(tick().held[ttk::camera_free_look]);CHECK(ttk::input_pad()==0xffff);
    inspect.type=SDL_MOUSEBUTTONUP;ttk::input_event(inspect);CHECK(!tick().held[ttk::camera_free_look]);
    {
        int original[ttk::action_count],custom[ttk::action_count];
        for(int i=0;i<ttk::action_count;++i)original[i]=custom[i]=ttk::input_binding_code(ttk::Action(i));
        custom[ttk::camera_free_look]=SDL_SCANCODE_G;ttk::input_set_binding_codes(custom);
        key(SDL_SCANCODE_G,true);CHECK(tick().held[ttk::camera_free_look]);
        key(SDL_SCANCODE_G,false);CHECK(!tick().held[ttk::camera_free_look]);ttk::input_set_binding_codes(original);
    }
    SDL_Event aim{};aim.type=SDL_MOUSEBUTTONDOWN;aim.button.windowID=SDL_GetWindowID(win);aim.button.button=3;
    ttk::input_event(aim);view_aim=true;
    CHECK(!tick().held[ttk::original_aim] && (ttk::input_pad()&2048));
    view_aim=false;CHECK(tick().held[ttk::original_aim] && !tick().held[ttk::grab] && !(ttk::input_pad()&2048)); // legacy: RMB aims
    // D08T1: with view aiming and the independent camera, RMB is Grab only.
    SDL_setenv_unsafe("DNTTK_WEAPON_AIM","view",1);
    CHECK(!tick().held[ttk::original_aim] && tick().held[ttk::grab] && (ttk::input_pad()&2048));
    aim.type=SDL_MOUSEBUTTONUP;ttk::input_event(aim);tick();
    SDL_unsetenv_unsafe("DNTTK_WEAPON_AIM");
    key(SDL_SCANCODE_W, true);
    CHECK(tick().move_y == 1 && (ttk::input_pad() & 16)); // no tank fallback
    key(SDL_SCANCODE_A,true);key(SDL_SCANCODE_LEFT,true);
    CHECK((ttk::input_pad() & (16|32|64|128|0x100))==(16|32|64|128|0x100));
    key(SDL_SCANCODE_A,false);key(SDL_SCANCODE_LEFT,false);
    refusal="identity";motion(33,0);tick();
    CHECK((ttk::input_pad() & (16|32|64|128))==(16|32|64|128));
    refusal="state";
    key(SDL_SCANCODE_E,true);CHECK((ttk::input_pad() & 16384)!=0);
    holstered=true;ready=true;CHECK((ttk::input_pad() & 16384)==0);
    key(SDL_SCANCODE_E,false);holstered=false;
    // D08K4: while the crouch walk owns the stance no direction reaches the
    // original (its crouch handler would roll), though WASD locomotion would.
    key(SDL_SCANCODE_W,true);tick();CHECK((ttk::input_pad() & 16)==0);
    crouch_walk=true;tick();CHECK((ttk::input_pad() & (16|32|64|128))==(16|32|64|128));
    crouch_walk=false;key(SDL_SCANCODE_W,false);tick();
    // Tap E while armed: original holster first, armed Cross never reaches SIO.
    key(SDL_SCANCODE_E,true);tick();
    CHECK((ttk::input_pad() & 16384)!=0 && (ttk::input_pad() & 8192)==0);
    key(SDL_SCANCODE_E,false);tick();CHECK((ttk::input_pad() & 16384)!=0);
    holstered=true;tick();CHECK((ttk::input_pad() & 16384)==0);
    for(int i=0;i<10;++i)tick();CHECK((ttk::input_pad() & 16384)!=0);
    // No target: redraw after the unarmed pulse, using Circle only.
    CHECK((ttk::input_pad() & 8192)!=0);
    for(int i=0;i<6;++i)tick();CHECK((ttk::input_pad() & 8192)==0);
    CHECK((ttk::input_pad() & 16384)!=0);
    for(int i=0;i<10;++i)tick();CHECK((ttk::input_pad() & 8192)!=0);
    holstered=false;
    // Hold E through a full ladder cycle: no redraw until normal ownership and release.
    key(SDL_SCANCODE_E,true);tick();holstered=true;tick();ready=false;traversal=true;
    for(int i=0;i<30;++i)tick();CHECK((ttk::input_pad() & 8192)!=0);
    key(SDL_SCANCODE_E,false);for(int i=0;i<30;++i)tick();CHECK((ttk::input_pad() & 8192)!=0);
    // Pause/focus clears keys, but does not lose temporary holster ownership.
    focus(false);focus(true);capture_fixture();
    traversal=false;ready=true;for(int i=0;i<8;++i)tick();CHECK((ttk::input_pad() & 8192)==0);
    for(int i=0;i<10;++i)tick();
    // Already holstered E does not acquire restoration ownership.
    key(SDL_SCANCODE_E,true);tick();key(SDL_SCANCODE_E,false);
    for(int i=0;i<30;++i){tick();CHECK((ttk::input_pad() & 8192)!=0);}
    holstered=false;
    key(SDL_SCANCODE_E,true);tick();key(SDL_SCANCODE_E,false);holstered=true;tick();
    key(SDL_SCANCODE_SCROLLLOCK,true);key(SDL_SCANCODE_SCROLLLOCK,false);
    for(int i=0;i<30;++i){tick();CHECK((ttk::input_pad() & 8192)!=0);}
    holstered=false;
    key(SDL_SCANCODE_E,true);tick();key(SDL_SCANCODE_E,false);holstered=true;tick();alive=false;tick();alive=true;
    for(int i=0;i<30;++i){tick();CHECK((ttk::input_pad() & 8192)!=0);}
    // Fire-to-draw uses the original Circle request, never unarmed interaction.
    auto mousefire=[](bool down){SDL_Event e{};e.type=down?SDL_MOUSEBUTTONDOWN:SDL_MOUSEBUTTONUP;
        e.button.windowID=SDL_GetWindowID(win);e.button.button=1;ttk::input_event(e);};
    holstered=true;ready=true;mousefire(true);tick();
    CHECK((ttk::input_pad()&8192)==0 && (ttk::input_pad()&16384)!=0);
    holstered=false;tick();CHECK((ttk::input_pad()&8192)!=0 && (ttk::input_pad()&16384)==0);
    mousefire(false);tick();CHECK((ttk::input_pad()&16384)!=0);
    holstered=true;ready=false;traversal=true;mousefire(true);tick();CHECK((ttk::input_pad()&8192)!=0);
    focus(false);focus(true);capture_fixture();ready=true;traversal=false;tick();
    CHECK((ttk::input_pad()&(8192|16384))==(8192|16384));
    holstered=false;
    key(SDL_SCANCODE_E,true);tick();focus(false);focus(true);capture_fixture();
    CHECK((ttk::input_pad() & (16384|8192))==(16384|8192));
    key(SDL_SCANCODE_H,true);CHECK((ttk::input_pad() & 8192)!=0);key(SDL_SCANCODE_H,false);
    // Captured gameplay no longer treats hard-coded C as holster; Scroll Lock does.
    key(SDL_SCANCODE_C,true);CHECK((ttk::input_pad() & 8192)!=0);key(SDL_SCANCODE_C,false);
    key(SDL_SCANCODE_SCROLLLOCK,true);CHECK((ttk::input_pad() & 8192)==0);key(SDL_SCANCODE_SCROLLLOCK,false);
    key(SDL_SCANCODE_W,true);
    auto ack=[](){ttk::input_ack_commands(ttk::input_snapshot(ttk::Context::Gameplay).command_serial);};
    key(SDL_SCANCODE_2,true);CHECK(tick().commands[0]==ttk::weapon_group_2);
    auto serial=tick().command_serial;CHECK(tick().command_serial==serial && tick().command_count==1);
    key(SDL_SCANCODE_2,true,true);CHECK(tick().command_count==1);ack();CHECK(tick().command_count==0);
    key(SDL_SCANCODE_2,false);
    auto wheel=[](float y, bool flipped=false){SDL_Event e{};e.type=SDL_MOUSEWHEEL;e.wheel.windowID=SDL_GetWindowID(win);e.wheel.y=y;
        e.wheel.direction=flipped?SDL_MOUSEWHEEL_FLIPPED:SDL_MOUSEWHEEL_NORMAL;ttk::input_event(e);};
    // D08A30: up advances, down goes back, including fractional and multi-notch input.
    wheel(0.5f);CHECK(tick().command_count==0);wheel(0.5f);CHECK(tick().commands[0]==ttk::weapon_next);ack();
    wheel(-0.5f);CHECK(tick().command_count==0);wheel(-0.5f);CHECK(tick().commands[0]==ttk::weapon_previous);ack();
    for(float y : {3.f,-3.f}) {
        wheel(y);CHECK(tick().command_count==3);
        for(int i=0;i<3;++i)CHECK(tick().commands[i]==(y>0?ttk::weapon_next:ttk::weapon_previous));
        ack();
    }
    wheel(1,true);CHECK(tick().commands[0]==ttk::weapon_previous);ack();
    wheel(-1,true);CHECK(tick().commands[0]==ttk::weapon_next);ack();
    key(SDL_SCANCODE_LSHIFT,true);wheel(1);CHECK(tick().commands[0]==ttk::weapon_next);ack();
    wheel(-1);CHECK(tick().commands[0]==ttk::weapon_previous);ack();key(SDL_SCANCODE_LSHIFT,false);
    key(SDL_SCANCODE_SEMICOLON,true);CHECK(tick().commands[0]==ttk::weapon_previous);ack();key(SDL_SCANCODE_SEMICOLON,false);
    key(SDL_SCANCODE_APOSTROPHE,true);CHECK(tick().commands[0]==ttk::weapon_next);ack();key(SDL_SCANCODE_APOSTROPHE,false);
    const bool d21_ready_before=ready;ready=true;ttk::input_state_loaded();
    CHECK(ttk::input_code_bindable(SDL_SCANCODE_C,ttk::crouch_alt));
    key(SDL_SCANCODE_C,true);CHECK(tick().held[ttk::crouch]);
    key(SDL_SCANCODE_C,false);CHECK(tick().held[ttk::crouch]); // release keeps stance
    key(SDL_SCANCODE_C,true,true);CHECK(tick().held[ttk::crouch]); // repeats do not toggle
    key(SDL_SCANCODE_C,true);CHECK(!tick().held[ttk::crouch]);key(SDL_SCANCODE_C,false);
    key(SDL_SCANCODE_LCTRL,true);CHECK(tick().held[ttk::crouch]);
    key(SDL_SCANCODE_C,true);key(SDL_SCANCODE_C,false);CHECK(tick().held[ttk::crouch]);
    key(SDL_SCANCODE_C,true);key(SDL_SCANCODE_C,false);CHECK(tick().held[ttk::crouch]); // Ctrl still held
    key(SDL_SCANCODE_LCTRL,false);CHECK(!tick().held[ttk::crouch]);
    key(SDL_SCANCODE_C,true);key(SDL_SCANCODE_C,false);CHECK(tick().held[ttk::crouch]);
    key(SDL_SCANCODE_LCTRL,true);CHECK(tick().held[ttk::crouch]);
    for(int i=0;i<5;++i)CHECK(tick().held[ttk::crouch]);
    key(SDL_SCANCODE_LCTRL,true,true);CHECK(tick().held[ttk::crouch]);
    key(SDL_SCANCODE_LCTRL,false);CHECK(!tick().held[ttk::crouch]);
    key(SDL_SCANCODE_LCTRL,true);CHECK(tick().held[ttk::crouch]);
    key(SDL_SCANCODE_LCTRL,false);CHECK(!tick().held[ttk::crouch]);
    key(SDL_SCANCODE_C,true);key(SDL_SCANCODE_C,false);CHECK(tick().held[ttk::crouch]);
    key(SDL_SCANCODE_SPACE,true);CHECK(!tick().held[ttk::crouch] && !tick().held[ttk::jump]);
    for(int i=0;i<12;++i) {CHECK(!tick().held[ttk::jump]);CHECK(!ttk::input_jump_pending());CHECK(ttk::input_pad()&32768);}
    key(SDL_SCANCODE_SPACE,true,true);CHECK(!tick().held[ttk::jump]);
    key(SDL_SCANCODE_SPACE,false);CHECK(!tick().held[ttk::jump]);
    key(SDL_SCANCODE_SPACE,true);CHECK(tick().held[ttk::jump] && ttk::input_jump_pending());
    key(SDL_SCANCODE_SPACE,false);ttk::input_state_loaded();
    key(SDL_SCANCODE_C,true);key(SDL_SCANCODE_C,false);CHECK(tick().held[ttk::crouch]);
    ttk::input_release();CHECK(!tick().active);capture_fixture();CHECK(tick().held[ttk::crouch]);
    ttk::input_state_loaded();CHECK(!tick().held[ttk::crouch]);
    key(SDL_SCANCODE_C,true);key(SDL_SCANCODE_C,false);CHECK(tick().held[ttk::crouch]);
    alive=false;CHECK(!tick().held[ttk::crouch]);alive=true;CHECK(!tick().held[ttk::crouch]);
    key(SDL_SCANCODE_C,true);key(SDL_SCANCODE_C,false);CHECK(tick().held[ttk::crouch]);
    jet=true;CHECK(!tick().held[ttk::crouch]);jet=false;CHECK(!tick().held[ttk::crouch]);
    for(bool* state : {&d21_swimming,&d21_underwater,&traversal,&trav_camera}) {
        key(SDL_SCANCODE_C,true);key(SDL_SCANCODE_C,false);CHECK(tick().held[ttk::crouch]);
        *state=true;CHECK(!tick().held[ttk::crouch]);*state=false;CHECK(!tick().held[ttk::crouch]);
    }
    int d21_codes[ttk::action_count];
    for(int i=0;i<ttk::action_count;++i)d21_codes[i]=ttk::input_binding_code(ttk::Action(i));
    d21_codes[ttk::crouch_alt]=SDL_SCANCODE_G;ttk::input_set_binding_codes(d21_codes);
    key(SDL_SCANCODE_C,true);CHECK(!tick().held[ttk::crouch]);key(SDL_SCANCODE_C,false);
    key(SDL_SCANCODE_G,true);key(SDL_SCANCODE_G,false);CHECK(tick().held[ttk::crouch]);
    d21_codes[ttk::crouch_alt]=0;ttk::input_set_binding_codes(d21_codes);CHECK(!tick().held[ttk::crouch]);
    key(SDL_SCANCODE_G,true);CHECK(!tick().held[ttk::crouch]);key(SDL_SCANCODE_G,false);
    d21_codes[ttk::crouch_alt]=-2;ttk::input_set_binding_codes(d21_codes);
    SDL_Event d21_mouse{};d21_mouse.button.windowID=SDL_GetWindowID(win);d21_mouse.button.button=2;
    d21_mouse.type=SDL_MOUSEBUTTONDOWN;ttk::input_event(d21_mouse);CHECK(tick().held[ttk::crouch]);
    d21_mouse.type=SDL_MOUSEBUTTONUP;ttk::input_event(d21_mouse);CHECK(tick().held[ttk::crouch]);
    d21_mouse.type=SDL_MOUSEBUTTONDOWN;ttk::input_event(d21_mouse);CHECK(!tick().held[ttk::crouch]);
    d21_mouse.type=SDL_MOUSEBUTTONUP;ttk::input_event(d21_mouse);
    d21_codes[ttk::crouch_alt]=SDL_SCANCODE_C;ttk::input_set_binding_codes(d21_codes);
    d21_codes[ttk::crouch]=-4;ttk::input_set_binding_codes(d21_codes);
    key(SDL_SCANCODE_C,true);key(SDL_SCANCODE_C,false);CHECK(tick().held[ttk::crouch]);
    d21_mouse.button.button=4;d21_mouse.type=SDL_MOUSEBUTTONDOWN;ttk::input_event(d21_mouse);
    CHECK(tick().held[ttk::crouch]);CHECK(tick().held[ttk::crouch]);
    d21_mouse.type=SDL_MOUSEBUTTONUP;ttk::input_event(d21_mouse);CHECK(!tick().held[ttk::crouch]);
    d21_mouse.type=SDL_MOUSEBUTTONDOWN;ttk::input_event(d21_mouse);CHECK(tick().held[ttk::crouch]);
    d21_mouse.type=SDL_MOUSEBUTTONUP;ttk::input_event(d21_mouse);CHECK(!tick().held[ttk::crouch]);
    d21_codes[ttk::crouch]=SDL_SCANCODE_LCTRL;ttk::input_set_binding_codes(d21_codes);
    ready=d21_ready_before;
    key(SDL_SCANCODE_LCTRL,true);CHECK(tick().active && tick().held[ttk::crouch]);
    CHECK((ttk::input_pad()&4096)!=0);key(SDL_SCANCODE_LCTRL,false);CHECK(!tick().held[ttk::crouch]);
    key(SDL_SCANCODE_LALT,true);wheel(2);CHECK(tick().command_count==0 && tick().distance_total==2);
    key(SDL_SCANCODE_LALT,false);wheel(-1);CHECK(tick().commands[0]==ttk::weapon_previous);ack();
    key(SDL_SCANCODE_X,true);CHECK(tick().commands[0]==ttk::weapon_last && (ttk::input_pad()&16384)!=0);key(SDL_SCANCODE_X,false);ack();
    wheel(-99);CHECK(tick().command_count==32);ack();CHECK(tick().command_count==0);
    wheel(-1);for(int i=0;i<10;++i)tick();CHECK(tick().command_count==0); // expires if no guest consumer
    wheel(-3);focus(false);CHECK(tick().command_count==0);focus(true);capture_fixture();
    key(SDL_SCANCODE_W,true);ready = true;
    CHECK((ttk::input_pad() & 1040) == 0); // default walk + forward
    key(SDL_SCANCODE_LSHIFT, true);
    CHECK((ttk::input_pad() & 1024) != 0); // hold run
    ready=false;flight=true;
    CHECK((ttk::input_pad() & 16)==0 && (ttk::input_pad() & 1024)!=0);
    flight=false;CHECK((ttk::input_pad() & 16)!=0);ready=true;
    // D10D: on foot the arrows are the turn keys, never the D-pad tank turn.
    key(SDL_SCANCODE_W,false);key(SDL_SCANCODE_LEFT,true);
    CHECK((ttk::input_pad() & (32|128))==(32|128) && tick().held[ttk::turn_left] && tick().command_count==0);
    key(SDL_SCANCODE_LEFT,false);key(SDL_SCANCODE_RIGHT,true);
    CHECK((ttk::input_pad() & (32|128))==(32|128) && tick().held[ttk::turn_right]);
    key(SDL_SCANCODE_RIGHT,false);key(SDL_SCANCODE_W,true);
    // D08Q6 Modern flight: the original still gets its flight pads (flame,
    // lean poses, thrust state; jetpack.inc cancels their thrust) but never
    // Shift's hover toggle (L1) or a Square that Ctrl cancels.
    ready=false;jet=true;key(SDL_SCANCODE_LSHIFT,false);
    CHECK((ttk::input_pad() & 16)==0 && (ttk::input_pad() & 0x300)==0x300); // W held
    // D10D: Left / Right turn the view in flight too, never a strafe pad.
    key(SDL_SCANCODE_W,false);key(SDL_SCANCODE_LEFT,true);CHECK((ttk::input_pad() & 0x3a0)==0x3a0 && tick().held[ttk::turn_left]);
    key(SDL_SCANCODE_LEFT,false);key(SDL_SCANCODE_SPACE,true);CHECK((ttk::input_pad() & 32768)==0);
    key(SDL_SCANCODE_LCTRL,true);CHECK((ttk::input_pad() & 32768)!=0);key(SDL_SCANCODE_LCTRL,false);key(SDL_SCANCODE_SPACE,false);
    key(SDL_SCANCODE_LSHIFT,true);CHECK(ttk::input_pad()==0xffff);key(SDL_SCANCODE_LSHIFT,false);
    key(SDL_SCANCODE_RSHIFT,true);CHECK(ttk::input_pad()==0xffff);key(SDL_SCANCODE_RSHIFT,false);
    // D08R Classic flight: W/S → original Up/Down, A/D → layout strafe pads,
    // Up / Down the same, Left / Right turn (D10D, never D-pad turns), Space passes as Square, Shift (either)
    // is the original hover toggle, Ctrl injects nothing.
    jet_classic=true;
    key(SDL_SCANCODE_W,true);CHECK((ttk::input_pad() & 16)==0 && (ttk::input_pad() & 0x300)==0x300);
    key(SDL_SCANCODE_W,false);key(SDL_SCANCODE_S,true);CHECK((ttk::input_pad() & 64)==0 && (ttk::input_pad() & 16)!=0);
    key(SDL_SCANCODE_S,false);key(SDL_SCANCODE_A,true);CHECK((ttk::input_pad() & 0x100)==0 && (ttk::input_pad() & 0x210)==0x210);
    key(SDL_SCANCODE_A,false);key(SDL_SCANCODE_D,true);CHECK((ttk::input_pad() & 0x200)==0 && (ttk::input_pad() & 0x110)==0x110);
    key(SDL_SCANCODE_D,false);key(SDL_SCANCODE_SPACE,true);CHECK((ttk::input_pad() & 32768)==0);
    key(SDL_SCANCODE_SPACE,false);key(SDL_SCANCODE_LCTRL,true);CHECK(ttk::input_pad()==0xffff);
    key(SDL_SCANCODE_LCTRL,false);
    key(SDL_SCANCODE_UP,true);CHECK((ttk::input_pad() & 16)==0 && (ttk::input_pad() & 0xe0)==0xe0);
    key(SDL_SCANCODE_UP,false);key(SDL_SCANCODE_DOWN,true);CHECK((ttk::input_pad() & 64)==0 && (ttk::input_pad() & 0xb0)==0xb0);
    key(SDL_SCANCODE_DOWN,false);key(SDL_SCANCODE_LEFT,true);CHECK((ttk::input_pad() & 0x3f0)==0x3f0);
    key(SDL_SCANCODE_LEFT,false);key(SDL_SCANCODE_RIGHT,true);CHECK((ttk::input_pad() & 0x3f0)==0x3f0);
    key(SDL_SCANCODE_RIGHT,false);
    // Captured, Right Shift is Shift and never the escape hatch's Select,
    // which would drop capture.
    key(SDL_SCANCODE_LSHIFT,true);CHECK((ttk::input_pad() & 1024)==0);key(SDL_SCANCODE_LSHIFT,false);
    key(SDL_SCANCODE_RSHIFT,true);CHECK((ttk::input_pad() & 1024)==0 && (ttk::input_pad() & 1)!=0);
    key(SDL_SCANCODE_RSHIFT,false);jet_classic=false;jet=false;
    key(SDL_SCANCODE_W,true);ready=true;
    notices=0;
    key(SDL_SCANCODE_CAPSLOCK, true);
    CHECK((ttk::input_pad() & 1024) != 0); // autorun
    CHECK(notices==1); // RUN MODE ON
    CHECK(ttk::input_running(false));
    key(SDL_SCANCODE_CAPSLOCK, true, true);
    CHECK((ttk::input_pad() & 1024) != 0); // no repeat toggle
    CHECK(notices==1);
    key(SDL_SCANCODE_LSHIFT, true);
    CHECK((ttk::input_pad() & 1024) == 0); // temporary walk while autorun
    CHECK(!ttk::input_running(false));
    focus(false); focus(true);
    capture_fixture();
    CHECK((ttk::input_pad() & 1024) != 0); // toggle survives capture/focus
    key(SDL_SCANCODE_CAPSLOCK, true);
    CHECK((ttk::input_pad() & 1024) == 0);
    CHECK(notices==2); // RUN MODE OFF
    // D08K12 crouch follows the speed action, even with Original water pace.
    CHECK(!ttk::input_running(false));
    wade_full_speed=true;
    CHECK(ttk::input_running() && !ttk::input_running(false));
    key(SDL_SCANCODE_LSHIFT,true);CHECK(ttk::input_running(false));
    key(SDL_SCANCODE_LSHIFT,false);CHECK(!ttk::input_running(false));
    wade_full_speed=false;
    ready = false;
    key(SDL_SCANCODE_W, true);
    key(SDL_SCANCODE_D, true);
    CHECK(std::abs(tick().move_x - 0.707106f) < 0.0001f);
    CHECK(ttk::input_snapshot(ttk::Context::Menu).move_x == 0);
    traversal=true;holstered=true;
    CHECK((ttk::input_pad() & (16|32))==0);
    key(SDL_SCANCODE_E,true);CHECK((ttk::input_pad() & 16384)==0);key(SDL_SCANCODE_E,false);
    key(SDL_SCANCODE_S,true);CHECK((ttk::input_pad() & (16|64))==(16|64));key(SDL_SCANCODE_S,false);
    traversal=false;holstered=false;
    key(SDL_SCANCODE_A, true);
    CHECK(tick().move_x == 0);
    // Same physical distance in one fast packet or many slow packets, with
    // and without the speed modifier. Check receipt before camera integration.
    for(bool run : {false,true}) {
        key(SDL_SCANCODE_LSHIFT,run);
        auto start=tick().total_x;
        motion(1600,0); CHECK(tick().total_x-start==1600);
        start=tick().total_x;
        for(int n=0;n<80;++n){motion(20,0);tick();}
        CHECK(tick().total_x-start==1600);
    }
    key(SDL_SCANCODE_LSHIFT,false);
    motion(12, -7); motion(3, 2);
    CHECK(tick().look_x == 15);
    CHECK(ttk::input_snapshot(ttk::Context::Gameplay).look_y == -5);
    CHECK(tick().look_x == 0); // one-frame deltas, not replayed
    key(SDL_SCANCODE_SPACE, true);
    CHECK((ttk::input_pad() & 0x8000) == 0);
    focus(false);
    CHECK(ttk::input_pad() == 0xffff && tick().move_y == 0);
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    focus(true);
    key(SDL_SCANCODE_W, true, true); // autorepeat cannot resurrect held state
    CHECK(tick().move_y == 0);
    capture_fixture();
    CHECK(tick().move_y == 0);
    motion(20, 20);
    key(SDL_SCANCODE_ESCAPE, true);
    CHECK(tick().look_x == 0 && !SDL_GetWindowRelativeMouseMode(win));
    CHECK((ttk::input_pad() & 8)==0);
    key(SDL_SCANCODE_ESCAPE,false);
    CHECK((ttk::input_pad() & 8)==0); // a quick tap still reaches the pad poll
    for(int i=0;i<7;++i)tick();
    CHECK((ttk::input_pad() & 8)!=0);
    guest[0x800be568]=1; // original menu navigation, not released gameplay
    key(SDL_SCANCODE_UP, true);
    key(SDL_SCANCODE_X, true);
    CHECK(ttk::input_pad() == (uint16_t)(0xffff & ~0x4010));
    guest[0x800be568]=0;
    ttk::input_release();
    capture_fixture();
    key(SDL_SCANCODE_RETURN, true);
    // Captured Enter uses selected inventory; it must not release like Escape.
    CHECK(tick().active && tick().command_count==1 && tick().commands[0]==ttk::item_use);
    key(SDL_SCANCODE_RETURN,false);ack();
    ttk::input_release();
    ttk::input_allow_capture(false);
    capture_fixture();
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    ttk::input_allow_capture(true);
    capture_fixture();
    ttk::input_pad_context(0xfffe); // controller inventory returns to menu context
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    CHECK(!(ttk::input_pad()&1));
    for(int i=0;i<8;++i)ttk::input_frame();
    CHECK(ttk::input_pad()&1);
    capture_fixture();ttk::input_pad_context(0xfffe);
    ttk::input_release();CHECK(ttk::input_pad()&1); // focus/reset cannot leave Select queued
    capture_fixture();
    ttk::input_pad_context(0xfff7); // controller pause releases too
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    guest[0x800be568]=1; // the original pause has acknowledged Start
    key(SDL_SCANCODE_Z, true);
    CHECK((ttk::input_pad() & 4096) == 0); // game's menu back is Triangle
    guest[0x800be568]=0;
    capture_fixture();
    SDL_Event quit{}; quit.type = SDL_QUIT;
    ttk::input_event(quit);
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    // Enter uses selected inventory while captured, once per physical press.
    ttk::input_init(win);focus(true);capture_fixture();tick();
    key(SDL_SCANCODE_RETURN,true);
    CHECK(tick().active && tick().command_count==1 && tick().commands[0]==ttk::item_use);
    CHECK(ttk::input_pad()&8);ack();
    for(int i=0;i<150;++i){key(SDL_SCANCODE_RETURN,true,true);CHECK(tick().active && !tick().command_count && (ttk::input_pad()&8));}
    focus(false);focus(true);capture_fixture();
    key(SDL_SCANCODE_RETURN,true,true);CHECK(tick().active && !tick().command_count && (ttk::input_pad()&8));
    key(SDL_SCANCODE_RETURN,false);key(SDL_SCANCODE_RETURN,true);
    CHECK(tick().active && tick().command_count==1 && tick().commands[0]==ttk::item_use);
    key(SDL_SCANCODE_RETURN,false);ack();
    capture_fixture();tick();
    key(SDL_SCANCODE_RETURN,true);tick();focus(false);
    key(SDL_SCANCODE_RETURN,false); // release may arrive while unfocused
    focus(true);capture_fixture();
    key(SDL_SCANCODE_RETURN,true);
    CHECK(tick().active && tick().command_count==1 && tick().commands[0]==ttk::item_use);
    key(SDL_SCANCODE_RETURN,false);ack();
    // D08A5: Comma / Period browse the mission inventory on the host; never guest commands.
    key(SDL_SCANCODE_COMMA,true);CHECK(ttk::mission_presses==1 && ttk::mission_steps==-1 && tick().command_count==0);
    key(SDL_SCANCODE_COMMA,false);
    key(SDL_SCANCODE_PERIOD,true);CHECK(ttk::mission_presses==2 && ttk::mission_steps==0 && tick().command_count==0);
    key(SDL_SCANCODE_PERIOD,false);
    key(SDL_SCANCODE_BACKSLASH,true);CHECK(ttk::mission_presses==2);key(SDL_SCANCODE_BACKSLASH,false);
    // Uncaptured Enter never takes inventory ownership (menus/skip keep Start).
    ttk::input_release();
    key(SDL_SCANCODE_RETURN,true);
    CHECK(!tick().active && !(ttk::input_pad()&8));key(SDL_SCANCODE_RETURN,false);
    // A saved custom speed binding keeps working; no action/schema migration.
    SDL_setenv_unsafe("DNTTK_INPUT_BINDINGS", "1:26,22,4,7,-1,44,11,25,21,0,0,0,12,8,0,0,0,-3,226,51,52,27,16,225,13,17,5,47,48,24,20,30,31,32,33,34,35,36,37,38,39,54,55,75,78,74,77,80,79,63,66,6,-2,45,46", 1);
    ttk::input_init(win);focus(true);capture_fixture();ready=true;
    CHECK((ttk::input_pad() & 1024) == 0); // launch resets autorun off
    key(SDL_SCANCODE_R,true);CHECK((ttk::input_pad() & 1024) != 0);
    key(SDL_SCANCODE_R,false);key(SDL_SCANCODE_LSHIFT,true);
    CHECK((ttk::input_pad() & 1024) == 0); // old key no longer runs
    ttk::input_release();key(SDL_SCANCODE_CAPSLOCK,true);capture_fixture();
    CHECK((ttk::input_pad() & 1024) == 0); // uncaptured Caps does not toggle
    ttk::input_release();ttk::input_init(win);focus(true);
    key(SDL_SCANCODE_W,true);for(int i=0;i<12;++i)tick();ttk::input_offer_gameplay_capture();
    CHECK(tick().active && tick().move_y==0); // synthetic W is not in SDL device state; capture resyncs bound keys from it
    key(SDL_SCANCODE_ESCAPE,true);ttk::input_offer_gameplay_capture();
    CHECK(!tick().active); // pause waits for a fresh gameplay offer after debounce
    key(SDL_SCANCODE_ESCAPE,false);
    for(int i=0;i<12;++i)tick();ttk::input_offer_gameplay_capture();
    CHECK(tick().active);
    key(SDL_SCANCODE_F10,true);
    for(int i=0;i<20;++i){ttk::input_offer_gameplay_capture();CHECK(tick().active);}
    ready=true;
    // A complete jump tap between polls must survive, but only on the ground.
    key(SDL_SCANCODE_SPACE,true);key(SDL_SCANCODE_SPACE,false);tick();
    CHECK((ttk::input_pad() & 32768)==0);
    ready=false;flight=true;CHECK((ttk::input_pad() & 32768)!=0);
    flight=false;ready=true;CHECK((ttk::input_pad() & 32768)==0);
    for(int i=0;i<9;++i){key(SDL_SCANCODE_SPACE,true,true);tick();}
    CHECK((ttk::input_pad() & 32768)!=0); // repeat does not refresh the buffer
    key(SDL_SCANCODE_SPACE,true);key(SDL_SCANCODE_SPACE,false);
    focus(false);focus(true);capture_fixture();CHECK((ttk::input_pad() & 32768)!=0);
    key(SDL_SCANCODE_SPACE,true);key(SDL_SCANCODE_SPACE,false);tick();
    CHECK(ttk::input_take_jump() && !ttk::input_take_jump());
    CHECK((ttk::input_pad() & 32768)!=0); // consumed press cannot replay on landing
    // D08Y: a run jump the original queued for the coming edge keeps Square
    // held past the 8-update tap buffer, for at most 40 updates after the press.
    edge_queued=true;CHECK((ttk::input_pad() & 32768)!=0); // no recent press
    key(SDL_SCANCODE_SPACE,true);key(SDL_SCANCODE_SPACE,false);
    for(int i=0;i<39;++i){tick();CHECK((ttk::input_pad() & 32768)==0);}
    tick();tick();CHECK((ttk::input_pad() & 32768)!=0);
    key(SDL_SCANCODE_SPACE,true);key(SDL_SCANCODE_SPACE,false);tick();
    edge_queued=false;for(int i=0;i<9;++i)tick();CHECK((ttk::input_pad() & 32768)!=0);
    edge_queued=true;CHECK((ttk::input_pad() & 32768)==0);
    CHECK(ttk::input_take_jump()==false);CHECK((ttk::input_pad() & 32768)==0);
    key(SDL_SCANCODE_SPACE,true);key(SDL_SCANCODE_SPACE,false);tick();
    CHECK(ttk::input_take_jump());CHECK((ttk::input_pad() & 32768)!=0); // launched: no hold
    edge_queued=false;
    // Accepted directional preparation supplies original Forward without a held
    // key, but capture/focus/menu boundaries still release it immediately.
    preparing=true;CHECK((ttk::input_pad() & 16)==0);
    focus(false);CHECK(ttk::input_pad()==65535);focus(true);CHECK((ttk::input_pad() & 16)!=0);
    capture_fixture();CHECK((ttk::input_pad() & 16)==0);
    key(SDL_SCANCODE_ESCAPE,true);CHECK((ttk::input_pad() & 16)!=0);
    key(SDL_SCANCODE_ESCAPE,false);capture_fixture();preparing=false;
    CHECK((ttk::input_pad() & 16)!=0);
    // Small-drop landing forwards live movement without the original walk-stop
    // selector; release/focus still removes movement immediately.
    ready=false;flight=true;shortfall=true;key(SDL_SCANCODE_W,true);tick();
    CHECK((ttk::input_pad() & 16)==0 && (ttk::input_pad() & 1024)!=0);
    key(SDL_SCANCODE_W,false);tick();CHECK((ttk::input_pad() & 16)!=0);
    focus(false);CHECK(ttk::input_pad()==65535);focus(true);capture_fixture();shortfall=false;
    // E-owned stow includes the post-holster blend, bounded by capture/focus/deadline.
    ready=false;flight=true;holstered=false;
    key(SDL_SCANCODE_E,true);key(SDL_SCANCODE_E,false);tick();
    CHECK(ttk::input_auto_stow_pending());
    CHECK((ttk::input_pad() & 16384)!=0);
    holstered=true;tick();CHECK(ttk::input_auto_stow_pending());
    for(int i=0;i<122;++i)tick();CHECK(!ttk::input_auto_stow_pending());
    focus(false);CHECK(!ttk::input_auto_stow_pending());focus(true);capture_fixture();
    // An E tap during a weapon switch survives its transient equipment0;
    // it spends the stow pulse only after the intended new weapon is settled.
    switching=true;holstered=false;key(SDL_SCANCODE_E,true);key(SDL_SCANCODE_E,false);tick();
    CHECK((ttk::input_pad()&(8192|16384))==(8192|16384));
    holstered=true;tick();CHECK((ttk::input_pad()&16384)!=0);
    holstered=false;switching=false;tick();CHECK((ttk::input_pad()&8192)==0);
    holstered=true;tick();CHECK((ttk::input_pad()&16384)==0);
    ttk::input_release();capture_fixture();
    // Midair E has a longer unarmed grab window and cannot become armed Cross.
    ready=false;flight=true;holstered=true;
    key(SDL_SCANCODE_E,true);key(SDL_SCANCODE_E,false);tick();
    for(int i=0;i<12;++i)tick();CHECK((ttk::input_pad() & 16384)==0);
    holstered=false;CHECK((ttk::input_pad() & 16384)!=0);holstered=true;
    for(int i=0;i<80;++i)tick();CHECK((ttk::input_pad() & 16384)==0);
    flight=false;ready=true;tick();CHECK((ttk::input_pad() & 16384)!=0);
    // A grounded tap immediately before takeoff becomes one airborne reach intent.
    key(SDL_SCANCODE_E,true);key(SDL_SCANCODE_E,false);tick();ready=false;flight=true;tick();
    for(int i=0;i<40;++i)tick();CHECK((ttk::input_pad() & 16384)==0);
    key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);tick();CHECK((ttk::input_pad() & 16384)!=0);
    // Typed DN commands consume shortcut letters after their prefix, execute
    // once and never survive focus/menu boundaries. Ordinary D still strafes.
    notices=0;
    ttk::input_release();
    unsetenv("DNTTK_INPUT_BINDINGS");
    ttk::input_init(win);focus(true);capture_fixture();ready=true;flight=false;
    key(SDL_SCANCODE_D,true);CHECK(tick().move_x==1);key(SDL_SCANCODE_D,false);
    key(SDL_SCANCODE_N,true);key(SDL_SCANCODE_N,false);tick();CHECK(ttk::input_pad()==65535);
    for(char c:std::string("stuff")){key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),false);tick();}
    CHECK(ttk::input_take_cheat()==ttk::Cheat::Stuff && ttk::input_take_cheat()==ttk::Cheat::None);
    CHECK(tick().command_count==0 && tick().move_x==0);
    // D08G5: a pending prefix keeps mouse look live in the same epoch, and the
    // key that breaks it acts at once (no second press needed).
    for(const char* prefix:{"dnstu","dnk","dnmon","dnwea","dnu"}) {
        for(const char* c=prefix;*c;++c){key(SDL_Scancode(SDL_SCANCODE_A+*c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+*c-'a'),false);tick();}
        CHECK(ttk::input_pad()==65535);
        const auto before=tick();
        for(int n=0;n<20;++n){motion(15,-4);const auto& f=tick();CHECK(f.epoch==before.epoch);}
        const auto after=tick();
        CHECK(after.total_x-before.total_x==300 && after.total_y-before.total_y==-80);
        key(SDL_SCANCODE_W,true);CHECK(tick().move_y==1);key(SDL_SCANCODE_W,false);CHECK(tick().move_y==0);
        CHECK(ttk::input_take_cheat()==ttk::Cheat::None);
    }
    // Space and a mouse click also break a prefix and keep their normal action.
    for(char c:std::string("dnst")){key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),false);tick();}
    key(SDL_SCANCODE_SPACE,true);key(SDL_SCANCODE_SPACE,false);
    for(char c:std::string("uff")){key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),false);tick();}
    CHECK(ttk::input_take_cheat()==ttk::Cheat::None);
    for(char c:std::string("dnst")){key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),false);tick();}
    {SDL_Event e{};e.type=SDL_MOUSEBUTTONDOWN;e.button.windowID=SDL_GetWindowID(win);e.button.button=SDL_BUTTON_LEFT;ttk::input_event(e);
     e.type=SDL_MOUSEBUTTONUP;ttk::input_event(e);}
    for(char c:std::string("uff")){key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),false);tick();}
    CHECK(ttk::input_take_cheat()==ttk::Cheat::None);
    for(int n=0;n<250;++n)tick();
    for(auto code:ttk::cheat_codes) {
        for(const char* c=code.text;*c;++c){key(SDL_Scancode(SDL_SCANCODE_A+*c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+*c-'a'),false);tick();}
        CHECK(ttk::input_take_cheat()==code.action && ttk::input_take_cheat()==ttk::Cheat::None);
    }
    for(char c:std::string("dnk")){key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),false);tick();}
    focus(false);focus(true);capture_fixture();
    for(char c:std::string("roz")){key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),false);tick();}
    CHECK(ttk::input_take_cheat()==ttk::Cheat::None);
    for(const char* text:{"dnq", "dnk"}) {
        for(const char* c=text;*c;++c){key(SDL_Scancode(SDL_SCANCODE_A+*c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+*c-'a'),false);tick();}
        for(int n=0;n<250;++n)tick();
        CHECK(ttk::input_take_cheat()==ttk::Cheat::None);
    }
    CHECK(notices==0); // Parser never publishes partial, cancellation or success text.
    // Deliberate Scroll Lock holster publishes WEAPON LOWERED / RAISED once each.
    holstered=false;key(SDL_SCANCODE_SCROLLLOCK,true);key(SDL_SCANCODE_SCROLLLOCK,false);
    CHECK(notices==1); // WEAPON LOWERED
    holstered=true;key(SDL_SCANCODE_SCROLLLOCK,true);key(SDL_SCANCODE_SCROLLLOCK,false);
    CHECK(notices==2); // WEAPON RAISED
    // I toggles crosshair (inventory action is outside the weapon_previous edge loop).
    const unsigned before_i=notices;
    CHECK(ttk_aim_crosshair_enabled());
    key(SDL_SCANCODE_I,true);key(SDL_SCANCODE_I,false);
    CHECK(!ttk_aim_crosshair_enabled() && notices==before_i+1);
    key(SDL_SCANCODE_I,true);key(SDL_SCANCODE_I,false);
    CHECK(ttk_aim_crosshair_enabled() && notices==before_i+2);
    CHECK(tick().command_count==0); // I must not enqueue an inventory command
    // D04B: rapid load/Escape/resume must not require an offer gap.
    ttk::input_release();SDL_setenv_unsafe("DNTTK_INPUT_MODE","modernized",1);
    unsetenv("DNTTK_INPUT_BINDINGS");ttk::input_init(win);focus(true);capture_fixture();
    CHECK(tick().active);
    key(SDL_SCANCODE_F10,true);key(SDL_SCANCODE_F10,false);
    CHECK(tick().active && SDL_GetWindowRelativeMouseMode(win));
    key(SDL_SCANCODE_ESCAPE,true);key(SDL_SCANCODE_ESCAPE,false);
    ttk::input_offer_gameplay_capture();tick();
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    ttk::input_release_for_menu(); // pause acknowledgement before eight frames
    key(SDL_SCANCODE_ESCAPE,true);key(SDL_SCANCODE_ESCAPE,false);
    // The restored host menu consumes this resume press in the same frame.
    ttk::input_set_menu_handler(+[](const ttk::MenuKeys&){return true;});
    ttk::input_menu_poll();
    ttk::input_set_menu_handler(+[](const ttk::MenuKeys&){return false;});
    ttk::input_menu_poll();
    CHECK(!tick().active); // no stale pre-menu offer may capture
    ttk::input_offer_gameplay_capture();
    CHECK(tick().active && SDL_GetWindowRelativeMouseMode(win)); // first new offer, no cooldown
    ttk::input_set_menu_handler(nullptr);
    // A load consumes the Start pulse without ever stopping gameplay offers.
    // This was a permanent hold; it must recover without a menu gap or F10.
    key(SDL_SCANCODE_ESCAPE,true);key(SDL_SCANCODE_ESCAPE,false);
    for(int i=0;i<24;++i) {ttk::input_offer_gameplay_capture();tick();}
    CHECK(tick().active && SDL_GetWindowRelativeMouseMode(win));
    // Host overlays and focus loss keep the cursor free, then recover unaided.
    key(SDL_SCANCODE_F7,true);key(SDL_SCANCODE_F7,false);ttk::input_allow_capture(false);
    for(int i=0;i<20;++i) {ttk::input_offer_gameplay_capture();CHECK(!tick().active);}
    ttk::input_allow_capture(true);
    for(int i=0;i<20;++i) {ttk::input_offer_gameplay_capture();tick();}
    CHECK(tick().active);
    focus(false);key(SDL_SCANCODE_F10,true);CHECK(!tick().active);
    focus(true);for(int i=0;i<20;++i) {ttk::input_offer_gameplay_capture();tick();}
    CHECK(tick().active);
    // Only an explicit setting enables Legacy; loading cannot override it.
    ttk_live_env_set("DNTTK_CONTROL_SCHEME","legacy");tick();
    CHECK(!ttk::input_modern_controls() && !tick().active);
    key(SDL_SCANCODE_W,true);key(SDL_SCANCODE_A,true);
    CHECK((ttk::input_pad() & (16|128))==0);
    ttk::input_state_loaded();key(SDL_SCANCODE_F10,true);tick();
    CHECK(!ttk::input_modern_controls());
    ttk_live_env_set("DNTTK_CONTROL_SCHEME","modern");
    key(SDL_SCANCODE_W,false);key(SDL_SCANCODE_A,false);CHECK(tick().active);
    // D10 camera keys: V counts recenter presses per epoch, H cycles the shoulder
    // center -> right -> left -> center from the profile's starting side. Neither
    // becomes a guest command; the original camera option ignores both.
    ttk::input_release();SDL_unsetenv_unsafe("DNTTK_INPUT_BINDINGS");SDL_setenv_unsafe("DNTTK_CAMERA_SHOULDER","left",1);
    ttk::input_init(win);focus(true);capture_fixture();
    CHECK(tick().active && tick().shoulder==-1 && tick().recenter_total==0);
    key(SDL_SCANCODE_V,true);key(SDL_SCANCODE_V,true,true);key(SDL_SCANCODE_V,false); // auto-repeat is one press
    key(SDL_SCANCODE_V,true);key(SDL_SCANCODE_V,false);
    CHECK(tick().recenter_total==2 && tick().command_count==0);
    // D10C look keys are held actions, never guest commands or pad bits.
    key(SDL_SCANCODE_PAGEUP,true);key(SDL_SCANCODE_END,true);
    {const auto& f=tick();CHECK(f.held[ttk::look_up] && f.held[ttk::lock_look_down] && !f.held[ttk::look_down] && f.command_count==0);}
    key(SDL_SCANCODE_PAGEUP,false);key(SDL_SCANCODE_END,false);CHECK(!tick().held[ttk::look_up]);
    // A Look press is counted (a tap between camera updates); repeats and Lock are not.
    key(SDL_SCANCODE_PAGEDOWN,true);key(SDL_SCANCODE_PAGEDOWN,true,true);key(SDL_SCANCODE_PAGEDOWN,false);
    key(SDL_SCANCODE_HOME,true);key(SDL_SCANCODE_HOME,false);
    CHECK(tick().look_press_total==2 && tick().command_count==0);
    key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);CHECK(tick().shoulder==0);
    key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);CHECK(tick().shoulder==1);
    key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);CHECK(tick().shoulder==-1 && tick().command_count==0);
    ttk::input_release();key(SDL_SCANCODE_V,true);key(SDL_SCANCODE_V,false);key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);
    capture_fixture();CHECK(tick().recenter_total==0 && tick().shoulder==-1); // uncaptured keys ignored; side kept
    SDL_setenv_unsafe("DNTTK_CAMERA_MODE","original",1);
    key(SDL_SCANCODE_V,true);key(SDL_SCANCODE_V,false);key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);
    CHECK(tick().recenter_total==0 && tick().shoulder==-1);
    SDL_unsetenv_unsafe("DNTTK_CAMERA_MODE");SDL_unsetenv_unsafe("DNTTK_CAMERA_SHOULDER");
    // D11 view key: P toggles first/third person from the profile's saved view;
    // auto-repeat is one press, it is never a guest command, and uncaptured or
    // original-camera presses are ignored.
    ttk::input_release();SDL_setenv_unsafe("DNTTK_CAMERA_VIEW","first",1);
    ttk::input_init(win);focus(true);capture_fixture();
    CHECK(tick().active && tick().first_person);
    key(SDL_SCANCODE_P,true);key(SDL_SCANCODE_P,true,true);key(SDL_SCANCODE_P,false);
    CHECK(!tick().first_person && tick().command_count==0);
    key(SDL_SCANCODE_P,true);key(SDL_SCANCODE_P,false);CHECK(tick().first_person);
    ttk::input_release();key(SDL_SCANCODE_P,true);key(SDL_SCANCODE_P,false);
    capture_fixture();CHECK(tick().first_person);
    // D26B: preference is observable even while every gameplay action is off.
    ttk::input_set_shoulder(1);ttk::input_allow_capture(false);
    for(int i=0;i<5;++i) {
        const auto& f=tick();CHECK(!f.active && f.first_person && f.shoulder==1);
        CHECK(f.move_x==0 && f.move_y==0 && f.look_x==0 && f.command_count==0);
    }
    ttk::input_state_loaded();CHECK(tick().first_person);
    SDL_setenv_unsafe("DNTTK_CONTROL_SCHEME","legacy",1);
    CHECK(!tick().active && tick().first_person);
    SDL_unsetenv_unsafe("DNTTK_CONTROL_SCHEME");
    ttk::input_allow_capture(true);capture_fixture();CHECK(tick().first_person);
    SDL_setenv_unsafe("DNTTK_CAMERA_MODE","original",1);
    key(SDL_SCANCODE_P,true);key(SDL_SCANCODE_P,false);CHECK(tick().first_person);
    SDL_unsetenv_unsafe("DNTTK_CAMERA_MODE");SDL_unsetenv_unsafe("DNTTK_CAMERA_VIEW");
    ttk::input_release();ttk::input_init(win);focus(true);capture_fixture();
    CHECK(!tick().first_person); // third person without a saved view
    // D08T1 pushable objects. E is the normal interaction/mantle: W + E walks
    // into the object with Cross (the original climbs a climbable one). Holding
    // RMB (view aiming) requests the grab: no directions, a holster pulse when
    // armed, Cross only once Duke idles; the grab latches and holds Cross while
    // RMB is held; W/S become Up/Down; Space, fire and Circle wait; releasing RMB
    // lets go; any end needs a fresh press. Legacy aiming grabs with Alt.
    ttk::input_release();SDL_setenv_unsafe("DNTTK_WEAPON_AIM","view",1);ttk::input_init(win);focus(true);capture_fixture();
    auto rmb=[](bool down){SDL_Event e{};e.type=down?SDL_MOUSEBUTTONDOWN:SDL_MOUSEBUTTONUP;
        e.button.windowID=SDL_GetWindowID(win);e.button.button=3;ttk::input_event(e);};
    ready=true;flight=false;traversal=false;holstered=true;push_contact=true;push_idle=false;
    notices=0;tick();CHECK(notices==1); // first touch names the grab input
    key(SDL_SCANCODE_W,true);tick();CHECK((ttk::input_pad()&16)==0); // ordinary walk into it
    key(SDL_SCANCODE_E,true);tick();
    CHECK((ttk::input_pad()&(16|16384))==0); // W + E: Forward and Cross, the normal mantle
    key(SDL_SCANCODE_E,false);for(int i=0;i<10;++i)tick();
    rmb(true);tick();
    CHECK((ttk::input_pad()&(16|16384))==(16|16384)); // grab request owns directions
    push_idle=true;for(int i=0;i<3;++i){tick();CHECK((ttk::input_pad()&16384)!=0);} // released ticks first
    for(int i=0;i<4;++i)tick();CHECK((ttk::input_pad()&16384)==0 && (ttk::input_pad()&16)!=0);
    notices=0;push_grab=true;push_contact=false;push_idle=false;tick();CHECK(notices==1);
    key(SDL_SCANCODE_W,false);tick();
    CHECK((ttk::input_pad()&(16|64|16384))==(16|64)); // latched: Cross only
    key(SDL_SCANCODE_W,true);tick();CHECK((ttk::input_pad()&(16|16384))==0 && (ttk::input_pad()&64));
    key(SDL_SCANCODE_W,false);key(SDL_SCANCODE_S,true);tick();CHECK((ttk::input_pad()&(64|16384))==0 && (ttk::input_pad()&16));
    key(SDL_SCANCODE_S,false);
    key(SDL_SCANCODE_SPACE,true);for(int i=0;i<3;++i){tick();CHECK((ttk::input_pad()&(16384|32768))==32768);} // Space waits
    key(SDL_SCANCODE_SPACE,false);tick();
    rmb(false);tick();CHECK((ttk::input_pad()&16384)!=0); // release lets go
    key(SDL_SCANCODE_W,true); // a push cycle still running gets no direction
    for(int i=0;i<12;++i){tick();CHECK((ttk::input_pad()&(16|16384|32768))==(16|16384|32768));} // no buffered jump
    key(SDL_SCANCODE_W,false);
    push_grab=false;tick();
    // Release then E: the normal interaction Cross flows immediately.
    key(SDL_SCANCODE_E,true);tick();CHECK((ttk::input_pad()&16384)==0);key(SDL_SCANCODE_E,false);for(int i=0;i<10;++i)tick();
    // Grab again, then E while holding: let go first, E's Cross once released.
    push_contact=true;push_idle=true;rmb(true);for(int i=0;i<8;++i)tick();
    push_grab=true;push_contact=push_idle=false;tick();CHECK((ttk::input_pad()&16384)==0);
    key(SDL_SCANCODE_E,true);tick();CHECK((ttk::input_pad()&16384)!=0);
    push_grab=false;tick();CHECK((ttk::input_pad()&16384)==0);
    key(SDL_SCANCODE_E,false);for(int i=0;i<10;++i)tick();
    // RMB still held after that end: no regrab until a fresh press.
    push_contact=true;push_idle=true;for(int i=0;i<10;++i){tick();CHECK((ttk::input_pad()&16384)!=0);}
    rmb(false);tick();rmb(true);tick();CHECK((ttk::input_pad()&16)!=0); // fresh press requests
    rmb(false);tick();
    // The original letting go (blocked object, hit) ends the latch within 12 frames.
    push_contact=true;push_idle=true;rmb(true);for(int i=0;i<8;++i)tick();
    push_grab=true;push_contact=push_idle=false;tick();CHECK((ttk::input_pad()&16384)==0);push_grab=false;
    for(int i=0;i<13;++i)tick();
    push_grab=true;tick();CHECK((ttk::input_pad()&16384)!=0); // held RMB never relatches
    rmb(false);push_grab=false;push_contact=false;tick();
    // Armed: the request pulses Circle once (holster) before the grab.
    holstered=false;push_contact=true;push_idle=false;rmb(true);tick();CHECK((ttk::input_pad()&8192)==0);
    for(int i=0;i<6;++i)tick();CHECK((ttk::input_pad()&8192)!=0);
    rmb(false);holstered=true;push_contact=false;tick();
    // RMB with nothing to grab does nothing: no R1, no Cross, movement unchanged.
    key(SDL_SCANCODE_W,true);rmb(true);tick();CHECK((ttk::input_pad()&(16|2048|16384))==(2048|16384));
    rmb(false);key(SDL_SCANCODE_W,false);tick();
    // Legacy aiming: RMB is precision aim, Alt (either key) grabs.
    SDL_setenv_unsafe("DNTTK_WEAPON_AIM","original",1);push_contact=true;push_idle=true;
    rmb(true);for(int i=0;i<8;++i)tick();CHECK((ttk::input_pad()&(2048|16384))==16384);
    rmb(false);tick();key(SDL_SCANCODE_RALT,true);for(int i=0;i<8;++i)tick();CHECK((ttk::input_pad()&16384)==0);
    push_grab=true;tick();CHECK((ttk::input_pad()&16384)==0);
    // Focus loss drops a latched grab (Cross released, the original lets go).
    focus(false);focus(true);capture_fixture();tick();CHECK((ttk::input_pad()&16384)!=0);
    key(SDL_SCANCODE_RALT,false);push_grab=false;push_contact=false;push_idle=false;tick();
    SDL_unsetenv_unsafe("DNTTK_WEAPON_AIM");
    // D08V: a mantle/hang or unowned fall keeps the camera lease: no tank
    // fallback banner; W/S still reach Up/Down, A/D the strafe pads, never turns.
    ttk::input_release();ttk::input_init(win);focus(true);capture_fixture();
    ready=false;flight=false;traversal=false;trav_camera=true;notices=0;
    key(SDL_SCANCODE_W,true);key(SDL_SCANCODE_A,true);
    for(int i=0;i<60;++i){tick();CHECK((ttk::input_pad()&(16|0x100))==0 && (ttk::input_pad()&(128|64))==(128|64));}
    CHECK(notices==0);
    // D22B: a dodge roll keeps the camera but gets no directions (a held A/D
    // would start the original strafe when it ends).
    roll_camera=true;
    for(int i=0;i<30;++i){tick();CHECK((ttk::input_pad()&(16|64|128|32|0x100|0x200))==(16|64|128|32|0x100|0x200));}
    CHECK(notices==0);
    roll_camera=false;
    trav_camera=false;
    key(SDL_SCANCODE_W,false);key(SDL_SCANCODE_A,false);tick();
    // D08U: E at a ladder top asks for the mount (after the E stow) instead of
    // pulsing Cross; the first arrivals name the key; S on a ladder adds Cross.
    ttk::input_release();ttk::input_init(win);focus(true);capture_fixture();
    ready=true;flight=false;traversal=false;trav_camera=false;holstered=false;notices=0;ladder_requests=0;
    ladder_top=true;tick();CHECK(notices==1);tick();CHECK(notices==1);
    // D08T2: a fresh ladder top names E again once the cooldown has passed.
    ladder_top=false;tick();ladder_top=true;tick();CHECK(notices==1);
    ladder_top=false;for(int i=0;i<300;++i)tick();ladder_top=true;tick();CHECK(notices==2);
    ladder_top=false;tick();ladder_top=true;tick();CHECK(notices==2);
    key(SDL_SCANCODE_E,true);tick();CHECK(ladder_requests==0 && (ttk::input_pad()&8192)==0); // armed: holster first
    key(SDL_SCANCODE_E,false);for(int i=0;i<4;++i)tick();holstered=true;tick();
    CHECK(ladder_requests==1);for(int i=0;i<10;++i){tick();CHECK((ttk::input_pad()&16384)!=0);}
    ladder_top=false;ready=false;traversal=true;ladder_on=true;
    key(SDL_SCANCODE_S,true);tick();CHECK((ttk::input_pad()&(64|16384))==0);
    ladder_on=false;tick();CHECK((ttk::input_pad()&64)==0 && (ttk::input_pad()&16384)!=0);
    // D08U1: S in the bottom-rung hang lets go (Square) instead of Down; on the way in, neither.
    ladder_hang=1;tick();CHECK((ttk::input_pad()&64)!=0 && (ttk::input_pad()&32768)!=0);
    ladder_hang=2;tick();CHECK((ttk::input_pad()&64)!=0 && (ttk::input_pad()&32768)==0);
    ladder_hang=0;tick();CHECK((ttk::input_pad()&64)==0 && (ttk::input_pad()&32768)!=0);
    // D08U1: the last rung of a ladder that ends above the floor: S lets go (Square), no Down or Cross.
    ladder_on=true;ladder_end=true;tick();CHECK((ttk::input_pad()&(64|16384|32768))==(64|16384));
    ladder_end=false;ladder_on=false;tick();
    ladder_on=true;key(SDL_SCANCODE_W,true);tick();CHECK((ttk::input_pad()&16384)!=0);
    ladder_exit=true;tick();CHECK((ttk::input_pad()&(16|64))==(16|64)); // exits play out with neutral directions
    ladder_exit=false;key(SDL_SCANCODE_W,false);key(SDL_SCANCODE_S,false);ladder_on=false;tick();
    // D08T8: Space releases a ledge through Down, overriding held W/E/A.
    ledge_hang=true;key(SDL_SCANCODE_SPACE,true);key(SDL_SCANCODE_W,true);key(SDL_SCANCODE_E,true);key(SDL_SCANCODE_A,true);tick();
    CHECK((ttk::input_pad()&(16|32|64|128|16384|32768))==(16|32|128|16384|32768));
    ledge_hang=false;tick();CHECK((ttk::input_pad()&(16|64|16384|32768))==(16|64|16384|32768)); // committed release
    traversal=false;tick();CHECK((ttk::input_pad()&(16384|32768))==(16384|32768)); // no re-jump/regrab
    key(SDL_SCANCODE_SPACE,false);key(SDL_SCANCODE_W,false);key(SDL_SCANCODE_E,false);key(SDL_SCANCODE_A,false);tick();(void)ttk::input_pad();
    traversal=true;key(SDL_SCANCODE_SPACE,true);tick();CHECK((ttk::input_pad()&32768)==0); // new ladder jump retains Square
    key(SDL_SCANCODE_SPACE,false);tick();
    // D08J2: A/D on a ladder send Left/Right; on a pole or chain (192..195) they are swapped.
    key(SDL_SCANCODE_A,true);tick();CHECK((ttk::input_pad()&(32|128))==32);
    pole=true;tick();CHECK((ttk::input_pad()&(32|128))==128);
    key(SDL_SCANCODE_A,false);key(SDL_SCANCODE_D,true);tick();CHECK((ttk::input_pad()&(32|128))==32);
    pole=false;tick();CHECK((ttk::input_pad()&(32|128))==128);
    key(SDL_SCANCODE_D,false);tick();
    // D08J3: seen from Duke's front (free camera) the swap is undone; the side
    // is only re-read while A/D are up.
    pole=true;pole_front=true;tick();(void)ttk::input_pad();key(SDL_SCANCODE_A,true);tick();CHECK((ttk::input_pad()&(32|128))==32);
    pole_front=false;tick();CHECK((ttk::input_pad()&(32|128))==32);
    key(SDL_SCANCODE_A,false);tick();(void)ttk::input_pad();key(SDL_SCANCODE_A,true);tick();CHECK((ttk::input_pad()&(32|128))==128);
    key(SDL_SCANCODE_A,false);pole=false;tick();
    // D08J5: Ctrl lets go of a pole or chain (Square, no Down); S at its end
    // with nothing below does too; S elsewhere stays Down.
    pole=true;key(SDL_SCANCODE_LCTRL,true);tick();CHECK((ttk::input_pad()&32768)==0);
    key(SDL_SCANCODE_LCTRL,false);key(SDL_SCANCODE_S,true);tick();CHECK((ttk::input_pad()&(64|32768))==32768);
    pole_end=true;tick();CHECK((ttk::input_pad()&(64|32768))==64);
    pole_end=false;key(SDL_SCANCODE_S,false);pole=false;key(SDL_SCANCODE_LCTRL,true);tick();CHECK((ttk::input_pad()&32768)!=0);
    key(SDL_SCANCODE_LCTRL,false);tick();
    // D08J4: under a climbable ceiling WASD reach no D-pad (they would turn
    // Duke); the host's advance is Up and Ctrl lets go (Square).
    ceiling=true;key(SDL_SCANCODE_A,true);key(SDL_SCANCODE_S,true);tick();CHECK((ttk::input_pad()&(16|32|64|128))==(16|32|64|128));
    ceiling_adv=true;tick();CHECK((ttk::input_pad()&(16|32|64|128))==(32|64|128) && (ttk::input_pad()&32768)!=0);
    key(SDL_SCANCODE_A,false);key(SDL_SCANCODE_S,false);ceiling_adv=false;ceiling_let_go=true;tick();CHECK((ttk::input_pad()&(16|32768))==16);
    ceiling_let_go=false;ceiling=false;key(SDL_SCANCODE_A,true);tick();CHECK((ttk::input_pad()&(32|128))==32);
    key(SDL_SCANCODE_A,false);traversal=false;tick();
    // E with no ladder top is the ordinary unarmed interaction.
    ready=true;key(SDL_SCANCODE_E,true);tick();CHECK(ladder_requests==1 && (ttk::input_pad()&16384)==0);key(SDL_SCANCODE_E,false);
    for(int i=0;i<40;++i)tick();holstered=false;
    // D08J1: E held at an overhead ladder presses the original jump (Square)
    // once per hold; without E, or after the press was used, nothing more.
    ttk::input_release();ttk::input_init(win);focus(true);capture_fixture();
    ready=true;holstered=true;ladder_leap=false;ladder_leaps=0;
    key(SDL_SCANCODE_E,true);tick();CHECK(ladder_leaps==0 && (ttk::input_pad()&32768)!=0);
    ladder_leap=true;tick();CHECK(ladder_leaps==1 && (ttk::input_pad()&32768)==0);
    for(int i=0;i<30;++i)tick();CHECK(ladder_leaps==1 && (ttk::input_pad()&32768)!=0);
    key(SDL_SCANCODE_E,false);for(int i=0;i<3;++i)tick();CHECK(ladder_leaps==1);
    key(SDL_SCANCODE_E,true);tick();CHECK(ladder_leaps==2 && (ttk::input_pad()&32768)==0);
    key(SDL_SCANCODE_E,false);for(int i=0;i<130;++i)tick();CHECK(ladder_leaps==2);
    // While the leap is under way the reach (Cross) stays held without E.
    ladder_leap=false;leap_on=true;tick();CHECK((ttk::input_pad()&16384)==0);
    leap_on=false;tick();CHECK((ttk::input_pad()&16384)!=0);
    holstered=false;
    // Rebinding fire preserves draw behavior; the old mouse binding is inactive.
    ttk::input_release();SDL_setenv_unsafe("DNTTK_INPUT_BINDINGS", "1:26,22,4,7,10,44,11,25,21,0,0,0,12,8,0,0,0,-3,226,51,52,27,16,225,13,17,5,47,48,24,20,30,31,32,33,34,35,36,37,38,39,54,55,75,78,74,77,80,79,63,66,6,-2,45,46",1);
    ttk::input_init(win);focus(true);capture_fixture();ready=true;flight=false;holstered=true;
    mousefire(true);tick();CHECK((ttk::input_pad()&(8192|16384))==(8192|16384));mousefire(false);
    key(SDL_SCANCODE_G,true);tick();CHECK((ttk::input_pad()&8192)==0 && (ttk::input_pad()&16384)!=0);
    key(SDL_SCANCODE_G,false);tick();CHECK((ttk::input_pad()&(8192|16384))==(8192|16384));
    // D19D: Escape / Backspace back out and Enter confirms in the original
    // menus (uncaptured); a press keeps its button while the screen changes.
    ttk::input_release();tick();
    auto pad_with=[&](SDL_Scancode code)->uint16_t {key(code,true);tick();const uint16_t v=ttk::input_pad();key(code,false);for(int i=0;i<8;++i){tick();ttk::input_pad();}return v;};
    auto pressed=[](uint16_t v){return (uint16_t)~v;};
    guest={{0x800bcbb0u,0},{0x800d24e4u,5}};                          // PRESS START
    CHECK(pressed(pad_with(SDL_SCANCODE_ESCAPE))==0 && pressed(pad_with(SDL_SCANCODE_BACKSPACE))==0);
    CHECK(pressed(pad_with(SDL_SCANCODE_RETURN))==8);
    guest[0x800d24e4u]=12;guest[0x800d24d2u]=7;                       // OPTIONS: SOUND
    CHECK(pressed(pad_with(SDL_SCANCODE_ESCAPE))==4096 && pressed(pad_with(SDL_SCANCODE_BACKSPACE))==4096);
    CHECK(pressed(pad_with(SDL_SCANCODE_RETURN))==16384);
    CHECK(pressed(pad_with(SDL_SCANCODE_V))==32768);
    guest[0x800d24d2u]=4;                                              // CONFIGURE CONTROLLER: Start
    CHECK(pressed(pad_with(SDL_SCANCODE_ESCAPE))==8 && pressed(pad_with(SDL_SCANCODE_RETURN))==8);
    guest[0x800dd276u]=1;                                              // its CONFIRM / CANCEL prompt
    CHECK(pressed(pad_with(SDL_SCANCODE_ESCAPE))==4096 && pressed(pad_with(SDL_SCANCODE_RETURN))==16384);
    guest={{0x800bcbb0u,1},{0x800be56au,12}};                         // pause menu: Triangle would QUIT
    CHECK(pressed(pad_with(SDL_SCANCODE_ESCAPE))==8 && pressed(pad_with(SDL_SCANCODE_BACKSPACE))==8);
    CHECK(pressed(pad_with(SDL_SCANCODE_RETURN))==16384);
    guest[0x800dd1c6u]=4;                                              // stale death prompt value: ignored here
    CHECK(pressed(pad_with(SDL_SCANCODE_ESCAPE))==8);
    guest[0x800dd1c4u]=1;                                              // its QUIT prompt
    CHECK(pressed(pad_with(SDL_SCANCODE_ESCAPE))==4096);
    guest={{0x800bcbb0u,1},{0x800be56au,15}};                         // death screen: Triangle QUIT, Start CONTINUE
    CHECK(pressed(pad_with(SDL_SCANCODE_ESCAPE))==0 && pressed(pad_with(SDL_SCANCODE_RETURN))==16384);
    guest={{0x800bcbb0u,1},{0x800be56au,15},{0x800dd1c6u,2}};          // death REALLY QUIT?: never Start
    CHECK(pressed(pad_with(SDL_SCANCODE_ESCAPE))==4096);
    guest={{0x800bcbb0u,1},{0x800be56au,2}};                          // pause OPTIONS
    key(SDL_SCANCODE_ESCAPE,true);tick();CHECK(pressed(ttk::input_pad())==4096);
    guest[0x800be56au]=12;                                             // back at the pause menu, Escape still held
    for(int i=0;i<10;++i){tick();CHECK(pressed(ttk::input_pad())==4096);}
    key(SDL_SCANCODE_ESCAPE,false);for(int i=0;i<8;++i)tick();CHECK(ttk::input_pad()==0xffff);
    guest={{0x800bcbb0u,0xffff}};                                      // intro FMV: Escape/Enter skip (Start)
    CHECK(pressed(pad_with(SDL_SCANCODE_ESCAPE))==8 && pressed(pad_with(SDL_SCANCODE_RETURN))==8);
    CHECK(pressed(pad_with(SDL_SCANCODE_BACKSPACE))==0);
    // Uncaptured wheel: one Down tap per notch on a menu, nothing elsewhere.
    guest={{0x800bcbb0u,0},{0x800d24e4u,7}};
    SDL_Event notch{};notch.type=SDL_MOUSEWHEEL;notch.wheel.windowID=SDL_GetWindowID(win);notch.wheel.y=-1;
    ttk::input_event(notch);ttk::input_event(notch);
    int downs=0;bool was=false;
    for(int i=0;i<40;++i){++s_frame_count;const bool d=!(ttk::input_pad()&64);if(d && !was)++downs;was=d;CHECK(!(pressed(ttk::input_pad())&~64));}
    CHECK(downs==2);
    guest={{0x800bcbb0u,1}};ttk::input_event(notch);
    for(int i=0;i<10;++i){++s_frame_count;CHECK(ttk::input_pad()==0xffff);}
    guest={{0x800bcbb0u,1}};
    // D20: F6 / F9 (bindable) and F2 / F3 reach the quick handler. A save keeps
    // the mouse; a load frees it (recapture follows); SAVE / LOAD GAME pause
    // with the Start pulse; Ignored passes on to the usual F-key release.
    ttk::input_release();unsetenv("DNTTK_INPUT_BINDINGS");
    ttk::input_init(win);focus(true);capture_fixture();key(SDL_SCANCODE_F10,false);
    CHECK(SDL_GetWindowRelativeMouseMode(win) && ttk::input_binding_code(ttk::quick_save)==SDL_SCANCODE_F6 && ttk::input_binding_code(ttk::quick_load)==SDL_SCANCODE_F9);
    ttk::input_set_quick_handler(quick_handler);
    quick_answer=ttk::QuickResult::Done;
    key(SDL_SCANCODE_F6,true);key(SDL_SCANCODE_F6,false);
    CHECK(quick_calls==1 && last_quick==ttk::Quick::Save && SDL_GetWindowRelativeMouseMode(win));
    key(SDL_SCANCODE_F6,true,true);CHECK(quick_calls==1); // repeats never save again
    {SDL_Event e{};e.type=SDL_KEYDOWN;e.key.windowID=SDL_GetWindowID(win);e.key.scancode=SDL_SCANCODE_F6;e.key.mod=SDL_KMOD_LCTRL;ttk::input_event(e);}
    CHECK(quick_calls==1 && !SDL_GetWindowRelativeMouseMode(win)); // Ctrl+F6 is not quick save
    key(SDL_SCANCODE_F6,false);capture_fixture();key(SDL_SCANCODE_F10,false);
    quick_answer=ttk::QuickResult::Load;
    key(SDL_SCANCODE_F9,true);key(SDL_SCANCODE_F9,false);
    CHECK(quick_calls==2 && last_quick==ttk::Quick::Load && !SDL_GetWindowRelativeMouseMode(win) && ttk::input_wants_initial_capture());
    capture_fixture();key(SDL_SCANCODE_F10,false);
    quick_answer=ttk::QuickResult::Pause;
    key(SDL_SCANCODE_F2,true);key(SDL_SCANCODE_F2,false);
    CHECK(quick_calls==3 && last_quick==ttk::Quick::SaveMenu && !SDL_GetWindowRelativeMouseMode(win));
    ++s_frame_count;CHECK((ttk::input_pad()&8)==0); // Start: the pause
    for(int i=0;i<12;++i){++s_frame_count;ttk::input_pad();}
    key(SDL_SCANCODE_F3,true);key(SDL_SCANCODE_F3,false);CHECK(quick_calls==4 && last_quick==ttk::Quick::LoadMenu);
    CHECK(ttk::input_quick(ttk::Quick::Load)==ttk::QuickResult::Pause && quick_calls==5); // the console's way in
    // Ignored leaves the F-key as before.
    for(int i=0;i<12;++i){++s_frame_count;ttk::input_pad();}
    capture_fixture();key(SDL_SCANCODE_F10,false);quick_answer=ttk::QuickResult::Ignored;
    key(SDL_SCANCODE_F9,true);key(SDL_SCANCODE_F9,false);
    CHECK(quick_calls==6 && !SDL_GetWindowRelativeMouseMode(win));
    ttk::input_set_quick_handler(nullptr);
    CHECK(ttk::input_quick(ttk::Quick::Save)==ttk::QuickResult::Ignored && quick_calls==6);
    capture_fixture();
    key(SDL_SCANCODE_MINUS,true);key(SDL_SCANCODE_MINUS,false);
    key(SDL_SCANCODE_EQUALS,true);key(SDL_SCANCODE_EQUALS,false);
    CHECK(ttk::input_take_hud_step()==-1);CHECK(ttk::input_take_hud_step()==1);
    CHECK(ttk::input_take_hud_step()==0);
    ttk::input_release();key(SDL_SCANCODE_MINUS,true);key(SDL_SCANCODE_MINUS,false);
    CHECK(ttk::input_take_hud_step()==0);
    capture_fixture();ttk::input_capture_key(true);
    key(SDL_SCANCODE_EQUALS,true);key(SDL_SCANCODE_EQUALS,false);
    CHECK(ttk::input_take_hud_step()==0);CHECK(ttk::input_take_captured_key()==SDL_SCANCODE_EQUALS);
    ttk::input_capture_key(false);
    ttk::input_release();capture_fixture();
    key(SDL_SCANCODE_MINUS,true);key(SDL_SCANCODE_MINUS,true,true);key(SDL_SCANCODE_MINUS,false);
    CHECK(ttk::input_take_hud_step()==-1 && ttk::input_take_hud_step()==0);
    int hud_codes[ttk::action_count];
    for(int i=0;i<ttk::action_count;++i)hud_codes[i]=ttk::input_binding_code(ttk::Action(i));
    hud_codes[ttk::hud_less]=SDL_SCANCODE_KP_MINUS;hud_codes[ttk::hud_more]=SDL_SCANCODE_KP_PLUS;
    ttk::input_set_binding_codes(hud_codes);
    key(SDL_SCANCODE_MINUS,true);key(SDL_SCANCODE_MINUS,false);CHECK(ttk::input_take_hud_step()==0);
    key(SDL_SCANCODE_KP_MINUS,true);key(SDL_SCANCODE_KP_MINUS,false);
    key(SDL_SCANCODE_KP_PLUS,true);key(SDL_SCANCODE_KP_PLUS,false);
    CHECK(ttk::input_take_hud_step()==-1 && ttk::input_take_hud_step()==1 && ttk::input_take_hud_step()==0);
    key(SDL_SCANCODE_KP_MINUS,true);ttk::input_state_loaded();CHECK(ttk::input_take_hud_step()==0);
    ttk::input_release();SDL_setenv_unsafe("DNTTK_INPUT_MODE", "vanilla", 1);
    ttk::input_init(win);focus(true);capture_fixture();
    key(SDL_SCANCODE_CAPSLOCK,true);key(SDL_SCANCODE_R,true);
    CHECK(!tick().active && ttk::input_pad()==0xffff);
    SDL_DestroyWindow(win);
    SDL_Quit();
    std::puts("PASS: independent movement, normalized diagonals, context isolation, mouse deltas, focus loss, repeat suppression, Escape/pause/inventory release, menu fallback, overlay capture guard, no implicit tank fallback, explicit persistent control scheme, F7 savestate recapture and fresh-offer capture, D08T1 hold-to-grab (E mantles, RMB/Alt grab latch, push-pull, release, fresh press, holster, legacy precision aim), D08U ladder-top E request, hint and S descent Cross, D08J1 one ladder leap jump per E hold, reach held through the leap, D08J5 Ctrl and S-at-the-end let go of a pole or chain, D19D menu Escape/Backspace/Enter/V/wheel per screen with latched buttons, D20 quick save / load and F2 / F3 keys");
}

namespace ttk { bool directional_takeoff_ready(){return preparing;} bool edge_jump_queued(){return edge_queued;}
bool short_fall_input_ready() { return shortfall && flight; } bool fire_draw_ready() { return ready && holstered; } bool airborne_input_ready() { return flight; } bool interaction_alive() { return alive; } bool interaction_restore_ready() { return ready && holstered; } bool interaction_swim_restore_ready() { return false; } bool view_aim_input_ready(){return view_aim;}
bool weapon_drawn() { return !holstered; } bool weapon_holstered() { return holstered; } bool player_identity_ready() { return true; }
bool crouch_walk_input_ready() { return crouch_walk; } bool movement_ready() { return ready; } bool locomotion_input_ready() { return flight; } bool traversal_input_ready() { return traversal; } bool traversal_camera_ready() { return trav_camera; } bool committed_camera_ready() { return roll_camera; } bool swim_input_ready() { return d21_swimming; } bool wade_full_speed_ready() { return wade_full_speed; } bool swim_host_owns_jump() { return false; } void swim_strafe_pads(uint16_t& left,uint16_t& right) { left=0x100; right=0x200; } bool swim_thrust_input_ready() { return d21_underwater; } bool jetpack_input_ready() { return jet; } bool jetpack_classic_input_ready() { return jet && jet_classic; } bool interaction_holster_ready() { return (ready || flight) && !holstered && !switching; } bool interaction_ready() { return (ready || flight || traversal) && holstered && !switching; } bool push_grab_ready() { return push_grab; } bool push_contact_ready() { return push_contact; } bool push_idle_ready() { return push_idle; } uint16_t push_pad(float,float y) { return !push_grab?0:y>0?16:y<0?64:0; } const char* controls_debug_json() { return "{}"; } const char* lease_refusal_reason() { return refusal; }
bool ladder_top_available() { return ladder_top; } void ladder_top_request() { ++ladder_requests; } bool ladder_descent_ready() { return ladder_on && traversal; } int ladder_bottom_hang() { return traversal ? ladder_hang : 0; } bool ladder_end_below() { return traversal && ladder_end; } bool ladder_mount_finishing() { return false; } bool ladder_exit_ready() { return ladder_exit && traversal; } bool object_hang_release_ready() { return false; } bool ledge_hang_ready() { return ledge_hang && traversal; } bool pole_sidestep_ready() { return pole && traversal; } bool pole_view_from_front(bool held) { static bool front; if(!held) front=pole_front; return front; } bool pole_let_go_ready() { return pole_end && pole && traversal; } bool ceiling_hang_ready() { return ceiling && traversal; } bool ceiling_hang_advance() { return ceiling_adv && ceiling && traversal; } bool ceiling_hang_drop() { return ceiling_let_go && ceiling && traversal; }
bool ladder_leap_ready() { return ladder_leap; } bool ladder_leap_active() { return leap_on; } void ladder_leap_note() { ++ladder_leaps; } }
