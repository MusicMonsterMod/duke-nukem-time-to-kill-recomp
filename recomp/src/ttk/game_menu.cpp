// D19: the Modernized title OPTIONS menu, replicated and extended. The original
// OPTIONS screen (frontend state 0x800d24e4 = 12, sub-screen 0x800d24d2 = 9,
// handler 0x800841c4) lists SOUND, CONTROLLER and GAME SETTINGS. Modernized
// shows the same list plus PC CONTROLS, DISPLAY and MODERNIZED, and those
// three screens, all drawn by the game's own code: text 0x8008b454, text width
// 0x8008b3c8, sprites 0x8008b678 (the prompt buttons), sliders 0x80083c40 /
// 0x80083edc and the menu sounds 0x8006bbd8. Nothing is drawn by the host, so
// the screens match the originals glyph for glyph and the D17 frontend
// interpolation redraws them like any menu.
//
// While one of these screens is up the sub-screen is 8, a value the frontend's
// handler table (0x80013200) leaves without a handler: the guest draws no menu
// and reads no menu input, and the attract demo keeps playing behind. SOUND,
// CONTROLLER and GAME SETTINGS still run the original handlers (sub-screens 7,
// 4, 11); they return 9, which brings our OPTIONS back with that item
// highlighted. Back from OPTIONS is the original's: sub-screen 10, cursor 0.
//
// Settings: a change is kept as a draft until CONFIRM (CANCEL drops it), as on
// GAME SETTINGS. Confirmed values are written to DNTTK_MENU_SETTINGS_FILE,
// which run.py folds into the Modernized profile when the game closes (and on
// the next launch). Options marked live also apply at once (ttk_live_env).
// Contract: documentation/145-d19-game-menu.md.
//
// D19H: in gameplay the pause menu (0x800be56a = 12) and the death screen (15)
// become lists in the same style (RESUME, MISSION BRIEFING, OPTIONS, LOAD
// GAME, RESTART LEVEL, MAIN MENU; CONTINUE, LOAD GAME, RESTART LEVEL, QUIT)
// on the free screen values 8 and 9, with YES / NO prompts. Each choice does
// what the original's handler and main loop do with it; the originals' LOAD
// screens are used as they are. Contract: documentation/147-d19h-pause-menu.md.
//
// D19I: in gameplay our pages are drawn at the pause dispatch, where the
// original draws its pause text, so they are as bright as the original's
// (documentation/148-d19i-menu-brightness.md).
//
// D19M: QUIT GAME at the end of both lists asks QUIT GAME? (NO first); YES
// sends the window's own close event, so the program shuts down exactly as
// when the window is closed (documentation/149-d19m-quit-game.md).
// D19P: the death list's QUIT reads QUIT TO MENU (asking QUIT TO MENU?);
// it still gives up through the game over screen to the title.
//
// D19J: the title main menu (frontend state 7), CHOOSE DIFFICULTY LEVEL (8),
// TWO PLAYER OPTIONS (8) and CHOOSE CHARACTER (9) are ours too, plus QUIT GAME.
// They run on state 12 with the free sub-screen 12 (no handler: the attract
// demo and its loads go on), drawn with the originals' own calls and
// positions, and keep their state in the originals' variables. Starting a
// game hands the last page to the original handler with one Cross press: it
// draws the same image and exits the frontend itself
// (documentation/150-d19j-title-menu.md).
#include "game_menu.h"
#include "live_env.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "code_identity.h"
#include "pc_input.h"
#include "modern_controls.h"
#include "psx_sdl.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

extern "C" int g_psx_call_bail, g_precise_mode, g_ls_mode;

namespace ttk {bool frame_rate_live();bool frame_rate_set(const char* value);}

