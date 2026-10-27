#include "concurrent_music.h"
#include "pc_input.h"
#include "code_identity.h"
#include "cdrom.h"
#include "spu.h"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" uint8_t* g_psx_ram;
namespace {
struct Guard {uint32_t address,size;const char* digest;};
const Guard guards[]={{0x8003b780,0xd4c,"296a0bb7eacd8327cfc9ff79989ce414c984d1d063ba0905c5ee5c4f42f7aab6"}};
std::array<std::vector<uint32_t>,1> expected;
constexpr int ends[]={162176,163257,160442,164435,156780};
struct Stream {
    int track=-1,sector=0,history[4]={},count=0,cursor=0;
    uint32_t loads=0;
    bool failed=false;
    uint64_t frames=0,overlap=0,limited=0;
    double limiter=1.0;
    int16_t pcm[9408*2]={};
} stream;
bool suspended=false;
bool trace() {return std::getenv("DNTTK_MUSIC_TRACE")!=nullptr;}
bool allowed() {
    const char* compare=std::getenv("DNTTK_CONCURRENT_MUSIC");
    return ttk::input_modernized() && (!compare || std::strcmp(compare,"0")) &&
        psx_mod_read_half(0x800bcbb0)==1 &&
        ttk::code_identity(guards,expected,psx_mod_read_word,g_psx_ram);
}
void clear() {
    if(trace() && stream.track>=0)
        std::fprintf(stderr,"[TTK music] stop track=%d frames=%llu overlap=%llu limited=%llu\n",
            stream.track,(unsigned long long)stream.frames,(unsigned long long)stream.overlap,
            (unsigned long long)stream.limited);
    stream=Stream{};
}
bool next_sector() {
    // Channels are interleaved every eight sectors. Bound malformed-disc work.
    for(int attempts=0;attempts<32;++attempts) {
        if(stream.sector>=ends[stream.track]) {
            stream.sector=144480;
            std::fill(std::begin(stream.history),std::end(stream.history),0);
        }
        int n=cdrom_host_xa_read(stream.sector++,1,stream.track+1,stream.history,stream.pcm);
        if(n<0)break;
        if(n>0) {stream.count=n;stream.cursor=0;return true;}
    }
    stream.failed=true;
    std::fprintf(stderr,"[TTK music] independent stream unavailable; native playback retained\n");
    return false;
}
bool prepare() {
    if(!allowed()) {if(stream.track>=0)clear();return false;}
    const int state=psx_mod_read_half(0x800c26a8);
    const int track=psx_mod_read_half(0x800c26b4);
    const auto loads=psx_mod_savestate_loads();
    if(state==0 || track<0 || track>=5 || psx_mod_read_word(0x800d7178)!=23 ||
        psx_mod_read_word(0x800d7194)!=144480 ||
        psx_mod_read_word(0x800d7180)!=unsigned(ends[track]) ||
        psx_mod_read_word(0x800d718c)!=unsigned(track+1)) {
        if(stream.track>=0)clear();return false;
    }
    if(stream.track!=track || stream.loads!=loads) {
        clear();stream.track=track;stream.loads=loads;
        stream.sector=std::clamp(int(psx_mod_read_word(0x800d7184)),144480,ends[track]-1);
        if(!next_sector())return false;
        if(trace())std::fprintf(stderr,"[TTK music] start track=%d sector=%d state=%d loads=%u\n",
            track,stream.sector-1,state,loads);
    }
    return !stream.failed;
}
int suppress(int lba,int file,int channel) {
    return !suspended && prepare() && file==1 && channel==stream.track+1 &&
        lba>=144480 && lba<ends[stream.track];
}
}
extern "C" void ttk_music_update() {
    suspended=false;
    cdrom_set_host_xa_filter(suppress);
    if(!prepare())return;
    const int state=psx_mod_read_half(0x800c26a8);
    if(state!=5 && state!=6 && state!=7)return;
    // The original MUSIC slider previews onto the shared CD gain, even during
    // speech. With two buses the native voice must retain SPEECH instead. This
    // also applies a speech-slider edit to an already playing line on resume.
    const uint32_t gain=std::clamp(int32_t(psx_mod_read_word(0x800c39e8)),0,16383);
    for(uint32_t addr:{0x1f801db0u,0x1f801db2u})
        if((spu_read(addr)&0xffffu)!=gain)spu_write(addr,gain);
}
extern "C" void ttk_music_reset() {clear();suspended=true;}
extern "C" void ttk_music_mix(int16_t* stereo,int frames) {
    suspended=false;
    cdrom_set_host_xa_filter(suppress);
    if(!stereo || frames<=0 || !prepare())return;
    const int state=psx_mod_read_half(0x800c26a8);
    // Pause owns time, including pause while a voice seek is pending. Preserve
    // the cursor until resume; leaving gameplay/death/load resets ownership.
    if(state==9 || psx_mod_read_half(0x800be568) || psx_mod_read_half(0x800d2540))return;
    if(state!=1 && state!=2 && state!=3 && state!=5 && state!=6 && state!=7)return;
    SpuDebugInfo spu{};spu_debug_info(&spu);
    if(!(spu.ctrl&0x8000))return;
    const int gain=std::clamp(int32_t(psx_mod_read_word(0x800c39e4)),0,16383);
    const bool mono=psx_mod_read_word(0x800c39ec)==1;
    const auto before=stream.frames/44100;
    for(int i=0;i<frames;++i) {
        if(stream.cursor>=stream.count && !next_sector())break;
        int l=stream.pcm[2*stream.cursor],r=stream.pcm[2*stream.cursor+1];
        ++stream.cursor;++stream.frames;
        if(state==5 || state==6 || state==7)++stream.overlap;
        if(mono)l=r=(l+r)/2;
        // Native CD input gain is signed 1.15. Debug main gain is 1.14.
        // This separate bus uses MUSIC even while native XA uses SPEECH.
        const int ml=int((int64_t(l)*gain*spu.main_l)/(32768LL*16384));
        const int mr=int((int64_t(r)*gain*spu.main_r)/(32768LL*16384));
        const int ol=int(stereo[2*i])+ml,orr=int(stereo[2*i+1])+mr;
        // Linked stereo peak protection adds no buffering/latency. Immediate
        // attack, 50 ms linear release; ordinary levels remain unity. Native
        // effects can already reach full scale before adding the music bus.
        const int peak=std::max(std::abs(ol),std::abs(orr));
        const double ceiling=peak>32760 ? 32760.0/peak : 1.0;
        stream.limiter=gain ? std::min(ceiling,stream.limiter+1.0/2205) : 1.0;
        stream.limited+=stream.limiter<1.0;
        stereo[2*i]=int16_t(std::clamp(int(ol*stream.limiter),-32768,32767));
        stereo[2*i+1]=int16_t(std::clamp(int(orr*stream.limiter),-32768,32767));
    }
    if(trace() && stream.frames/44100!=before)
        std::fprintf(stderr,"[TTK music] playing track=%d state=%d sector=%d frames=%llu overlap=%llu limited=%llu gain=%d main=%d,%d\n",
            stream.track,state,stream.sector-1,(unsigned long long)stream.frames,
            (unsigned long long)stream.overlap,(unsigned long long)stream.limited,gain,spu.main_l,spu.main_r);
}
