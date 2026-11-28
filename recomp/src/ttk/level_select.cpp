// SLUS-00583 debug level select (D26A). Lists the levels the original
// title-screen level-select cheat offers, with the game's own names, and
// travels by ending the current level through the original restart code.
// Contract: documentation/101-d26a-level-select.md.
//
// Game mode machine 8001BB84: mode 800BCBB0, phase 800CE1B0 (2 = running),
// handler table 800BCBB4. Mode 1 is gameplay: its running handler 8002666C
// ends the level once end code 800BE55E leaves 1; exit 800270E8 sends code
// 0xFD (the pause menu's restart) back into mode 1, whose init 80025438
// loads level index 800BE570 through 80024FDC.
//
// The index must not change while the old level still runs: the main loop
// calls the per-level player routine 800C5580[index], which lives in the
// resident level overlay. So the command only sets 0xFD; the index changes at
// the init's own teardown call (8002B9F4 from 8002546C), after its saved-game
// check and before 800254D0 reads the index for the loader.
#include "level_select.h"
#include "hidden_titles.h"
#include "ttk_register.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "code_identity.h"
#include "pc_input.h"
#include "console_syntax.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace ttk {
namespace {
struct Guard {uint32_t address,size;const char* digest;};
// Read-only digests of the owned executable's bytes.
const Guard guards[]={
    {0x8001bb70u,0x188u,"86fe7995b23089e443f8bd26b13682d75b138a95d83861ef1e8083d560ce0a21"}, // set_mode, mode machine
    {0x800bcbb4u,0x40u,"b9bfeae05a1c7d9213d5f68ab6d417b01b9ca4700c448dda0a06e36ea89dbd98"},  // mode handler table
    {0x80025438u,0x11cu,"6d52a2e1a54040f0764c68d5b4ceea2b955598037e8799681703df2153de9576"}, // mode 1 init, level load
    {0x8002666cu,0x7cu,"4d1b0a54acc03e10da0f5eda5cecd9f3e83ad3da0bf160f95f4726268651243d"},  // mode 1 end-code check
    {0x800270e8u,0xe8u,"eb6e3fbd7db19271a49d47b0767dafd6f9ff9d4035320933519db031d8d0ffe2"},  // mode 1 exit
    {0x80024fdcu,0x38u,"10397f6c68df68de25734e699c51273876cc72f5da5c9db1e3a23e99146545d3"},  // level loader entry
    {0x80022d48u,0x120u,"2e3f62a402a91439969465169b283075636400217c370bc3f3e629d6652ed1ba"}, // title level-select cycle
    {0x800c3d2cu,0x40u,"c9ef39aacbbd3027c182f6bdb255613e0794c223519cfcda97ce4a45a604c513"},  // level name string ids
};
std::array<std::vector<uint32_t>,sizeof guards/sizeof guards[0]> expected;

constexpr uint32_t mode=0x800bcbb0, phase=0x800ce1b0, level=0x800be570;
constexpr uint32_t end_code=0x800be55e, load_request=0x800be560, dying=0x800be55c;
constexpr uint32_t menu=0x800be56a, players=0x800c27bc;
constexpr uint32_t names=0x800c3d2c, strings=0x800c60f4;
constexpr uint8_t playing=1, restart=0xfd, level_complete=4;
constexpr uint32_t init_teardown_return=0x80025474;

long pending=-1;
uint32_t pending_loads;

// The title cheat's left/right cycle (80022D48) covers 0-31 and skips 4,
// 13-20 (unused, animation test, two-player arenas) and 30-31.
bool selectable(long n) {return n>=0 && n<30 && n!=4 && (n<13 || n>20);}

bool authentic() {return code_identity(guards,expected);}

std::string name(unsigned n) {
    if(const char* hidden=ttk::hidden_level_title(n)) return hidden;   // D61B
    const int id=int16_t(psx_mod_read_half(names+2*n));
    if(id<=0 || id>400)return {};
    const uint32_t text=psx_mod_read_word(strings+4u*unsigned(id));
    if(text<0x80010000u || text>=0x800ca968u)return {};
    std::string s;
    for(uint32_t a=text;s.size()<40;++a) {
        const uint8_t c=psx_mod_read_byte(a);
        if(!c)return s;
        if(c<32 || c>126)return {};
        s+=char(c);
    }
    return {};
}

bool in_play() {
    return psx_mod_read_half(mode)==1 && psx_mod_read_half(phase)==2;
}

const char* travel_refusal() {
    if(!in_play())return "Start or load a game first";
    if(psx_mod_read_byte(end_code)!=playing || psx_mod_read_word(load_request) || psx_mod_read_half(dying))
        return "The level is already ending";
    if(psx_mod_read_half(menu))return "Close the pause menu first";
    if(psx_mod_read_word(players)!=1)return "One-player games only";
    return nullptr;
}

void init_hook(CPUState* cpu,uint32_t address) {
    // The ending level tears down first (from 800270D0); wait for the init's call.
    if(pending<0 || address!=0x8002b9f4 || cpu->gpr[31]!=init_teardown_return)return;
    const long n=pending;pending=-1;
    // Only the restart this command started: not a death, a saved-game load
    // or a savestate loaded meanwhile.
    if(cpu->gpr[4]!=1 || psx_mod_savestate_loads()!=pending_loads ||
       psx_mod_read_half(mode)!=1 || psx_mod_read_byte(end_code)!=restart || psx_mod_read_word(load_request) ||
       !authentic()) {
        std::fprintf(stderr,"[TTK level] travel to %ld dropped (a0=%u ra=%08x mode=%u end=%02x load=%u states=%u/%u)\n",
                     n,cpu->gpr[4],cpu->gpr[31],psx_mod_read_half(mode),psx_mod_read_byte(end_code),
                     psx_mod_read_word(load_request),psx_mod_savestate_loads(),pending_loads);
        return;
    }
    psx_mod_write_word(level,uint32_t(n));
}

// D26L4: the list as a grid, one row per era (user 2026-11-27: "it just feels
// like a brick wall of text"). Tabs separate the cells; the console lines a
// block of tabbed rows up (elastic tab stops) and colours the label, numbers
// and names. Era rows follow level_era() (the savestate panel's).
struct Row {const char* label;std::array<int,3> levels;};
const Row campaign[]={
    {"PRESENT DAY",{0,5,9}},{"OLD WEST",{1,2,3}},{"MEDIEVAL",{6,7,8}},{"ANCIENT ROME",{10,11,12}},
    {"BOSSES",{27,28,29}},{"CHALLENGE STAGES",{21,22,23}},{"",{24,25,26}},
};
const Row hidden[]={
    {"CUT LEVELS",{4,13,30}},{"TEST MAPS",{14,31,-1}},
    {"2-PLAYER ARENAS",{15,16,17}},{"",{18,19,20}},
};
void grid(const Row* rows,size_t count,bool here,unsigned current,void (*say)(const char*)) {
    for(size_t r=0;r<count;++r) {
        std::string line=rows[r].label;
        for(int n:rows[r].levels) {
            if(n<0)continue;
            std::string title=name(unsigned(n));
            if(title.empty())title="?";
            // "CHALLENGE STAGE 4" under its CHALLENGE STAGES label reads as "STAGE 4".
            if(n>=21 && n<=26 && !title.compare(0,10,"CHALLENGE "))title.erase(0,10);
            char cell[64];
            std::snprintf(cell,sizeof cell,"\t%s%u %s",here && unsigned(n)==current?"*":"",unsigned(n),title.c_str());
            line+=cell;
        }
        say(line.c_str());
    }
}
void list(void (*say)(const char*)) {
    const bool here=in_play();
    const uint32_t current=psx_mod_read_word(level);
    say(here ? "Levels: type level N to travel (* is this level)" : "Levels: type level N to travel");
    grid(campaign,sizeof campaign/sizeof campaign[0],here,current,say);
    // D61: the indices the title cheat skips (note 101) load too (developer).
    // goto is Modernized only; Vanilla reaches them with level force N.
    say(input_modernized() ? "Hidden levels: type goto N (developer: unfinished maps the campaign never visits)"
                           : "Hidden levels: type level force N (developer: unfinished maps the campaign never visits)");
    grid(hidden,sizeof hidden/sizeof hidden[0],here,current,say);
}
}

