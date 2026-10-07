#include "inventory_hud.h"
#include "pc_input.h"
#include <cassert>
#include <cstdio>
#include <string>
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
    if(argc>=5)setenv("DNTTK_MISSION_ITEMS",argv[4],1);
    const char* dump=argc>=6?argv[5]:nullptr; // optional review PPMs (composited on grey)
    ttk::input.active=true;ttk::input.epoch=1;ttk::input.sequence=10;
    uint16_t flags[6]{},capacities[6]={0,9000,13500,18000,9000,10000};int16_t amount[6]{};
    const uint32_t* pixels=nullptr;int w,h;
    auto visible=[&](int width=304){auto v=ttk_inventory_image(&pixels,&w,&h,width);assert(bool(v)==(ttk::inventory_visible() || ttk::mission_visible()));return v;};
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
    if(argc<5)return 0;
    // D08A5 mission row: level 6 (two skeleton keys, three jewels), key 2 and a jewel found.
    auto save=[&](const char* name,const uint32_t* px,int pw,int ph){
        if(!dump)return;
        std::string path=std::string(dump)+"/"+name+".ppm";
        FILE* f=std::fopen(path.c_str(),"wb");std::fprintf(f,"P6 %d %d 255\n",pw,ph);
        for(int i=0;i<pw*ph;++i){unsigned a=px[i]>>24;for(int s:{16,8,0}){unsigned c=(px[i]>>s)&255;std::fputc((c*a+96*(255-a))/255,f);}}
        std::fclose(f);
    };
    uint16_t mission[17]{};mission[8]=1;mission[15]=1;
    const uint32_t* card=nullptr;int cw,ch;
    // [ / ] show only the gadgets, even in a level with mission items.
    ttk::mission_update(6,mission);
    ttk::inventory_update(2,flags,amount,capacities,true);ttk::mission_update(6,mission);
    assert(visible() && w==304 && h==64 && !ttk::mission_visible());
    assert(!ttk_mission_card_image(&card,&cw,&ch,624));
    save("gadgets-level6",pixels,w,h);
    // Comma/Period open the mission inventory instead (the gadgets close), on the
    // first item; the card shows with it.
    ttk::mission_browse_press(1);ttk::mission_update(6,mission);
    assert(ttk::mission_visible() && !ttk::inventory_visible());
    assert(visible() && w==304 && h==72);
    unsigned translucent=0;for(int i=0;i<w*h;++i)translucent+=(pixels[i]>>24)>0 && (pixels[i]>>24)<255;
    assert(translucent>1000); // the grey panel
    for(int x=0;x<w;++x)assert(!pixels[(h-1)*w+x]); // margin under the outline
    save("mission-level6-key1",pixels,w,h);
    assert(ttk_mission_card_image(&card,&cw,&ch,624) && cw==592 && ch==52);
    save("card-key1-missing",card,cw,ch);
    assert(!ttk_mission_card_image(&card,&cw,&ch,500)); // needs the full width
    ttk::mission_browse_press(1);ttk::mission_update(6,mission);
    ttk_mission_card_image(&card,&cw,&ch,624);save("card-key2-found",card,cw,ch);
    visible();save("mission-level6-key2",pixels,w,h);
    ttk::mission_browse_press(-1);ttk::mission_browse_press(-1);ttk::mission_update(6,mission); // wraps to the last jewel
    visible();save("mission-level6-last",pixels,w,h);
    // [ / ] close the mission inventory and show the gadgets.
    ttk::inventory_update(2,flags,amount,capacities,true);ttk::mission_update(6,mission);
    assert(!ttk::mission_visible() && ttk::inventory_visible() && visible() && h==64);
    // Reopening shows the item last browsed; closing (Enter / U) hides both surfaces.
    ttk::mission_browse_press(1);ttk::mission_update(6,mission);assert(ttk::mission_visible());
    ttk::mission_close();assert(!ttk::mission_visible() && !ttk_mission_card_image(&card,&cw,&ch,624));
    // Levels without mission items: Comma/Period do nothing.
    ttk::mission_update(8,mission);
    ttk::mission_browse_press(1);ttk::mission_update(8,mission);assert(!ttk::mission_visible() && !ttk_mission_card_image(&card,&cw,&ch,624));
    // All found turns the count green; works without gadgets.
    uint16_t all[17]{};for(unsigned i:{6u,7u,11u,12u,13u})all[i]=1;
    uint16_t none[6]{};
    ttk::inventory_update(5,none,amount,capacities,false);ttk::mission_update(0,all);
    ttk::mission_browse_press(1);ttk::mission_update(0,all);
    assert(visible() && h==72);save("mission-level0-complete",pixels,w,h);
    SDL_Delay(2510);ttk::mission_update(0,all);assert(!visible() && !ttk_mission_card_image(&card,&cw,&ch,624));
    std::puts("PASS: inventory strip, D08A5 mission inventory, browsing, card, timeout");
}
