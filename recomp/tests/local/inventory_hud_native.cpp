#include "inventory_hud.h"
#include "pc_input.h"
#include <cassert>
#include <cstdlib>
#include <initializer_list>
namespace ttk {
bool modern=true;InputFrame input;
const char* input_binding_name(Action){return "L";}
bool input_modernized(){return modern;}
const InputFrame& input_snapshot(Context){return input;}
}
static bool menu;
extern "C" uint16_t psx_mod_read_half(uint32_t a){return a==0x800bcbb0?1:menu;}
int main(int argc,char**argv) {
    assert(argc>=3);setenv("DNTTK_FONT_PACK",argv[1],1); // unused by strip now, kept for link
    setenv("DNTTK_INV_ICONS",argv[2],1);
    if(argc>=4)setenv("DNTTK_INV_DIGITS",argv[3],1);
    ttk::input.active=true;ttk::input.epoch=1;ttk::input.sequence=10;
    uint16_t flags[6]{},capacities[6]={0,9000,13500,18000,9000,10000};int16_t amount[6]{};
    const uint32_t* pixels=nullptr;int w,h;
    auto visible=[&](int width=304){auto v=ttk_inventory_image(&pixels,&w,&h,width);assert(bool(v)==ttk::inventory_visible());return v;};
    ttk::inventory_update(0,flags,amount,capacities,false);assert(!visible());
    ttk::inventory_update(0,flags,amount,capacities,true);assert(!visible());
    for(unsigned i:{1u,2u,3u,5u}){flags[i]=1;amount[i]=100;}
    for(unsigned selected:{1u,2u,3u,5u}) {
        ttk::inventory_update(selected,flags,amount,capacities,true);
        for(int width:{240,304,624,1008}) {
            assert(visible(width) && w<=width && h==64);
            unsigned opaque=0,transparent=0;
            for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
                auto c=pixels[y*w+x];opaque+=(c>>24)==255;transparent+=c==0;
            }
            assert(opaque>100 && transparent);
        }
    }
    menu=true;assert(!visible());menu=false;
    ttk::input.active=false;assert(!visible());ttk::input.active=true;
    ++ttk::input.epoch;assert(!visible());--ttk::input.epoch;
    ttk::input.sequence+=5;assert(!visible());ttk::input.sequence-=5;
    ttk::modern=false;assert(!visible());ttk::modern=true;
    SDL_Delay(2010);assert(!visible());
}
