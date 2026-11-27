// D26L2: console syntax highlighting. The one table of console commands and
// the words each knows; the highlighter, `help` and (later) tab completion read
// it, so they cannot drift apart. tests/local/test_console_syntax.py checks it
// against every command the handlers parse. Contract:
// documentation/567-d26l2-console-syntax-highlighting.md.
#include "console_syntax.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace ttk {
namespace {
enum : unsigned {
    family=1,    // the first word only begins with the name (crouchstand...)
    hex_args=2,  // bare hex numbers (testmove's actor address)
    note_text=4, // free text after "note" (lamp note ...)
    spawn_rest=8,// the rest of the line is one item name (spawn ...)
    landmarks=16,// a landmark name (goto NAME)
    option_kv=32,// option KEY VALUE
};
struct Command {
    const char* name;     // first word
    const char* words;    // the later words it knows, space separated
    unsigned flags;
    int tier;             // 0: help lists it, 1: help all, 2: an alias (never listed)
    const char* usage;    // help's left column
    const char* summary;  // help's right column
};
constexpr const char* toggle="on off default";
// Keep this table in step with the handlers (src/ttk/*.inc, *.cpp and the
// framework built-ins in main.cpp); test_console_syntax.py fails otherwise.
const Command table[]={
    // Framework built-ins (main.cpp debug_console_run).
    {"clear","",0,0,"clear","Clear the console (PgUp/PgDn, wheel: scroll)"},
    {"fps","",0,0,"fps","Toggle FPS statistics"},
    {"help","all",0,0,"help [all]","These commands; help all adds the developer ones"},
    {"history","",0,0,"history","List typed commands (Up/Down recall them)"},
    {"quit","",0,0,"quit","Exit the game"},
    // level_select.cpp
    {"endlevel","",0,0,"endlevel","Finish the level (statistics screen)"},
    {"levels","",0,0,"levels","List levels for level select"},
    {"level","force",0,0,"level N","Travel to level N (level force N: any index 0-31)"},
    {"quicksave","",0,0,"quicksave","Save over the last slot used (F6)"},
    {"quickload","",0,0,"quickload","Load the last slot used (F9)"},
    // spawn.inc, goto.inc, far_hits.inc
    {"goto","help landmarks here fix rooms room object at face quiet safe",landmarks,0,"goto help",
     "Put Duke anywhere: rooms, objects, points, landmarks"},
    {"items","",0,0,"items","List items spawn can create here"},
    {"spawn","enemy enemies",spawn_rest,0,"spawn X","Spawn item X (name or type) in front of Duke"},
    {"enemies","",0,0,"enemies","List enemies spawn enemy can create here"},
    {"warp","object",0,1,"warp x y z [degrees]","Move Duke (warp object N [distance] [degrees])"},
    {"farhits","probe on off default",0,1,"farhits [on|off|default]","Duke's rounds reach the extended view"},
    // game_menu.cpp
    {"option","",option_kv,0,"option KEY VALUE","Set one OPTIONS choice for this session"},
    // screen_flash.cpp
    {"palfrom","",0,0,"palfrom I R G B","Flash the screen, Duke3D units (alias flash)"},
    {"flash","",0,2,"flash I R G B","palfrom"},
    // frame_replay.cpp
    {"renderer","polymorph classic",0,0,"renderer R","Switch renderer: polymorph or classic (this session)"},
    {"redrawonly",toggle,0,1,"redrawonly [on|off|default]","Frame redraw lab"},
    {"boltbarrel",toggle,0,1,"boltbarrel [on|off|default]","Crossbow bolts leave the barrel"},
    // earthquake.inc, dynamic_lights.inc, light_shadows.inc, blast_audio.inc
    {"earthquake","",0,1,"earthquake [seconds]","Shake the level (0 to 600 seconds)"},
    {"light","off bake on default mode enhanced relit original budget smooth fire bolts shadows ray moving "
     "sharp simple reference at",0,1,"light [r g b [radius]]","Test light at Duke (light off; light mode ...)"},
    {"chainsound",toggle,0,1,"chainsound [on|off|default]","Chained blast sounds"},
    // level_lights.inc, club_lights.inc, relit.inc
    {"lamp","help off on list crystals pickups lava props breaks gate emitters fixtures all ambient add here pick "
     "move nudge color colour radius power style note del save reload clear scan signs bind unbind club apartment "
     "steady flicker pulse candle strobe broken wave signal duke unlit outdoor room reach lamps",note_text,1,
     "lamp help","Level lights editor"},
    // steroids_beat.inc
    {"sfx","",0,1,"sfx ID","Play a sound effect (bank<<12|index, e.g. sfx 0x100f)"},
    // crouch_hull.inc
    {"crouchstand","block free",family,1,"crouchstand [block|free]","Crouch stand-up test"},
    {"crouchwalk","off",0,1,"crouchwalk [fwd side height]","Crouch walk test (crouchwalk off)"},
    {"crouchgo","off",0,2,"crouchgo","crouchwalk"},
    {"crouchscan","",0,1,"crouchscan [radius [step]]","Crouch hull scan"},
    // first_person.inc, console_idle.inc, ladder_armed_hang.inc
    {"fov","default",0,1,"fov [60..120|default]","First-person field of view"},
    {"fpidle","inspect heft rest pose off",0,1,"fpidle [inspect|heft|rest]","First-person idle gesture"},
    {"hangpose","hang side pull off tune",0,1,"hangpose [1|2|3|off]","Armed ladder hang pose (hangpose tune ...)"},
    // weapon_aim.cpp
    {"beamfollow",toggle,0,1,"beamfollow [on|off|default]","Energy beams follow the gun"},
    {"beamanim",toggle,0,1,"beamanim [on|off|default]","Energy beam animation per redraw"},
    {"boltlaunch",toggle,0,1,"boltlaunch [on|off|default]","Crossbow bolt launch"},
    {"pointblank","default",0,1,"pointblank [lift [max]|default]","Point-blank aim"},
    {"shotview",toggle,0,1,"shotview [on|off|default]","Shots from the view"},
    {"shootseen",toggle,0,1,"shootseen [on|off|default]","Shoot what is seen"},
    {"roundfollow",toggle,0,1,"roundfollow [on|off|default]","Rounds follow the redraw"},
    {"autoaim","default",0,1,"autoaim [1-256|default]","Auto-aim reach"},
    {"testmove","off late",hex_args,1,"testmove ACTOR SPEED [span [late]]","Move an actor (testmove off)"},
    // host_visibility.inc, near_clip.cpp
    {"hostvis","on off default walls region all",0,1,"hostvis [on|off|default]","Host visibility (hostvis walls ...)"},
    {"smoothquads","on off default min",0,1,"smoothquads [on|off|default]","Smooth quads (smoothquads min N)"},
};
constexpr int table_n=int(sizeof table/sizeof table[0]);

bool has_word(const char* words,const std::string& w,bool prefix) {
    if(w.empty())return false;
    for(const char* p=words;*p;) {
        while(*p==' ')++p;
        const char* e=p;while(*e && *e!=' ')++e;
        const size_t n=size_t(e-p);
        if(n && (n==w.size() || (prefix && n>w.size())) && !std::strncmp(p,w.c_str(),w.size()))return true;
        p=e;
    }
    return false;
}
const Command* find_command(const std::string& w,bool prefix,bool* partial) {
    if(partial)*partial=false;
    for(const Command& c:table)
        if(w==c.name || ((c.flags&family) && !w.compare(0,std::strlen(c.name),c.name)))return &c;
    if(prefix)for(const Command& c:table)
        if(w.size()<std::strlen(c.name) && !std::strncmp(c.name,w.c_str(),w.size())) {if(partial)*partial=true;return nullptr;}
    return nullptr;
}
bool is_number(const std::string& w,bool hex) {
    if(w.empty())return false;
    size_t i=0;
    if(w.size()>2 && w[0]=='0' && (w[1]=='x' || w[1]=='X')) {
        for(i=2;i<w.size();++i)if(!std::isxdigit((unsigned char)w[i]))return false;
        return true;
    }
    if(hex) {bool digit=false;for(char c:w){if(!std::isxdigit((unsigned char)c))goto decimal;digit|=std::isdigit((unsigned char)c)!=0;}
             if(digit)return true;}
decimal:
    if(w[0]=='+' || w[0]=='-')++i;
    bool digits=false,dot=false;
    for(;i<w.size();++i) {
        const char c=w[i];
        if(std::isdigit((unsigned char)c))digits=true;
        else if(c=='.' && !dot)dot=true;
        else if(c==':' && digits && i+1<w.size())  // 16:9
            {for(++i;i<w.size();++i)if(!std::isdigit((unsigned char)w[i]))return false;return true;}
        else if((c=='e' || c=='E') && digits && i+1<w.size()) {
            ++i;if(w[i]=='+' || w[i]=='-')++i;
            if(i>=w.size())return false;
            for(;i<w.size();++i)if(!std::isdigit((unsigned char)w[i]))return false;
            return true;
        } else return false;
    }
    return digits;
}
// Half a number while it is typed: "-", "+", "1.", "0x".
bool number_start(const std::string& w) {
    return w=="-" || w=="+" || w=="0x" || w=="." || (w.size()>1 && w.back()=='.' && is_number(w.substr(0,w.size()-1),false));
}
struct Word {int a,b;std::string s;};
std::vector<Word> words_of(const char* text,int from,int to) {
    std::vector<Word> out;
    for(int i=from;i<to;) {
        while(i<to && (text[i]==' ' || text[i]=='\t'))++i;   // D26L4: tabbed replies
        if(i>=to)break;
        int e=i;while(e<to && text[e]!=' ' && text[e]!='\t')++e;
        out.push_back({i,e,std::string(text+i,size_t(e-i))});
        i=e;
    }
    return out;
}
void fill(Syn* out,int n,int a,int b,Syn s) {for(int i=std::max(a,0);i<b && i<n;++i)out[i]=s;}

// A typed command line from byte `from`: word by word, as the handlers read it.
void classify_command(const char* text,int from,int to,int caret,Syn* out,int n) {
    const std::vector<Word> w=words_of(text,from,to);
    if(w.empty())return;
    auto typing=[&](const Word& x){return caret>=0 && caret==x.b;};
    bool partial=false;
    const Command* c=find_command(w[0].s,typing(w[0]),&partial);
    if(!c) {fill(out,n,w[0].a,w[0].b,partial?Syn::plain:Syn::unknown);
            for(size_t i=1;i<w.size();++i)fill(out,n,w[i].a,w[i].b,Syn::unknown);
            return;}
    fill(out,n,w[0].a,w[0].b,Syn::command);
    if((c->flags&spawn_rest) && w.size()>1 && !has_word(c->words,w[1].s,false)) {
        // spawn <name>: the whole rest is one name, or a type number.
        const Word& last=w.back();
        const std::string rest(text+w[1].a,size_t(last.b-w[1].a));
        if(w.size()==2 && is_number(w[1].s,false)) {fill(out,n,w[1].a,w[1].b,Syn::number);return;}
        const int got=console_spawn_name(rest,typing(last));
        fill(out,n,w[1].a,last.b,got==1?Syn::name:got==2?Syn::plain:Syn::unknown);
        return;
    }
    for(size_t i=1;i<w.size();++i) {
        const Word& x=w[i];
        const bool t=typing(x);
        Syn s=Syn::unknown;
        if(has_word(c->words,x.s,false))s=Syn::keyword;
        else if((c->flags&option_kv) && i==1 && console_option_key(x.s,t)==1)s=Syn::name;
        else if((c->flags&option_kv) && i==2 && console_option_value(w[1].s,x.s,t)==1)s=Syn::name;
        else if((c->flags&option_kv) && i==2)s=Syn::unknown;   // a value that row does not have, even 999
        else if(is_number(x.s,(c->flags&hex_args)!=0))s=Syn::number;
        else if((c->flags&landmarks) && console_landmark_name(x.s,t)==1)s=Syn::name;
        else if((c->flags&note_text) && i>1 && w[i-1].s=="note") {fill(out,n,x.a,w.back().b,Syn::plain);return;}
        else if(c->flags&note_text && x.s.find(':')!=std::string::npos) {
            // lamp style wave:phase:depth, duke:hitag:on:off
            s=Syn::keyword;
            for(int a=x.a;a<x.b;) {
                int e=a;while(e<x.b && text[e]!=':')++e;
                const std::string part(text+a,size_t(e-a));
                fill(out,n,a,e,has_word(c->words,part,false)?Syn::keyword:is_number(part,false)?Syn::number:Syn::unknown);
                a=e+1;
            }
            continue;
        }
        if(s==Syn::unknown && t) {
            // Still being typed: not a typo while it can still become a word.
            const bool may=has_word(c->words,x.s,true) || number_start(x.s) ||
                ((c->flags&landmarks) && console_landmark_name(x.s,true)) ||
                ((c->flags&option_kv) && i==1 && console_option_key(x.s,true)) ||
                ((c->flags&option_kv) && i==2 && console_option_value(w[1].s,x.s,true));
            if(may)s=Syn::plain;
        }
        fill(out,n,x.a,x.b,s);
    }
}

bool starts(const char* text,const char* p) {return !std::strncmp(text,p,std::strlen(p));}
bool contains(const char* text,const char* p) {return std::strstr(text,p)!=nullptr;}
// The reply's own colour from its wording (the handlers' shared phrases).
Syn reply_tone(const char* t) {
    while(*t==' ')++t;
    static const char* const errors[]={"Unknown ","Cannot ","Can't ","No such","Already "};
    for(const char* e:errors)if(starts(t,e))return Syn::error;
    static const char* const error_parts[]={" unavailable","profile only","Modernized only","Modernized gameplay only",
        "not understood","no such","not one of","not a live","not ready","unknown ","start or load a game first",
        "is dying","not loaded","no landmark","need the","needs the","no free object","only in play","whole numbers only",
        "One-player games only","already ending","Close the pause menu first","where to?"};
    for(const char* e:error_parts)if(contains(t,e))return Syn::error;
    static const char* const warnings[]={"Usage:","Try:","Note:"};
    for(const char* w:warnings)if(starts(t,w) || contains(t,w))return Syn::warning;
    if(contains(t,"is off in OPTIONS") || contains(t,"is hidden:") || contains(t,"is hidden ("))return Syn::warning;
    return Syn::plain;
}
bool all_caps(const std::string& w) {
    bool letter=false;
    for(char c:w){if(std::islower((unsigned char)c))return false;letter|=std::isupper((unsigned char)c)!=0;}
    return letter;
}
// One reply word inside a command span: its pieces split at | [ ] < > (help
// writes [on|off|default], <type>, [r g b [radius]]). Returns false when the
// word does not belong to the command, which ends the span.
bool span_word(const char* text,const Word& x,const Command& c,int& depth,Syn* out,int n) {
    const char* const brackets="[]<>|";
    bool any=false,bad=false;
    std::vector<std::pair<int,int>> pieces;
    int d=depth;
    for(int a=x.a;a<x.b;) {
        if(std::strchr(brackets,text[a])) {if(text[a]=='[' || text[a]=='<')++d;else if(text[a]==']' || text[a]=='>')--d;++a;continue;}
        int e=a;while(e<x.b && !std::strchr(brackets,text[e]))++e;
        const std::string part(text+a,size_t(e-a));
        Syn s;
        if(has_word(c.words,part,false))s=Syn::keyword;
        else if(is_number(part,false))s=Syn::number;
        else if(d>0 || all_caps(part) || (part.size()==1 && std::islower((unsigned char)part[0])))s=Syn::placeholder;
        else if((c.flags&landmarks) && console_landmark_name(part,false)==1)s=Syn::name;
        else {bad=true;break;}
        pieces.push_back({a,e});fill(out,n,a,e,s);any=true;
        a=e;
    }
    if(bad || !any) {for(auto& p:pieces)fill(out,n,p.first,p.second,Syn::plain);return false;}
    depth=std::max(d,0);
    return true;
}
// Colours command examples inside a reply: a known command at the line's
// start or after Try: / type / or / Also: / ( / , / : / |, then the words
// that command knows, up to a column gap (two spaces) or other text.
void classify_examples(const char* text,int len,Syn* out,int n) {
    const std::vector<Word> w=words_of(text,0,len);
    static const char* const triggers[]={"try","type","or","also","usage","e.g.","then","and"};
    for(size_t i=0;i<w.size();++i) {
        // Where may a command example start?
        std::string core=w[i].s;
        int lead=0;
        while(lead<int(core.size()) && (core[size_t(lead)]=='(' || core[size_t(lead)]=='\''))++lead;
        bool at=i==0 || lead>0 || (i>0 && std::memchr(text+w[i-1].b,'\t',size_t(w[i].a-w[i-1].b)));   // a cell's start
        if(!at && i>0) {
            std::string prev=w[i-1].s;
            const char tail=prev.back();
            if(tail==',' || tail==':' || tail==';' || prev=="|")at=true;
            while(!prev.empty() && std::strchr(",:;.",prev.back()))prev.pop_back();
            while(!prev.empty() && prev[0]=='(')prev.erase(0,1);
            for(char& ch:prev)ch=char(std::tolower((unsigned char)ch));
            for(const char* t:triggers)if(prev==t)at=true;
        }
        if(!at)continue;
        core=core.substr(size_t(lead));
        size_t cut=core.size();
        while(cut && std::strchr(",:;.)'",core[cut-1]))--cut;
        const Command* c=find_command(core.substr(0,cut),false,nullptr);
        bool extra=false;
        if(!c && i==0 && w[0].a>=2 && cut==core.size()) {
            // An indented help row that starts with a word only one command
            // knows (goto help's "  face 90", "  quiet"): that command's extra.
            for(const Command& k:table)if(has_word(k.words,core,false)){if(c){c=nullptr;break;}c=&k;}
            extra=c!=nullptr;
        }
        if(!c || core.empty() || std::isupper((unsigned char)core[0]))continue;
        const int a=w[i].a+lead;
        fill(out,n,a,a+int(cut),extra?Syn::keyword:Syn::command);
        if(cut<core.size())continue;   // "goto," "(levels)" end here
        int depth=0;
        size_t j=i+1;
        for(;j<w.size();++j) {
            if(w[j].a-w[j-1].b>=2 || std::memchr(text+w[j-1].b,'\t',size_t(w[j].a-w[j-1].b)))break;   // a column gap
            if(w[j].s=="|") continue;           // lamp list | scan | signs
            std::string s=w[j].s;size_t trim=s.size();
            while(trim && std::strchr(",:;.)'",s[trim-1]) && !(s[trim-1]==')' && depth))--trim;
            Word x{w[j].a,w[j].a+int(trim),s.substr(0,trim)};
            if(!trim || !span_word(text,x,*c,depth,out,n))break;
            if(trim<s.size()){++j;break;}       // punctuation ends the example
        }
        i=j-1;
    }
}
}

