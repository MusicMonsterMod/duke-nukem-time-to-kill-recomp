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
    {0x800c3f30u,0x400u,"cccee537b47e6e10ccf58b4efa1998990a13f4d2fb2ee94421bfbbbccb8450fb"},   // font glyph tables
    {0x800c43b4u,0xf0u,"9ea07ba25cff15a7da9de43768b28d70b836cf13269c705e70e6730b55395b3b"},   // sprite records and font info
    {0x800c60f4u,0x400u,"1298dbf2ac90b8efff35a60c7ec93bc3dd6897d1434ec84ba5b1871093b09a7a"},  // text table
};
std::array<std::vector<uint32_t>,sizeof guards/sizeof guards[0]> expected;

constexpr uint32_t game_mode=0x800bcbb0, controller_pad=0x800be56c, geometry=0x800d2210;
constexpr uint32_t image_counter=0x800bdbd0;
constexpr uint32_t fn_text=0x8008b454, fn_width=0x8008b3c8, fn_sprite=0x8008b678;
constexpr uint32_t fn_slider_init=0x80083c40, fn_slider_draw=0x80083edc, fn_sound=0x8006bbd8;
constexpr uint32_t sprites=0x800c43b4, sentinel=0x800000fc;
constexpr uint16_t ours_sub=8;
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
bool owned=false;         // sub-screen 8 is ours
// Actions for the next live frame (guest writes and sounds happen there).
int pending_sub=-1, pending_cursor=-1, pending_sound=-1;
const char* back_label="CANCEL";
bool identity_ok=false, identity_checked=false;
uint32_t last_image=~0u;
uint64_t draws=0, entries=0, writes=0;
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
bool prev_confirm=true, prev_back=true, prev_square=true; // held keys at takeover are not presses
// A press, then 350 ms, then every 80 ms while held (D19B: predictable repeat).
int repeat(Repeat& r,bool held,uint64_t now) {
    if(!held) {r.held=false;return 0;}
    if(!r.held) {r.held=true;r.since=now;r.next=now+350;return 1;}
    if(now<r.next) return 0;
    r.next+=80;
    return 1;
}
// More than five rows: the compact layout (14 apart), nine at a time.
bool compact(const Screen& s) {return s.rows.size()>5;}
int rows_visible(const Screen& s) {return compact(s) ? 9 : (int)s.rows.size();}
void sound(int which) {if(pending_sound<0 || which==0) pending_sound=which;}
void move(int dir) {
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
    if(screen<0) return;
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
    if(screen<0) return;
    for(size_t i=0;i<screens[screen].rows.size();++i) draft[screen][i]=screens[screen].rows[i].def;
    sound(0);
}
void confirm() {
    if(screen>=0) {leave_screen(true);return;}
    const Item& it=options_items[cursor];
    if(it.guest_sub>=0) {pending_cursor=cursor;pending_sub=it.guest_sub;sound(0);}
    else open_screen(it.screen);
}
void back() {
    if(screen>=0) {leave_screen(false);return;}
    pending_cursor=0;pending_sub=-2;sound(0); // -2: the context's back
}
} // namespace

bool game_menu_input(const MenuKeys& k) {
    if(!enabled()) return false;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    // Busy drawing, or a screen change waits for the next tick: keys wait too.
    if(busy || (owned && pending_sub>=0)) return owned;
    const MenuContext* c=current_context();
    if(!owned || !c || psx_mod_read_half(c->sub)!=ours_sub) {
        prev_confirm=prev_back=prev_square=true; // keys held at takeover are not presses
        rep_up=rep_down=rep_left=rep_right=Repeat{};
        return false;
    }
    const uint64_t now=SDL_GetTicks();
    if(repeat(rep_up,k.up,now)) move(-1);
    if(repeat(rep_down,k.down,now)) move(1);
    if(repeat(rep_left,k.left,now)) change(-1,now-rep_left.since);
    if(repeat(rep_right,k.right,now)) change(1,now-rep_right.since);
    for(int w=k.wheel;w>0;--w) move(-1);
    for(int w=k.wheel;w<0;++w) move(1);
    if(k.confirm && !prev_confirm) confirm();
    else if(k.back && !prev_back) back();
    else if(k.square && !prev_square) defaults();
    prev_confirm=k.confirm;prev_back=k.back;prev_square=k.square;
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
} // namespace

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
    if(pending_sound>=0) {
        busy=true;
        {Guest g(cpu);g.call(fn_sound,(uint32_t)pending_sound,0,0,0);}
        busy=false;
        pending_sound=-1;
    }
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

void game_menu_frame(CPUState* cpu) {
    if(!enabled() || psx_mod_replay_active() || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    const MenuContext* c=current_context();
    if(!c) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        owned=false;screen=-1;
        return;
    }
    ensure_init();
    if(!authentic()) return;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if(!owned || psx_mod_read_half(c->sub)!=ours_sub) return;
    const uint32_t image=psx_mod_read_word(image_counter);
    if(image==last_image) return;
    last_image=image;
    if(!sliders) sliders=psx_mod_alloc_gpu_dma_memory(slider_size*slider_slots,16);
    busy=true;
    {
        Guest g(cpu);
        const Geometry geo=read_geometry();
        if(screen<0) draw_options(g,geo); else draw_settings(g,geo);
    }
    busy=false;
    ++draws;
}
} // namespace ttk

extern "C" const char* ttk_game_menu_json() {
    using namespace ttk;
    static char buffer[256];
    std::snprintf(buffer,sizeof buffer,"{\"owned\":%s,\"screen\":%d,\"cursor\":%d,\"top\":%d,\"entries\":%llu,\"draws\":%llu,\"writes\":%llu,\"identity\":%s}",
        owned?"true":"false",screen,cursor,top,(unsigned long long)entries,(unsigned long long)draws,(unsigned long long)writes,
        identity_checked?(identity_ok?"true":"false"):"null");
    return buffer;
}

PSX_MOD_CONSTRUCTOR(register_ttk_game_menu) {
    ttk::input_set_menu_handler(ttk::game_menu_input);
    // Each context's tick, just before its screen dispatch: the frontend's
    // background 0x800220f4 and the pause menu dispatch 0x80025fb0.
    for(uint32_t a:{0x800220f4u,0x80025fb0u})
        psx_mod_register_function_entry_plugin("ttk.game.menu",a,[](CPUState* cpu,uint32_t){ttk::game_menu_tick(cpu);});
}