namespace ttk {
namespace {
struct Guard {uint32_t address,size;const char* digest;};
// Read-only digests of the owned executable's bytes (tests/local/test_game_menu.py).
const Guard guards[]={
    {0x80013200u,0x30u,"c8df06f70c493d55ff7bd25b49dd202cd611692bb6a445213eed75d5dcf75dd6"},        // frontend sub-screen handler table
    {0x80023228u,0xd8u,"f977b9b9f9075bc46ea87de071e8a37912fb9a2205bb521978ee07e7e9d1dec7"},  // state 12 entry and sub-screen dispatch
    {0x80083c40u,0x170u,"7ff2ef0497a53afcf63c77b1ccb414f0f56a439844ec59d150d605b6ff480841"},    // slider init
    {0x80083edcu,0x204u,"b26f6edb991ac07642f0fd54db110440042b5884fdd45a94f6a86492bd26eef3"},    // slider draw
    {0x8008411cu,0x418u,"bf064314cbaaf45fc8e26ccd6d489a541ce00344518bb82b1b727f2b2aac5d62"},  // prompt helper and the OPTIONS handler
    {0x80013558u,0x40u,"e38a09bc1b05e4f6850535e623455c202dadb6846e794b920a5094cbc8550df2"},   // pause menu handler table
    {0x80025fb0u,0x1b4u,"954687d1cca308195b7207a86abbd7343d565e70529d788655aeef18c545dd3d"},  // pause menu dispatch
    {0x80086174u,0x308u,"a51470c88430cd8b1d31be722968c6f26c74b2b8876ccf75f4a970218049f41a"},  // pause OPTIONS handler
    {0x8008b210u,0x3e8u,"4a2ed47060bc5ca37cc486f180c0a3f949b941d9ad1e0979a6ab7f480ed45cc9"},     // glyph, width and text
    {0x8008b678u,0x40u,"226e8fcb4f143545c487c091f96ca9a853daa3a332e5ab5ad9e66cd4038c188a"},    // sprite draw entry
    // Font glyph tables from 0x800c3f38 (0x800c3f32 and 0x800c3f34 are menu
    // variables). D19P: 0x800c3f94 is the selected item (D08A6), not guarded:
    // a state with an item selected failed the check and kept the original menus.
    {0x800c3f38u,0x5cu,"a73396cc61ae5213b2ebcbc29e8300c06854a58ada6ef55179341636c2f926ee"},
    {0x800c3f98u,0x398u,"413e536637b8c40390858d4a009541249191487547b241c66544625277c9fafd"},
    {0x800c43b4u,0xf0u,"9ea07ba25cff15a7da9de43768b28d70b836cf13269c705e70e6730b55395b3b"},   // sprite records and font info
    {0x800c60f4u,0x400u,"1298dbf2ac90b8efff35a60c7ec93bc3dd6897d1434ec84ba5b1871093b09a7a"},  // text table
    // D19H: the pause and death screens our lists stand in for.
    {0x800857e4u,0x990u,"80acc5a03247a7a97930a0d8278cc68839fd0694914e5e1dee3cf04e5d5d4cc8"},  // pause top handler (12)
    {0x8008647cu,0x7f0u,"af33ec8f74e797df8f960b844588faa1011fdf22e8a6469ca34177a363b1da38"},  // death screen handler (15)
    {0x8002690cu,0x90u,"03968170f1610cb68b72423a8fb70e3664bd36899f7b5d15b2fdaba62fe67c19"},   // mode 1: dispatch, resume on 0
    {0x800c3d2cu,0x40u,"c9ef39aacbbd3027c182f6bdb255613e0794c223519cfcda97ce4a45a604c513"},   // level name string ids
    {0x800c3e10u,0x104u,"76189e6a61e1552476bb95a777ad819198c94812700157401ed2092b1f146fb8"},  // briefing string ids (13 x 10)
    {0x80094a38u,0x40u,"8e661ce6839ad2f8cefacf50eea5e0a97e9f547f5f060ce100590cc04d12f03b"},   // game event (restart, quit, replay)
    {0x8006ac24u,0x30u,"d45ab3dd28ba07a76ba1e90b5f64cc4ab0f3342cd41c224cbe0b5e022b8265d9"},   // resume: sound
    {0x8003c3dcu,0x40u,"b02ecd72ad0fe5b8072a35109c6e6c88ae7cd108fbfe80c604ab8a3999633b64"},   // resume: music
    {0x800c64f4u,0x26cu,"3716c4900759d23bbd1d3f71ca5024a5aef410814ab33aca47a435d1d6fd5945"},  // text table 0x100-0x19a (briefings)
    // D19J: the title pages we stand in for and the tables they read.
    {0x80022324u,0xfd8u,"cbcdd79ea3bfa28c53362e64f06b2bf3cb192bec173cdd906f1aa349e801b908"},  // frontend tick and states 0-12
    {0x800131b0u,0x34u,"14222073a6cd63ed99f8768a6ac93f00160a251c4ead8600cc096210b79636b8"},   // frontend state table
    {0x800131e8u,0x14u,"fe4a165c813fddb5da320da4e98bd354f3c4d3beee23f4728837ced6d7abb939"},   // main menu choice table
    {0x80084534u,0x318u,"f117c07c8be0dad501401838d2bc854c3b1a999a08808baa0187e89eb573a14d"},  // CHOOSE DIFFICULTY LEVEL
    {0x8008484cu,0x814u,"18b50b14cc0a6cdf340c521f56855fef0a0839921c294dab61750ec4d393d810"},  // TWO PLAYER OPTIONS
    {0x80085060u,0x6b0u,"5bbfb37b63bccf7306ddd0d46e2c9b7e163509795ece67dd50fddf601bd36ec5"},  // CHOOSE CHARACTER
    {0x800c3de4u,0x10u,"5cd023106a3b69feb9e1a71b77471c35d4a43b210a1751a4a3aab314a1dfc30f"},   // difficulty string ids
    {0x80016e8cu,0x30u,"9d528df15f856133001784171950a0e3c1a7c9d05b1ae7ecb1393b71a5f0b164"},   // two player levels, formats, characters
};
std::array<std::vector<uint32_t>,sizeof guards/sizeof guards[0]> expected;

constexpr uint32_t game_mode=0x800bcbb0, controller_pad=0x800be56c, geometry=0x800d2210;
constexpr uint32_t image_counter=0x800bdbd0;
constexpr uint32_t fn_text=0x8008b454, fn_width=0x8008b3c8, fn_sprite=0x8008b678;
constexpr uint32_t fn_slider_init=0x80083c40, fn_slider_draw=0x80083edc, fn_sound=0x8006bbd8;
constexpr uint32_t fn_prompt=0x8008411c, sprites=0x800c43b4, sentinel=0x800000fc;
// D19G: the glyph call (0x8008b210) adds one 40-byte packet at the pointer
// 0x800d67a8, its CLUT at +14. Section headings use the font column's amber
// ramp (row 239, 14 rows below gold 225; entry 0 transparent like gold's).
constexpr uint32_t glyph_next=0x800d67a8, glyph_size=40;
constexpr int clut_amber=14*64;
constexpr uint16_t ours_sub=8, death_sub=9;
// D19H: the pause menu (0x800be56a = 12, its prompt 0x800dd1c4, reset flag
// 0x800c3f28) and the death screen (15, prompt 0x800dd1c6, reset 0x800c3f34).
constexpr uint32_t pause_screen=0x800be56a, death_prompt=0x800dd1c6;
constexpr uint32_t pause_reset=0x800c3f28, death_reset=0x800c3f34;
constexpr uint32_t level_index=0x800be570, players=0x800c27bc, continues=0x800d7411;
constexpr uint32_t level_names=0x800c3d2c, briefings=0x800c3e10, strings=0x800c60f4;
constexpr uint16_t pause_top=12, death_top=15, pause_load=5;
constexpr uint32_t paused=0x800be568, player=0x800d7198, player_size=0x8a4, player_flags=0x224;
constexpr uint32_t fn_resume_sound=0x8006ac24, fn_resume_music=0x8003c3dc, fn_event=0x80094a38;
constexpr uint32_t continue_request=0x800d71cd; // byte 2: the death screen's CONTINUE (not the challenge stages)
constexpr uint16_t restart_code=13, main_menu_code=1; // the pause handler's results the main loop acts on
constexpr uint32_t event_replay=0xfd, event_restart=0xfe, event_quit=0xff;
// Text ids: YES, NO, REALLY QUIT?, RESTART LEVEL?, CONFIRM CONTINUE, CONFIRM
// REPLAY, CONTINUE / CONTINUES REMAINING.
constexpr int text_yes=0xfb, text_no=0xfc, text_quit=0x102, text_restart=0x103;
constexpr int text_continue=0x104, text_replay=0x113, text_remaining=0xf7, text_remaining_many=0xf8;
constexpr int text_last=0x19a; // the highest id a level name or briefing uses (guarded)
// D19J: the frontend (state 0x800d24e4 and its handlers' variables).
constexpr uint32_t fe_state=0x800d24e4, fe_cursor=0x800d24e6, fe_items=0x800d2528, fe_timer=0x800d24d0, fe_fields=0x800d21fc;
constexpr uint32_t players_choice=0x800bdea4, level_choice=0x800bdeaa;
constexpr uint32_t diff_cursor=0x800c3d7c, diff_saved=0x800c3df6, diff_fresh=0x800c3df8, diff_ids=0x800c3de4;
constexpr uint32_t two_cursor=0x800dd1b0, two_split=0x800dd1b2, two_level=0x800c3dfc, two_time=0x800c3dfe, two_kills=0x800c3e00;
constexpr uint32_t two_fresh=0x800c3e04, split_mode=0x800be564, two_levels=0x80016e8c, two_format=0x80016e98;
constexpr uint32_t char_choice=0x800dd1b8, char_locks=0x800dd1c0, char_saved=0x800c3e08, char_fresh=0x800c3e0c, char_ids=0x80016ea4;
// Pad history words (0x8001d5ac): pad 1 from +0x10 (Start), pad 2 0x54 on;
// bit 0 = pressed at this tick, bit 1 = at the tick before.
constexpr uint32_t pad_history=0x800d1aa0, history_cross=0x3c;
constexpr uint32_t history_checked[]={0x10,0x14,0x18,0x1c,0x20,0x34,0x3c};
constexpr uint16_t title_sub=12, state_start=4, state_load=10, state_difficulty=8, state_character=9;
constexpr uint16_t idle_fields=0x4650; // state 6: the main menu's idle time before PRESS START
// Where the frontend calls 0x800220f4: state 12 before its sub-screen
// dispatch, states 8 and 9 before their handlers.
constexpr uint32_t from_options=0x80023248, from_difficulty=0x80023130, from_character=0x800231b4;
constexpr uint32_t demo_load=0x800d24e2; // D19O: 0 = a demo scene's blocking load (no pad read)
// Text ids of the title pages (the originals pass the same).
constexpr int text_title1=1, text_title2=2, text_one=0x57, text_two=0x58, text_load=0x31, text_options=0x59;
constexpr int text_select=0x11, text_confirm=0x12, text_return=0x13, text_change=0x14, text_cancel=0x15, text_start=0x8a;
constexpr int text_difficulty=0x21, text_two_heading=0x2c, text_level=0x25, text_time=0x2d, text_kills=0x2e, text_split=0x24;
constexpr int text_horizontal=0x22, text_vertical=0x23, text_unlimited=0x2f, text_minutes=0x30;
constexpr int text_character=0xc9, text_player1=0xca, text_player2=0xcb;
constexpr uint16_t screen_no_titles=14; // 0x800be56a: the title pages leave their headings out
// Where OPTIONS lives: the title frontend (state 0x800d24e4 = 12, sub-screen
// 0x800d24d2, handler 0x800841c4) and the pause menu (mode 1, 0x800be56a,
// handler 0x80086174). Both use 8 as an unused screen with no handler.
struct MenuContext {
    uint16_t mode; uint32_t state; uint16_t state_value; uint32_t sub; uint16_t options,back;
    uint32_t cursor; const char* back_label;
};
const MenuContext contexts[]={
    {0,0x800d24e4u,12,0x800d24d2u,9,10,0x800c3de0u,"CANCEL"},
    {1,0,0,0x800be56au,2,12,0x800c3f32u,"RETURN"},
};
// The context showing OPTIONS (or one of our screens) now, else null.
const MenuContext* current_context() {
    const uint16_t mode=psx_mod_read_half(game_mode);
    for(const MenuContext& c:contexts)
        if(c.mode==mode && (!c.state || psx_mod_read_half(c.state)==c.state_value)) return &c;
    return nullptr;
}
// Screen values with no handler that our screens use: 8 (OPTIONS, settings,
// the pause list and its pages), 9 in gameplay (the death list), 12 on the
// title (D19J: the main menu and its pages).
bool our_screen(const MenuContext* c,uint16_t sub) {
    return sub==ours_sub || (c->mode==1 && sub==death_sub) || (c->mode==0 && sub==title_sub);
}
// Text flags: font << 8, 4 centred, 8 right-aligned, 1 highlighted (blue CLUT).
constexpr uint32_t font_small=0, font_menu=0x100, centre=4, right=8, blue=1;
enum Icon {icon_triangle=4, icon_cross=6, icon_square=7, icon_change=11, icon_select=12};

struct Choice {const char* label;const char* env;const char* json;};
enum class Section {Controls, Presentation};
struct Row {
    const char* label; Section section; const char* key; const char* env; bool live; const char* desc;
    std::vector<Choice> choices; int def; // a pick
    bool slider=false; int lo=0,hi=0,step=1; double scale=1; // value * scale is the setting
    bool link=false; // D19G: CONFIRM opens a page (KEY BINDINGS), no value of its own
};
// One value per row: a choice index, or the slider value.
using Values=std::vector<int>;
struct Screen {const char* title; std::vector<Row> rows;};

const Choice on_off[]={{"ON","1","true"},{"OFF","0","false"}};
std::vector<Screen> make_screens() {
    std::vector<Screen> s(3);
    s[0]={"PC CONTROLS",{
        {"MOUSE SENSITIVITY",Section::Controls,"mouse_sensitivity","DNTTK_MOUSE_SENSITIVITY",true,
         "HOW FAR THE CAMERA TURNS FOR EACH MOUSE MOVE",{},12,true,1,200,1,0.01},
        {"INVERT MOUSE",Section::Controls,"invert_y","DNTTK_MOUSE_INVERT_Y",true,
         "MOVING THE MOUSE UP LOOKS DOWN",{{"OFF","0","false"},{"ON","1","true"}},0},
        {"KEY BINDINGS",Section::Controls,nullptr,nullptr,true,"THE KEY OR MOUSE BUTTON FOR EACH ACTION. CONFIRM OPENS THE LIST",
         {{">",">",">"}},0,false,0,0,1,1,true},
    }};
    s[1]={"DISPLAY",{
        {"RENDERER",Section::Presentation,"renderer",nullptr,false,"SOFTWARE IS THE ORIGINAL LOOK",
         {{"OPENGL","opengl","\"opengl\""},{"SOFTWARE","software","\"software\""}},0},
        {"RESOLUTION",Section::Presentation,"internal_scale",nullptr,false,"INTERNAL 3D RESOLUTION. 1X IS THE ORIGINAL",
         {{"1X","1","1"},{"2X","2","2"},{"3X","3","3"},{"4X","4","4"}},3},
        {"DISPLAY",Section::Presentation,"display",nullptr,true,"HOW THE GAME OPENS. F11 SWITCHES TO FULLSCREEN",
         {{"WINDOWED","windowed","\"windowed\""},{"BORDERLESS","borderless","\"borderless\""},{"EXCLUSIVE","exclusive","\"exclusive\""}},0},
        {"FILTER",Section::Presentation,"output_filter",nullptr,false,"HOW THE FINISHED IMAGE IS SCALED TO THE WINDOW",
         {{"LINEAR","linear","\"linear\""},{"NEAREST","nearest","\"nearest\""}},0},
        {"WIDESCREEN",Section::Controls,"widescreen","DNTTK_WIDESCREEN",true,"A WIDER VIEW, HUD IN THE CORNERS. MOVIES STAY 4:3",
         {{"16:9","16:9","\"16:9\""},{"16:10","16:10","\"16:10\""},{"21:9","21:9","\"21:9\""},{"AUTO","auto","\"auto\""},{"OFF","off","\"off\""}},0},
        {"FRAME RATE",Section::Controls,"frame_rate","DNTTK_FRAME_RATE",true,"SMOOTHER MOTION. THE GAME SPEED NEVER CHANGES",
         {{"DISPLAY","display","\"display\""},{"30","30","\"30\""},{"60","60","\"60\""},{"120","120","\"120\""},{"144","144","\"144\""},
          {"165","165","\"165\""},{"180","180","\"180\""},{"240","240","\"240\""},{"UNLIMITED","unlimited","\"unlimited\""}},2},
        {"DRAW DISTANCE",Section::Controls,"draw_distance","DNTTK_DRAW_DISTANCE",false,"EXTENDED: TWICE THE VIEW, NO DISTANT BLACK SEAMS",
         {{"EXTENDED","extended","\"extended\""},{"ORIGINAL","original","\"original\""}},0},
        {"GEOMETRY",Section::Controls,"geometry_precision","DNTTK_GEOMETRY_PRECISION",false,"CORRECTED: STEADIER POLYGON EDGES",
         {{"ORIGINAL","original","\"original\""},{"CORRECTED","corrected","\"corrected\""}},0},
        {"TEXTURES",Section::Controls,"texture_precision","DNTTK_TEXTURE_PRECISION",false,"CORRECTED: NO TEXTURE WARP",
         {{"ORIGINAL","original","\"original\""},{"CORRECTED","corrected","\"corrected\""}},0},
        {"CPU SPEED",Section::Controls,"cpu_overclock","PSX_CPU_OVERCLOCK",false,"150 KEEPS BUSY SCENES SMOOTH. 100 IS THE ORIGINAL",
         {{"100","100","100"},{"125","125","125"},{"150","150","150"},{"175","175","175"},{"200","200","200"}},2},
        {"CPU TIMING",Section::Controls,"cpu_timing","DNTTK_CPU_TIMING",false,"FAST USES FAR LESS HOST CPU AT THE SAME GAME SPEED",
         {{"FAST","fast","\"fast\""},{"ACCURATE","accurate","\"accurate\""}},0},
    }};
    s[2]={"MODERNIZED",{
        {"CAMERA",Section::Controls,"camera","DNTTK_CAMERA_MODE",true,"INDEPENDENT: THE MOUSE TURNS THE CAMERA",
         {{"INDEPENDENT","independent","\"independent\""},{"ORIGINAL","original","\"original\""}},0},
        {"VIEW",Section::Controls,"view","DNTTK_CAMERA_VIEW",true,"THE FIRST / THIRD PERSON KEY SWITCHES IN GAME",
         {{"THIRD PERSON","third","\"third\""},{"FIRST PERSON","first","\"first\""}},0},
        {"SHOULDER",Section::Controls,"shoulder","DNTTK_CAMERA_SHOULDER",true,"THE SHOULDER KEY CYCLES IN GAME",
         {{"CENTER","center","\"center\""},{"RIGHT","right","\"right\""},{"LEFT","left","\"left\""}},0},
        {"CAMERA DISTANCE",Section::Controls,"camera_distance","DNTTK_CAMERA_DISTANCE",true,"ALT AND THE MOUSE WHEEL ALSO SET IT IN GAME",
         {{"ORIGINAL","0","0"},{"1024","1024","1024"},{"1536","1536","1536"},{"2048","2048","2048"},
          {"3072","3072","3072"},{"4096","4096","4096"},{"6144","6144","6144"}},0},
        {"WEAPON AIM",Section::Controls,"weapon_aim","DNTTK_WEAPON_AIM",true,"VIEW: SHOTS GO WHERE THE CROSSHAIR POINTS",
         {{"VIEW","view","\"view\""},{"ORIGINAL","original","\"original\""}},0},
        {"CROSSHAIR",Section::Controls,"crosshair","DNTTK_CROSSHAIR",true,"SHOWS OR HIDES THE CROSSHAIR",
         {on_off[0],on_off[1]},0},
        {"AIM ASSIST",Section::Controls,"aim_assist","DNTTK_AIM_ASSIST",true,"THE ORIGINAL TARGET LOCK HELPS THE VIEW AIM",
         {{"OFF","off","\"off\""},{"ORIGINAL LOCK","original-lock","\"original-lock\""}},0},
        {"RED DOT",Section::Controls,"red_dot","DNTTK_RED_DOT",true,"THE ORIGINAL TARGET MARKER",
         {{"OFF","0","false"},{"ON","1","true"}},0},
        {"JUMP",Section::Controls,"jump","DNTTK_JUMP",true,"MANUAL: LEAVE ON THE PRESS AND STEER IN THE AIR",
         {{"ASSISTED","assisted","\"assisted\""},{"MANUAL","manual","\"manual\""}},0},
        {"JUMP WALLS",Section::Controls,"jump_walls","DNTTK_JUMP_WALLS",true,"SLIDE: A WALL ONLY STOPS THE MOTION INTO IT",
         {{"SLIDE","slide","\"slide\""},{"ORIGINAL","original","\"original\""}},0},
        {"LANDING",Section::Controls,"landing","DNTTK_LANDING",true,"MODERN: MOVE AT ONCE ON LANDING",
         {{"MODERN","modern","\"modern\""},{"ORIGINAL","original","\"original\""}},0},
        {"SWIMMING",Section::Controls,"swimming","DNTTK_SWIMMING",true,"MODERN: SHIFT WALKS OR RUNS IN WATER AS ON LAND",
         {{"MODERN","modern","\"modern\""},{"ORIGINAL","original","\"original\""}},0},
        {"JETPACK",Section::Controls,"jetpack","DNTTK_JETPACK",false,"MODERN: CAMERA RELATIVE FLIGHT AND HOVER",
         {{"MODERN","modern","\"modern\""},{"CLASSIC","classic","\"classic\""}},0},
        {"DYNAMITE",Section::Controls,"dynamite","DNTTK_DYNAMITE",true,"MODERN: A DRAWN STICK STAYS UNLIT UNTIL YOU FIRE",
         {{"MODERN","modern","\"modern\""},{"ORIGINAL","original","\"original\""}},0},
        {"STEROIDS",Section::Controls,"steroids","DNTTK_STEROIDS",false,"PORTABLE: A PICKUP IS STORED, R TAKES IT",
         {{"PORTABLE","portable","\"portable\""},{"ORIGINAL","original","\"original\""}},0},
        {"VIEW BOB",Section::Controls,"view_bob","DNTTK_VIEW_BOB",true,"FIRST PERSON HEAD BOB WHILE WALKING",
         {{"OFF","off","\"off\""},{"SUBTLE","subtle","\"subtle\""},{"ON","on","\"on\""},{"STRONG","strong","\"strong\""}},2},
    }};
    return s;
}

struct Item {const char* label;int guest_sub;int screen;};
// OPTIONS: the original three (the handler's own return values), then ours.
const Item options_items[]={{"SOUND",7,-1},{"CONTROLLER",4,-1},{"GAME SETTINGS",11,-1},
                            {"PC CONTROLS",-1,0},{"DISPLAY",-1,1},{"MODERNIZED",-1,2}};
constexpr int option_count=6;

// Recursive: a pad read inside one of our guest calls comes back into
// game_menu_input on this thread; it then leaves the keys for the next read.
std::recursive_mutex mutex;
bool busy=false;
std::vector<Screen> screens;
std::vector<Values> committed, draft;
std::once_flag init_once;
std::deque<std::string> owned_strings; // env values not among the choices (stable addresses)

int screen=-1;            // -1 OPTIONS, else screens[]
int cursor=0, top=0;      // selection and first visible row
bool owned=false;         // sub-screen 8 (or 9) is ours
// Actions for the next live frame (guest writes and sounds happen there).
int pending_sub=-1, pending_cursor=-1, pending_sound=-1;

// D19H: the pause menu and death screen as lists. Page::Options is D19's
// OPTIONS (screen -1) and settings screens; the rest are gameplay pages.
// D19J: Main, Difficulty, TwoPlayer and Character are the title pages.
// D19G: Keys is KEY BINDINGS (from PC CONTROLS).
enum class Page {Options, Pause, Death, Brief, Ask, Main, Difficulty, TwoPlayer, Character, Keys};
enum class Act {Resume, Brief, Options, Load, Restart, MainMenu, Continue, Quit, QuitGame, OnePlayer, TwoPlayer, Level, RestoreKeys};
struct ListItem {const char* label;Act act;};
bool title_page(Page p) {return p==Page::Main || p==Page::Difficulty || p==Page::TwoPlayer || p==Page::Character;}
// D19J: the title pages act in the frontend tick, on the originals' variables,
// in the order the keys came (a page change and the next key in one tick).
enum class Cmd {Up, Down, Left, Right, Confirm, Back};
std::vector<Cmd> title_cmds;
bool title_activity=false;    // a key this tick: the main menu's idle time starts again
bool title_enter=false;       // the main menu opens: state 6's idle time and level reset
// A frontend state for the next tick's entry (LOAD 10, PRESS START 4, or a
// handoff): the image of this tick still shows our page, as the original
// handler draws its screen in the tick that chooses.
int pending_state=-1;
// Starting a game: the original handler (difficulty 8, character 9) runs
// with one Cross press and exits the frontend itself; it draws the same image.
enum class Handoff {None, Pending, Armed};
Handoff handoff=Handoff::None;
Page page=Page::Options;
std::vector<ListItem> list;   // the pause or death list on screen
Page ask_from=Page::Pause;    // a YES / NO prompt: the list it came from,
Act ask_act=Act::Resume;      // what YES does
Act return_item=Act::Resume;  // the item highlighted when the list comes back
// Every final action runs, in the tick, exactly what the original's handler
// does on that confirmation (its sound, flags and calls) and what the main
// loop then does with its result. The handler itself never runs: it would draw
// the old screen into the next image, which then shows for one frame.
enum class Final {None, Resume, PauseRestart, PauseMainMenu, DeathContinue, DeathRestart, DeathQuit};
Final pending_final=Final::None;
bool pending_load=false;      // LOAD: the original LOAD screen (pause 5, death prompt 3)
bool death_load=false;        // the death screen's LOAD is the original's now
// After a resume the screen value stays ours through that dispatch and reads 0
// again at the next composition.
bool resumed=false;
bool quitting=false;          // D19M: the close event is sent; keys are ignored until the exit
const char* back_label="CANCEL";
bool identity_ok=false, identity_checked=false;
uint32_t last_image=~0u;
// D19O: the call site of the last draw. Where a demo scene's load hands over
// (the blocking load to the tick's logo, the logo to the demo), the guest
// builds the same image twice and shows the second pass, which covered the
// menu drawn in the first: about 100 ms without the menu each time. A new
// call site in the same image draws again.
uint32_t last_draw_ra=0;
uint64_t draws=0, entries=0, writes=0, finals=0;
uint32_t sliders=0; // GPU-addressable slider objects
constexpr int slider_slots=12, slider_size=0x40;

// D19J: a savestate load drops what the menu knew of the guest before it; the
// screens reopen from the loaded state (our screens: OPTIONS, the lists, the
// title main menu).
bool key_wait=false; // D19G: KEY BINDINGS waits for the new key (input_capture_key)
uint32_t seen_loads=0;
void savestate_check() {
    const uint32_t n=psx_mod_savestate_loads();
    if(n==seen_loads) return;
    seen_loads=n;
    owned=false;page=Page::Options;screen=-1;pending_sub=pending_cursor=-1;
    pending_final=Final::None;pending_load=false;death_load=false;resumed=false;
    pending_state=-1;handoff=Handoff::None;title_cmds.clear();title_enter=false;
    if(key_wait) {key_wait=false;input_capture_key(false);}
}

const char* intern(const char* text) {owned_strings.emplace_back(text);return owned_strings.back().c_str();}

int choice_from_env(Row& row,const char* value) {
    if(!value) return row.def;
    for(size_t i=0;i<row.choices.size();++i) if(!std::strcmp(row.choices[i].env,value)) return (int)i;
    // A value the list does not offer (an Alt-wheel camera distance): keep it as a choice.
    const char* v=intern(value);
    row.choices.push_back({v,v,v});
    return (int)row.choices.size()-1;
}
const char* presentation_value(const char* key) {
    static std::string text=[]{const char* t=std::getenv("DNTTK_MENU_PRESENTATION");return std::string(t?t:"");}();
    const std::string want=std::string(key)+"=";
    size_t at=0;
    while(at<text.size()) {
        size_t end=text.find(',',at);if(end==std::string::npos) end=text.size();
        if(text.compare(at,want.size(),want)==0) return intern(text.substr(at+want.size(),end-at-want.size()).c_str());
        at=end+1;
    }
    return nullptr;
}
void init() {
    screens=make_screens();
    for(auto& s:screens) {
        Values v;
        for(size_t i=0;i<s.rows.size();++i) {
            Row& r=s.rows[i];
            const char* env=r.link ? nullptr : r.section==Section::Presentation ? presentation_value(r.key) : r.env ? std::getenv(r.env) : nullptr;
            if(r.slider) {
                int value=r.def;
                if(env) {const double d=std::strtod(env,nullptr)/r.scale;value=std::clamp((int)(d+0.5),r.lo,r.hi);}
                v.push_back(value);
            } else v.push_back(choice_from_env(r,env));
        }
        committed.push_back(v);
    }
    draft=committed;
}
void ensure_init() {std::call_once(init_once,init);}

bool authentic() {
    if(!identity_checked) {identity_checked=true;identity_ok=code_identity(guards,expected);
        if(!identity_ok) std::fprintf(stderr,"[TTK menu] executable identity mismatch: original OPTIONS kept\n");}
    return identity_ok;
}
bool enabled() {
    static const bool v=[]{const char* t=std::getenv("DNTTK_GAME_MENU");return !(t && !std::strcmp(t,"off"));}();
    return v && input_modernized();
}

// ---- D19G: key names ----
const char* const action_names[action_count]={
#define TTK_ACTION(name, token, code, pad) #name,
#include "input_bindings.def"
#undef TTK_ACTION
};
struct KeyName {int code;const char* token;const char* shown;};
// The profile's tokens (tools/local/pc_input.py TOKENS) for the codes that are
// not a plain letter or digit, and how the menu shows them (menu font: capitals,
// digits and a little punctuation, so no brackets).
const KeyName key_names[]={
    {0,"Unbound","NONE"},{44,"Space","SPACE"},{224,"LCtrl","LEFT CTRL"},{225,"LShift","SHIFT"},{226,"Alt","ALT"},
    {71,"ScrollLock","SCROLL LOCK"},{47,"LBracket","["},{48,"RBracket","]"},
    {74,"Home","HOME"},{75,"PageUp","PAGE UP"},{77,"End","END"},{78,"PageDown","PAGE DOWN"},
    {51,"Semicolon","SEMICOLON"},{52,"Apostrophe","APOSTROPHE"},{54,"Comma","COMMA"},{55,"Period","PERIOD"},
    {-1,"Mouse1","LEFT MOUSE"},{-3,"Mouse2","RIGHT MOUSE"},{-2,"Mouse3","MIDDLE MOUSE"},{-4,"Mouse4","MOUSE 4"},{-5,"Mouse5","MOUSE 5"},
};
std::string token_of(int code) {
    if(code>=SDL_SCANCODE_A && code<=SDL_SCANCODE_Z) return std::string(1,char('A'+code-SDL_SCANCODE_A));
    if(code>=SDL_SCANCODE_1 && code<=SDL_SCANCODE_9) return std::string(1,char('1'+code-SDL_SCANCODE_1));
    if(code==SDL_SCANCODE_0) return "0";
    for(const KeyName& k:key_names) if(k.code==code) return k.token;
    return "Unbound";
}
std::string key_shown(int code) {
    for(const KeyName& k:key_names) if(k.code==code) return k.shown;
    return token_of(code);
}
// In sentences (the text call has no bracket glyph): the brackets by name.
std::string key_said(int code) {
    if(code==47) return "LEFT BRACKET";
    if(code==48) return "RIGHT BRACKET";
    return key_shown(code);
}

// The confirmed settings, for run.py (DNTTK_MENU_SETTINGS_FILE).
void write_settings() {
    const char* path=std::getenv("DNTTK_MENU_SETTINGS_FILE");
    if(!path || !*path) return;
    // Vanilla is a launch flag (run.py --vanilla, D33); the side file keeps its
    // version 1 "active" field, always Modernized.
    std::string controls, presentation, bindings;
    for(size_t s=0;s<screens.size();++s) for(size_t i=0;i<screens[s].rows.size();++i) {
        const Row& r=screens[s].rows[i];const int v=committed[s][i];
        if(r.link) continue;
        char value[64];
        if(r.slider) std::snprintf(value,sizeof value,"%.2f",v*r.scale);
        else std::snprintf(value,sizeof value,"%s",r.choices[v].json);
        std::string& out=r.section==Section::Controls ? controls : presentation;
        out+=std::string(out.empty()?"":",")+"\""+r.key+"\":"+value;
    }
    // D19G: the bindings in the profile's terms (run.py validates them).
    for(int a=0;a<action_count;++a)
        bindings+=std::string(a?",":"")+"\""+action_names[a]+"\":\""+token_of(input_binding_code(Action(a)))+"\"";
    const std::string text="{\"version\":1,\"active\":\"modernized\",\"controls\":{"+controls+"},\"presentation\":{"+presentation+"},\"bindings\":{"+bindings+"}}\n";
    const std::string temp=std::string(path)+".tmp";
    if(FILE* f=std::fopen(temp.c_str(),"wb")) {
        const bool ok=std::fwrite(text.data(),1,text.size(),f)==text.size();
        if(std::fclose(f)==0 && ok && std::rename(temp.c_str(),path)==0) {++writes;return;}
    }
    std::remove(temp.c_str());
    std::fprintf(stderr,"[TTK menu] could not write %s\n",path);
}

// ---- input (from input_pad) ----
struct Repeat {bool held=false;uint64_t next=0,since=0;};
Repeat rep_up,rep_down,rep_left,rep_right;
// A press, then 350 ms, then every 80 ms while held (D19B: predictable repeat).
// A press (a key-down event since the last read, so a tap shorter than the
// gap between two reads still counts and a key held when the screen opened
// does not), then 350 ms, then every 80 ms while held (D19B: predictable repeat).
int repeat(Repeat& r,bool held,int taps,uint64_t now) {
    if(taps>0) {r.held=held;r.since=now;r.next=now+350;return taps;}
    if(!held || !r.held) {r.held=false;return 0;}
    if(now<r.next) return 0;
    // D19O: after a gap with no read the steps go on from now; replaying the
    // missed ones ran the cursor through the list in a burst.
    r.next=now>r.next+80 ? now+80 : r.next+80;
    return 1;
}
// More than five rows: the compact layout (14 apart), nine at a time.
bool compact(const Screen& s) {return s.rows.size()>5;}
int rows_visible(const Screen& s) {return compact(s) ? 9 : (int)s.rows.size();}
void sound(int which) {if(pending_sound<0 || which==0) pending_sound=which;}
// ---- D19H: the pause and death lists ----
int level() {return (int)psx_mod_read_word(level_index);}
bool challenge() {return (unsigned)(level()-21)<6u;} // stages 21-26: no RESTART, REPLAY / GIVE UP
bool has_briefing() {
    const int n=level();
    if(n<0 || n>=13) return false;
    for(int i=0;i<10;++i) if(psx_mod_read_half(briefings+20*n+2*i)) return true;
    return false;
}
// The original's choices, in the original's terms: the pause menu's RESUME,
// OPTIONS, LOAD (one player only) and its QUIT prompt's RESTART (not in the
// challenge stages) and MAIN MENU; the death screen's CONTINUE, LOAD, RESTART
// and QUIT, or REPLAY LEVEL and GIVE UP in the challenge stages.
void build_list(Page p) {
    list.clear();
    if(p==Page::Main) {
        // D19J: the original main menu's items (its fifth, the level, only
        // while the original's debug level choice is on: 0x800d2528 = 5).
        list.push_back({"ONE PLAYER",Act::OnePlayer});
        list.push_back({"TWO PLAYER",Act::TwoPlayer});
        list.push_back({"LOAD GAME",Act::Load});
        list.push_back({"OPTIONS",Act::Options});
        if((int16_t)psx_mod_read_half(fe_items)>4) list.push_back({"LEVEL",Act::Level});
    } else if(p==Page::Pause) {
        list.push_back({"RESUME",Act::Resume});
        if(has_briefing()) list.push_back({"MISSION BRIEFING",Act::Brief});
        list.push_back({"OPTIONS",Act::Options});
        if(psx_mod_read_word(players)==1) list.push_back({"LOAD GAME",Act::Load});
        if(!challenge()) list.push_back({"RESTART LEVEL",Act::Restart});
        list.push_back({"MAIN MENU",Act::MainMenu});
    } else if(challenge()) {
        list.push_back({"REPLAY LEVEL",Act::Continue});
        list.push_back({"GIVE UP",Act::Quit});
    } else {
        list.push_back({"CONTINUE",Act::Continue});
        list.push_back({"LOAD GAME",Act::Load});
        list.push_back({"RESTART LEVEL",Act::Restart});
        list.push_back({"QUIT TO MENU",Act::Quit}); // D19P: QUIT, beside QUIT GAME
    }
    list.push_back({"QUIT GAME",Act::QuitGame}); // D19M: closes the program
}
int item_of(Act a) {
    for(size_t i=0;i<list.size();++i) if(list[i].act==a) return (int)i;
    return 0;
}
void open_list(Page p,Act highlight) {
    page=p;build_list(p);cursor=item_of(highlight);top=0;screen=-1;
}
// ---- D19G: KEY BINDINGS ----
// Every action, grouped; a row without an action is a section heading (the
// cursor skips it). Changes apply and are saved at once; DEFAULT asks first.
// fixed: a key that always does it too (not rebindable), shown first.
// hint: one line about the action, above the CONFIRM line.
struct KeyRow {const char* label;int action;const char* fixed=nullptr;const char* hint=nullptr;};
const KeyRow key_rows[]={
    {"MOVEMENT",-1},{"MOVE FORWARD",move_forward},{"MOVE BACK",move_back},{"STRAFE LEFT",move_left},{"STRAFE RIGHT",move_right},
    {"JUMP",jump},{"CROUCH",crouch},{"WALK / RUN",walk},{"USE / MANTLE",interact},
    {"GRAB",grab,nullptr,"HOLD TO GRAB, PUSH AND PULL"},{"GRAB 2",grab_alt,nullptr,"A SECOND GRAB KEY"},
    // The weapon keys select a group; each is named after its first weapon.
    {"WEAPONS",-1},{"FIRE",fire},{"QUICK KICK",quick_kick},{"HOLSTER",holster},{"PREVIOUS WEAPON",weapon_previous},
    {"NEXT WEAPON",weapon_next},{"LAST WEAPON",weapon_last},
    {"MIGHTY BOOT",weapon_group_1,nullptr,"ALSO THROWING KNIFE AND AXE"},{"DESERT EAGLE",weapon_group_2},
    {"COMBAT SHOTGUN",weapon_group_3},{"GATLING GUN",weapon_group_4},{"RPG",weapon_group_5},
    {"PIPE BOMB",weapon_group_6,nullptr,"ALSO DYNAMITE AND HOLY HAND GRENADE"},{"ENERGY WEAPON",weapon_group_7},
    {"FLAMETHROWER",weapon_group_8},{"BUFFALO RIFLE",weapon_group_9,nullptr,"ALSO THE CROSSBOW"},{"FREEZER",weapon_group_0},
    // inventory's key toggles the crosshair (the inventory screen is Right Shift).
    {"ITEMS",-1},{"CROSSHAIR",inventory,nullptr,"SHOWS OR HIDES THE CROSSHAIR"},{"PREVIOUS ITEM",item_previous},{"NEXT ITEM",item_next},
    {"USE ITEM",item_use,"ENTER","ENTER ALWAYS USES IT TOO"},
    {"MEDKIT",medkit},{"STEROIDS",steroids},{"JETPACK",jetpack},{"GOGGLES",goggles},{"BIO MASK",biomask},
    {"PREVIOUS MISSION ITEM",mission_previous},{"NEXT MISSION ITEM",mission_next},
    {"CAMERA",-1},{"FIRST / THIRD PERSON",camera_view},{"SHOULDER",camera_shoulder},{"RECENTER CAMERA",camera_recenter},
    {"LOOK UP",look_up,nullptr,"HOLD, EASES BACK ON RELEASE"},{"LOOK DOWN",look_down,nullptr,"HOLD, EASES BACK ON RELEASE"},
    {"LOCK LOOK UP",lock_look_up,nullptr,"HOLD, STAYS UNTIL A LOOK KEY"},{"LOCK LOOK DOWN",lock_look_down,nullptr,"HOLD, STAYS UNTIL A LOOK KEY"},
    {"ORIGINAL CONTROLS",-1},{"ORIGINAL AIM",original_aim},{"ORIGINAL STRAFE LEFT",original_strafe_left},
    {"ORIGINAL STRAFE RIGHT",original_strafe_right},
};
constexpr int key_row_count=sizeof key_rows/sizeof key_rows[0], key_rows_visible=9;
static_assert(key_row_count==action_count+5,"every action has one KEY BINDINGS row");
std::string key_note;         // the description's message after a change
int keys_cursor=1, keys_top=0; // kept while DEFAULT asks
const char* action_label(int a) {
    for(const KeyRow& k:key_rows) if(k.action==a) return k.label;
    return "";
}
void key_wait_end() {if(key_wait) {key_wait=false;input_capture_key(false);}}
// The cursor's row visible, with its section heading when that is just above.
void keys_scroll() {
    if(cursor<top) top=cursor;
    if(cursor>=top+key_rows_visible) top=cursor-key_rows_visible+1;
    if(top==cursor && top>0 && key_rows[top-1].action<0) --top;
}
void open_keys() {page=Page::Keys;cursor=1;top=0;key_note.clear();key_wait_end();sound(0);}
void keys_move(int dir) {
    do cursor=(cursor+dir+key_row_count)%key_row_count; while(key_rows[cursor].action<0);
    keys_scroll();key_note.clear();sound(1);
}
void keys_back() {
    key_wait_end();key_note.clear();
    page=Page::Options;screen=0;top=0;cursor=0;
    for(size_t i=0;i<screens[0].rows.size();++i) if(screens[0].rows[i].link) cursor=(int)i;
    sound(0);
}
void keys_ask_default() {
    keys_cursor=cursor;keys_top=top;key_note.clear();
    ask_from=Page::Keys;ask_act=Act::RestoreKeys;page=Page::Ask;cursor=1;sound(1);
}
void keys_return() {page=Page::Keys;cursor=keys_cursor;top=keys_top;}
void restore_keys() {
    int codes[action_count];
    for(int a=0;a<action_count;++a) codes[a]=input_default_code(Action(a));
    input_set_binding_codes(codes);
    write_settings();
    keys_return();key_note="DEFAULT KEYS RESTORED";sound(0);
}
// The key pressed while waiting: Escape cancels, Backspace clears; a key
// another action has swaps with it (that action takes this one's old key).
void key_pressed(int code) {
    const Action a=Action(key_rows[cursor].action);
    if(code==SDL_SCANCODE_ESCAPE) {key_wait_end();sound(0);return;}
    if(code==SDL_SCANCODE_BACKSPACE) code=0;
    else if(code==SDL_SCANCODE_RSHIFT) code=SDL_SCANCODE_LSHIFT;  // either Shift, as in play
    else if(code==SDL_SCANCODE_RALT) code=SDL_SCANCODE_LALT;      // either Alt
    if(code && !input_code_bindable(code,a)) {
        key_note=code==SDL_SCANCODE_C ? "C IS THE HOLSTER KEY ONLY. PRESS ANOTHER" : "THAT KEY IS RESERVED. PRESS ANOTHER";
        sound(1);return;
    }
    int codes[action_count];
    for(int i=0;i<action_count;++i) codes[i]=input_binding_code(Action(i));
    const int old=codes[a];
    key_note.clear();
    if(code!=old) {
        for(int b=0;code && b<action_count;++b) if(b!=a && codes[b]==code) {
            codes[b]=old==SDL_SCANCODE_C && b!=holster ? 0 : old; // C stays the holster's
            key_note=key_said(code)+" WAS "+action_label(b)+": "+(codes[b] ? "THAT IS ON "+key_said(codes[b])+" NOW" : "THAT HAS NO KEY NOW");
        }
        codes[a]=code;
        input_set_binding_codes(codes);
        write_settings();
    }
    key_wait_end();sound(0);
}
void ask_cancel() {
    if(ask_from==Page::Keys) keys_return();
    else open_list(ask_from,ask_act);
    sound(1);
}

void act_now(Act a) {
    const bool death=page==Page::Death || (page==Page::Ask && ask_from==Page::Death);
    switch(a) {
    case Act::Resume: pending_final=Final::Resume;break;
    case Act::Restart: pending_final=death ? Final::DeathRestart : Final::PauseRestart;break;
    case Act::MainMenu: pending_final=Final::PauseMainMenu;break;
    case Act::Continue: pending_final=Final::DeathContinue;break;
    case Act::Quit: pending_final=Final::DeathQuit;break;
    case Act::RestoreKeys: restore_keys();break;
    case Act::QuitGame: {
        // The window's close event: the main loop's own shutdown (settings
        // were written on CONFIRM; run.py folds them in when the game exits).
        quitting=true;sound(0);
        SDL_Event quit{};quit.type=SDL_QUIT;
        SDL_PushEvent(&quit);
        break;
    }
    default: break;
    }
}
// The original's prompt for each choice: CONTINUE (REPLAY) is confirmed as
// the original asks (YES first); RESTART, MAIN MENU, QUIT and QUIT GAME
// start on NO. QUIT GAME? and QUIT TO MENU? have no text id: -1 and -2.
int ask_heading() {
    switch(ask_act) {
    case Act::Continue: return challenge() ? text_replay : text_continue;
    case Act::Restart: return text_restart;
    case Act::QuitGame: return -1;
    case Act::Quit: return challenge() ? text_quit : -2; // D19P: GIVE UP keeps REALLY QUIT?
    case Act::RestoreKeys: return -3;
    default: return text_quit;
    }
}
void list_confirm() {
    const Act a=list[cursor].act;
    return_item=a;
    switch(a) {
    case Act::Brief: page=Page::Brief;sound(0);break;
    case Act::Options: page=Page::Options;screen=-1;cursor=0;top=0;back_label=contexts[1].back_label;sound(0);break;
    case Act::Load: pending_load=true;sound(0);break;
    case Act::Resume: act_now(a);break;
    default: ask_from=page;ask_act=a;page=Page::Ask;cursor=a==Act::Continue ? 0 : 1;sound(1);break;
    }
}

void move(int dir) {
    if(title_page(page)) {title_cmds.push_back(dir<0 ? Cmd::Up : Cmd::Down);return;}
    if(page==Page::Keys) {if(!key_wait) keys_move(dir);return;}
    if(page==Page::Brief) return;
    if(page!=Page::Options) {
        const int n=page==Page::Ask ? 2 : (int)list.size();
        if(n) cursor=(cursor+dir+n)%n;
        sound(1);
        return;
    }
    const int n=screen<0 ? option_count : (int)screens[screen].rows.size();
    cursor=(cursor+dir+n)%n;
    if(screen>=0) {
        const int vis=rows_visible(screens[screen]);
        if(cursor<top) top=cursor;
        if(cursor>=top+vis) top=cursor-vis+1;
    }
    sound(1);
}
void change(int dir,uint64_t held_ms) {
    if(title_page(page)) {title_cmds.push_back(dir<0 ? Cmd::Left : Cmd::Right);return;}
    if(page!=Page::Options || screen<0) return;
    Row& r=screens[screen].rows[cursor];
    int& v=draft[screen][cursor];
    const int before=v;
    if(r.slider) v=std::clamp(v+dir*r.step*(held_ms>1200?5:1),r.lo,r.hi);
    else {const int n=(int)r.choices.size();v=(v+dir+n)%n;}
    if(v!=before) sound(1);
}
// D19G: rows that apply when confirmed. FRAME RATE only while replay
// presentation runs (not after a 60 launch).
bool row_live(const Row& r) {
    if(!r.live) return false;
    if(r.key && !std::strcmp(r.key,"frame_rate")) return frame_rate_live();
    return true;
}
int row_of(int s,const char* key) {
    for(size_t i=0;i<screens[s].rows.size();++i) if(screens[s].rows[i].key && !std::strcmp(screens[s].rows[i].key,key)) return (int)i;
    return -1;
}
// A confirmed value, now. Settings read through ttk_live_env() take the new
// value; view, shoulder, distance and the display go to their owners.
void apply_live(const Row& r,int v) {
    const char* value;
    if(r.slider) {char t[32];std::snprintf(t,sizeof t,"%.2f",v*r.scale);value=intern(t);}
    else value=r.choices[v].env;
    const std::string key=r.key;
    if(key=="frame_rate" && !frame_rate_set(value)) return;
    // Until a change ttk_live_env() is plain getenv (an unset variable stays unset).
    if(r.env) ttk_live_env_set(r.env,value);
    if(key=="view") input_set_view(!std::strcmp(value,"first"));
    else if(key=="shoulder") input_set_shoulder(!std::strcmp(value,"right") ? 1 : !std::strcmp(value,"left") ? -1 : 0);
    else if(key=="camera_distance") camera_set_distance(std::atof(value));
    else if(key=="display") psx_mod_request_fullscreen_mode(!std::strcmp(value,"exclusive") ? 2 : !std::strcmp(value,"borderless") ? 1 : 0);
    else if(key=="widescreen") {
        unsigned num=4,den=3;
        if(!std::strcmp(value,"auto")) psx_mod_request_display_aspect(21,9,1);
        else {if(std::strcmp(value,"off")) std::sscanf(value,"%u:%u",&num,&den);psx_mod_request_display_aspect(num,den,0);}
    }
}
// P, H and Alt + wheel change view, shoulder and distance in game: the
// MODERNIZED screen opens on what is live now.
void refresh_from_game(int s) {
    auto set=[&](const char* key,const char* value) {
        const int i=row_of(s,key);
        if(i>=0) committed[s][i]=choice_from_env(screens[s].rows[i],value);
    };
    set("view",input_first_person() ? "first" : "third");
    const int side=input_shoulder();
    set("shoulder",side>0 ? "right" : side<0 ? "left" : "center");
    char d[32];std::snprintf(d,sizeof d,"%.0f",camera_distance_setting());
    if(row_of(s,"camera_distance")>=0) {
        Row& r=screens[s].rows[row_of(s,"camera_distance")];
        const char* match=nullptr;
        for(const Choice& c:r.choices) if(!std::strcmp(c.env,d)) match=c.env;
        set("camera_distance",match ? match : intern(d));
    }
}
void open_screen(int s) {
    refresh_from_game(s);
    screen=s;cursor=0;top=0;draft[s]=committed[s];sound(0);
}
void leave_screen(bool keep) {
    const int s=screen;
    if(keep && draft[s]!=committed[s]) {
        for(size_t i=0;i<screens[s].rows.size();++i) {
            const Row& r=screens[s].rows[i];
            if(draft[s][i]!=committed[s][i] && !r.link && row_live(r)) apply_live(r,draft[s][i]);
        }
        committed[s]=draft[s];
        write_settings();
    }
    draft[s]=committed[s];
    screen=-1;cursor=3+s;top=0;sound(0);
}
void defaults() {
    if(page==Page::Keys) {if(!key_wait) keys_ask_default();return;}
    if(page!=Page::Options || screen<0) return;
    for(size_t i=0;i<screens[screen].rows.size();++i) draft[screen][i]=screens[screen].rows[i].def;
    sound(0);
}
void confirm() {
    switch(page) {
    case Page::Pause: case Page::Death: list_confirm();return;
    case Page::Brief: return;
    case Page::Ask:
        if(cursor==0) act_now(ask_act);
        else ask_cancel();
        return;
    case Page::Keys: if(!key_wait) {key_wait=true;key_note.clear();input_capture_key(true);sound(0);} return;
    case Page::Options: break;
    default: title_cmds.push_back(Cmd::Confirm);return; // D19J: the title pages
    }
    if(screen>=0 && screens[screen].rows[cursor].link) {open_keys();return;}
    if(screen>=0) {leave_screen(true);return;}
    const Item& it=options_items[cursor];
    if(it.guest_sub>=0) {pending_cursor=cursor;pending_sub=it.guest_sub;sound(0);}
    else open_screen(it.screen);
}
void back(uint16_t mode) {
    switch(page) {
    case Page::Pause: act_now(Act::Resume);return;  // Escape resumes, as the original's Start
    case Page::Death: return;                        // nothing to go back to
    case Page::Brief: open_list(Page::Pause,Act::Brief);sound(0);return;
    case Page::Ask: ask_cancel();return;
    case Page::Keys: keys_back();return;
    case Page::Options: break;
    default: title_cmds.push_back(Cmd::Back);return; // D19J: the title pages
    }
    if(screen>=0) {leave_screen(false);return;}
    // In gameplay OPTIONS goes back to our pause list (the original's 12 would
    // draw its own pause menu for a frame first).
    if(mode==1) {open_list(Page::Pause,Act::Options);sound(0);return;}
    // D19J: on the title to our main menu, OPTIONS highlighted (the original's
    // 10 is a tick with no menu, then state 6). The OPTIONS cursor resets.
    open_list(Page::Main,Act::Options);title_enter=true;
    pending_cursor=0;pending_sub=title_sub;sound(0);
}
} // namespace

uint64_t drops=0; int drop_why=0; uint16_t drop_state=0, drop_sub=0;   // diagnostic
bool game_menu_input(const MenuKeys& keys) {
    if(!enabled()) return false;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    savestate_check();
    // Taps that arrive while the menu cannot act yet wait for the next read.
    static MenuKeys carry;
    MenuKeys k=keys;
    k.up_taps+=carry.up_taps;k.down_taps+=carry.down_taps;k.left_taps+=carry.left_taps;k.right_taps+=carry.right_taps;
    k.confirm_taps+=carry.confirm_taps;k.back_taps+=carry.back_taps;k.square_taps+=carry.square_taps;
    carry=MenuKeys{};
    // Busy drawing, or a screen change waits for the next tick: keys wait too.
    if(quitting) return owned;
    // D19J: starting a game; no key reaches the original handler meanwhile.
    if(handoff!=Handoff::None) {
        if(psx_mod_read_half(game_mode)==0) return true;
        handoff=Handoff::None;
    }
    if(busy || (owned && (pending_sub>=0 || pending_final!=Final::None || pending_load || pending_state>=0))) {
        if(owned) carry=k;
        return owned;
    }
    const MenuContext* c=current_context();
    if(!owned || !c || !our_screen(c,psx_mod_read_half(c->sub))) {
        if(rep_up.held || rep_down.held || rep_left.held || rep_right.held || k.up || k.down) {
            ++drops; drop_why=!owned?1:!c?2:3; drop_state=psx_mod_read_half(fe_state); drop_sub=c?psx_mod_read_half(c->sub):0xffff;
        }
        rep_up=rep_down=rep_left=rep_right=Repeat{};
        key_wait_end();
        return false;
    }
    // D19G: waiting for a new key, only that key counts.
    if(key_wait) {
        if(page!=Page::Keys) key_wait_end();
        else {
            const int code=input_take_captured_key();
            if(code!=input_no_key()) key_pressed(code);
            return true;
        }
    }
    const uint64_t now=SDL_GetTicks();
    for(int n=repeat(rep_up,k.up,k.up_taps,now);n>0;--n) move(-1);
    for(int n=repeat(rep_down,k.down,k.down_taps,now);n>0;--n) move(1);
    for(int n=repeat(rep_left,k.left,k.left_taps,now);n>0;--n) change(-1,now-rep_left.since);
    for(int n=repeat(rep_right,k.right,k.right_taps,now);n>0;--n) change(1,now-rep_right.since);
    for(int w=k.wheel;w>0;--w) move(-1);
    for(int w=k.wheel;w<0;++w) move(1);
    if(k.confirm_taps) confirm();
    else if(k.back_taps) back(c->mode);
    else if(k.square_taps) defaults();
    // D19J: any key keeps the main menu from going back to PRESS START.
    if(k.up || k.down || k.left || k.right || k.confirm || k.back || k.square || k.wheel || k.up_taps || k.down_taps
       || k.left_taps || k.right_taps || k.confirm_taps || k.back_taps || k.square_taps) title_activity=true;
    return true;
}

namespace {
// ---- drawing: the original's own calls, on one saved stack and scratchpad ----
struct Guest {
    CPUState* source;uint32_t top,text,base;bool ok=true;
    uint32_t saved[1024],scratch[256];
    explicit Guest(CPUState* cpu):source(cpu),top(cpu->gpr[29]),text(top-0x80),base((top-4096)&~3u) {
        for(unsigned i=0;i<1024;++i) saved[i]=psx_mod_read_word(base+4*i);
        for(unsigned i=0;i<256;++i) scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    }
    ~Guest() {
        for(unsigned i=0;i<1024;++i) if(psx_mod_read_word(base+4*i)!=saved[i]) psx_mod_write_word(base+4*i,saved[i]);
        for(unsigned i=0;i<256;++i) if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i]) psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    }
    uint32_t call(uint32_t address,uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t s16=0,uint32_t s20=0,uint32_t s24=0) {
        if(!ok) return 0;
        CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=sentinel;
        cpu.gpr[4]=a0;cpu.gpr[5]=a1;cpu.gpr[6]=a2;cpu.gpr[7]=a3;
        psx_mod_write_word(top-0x100+16,s16);psx_mod_write_word(top-0x100+20,s20);psx_mod_write_word(top-0x100+24,s24);
        psx_mod_host_call_begin(); // D19J: no savestate inside our call
        psx_dispatch_call(&cpu,address,sentinel);
        psx_mod_host_call_end();
        ok=cpu.pc==0 && !g_psx_call_bail;
        return cpu.gpr[2];
    }
    void put(const char* s) {
        unsigned i=0;
        for(;s[i] && i<100;++i) psx_mod_write_byte(text+i,(uint8_t)s[i]);
        psx_mod_write_byte(text+i,0);
    }
    void say(const char* s,int x,int y,uint32_t flags) {put(s);call(fn_text,text,(uint32_t)x,(uint32_t)y,flags);}
    // A string of the game's text table, by id, as the original passes it.
    void say_id(int id,int x,int y,uint32_t flags) {
        const uint32_t s=psx_mod_read_word(strings+4u*(uint32_t)id);
        if(s>=0x80010000u && s<0x80200000u) call(fn_text,s,(uint32_t)x,(uint32_t)y,flags);
    }
    int width(const char* s,uint32_t font) {put(s);return (int)call(fn_width,text,font>>8,0,0);}
    // D19G: text in another CLUT row of the font's column (the call offers
    // gold, blue and red): shift the CLUT of the glyph packets it just added.
    void say_clut(const char* s,int x,int y,uint32_t flags,int clut_delta) {
        const uint32_t before=psx_mod_read_word(glyph_next);
        say(s,x,y,flags);
        const uint32_t after=psx_mod_read_word(glyph_next);
        if(after<=before || after-before>glyph_size*128) return; // the buffer wrapped: leave it gold
        for(uint32_t p=before;p<after;p+=glyph_size) psx_mod_write_half(p+14,(uint16_t)(psx_mod_read_half(p+14)+clut_delta));
    }
    // D19J: the prompt helper itself, with a text id, as the title pages call it.
    void prompt_id(Icon icon,int x,int y,int id) {call(fn_prompt,(uint32_t)icon,(uint32_t)x,(uint32_t)y,(uint32_t)id);}
    // The prompt helper 0x8008411c with our own label: button left of the
    // centred small-font label.
    void prompt(Icon icon,const char* label,int x,int y) {
        const int w=width(label,font_small);
        call(fn_sprite,sprites+16*icon,(uint32_t)(x-w/2-26),(uint32_t)(y-3),0,100);
        say(label,x,y,font_small|centre);
    }
};
struct Geometry {int x0,y0,w,h;};
Geometry read_geometry() {
    return {(int16_t)psx_mod_read_half(geometry),(int16_t)psx_mod_read_half(geometry+2),
            (int16_t)psx_mod_read_half(geometry+4),(int16_t)psx_mod_read_half(geometry+6)};
}
// Prompt row at the original's y. OPTIONS: its three, w/4 apart. Settings:
// GAME SETTINGS' four (SELECT -3w/8, CHANGE 5 - w/8, CONFIRM w/8 - 5, CANCEL
// 3w/8), and DEFAULT on the heading line in the CANCEL column.
void prompts(Guest& g,const Geometry& geo,bool settings) {
    const int y=-geo.y0-(geo.h>>4)-19;
    if(!settings) {
        g.prompt(icon_select,"SELECT",-geo.w/4,y);
        g.prompt(icon_cross,"CONFIRM",0,y);
        g.prompt(icon_triangle,back_label,geo.w/4,y);
        return;
    }
    const int eighth=geo.w/8;
    g.prompt(icon_select,"SELECT",-3*eighth,y);
    g.prompt(icon_change,"CHANGE",5-eighth,y);
    g.prompt(icon_cross,"CONFIRM",eighth-5,y);
    g.prompt(icon_triangle,"CANCEL",3*eighth,y);
    g.prompt(icon_square,"DEFAULT",3*eighth,geo.y0+(171*geo.h>>11)+3);
}
void draw_options(Guest& g,const Geometry& geo) {
    // The handler's 19-line spacing, centred where its three were.
    const int y=(geo.h-57)/2+geo.y0-19*(option_count-3)/2;
    for(int i=0;i<option_count;++i)
        g.say(options_items[i].label,0,y+19*i,font_menu|centre|(i==cursor?blue:0));
    prompts(g,geo,false);
}
void wrap(Guest& g,const char* text,int max,std::vector<std::string>& lines) {
    std::string line,word;
    auto flush=[&]{
        const std::string t=line.empty()?word:line+" "+word;
        if(!line.empty() && g.width(t.c_str(),font_small)>max) {lines.push_back(line);line=word;}
        else line=t;
        word.clear();
    };
    for(const char* p=text;;++p) {
        if(*p==' ' || !*p) {if(!word.empty()) flush();if(!*p) break;}
        else word+=*p;
    }
    if(!line.empty()) lines.push_back(line);
}
void draw_settings(Guest& g,const Geometry& geo) {
    const Screen& s=screens[screen];
    const Values& v=draft[screen];
    // GAME SETTINGS: heading y0 + 171h/2048, rows from y0/2 + 10, 20 apart,
    // label right-aligned at -10, value at +10; slider label right-aligned at
    // x0 + w/3, its bar 20 further on.
    g.say(s.title,0,geo.y0+(171*geo.h>>11),font_menu|centre);
    const int n=(int)s.rows.size(), vis=rows_visible(s);
    const bool tight=compact(s);
    const int spacing=tight?14:20, y0=tight?geo.y0/2-14:geo.y0/2+10;
    int slot=0;
    for(int j=0;j<vis && top+j<n;++j) {
        const int i=top+j, y=y0+j*spacing;
        const Row& r=s.rows[i];
        const uint32_t hi=i==cursor?blue:0;
        if(r.slider && sliders && slot<slider_slots) {
            const uint32_t obj=sliders+slider_size*slot++;
            g.say(r.label,geo.x0+geo.w/3,y,font_small|right|hi);
            g.call(fn_slider_init,obj,0,(uint32_t)(y+5),(uint32_t)r.lo,(uint32_t)r.hi,(uint32_t)v[i],(uint32_t)r.step);
            g.call(fn_slider_draw,obj,hi,0,0);
        } else if(r.slider) {
            char value[16];std::snprintf(value,sizeof value,"%.2f",v[i]*r.scale);
            g.say((std::string(r.label)+":").c_str(),-10,y,font_small|right|hi);
            g.say(value,10,y,font_small);
        } else {
            g.say((std::string(r.label)+":").c_str(),-10,y,font_small|right|hi);
            g.say(r.choices[v[i]].label,10,y,font_small);
        }
    }
    if(tight) {
        // Scroll marks: the SELECT button beside the list while rows are hidden.
        if(top>0 || top+vis<n) g.call(fn_sprite,sprites+16*icon_select,(uint32_t)(geo.w/2-geo.w/8),(uint32_t)(y0+spacing*vis/2-8),0,100);
    }
    const Row& r=s.rows[cursor];
    std::string desc=r.desc;
    if(!r.live) desc+=". NEXT LAUNCH";
    std::vector<std::string> lines;
    wrap(g,desc.c_str(),geo.w*3/4,lines);
    const int ybase=-geo.y0-(geo.h>>4)-19-14-12*((int)lines.size()-1);
    for(size_t l=0;l<lines.size() && l<2;++l) g.say(lines[l].c_str(),0,ybase+12*(int)l,font_small|centre);
    prompts(g,geo,true);
}

