// D39B: the screen flash mix rules (screen_flash.h), against note 370's spec.
#include "screen_flash.h"
#include <cassert>
#include <cmath>
#include <cstdio>
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
    // The last tic fades to nothing; a 20 fps frame (delta 15) keeps the line.
    assert(ttk::flash_mix(st(10,32,0,0,1),st(0,32,0,0,1),0.5,false,c) && near(c[3],0.5/64));
    assert(!ttk::flash_mix(st(10,32,0,0,1),st(0,32,0,0,1),1.0,false,c));
    assert(ttk::flash_mix(st(300,32,0,0,1),st(285,32,0,0,1),0.5,false,c) && near(c[3],29.25/64));
    std::puts("screen flash: ok");
}
