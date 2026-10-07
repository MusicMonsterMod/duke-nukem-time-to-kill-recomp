// SDL integration of production input module; no guest/disc/card dependencies.
#include "pc_input.h"
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
static SDL_Window* win;
static bool inventory_visible;
namespace ttk {
bool hold_button_sampled(){return false;}
bool inventory_visible(){return ::inventory_visible;}
unsigned mission_presses;int mission_steps;void mission_browse_press(int d){++mission_presses;mission_steps+=d;}}
static bool ready, holstered, flight, traversal, preparing, shortfall, switching, jet, jet_classic, edge_queued;
static bool alive=true,view_aim;
static bool push_grab,push_contact,push_idle;
static bool trav_camera, roll_camera;
static bool ladder_top,ladder_on,ladder_exit,ladder_end,pole;static int ladder_hang;
static unsigned ladder_requests;
static bool ladder_leap,leap_on;static unsigned ladder_leaps;
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
int main() {
    CHECK(SDL_Init(SDL_INIT_VIDEO) == 0);
    win = SDL_CreateWindow("D04 native test", 0, 0, 320, 240, 0);
    CHECK(win);
    SDL_setenv_unsafe("DNTTK_INPUT_MODE", "modernized", 1);
    ttk::input_init(win);
    CHECK(std::strcmp(ttk::input_binding_name(ttk::original_aim),"Unbound")==0);
    CHECK(std::strcmp(ttk::input_binding_name(ttk::grab),"Mouse2")==0);
    CHECK(std::strcmp(ttk::input_binding_name(ttk::grab_alt),"Left Alt")==0);
    CHECK(std::strcmp(ttk::input_binding_name(ttk::item_use),"U")==0);
    focus(true);
    key(SDL_SCANCODE_W, true);
    CHECK(tick().move_y == 0); // menu context never leaks movement
    key(SDL_SCANCODE_F10, true);
    CHECK(SDL_GetWindowRelativeMouseMode(win));
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
    CHECK(tick().move_y == 1 && (ttk::input_pad() & 16)==0); // captured tank fallback until a lease
    key(SDL_SCANCODE_A, true);
    CHECK((ttk::input_pad() & (16|0x100))==0); // W forward + A L2 strafe, never D-pad turn
    CHECK((ttk::input_pad() & 128)==128);
    key(SDL_SCANCODE_A, false);
    key(SDL_SCANCODE_E,true);CHECK((ttk::input_pad() & 16384)!=0);
    holstered=true;ready=true;CHECK((ttk::input_pad() & 16384)==0);
    key(SDL_SCANCODE_E,false);holstered=false;
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
    focus(false);focus(true);key(SDL_SCANCODE_F10,true);
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
    focus(false);focus(true);key(SDL_SCANCODE_F10,true);ready=true;traversal=false;tick();
    CHECK((ttk::input_pad()&(8192|16384))==(8192|16384));
    holstered=false;
    key(SDL_SCANCODE_E,true);tick();focus(false);focus(true);key(SDL_SCANCODE_F10,true);
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
    auto wheel=[](float y){SDL_Event e{};e.type=SDL_MOUSEWHEEL;e.wheel.windowID=SDL_GetWindowID(win);e.wheel.y=y;ttk::input_event(e);};
    wheel(0.5f);CHECK(tick().command_count==0);wheel(0.5f);CHECK(tick().commands[0]==ttk::weapon_previous);ack();
    wheel(-3);CHECK(tick().command_count==3);ack();
    key(SDL_SCANCODE_LCTRL,true);CHECK(tick().active && tick().held[ttk::crouch]);
    CHECK((ttk::input_pad()&4096)!=0);key(SDL_SCANCODE_LCTRL,false);CHECK(!tick().held[ttk::crouch]);
    key(SDL_SCANCODE_LALT,true);wheel(2);CHECK(tick().command_count==0 && tick().distance_total==2);
    key(SDL_SCANCODE_LALT,false);wheel(-1);CHECK(tick().commands[0]==ttk::weapon_next);ack();
    key(SDL_SCANCODE_X,true);CHECK(tick().commands[0]==ttk::weapon_last && (ttk::input_pad()&16384)!=0);key(SDL_SCANCODE_X,false);ack();
    wheel(-99);CHECK(tick().command_count==32);ack();CHECK(tick().command_count==0);
    wheel(-1);for(int i=0;i<10;++i)tick();CHECK(tick().command_count==0); // expires if no guest consumer
    wheel(-3);focus(false);CHECK(tick().command_count==0);focus(true);key(SDL_SCANCODE_F10,true);
    key(SDL_SCANCODE_W,true);ready = true;
    CHECK((ttk::input_pad() & 1040) == 0); // default walk + forward
    key(SDL_SCANCODE_LSHIFT, true);
    CHECK((ttk::input_pad() & 1024) != 0); // hold run
    ready=false;flight=true;
    CHECK((ttk::input_pad() & 16)==0 && (ttk::input_pad() & 1024)!=0);
    flight=false;CHECK((ttk::input_pad() & 16)==0);ready=true;
    // D08Q6 Modern flight: the original still gets its flight pads (flame,
    // lean poses, thrust state; jetpack.inc cancels their thrust) but never
    // Shift's hover toggle (L1) or a Square that Ctrl cancels.
    ready=false;jet=true;key(SDL_SCANCODE_LSHIFT,false);
    CHECK((ttk::input_pad() & 16)==0 && (ttk::input_pad() & 0x300)==0x300); // W held
    key(SDL_SCANCODE_W,false);key(SDL_SCANCODE_LEFT,true);CHECK((ttk::input_pad() & 0x100)==0 && (ttk::input_pad() & 0xa0)==0xa0);
    key(SDL_SCANCODE_LEFT,false);key(SDL_SCANCODE_SPACE,true);CHECK((ttk::input_pad() & 32768)==0);
    key(SDL_SCANCODE_LCTRL,true);CHECK((ttk::input_pad() & 32768)!=0);key(SDL_SCANCODE_LCTRL,false);key(SDL_SCANCODE_SPACE,false);
    key(SDL_SCANCODE_LSHIFT,true);CHECK(ttk::input_pad()==0xffff);key(SDL_SCANCODE_LSHIFT,false);
    key(SDL_SCANCODE_RSHIFT,true);CHECK(ttk::input_pad()==0xffff);key(SDL_SCANCODE_RSHIFT,false);
    // D08R Classic flight: W/S → original Up/Down, A/D → layout strafe pads,
    // arrows the same (never D-pad turns), Space passes as Square, Shift (either)
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
    key(SDL_SCANCODE_DOWN,false);key(SDL_SCANCODE_LEFT,true);CHECK((ttk::input_pad() & 0x100)==0 && (ttk::input_pad() & 0x2f0)==0x2f0);
    key(SDL_SCANCODE_LEFT,false);key(SDL_SCANCODE_RIGHT,true);CHECK((ttk::input_pad() & 0x200)==0 && (ttk::input_pad() & 0x1f0)==0x1f0);
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
    key(SDL_SCANCODE_CAPSLOCK, true, true);
    CHECK((ttk::input_pad() & 1024) != 0); // no repeat toggle
    CHECK(notices==1);
    key(SDL_SCANCODE_LSHIFT, true);
    CHECK((ttk::input_pad() & 1024) == 0); // temporary walk while autorun
    focus(false); focus(true);
    key(SDL_SCANCODE_F10, true);
    CHECK((ttk::input_pad() & 1024) != 0); // toggle survives capture/focus
    key(SDL_SCANCODE_CAPSLOCK, true);
    CHECK((ttk::input_pad() & 1024) == 0);
    CHECK(notices==2); // RUN MODE OFF
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
    key(SDL_SCANCODE_F10, true);
    CHECK(tick().move_y == 0);
    motion(20, 20);
    key(SDL_SCANCODE_ESCAPE, true);
    CHECK(tick().look_x == 0 && !SDL_GetWindowRelativeMouseMode(win));
    CHECK((ttk::input_pad() & 8)==0);
    key(SDL_SCANCODE_ESCAPE,false);
    CHECK((ttk::input_pad() & 8)==0); // a quick tap still reaches the pad poll
    for(int i=0;i<7;++i)tick();
    CHECK((ttk::input_pad() & 8)!=0);
    key(SDL_SCANCODE_UP, true);
    key(SDL_SCANCODE_X, true);
    CHECK(ttk::input_pad() == (uint16_t)(0xffff & ~0x4010));
    ttk::input_release();
    key(SDL_SCANCODE_F10, true);
    key(SDL_SCANCODE_RETURN, true);
    // Captured Enter uses selected inventory; it must not release like Escape.
    CHECK(tick().active && tick().command_count==1 && tick().commands[0]==ttk::item_use);
    key(SDL_SCANCODE_RETURN,false);ack();
    ttk::input_release();
    ttk::input_allow_capture(false);
    key(SDL_SCANCODE_F10, true);
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    ttk::input_allow_capture(true);
    key(SDL_SCANCODE_F10, true);
    ttk::input_pad_context(0xfffe); // controller inventory returns to menu context
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    CHECK(!(ttk::input_pad()&1));
    for(int i=0;i<8;++i)ttk::input_frame();
    CHECK(ttk::input_pad()&1);
    key(SDL_SCANCODE_F10,true);ttk::input_pad_context(0xfffe);
    ttk::input_release();CHECK(ttk::input_pad()&1); // focus/reset cannot leave Select queued
    key(SDL_SCANCODE_F10, true);
    ttk::input_pad_context(0xfff7); // controller pause releases too
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    key(SDL_SCANCODE_Z, true);
    CHECK((ttk::input_pad() & 4096) == 0); // game's menu back is Triangle
    key(SDL_SCANCODE_F10, true);
    SDL_Event quit{}; quit.type = SDL_QUIT;
    ttk::input_event(quit);
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    // Enter uses selected inventory while captured, once per physical press.
    ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);tick();
    key(SDL_SCANCODE_RETURN,true);
    CHECK(tick().active && tick().command_count==1 && tick().commands[0]==ttk::item_use);
    CHECK(ttk::input_pad()&8);ack();
    for(int i=0;i<150;++i){key(SDL_SCANCODE_RETURN,true,true);CHECK(tick().active && !tick().command_count && (ttk::input_pad()&8));}
    focus(false);focus(true);key(SDL_SCANCODE_F10,true);
    key(SDL_SCANCODE_RETURN,true,true);CHECK(tick().active && !tick().command_count && (ttk::input_pad()&8));
    key(SDL_SCANCODE_RETURN,false);key(SDL_SCANCODE_RETURN,true);
    CHECK(tick().active && tick().command_count==1 && tick().commands[0]==ttk::item_use);
    key(SDL_SCANCODE_RETURN,false);ack();
    key(SDL_SCANCODE_F10,true);tick();
    key(SDL_SCANCODE_RETURN,true);tick();focus(false);
    key(SDL_SCANCODE_RETURN,false); // release may arrive while unfocused
    focus(true);key(SDL_SCANCODE_F10,true);
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
    SDL_setenv_unsafe("DNTTK_INPUT_BINDINGS", "1:26,22,4,7,-1,44,11,25,21,0,0,0,12,8,0,0,0,-3,226,51,52,27,16,225,13,17,5,47,48,24,20,30,31,32,33,34,35,36,37,38,39,54,55", 1);
    ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);ready=true;
    CHECK((ttk::input_pad() & 1024) == 0); // launch resets autorun off
    key(SDL_SCANCODE_R,true);CHECK((ttk::input_pad() & 1024) != 0);
    key(SDL_SCANCODE_R,false);key(SDL_SCANCODE_LSHIFT,true);
    CHECK((ttk::input_pad() & 1024) == 0); // old key no longer runs
    ttk::input_release();key(SDL_SCANCODE_CAPSLOCK,true);key(SDL_SCANCODE_F10,true);
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
    for(int i=0;i<20;++i){ttk::input_offer_gameplay_capture();CHECK(!tick().active);}
    key(SDL_SCANCODE_F10,true);ready=true;
    // A complete jump tap between polls must survive, but only on the ground.
    key(SDL_SCANCODE_SPACE,true);key(SDL_SCANCODE_SPACE,false);tick();
    CHECK((ttk::input_pad() & 32768)==0);
    ready=false;flight=true;CHECK((ttk::input_pad() & 32768)!=0);
    flight=false;ready=true;CHECK((ttk::input_pad() & 32768)==0);
    for(int i=0;i<9;++i){key(SDL_SCANCODE_SPACE,true,true);tick();}
    CHECK((ttk::input_pad() & 32768)!=0); // repeat does not refresh the buffer
    key(SDL_SCANCODE_SPACE,true);key(SDL_SCANCODE_SPACE,false);
    focus(false);focus(true);key(SDL_SCANCODE_F10,true);CHECK((ttk::input_pad() & 32768)!=0);
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
    key(SDL_SCANCODE_F10,true);CHECK((ttk::input_pad() & 16)==0);
    key(SDL_SCANCODE_ESCAPE,true);CHECK((ttk::input_pad() & 16)!=0);
    key(SDL_SCANCODE_ESCAPE,false);key(SDL_SCANCODE_F10,true);preparing=false;
    CHECK((ttk::input_pad() & 16)!=0);
    // Small-drop landing forwards live movement without the original walk-stop
    // selector; release/focus still removes movement immediately.
    ready=false;flight=true;shortfall=true;key(SDL_SCANCODE_W,true);tick();
    CHECK((ttk::input_pad() & 16)==0 && (ttk::input_pad() & 1024)!=0);
    key(SDL_SCANCODE_W,false);tick();CHECK((ttk::input_pad() & 16)!=0);
    focus(false);CHECK(ttk::input_pad()==65535);focus(true);key(SDL_SCANCODE_F10,true);shortfall=false;
    // E-owned stow includes the post-holster blend, bounded by capture/focus/deadline.
    ready=false;flight=true;holstered=false;
    key(SDL_SCANCODE_E,true);key(SDL_SCANCODE_E,false);tick();
    CHECK(ttk::input_auto_stow_pending());
    CHECK((ttk::input_pad() & 16384)!=0);
    holstered=true;tick();CHECK(ttk::input_auto_stow_pending());
    for(int i=0;i<122;++i)tick();CHECK(!ttk::input_auto_stow_pending());
    focus(false);CHECK(!ttk::input_auto_stow_pending());focus(true);key(SDL_SCANCODE_F10,true);
    // An E tap during a weapon switch survives its transient equipment0;
    // it spends the stow pulse only after the intended new weapon is settled.
    switching=true;holstered=false;key(SDL_SCANCODE_E,true);key(SDL_SCANCODE_E,false);tick();
    CHECK((ttk::input_pad()&(8192|16384))==(8192|16384));
    holstered=true;tick();CHECK((ttk::input_pad()&16384)!=0);
    holstered=false;switching=false;tick();CHECK((ttk::input_pad()&8192)==0);
    holstered=true;tick();CHECK((ttk::input_pad()&16384)==0);
    ttk::input_release();key(SDL_SCANCODE_F10,true);
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
    ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);ready=true;flight=false;
    key(SDL_SCANCODE_D,true);CHECK(tick().move_x==1);key(SDL_SCANCODE_D,false);
    key(SDL_SCANCODE_N,true);key(SDL_SCANCODE_N,false);tick();CHECK(ttk::input_pad()==65535);
    for(char c:std::string("stuff")){key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),false);tick();}
    CHECK(ttk::input_take_cheat()==ttk::Cheat::Stuff && ttk::input_take_cheat()==ttk::Cheat::None);
    CHECK(tick().command_count==0 && tick().move_x==0);
    for(auto code:ttk::cheat_codes) {
        for(const char* c=code.text;*c;++c){key(SDL_Scancode(SDL_SCANCODE_A+*c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+*c-'a'),false);tick();}
        CHECK(ttk::input_take_cheat()==code.action && ttk::input_take_cheat()==ttk::Cheat::None);
    }
    for(char c:std::string("dnk")){key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),true);key(SDL_Scancode(SDL_SCANCODE_A+c-'a'),false);tick();}
    focus(false);focus(true);key(SDL_SCANCODE_F10,true);
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
    // F10 opts out of auto-capture. F7 (savestate load) must request recapture
    // like Escape, including when the mouse is already free, and must not eat a
    // gameplay offer while the F7 menu holds allow_capture false.
    ttk::input_release();SDL_setenv_unsafe("DNTTK_INPUT_MODE","modernized",1);
    unsetenv("DNTTK_INPUT_BINDINGS");
    ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);
    CHECK(SDL_GetWindowRelativeMouseMode(win));
    CHECK(!ttk::input_wants_initial_capture());
    key(SDL_SCANCODE_F7,true);key(SDL_SCANCODE_F7,false);
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    CHECK(ttk::input_wants_initial_capture());
    ttk::input_allow_capture(false);
    ttk::input_offer_gameplay_capture();
    for(int i=0;i<20;++i)tick();
    CHECK(!SDL_GetWindowRelativeMouseMode(win) && ttk::input_wants_initial_capture());
    ttk::input_allow_capture(true);
    // The player update keeps offering every frame while gameplay runs.
    for(int i=0;i<20;++i){ttk::input_offer_gameplay_capture();tick();}
    CHECK(SDL_GetWindowRelativeMouseMode(win) && tick().active);
    CHECK(!ttk::input_wants_initial_capture());
    // Escape releases; the original pause menu stops the offers. A stale offer
    // (older than the freshness window) must not capture inside that menu.
    key(SDL_SCANCODE_ESCAPE,true);key(SDL_SCANCODE_ESCAPE,false);
    ttk::input_offer_gameplay_capture();
    for(int i=0;i<30;++i)tick();
    CHECK(!SDL_GetWindowRelativeMouseMode(win) && ttk::input_wants_initial_capture());
    ttk::input_offer_gameplay_capture();tick();
    CHECK(SDL_GetWindowRelativeMouseMode(win) && tick().active);
    key(SDL_SCANCODE_F10,true);
    CHECK(!SDL_GetWindowRelativeMouseMode(win) && !ttk::input_wants_initial_capture());
    key(SDL_SCANCODE_F7,true);key(SDL_SCANCODE_F7,false);
    CHECK(ttk::input_wants_initial_capture());
    for(int i=0;i<20;++i){ttk::input_offer_gameplay_capture();tick();}
    CHECK(SDL_GetWindowRelativeMouseMode(win) && tick().active);
    // Escape keeps the mouse free while gameplay offers continue (the pause has
    // not taken hold yet); only after offers stop does resume recapture.
    key(SDL_SCANCODE_ESCAPE,true);key(SDL_SCANCODE_ESCAPE,false);
    for(int i=0;i<40;++i){ttk::input_offer_gameplay_capture();tick();}
    CHECK(!SDL_GetWindowRelativeMouseMode(win) && ttk::input_wants_initial_capture());
    for(int i=0;i<12;++i)tick();
    for(int i=0;i<3;++i){ttk::input_offer_gameplay_capture();tick();}
    CHECK(SDL_GetWindowRelativeMouseMode(win) && tick().active);
    // Escape in the pause menu (offers stopped) resumes; the resumed offers
    // recapture right away instead of being held by that second Escape.
    key(SDL_SCANCODE_ESCAPE,true);key(SDL_SCANCODE_ESCAPE,false);
    for(int i=0;i<12;++i)tick();
    CHECK(!SDL_GetWindowRelativeMouseMode(win));
    key(SDL_SCANCODE_ESCAPE,true);key(SDL_SCANCODE_ESCAPE,false);
    for(int i=0;i<14;++i){ttk::input_offer_gameplay_capture();tick();}
    CHECK(SDL_GetWindowRelativeMouseMode(win) && tick().active);
    // D22C: F10 release then F10 recapture opts back in. The console (a host
    // release, as for `level N`) is then followed by automatic recapture.
    key(SDL_SCANCODE_F10,true);CHECK(!SDL_GetWindowRelativeMouseMode(win) && !ttk::input_wants_initial_capture());
    key(SDL_SCANCODE_F10,true);CHECK(SDL_GetWindowRelativeMouseMode(win));
    ttk::input_release();CHECK(ttk::input_wants_initial_capture());
    for(int i=0;i<20;++i){ttk::input_offer_gameplay_capture();tick();}
    CHECK(SDL_GetWindowRelativeMouseMode(win) && tick().active);
    // An F10 release still keeps the mouse free through later offers.
    key(SDL_SCANCODE_F10,true);ttk::input_release();
    for(int i=0;i<20;++i){ttk::input_offer_gameplay_capture();CHECK(!tick().active);}
    key(SDL_SCANCODE_F10,true);CHECK(tick().active);
    // D10 camera keys: V counts recenter presses per epoch, H cycles the shoulder
    // center -> right -> left -> center from the profile's starting side. Neither
    // becomes a guest command; the original camera option ignores both.
    ttk::input_release();SDL_unsetenv_unsafe("DNTTK_INPUT_BINDINGS");SDL_setenv_unsafe("DNTTK_CAMERA_SHOULDER","left",1);
    ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);
    CHECK(tick().active && tick().shoulder==-1 && tick().recenter_total==0);
    key(SDL_SCANCODE_V,true);key(SDL_SCANCODE_V,true,true);key(SDL_SCANCODE_V,false); // auto-repeat is one press
    key(SDL_SCANCODE_V,true);key(SDL_SCANCODE_V,false);
    CHECK(tick().recenter_total==2 && tick().command_count==0);
    key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);CHECK(tick().shoulder==0);
    key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);CHECK(tick().shoulder==1);
    key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);CHECK(tick().shoulder==-1 && tick().command_count==0);
    ttk::input_release();key(SDL_SCANCODE_V,true);key(SDL_SCANCODE_V,false);key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);
    key(SDL_SCANCODE_F10,true);CHECK(tick().recenter_total==0 && tick().shoulder==-1); // uncaptured keys ignored; side kept
    SDL_setenv_unsafe("DNTTK_CAMERA_MODE","original",1);
    key(SDL_SCANCODE_V,true);key(SDL_SCANCODE_V,false);key(SDL_SCANCODE_H,true);key(SDL_SCANCODE_H,false);
    CHECK(tick().recenter_total==0 && tick().shoulder==-1);
    SDL_unsetenv_unsafe("DNTTK_CAMERA_MODE");SDL_unsetenv_unsafe("DNTTK_CAMERA_SHOULDER");
    // D11 view key: P toggles first/third person from the profile's saved view;
    // auto-repeat is one press, it is never a guest command, and uncaptured or
    // original-camera presses are ignored.
    ttk::input_release();SDL_setenv_unsafe("DNTTK_CAMERA_VIEW","first",1);
    ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);
    CHECK(tick().active && tick().first_person);
    key(SDL_SCANCODE_P,true);key(SDL_SCANCODE_P,true,true);key(SDL_SCANCODE_P,false);
    CHECK(!tick().first_person && tick().command_count==0);
    key(SDL_SCANCODE_P,true);key(SDL_SCANCODE_P,false);CHECK(tick().first_person);
    ttk::input_release();key(SDL_SCANCODE_P,true);key(SDL_SCANCODE_P,false);
    key(SDL_SCANCODE_F10,true);CHECK(tick().first_person);
    SDL_setenv_unsafe("DNTTK_CAMERA_MODE","original",1);
    key(SDL_SCANCODE_P,true);key(SDL_SCANCODE_P,false);CHECK(tick().first_person);
    SDL_unsetenv_unsafe("DNTTK_CAMERA_MODE");SDL_unsetenv_unsafe("DNTTK_CAMERA_VIEW");
    ttk::input_release();ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);
    CHECK(!tick().first_person); // third person without a saved view
    // D08T1 pushable objects. E is the normal interaction/mantle: W + E walks
    // into the object with Cross (the original climbs a climbable one). Holding
    // RMB (view aiming) requests the grab: no directions, a holster pulse when
    // armed, Cross only once Duke idles; the grab latches and holds Cross while
    // RMB is held; W/S become Up/Down; Space, fire and Circle wait; releasing RMB
    // lets go; any end needs a fresh press. Legacy aiming grabs with Alt.
    ttk::input_release();SDL_setenv_unsafe("DNTTK_WEAPON_AIM","view",1);ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);
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
    focus(false);focus(true);key(SDL_SCANCODE_F10,true);tick();CHECK((ttk::input_pad()&16384)!=0);
    key(SDL_SCANCODE_RALT,false);push_grab=false;push_contact=false;push_idle=false;tick();
    SDL_unsetenv_unsafe("DNTTK_WEAPON_AIM");
    // D08V: a mantle/hang or unowned fall keeps the camera lease: no tank
    // fallback banner; W/S still reach Up/Down, A/D the strafe pads, never turns.
    ttk::input_release();ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);
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
    ttk::input_release();ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);
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
    // D08J2: A/D on a ladder send Left/Right; on a pole or chain (192..195) they are swapped.
    key(SDL_SCANCODE_A,true);tick();CHECK((ttk::input_pad()&(32|128))==32);
    pole=true;tick();CHECK((ttk::input_pad()&(32|128))==128);
    key(SDL_SCANCODE_A,false);key(SDL_SCANCODE_D,true);tick();CHECK((ttk::input_pad()&(32|128))==32);
    pole=false;tick();CHECK((ttk::input_pad()&(32|128))==128);
    key(SDL_SCANCODE_D,false);traversal=false;tick();
    // E with no ladder top is the ordinary unarmed interaction.
    ready=true;key(SDL_SCANCODE_E,true);tick();CHECK(ladder_requests==1 && (ttk::input_pad()&16384)==0);key(SDL_SCANCODE_E,false);
    for(int i=0;i<40;++i)tick();holstered=false;
    // D08J1: E held at an overhead ladder presses the original jump (Square)
    // once per hold; without E, or after the press was used, nothing more.
    ttk::input_release();ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);
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
    ttk::input_release();SDL_setenv_unsafe("DNTTK_INPUT_BINDINGS", "1:26,22,4,7,10,44,11,25,21,0,0,0,12,8,0,0,0,-3,226,51,52,27,16,225,13,17,5,47,48,24,20,30,31,32,33,34,35,36,37,38,39,54,55",1);
    ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);ready=true;flight=false;holstered=true;
    mousefire(true);tick();CHECK((ttk::input_pad()&(8192|16384))==(8192|16384));mousefire(false);
    key(SDL_SCANCODE_G,true);tick();CHECK((ttk::input_pad()&8192)==0 && (ttk::input_pad()&16384)!=0);
    key(SDL_SCANCODE_G,false);tick();CHECK((ttk::input_pad()&(8192|16384))==(8192|16384));
    ttk::input_release();SDL_setenv_unsafe("DNTTK_INPUT_MODE", "vanilla", 1);
    ttk::input_init(win);focus(true);key(SDL_SCANCODE_F10,true);
    key(SDL_SCANCODE_CAPSLOCK,true);key(SDL_SCANCODE_R,true);
    CHECK(!tick().active && ttk::input_pad()==0xffff);
    SDL_DestroyWindow(win);
    SDL_Quit();
    std::puts("PASS: independent movement, normalized diagonals, context isolation, mouse deltas, focus loss, repeat suppression, Escape/pause/inventory release, menu fallback, overlay capture guard, captured tank WASD strafe fallback, F7 savestate recapture and fresh-offer capture, D08T1 hold-to-grab (E mantles, RMB/Alt grab latch, push-pull, release, fresh press, holster, legacy precision aim), D08U ladder-top E request, hint and S descent Cross, D08J1 one ladder leap jump per E hold, reach held through the leap");
}