// D19G: a key name; [ and ] (the menu font has no brackets) are built from
// its own glyphs, as D24C built '!' from I and a period: "[" is L (stem and
// bottom bar in one glyph, no seam) with a hyphen as its top bar; "]" is I
// with hyphen bars, the stem queued first so the top bar does not cut it.
void say_key(Guest& g,const std::string& key,int x,int y,uint32_t flags) {
    if(key=="[") {g.say("L",x,y,flags);g.say("-",x+3,y-5,flags);return;}
    if(key=="]") {g.say("I",x+6,y,flags);g.say("-",x+4,y-5,flags);g.say("-",x,y+3,flags);return;}
    g.say(key.c_str(),x,y,flags);
}
// D19G: KEY BINDINGS, laid out as the compact settings screens.
void draw_keys(Guest& g,const Geometry& geo) {
    const int hy=geo.y0+(171*geo.h>>11);
    g.say("KEY BINDINGS",0,hy,font_menu|centre);
    const int spacing=14, y0=geo.y0/2-14;
    for(int j=0;j<key_rows_visible && top+j<key_row_count;++j) {
        const int i=top+j, y=y0+j*spacing;
        const KeyRow& k=key_rows[i];
        if(k.action<0) {g.say_clut(k.label,0,y,font_small|centre,clut_amber);continue;}
        g.say((std::string(k.label)+":").c_str(),-10,y,font_small|right|(i==cursor?blue:0));
        if(i==cursor && key_wait) g.say("PRESS A KEY",10,y,font_small|blue);
        else {
            const int code=input_binding_code(Action(k.action));
            int x=10;
            if(k.fixed) {
                g.say(code ? (std::string(k.fixed)+" /").c_str() : k.fixed,x,y,font_small);
                x+=g.width((std::string(k.fixed)+" / ").c_str(),font_small);
            }
            if(code || !k.fixed) say_key(g,key_shown(code),x,y,font_small);
        }
    }
    if(top>0 || top+key_rows_visible<key_row_count)
        g.call(fn_sprite,sprites+16*icon_select,(uint32_t)(geo.w/2-geo.w/8),(uint32_t)(y0+spacing*key_rows_visible/2-8),0,100);
    std::string desc=key_note;
    std::vector<std::string> lines;
    if(!desc.empty()) wrap(g,desc.c_str(),geo.w*3/4,lines);
    else if(key_wait) lines={"PRESS A KEY OR MOUSE BUTTON.","ESCAPE CANCELS, BACKSPACE CLEARS"};
    else if(key_rows[cursor].hint) lines={key_rows[cursor].hint,"CONFIRM, THEN PRESS THE NEW KEY"};
    else wrap(g,(std::string(key_rows[cursor].label)+". CONFIRM, THEN PRESS THE NEW KEY").c_str(),geo.w*3/4,lines);
    const int py=-geo.y0-(geo.h>>4)-19;
    const int ybase=py-14-12*((int)std::min<size_t>(lines.size(),2)-1);
    for(size_t l=0;l<lines.size() && l<2;++l) g.say(lines[l].c_str(),0,ybase+12*(int)l,font_small|centre);
    if(key_wait) {g.prompt(icon_triangle,"CANCEL",0,py);return;}
    g.prompt(icon_select,"SELECT",-geo.w/4,py);
    g.prompt(icon_cross,"CHANGE",0,py);
    g.prompt(icon_triangle,"RETURN",geo.w/4,py);
    g.prompt(icon_square,"DEFAULT",3*(geo.w/8),hy+3);
}

