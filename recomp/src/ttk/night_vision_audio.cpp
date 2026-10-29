#include "night_vision_audio.h"
#include "piano_audio.h"
#include "night_vision_pcm.h"
#include "bio_mask_pcm.h"
#include "modern_controls.h"
#include "pc_input.h"
#include "live_env.h"
#include "mod_plugins.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
const int16_t* pcm=night_vision_pcm;
unsigned samples=sizeof(night_vision_pcm)/sizeof(night_vision_pcm[0]);
bool mask_cue;
unsigned cursor=samples;
uint32_t loads;
bool enabled(bool mask) {
    const char* mode=ttk_live_env(mask?"DNTTK_AUTO_BIO_MASK":"DNTTK_NIGHT_VISION");
    return ttk::input_modernized() && (!mode || std::strcmp(mode,mask?"off":"original"));
}
}
namespace ttk {
void night_vision_sound() {
    if(!enabled(false))return;
    mask_cue=false;pcm=night_vision_pcm;samples=sizeof(night_vision_pcm)/sizeof(int16_t);
    // No accumulating queue on rapid toggles. The approved 0.24 s cue restarts.
    cursor=0;loads=psx_mod_savestate_loads();
    if(const char* trace=std::getenv("DNTTK_TRACE_NIGHT_VISION_SOUND");trace && *trace=='1')
        std::fprintf(stderr,"[night-vision-sound] toggle A samples=%u effects=%u\n",
                     samples,psx_mod_read_word(0x800c39e0));
}
void bio_mask_sound() {
    if(!enabled(true))return;
    mask_cue=true;pcm=bio_mask_pcm;samples=sizeof(bio_mask_pcm)/sizeof(int16_t);
    // One shared gadget voice: rapid toggles replace the current cue.
    cursor=0;loads=psx_mod_savestate_loads();
    if(const char* trace=std::getenv("DNTTK_TRACE_BIO_MASK_SOUND");trace && *trace=='1')
        std::fprintf(stderr,"[bio-mask-sound] toggle samples=%u effects=%u\n",
                     samples,psx_mod_read_word(0x800c39e0));
}
}
extern "C" void ttk_audio_reset() {cursor=samples;ttk::piano_audio_reset();}
extern "C" void ttk_audio_mix(int16_t* stereo,int frames) {
    ttk::piano_audio_mix(stereo,frames);
    if(cursor==samples || !stereo || frames<=0)return;
    if(!enabled(mask_cue) || loads!=psx_mod_savestate_loads() || !ttk::replay_gameplay_context()) {
        cursor=samples;return;
    }
    // SLUS-00583 SOUND effects slider: 8008acc8..8008acfc stores
    // floor(percent*0x3fff/100) at 800c39e0. Native SFX also gate on it.
    // Read live, so muting never queues a cue for a later unmute.
    const int gain=std::clamp(int32_t(psx_mod_read_word(0x800c39e0)),0,0x3fff);
    const unsigned n=std::min(unsigned(frames),samples-cursor);
    for(unsigned i=0;i<n;++i) {
        const int sample=int(pcm[cursor++])*gain/0x3fff;
        stereo[2*i]=int16_t(std::clamp(int(stereo[2*i])+sample,-32768,32767));
        stereo[2*i+1]=int16_t(std::clamp(int(stereo[2*i+1])+sample,-32768,32767));
    }
}
