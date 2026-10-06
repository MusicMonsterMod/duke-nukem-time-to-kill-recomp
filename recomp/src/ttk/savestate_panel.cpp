// D24B: the F7 savestate slot browser in Time to Kill's own fonts and disc art,
// to the design the user approved in the font picker (2026-10-06). Modernized
// only; Vanilla and a build without the disc packs keep the framework panel
// (psx_savestate_menu.c), which calls this through a registered renderer.
//
// Each slot card shows the savestate thumbnail, "SLOT NN - <level>" (the game's
// own level name), the save time and the level's era. The level is recorded
// beside the slot file (".ttk", one line "level N") when the save succeeds,
// while guest RAM still holds the saved state; slots saved before this show the
// slot number and time only.
#include "ttk_font.h"
#include "level_select.h"
#include "pc_input.h"
#include "mod_plugins.h"
#include "savestate.h"
#include "host_keymap.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

extern "C" void psx_savestate_menu_set_panel_renderer(int (*renderer)(uint32_t*,int,int,int));
namespace {
struct Sprite {int w=0,h=0;std::vector<uint32_t> argb;};
enum {up_down=1,cross=2,square=3,circle=4,triangle=5,radiation=6};
Sprite sprites[8];
bool ui_tried,ui_ok;
uint32_t thumb[SAVESTATE_THUMB_W*SAVESTATE_THUMB_H];

bool load_ui() {
    if(ui_tried)return ui_ok;
    ui_tried=true;
    const char* override_path=std::getenv("DNTTK_UI_PACK");
    std::string path;
    if(override_path)path=override_path;
    else {auto base=SDL_GetBasePath();if(!base)return false;path=std::string(base)+"ttk-ui.pack";}
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<12 || file.tellg()>1024*1024)return false;
    std::vector<unsigned char> data(static_cast<size_t>(file.tellg()));file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(data.data()),data.size()) || std::memcmp(data.data(),"TTKUI1\0\0",8))return false;
    auto u16=[&](size_t o){return unsigned(data[o]|data[o+1]<<8);};
    const unsigned count=data[8]|data[9]<<8|data[10]<<16|unsigned(data[11])<<24;
    size_t o=12;
    for(unsigned n=0;n<count;++n) {
        if(o+6>data.size())return false;
        const unsigned id=u16(o),w=u16(o+2),h=u16(o+4);o+=6;
        if(!w || !h || w>128 || h>128 || o+size_t(w)*h*4>data.size())return false;
        if(id<8) {
            Sprite& s=sprites[id];s.w=int(w);s.h=int(h);s.argb.resize(size_t(w)*h);
            for(size_t p=0;p<s.argb.size();++p)s.argb[p]=data[o+4*p]|data[o+4*p+1]<<8|data[o+4*p+2]<<16|unsigned(data[o+4*p+3])<<24;
        }
        o+=size_t(w)*h*4;
    }
    ui_ok=sprites[cross].w>0 && sprites[radiation].w>0;
    return ui_ok;
}

struct Surface {
    uint32_t* px;int w,h;
    void fill(int x,int y,int rw,int rh,uint32_t c) {
        for(int yy=std::max(0,y);yy<std::min(h,y+rh);++yy)for(int xx=std::max(0,x);xx<std::min(w,x+rw);++xx)px[yy*w+xx]=c;
    }
    void stroke(int x,int y,int rw,int rh,int t,uint32_t c) {fill(x,y,rw,t,c);fill(x,y+rh-t,rw,t,c);fill(x,y,t,rh,c);fill(x+rw-t,y,t,rh,c);}
    void blend(int x,int y,uint32_t c,unsigned alpha) {
        if(x<0||y<0||x>=w||y>=h)return;
        const uint32_t d=px[y*w+x];
        auto mix=[&](int s){return (((c>>s)&255)*alpha+((d>>s)&255)*(255-alpha))/255;};
        px[y*w+x]=0xff000000u|mix(16)<<16|mix(8)<<8|mix(0);
    }
    void sprite(const Sprite& s,int x,int y,int scale,unsigned alpha=255) {
        for(int sy=0;sy<s.h*scale;++sy)for(int sx=0;sx<s.w*scale;++sx) {
            const uint32_t c=s.argb[size_t(sy/scale)*s.w+sx/scale];
            if(!(c>>24))continue;
            if(alpha==255){if(x+sx>=0&&y+sy>=0&&x+sx<w&&y+sy<h)px[(y+sy)*w+x+sx]=c;}
            else blend(x+sx,y+sy,c,alpha);
        }
    }
    int text(int set,const char* s,int x,int y) {return ttk_font_draw(set,s,px,w,h,x,y);}
};

const char* era(unsigned level) {
    if(level==0 || level==5 || level==9)return "PRESENT DAY";
    if(level<=3)return "OLD WEST";
    if(level<=8)return "MEDIEVAL";
    if(level<=12)return "ANCIENT ROME";
    if(level>=21 && level<=26)return "CHALLENGE STAGE";
    if(level>=27 && level<=29)return "BOSS";
    return "";
}

std::string sidecar_path(int slot) {
    char path[1024];
    if(!savestate_slot_path(slot,path,sizeof path))return {};
    return std::string(path)+".ttk";
}
// The recorded level, if this slot was saved with the sidecar and is not older than it.
bool slot_level(int slot,unsigned& level) {
    std::ifstream file(sidecar_path(slot));
    unsigned value=0;std::string word;
    if(!(file>>word>>value) || word!="level" || value>=32)return false;
    level=value;return true;
}

