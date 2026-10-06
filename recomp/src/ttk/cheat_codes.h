#pragma once
#include <cstdint>
#include <cstring>
namespace ttk {
enum class Cheat { None, Monsters, God, Stuff, Keys, Weapons, Inventory, Items, Ammo, Health, Unlimited, Hyper, Upgrade };
struct CheatCode {const char* text;Cheat action;};
inline constexpr CheatCode cheat_codes[]={
    {"dnmonsters",Cheat::Monsters},{"dnkroz",Cheat::God},{"dncornholio",Cheat::God},
    {"dnstuff",Cheat::Stuff},{"dnkeys",Cheat::Keys},{"dnweapons",Cheat::Weapons},
    {"dninventory",Cheat::Inventory},{"dnitems",Cheat::Items},
    {"dnammo",Cheat::Ammo},{"dnhealth",Cheat::Health},{"dnunlimited",Cheat::Unlimited},
    {"dnhyper",Cheat::Hyper},{"dnupgrade",Cheat::Upgrade}};
// Physical letters match the existing PC action layer. D alone remains normal
// strafing; DN enters cheat typing and consumes the remaining gameplay keys.
struct CheatTyping {
    char text[24]{};unsigned length=0;uint64_t deadline=0;
    void reset(){length=0;text[0]=0;deadline=0;}
    bool active()const{return length>=2;}
    Cheat feed(char c,uint64_t tick) {
        if(tick>deadline)reset();
        if(length==0) {if(c=='d'){text[0]='d';text[1]=0;length=1;deadline=tick+60;}return Cheat::None;}
        if(length+1>=sizeof text){reset();return Cheat::None;}
        text[length++]=c;text[length]=0;deadline=tick+180;
        bool prefix=false;
        for(auto code:cheat_codes) {
            if(!std::strcmp(text,code.text)){auto result=code.action;reset();return result;}
            if(!std::strncmp(text,code.text,length))prefix=true;
        }
        if(!prefix){reset();if(c=='d'){text[0]='d';text[1]=0;length=1;deadline=tick+60;}}
        return Cheat::None;
    }
};
}