// ---- D19H pages ----
// The original's text height: headings at y0 + 171h/2048 in the menu font.
int heading_y(const Geometry& geo) {return geo.y0+(171*geo.h>>11);}
// A centred list in the menu font, 19 apart, centred where OPTIONS' three were
// (the original's own arithmetic), plus a shift.
void draw_list(Guest& g,const Geometry& geo,int shift) {
    const int n=(int)list.size(), y=(geo.h-57)/2+geo.y0-19*(n-3)/2+shift;
    for(int i=0;i<n;++i) g.say(list[i].label,0,y+19*i,font_menu|centre|(i==cursor?blue:0));
}
void level_title(Guest& g,const Geometry& geo) {
    const int n=level();
    if(n<0 || n>=32) return;
    const int id=(int16_t)psx_mod_read_half(level_names+2*n);
    if(id>0 && id<=text_last) g.say_id(id,0,heading_y(geo),font_menu|centre);
}
int prompt_y(const Geometry& geo) {return -geo.y0-(geo.h>>4)-19;}
void draw_pause(Guest& g,const Geometry& geo) {
    level_title(g,geo);
    draw_list(g,geo,0);
    const int y=prompt_y(geo);
    g.prompt(icon_select,"SELECT",-geo.w/4,y);
    g.prompt(icon_cross,"CONFIRM",0,y);
    g.prompt(icon_triangle,"RESUME",geo.w/4,y);
}
void draw_death(Guest& g,const Geometry& geo) {
    // The original's "N CONTINUES REMAINING" (its "%d %s", at y -80).
    const int n=psx_mod_read_byte(continues);
    const uint32_t s=psx_mod_read_word(strings+4u*(n==1 ? text_remaining : text_remaining_many));
    std::string words;
    for(uint32_t a=s;s>=0x80010000u && s<0x80200000u && words.size()<40;++a) {const char ch=(char)psx_mod_read_byte(a);if(!ch) break;words+=ch;}
    char line[64];std::snprintf(line,sizeof line,"%d %s",n,words.c_str());
    g.say(line,0,-80,font_menu|centre);
    draw_list(g,geo,8);
    const int y=prompt_y(geo);
    g.prompt(icon_select,"SELECT",-geo.w/8,y);
    g.prompt(icon_cross,"CONFIRM",geo.w/8,y);
}
void draw_brief(Guest& g,const Geometry& geo) {
    // The original pause screen's briefing: title, then up to ten lines from
    // x0 + w/32 + 15, 26 below the title and 15 apart, in the small font.
    level_title(g,geo);
    const int n=level(), x=geo.x0+(geo.w>>5)+15, y=heading_y(geo)+26;
    for(int i=0;i<10;++i) {
        const int id=(int16_t)psx_mod_read_half(briefings+20*n+2*i);
        if(id>0 && id<=text_last) g.say_id(id,x,y+15*i,font_small);
    }
    g.prompt(icon_triangle,"RETURN",0,prompt_y(geo));
}
void draw_ask(Guest& g,const Geometry& geo) {
    const int heading=ask_heading();
    if(heading<0) g.say(heading==-1 ? "QUIT GAME?" : heading==-2 ? "QUIT TO MENU?" : "RESTORE DEFAULT KEYS?",0,-40,font_menu|centre);
    else g.say_id(heading,0,-40,font_menu|centre);
    g.say_id(text_yes,0,-6,font_menu|centre|(cursor==0?blue:0));
    g.say_id(text_no,0,13,font_menu|centre|(cursor==1?blue:0));
    const int y=prompt_y(geo);
    g.prompt(icon_select,"SELECT",-geo.w/4,y);
    g.prompt(icon_cross,"CONFIRM",0,y);
    g.prompt(icon_triangle,"CANCEL",geo.w/4,y);
}