void console_classify(const char* text,bool typed,int caret,Syn* out,int n) {
    for(int i=0;i<n;++i)out[i]=Syn::plain;
    if(!text)return;
    const int len=std::min(n,int(std::strlen(text)));
    if(typed) {classify_command(text,0,len,caret,out,n);return;}
    // The echoed command: "> " then the line as typed (a typo stays red).
    if(starts(text,"> ")) {fill(out,n,0,2,Syn::prompt);classify_command(text,2,len,-1,out,n);return;}
    // history rows: "  3  goto room 12".
    {int i=0;while(i<len && text[i]==' ')++i;int d=i;while(d<len && std::isdigit((unsigned char)text[d]))++d;
     if(d>i && d+2<len && text[d]==' ' && text[d+1]==' ' && text[d+2]!=' ' && std::islower((unsigned char)text[d+2])) {
         fill(out,n,i,d,Syn::number);classify_command(text,d+2,len,-1,out,n);return;}}
    if(starts(text,"Unknown command: ")) {
        fill(out,n,0,len,Syn::error);
        // D26L4: the suggestion after "Try:" is coloured as an example; the
        // typed word before it stays error.
        if(const char* t=std::strstr(text,"Try: ")) {
            const int off=int(t-text);
            std::vector<Syn> ex(size_t(n),Syn::plain);
            classify_examples(text+off,len-off,ex.data()+off,n-off);
            for(int i=off;i<len;++i)if(ex[size_t(i)]!=Syn::plain)out[i]=ex[size_t(i)];
        }
        return;
    }
    const Syn tone=reply_tone(text);
    fill(out,n,0,len,tone);
    // D26L4: a reply's heading: an unindented line ending in ':' ("Weapons
    // here:"), or the label cell that starts an unindented tabbed row
    // ("OLD WEST<tab>1 DUKE HILL<tab>...").
    const char* tab=static_cast<const char*>(std::memchr(text,'\t',size_t(len)));
    if(tone==Syn::plain && text[0]!=' ' && text[0]!='\t') {
        int e=len;while(e>0 && text[e-1]==' ')--e;
        if(tab)fill(out,n,0,int(tab-text),Syn::heading);
        else if(e>0 && text[e-1]==':')fill(out,n,0,len,Syn::heading);
        else {
            // A label of one or two capitalised words before ':' ("Levels:",
            // "Hidden levels:", "Renderer:").
            int c=0;while(c<len && c<24 && text[c]!=':' && (std::isalpha((unsigned char)text[c]) || text[c]==' ' || text[c]=='-'))++c;
            int spaces=0;for(int i=0;i<c;++i)spaces+=text[i]==' ';
            if(c>1 && c<len && text[c]==':' && spaces<=1 && std::isupper((unsigned char)text[0]))fill(out,n,0,c+1,Syn::heading);
        }
    }
    std::vector<Syn> ex(size_t(n),Syn::plain);
    classify_examples(text,len,ex.data(),n);
    for(int i=0;i<len;++i)if(ex[size_t(i)]!=Syn::plain)out[i]=ex[size_t(i)];
    // D26L4: the entries of a tabbed list (every cell after the first): a
    // cell of a number and a name ("12 BLOOD BATHS", "*0 TIME TO KILL",
    // "armor 320") shows the number green and the name violet.
    if(tone!=Syn::plain || !tab)return;
    for(int a=int(tab-text)+1;a<len;) {
        int b=a;while(b<len && text[b]!='\t')++b;
        bool numbers=b>a;   // "54 155 156": a cell of numbers only
        for(int i=a;i<b;++i)numbers&=std::isdigit((unsigned char)text[i]) || text[i]==' ';
        if(numbers) {for(int i=a;i<b;++i)if(text[i]!=' ')out[i]=Syn::number;a=b+1;continue;}
        int x=a;if(x<b && text[x]=='*') {fill(out,n,a,a+1,Syn::heading);++x;}   // this level
        int d=x;while(d<b && std::isdigit((unsigned char)text[d]))++d;
        if(d>x && d+1<b && text[d]==' ') {fill(out,n,x,d,Syn::number);fill(out,n,d+1,b,Syn::name);}
        else if(b>a && console_landmark_name(std::string(text+a,size_t(b-a)),false)==1)fill(out,n,a,b,Syn::name);
        else {
            int e=b;while(e>a && text[e-1]==' ')--e;
            int f=e;while(f>a && std::isdigit((unsigned char)text[f-1]))--f;
            if(f<e && f-1>a && text[f-1]==' ') {fill(out,n,a,f-1,Syn::name);fill(out,n,f,e,Syn::number);}
        }
        a=b+1;
    }
}

