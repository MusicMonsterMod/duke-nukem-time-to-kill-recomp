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
#include "game_menu.h"
#include "live_env.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "code_identity.h"
#include "pc_input.h"
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
    {0x800c3f38u,0x3f8u,"ff6caefc38543ee771b477ac72c9c5a6ad95214c47a0a8ea6846489ed58f2a37"},   // font glyph tables (from 0x800c3f38: 0x800c3f32 and 0x800c3f34 are menu variables)
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
};
std::array<std::vector<uint32_t>,sizeof guards/sizeof guards[0]> expected;

constexpr uint32_t game_mode=0x800bcbb0, controller_pad=0x800be56c, geometry=0x800d2210;
constexpr uint32_t image_counter=0x800bdbd0;
constexpr uint32_t fn_text=0x8008b454, fn_width=0x8008b3c8, fn_sprite=0x8008b678;
constexpr uint32_t fn_slider_init=0x80083c40, fn_slider_draw=0x80083edc, fn_sound=0x8006bbd8;
constexpr uint32_t sprites=0x800c43b4, sentinel=0x800000fc;
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
// the pause list and its pages), 9 in gameplay (the death list).
bool our_screen(const MenuContext* c,uint16_t sub) {return sub==ours_sub || (c->mode==1 && sub==death_sub);}
// Text flags: font << 8, 4 centred, 8 right-aligned, 1 highlighted (blue CLUT).
constexpr uint32_t font_small=0, font_menu=0x100, centre=4, right=8, blue=1;
enum Icon {icon_triangle=4, icon_cross=6, icon_square=7, icon_change=11, icon_select=12};

struct Choice {const char* label;const char* env;const char* json;};
enum class Section {Controls, Presentation, Active};
struct Row {
    const char* label; Section section; const char* key; const char* env; bool live; const char* desc;
    std::vector<Choice> choices; int def; // a pick
    bool slider=false; int lo=0,hi=0,step=1; double scale=1; // value * scale is the setting
};
// One value per row: a choice index, or the slider value.
using Values=std::vector<int>;
struct Screen {const char* title; std::vector<Row> rows;};