// ---- D19J title pages: the originals' calls, arguments and order ----
bool show_headings() {return psx_mod_read_half(pause_screen)!=screen_no_titles;}
int16_t half(uint32_t a) {return (int16_t)psx_mod_read_half(a);}
// State 7 (0x80022b64): the title in two lines, the items from y -30, 15
// apart, in font 0 (blue highlighted), then SELECT and CONFIRM.
void draw_main(Guest& g,const Geometry& geo) {
    const int hy=heading_y(geo);
    if(show_headings()) {g.say_id(text_title1,0,hy,font_menu|centre);g.say_id(text_title2,0,hy+19,font_menu|centre);}
    for(int i=0;i<(int)list.size();++i) {
        const int y=-30+15*i;
        const uint32_t f=font_small|centre|(i==cursor?blue:0);
        switch(list[i].act) {
        case Act::OnePlayer: g.say_id(text_one,0,y,f);break;
        case Act::TwoPlayer: g.say_id(text_two,0,y,f);break;
        case Act::Load: g.say_id(text_load,0,y,f);break;
        case Act::Options: g.say_id(text_options,0,y,f);break;
        case Act::Level: {
            const int id=half(level_names+2*(psx_mod_read_half(level_choice)&31));
            if(id>0 && id<=text_last) g.say_id(id,0,y,f);
            break;
        }
        default: g.say(list[i].label,0,y,f);break; // QUIT GAME
        }
    }
    const int y=prompt_y(geo);
    g.prompt_id(icon_select,-(geo.w/4),y,text_select);
    g.prompt_id(icon_cross,geo.w/4,y,text_confirm);
}
// 0x80084534: SELECT, CONFIRM and RETURN, the heading at y -76, then the
// four difficulties from y -47, 19 apart.
void draw_difficulty(Guest& g,const Geometry& geo) {
    const int y=prompt_y(geo);
    g.prompt_id(icon_select,-(geo.w/4),y,text_select);
    g.prompt_id(icon_cross,0,y,text_confirm);
    g.prompt_id(icon_triangle,geo.w/4,y,text_return);
    if(show_headings()) g.say_id(text_difficulty,0,-76,font_menu|centre);
    const int cur=half(diff_cursor);
    for(int i=0;i<4;++i) g.say_id((int)psx_mod_read_word(diff_ids+4*i),0,19*i-47,font_menu|centre|(i==cur?blue:0));
}
// 0x8008484c: four prompts w/8 apart, the heading, then LEVEL (menu font) and
// TIME, KILLS and SPLIT (font 0), labels right-aligned at 0, values from 0.
void draw_two(Guest& g,const Geometry& geo) {
    const int y=prompt_y(geo), e=geo.w/8;
    g.prompt_id(icon_select,15-3*e,y,text_select);
    g.prompt_id(icon_change,5-e,y,text_change);
    g.prompt_id(icon_cross,e-5,y,text_confirm);
    g.prompt_id(icon_triangle,3*e,y,text_cancel);
    if(show_headings()) g.say_id(text_two_heading,0,heading_y(geo),font_menu|centre);
    const int cur=half(two_cursor);
    int row=geo.y0+(geo.h-76)/2;
    g.say_id(text_level,0,row,font_menu|right|(cur==0?blue:0));
    g.say_id(half(two_levels+2*std::clamp<int>(half(two_level),0,5)),0,row,font_menu);
    row+=38;
    // The original's formats (0x80016e98, guarded): "%d %s" with MINUTES, "%d".
    char value[48];
    g.say_id(text_time,0,row,font_small|right|(cur==1?blue:0));
    if(const int t=half(two_time)) {
        const uint32_t s=psx_mod_read_word(strings+4u*text_minutes);
        std::string minutes;
        for(uint32_t a=s;s>=0x80010000u && s<0x80200000u && minutes.size()<20;++a) {const char ch=(char)psx_mod_read_byte(a);if(!ch) break;minutes+=ch;}
        std::snprintf(value,sizeof value,"%d %s",t,minutes.c_str());
        g.say(value,0,row,font_small);
    } else g.say_id(text_unlimited,0,row,font_small);
    row+=15;
    g.say_id(text_kills,0,row,font_small|right|(cur==2?blue:0));
    if(const int k=half(two_kills)) {std::snprintf(value,sizeof value,"%d",k);g.say(value,0,row,font_small);}
    else g.say_id(text_unlimited,0,row,font_small);
    row+=15;
    g.say_id(text_split,0,row,font_small|right|(cur==3?blue:0));
    g.say_id(half(two_split) ? text_vertical : text_horizontal,0,row,font_small);
}
// 0x80085060: the heading, PLAYER 1 and PLAYER 2 (a name blue until that
// player confirms), then CHANGE and CONFIRM, or START once both have; CANCEL.
void draw_character(Guest& g,const Geometry& geo) {
    if(show_headings()) g.say_id(text_character,0,heading_y(geo),font_menu|centre);
    const uint32_t locks=psx_mod_read_word(char_locks);
    int row=geo.y0+(geo.h-76)/2;
    for(uint32_t p=0;p<2;++p,row+=38) {
        g.say_id(p ? text_player2 : text_player1,0,row,font_menu|right);
        const uint32_t sel=psx_mod_read_word(char_choice+4*p);
        g.say_id((int)psx_mod_read_word(char_ids+4*(sel<6 ? sel : 0)),0,row,font_menu|(locks&(1u<<p) ? 0 : blue));
    }
    const int y=prompt_y(geo);
    if(locks!=3) {g.prompt_id(icon_change,-(geo.w/4),y,text_change);g.prompt_id(icon_cross,0,y,text_confirm);}
    else g.prompt_id(icon_cross,geo.x0+geo.w/3,y,text_start);
    g.prompt_id(icon_triangle,geo.w/4,y,text_cancel);
}

