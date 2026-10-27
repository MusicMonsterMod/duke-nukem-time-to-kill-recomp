#include "concurrent_music.h"
#include "cdrom.h"
#include "spu.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <vector>
#include <cstring>
static std::vector<uint8_t> ram(2*1024*1024);
extern "C" {uint8_t* g_psx_ram=ram.data();}
static bool modern=true,fail=false;
static uint32_t loads;
static int reads,last_sector;
static uint32_t cd_gain=16383;
static int (*filter)(int,int,int);
namespace ttk {bool input_modernized(){return modern;}}
extern "C" uint32_t psx_mod_read_word(uint32_t a){uint32_t v;std::memcpy(&v,&ram[a&0x1fffff],4);return v;}
extern "C" uint16_t psx_mod_read_half(uint32_t a){uint16_t v;std::memcpy(&v,&ram[a&0x1fffff],2);return v;}
extern "C" uint32_t psx_mod_savestate_loads(){return loads;}
static void put(uint32_t a,uint32_t v){std::memcpy(&ram[a&0x1fffff],&v,4);}
extern "C" void cdrom_set_host_xa_filter(int (*f)(int,int,int)){filter=f;}
extern "C" int cdrom_host_xa_read(int sector,int file,int channel,int history[4],int16_t* out){
 ++reads;last_sector=sector;assert(file==1 && channel>=1 && channel<=5);
 if(fail)return -1;
 if((sector-144480)%8!=channel-1)return 0;
 ++history[0];
 for(int i=0;i<2352;++i){out[2*i]=1000+history[0];out[2*i+1]=2000+history[0];}
 return 2352;
}
extern "C" uint32_t spu_read(uint32_t addr){assert(addr==0x1f801db0 || addr==0x1f801db2);return cd_gain;}
extern "C" void spu_write(uint32_t addr,uint32_t value){assert(addr==0x1f801db0 || addr==0x1f801db2);cd_gain=value;}
extern "C" void spu_debug_info(SpuDebugInfo* out){*out={};out->ctrl=0x8001;out->main_l=out->main_r=16383;}
int main(int argc,char** argv){
 assert(argc==2);std::ifstream in(argv[1],std::ios::binary);assert(in);
 std::vector<uint8_t> exe((std::istreambuf_iterator<char>(in)),{});
 uint32_t base;std::memcpy(&base,exe.data()+24,4);
 std::copy(exe.begin()+2048,exe.end(),ram.begin()+(base&0x1fffff));
 auto setup=[] {put(0x800bcbb0,1);put(0x800be568,0);put(0x800d2540,0);put(0x800c26a8,2);
 put(0x800c26b4,0);put(0x800d7178,23);put(0x800d7194,144480);put(0x800d7180,162176);
 put(0x800d718c,1);put(0x800d7184,144480);put(0x800c39e4,16383);put(0x800c39ec,0);};
 setup();std::vector<int16_t> out(2*5000);
 auto mix=[&](int n=100){std::fill(out.begin(),out.end(),0);ttk_music_mix(out.data(),n);};
 auto silent=[&]{return std::all_of(out.begin(),out.end(),[](int16_t x){return x==0;});};
 mix();assert(out[0]>0 && out[1]>out[0]);assert(filter(144480,1,1));
 assert(!filter(124016,1,1) && !filter(144480,1,7) && !filter(144480,2,1));
 // Live MUSIC previews cannot overwrite SPEECH, including mute/unmute.
 put(0x800c26a8,6);put(0x800c39e8,4321);ttk_music_update();assert(cd_gain==4321);
 put(0x800c39e8,0);ttk_music_update();assert(cd_gain==0);
 put(0x800c39e8,16383);ttk_music_update();assert(cd_gain==16383);
 modern=false;put(0x800c39e8,0);ttk_music_update();assert(cd_gain==16383);modern=true;
 // All voice/seek states keep the independent cursor; stale native resume
 // position does not repeatedly rewind it. No special handling of speech gain.
 for(int state:{5,6,7,1,3,2}){put(0x800c26a8,state);mix(2500);assert(!silent());}
 assert(last_sector>144480);const int before=reads;
 put(0x800c26a8,9);mix(5000);assert(silent() && reads==before);
 put(0x800c26a8,6);put(0x800be568,1);mix(5000);assert(silent() && reads==before);
 put(0x800be568,0);put(0x800c39e4,0);mix(5000);assert(silent() && reads>before);
 put(0x800c39e4,16383);put(0x800c39ec,1);mix();assert(out[0]==out[1] && out[0]>0);
 // Track changes and loads discard predictors/buffer; loop endpoint is bounded.
 put(0x800c26b4,1);put(0x800d718c,2);put(0x800d7180,163257);put(0x800d7184,163250);mix(5000);
 assert(last_sector<144520);++loads;put(0x800d7184,144480);mix();assert(last_sector==144481);
 // Bad data/read failures fall back without silencing native audio.
 fail=true;++loads;mix();assert(silent() && !filter(144481,1,2));fail=false;
 ++loads;mix();assert(!silent());
 modern=false;mix();assert(silent() && !filter(144481,1,2));modern=true;
 put(0x800bcbb0,0);mix();assert(silent());setup();
 put(0x800c26a8,0);mix();assert(silent());setup();
 const uint32_t word=psx_mod_read_word(0x8003b780);put(0x8003b780,word^1);mix();assert(silent());
 put(0x8003b780,word);mix();assert(!silent());
 // Final mixing saturates rather than wrapping.
 std::fill(out.begin(),out.end(),32760);ttk_music_mix(out.data(),100);assert(out[0]>32000 && out[0]<=32760);
 ttk_music_reset();assert(!filter(144480,1,1));std::puts("concurrent music contracts PASS");
}
