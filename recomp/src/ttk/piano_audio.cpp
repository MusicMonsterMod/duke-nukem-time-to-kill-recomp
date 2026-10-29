#include "piano_audio.h"
#include "piano_pcm.h"
#include "pc_input.h"
#include "modern_controls.h"
#include "mod_plugins.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cmath>

namespace {
constexpr unsigned samples=sizeof(piano_pcm)/sizeof(piano_pcm[0]);
unsigned cursor=samples;
uint32_t loads;
int32_t source_x,source_y,source_z;
double proximity;
// World units: retain full level at the keyboard, then smoothly fall to zero.
// Use Duke, not the third-person camera, so changing view cannot move the ear.
constexpr double inner_radius=768,outer_radius=4096;
double distance_gain() {
    const double dx=double(int32_t(psx_mod_read_word(0x800d719c)))-source_x;
    const double dy=double(int32_t(psx_mod_read_word(0x800d71a0)))-source_y;
    const double dz=double(int32_t(psx_mod_read_word(0x800d71a4)))-source_z;
    const double t=std::clamp((std::sqrt(dx*dx+dy*dy+dz*dz)-inner_radius)/(outer_radius-inner_radius),0.0,1.0);
    return 1-t*t*(3-2*t);
}
}
namespace ttk {
void piano_audio_reset() {cursor=samples;}
void piano_sound(int32_t x,int32_t y,int32_t z) {
    if(!input_modernized() || !replay_gameplay_context())return;
    const auto generation=psx_mod_savestate_loads();
    // A second successful use during the performance neither layers nor restarts
    // it. Gadget cues have their own voice. A load discards the previous world.
    if(loads==generation && cursor<samples)return;
    loads=generation;cursor=0;
    source_x=x;source_y=y;source_z=z;proximity=distance_gain();
    if(std::getenv("DNTTK_PIANO_TRACE"))
        std::fprintf(stderr,"[piano-sound] start samples=%u\n",samples);
}
void piano_audio_mix(int16_t* stereo,int frames) {
    if(cursor==samples || !stereo || frames<=0)return;
    if(!input_modernized() || loads!=psx_mod_savestate_loads() || !replay_gameplay_context()) {
        piano_audio_reset();return;
    }
    const int gain=std::clamp(int32_t(psx_mod_read_word(0x800c39e0)),0,0x3fff);
    const double target=distance_gain();
    const unsigned n=std::min(unsigned(frames),samples-cursor);
    for(unsigned i=0;i<n;++i) {
        // A 20 ms linear slew avoids clicks from discrete player updates.
        // Cursor always advances, including when outside the audible radius.
        proximity+=std::clamp(target-proximity,-1.0/882,1.0/882);
        const int sample=int(int(piano_pcm[cursor++])*gain/0x3fff*proximity);
        stereo[2*i]=int16_t(std::clamp(int(stereo[2*i])+sample,-32768,32767));
        stereo[2*i+1]=int16_t(std::clamp(int(stereo[2*i+1])+sample,-32768,32767));
    }
    if(std::getenv("DNTTK_PIANO_TRACE") && cursor/4410!=(cursor-n)/4410)
        std::fprintf(stderr,"[piano-proximity] sample=%u gain=%.4f\n",cursor,proximity);
}
}