// ---- D19J: the title pages' actions, in the frontend tick ----
// Each page's entry is what its handler does while its fresh flag is set.
// The main menu on the original's cursor (the item a page was entered from).
void enter_main() {
    open_list(Page::Main,Act::OnePlayer);
    cursor=std::clamp<int>(half(fe_cursor),0,(int)list.size()-2);
    title_enter=true;
}
void enter_difficulty() {
    page=Page::Difficulty;
    if(psx_mod_read_word(diff_fresh)) {psx_mod_write_word(diff_fresh,0);psx_mod_write_half(diff_saved,psx_mod_read_half(diff_cursor));}
}
void enter_two() {
    page=Page::TwoPlayer;
    if(psx_mod_read_word(two_fresh)) {
        psx_mod_write_half(two_cursor,0);
        const uint32_t m=psx_mod_read_word(split_mode);
        if(m==1) psx_mod_write_half(two_split,1);
        else if(m==2) psx_mod_write_half(two_split,0);
        psx_mod_write_word(two_fresh,0);
    }
}
void enter_character() {
    page=Page::Character;
    if(psx_mod_read_word(char_fresh)) {
        for(uint32_t p=0;p<2;++p) psx_mod_write_word(char_choice+4*p,(uint32_t)(int32_t)half(char_saved+2*p));
        psx_mod_write_word(char_locks,0);psx_mod_write_word(char_fresh,0);
    }
}
// Wrap v + step into [0, n), as the handlers do for their values.
int16_t wrap_add(int16_t v,int step,int n) {const int r=v+step;return (int16_t)(r>=n ? 0 : r<0 ? n-1 : r);}
// The original main menu's level choice (its fifth item): 0-3, 5-12, 21-29.
void step_level(int dir) {
    int v=half(level_choice)+dir;
    if(dir>0) {if(v>=32) v=0;if(v==13) v=21;else if(v==4) v=5;else if(v==30) v=0;}
    else {if(v<0) v=31;if(v==20) v=12;else if(v==4) v=3;else if(v==31) v=29;}
    psx_mod_write_half(level_choice,(uint16_t)v);
}
// One key on a title page: the original handler's response. Returns the sound.
int title_command(Cmd cmd) {
    switch(page) {
    case Page::Main: {
        const int n=(int)list.size();
        const Act a=list[cursor].act;
        int s=-1;
        switch(cmd) {
        case Cmd::Up: cursor=(cursor+n-1)%n;s=1;break;
        case Cmd::Down: cursor=(cursor+1)%n;s=1;break;
        case Cmd::Left: case Cmd::Right: if(a==Act::Level) {step_level(cmd==Cmd::Right ? 1 : -1);s=1;} break;
        case Cmd::Back: break; // nothing, as the original
        case Cmd::Confirm:
            s=1;
            switch(a) {
            case Act::OnePlayer: case Act::Level: psx_mod_write_half(players_choice,1);enter_difficulty();break;
            case Act::TwoPlayer: psx_mod_write_half(players_choice,2);enter_two();break;
            case Act::Load: psx_mod_write_half(players_choice,1);pending_state=state_load;break;
            case Act::Options:
                // State 11: sub-screen 9, which D19 makes ours in this very tick.
                page=Page::Options;screen=-1;cursor=0;top=0;back_label=contexts[0].back_label;
                psx_mod_write_half(contexts[0].sub,contexts[0].options);
                break;
            default: ask_from=Page::Main;ask_act=Act::QuitGame;page=Page::Ask;cursor=1;break; // QUIT GAME?
            }
            break;
        }
        if(page==Page::Main && list[cursor].act!=Act::QuitGame) psx_mod_write_half(fe_cursor,(uint16_t)cursor);
        return s;
    }
    case Page::Difficulty: {
        const int16_t c=half(diff_cursor);
        switch(cmd) {
        case Cmd::Up: psx_mod_write_half(diff_cursor,(uint16_t)wrap_add(c,-1,4));return 1;
        case Cmd::Down: psx_mod_write_half(diff_cursor,(uint16_t)wrap_add(c,1,4));return 1;
        case Cmd::Back:
            psx_mod_write_half(diff_cursor,psx_mod_read_half(diff_saved));psx_mod_write_word(diff_fresh,1);
            enter_main();return 0;
        case Cmd::Confirm: pending_state=state_difficulty;handoff=Handoff::Pending;return -1; // its handler sounds
        default: return -1;
        }
    }
    case Page::TwoPlayer: {
        const int16_t c=half(two_cursor);
        switch(cmd) {
        case Cmd::Up: psx_mod_write_half(two_cursor,(uint16_t)wrap_add(c,-1,4));return 1;
        case Cmd::Down: psx_mod_write_half(two_cursor,(uint16_t)wrap_add(c,1,4));return 1;
        case Cmd::Left: case Cmd::Right: {
            const int d=cmd==Cmd::Right ? 1 : -1;
            switch(c) {
            case 0: psx_mod_write_half(two_level,(uint16_t)wrap_add(half(two_level),d,6));break;
            case 1: psx_mod_write_half(two_time,(uint16_t)(d>0 ? (half(two_time)+5>=16 ? 0 : half(two_time)+5) : (half(two_time)-5<0 ? 15 : half(two_time)-5)));break;
            case 2: psx_mod_write_half(two_kills,(uint16_t)(d>0 ? (half(two_kills)+5>=16 ? 0 : half(two_kills)+5) : (half(two_kills)-5<0 ? 15 : half(two_kills)-5)));break;
            case 3: psx_mod_write_half(two_split,(uint16_t)wrap_add(half(two_split),1,2));break; // both keys toggle
            }
            return 1;
        }
        case Cmd::Back: psx_mod_write_word(two_fresh,1);enter_main();return 0;
        case Cmd::Confirm: {
            const int16_t split=half(two_split);
            if(split==0) psx_mod_write_word(split_mode,2);
            else if(split==1) psx_mod_write_word(split_mode,1);
            psx_mod_write_word(two_fresh,1);
            psx_mod_write_half(level_choice,(uint16_t)(half(two_level)+15));
            enter_character();
            return 1;
        }
        }
        return -1;
    }
    case Page::Character: {
        // One keyboard for both players: player 1 chooses and confirms, then
        // player 2 (the original reads each player's own pad).
        const uint32_t locks=psx_mod_read_word(char_locks);
        const int row=!(locks&1) ? 0 : !(locks&2) ? 1 : -1;
        switch(cmd) {
        case Cmd::Left: case Cmd::Right: {
            if(row<0) return -1;
            const uint32_t a=char_choice+4*row, v=psx_mod_read_word(a);
            psx_mod_write_word(a,cmd==Cmd::Right ? (v+1>=6 ? 0 : v+1) : (v==0 || v>=6 ? 5 : v-1));
            return 1;
        }
        case Cmd::Confirm:
            if(row>=0) {psx_mod_write_word(char_locks,locks|(1u<<row));return 1;}
            pending_state=state_character;handoff=Handoff::Pending;return -1; // its handler sounds
        case Cmd::Back:
            if(locks&2) {psx_mod_write_word(char_locks,locks&~2u);return 4;}
            if(locks&1) {psx_mod_write_word(char_locks,locks&~1u);return 4;}
            psx_mod_write_word(char_fresh,1);enter_two();return 0;
        default: return -1;
        }
    }
    default: return -1;
    }
}
} // namespace