const Choice on_off[]={{"ON","1","true"},{"OFF","0","false"}};
std::vector<Screen> make_screens() {
    std::vector<Screen> s(3);
    s[0]={"PC CONTROLS",{
        {"MOUSE SENSITIVITY",Section::Controls,"mouse_sensitivity","DNTTK_MOUSE_SENSITIVITY",false,
         "HOW FAR THE CAMERA TURNS FOR EACH MOUSE MOVE",{},12,true,1,200,1,0.01},
        {"INVERT MOUSE",Section::Controls,"invert_y","DNTTK_MOUSE_INVERT_Y",false,
         "MOVING THE MOUSE UP LOOKS DOWN",{{"OFF","0","false"},{"ON","1","true"}},0},
    }};
    s[1]={"DISPLAY",{
        {"RENDERER",Section::Presentation,"renderer",nullptr,false,"SOFTWARE IS THE ORIGINAL LOOK",
         {{"OPENGL","opengl","\"opengl\""},{"SOFTWARE","software","\"software\""}},0},
        {"RESOLUTION",Section::Presentation,"internal_scale",nullptr,false,"INTERNAL 3D RESOLUTION. 1X IS THE ORIGINAL",
         {{"1X","1","1"},{"2X","2","2"},{"3X","3","3"},{"4X","4","4"}},3},
        {"DISPLAY",Section::Presentation,"display",nullptr,false,"HOW THE GAME OPENS. F11 SWITCHES TO FULLSCREEN",
         {{"WINDOWED","windowed","\"windowed\""},{"BORDERLESS","borderless","\"borderless\""},{"EXCLUSIVE","exclusive","\"exclusive\""}},0},
        {"FILTER",Section::Presentation,"output_filter",nullptr,false,"HOW THE FINISHED IMAGE IS SCALED TO THE WINDOW",
         {{"LINEAR","linear","\"linear\""},{"NEAREST","nearest","\"nearest\""}},0},
        {"WIDESCREEN",Section::Controls,"widescreen","DNTTK_WIDESCREEN",false,"A WIDER VIEW, HUD IN THE CORNERS. MOVIES STAY 4:3",
         {{"16:9","16:9","\"16:9\""},{"16:10","16:10","\"16:10\""},{"21:9","21:9","\"21:9\""},{"AUTO","auto","\"auto\""},{"OFF","off","\"off\""}},0},
        {"FRAME RATE",Section::Controls,"frame_rate","DNTTK_FRAME_RATE",false,"SMOOTHER MOTION. THE GAME SPEED NEVER CHANGES",
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
        {"PROFILE",Section::Active,"active",nullptr,false,"VANILLA PLAYS THE ORIGINAL GAME AND ITS MENUS",
         {{"MODERNIZED","modernized","\"modernized\""},{"VANILLA","vanilla","\"vanilla\""}},0},
        {"CAMERA",Section::Controls,"camera","DNTTK_CAMERA_MODE",false,"INDEPENDENT: THE MOUSE TURNS THE CAMERA",
         {{"INDEPENDENT","independent","\"independent\""},{"ORIGINAL","original","\"original\""}},0},
        {"VIEW",Section::Controls,"view","DNTTK_CAMERA_VIEW",false,"P SWITCHES IN GAME",
         {{"THIRD PERSON","third","\"third\""},{"FIRST PERSON","first","\"first\""}},0},
        {"SHOULDER",Section::Controls,"shoulder","DNTTK_CAMERA_SHOULDER",false,"H CYCLES IN GAME",
         {{"CENTER","center","\"center\""},{"RIGHT","right","\"right\""},{"LEFT","left","\"left\""}},0},
        {"CAMERA DISTANCE",Section::Controls,"camera_distance","DNTTK_CAMERA_DISTANCE",false,"ALT AND THE MOUSE WHEEL ALSO SET IT IN GAME",
         {{"ORIGINAL","0","0"},{"1024","1024","1024"},{"1536","1536","1536"},{"2048","2048","2048"},
          {"3072","3072","3072"},{"4096","4096","4096"},{"6144","6144","6144"}},0},
        {"WEAPON AIM",Section::Controls,"weapon_aim","DNTTK_WEAPON_AIM",false,"VIEW: SHOTS GO WHERE THE CROSSHAIR POINTS",
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
enum class Page {Options, Pause, Death, Brief, Ask};
enum class Act {Resume, Brief, Options, Load, Restart, MainMenu, Continue, Quit, QuitGame};
struct ListItem {const char* label;Act act;};
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
uint64_t draws=0, entries=0, writes=0, finals=0;
uint32_t sliders=0; // GPU-addressable slider objects
constexpr int slider_slots=12, slider_size=0x40;

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
            const char* env=r.section==Section::Presentation ? presentation_value(r.key) : r.env ? std::getenv(r.env) : nullptr;
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

// The confirmed settings, for run.py (DNTTK_MENU_SETTINGS_FILE).
void write_settings() {
    const char* path=std::getenv("DNTTK_MENU_SETTINGS_FILE");
    if(!path || !*path) return;
    std::string controls, presentation, active="\"modernized\"";
    for(size_t s=0;s<screens.size();++s) for(size_t i=0;i<screens[s].rows.size();++i) {
        const Row& r=screens[s].rows[i];const int v=committed[s][i];
        char value[64];
        if(r.slider) std::snprintf(value,sizeof value,"%.2f",v*r.scale);
        else std::snprintf(value,sizeof value,"%s",r.choices[v].json);
        if(r.section==Section::Active) {active=value;continue;}
        std::string& out=r.section==Section::Controls ? controls : presentation;
        out+=std::string(out.empty()?"":",")+"\""+r.key+"\":"+value;
    }
    const std::string text="{\"version\":1,\"active\":"+active+",\"controls\":{"+controls+"},\"presentation\":{"+presentation+"}}\n";
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
    r.next+=80;
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
    if(p==Page::Pause) {
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
        list.push_back({"QUIT",Act::Quit});
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
void act_now(Act a) {
    const bool death=page==Page::Death || (page==Page::Ask && ask_from==Page::Death);
    switch(a) {
    case Act::Resume: pending_final=Final::Resume;break;
    case Act::Restart: pending_final=death ? Final::DeathRestart : Final::PauseRestart;break;
    case Act::MainMenu: pending_final=Final::PauseMainMenu;break;
    case Act::Continue: pending_final=Final::DeathContinue;break;
    case Act::Quit: pending_final=Final::DeathQuit;break;
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
// start on NO. QUIT GAME? has no text id: -1.
int ask_heading() {
    switch(ask_act) {
    case Act::Continue: return challenge() ? text_replay : text_continue;
    case Act::Restart: return text_restart;
    case Act::QuitGame: return -1;
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
    if(page!=Page::Options || screen<0) return;
    Row& r=screens[screen].rows[cursor];
    int& v=draft[screen][cursor];
    const int before=v;
    if(r.slider) v=std::clamp(v+dir*r.step*(held_ms>1200?5:1),r.lo,r.hi);
    else {const int n=(int)r.choices.size();v=(v+dir+n)%n;}
    if(v!=before) sound(1);
}
void open_screen(int s) {
    screen=s;cursor=0;top=0;draft[s]=committed[s];sound(0);
}
void leave_screen(bool keep) {
    const int s=screen;
    if(keep && draft[s]!=committed[s]) {
        committed[s]=draft[s];
        for(size_t i=0;i<screens[s].rows.size();++i) {
            const Row& r=screens[s].rows[i];
            // Until a change ttk_live_env() is plain getenv (an unset variable stays unset).
            if(r.live && !r.slider && r.env) ttk_live_env_set(r.env,r.choices[committed[s][i]].env);
        }
        write_settings();
    }
    draft[s]=committed[s];
    screen=-1;cursor=3+s;top=0;sound(0);
}
void defaults() {
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
        else {open_list(ask_from,ask_act);sound(1);}
        return;
    case Page::Options: break;
    }
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
    case Page::Ask: open_list(ask_from,ask_act);sound(1);return;
    case Page::Options: break;
    }
    if(screen>=0) {leave_screen(false);return;}
    // In gameplay OPTIONS goes back to our pause list (the original's 12 would
    // draw its own pause menu for a frame first).
    if(mode==1) {open_list(Page::Pause,Act::Options);sound(0);return;}
    pending_cursor=0;pending_sub=-2;sound(0); // -2: the context's back
}
} // namespace

bool game_menu_input(const MenuKeys& keys) {
    if(!enabled()) return false;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    // Taps that arrive while the menu cannot act yet wait for the next read.
    static MenuKeys carry;
    MenuKeys k=keys;
    k.up_taps+=carry.up_taps;k.down_taps+=carry.down_taps;k.left_taps+=carry.left_taps;k.right_taps+=carry.right_taps;
    k.confirm_taps+=carry.confirm_taps;k.back_taps+=carry.back_taps;k.square_taps+=carry.square_taps;
    carry=MenuKeys{};
    // Busy drawing, or a screen change waits for the next tick: keys wait too.
    if(quitting) return owned;
    if(busy || (owned && (pending_sub>=0 || pending_final!=Final::None || pending_load))) {
        if(owned) carry=k;
        return owned;
    }
    const MenuContext* c=current_context();
    if(!owned || !c || !our_screen(c,psx_mod_read_half(c->sub))) {
        rep_up=rep_down=rep_left=rep_right=Repeat{};
        return false;
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
        psx_dispatch_call(&cpu,address,sentinel);
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
    if(heading<0) g.say("QUIT GAME?",0,-40,font_menu|centre);
    else g.say_id(heading,0,-40,font_menu|centre);
    g.say_id(text_yes,0,-6,font_menu|centre|(cursor==0?blue:0));
    g.say_id(text_no,0,13,font_menu|centre|(cursor==1?blue:0));
    const int y=prompt_y(geo);
    g.prompt(icon_select,"SELECT",-geo.w/4,y);
    g.prompt(icon_cross,"CONFIRM",0,y);
    g.prompt(icon_triangle,"CANCEL",geo.w/4,y);
}
} // namespace

bool gameplay_tick(CPUState* cpu);
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
    const MenuContext* c=current_context();
    if(!c) return;
    ensure_init();
    if(!authentic()) return;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    // D19I: in gameplay our pages are drawn here, after the actions, like the
    // original's pause text (see draw_page).
    struct DrawAfter {CPUState* cpu;const MenuContext* c;~DrawAfter() {if(c->mode==1) draw_page(cpu,c);}} draw_after{cpu,c};
    if(pending_sound>=0) {
        busy=true;
        {Guest g(cpu);g.call(fn_sound,(uint32_t)pending_sound,0,0,0);}
        busy=false;
        pending_sound=-1;
    }
    if(c->mode==1) {
        if(gameplay_tick(cpu)) return;
    } else page=Page::Options;
    const uint16_t sub=psx_mod_read_half(c->sub);
    if(pending_sub!=-1) {
        if(sub==ours_sub) {
            if(pending_cursor>=0) psx_mod_write_half(c->cursor,(uint16_t)pending_cursor);
            if(c->mode==0) psx_mod_write_half(controller_pad,0);
            psx_mod_write_half(c->sub,(uint16_t)(pending_sub==-2 ? c->back : pending_sub));
        }
        pending_sub=pending_cursor=-1;owned=false;screen=-1;
        return;
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

// Our page for this image, once per image. The title draws it at the view
// composition (game_menu_frame). D19I: gameplay draws it at the pause
// dispatch, after 0x80025f28 has put the paused world's darkening quad into the
// drawing chain that follows the composition: the original pause text is drawn
// there too. Drawn at the composition, our text went into the world's chain,
// under that quad and behind Duke, at about half brightness.
void draw_page(CPUState* cpu,const MenuContext* c) {
    if(!owned || !our_screen(c,psx_mod_read_half(c->sub))) return;
    if(c->mode==1 && !psx_mod_read_half(paused)) return; // only over the paused world
    const uint32_t image=psx_mod_read_word(image_counter);
    if(image==last_image) return;
    last_image=image;
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
        }
    }
    busy=false;
    ++draws;
}

void game_menu_frame(CPUState* cpu) {
    if(!enabled() || psx_mod_replay_active() || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
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
    static char buffer[384];
    std::snprintf(buffer,sizeof buffer,"{\"owned\":%s,\"page\":%d,\"screen\":%d,\"cursor\":%d,\"top\":%d,\"items\":%d,\"entries\":%llu,\"draws\":%llu,\"writes\":%llu,\"finals\":%llu,\"identity\":%s}",
        owned?"true":"false",(int)page,screen,cursor,top,(int)list.size(),(unsigned long long)entries,(unsigned long long)draws,(unsigned long long)writes,
        (unsigned long long)finals,identity_checked?(identity_ok?"true":"false"):"null");
    return buffer;
}

PSX_MOD_CONSTRUCTOR(register_ttk_game_menu) {
    ttk::input_set_menu_handler(ttk::game_menu_input);
    // Each context's tick, just before its screen dispatch: the frontend's
    // background 0x800220f4 and the pause menu dispatch 0x80025fb0.
    for(uint32_t a:{0x800220f4u,0x80025fb0u})
        psx_mod_register_function_entry_plugin("ttk.game.menu",a,[](CPUState* cpu,uint32_t){ttk::game_menu_tick(cpu);});
}