namespace ttk { bool directional_takeoff_ready(){return preparing;} bool edge_jump_queued(){return edge_queued;}
bool short_fall_input_ready() { return shortfall && flight; } bool fire_draw_ready() { return ready && holstered; } bool airborne_input_ready() { return flight; } bool interaction_alive() { return alive; } bool interaction_restore_ready() { return ready && holstered; } bool interaction_swim_restore_ready() { return false; } bool view_aim_input_ready(){return view_aim;}
bool weapon_drawn() { return !holstered; } bool weapon_holstered() { return holstered; } bool player_identity_ready() { return true; }
bool movement_ready() { return ready; } bool locomotion_input_ready() { return flight; } bool traversal_input_ready() { return traversal; } bool traversal_camera_ready() { return trav_camera; } bool committed_camera_ready() { return roll_camera; } bool swim_input_ready() { return false; } bool wade_full_speed_ready() { return false; } bool swim_host_owns_jump() { return false; } void swim_strafe_pads(uint16_t& left,uint16_t& right) { left=0x100; right=0x200; } bool swim_thrust_input_ready() { return false; } bool jetpack_input_ready() { return jet; } bool jetpack_classic_input_ready() { return jet && jet_classic; } bool interaction_holster_ready() { return (ready || flight) && !holstered && !switching; } bool interaction_ready() { return (ready || flight || traversal) && holstered && !switching; } bool push_grab_ready() { return push_grab; } bool push_contact_ready() { return push_contact; } bool push_idle_ready() { return push_idle; } uint16_t push_pad(float,float y) { return !push_grab?0:y>0?16:y<0?64:0; } const char* controls_debug_json() { return "{}"; } const char* lease_refusal_reason() { return "state"; }
bool ladder_top_available() { return ladder_top; } void ladder_top_request() { ++ladder_requests; } bool ladder_descent_ready() { return ladder_on && traversal; } int ladder_bottom_hang() { return traversal ? ladder_hang : 0; } bool ladder_end_below() { return traversal && ladder_end; } bool ladder_mount_finishing() { return false; } bool ladder_exit_ready() { return ladder_exit && traversal; } bool object_hang_release_ready() { return false; } bool pole_sidestep_ready() { return pole && traversal; }
bool ladder_leap_ready() { return ladder_leap; } bool ladder_leap_active() { return leap_on; } void ladder_leap_note() { ++ladder_leaps; } }