bool gameplay_tick(CPUState* cpu);
bool title_tick(CPUState* cpu,const MenuContext* c);
void draw_page(CPUState* cpu,const MenuContext* c);

// Frontend state 12 calls 0x800220f4 each tick just before the sub-screen
// dispatch: the place to apply the keys' actions (the original screen then
// runs in this very tick) and to take over the original OPTIONS before its
// handler draws anything.
bool trace() {static const bool v=std::getenv("DNTTK_MENU_TRACE")!=nullptr;return v;}
void game_menu_tick(CPUState* cpu) {
    if(trace()) {static uint32_t n;if(n++<40) std::fprintf(stderr,"[TTK menu] tick pc=%08x mode %u enabled %d replay %d precise %d ls %d bail %d\n",
        cpu->pc,psx_mod_read_half(game_mode),enabled(),psx_mod_replay_active(),g_precise_mode,g_ls_mode,g_psx_call_bail);}
    if(!enabled() || psx_mod_replay_active() || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    if(handoff==Handoff::Armed) {
        // D19J: the original difficulty (8) or character (9) handler is about to
        // run: one Cross press on pad 1, nothing else new. It draws the page we
        // showed and starts the game, as on the original's own Cross.
        const uint32_t ra=cpu->gpr[31];
        const uint16_t state=psx_mod_read_half(fe_state);
        if(psx_mod_read_half(game_mode)==0 && ((ra==from_difficulty && state==state_difficulty) || (ra==from_character && state==state_character))) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            for(uint32_t pad:{0u,0x54u}) for(uint32_t at:history_checked) {
                const uint32_t a=pad_history+pad+at, w=psx_mod_read_word(a);
                if(w&3u) psx_mod_write_word(a,w&~3u);
            }
            psx_mod_write_word(pad_history+history_cross,(psx_mod_read_word(pad_history+history_cross)&~3u)|1u);
            handoff=Handoff::None;++finals;
            if(trace()) std::fprintf(stderr,"[TTK menu] title handoff %u\n",state);
        }
        return;
    }
    const MenuContext* c=current_context();
    if(!c) return;
    ensure_init();
    if(!authentic()) return;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    savestate_check();
    // D19I: in gameplay our pages are drawn here, after the actions, like the
    // original's pause text (see draw_page).
    struct DrawAfter {CPUState* cpu;const MenuContext* c;~DrawAfter() {if(c->mode==1) draw_page(cpu,c);}} draw_after{cpu,c};
    // D19O: the blocking load of a demo scene reads no pad; our menu still does.
    if(c->mode==0 && owned && cpu->gpr[31]!=from_options && psx_mod_read_half(demo_load)==0) input_menu_poll();
    if(pending_sound>=0) {
        busy=true;
        {Guest g(cpu);g.call(fn_sound,(uint32_t)pending_sound,0,0,0);}
        busy=false;
        pending_sound=-1;
    }
    if(c->mode==1) {
        if(gameplay_tick(cpu)) return;
    }
    uint16_t sub=psx_mod_read_half(c->sub);
    if(pending_sub!=-1) {
        // D19J: back from OPTIONS to our main menu keeps the keys.
        const bool title=pending_sub==title_sub && sub==ours_sub;
        if(sub==ours_sub) {
            if(pending_cursor>=0) psx_mod_write_half(c->cursor,(uint16_t)pending_cursor);
            if(c->mode==0) psx_mod_write_half(controller_pad,0);
            psx_mod_write_half(c->sub,(uint16_t)(pending_sub==-2 ? c->back : pending_sub));
        }
        pending_sub=pending_cursor=-1;owned=title;screen=-1;
        if(!title && title_page(page)) page=Page::Options;
        return;
    }
    if(c->mode==0) {
        if(sub==title_sub) {
            if(title_tick(cpu,c)) return;
            sub=psx_mod_read_half(c->sub); // OPTIONS chosen: sub-screen 9, below
        } else if(page!=Page::Keys && !(page==Page::Ask && ask_from==Page::Keys)) page=Page::Options;
    }
    if(sub==c->options) {
        // The original OPTIONS (entered, or back from SOUND, CONTROLLER or GAME
        // SETTINGS): ours instead, that item highlighted.
        cursor=std::clamp<int>((int16_t)psx_mod_read_half(c->cursor),0,2);
        screen=-1;top=0;owned=true;back_label=c->back_label;++entries;
        psx_mod_write_half(c->sub,ours_sub);
    } else if(sub==ours_sub && !owned) {
        // A savestate saved on one of our screens: start again at OPTIONS.
        cursor=0;screen=-1;top=0;owned=true;back_label=c->back_label;
    } else if(sub!=ours_sub) owned=false;
}