uint32_t console_colour(Syn s) {
    switch(s) {
    case Syn::prompt: return 0xff7c8ca4u;      // dim steel
    case Syn::command: return 0xfff4bc48u;     // TTK gold (the caret's)
    case Syn::keyword: return 0xff62d4ecu;     // cyan
    case Syn::number: return 0xff9ee27au;      // green
    case Syn::name: return 0xffe0a4ffu;        // violet
    case Syn::placeholder: return 0xff8c9cd0u; // dim periwinkle
    case Syn::unknown: return 0xffff5c4cu;     // red
    case Syn::error: return 0xffff8c7cu;       // soft red
    case Syn::warning: return 0xffffcc70u;     // amber
    case Syn::heading: return 0xfff4f8ffu;     // near white (D26L4)
    case Syn::plain: break;
    }
    return 0u;
}

// D26L4: help in sections, one aligned block of tabbed rows (section, usage,
// summary). A command no section names is listed under OTHER.
struct Section {const char* label;const char* names;};
const Section sections[]={
    {"TRAVEL","goto levels level endlevel quicksave quickload"},
    {"SPAWN","spawn items enemies"},
    {"GAME","option renderer palfrom fps"},
    {"CONSOLE","help history clear quit"},
    {"WORLD LABS","warp farhits earthquake light chainsound lamp sfx"},
    {"AIM LABS","autoaim pointblank shotview shootseen roundfollow beamfollow beamanim boltlaunch boltbarrel testmove"},
    {"DUKE LABS","fov fpidle hangpose crouchstand crouchwalk crouchscan"},
    {"RENDER LABS","redrawonly hostvis smoothquads"},
};
void console_help(void (*say)(const char*),bool all) {
    say(all ? "Commands, with the developer labs (help: the everyday ones):"
            : "Commands (help all: the developer labs too):");
    std::vector<bool> shown(size_t(table_n),false);
    auto listed=[&](const Command& c){return c.tier==0 || (all && c.tier==1);};
    auto row=[&](const char* label,bool& first,const Command& c) {
        char line[192];
        std::snprintf(line,sizeof line,"%s\t%s\t%s",first?label:"",c.usage,c.summary);
        say(line);first=false;
    };
    for(const Section& sec:sections) {
        bool first=true;
        for(const char* p=sec.names;*p;) {
            while(*p==' ')++p;
            const char* e=p;while(*e && *e!=' ')++e;
            const std::string w(p,size_t(e-p));p=e;
            for(int i=0;i<table_n;++i)
                if(!shown[size_t(i)] && w==table[i].name && listed(table[i])) {shown[size_t(i)]=true;row(sec.label,first,table[i]);}
        }
    }
    bool first=true;
    for(int i=0;i<table_n;++i)if(!shown[size_t(i)] && listed(table[i]))row("OTHER",first,table[i]);
    say("Paste: Shift+Insert, Ctrl+V or right-click. Selecting copies; Ctrl+C copies the line");
}
// D26L4: a known command whose arguments no handler took: its usage, so it
// never answers "Unknown command" (warp, option, sfx alone, quicksave x).
bool console_usage(const char* line,void (*say)(const char*)) {
    if(!line)return false;
    const char* e=line;while(*e && *e!=' ')++e;
    const Command* c=find_command(std::string(line,size_t(e-line)),false,nullptr);
    if(!c)return false;
    while(c->tier==2) {   // an alias: its summary names the command
        const Command* to=find_command(c->summary,false,nullptr);
        if(!to || to==c)break;
        c=to;
    }
    char reply[192];
    std::snprintf(reply,sizeof reply,"Usage: %s - %s",c->usage,c->summary);
    say(reply);
    return true;
}
// D26L4: an unknown first word: the nearest listed command (edit distance
// up to 2, or a word that begins one), else a pointer to help.
void console_unknown(const char* line,void (*say)(const char*)) {
    const char* e=line;while(*e && *e!=' ')++e;
    const std::string w(line,size_t(e-line));
    auto distance=[](const std::string& a,const std::string& b) {
        std::vector<size_t> d(b.size()+1);
        for(size_t j=0;j<=b.size();++j)d[j]=j;
        for(size_t i=1;i<=a.size();++i) {
            size_t diag=d[0];d[0]=i;
            for(size_t j=1;j<=b.size();++j) {
                const size_t up=d[j];
                d[j]=std::min({d[j]+1,d[j-1]+1,diag+(a[i-1]==b[j-1]?0:1)});
                diag=up;
            }
        }
        return d[b.size()];
    };
    const Command* best=nullptr;size_t best_d=3;
    for(const Command& c:table) {
        if(c.tier==2)continue;
        const std::string name=c.name;
        size_t d=distance(w,name);
        if(w.size()>=3 && !name.compare(0,w.size(),w))d=std::min<size_t>(d,1);
        if(d<best_d){best_d=d;best=&c;}
    }
    char reply[192];
    if(best && w.size()>=2)std::snprintf(reply,sizeof reply,"Unknown command: %s. Try: %s (help: every command)",w.c_str(),best->usage);
    else std::snprintf(reply,sizeof reply,"Unknown command: %s. Type help for the list",w.c_str());
    say(reply);
}
int console_command_count() {return table_n;}
const char* console_command_name(int i) {return i>=0 && i<table_n ? table[i].name : nullptr;}
}

extern "C" int ttk_console_tints(const char* text,int kind,int caret,uint32_t* tints,int n) {
    if(!text || !tints || n<=0)return 0;
    std::vector<ttk::Syn> s(static_cast<size_t>(n),ttk::Syn::plain);
    ttk::console_classify(text,kind==0,caret,s.data(),n);
    bool any=false;
    for(int i=0;i<n;++i){tints[i]=ttk::console_colour(s[size_t(i)]);any|=tints[i]!=0;}
    return any;
}
