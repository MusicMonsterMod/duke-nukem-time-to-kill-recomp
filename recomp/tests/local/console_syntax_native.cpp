// D26L2: console syntax highlighting, the classifier alone (the game's
// run-time names stubbed). Build: cmake --build build-local --target
// ttk-console-syntax-test, then run build-local/ttk-console-syntax-test.
#include "console_syntax.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace ttk {
static int match(const std::string& w,const char* name,bool prefix) {
    const size_t n=std::strlen(name);
    if(w.size()>n || std::strncmp(w.c_str(),name,w.size()))return 0;
    return w.size()==n ? 1 : prefix ? 2 : 0;
}
static int best(const std::string& w,std::initializer_list<const char*> names,bool prefix) {
    int b=0;for(const char* n:names){const int m=match(w,n,prefix);if(m==1)return 1;if(m>b)b=m;}return b;
}
int console_spawn_name(const std::string& r,bool p) {return best(r,{"desert eagle","desert eagle ammo","red energy crystal","rpg"},p);}
int console_landmark_name(const std::string& w,bool p) {return best(w,{"gag-lava-south","ttk-start"},p);}
int console_option_key(const std::string& w,bool p) {return best(w,{"crosshair","frame_rate"},p);}
int console_option_value(const std::string& k,const std::string& w,bool p) {
    if(k=="crosshair")return best(w,{"on","off","subtle"},p);
    if(k=="frame_rate")return best(w,{"60","120","unlimited"},p);
    return 0;
}
// D26L tab completion: the same stub names as lists.
void console_spawn_names(std::vector<std::string>& o) {o.insert(o.end(),{"desert eagle","desert eagle ammo","red energy crystal","rpg"});}
void console_landmark_names(std::vector<std::string>& o) {o.insert(o.end(),{"gag-lava-south","ttk-start"});}
void console_option_keys(std::vector<std::string>& o) {o.insert(o.end(),{"crosshair","frame_rate"});}
void console_option_values(const std::string& k,std::vector<std::string>& o) {
    if(k=="crosshair")o.insert(o.end(),{"on","off","subtle"});
    if(k=="frame_rate")o.insert(o.end(),{"60","120","unlimited"});
}
}
using ttk::Syn;
// D26L: one Tab on line with the caret at '|' (or the end): the new line with
// '|' at the caret, or "LIST:" and the candidates.
static std::string tab(const char* typed) {
    std::string line=typed;
    int caret=int(line.size());
    const size_t bar=line.find('|');
    if(bar!=std::string::npos){line.erase(bar,1);caret=int(bar);}
    bool list=false;std::vector<std::string> c;
    ttk::console_tab(line,&caret,&list,c);
    if(list){std::string o="LIST:";for(auto& k:c)o+=" "+k;return o;}
    line.insert(size_t(caret),"|");
    return line;
}
static void expect_tab(const char* typed,const char* want);
// One letter per byte: . plain, > prompt, C command, K keyword, 0 number,
// n name, p placeholder, ? unknown, E error, W warning, H heading (D26L4).
static std::string code(const char* text,bool typed,int caret=-1) {
    const int n=int(std::strlen(text));
    std::vector<Syn> s(static_cast<size_t>(n),Syn::plain);
    ttk::console_classify(text,typed,caret,s.data(),n);
    std::string out;
    for(Syn x:s)switch(x) {
        case Syn::plain: out+='.';break; case Syn::prompt: out+='>';break; case Syn::command: out+='C';break;
        case Syn::keyword: out+='K';break; case Syn::number: out+='0';break; case Syn::name: out+='n';break;
        case Syn::placeholder: out+='p';break; case Syn::unknown: out+='?';break; case Syn::error: out+='E';break;
        case Syn::warning: out+='W';break; case Syn::heading: out+='H';break;
    }
    return out;
}
static int failures;
static void expect(const char* text,bool typed,const char* want,int caret=-1) {
    const std::string got=code(text,typed,caret);
    if(got!=want) {
        std::fprintf(stderr,"FAIL %s \"%s\"\n  want %s\n  got  %s\n",typed?"typed":"reply",text,want,got.c_str());
        ++failures;
    }
}
int main() {
    // Typed lines.
    expect("goto room 12 face 95 quiet safe",true,"CCCC.KKKK.00.KKKK.00.KKKKK.KKKK");
    expect("goto gag-lava-south",true,"CCCC.nnnnnnnnnnnnnn");
    expect("goto 3 at 45000 -300 -20600",true,"CCCC.0.KK.00000.0000.000000");
    expect("goto rooom 12",true,"CCCC.?????.00");
    expect("gotoo room",true,"?????.????");
    expect("spawn desert eagle",true,"CCCCC.nnnnnnnnnnnn");
    expect("spawn desert eagel",true,"CCCCC.????????????");
    expect("spawn 761",true,"CCCCC.000");
    expect("spawn enemy 50",true,"CCCCC.KKKKK.00");
    expect("option crosshair subtle",true,"CCCCCC.nnnnnnnnn.nnnnnn");
    expect("option crosshair purple",true,"CCCCCC.nnnnnnnnn.??????");
    expect("option frame_rate 120",true,"CCCCCC.nnnnnnnnnn.nnn");
    expect("lamp style wave:3:0.5",true,"CCCC.KKKKK.KKKK.0.000");
    expect("lamp note any free text",true,"CCCC.KKKK..............");
    expect("sfx 0x100f",true,"CCC.000000");
    expect("testmove 800da978 1.5",true,"CCCCCCCC.00000000.000");
    expect("crouchstandblock",true,"CCCCCCCCCCCCCCCC");
    expect("beamfollow default",true,"CCCCCCCCCC.KKKKKKK");
    expect("help all",true,"CCCC.KKK");
    // The word at the caret is still being typed: not a typo yet.
    expect("got",true,"...",3);
    expect("goto ro",true,"CCCC...",7);
    expect("goto ro",true,"CCCC.??");
    expect("goto -",true,"CCCC..",6);
    expect("spawn desert ea",true,"CCCCC..........",15);
    expect("spawn desert ea",true,"CCCCC.?????????");
    expect("goto gag-la",true,"CCCC.......",11);
    expect("option cross",true,"CCCCCC......",12);
    // Replies.
    expect("> goto rooom 12",false,">>CCCC.?????.00");
    expect("Unknown command: gotoo",false,"EEEEEEEEEEEEEEEEEEEEEE");
    expect("Try: goto room 12, or goto 3 room 12 (goto rooms: how many)",false,
           "WWWWWCCCCWKKKKW00WWWWWCCCCW0WKKKKW00WWCCCCWKKKKKWWWWWWWWWWW");
    expect("  goto room 12            room 12 of this level (goto rooms: how many)",false,
           "..CCCC.KKKK.00...................................CCCC.KKKKK...........");
    expect("  palfrom I R G B  Flash the screen, Duke3D units (alias flash)",false,
           "..CCCCCCC.p.p.p.p..............................................");
    expect("Levels (type level N):",false,"HHHHHHHHHHHHHCCCCCHpHH");
    expect("Usage: beamfollow [on|off|default] (on: energy beams)",false,
           "WWWWWWWCCCCCCCCCCWWKKWKKKWKKKKKKKWWWWWWWWWWWWWWWWWWWW");
    expect("  3  goto room 12",false,"..0..CCCC.KKKK.00");
    expect("Goto: Modernized profile only (Vanilla is unchanged)",false,
           "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");
    expect("Spawn: unknown item - type items for the list",false,
           "EEEEEEEEEEEEEEEEEEEEEEEEEEECCCCCEEEEEEEEEEEEE");
    expect("Travelling to 3 TOXIC WASTE...",false,"..............................");
    expect("Lamp: lamp list | scan [threshold] | signs",false,"HHHHH.CCCC.KKKK...KKKK..ppppppppp....KKKKK");
    expect("  face 90                 face that compass direction",false,"..KKKK.00............................................");
    expect("  quiet                   invisible for 8 seconds",false,"..KKKKK..........................................");
    expect("  on   the light",false,"................");   // a word many commands know: not coloured
    // D26L4: headings and tabbed lists.
    expect("Weapons here:",false,"HHHHHHHHHHHHH");
    expect("OLD WEST\t1 DUKE HILL\t2 MINER 69ER",false,"HHHHHHHH.0.nnnnnnnnn.0.nnnnnnnnnn");
    expect("\t*0 TIME TO KILL\t5 OBEY OR DIE",false,".H0.nnnnnnnnnnnn.0.nnnnnnnnnnn");
    expect("ITEMS\tarmor 320\tkey 1 153",false,"HHHHH.nnnnn.000.nnnnn.000");
    expect("OTHER (TYPE)\t54 155 156",false,"HHHHHHHHHHHH.00.000.000");
    expect("TRAVEL\tgoto help\tPut Duke anywhere",false,"HHHHHH.CCCC.KKKK..................");
    expect("\tlevel N\tTravel to level N",false,".CCCCC.p..................");
    expect("  goto room 12\troom 12 of this level",false,"..CCCC.KKKK.00......................");
    expect("0 TIME TO KILL\tttk-start\tthe level start",false,"HHHHHHHHHHHHHH.nnnnnnnnn................");
    expect("Levels: type level N to travel",false,"HHHHHHH......CCCCC.p..........");
    expect("Renderer: Polymorph.",false,"HHHHHHHHH...........");
    expect("Unknown command: gotoo. Try: goto help (help: every command)",false,
           "EEEEEEEEEEEEEEEEEEEEEEEEEEEEECCCCEKKKKEECCCCEEEEEEEEEEEEEEEE");
    expect("Level 15 MESA is hidden (a 2-player arena): type goto 15",false,
           "WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWCCCCW00");
    // D26L4: a known command with arguments nothing took gives its usage; an
    // unknown word, the nearest command.
    static std::string said;
    auto say=[](const char* l){said+=l;said+='\n';};
    said.clear();assert(ttk::console_usage("warp",say) && said=="Usage: warp x y z [degrees] - Move Duke (warp object N [distance] [degrees])\n");
    said.clear();assert(ttk::console_usage("flash 1 2",say) && said.find("Usage: palfrom I R G B")==0);   // an alias: its command
    said.clear();assert(!ttk::console_usage("zzz",say) && said.empty());
    said.clear();ttk::console_unknown("gotoo room 3",say);assert(said=="Unknown command: gotoo. Try: goto help (help: every command)\n");
    said.clear();ttk::console_unknown("lvels",say);assert(said.find("Try: levels")!=std::string::npos);
    said.clear();ttk::console_unknown("zzz",say);assert(said=="Unknown command: zzz. Type help for the list\n");
    // D26L tab completion.
    expect_tab("got","goto |");                         // one match: the word and a space
    expect_tab("goto ","LIST: at face fix gag-lava-south help here landmarks object quiet room rooms safe ttk-start");
    expect_tab("goto roo","goto room|");                // room / rooms agree on "room"
    expect_tab("goto room","LIST: room rooms");         // nothing more to agree on: list
    expect_tab("goto gag","goto gag-lava-south |");     // a landmark
    expect_tab("goto room 12 fa","goto room 12 face |");
    expect_tab("goto room 12 ","LIST: at face fix gag-lava-south help here landmarks object quiet room rooms safe ttk-start");
    expect_tab("crouchw","crouchwalk |");
    expect_tab("crouch","LIST: crouchgo crouchscan crouchstand crouchwalk");
    expect_tab("he","help |");
    expect_tab("h","LIST: hangpose help history hostvis");
    expect_tab("help a","help all |");
    expect_tab("spawn d","spawn desert eagle|");        // spawn names have spaces
    expect_tab("spawn desert eagle","LIST: desert eagle desert eagle ammo");
    expect_tab("spawn desert eagle a","spawn desert eagle ammo |");
    expect_tab("spawn r","LIST: red energy crystal rpg");
    expect_tab("spawn ene","spawn enem|");              // enemy, enemies
    expect_tab("spawn enemy ","spawn enemy |");         // a type number: nothing
    expect_tab("option c","option crosshair |");
    expect_tab("option crosshair s","option crosshair subtle |");
    expect_tab("option frame_rate ","LIST: 120 60 unlimited");
    expect_tab("option crosshair subtle ","option crosshair subtle |");
    expect_tab("lamp note li","lamp note li|");         // free text
    expect_tab("level f","level force |");
    expect_tab("fps ","fps |");                         // takes nothing
    expect_tab("zzz ","zzz |");                         // unknown command
    expect_tab("go|to","goto|to");                      // mid-word: no space
    expect_tab("got| room","goto |room");               // a space already there: the caret steps over it
    expect_tab("goto  ro","goto  room|");
    {
        static std::string rows;rows.clear();
        std::vector<std::string> c;int from=-1;
        ttk::console_complete("",0,0,&from,c);
        assert(from==0 && int(c.size())==ttk::console_command_count());   // Tab on an empty line: every command
        ttk::console_tab_list(c,[](const char* l){assert(std::strlen(l)<96);rows+=l;rows+='\n';});
        assert(rows[0]=='\t' && rows.find("\tclear\t")!=std::string::npos);
        std::vector<std::string> many(150,"x");
        rows.clear();ttk::console_tab_list(many,[](const char* l){rows+=l;rows+='\n';});
        assert(rows.find("... and 30 more")!=std::string::npos);
    }
    // The listing's echo: the half word is still being typed, not a typo;
    // an echoed command keeps its typo red.
    expect("> goto ro ",false,">>CCCC....");
    expect("> goto ro",false,">>CCCC.??");
    // Candidate rows: commands gold, keywords cyan, names violet.
    expect("\tgoto\troom\tgag-lava-south\tdesert eagle\tcrosshair",false,
           ".CCCC.KKKK.nnnnnnnnnnnnnn.nnnnnnnnnnnn.nnnnnnnnn");
    // The table: every command once, help lists them.
    for(int i=0;i<ttk::console_command_count();++i)
        for(int j=i+1;j<ttk::console_command_count();++j)
            assert(std::strcmp(ttk::console_command_name(i),ttk::console_command_name(j)));
    static int lines;lines=0;
    ttk::console_help([](const char* l){assert(std::strlen(l)<96);++lines;},false);
    const int everyday=lines;lines=0;
    ttk::console_help([](const char* l){assert(std::strlen(l)<96);++lines;},true);
    assert(everyday>10 && lines>everyday);
    // Tints: 0 for plain, a colour otherwise.
    uint32_t t[8]={};
    assert(ttk_console_tints("fps",0,-1,t,3)==1 && t[0]==ttk::console_colour(Syn::command));
    assert(ttk_console_tints("ok",1,-1,t,2)==0 && t[0]==0);
    // Cost: one redraw classifies every visible row (about 25); time 1000 pages.
    {
        const char* rows[]={"  goto room 12            room 12 of this level (goto rooms: how many)",
            "Try: goto room 12, or goto 3 room 12 (goto rooms: how many)","> spawn desert eagle",
            "Usage: beamfollow [on|off|default] (on: energy beams)","Travelling to 3 TOXIC WASTE..."};
        uint32_t tints[96];
        const auto t0=std::chrono::steady_clock::now();
        for(int page=0;page<1000;++page)for(int r=0;r<25;++r){const char* l=rows[r%5];ttk_console_tints(l,1,-1,tints,int(std::strlen(l)));}
        const double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-t0).count()/1000;
        std::printf("classify: %.1f us per 25-row console page\n",us);
        assert(us<2000);
    }
    if(failures){std::fprintf(stderr,"%d failures\n",failures);return 1;}
    std::puts("console syntax: all checks passed");
    return 0;
}
static void expect_tab(const char* typed,const char* want) {
    const std::string got=tab(typed);
    if(got!=want) {std::fprintf(stderr,"FAIL tab \"%s\"\n  want %s\n  got  %s\n",typed,want,got.c_str());++failures;}
}
