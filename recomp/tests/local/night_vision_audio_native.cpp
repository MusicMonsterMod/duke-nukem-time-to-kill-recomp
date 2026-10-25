#include "night_vision_audio.h"
#include "night_vision_pcm.h"
#include "bio_mask_pcm.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
static bool modern=true,gameplay=true;
static const char* mode=nullptr;
static const char* mask_mode=nullptr;
static uint32_t loads=0,volume=0x3fff;
namespace ttk {
bool input_modernized() {return modern;}
bool replay_gameplay_context() {return gameplay;}
}
extern "C" const char* ttk_live_env(const char* name) {return std::strcmp(name,"DNTTK_AUTO_BIO_MASK")==0?mask_mode:mode;}
extern "C" uint32_t psx_mod_savestate_loads() {return loads;}
extern "C" uint32_t psx_mod_read_word(uint32_t addr) {assert(addr==0x800c39e0);return volume;}
int main() {
    constexpr unsigned n=sizeof(night_vision_pcm)/sizeof(int16_t);
    constexpr unsigned b=sizeof(bio_mask_pcm)/sizeof(int16_t);
    std::array<int16_t,2*(std::max(n,b)+32)> out{};
    auto clear=[&]{out.fill(0);};
    auto silent=[&]{return std::all_of(out.begin(),out.end(),[](int x){return x==0;});};
    ttk_audio_mix(out.data(),n);assert(silent());
    ttk::night_vision_sound();ttk_audio_mix(out.data(),n+32);
    for(unsigned i=0;i<n;++i)assert(out[2*i]==night_vision_pcm[i] && out[2*i+1]==night_vision_pcm[i]);
    for(unsigned i=2*n;i<out.size();++i)assert(!out[i]);
    clear();ttk_audio_mix(out.data(),n);assert(silent()); // one shot
    volume=8191;ttk::night_vision_sound();ttk_audio_mix(out.data(),n);
    for(unsigned i=0;i<n;++i)assert(out[2*i]==int(night_vision_pcm[i])*8191/0x3fff);
    clear();volume=0;ttk::night_vision_sound();ttk_audio_mix(out.data(),n);assert(silent());
    volume=0x3fff;ttk_audio_mix(out.data(),n);assert(silent()); // no deferred unmute
    for(int why=0;why<5;++why) {
        clear();ttk::night_vision_sound();
        if(why==0)++loads;
        if(why==1)gameplay=false;
        if(why==2)modern=false;
        if(why==3)mode="original";
        if(why==4)ttk_audio_reset();
        ttk_audio_mix(out.data(),n);assert(silent());
        gameplay=modern=true;mode=nullptr;ttk_audio_mix(out.data(),n);assert(silent());
    }
    // Restart, not a growing queue; chunk boundaries preserve exact samples.
    ttk::night_vision_sound();ttk_audio_mix(out.data(),19);
    ttk::night_vision_sound();clear();
    ttk_audio_mix(out.data(),37);ttk_audio_mix(out.data()+74,n-37);
    for(unsigned i=0;i<n;++i)assert(out[2*i]==night_vision_pcm[i]);
    // Saturating mix never wraps and does not alter the opposite channel's sign.
    for(unsigned i=0;i<out.size();i+=2){out[i]=32760;out[i+1]=-32760;}
    ttk::night_vision_sound();ttk_audio_mix(out.data(),n);
    for(unsigned i=0;i<n;++i) {
        assert(out[2*i]==std::clamp(32760+int(night_vision_pcm[i]),-32768,32767));
        assert(out[2*i+1]==std::clamp(-32760+int(night_vision_pcm[i]),-32768,32767));
    }
    // D08A24: the Bio Mask uses the same volume, reset and bounded voice
    // contract, with its own feature gate and exact licensed PCM.
    clear();ttk::bio_mask_sound();ttk_audio_mix(out.data(),b+32);
    for(unsigned i=0;i<b;++i)assert(out[2*i]==bio_mask_pcm[i] && out[2*i+1]==bio_mask_pcm[i]);
    for(unsigned i=2*b;i<out.size();++i)assert(!out[i]);
    clear();ttk_audio_mix(out.data(),b);assert(silent());
    volume=8191;ttk::bio_mask_sound();ttk_audio_mix(out.data(),b);
    for(unsigned i=0;i<b;++i)assert(out[2*i]==int(bio_mask_pcm[i])*8191/0x3fff);
    clear();volume=0;ttk::bio_mask_sound();ttk_audio_mix(out.data(),b);assert(silent());
    volume=0x3fff;ttk_audio_mix(out.data(),b);assert(silent());
    for(int why=0;why<5;++why) {
        clear();ttk::bio_mask_sound();
        if(why==0)++loads;
        if(why==1)gameplay=false;
        if(why==2)modern=false;
        if(why==3)mask_mode="off";
        if(why==4)ttk_audio_reset();
        ttk_audio_mix(out.data(),b);assert(silent());
        gameplay=modern=true;mask_mode=nullptr;ttk_audio_mix(out.data(),b);assert(silent());
    }
    ttk::bio_mask_sound();ttk_audio_mix(out.data(),19);ttk::bio_mask_sound();clear();
    ttk_audio_mix(out.data(),37);ttk_audio_mix(out.data()+74,b-37);
    for(unsigned i=0;i<b;++i)assert(out[2*i]==bio_mask_pcm[i]);
    // Changing face equipment replaces the old cue, never layers a queue.
    ttk::night_vision_sound();ttk::bio_mask_sound();clear();ttk_audio_mix(out.data(),b);
    for(unsigned i=0;i<b;++i)assert(out[2*i]==bio_mask_pcm[i]);
    ttk::bio_mask_sound();ttk::night_vision_sound();clear();ttk_audio_mix(out.data(),b);
    for(unsigned i=0;i<b;++i)assert(out[2*i]==(i<n?night_vision_pcm[i]:0));
    std::puts("PASS: Bio Mask exact PCM, volume/mute, pause/load/Off/Vanilla, restart and face-cue replacement");
    std::puts("PASS: selected PCM, one shot, live effects volume/mute, load/pause/reset/Original/Vanilla, restart and saturation");
}