const char* level_travel(long n, bool any_index) {
    // D61: the travel of "level N" for goto and the atlas. any_index (developer
    // "level force N") also takes the indices the title cheat skips (4, 13-20,
    // 30, 31: note 101), for the cut-content survey; they may not be levels.
    if(!authentic())return "Level select unavailable: unrecognised game code";
    if(any_index ? (n<0 || n>31) : !selectable(n))return "Unknown level - type levels for the list";
    if(!any_index && name(unsigned(n)).empty())return "Level select unavailable: level names not found";
    if(const char* refusal=travel_refusal())return refusal;
    pending=n;pending_loads=psx_mod_savestate_loads();
    psx_mod_write_byte(end_code,restart);
    const std::string title=name(unsigned(n));
    std::fprintf(stderr,"[TTK level] travel to %ld %s%s\n",n,title.empty()?"(unnamed)":title.c_str(),any_index?" (forced)":"");
    return nullptr;
}

long level_current() {return in_play() ? long(psx_mod_read_word(level)) : -1;}

bool level_title(unsigned n, char* out, unsigned cap) {
    if(!out || !cap)return false;
    out[0]=0;
    if(n>=32 || !authentic())return false;
    const std::string title=name(n);
    if(title.empty())return false;
    std::snprintf(out,cap,"%s",title.c_str());
    return true;
}