// D19H, in gameplay (mode 1) at the pause dispatch, before the screen's
// handler runs: our actions, and the original pause menu and death screen
// replaced by our lists before they draw. True when the tick is done.
bool gameplay_tick(CPUState* cpu) {
    uint16_t sub=psx_mod_read_half(pause_screen);
    if(pending_final!=Final::None) {
        const Final f=pending_final;
        pending_final=Final::None;
        const bool pause=f==Final::Resume || f==Final::PauseRestart || f==Final::PauseMainMenu;
        if(sub==(pause ? ours_sub : death_sub)) {
            // D19I: this image still shows the page, as the original's
            // handler draws its screen in the tick that acts on the press.
            draw_page(cpu,&contexts[1]);
            busy=true;
            {
                Guest g(cpu);
                // The handler's part (pause 0x80085c04 / 0x80086130, death
                // 0x8008692c / 0x80086c04 / 0x80086aa4).
                g.call(fn_sound,f==Final::Resume ? 0u : 1u,0,0,0);
                switch(f) {
                case Final::Resume: psx_mod_write_half(pause_reset,1);break;
                case Final::PauseRestart: psx_mod_write_half(pause_reset,1);psx_mod_write_half(pause_screen,restart_code);break;
                case Final::PauseMainMenu: psx_mod_write_half(pause_reset,1);psx_mod_write_half(pause_screen,main_menu_code);break;
                case Final::DeathContinue:
                    if(challenge()) {psx_mod_write_byte(continues,(uint8_t)(psx_mod_read_byte(continues)-1));g.call(fn_event,event_replay,0,0,0);}
                    else psx_mod_write_byte(continue_request,2);
                    psx_mod_write_half(death_reset,1);break;
                case Final::DeathRestart: g.call(fn_event,event_restart,0,0,0);psx_mod_write_half(death_reset,1);break;
                case Final::DeathQuit: psx_mod_write_half(death_reset,1);g.call(fn_event,challenge() ? event_restart : event_quit,0,0,0);break;
                case Final::None: break;
                }
                // The handler returned 0 (resume, continue, restart and quit
                // from death): the main loop's resume (0x80026950-0x80026994).
                if(f!=Final::PauseRestart && f!=Final::PauseMainMenu) {
                    const uint32_t flags=player+player_size*psx_mod_read_half(controller_pad)+player_flags;
                    psx_mod_write_half(paused,0);
                    psx_mod_write_word(flags,psx_mod_read_word(flags)|0x4000u);
                    g.call(fn_resume_sound,0,0,0,0);g.call(fn_resume_music,0,0,0,0);
                    resumed=true;
                }
            }
            busy=false;
            ++finals;
            if(trace()) std::fprintf(stderr,"[TTK menu] final %d\n",(int)f);
        }
        owned=false;page=Page::Options;screen=-1;return_item=Act::Resume;
        return true;
    }
    if(pending_load) {
        pending_load=false;
        if(sub==death_sub) {
            psx_mod_write_half(death_reset,0);psx_mod_write_half(death_prompt,3);
            psx_mod_write_half(pause_screen,death_top);death_load=true;
        } else if(sub==ours_sub) {
            psx_mod_write_half(pause_reset,1);psx_mod_write_half(pause_screen,pause_load);
        }
        owned=false;page=Page::Options;screen=-1;
        return true;
    }
    if(sub==pause_top) {
        open_list(Page::Pause,return_item);return_item=Act::Resume;
        owned=true;back_label="RESUME";++entries;
        input_release_for_menu();
        psx_mod_write_half(pause_screen,ours_sub);
        return true;
    }
    if(sub==death_top) {
        // Not while the original's LOAD runs for the death screen (prompt 3);
        // its CANCEL clears the prompt and brings the list back.
        if(death_load && psx_mod_read_half(death_prompt)==3) return true;
        const Act highlight=death_load ? Act::Load : Act::Continue;
        death_load=false;
        open_list(Page::Death,highlight);owned=true;++entries;
        input_release_for_menu();
        psx_mod_write_half(pause_screen,death_sub);
        return true;
    }
    death_load=false;
    if(!sub) return_item=Act::Resume;
    if(!owned && !resumed && (sub==ours_sub || sub==death_sub)) {
        // A savestate saved on one of our screens: the list again. (Not after
        // our own resume: the level start after the death screen's RESTART
        // LEVEL runs this tick through 0x800220f4 before the composition
        // clears the value.)
        open_list(sub==death_sub ? Page::Death : Page::Pause,Act::Resume);
        owned=true;back_label="RESUME";
        return true;
    }
    if(sub==death_sub) return true;
    if(page!=Page::Options && sub==ours_sub) return true;
    page=Page::Options;
    return false; // OPTIONS and its screens: D19's own handling
}

// D19J: the title pages, at state 12's call before its sub-screen dispatch
// (the pad was read just before). True when the tick is done.
bool title_tick(CPUState* cpu,const MenuContext* c) {
    if(cpu->gpr[31]!=from_options) {
        // A load loop's backdrop (D19O): the menu is drawn over the loading
        // logo, so it moves there too. Confirm and Back wait for a normal tick.
        if(!owned || !title_page(page)) return true;
        size_t i=0;
        busy=true;
        {
            Guest g(cpu);
            for(;i<title_cmds.size() && title_cmds[i]!=Cmd::Confirm && title_cmds[i]!=Cmd::Back && title_page(page);++i) {
                const int s=title_command(title_cmds[i]);
                if(s>=0) g.call(fn_sound,(uint32_t)s,0,0,0);
            }
        }
        busy=false;
        title_cmds.erase(title_cmds.begin(),title_cmds.begin()+(std::ptrdiff_t)i);
        return true;
    }
    if(!owned) {
        // A savestate saved on one of our pages: the main menu again.
        enter_main();owned=true;back_label=c->back_label;title_cmds.clear();
    }
    if(!title_page(page) && page!=Page::Ask) return false;
    busy=true;
    {
        Guest g(cpu);
        for(size_t i=0;i<title_cmds.size() && pending_state<0 && title_page(page);++i) {
            const int s=title_command(title_cmds[i]);
            if(s>=0) g.call(fn_sound,(uint32_t)s,0,0,0);
        }
        title_cmds.clear();
        if(title_enter) {
            // State 6: the idle time starts again and the level choice resets.
            psx_mod_write_half(fe_timer,idle_fields);psx_mod_write_half(level_choice,0);
            title_enter=false;
        }
        if(page==Page::Main && pending_state<0) {
            // State 7: back to PRESS START (state 4) after the idle time.
            if(title_activity) psx_mod_write_half(fe_timer,idle_fields);
            const uint16_t t=(uint16_t)(psx_mod_read_half(fe_timer)-psx_mod_read_half(fe_fields));
            psx_mod_write_half(fe_timer,t);
            if((int16_t)t<=0) pending_state=state_start;
        }
        title_activity=false;
    }
    busy=false;
    return psx_mod_read_half(c->sub)==title_sub;
}

// D19J: the frontend tick's entry (0x80022324), before the pad read and the
// state dispatch: the original main menu (state 6 or 7) and its pages (8, 9)
// become ours before their handlers draw anything, and a state chosen on our
// pages in the last tick is set.
void title_entry(CPUState* cpu) {
    (void)cpu;
    if(!enabled() || psx_mod_replay_active() || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    if(psx_mod_read_half(game_mode)!=0) return;
    ensure_init();
    if(!authentic()) return;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    savestate_check();
    if(handoff==Handoff::Armed) handoff=Handoff::None; // the handler never ran
    const uint16_t state=psx_mod_read_half(fe_state);
    if(pending_state>=0) {
        if(state==contexts[0].state_value && psx_mod_read_half(contexts[0].sub)==title_sub) {
            psx_mod_write_half(fe_state,(uint16_t)pending_state);
            if(handoff==Handoff::Pending) handoff=Handoff::Armed;
        } else handoff=Handoff::None;
        pending_state=-1;owned=false;page=Page::Options;screen=-1;title_cmds.clear();
        return;
    }
    if(handoff!=Handoff::None) return;
    if(state==6 || state==7) {
        enter_main();
    } else if(state==state_difficulty || state==state_character) {
        // A savestate saved on the original's pages: ours.
        if(state==state_character) enter_character();
        else if(half(players_choice)==2) enter_two();
        else enter_difficulty();
    } else return;
    owned=true;screen=-1;top=0;back_label=contexts[0].back_label;title_cmds.clear();title_activity=false;++entries;
    psx_mod_write_half(fe_state,contexts[0].state_value);
    psx_mod_write_half(contexts[0].sub,title_sub);
}

// Our page for this image, once per image. The title draws it at the view
// composition (game_menu_frame). D19I: gameplay draws it at the pause
// dispatch, after 0x80025f28 has put the paused world's darkening quad into the
// drawing chain that follows the composition: the original pause text is drawn
// there too. Drawn at the composition, our text went into the world's chain,
// under that quad and behind Duke, at about half brightness.
uint64_t skip_owned=0,skip_screen=0,skip_image=0,drawn_calls=0;uint32_t skip_ra=0;   // diagnostic
void draw_page(CPUState* cpu,const MenuContext* c) {
    if(!owned) {++skip_owned;skip_ra=cpu->gpr[31];return;}
    if(!our_screen(c,psx_mod_read_half(c->sub))) {++skip_screen;skip_ra=cpu->gpr[31];return;}
    if(c->mode==1 && !psx_mod_read_half(paused)) return; // only over the paused world
    const uint32_t image=psx_mod_read_word(image_counter);
    if(image==last_image && cpu->gpr[31]==last_draw_ra) {++skip_image;skip_ra=cpu->gpr[31];return;}
    ++drawn_calls;
    last_image=image;last_draw_ra=cpu->gpr[31];
    if(!sliders) sliders=psx_mod_alloc_gpu_dma_memory(slider_size*slider_slots,16);
    busy=true;
    {
        Guest g(cpu);
        const Geometry geo=read_geometry();
        switch(page) {
        case Page::Pause: draw_pause(g,geo);break;
        case Page::Death: draw_death(g,geo);break;
        case Page::Brief: draw_brief(g,geo);break;
        case Page::Ask: draw_ask(g,geo);break;
        case Page::Options: if(screen<0) draw_options(g,geo); else draw_settings(g,geo);break;
        case Page::Main: draw_main(g,geo);break;
        case Page::Difficulty: draw_difficulty(g,geo);break;
        case Page::TwoPlayer: draw_two(g,geo);break;
        case Page::Character: draw_character(g,geo);break;
        case Page::Keys: draw_keys(g,geo);break;
        }
    }
    busy=false;
    ++draws;
}

void game_menu_frame(CPUState* cpu) {
    if(!enabled() || psx_mod_replay_active() || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    {std::lock_guard<std::recursive_mutex> lock(mutex);savestate_check();}
    if(resumed) {
        // After our resume (or continue, restart, quit) the screen value goes
        // back to the original's 0, in whatever mode the game is in now.
        std::lock_guard<std::recursive_mutex> lock(mutex);
        const uint16_t v=psx_mod_read_half(pause_screen);
        if(!psx_mod_read_half(paused) && (v==ours_sub || v==death_sub)) psx_mod_write_half(pause_screen,0);
        resumed=false;
    }
    const MenuContext* c=current_context();
    if(!c) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        owned=false;screen=-1;
        return;
    }
    if(c->mode==1) return; // D19I: drawn at the pause dispatch (game_menu_tick)
    ensure_init();
    if(!authentic()) return;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    draw_page(cpu,c);
}
} // namespace ttk

extern "C" const char* ttk_game_menu_json() {
    using namespace ttk;
    static char buffer[640];
    std::snprintf(buffer,sizeof buffer,"{\"owned\":%s,\"page\":%d,\"screen\":%d,\"cursor\":%d,\"top\":%d,\"items\":%d,\"entries\":%llu,\"draws\":%llu,\"writes\":%llu,\"finals\":%llu,\"handoff\":%d,\"pending_state\":%d,\"identity\":%s,\"drops\":%llu,\"drop\":[%d,%u,%u],\"skips\":[%llu,%llu,%llu,%llu],\"skip_ra\":\"%x\"}",
        owned?"true":"false",(int)page,screen,cursor,top,(int)list.size(),(unsigned long long)entries,(unsigned long long)draws,(unsigned long long)writes,
        (unsigned long long)finals,(int)handoff,pending_state,identity_checked?(identity_ok?"true":"false"):"null",(unsigned long long)drops,drop_why,drop_state,drop_sub,(unsigned long long)skip_owned,(unsigned long long)skip_screen,(unsigned long long)skip_image,(unsigned long long)drawn_calls,skip_ra);
    return buffer;
}

PSX_MOD_CONSTRUCTOR(register_ttk_game_menu) {
    ttk::input_set_menu_handler(ttk::game_menu_input);
    // Each context's tick, just before its screen dispatch: the frontend's
    // background 0x800220f4 and the pause menu dispatch 0x80025fb0.
    for(uint32_t a:{0x800220f4u,0x80025fb0u})
        psx_mod_register_function_entry_plugin("ttk.game.menu",a,[](CPUState* cpu,uint32_t){ttk::game_menu_tick(cpu);});
    // D19J: the frontend tick itself, before its pad read and state dispatch.
    psx_mod_register_function_entry_plugin("ttk.game.menu",0x80022324u,[](CPUState* cpu,uint32_t){ttk::title_entry(cpu);});
}
