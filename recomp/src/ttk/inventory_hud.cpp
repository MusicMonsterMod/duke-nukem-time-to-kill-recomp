#include "inventory_hud.h"
#include "duke_font.h"
#include "pc_input.h"
#include "mod_plugins.h"
#include <algorithm>
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
constexpr unsigned order[]={5,1,2,3};
struct Sprite {int w,h;std::vector<uint32_t> rgba;};
Sprite icons[6], cursor, digits[10], percent_glyph;
bool icons_loaded, digits_loaded;
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
        int sx=x*sw/dw,sy=y*sh/dh;
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
namespace ttk {
bool inventory_visible() {
    const auto& f=input_snapshot(Context::Gameplay);
    return input_modernized() && f.active && epoch==f.epoch && f.sequence>=sequence &&
        f.sequence-sequence<=4 && SDL_GetTicks()<expires &&
        psx_mod_read_half(0x800bcbb0)==1 && !psx_mod_read_half(0x800be568) && !psx_mod_read_half(0x800d2540);
}
void inventory_update(unsigned item,const uint16_t* f,const int16_t* a,const uint16_t* capacity_values,bool announce) {
    const auto& input=input_snapshot(Context::Gameplay);
    if(epoch!=input.epoch)expires=0;
    epoch=input.epoch;sequence=input.sequence;selected=item;
    std::memcpy(charge_capacity,capacity_values,sizeof charge_capacity);
    std::memcpy(flags,f,sizeof flags);std::memcpy(amount,a,sizeof amount);
    unsigned count=0;for(unsigned i:order)count+=available(i);
    if(announce && count && input.active)expires=SDL_GetTicks()+2000;
}
}
extern "C" int ttk_inventory_pixels(const uint32_t* p){return p==pixels;}
extern "C" int ttk_inventory_image(const uint32_t** out,int* width,int* height,int available_width) {
    *out=nullptr;*width=*height=0;
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
        if(selected==i && have_icons && cursor.w)
            blit_stretch(cursor.rgba.data(),cursor.w,cursor.h,cell_x,cell_top+frame_dy,cell_w,cell_h,w,h);
        // % after the frame so the bottom border cannot cover the digits.
        draw_percent(percent(i),x+slot/2,pct_top,w,h);
    }
    *out=pixels;*width=w;*height=h;return 1;
}