bool level_console_command(const char* line, void (*say)(const char*), bool& close) {
    close=false;
    if(spawn_console_command(line,say,close))return true;
    if(earthquake_console_command(line,say,close))return true;
    if(lights_console_command(line,say,close))return true;
    if(blast_audio_console_command(line,say,close))return true; // D41U
    if(lamp_console_command(line,say,close))return true; // D41D
    if(option_console_command(line,say,close))return true; // D53E
    if(beat_console_command(line,say,close))return true;
    if(crouch_console_command(line,say,close))return true;
    if(fov_console_command(line,say,close))return true;
    if(fp_idle_console_command(line,say,close))return true; // D53G
    if(fp_grip_console_command(line,say,close))return true; // D36N
    if(armed_hang_console_command(line,say,close))return true; // D08J11
    if(point_blank_console_command(line,say,close))return true;
    if(shot_view_console_command(line,say,close))return true;
    if(beam_follow_console_command(line,say,close))return true;
    if(bolt_launch_console_command(line,say,close))return true;
    if(bolt_barrel_console_command(line,say,close))return true;
    if(beam_anim_console_command(line,say,close))return true;
    if(shoot_seen_console_command(line,say,close))return true;
    if(auto_aim_console_command(line,say,close))return true;
    if(test_move_console_command(line,say,close))return true;
    if(round_follow_console_command(line,say,close))return true;
    if(hostvis_console_command(line,say,close))return true;
    if(smoothquads_console_command(line,say,close))return true;
    if(redraw_only_console_command(line,say,close))return true;
    if(renderer_console_command(line,say,close))return true;
    if(flash_console_command(line,say,close))return true;
    // D20: EDuke32's quicksave / quickload, the F6 / F9 keys' own action.
    if(!std::strcmp(line,"quicksave") || !std::strcmp(line,"quickload")) {
        const bool save=line[5]=='s';
        if(input_quick(save ? Quick::Save : Quick::Load)==QuickResult::Ignored)
            say(save ? "Can't quicksave while not in a game." : "Can't quickload while not in a game.");
        else close=true;
        return true;
    }
    // D20B (testing): end the level as reaching its exit does (end code 4):
    // the statistics screen, then the next level.
    if(!std::strcmp(line,"endlevel")) {
        if(!authentic()) {say("endlevel unavailable: unrecognised game code");return true;}
        if(const char* refusal=travel_refusal()) {
            char reply[96];std::snprintf(reply,sizeof reply,"Cannot end the level now: %s",refusal);
            say(reply);return true;
        }
        psx_mod_write_byte(end_code,level_complete);
        say("Level complete");
        close=true;
        return true;
    }
    // D61 (developer): "level force N" travels to any index 0-31, the ones the
    // title cheat skips too, for the cut-content survey (documentation/559).
    if(!std::strncmp(line,"level force ",12)) {
        char* end=nullptr;const long n=std::strtol(line+12,&end,10);
        if(end==line+12 || *end) {say("level force N (0-31; developer: skipped indices may not be levels)");return true;}
        if(const char* refusal=level_travel(n,true)) {
            char reply[96];std::snprintf(reply,sizeof reply,"Cannot travel now: %s",refusal);say(reply);return true;
        }
        char reply[64];std::snprintf(reply,sizeof reply,"Forcing level index %ld...",n);say(reply);
        close=true;return true;
    }
    if(!std::strcmp(line,"level force")) {say("Usage: level force N (0-31; developer: skipped indices may not be levels)");return true;}
    const bool all=!std::strcmp(line,"levels") || !std::strcmp(line,"level");
    if(!all && std::strncmp(line,"level ",6)) {
        // D26L4: every other line gets an answer here: a known command's usage,
        // or the nearest command to a typo.
        if(!*line)return false;
        if(!console_usage(line,say))console_unknown(line,say);
        return true;
    }
    if(!authentic()) {say("Level select unavailable: unrecognised game code");return true;}
    if(all) {list(say);return true;}
    char* end=nullptr;
    const long n=std::strtol(line+6,&end,10);
    if(end==line+6 || *end) {say("Try: level 3 (type levels for the list)");return true;}
    if(n<0 || n>31) {char reply[96];std::snprintf(reply,sizeof reply,"Level: no such level %ld. Type levels for the list",n);say(reply);return true;}
    if(!selectable(n)) {
        // D26L4: say which hidden level it is (user 2026-11-27 asked whether
        // the arenas can be reached): goto takes every hidden index.
        const std::string title=name(unsigned(n));
        const char* kind=n>=15 && n<=20 ? "a 2-player arena" : n==14 || n==31 ? "a test map" : "a cut level";
        char reply[128];
        std::snprintf(reply,sizeof reply,"Level %ld%s%s is hidden (%s): type %s %ld",n,title.empty()?"":" ",title.c_str(),kind,
                      input_modernized()?"goto":"level force",n);
        say(reply);return true;
    }
    const std::string title=name(unsigned(n));
    if(title.empty()) {say("Level select unavailable: level names not found");return true;}
    if(const char* refusal=travel_refusal()) {
        char reply[96];std::snprintf(reply,sizeof reply,"Cannot travel now: %s",refusal);
        say(reply);return true;
    }
    pending=n;pending_loads=psx_mod_savestate_loads();
    psx_mod_write_byte(end_code,restart);
    std::fprintf(stderr,"[TTK level] travel to %ld %s\n",n,title.c_str());
    char reply[96];std::snprintf(reply,sizeof reply,"Travelling to %ld %s...",n,title.c_str());
    say(reply);
    close=true;
    return true;
}
}

TTK_REGISTER(register_ttk_level_select, TTK_RANK_LEVEL_SELECT) {
    psx_mod_register_function_entry_plugin("ttk.level.select",0x8002b9f4,ttk::init_hook);
}
