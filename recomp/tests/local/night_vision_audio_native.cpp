#include "night_vision_audio.h"
#include "piano_audio.h"
#include "piano_pcm.h"
#include <vector>
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
static int32_t listener[3]={};
namespace ttk {
bool input_modernized() {return modern;}
bool replay_gameplay_context() {return gameplay;}
}
extern "C" const char* ttk_live_env(const char* name) {return std::strcmp(name,"DNTTK_AUTO_BIO_MASK")==0?mask_mode:mode;}
extern "C" uint32_t psx_mod_savestate_loads() {return loads;}
extern "C" uint32_t psx_mod_read_word(uint32_t addr) {if(addr>=0x800d719c && addr<=0x800d71a4)return listener[(addr-0x800d719c)/4];assert(addr==0x800c39e0);return volume;}
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
    // D38: full-length piano with chunked playback and a separate gadget
    // voice. Retriggers cannot restart, layer, or queue the performance.
    constexpr unsigned pn=sizeof(piano_pcm)/sizeof(int16_t);
    std::vector<int16_t> song(2*(pn+32));
    auto silence=[&]{return std::all_of(song.begin(),song.end(),[](int x){return x==0;});};
    ttk_audio_reset();ttk::piano_sound(0,0,0);ttk_audio_mix(song.data(),100);
    ttk::piano_sound(0,0,0);ttk_audio_mix(song.data()+200,pn-100+32);
    for(unsigned i=0;i<pn+32;++i)assert(song[2*i]==(i<pn?piano_pcm[i]:0) && song[2*i]==song[2*i+1]);
    std::fill(song.begin(),song.end(),0);ttk_audio_mix(song.data(),pn);assert(silence());
    ttk::piano_sound(0,0,0);ttk::night_vision_sound();ttk_audio_mix(song.data(),pn);
    for(unsigned i=0;i<pn;++i)assert(song[2*i]==std::clamp(int(piano_pcm[i])+(i<n?night_vision_pcm[i]:0),-32768,32767));
    ttk_audio_reset();std::fill(song.begin(),song.end(),0);
    volume=8191;ttk::piano_sound(0,0,0);ttk_audio_mix(song.data(),pn);
    for(unsigned i=0;i<pn;++i)assert(song[2*i]==int(piano_pcm[i])*8191/0x3fff);
    std::fill(song.begin(),song.end(),0);volume=0;ttk::piano_sound(0,0,0);ttk_audio_mix(song.data(),pn);assert(silence());
    volume=0x3fff;ttk_audio_mix(song.data(),pn);assert(silence());
    for(int why=0;why<5;++why) {
        ttk::piano_sound(0,0,0);
        if(why==0)++loads;
        if(why==1)gameplay=false;
        if(why==2)modern=false;
        if(why==3)ttk_audio_reset();
        if(why==4)ttk::piano_audio_reset();
        ttk_audio_mix(song.data(),pn);assert(silence());
        gameplay=modern=true;ttk_audio_mix(song.data(),pn);assert(silence());
    }
    // Disabled profiles/contexts cannot queue a delayed performance.
    modern=false;ttk::piano_sound(0,0,0);modern=true;ttk_audio_mix(song.data(),pn);assert(silence());
    gameplay=false;ttk::piano_sound(0,0,0);gameplay=true;ttk_audio_mix(song.data(),pn);assert(silence());
    ttk::piano_sound(0,0,0);ttk::piano_audio_mix(nullptr,1);ttk::piano_audio_mix(song.data(),0);
    ttk_audio_mix(song.data(),pn);
    for(unsigned i=0;i<pn;++i)assert(song[2*i]==piano_pcm[i]);
    // Fixed source, three-dimensional smooth falloff, and silent playback
    // advancing outside the radius. A repeat cannot move the source to Duke.
    for(int axis=0;axis<3;++axis) for(int distance:{0,768,2432,4096,8192}) {
        ttk_audio_reset();std::fill(song.begin(),song.end(),0);
        listener[axis]=distance;ttk::piano_sound(0,0,0);ttk_audio_mix(song.data(),pn);
        const double expected=distance<=768?1:distance==2432?.5:0;
        for(unsigned i=0;i<pn;++i)assert(song[2*i]==int(int(piano_pcm[i])*expected));
        listener[axis]=0;
    }
    ttk_audio_reset();std::fill(song.begin(),song.end(),0);
    listener[0]=5000;ttk::piano_sound(0,0,0);ttk_audio_mix(song.data(),2000);assert(silence());
    ttk::piano_sound(5000,0,0);ttk_audio_mix(song.data(),2000);assert(silence());
    listener[0]=0;ttk_audio_mix(song.data(),1000); // fade back in, no restart
    std::fill(song.begin(),song.end(),0);ttk_audio_mix(song.data(),1000);
    for(unsigned i=0;i<1000;++i)assert(song[2*i]==piano_pcm[5000+i]);
    listener[0]=5000;ttk_audio_mix(song.data(),1000); // fade out
    std::fill(song.begin(),song.end(),0);ttk_audio_mix(song.data(),1000);assert(silence());
    std::puts("PASS: D38 fixed-source XYZ proximity, full/half/zero range, smooth transitions, silent cursor progression and no source relocation");
    std::puts("PASS: D38 full piano PCM, chunk boundaries, repeat suppression, gadget coexistence, effects gain/mute, pause/load/world/reset/Vanilla cancellation");
    std::puts("PASS: Bio Mask exact PCM, volume/mute, pause/load/Off/Vanilla, restart and face-cue replacement");
    std::puts("PASS: selected PCM, one shot, live effects volume/mute, load/pause/reset/Original/Vanilla, restart and saturation");
}
