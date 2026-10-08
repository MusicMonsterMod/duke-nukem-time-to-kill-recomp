#include "inventory_hud.h"
#include "ttk_font.h"
#include "pc_input.h"
#include "mod_plugins.h"
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
namespace {
constexpr int capacity=1024, max_height=160;
uint32_t pixels[capacity*max_height];
uint16_t flags[6],charge_capacity[6];int16_t amount[6];unsigned selected;
uint64_t epoch,sequence,expires;
constexpr unsigned order[]={5,4,1,2,3}; // D08A4: steroids after the medkit, as in EDuke32
struct Sprite {int w,h;std::vector<uint32_t> rgba;};
Sprite icons[6], cursor, digits[10], percent_glyph;
bool icons_loaded, digits_loaded;
// D08A5 mission row (approved design E). The level's mission items are the
// original Select inventory's own list: its name function 0x80087d4c returns a
// name for inventory item i (flag halfword player+0x354+4*i, bit 0 = held) per
// level, or its "none" string. Decoded for levels 0-31; only these have any.
struct MissionDef {unsigned level,item;const char* name;};
constexpr MissionDef mission_table[]={
    {0,6,"SUBWAY SECURITY KEY"},{0,7,"TRANSPORT ROOM ID"},{0,11,"RED ENERGY CRYSTAL"},
    {0,12,"BLUE ENERGY CRYSTAL"},{0,13,"GREEN ENERGY CRYSTAL"},
    {1,6,"SKELETON KEY"},{1,14,"SCRAP OF PAPER"},{1,15,"OLD NOTE"},{1,16,"TORN PAPER"},
    {2,6,"SKELETON KEY"},{2,7,"SKELETON KEY"},
    {3,6,"SKELETON KEY"},
    {5,6,"WAREHOUSE KEY"},{5,11,"RED ENERGY CRYSTAL"},{5,12,"BLUE ENERGY CRYSTAL"},{5,13,"GREEN ENERGY CRYSTAL"},
    {6,7,"SKELETON KEY"},{6,8,"SKELETON KEY"},{6,14,"FAMILY JEWEL"},{6,15,"FAMILY JEWEL"},{6,16,"FAMILY JEWEL"},
    {7,6,"GANTRY KEY"},{7,7,"VALVE KEY"},
    {9,6,"LAB KEY"},{9,7,"VALVE ROOM KEY"},{9,11,"RED ENERGY CRYSTAL"},{9,12,"BLUE ENERGY CRYSTAL"},
    {9,13,"GREEN ENERGY CRYSTAL"},
    {10,6,"SKELETON KEY"},{10,7,"SKELETON KEY"},
    {11,6,"SKELETON KEY"},
};
constexpr int max_slots=8;
const MissionDef* slots[max_slots];
int slot_count;
unsigned mission_level=~0u;
uint16_t mission_flags[17];
// Comma/Period presses from the input thread: count and net direction.
std::atomic<unsigned> browse_presses{0};
std::atomic<int> browse_steps{0};
int browse;
uint64_t mission_expires;
uint32_t card_pixels[592*52];
struct MissionArt {std::string name;Sprite icon,missing;};
std::vector<MissionArt> mission_art;
Sprite frame_grey,frame_orange,frame_steel,used_tick;
bool mission_loaded;
bool found(const MissionDef& d){return mission_flags[d.item]&1;}
// D08A19: used up at its lock (see ttk::mission_used_bit); it stays collected.
bool used(const MissionDef& d){return (mission_flags[d.item]&(ttk::mission_used_bit|1))==ttk::mission_used_bit;}
bool collected(const MissionDef& d){return found(d) || used(d);}
const char* mission_kind(const char* name){
    if(std::strstr(name,"CRYSTAL"))return "CRYSTAL";
    if(std::strstr(name,"JEWEL"))return "JEWEL";
    if(std::strstr(name,"PAPER") || std::strstr(name,"NOTE"))return "COMBO PIECE";
    if(!std::strcmp(name,"SKELETON KEY"))return "KEY";
    return "KEY CARD";
}
bool available(unsigned item){return (flags[item]&1) && ((flags[item]&2)||amount[item]>0);}
unsigned percent(unsigned item){
    unsigned charge=std::max(0,int(amount[item]));
    return charge_capacity[item]?charge*100/charge_capacity[item]:charge;
}
void blit(const uint32_t* src,int sw,int sh,int dx,int dy,int width,int height,int scale=2){
    for(int y=0;y<sh;++y)for(int x=0;x<sw;++x){
        uint32_t c=src[y*sw+x];if(!(c>>24))continue;
        for(int sy=0;sy<scale;++sy)for(int sx=0;sx<scale;++sx){
            int px=dx+x*scale+sx,py=dy+y*scale+sy;
            if(px>=0 && py>=0 && px<width && py<height)pixels[py*width+px]=c;
        }
    }
}
// Nearest-neighbor stretch (tile0020 frame → fixed cell that covers icon + %).
void blit_stretch(const uint32_t* src,int sw,int sh,int dx,int dy,int dw,int dh,int width,int height){
    if(sw<=0||sh<=0||dw<=0||dh<=0)return;
    for(int y=0;y<dh;++y)for(int x=0;x<dw;++x){
        // Sample pixel centres so both edges of a frame keep the same width.
        int sx=(2*x+1)*sw/(2*dw),sy=(2*y+1)*sh/(2*dh);
        uint32_t c=src[sy*sw+sx];if(!(c>>24))continue;
        int px=dx+x,py=dy+y;
        if(px>=0 && py>=0 && px<width && py<height)pixels[py*width+px]=c;
    }
}
uint32_t argb(const unsigned char* b){
    return (uint32_t(b[3])<<24)|(uint32_t(b[0])<<16)|(uint32_t(b[1])<<8)|uint32_t(b[2]);
}
bool read_sprite(Sprite& s,const unsigned char* pack,size_t& o,size_t size,int max_dim=64){
    if(o+4>size)return false;
    int w=pack[o]|unsigned(pack[o+1])<<8;o+=2;
    int h=pack[o]|unsigned(pack[o+1])<<8;o+=2;
    if(w<=0 || h<=0 || w>max_dim || h>max_dim || o+size_t(w*h*4)>size)return false;
    s.w=w;s.h=h;s.rgba.resize(w*h);
    for(int p=0;p<w*h;++p)s.rgba[p]=argb(&pack[o+size_t(p)*4]);
    o+=size_t(w*h*4);return true;
}
bool load_icons(){
    if(icons_loaded)return icons[5].w>0 && cursor.w>0;
    icons_loaded=true;
    const char* override_path=std::getenv("DNTTK_INV_ICONS");
    std::string path;
    if(override_path)path=override_path;
    else {
        auto base=SDL_GetBasePath();if(!base)return false;
        path=std::string(base)+"ttk-inv-icons.pack";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    }
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<16 || file.tellg()>256*1024)return false;
    std::vector<unsigned char> pack(static_cast<size_t>(file.tellg()));
    file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(pack.data()),pack.size()))return false;
    if(std::memcmp(pack.data(),"TTKICO2\0",8))return false;
    unsigned count=pack[8]|unsigned(pack[9])<<8|unsigned(pack[10])<<16|unsigned(pack[11])<<24;
    size_t o=12;
    for(unsigned n=0;n<count;++n){
        if(o+10>pack.size())return false;
        unsigned kind=pack[o]|unsigned(pack[o+1])<<8;o+=2;
        unsigned item=pack[o]|unsigned(pack[o+1])<<8;o+=2;
        o+=2; // source tile
        if(kind==0){
            if(item>5 || !read_sprite(icons[item],pack.data(),o,pack.size()))return false;
        } else if(kind==1){
            if(!read_sprite(cursor,pack.data(),o,pack.size()))return false;
        } else return false;
    }
    return icons[5].w>0 && cursor.w>0;
}
bool load_digits(){
    if(digits_loaded)return digits[0].w>0 && percent_glyph.w>0;
    digits_loaded=true;
    const char* override_path=std::getenv("DNTTK_INV_DIGITS");
    std::string path;
    if(override_path)path=override_path;
    else {
        auto base=SDL_GetBasePath();if(!base)return false;
        path=std::string(base)+"ttk-inv-digits.pack";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    }
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<16 || file.tellg()>64*1024)return false;
    std::vector<unsigned char> pack(static_cast<size_t>(file.tellg()));
    file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(pack.data()),pack.size()))return false;
    if(std::memcmp(pack.data(),"TTKDIG3\0",8))return false;
    unsigned count=pack[8]|unsigned(pack[9])<<8|unsigned(pack[10])<<16|unsigned(pack[11])<<24;
    if(count!=11)return false;
    size_t o=12;
    for(unsigned i=0;i<10;++i){
        if(o+2>pack.size())return false;
        unsigned tile=pack[o]|unsigned(pack[o+1])<<8;o+=2;
        if(tile!=3010+i || !read_sprite(digits[i],pack.data(),o,pack.size(),16))return false;
    }
    if(o+2>pack.size())return false;
    unsigned tile=pack[o]|unsigned(pack[o+1])<<8;o+=2;
    return tile==3076 && read_sprite(percent_glyph,pack.data(),o,pack.size(),16);
}
bool load_mission(){
    if(mission_loaded)return frame_grey.w>0;
    mission_loaded=true;
    const char* override_path=std::getenv("DNTTK_MISSION_ITEMS");
    std::string path;
    if(override_path)path=override_path;
    else {
        auto base=SDL_GetBasePath();if(!base)return false;
        path=std::string(base)+"ttk-mission-items.pack";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    }
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<16 || file.tellg()>256*1024)return false;
    std::vector<unsigned char> pack(static_cast<size_t>(file.tellg()));
    file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(pack.data()),pack.size()))return false;
    if(std::memcmp(pack.data(),"TTKMIS1\0",8))return false;
    unsigned count=pack[8]|unsigned(pack[9])<<8|unsigned(pack[10])<<16|unsigned(pack[11])<<24;
    size_t o=12;
    std::vector<MissionArt> art;
    for(unsigned n=0;n<count;++n){
        if(o+4>pack.size())return false;
        unsigned kind=pack[o]|unsigned(pack[o+1])<<8,length=pack[o+2]|unsigned(pack[o+3])<<8;o+=4;
        if(length>64 || o+length>pack.size())return false;
        std::string name(reinterpret_cast<const char*>(&pack[o]),length);o+=length;
        Sprite sprite;
        if(!read_sprite(sprite,pack.data(),o,pack.size()))return false;
        if(kind<=1){
            auto it=std::find_if(art.begin(),art.end(),[&](const MissionArt& a){return a.name==name;});
            if(it==art.end()){art.push_back({name,{},{}});it=art.end()-1;}
            (kind?it->missing:it->icon)=std::move(sprite);
        } else if(kind==2)frame_grey=std::move(sprite);
        else if(kind==3)frame_orange=std::move(sprite);
        else if(kind==4)frame_steel=std::move(sprite);
        else if(kind==5 && sprite.w<=16 && sprite.h<=16)used_tick=std::move(sprite);
        else return false;
    }
    if(!frame_grey.w || !frame_orange.w || !frame_steel.w)return false;
    mission_art=std::move(art);
    return true;
}
const MissionArt* art_for(const char* name){
    for(const auto& a:mission_art)if(a.name==name && a.icon.w && a.missing.w)return &a;
    return nullptr;
}
// Non-premultiplied ARGB "over": the mission panel is translucent, so icons,
// silhouettes and frames composite onto it instead of replacing its alpha.
uint32_t over(uint32_t dst,uint32_t src){
    const unsigned sa=src>>24;if(sa==255)return src;if(!sa)return dst;
    const unsigned da=dst>>24,oa=sa+da*(255-sa)/255;if(!oa)return 0;
    auto mix=[&](int s){return (((src>>s)&255)*sa+((dst>>s)&255)*da*(255-sa)/255)/oa;};
    return oa<<24|mix(16)<<16|mix(8)<<8|mix(0);
}
struct Canvas {
    uint32_t* px;int w,h;
    void set(int x,int y,uint32_t c){if(x>=0 && y>=0 && x<w && y<h)px[y*w+x]=over(px[y*w+x],c);}
    void panel(int x,int y,int rw,int rh){
        // rgba(12,14,20,0.62) with a 1 px #3a4150 outline (design E).
        for(int yy=y;yy<y+rh;++yy)for(int xx=x;xx<x+rw;++xx)
            if(xx>=0 && yy>=0 && xx<w && yy<h)
                px[yy*w+xx]=(yy==y || yy==y+rh-1 || xx==x || xx==x+rw-1)?0xff3a4150u:0x9e0c0e14u;
    }
    void sprite(const Sprite& s,int x,int y,int scale){
        for(int yy=0;yy<s.h*scale;++yy)for(int xx=0;xx<s.w*scale;++xx)set(x+xx,y+yy,s.rgba[(yy/scale)*s.w+xx/scale]);
    }
    void stretch(const Sprite& s,int x,int y,int dw,int dh){
        // Pixel-centre sampling: the 25x23 frame at 42x39 keeps equal top/bottom
        // and left/right borders (plain y*h/dh gave a 2 px top, 1 px bottom).
        for(int yy=0;yy<dh;++yy)for(int xx=0;xx<dw;++xx)
            set(x+xx,y+yy,s.rgba[((2*yy+1)*s.h/(2*dh))*s.w+(2*xx+1)*s.w/(2*dw)]);
    }
};
// One framed mission slot: the 25x23 project frame at 42x39, the 16x16 icon at
// 2x inside it, a dim silhouette while the item is missing. D08A19: a used
// item keeps its icon with the user's tick on the icon's bottom-right corner.
void framed_icon(Canvas& c,const Sprite& frame,const MissionDef& d,int x,int y){
    c.stretch(frame,x,y,42,39);
    if(const MissionArt* a=art_for(d.name))c.sprite(collected(d)?a->icon:a->missing,x+5,y+3,2);
    if(used(d) && used_tick.w)c.sprite(used_tick,x+5+(16-used_tick.w)*2,y+3+(16-used_tick.h)*2,2);
}
unsigned found_count(){unsigned n=0;for(int i=0;i<slot_count;++i)n+=collected(*slots[i]);return n;}
void draw_mission_row(uint32_t* px,int w,int h,int y0){
    Canvas c{px,w,h};
    const int pw=slot_count*46+12,ph=70,x0=(w-pw)/2;
    c.panel(x0,y0,pw,ph);
    const bool art=load_mission();
    for(int i=0;i<slot_count;++i)if(art)
        framed_icon(c,i==browse?frame_steel:frame_grey,*slots[i],x0+8+i*46,y0+6);
    char text[16];
    const unsigned got=found_count();
    std::snprintf(text,sizeof text,"%u/%d",got,slot_count);
    ttk_font_draw(TTK_FONT_MISSION_LABEL,"MISSION",px,w,h,x0+8,y0+52);
    const int set=int(got)==slot_count?TTK_FONT_MISSION_COMPLETE:TTK_FONT_MISSION_COUNT;
    const int tw=ttk_font_text_width(set,text);
    if(tw>0)ttk_font_draw(set,text,px,w,h,x0+pw-tw-8,y0+52);
}
void draw_percent(unsigned value,int center_x,int top,int width,int height){
    if(!load_digits())return;
    char text[8];std::snprintf(text,sizeof text,"%u",value);
    int total=percent_glyph.w;
    for(const char* p=text;*p;++p)if(*p>='0'&&*p<='9')total+=digits[*p-'0'].w+1;
    // 2x green THREEBYFIVE digits (palette 22) including %.
    int x=center_x-(total*2)/2;
    for(const char* p=text;*p;++p){
        if(*p<'0'||*p>'9')continue;
        const Sprite& d=digits[*p-'0'];
        blit(d.rgba.data(),d.w,d.h,x,top,width,height,2);
        x+=(d.w+1)*2;
    }
    blit(percent_glyph.rgba.data(),percent_glyph.w,percent_glyph.h,x,top,width,height,2);
}
}
namespace {
// Shared gates: Modernized, captured live gameplay, no menu or pause.
bool overlay_ready() {
    const auto& f=ttk::input_snapshot(ttk::Context::Gameplay);
    return ttk::input_modernized() && f.active && epoch==f.epoch && f.sequence>=sequence &&
        f.sequence-sequence<=4 &&
        psx_mod_read_half(0x800bcbb0)==1 && !psx_mod_read_half(0x800be568) && !psx_mod_read_half(0x800d2540);
}
}
namespace ttk {
bool inventory_visible() {return overlay_ready() && SDL_GetTicks()<expires;}
// D08A5 mission inventory: its own view on Comma/Period, never with the gadgets.
bool mission_visible() {return overlay_ready() && slot_count && SDL_GetTicks()<mission_expires;}
void mission_close() {mission_expires=0;}
int mission_browsed_item() {return mission_visible() && browse<slot_count?int(slots[browse]->item):-1;}
void inventory_update(unsigned item,const uint16_t* f,const int16_t* a,const uint16_t* capacity_values,bool announce) {
    const auto& input=input_snapshot(Context::Gameplay);
    if(epoch!=input.epoch)expires=mission_expires=0;
    epoch=input.epoch;sequence=input.sequence;selected=item;
    std::memcpy(charge_capacity,capacity_values,sizeof charge_capacity);
    std::memcpy(flags,f,sizeof flags);std::memcpy(amount,a,sizeof amount);
    unsigned count=0;for(unsigned i:order)count+=available(i);
    if(announce && count && input.active){expires=SDL_GetTicks()+2000;mission_expires=0;}
}
void mission_update(unsigned level,const uint16_t* mission) {
    if(level!=mission_level){browse=0;mission_expires=0;}
    mission_level=level;
    std::memcpy(mission_flags,mission,sizeof mission_flags);
    slot_count=0;
    for(const auto& d:mission_table)if(d.level==level && slot_count<max_slots)slots[slot_count++]=&d;
    const unsigned presses=browse_presses.exchange(0);
    const int steps=browse_steps.exchange(0);
    if(!presses || !slot_count || !input_snapshot(Context::Gameplay).active)return;
    // The first press opens on the item last shown; later presses step (wrapping).
    if(mission_visible())browse=((browse+steps)%slot_count+slot_count)%slot_count;
    else if(browse>=slot_count)browse=0;
    mission_expires=SDL_GetTicks()+2500;
    expires=0;
}
void mission_browse_press(int direction) {browse_steps.fetch_add(direction);browse_presses.fetch_add(1);}
}
extern "C" int ttk_inventory_pixels(const uint32_t* p){return p==pixels || p==card_pixels;}
// D08A5 item card: fixed 592x52 at the top of the screen with the mission inventory.
extern "C" int ttk_mission_card_image(const uint32_t** out,int* width,int* height,int available_width) {
    *out=nullptr;*width=*height=0;
    constexpr int w=592,h=52;
    if(!ttk::mission_visible() || browse>=slot_count || available_width<w || !load_mission())return 0;
    const MissionDef& d=*slots[browse];
    const bool got=collected(d);
    Canvas c{card_pixels,w,h};
    std::fill(card_pixels,card_pixels+w*h,0u);
    c.panel(0,0,w,h);
    framed_icon(c,frame_steel,d,10,6);
    ttk_font_draw(got?TTK_FONT_PANEL_SLOT:TTK_FONT_PANEL_SLOT_SELECTED,d.name,card_pixels,w,h,64,12);
    ttk_font_draw(TTK_FONT_PANEL_DIM,mission_kind(d.name),card_pixels,w,h,66,30);
    const char* status=used(d)?"USED":got?"FOUND":"NOT FOUND YET";
    const int set=used(d)?int(TTK_FONT_MISSION_USED):got?int(TTK_FONT_MISSION_FOUND):int(TTK_FONT_PANEL_DIM);
    const int sw=ttk_font_text_width(set,status);
    if(sw>0)ttk_font_draw(set,status,card_pixels,w,h,w-sw-14,22);
    *out=card_pixels;*width=w;*height=h;return 1;
}
extern "C" int ttk_inventory_image(const uint32_t** out,int* width,int* height,int available_width) {
    *out=nullptr;*width=*height=0;
    // D08A5: the mission inventory is its own surface at the switcher's place:
    // the panel plus 2 transparent lines under its outline.
    if(ttk::mission_visible()) {
        const int w=std::min(available_width,304),h=72;
        if(w<240)return 0;
        std::fill(pixels,pixels+w*h,0u);
        draw_mission_row(pixels,w,h,0);
        *out=pixels;*width=w;*height=h;return 1;
    }
    if(!ttk::inventory_visible())return 0;
    unsigned count=0;for(unsigned i:order)count+=available(i);
    if(!count)return 0;
    // Locked strip cell: frame 50×60, frame_dy=-6 (icons/% stay at cell_top).
    // Hybrid strip: every owned gadget with charge. Fixed cell holds icon above
    // and THREEBYFIVE % on one baseline; tile0020 is stretched to that cell and
    // drawn under the % so the frame never clips the numbers.
    int w=std::min(available_width,304),h=64;
    if(w<240)return 0;
    std::fill(pixels,pixels+w*h,0u);
    constexpr int cell_w=50,cell_h=60,cell_top=4,icon_box=36,frame_dy=-6;
    int pct_top=cell_top+icon_box+2;
    int slot=std::min(72,w/int(count)),left=(w-slot*count)/2,index=0;
    const bool have_icons=load_icons();
    for(unsigned i:order)if(available(i)) {
        int x=left+index++*slot;
        int cell_x=x+(slot-cell_w)/2;
        if(have_icons && icons[i].w) {
            int iw=icons[i].w*2,ih=icons[i].h*2;
            blit(icons[i].rgba.data(),icons[i].w,icons[i].h,
                 cell_x+(cell_w-iw)/2,cell_top+(icon_box-ih)/2,w,h,2);
        }
        if(flags[i]&2)for(int d=-2;d<=2;++d) {
            int mx=cell_x+cell_w-4,my=cell_top+4;
            if(mx>=0 && mx<w && my>=0 && my<h) {
                if(mx+d>=0 && mx+d<w)pixels[my*w+mx+d]=0xffffd050u;
                if(my+d>=0 && my+d<h)pixels[(my+d)*w+mx]=0xffffd050u;
            }
        }
        // D08A5: the project frame in the tile0020 orange ramp (design E).
        if(selected==i && load_mission())
            blit_stretch(frame_orange.rgba.data(),frame_orange.w,frame_orange.h,cell_x,cell_top+frame_dy,cell_w,cell_h,w,h);
        else if(selected==i && have_icons && cursor.w)
            blit_stretch(cursor.rgba.data(),cursor.w,cursor.h,cell_x,cell_top+frame_dy,cell_w,cell_h,w,h);
        // % after the frame so the bottom border cannot cover the digits.
        draw_percent(percent(i),x+slot/2,pct_top,w,h);
    }
    *out=pixels;*width=w;*height=h;return 1;
}
