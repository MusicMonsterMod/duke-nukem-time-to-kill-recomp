// D39B/C: the screen flash mix rules, the Screen Flashes option and the
// console's palfrom arguments (screen_flash.h), against note 370's spec.
#include "screen_flash.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
using ttk::FlashState;
static bool near(double a,double b,double e=1e-6) {return std::fabs(a-b)<=e;}
static FlashState st(uint32_t intensity_tenths,int r,int g,int b,int seq) {FlashState s;s.level=intensity_tenths;s.r=r;s.g=g;s.b=b;s.seq=seq;return s;}
int main() {
    float c[4];
    // Weight min(I, 63) / 64 and colour 4 x component (display values).
    assert(ttk::flash_rgbw(st(320,32,0,0,1),320,c) && near(c[3],0.5) && near(c[0],128.0/255) && c[1]==0 && c[2]==0);
    assert(ttk::flash_rgbw(st(630,63,63,63,1),630,c) && near(c[3],63.0/64) && near(c[0],252.0/255));
    // Above 63 holds full strength.
    assert(ttk::flash_rgbw(st(1000,0,0,63,1),1000,c) && near(c[3],63.0/64));
    assert(!ttk::flash_rgbw(st(0,63,0,0,1),0,c));
    // A fade between two frames is linear in the redraw's alpha.
    const FlashState a=st(320,32,0,0,4),b=st(310,32,0,0,4);
    assert(ttk::flash_mix(a,b,0.0,false,c) && near(c[3],32.0/64));
    assert(ttk::flash_mix(a,b,0.5,false,c) && near(c[3],31.5/64));
    assert(ttk::flash_mix(a,b,1.0,false,c) && near(c[3],31.0/64));
    // Steps: the base frame's own level (Duke3D's 30 Hz staircase).
    assert(ttk::flash_mix(a,b,0.9,true,c) && near(c[3],32.0/64));
    // A flash started in the next frame shows from the next pair, at once
    // and at full intensity: never blended in.
    const FlashState none=st(0,0,0,0,4),red=st(480,52,0,0,5);
    assert(!ttk::flash_mix(none,red,0.9,false,c));
    assert(ttk::flash_mix(red,st(470,52,0,0,5),0.0,false,c) && near(c[3],48.0/64) && near(c[0],208.0/255));
    // Replacement by a weaker flash of another colour: the old one until the
    // new frame, then the new one (no blend between the two).
    assert(ttk::flash_mix(st(440,63,0,0,5),st(160,0,63,0,6),0.7,false,c) && near(c[3],44.0/64) && near(c[0],252.0/255));
    assert(ttk::flash_mix(st(160,0,63,0,6),st(150,0,63,0,6),0.0,false,c) && near(c[3],16.0/64) && near(c[1],252.0/255));
    // D39E: a flash cleared in the next frame (palfrom 0, the thaw) still
    // shows across the pair: those images are the base frame's composition.
    const FlashState blue=st(160,0,0,24,9),cleared=st(0,0,0,24,10);
    assert(ttk::flash_mix(blue,cleared,0.0,false,c) && ttk::flash_mix(blue,cleared,0.9,false,c) && near(c[3],0.25));
    // The last tic fades to nothing; a 20 fps frame (delta 15) keeps the line.
    assert(ttk::flash_mix(st(10,32,0,0,1),st(0,32,0,0,1),0.5,false,c) && near(c[3],0.5/64));
    assert(!ttk::flash_mix(st(10,32,0,0,1),st(0,32,0,0,1),1.0,false,c));
    assert(ttk::flash_mix(st(300,32,0,0,1),st(285,32,0,0,1),0.5,false,c) && near(c[3],29.25/64));
    // D39C option: On draws the measured weight, Reduced half of it capped at
    // a quarter, Off nothing; colour unchanged.
    using ttk::FlashOption;
    float o[4]={128.f/255,0,0,0.5f};
    assert(ttk::flash_option_rgbw(FlashOption::On,o) && near(o[3],0.5));
    assert(ttk::flash_option_rgbw(FlashOption::Reduced,o) && near(o[3],0.25) && near(o[0],128.0/255));
    o[3]=63.f/64; assert(ttk::flash_option_rgbw(FlashOption::Reduced,o) && near(o[3],0.25));
    o[3]=0.25f; assert(ttk::flash_option_rgbw(FlashOption::Reduced,o) && near(o[3],0.125));
    o[3]=0.5f; assert(!ttk::flash_option_rgbw(FlashOption::Off,o));
    assert(ttk::flash_option_parse(nullptr)==FlashOption::On && ttk::flash_option_parse("on")==FlashOption::On);
    assert(ttk::flash_option_parse("reduced")==FlashOption::Reduced && ttk::flash_option_parse("off")==FlashOption::Off);
    assert(!std::strcmp(ttk::flash_option_name(FlashOption::Reduced),"reduced"));
    // Console arguments: palfrom's own, omitted components 0.
    int v[4];
    assert(!ttk::flash_parse_palfrom(" 48 52",v) && v[0]==48 && v[1]==52 && v[2]==0 && v[3]==0);
    assert(!ttk::flash_parse_palfrom(" 63 63 63 63",v) && v[0]==63 && v[3]==63);
    assert(!ttk::flash_parse_palfrom(" 100 0 0 63",v) && v[0]==100 && v[3]==63);
    assert(!ttk::flash_parse_palfrom(" 0",v) && v[0]==0);
    assert(!ttk::flash_parse_palfrom(" 255\t1",v) && v[0]==255 && v[1]==1);
    assert(ttk::flash_parse_palfrom("",v) && ttk::flash_parse_palfrom("   ",v));
    assert(ttk::flash_parse_palfrom(" 256",v) && ttk::flash_parse_palfrom(" 32 64",v));
    assert(ttk::flash_parse_palfrom(" -1",v) && ttk::flash_parse_palfrom(" 32 red",v) && ttk::flash_parse_palfrom(" 3.5",v));
    assert(ttk::flash_parse_palfrom(" 1 2 3 4 5",v) && ttk::flash_parse_palfrom(" 99999999999999999999",v));
    // D39E: real hits on Duke (damage type, health + armour taken, death,
    // freeze, attacker type, projectile row) to Duke3D palfrom values.
    auto df=[](unsigned t,int lost,bool died=false,bool lit=false,unsigned src=0,int proj=-1){return ttk::damage_flash_for(t,lost,died,lit,src,proj);};
    auto is=[](ttk::DamageFlash f,int i,int r,int g,int b){return f.intensity==i && f.r==r && f.g==g && f.b==b;};
    assert(is(df(2,750),32,32,0,0) && !std::strcmp(df(2,750).kind,"hit"));   // pig cop shot (slot 3)
    assert(is(df(2,224),16,32,0,0) && is(df(1,300),32,32,0,0));             // light hit: Duke3D's knee
    assert(is(df(4,1124),32,32,0,0) && is(df(4,3000),48,52,0,0));           // blasts: RPG strength when heavy
    assert(is(df(13,1500),48,52,0,0) && is(df(16,10,true),48,52,0,0));
    assert(is(df(11,2250),16,32,0,0) && is(df(11,2500),32,32,0,0));         // falls
    assert(is(df(12,5),16,24,32,0));                                         // gas, each frame in it
    assert(is(df(7,0,false,true),16,0,0,24) && df(7,0).intensity==0);       // the Freezer: EDuke32's frozen blue
    assert(is(df(2,750,false,false,50,20),48,48,0,48) && !std::strcmp(df(2,750,false,false,50,20).kind,"necro")); // Necro bolt
    assert(is(df(2,750,false,false,0,20),48,48,0,48) && is(df(2,750,false,false,57,2),32,32,0,0));
    assert(df(2,0,false,false,50,20).intensity==0);                         // a Necro bolt god mode cancelled
    assert(is(df(2,0,true),32,32,0,0));                                      // a killing hit
    for(unsigned t=1;t<=20;++t) assert(df(t,0).intensity==0);               // cancelled (god mode, mask) or nothing taken
    std::puts("screen flash: ok");
}