int render(uint32_t* dst,int w,int h,int selected) {
    if(!ttk::input_modernized() || !dst || w<640 || h<480 || ttk_font_line_height(TTK_FONT_PANEL_TITLE)<=0 || !load_ui())return 0;
    Surface s{dst,w,h};
    // Deep steel gradient backdrop with the disc's radiation emblem as a faint watermark.
    for(int y=0;y<h;++y) {
        const double t=double(y)/(h-1);
        const auto lerp=[&](int a,int b){return unsigned(std::lround(a+(b-a)*t));};
        s.fill(0,y,w,1,0xff000000u|lerp(0x14,0x0a)<<16|lerp(0x16,0x0b)<<8|lerp(0x1c,0x0f));
    }
    s.sprite(sprites[radiation],w-300,90,4,26);
    // Header.
    s.fill(0,0,w,58,0xff1b1e26u);s.fill(0,58,w,2,0xff3a4150u);
    s.text(TTK_FONT_PANEL_TITLE,"SAVE STATES",22,18);
    char key[32]="",line[96];
    host_keymap_label(HOST_KEYMAP_SAVE_STATE_MENU,key,sizeof key);
    std::snprintf(line,sizeof line,"%s MENU",key[0]?key:"F7");
    s.text(TTK_FONT_PANEL_DIM,line,w-24-8*int(std::strlen(line)),18);
    int first=std::clamp(selected-1,0,SAVESTATE_SLOTS-3);
    std::snprintf(line,sizeof line,"%02d-%02d / %02d",first+1,first+3,SAVESTATE_SLOTS);
    s.text(TTK_FONT_PANEL_DIM,line,w-24-8*int(std::strlen(line)),34);
    // Three slot cards.
    for(int row=0;row<3;++row) {
        const int slot=first+row,y=74+row*108;
        const bool on=slot==selected;
        s.fill(24,y,592,98,on?0xff1d2230u:0xff15181fu);
        s.stroke(24,y,592,98,2,on?0xff5fa8ffu:0xff2c323eu);
        if(on)s.fill(24,y,4,98,0xff5fa8ffu);
        // 4:3 thumbnail, 104x78.
        if(savestate_read_thumb(slot,thumb,SAVESTATE_THUMB_W,SAVESTATE_THUMB_H)) {
            for(int ty=0;ty<78;++ty)for(int tx=0;tx<104;++tx)
                s.fill(38+tx,y+10+ty,1,1,0xff000000u|thumb[(ty*SAVESTATE_THUMB_H/78)*SAVESTATE_THUMB_W+tx*SAVESTATE_THUMB_W/104]);
            s.stroke(37,y+9,106,80,1,0xff3a4150u);
        } else {
            s.fill(38,y+10,104,78,0xff242a35u);s.stroke(38,y+10,104,78,1,0xff3a4150u);
            s.text(TTK_FONT_PANEL_DIM,"EMPTY",38+52-20,y+45);
        }
        unsigned level=0;
        char title[96]="",name[48]="";
        const bool known=savestate_slot_exists(slot) && slot_level(slot,level) && ttk::level_title(level,name,sizeof name);
        if(known)std::snprintf(title,sizeof title,"SLOT %02d - %s",slot+1,name);
        else std::snprintf(title,sizeof title,"SLOT %02d",slot+1);
        s.text(on?TTK_FONT_PANEL_SLOT_SELECTED:TTK_FONT_PANEL_SLOT,title,160,y+14);
        int64_t stamp=0;
        if(savestate_slot_mtime(slot,&stamp)) {
            std::time_t t=std::time_t(stamp);std::tm tm{};
#ifdef _WIN32
            localtime_s(&tm,&t);
#else
            localtime_r(&t,&tm);
#endif
            std::strftime(line,sizeof line,"%Y-%m-%d %H:%M",&tm);
        } else std::snprintf(line,sizeof line,"NEW SLOT");
        s.text(TTK_FONT_PANEL_TEXT,line,162,y+42);
        if(known) {
            std::snprintf(line,sizeof line,"LEVEL %u  %s",level,era(level));
            s.text(TTK_FONT_PANEL_DIM,line,162,y+58);
        }
    }
    // Prompts: the disc's button sprites with pause-menu style labels.
    s.fill(0,412,w,68,0xff1b1e26u);s.fill(0,412,w,2,0xff3a4150u);
    int x=26;
    const struct {int sprite;const char* label;} prompts[]={{up_down,"SLOT"},{cross,"LOAD"},{square,"SAVE"},{circle,"BACK"}};
    for(auto p:prompts) {
        s.sprite(sprites[p.sprite],x,428,1);x+=20;
        const int drawn=s.text(TTK_FONT_PANEL_SLOT,p.label,x,430);
        x+=std::max(0,drawn)+22;
    }
    s.text(TTK_FONT_PANEL_DIM,"KEYS: ARROWS SLOT  ENTER/L LOAD  SHIFT+ENTER/S SAVE  ESC BACK",26,462);
    return 1;
}
}

// Called by the framework after a successful save, while guest RAM still holds
// the saved state: record the level beside the slot (gameplay only).
extern "C" void ttk_savestate_saved(int slot) {
    const std::string path=sidecar_path(slot);
    if(path.empty())return;
    std::remove(path.c_str());
    if(psx_mod_read_half(0x800bcbb0)!=1)return; // not in gameplay: no level to record
    const unsigned level=psx_mod_read_word(0x800be570);
    if(level>=32)return;
    std::ofstream(path)<<"level "<<level<<"\n";
}

PSX_MOD_CONSTRUCTOR(register_ttk_savestate_panel) {
    psx_savestate_menu_set_panel_renderer(render);
}
