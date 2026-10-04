// Collision fixture checks for the actual adapter; not retail geometry proof.
#include "weapon_aim.h"
#include "modern_controls.h"
#include "pc_input.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <cstdio>
extern "C" {int g_precise_mode=0,g_ls_mode=0,g_psx_call_bail=0;}
static unsigned char ram[0x200000],scratch[1024];
static PSXModFunctionEntryCallback callback;
static bool ready=true,modern=true,cover=false,jet_ready=false;
static unsigned writes;
static bool actor_visible, block_actor, block_muzzle, close_actor, behind_cover;
static unsigned char& byte(uint32_t a) {return (a&0x1ffffc00)==0x1f800000?scratch[a&1023]:ram[a&0x1fffff];}
extern "C" uint8_t psx_mod_read_byte(uint32_t a){return byte(a);}
extern "C" uint16_t psx_mod_read_half(uint32_t a){return byte(a)|(byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a){return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
extern "C" {uint8_t* g_psx_ram=ram;uint32_t g_dirty_ram_code_gen=1;}
static void pokeb(uint32_t a){byte(a)^=1;++g_dirty_ram_code_gen;}
extern "C" void psx_mod_write_byte(uint32_t a,uint8_t v){byte(a)=v;++writes;++g_dirty_ram_code_gen;}
extern "C" void psx_mod_write_half(uint32_t a,uint16_t v){psx_mod_write_byte(a,v);psx_mod_write_byte(a+1,v>>8);}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v){psx_mod_write_half(a,v);psx_mod_write_half(a+2,v>>16);}
extern "C" uint32_t psx_mod_alloc_guest_memory(uint32_t n,uint32_t){static uint32_t next=0x801e0000;auto a=next;next+=(n+15)&~15;return a;}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t a,PSXModFunctionEntryCallback cb){if(a==0x8003c500)callback=cb;return 1;}
namespace ttk {static InputFrame f;const InputFrame& input_snapshot(Context){return f;}uint64_t input_host_frame(){return f.sequence;}bool input_modernized(){return modern;}bool movement_ready(){return ready;}bool locomotion_input_ready(){return false;}bool jetpack_input_ready(){return modern && jet_ready;}bool player_identity_ready(){return ready;}bool frame_trace_on(){return false;}void frame_trace_account(uint32_t,long){}}
extern "C" void psx_dispatch_call(CPUState* c,uint32_t address,uint32_t ret) {
    assert(address==0x8006d980 && ret==0x800000fc && c->gpr[4]==0x800d7198);
    uint32_t a=c->gpr[5],sp=c->gpr[29];
    double from[3],to[3],t=1;
    for(int i=0;i<3;++i){from[i]=(int32_t)psx_mod_read_word(a+4*i);to[i]=(int32_t)psx_mod_read_word(a+16+4*i);}
    // Infinite forward wall, plus a thin wall between actor and protruding muzzle.
    if(to[2]>1000 && from[2]<1000)t=(1000-from[2])/(to[2]-from[2]);
    if(cover && from[0]<50 && to[0]>50)t=std::fmin(t,(50-from[0])/(to[0]-from[0]));
    uint32_t close_hit=0;
    if(behind_cover && from[2]<-500 && to[2]>-500)t=std::fmin(t,(-500-from[2])/(to[2]-from[2]));
    if(close_actor) {
        // Sphere: the offset muzzle's entry is deeper than the screen ray's
        // front face. Clamping to that entry previously produced a backwards shot.
        double d[3],q[3],aa=0,bb=0,cc=-200*200;
        for(int i=0;i<3;++i){d[i]=to[i]-from[i];q[i]=from[i]-(i==2?500:0);aa+=d[i]*d[i];bb+=2*q[i]*d[i];cc+=q[i]*q[i];}
        double disc=bb*bb-4*aa*cc;
        if(aa>0 && disc>=0){double near=(-bb-std::sqrt(disc))/(2*aa);if(near>=0 && near<t){t=near;close_hit=0x800e0000;}}
    }
    c->gpr[2]=close_hit?close_hit:actor_visible && to[2]==500 ? ((block_actor || (block_muzzle && from[0]==100))?1:0x800e0000) : (t<1?1:0);psx_mod_write_word(c->gpr[7],2);
    for(int i=0;i<3;++i)psx_mod_write_word(psx_mod_read_word(sp+16)+4*i,std::lround(from[i]+t*(to[i]-from[i])));
    c->gte_data[0]=0xdeadbeef; // copying CPU must isolate query temporaries
}
int main(int argc,char**argv) {
    assert(argc==2);std::ifstream f(argv[1],std::ios::binary);f.seekg(2048);f.read((char*)ram+0x10000,0xbb000);assert(f.gcount()==0xbb000);
    constexpr uint32_t p=0x800d7198,cam=0x800d6eb0,sp=0x801ffe50;
    setenv("DNTTK_WEAPON_AIM","view",1);
    CPUState c{};c.gpr[29]=sp;c.gpr[31]=0x8004f7b4;c.gpr[4]=p;c.gpr[5]=sp+0x40;c.gpr[6]=p+0x144;c.gpr[7]=4;
    auto seed=[&](){c.gpr[6]=p+0x144;psx_mod_write_word(sp+0x40,100);psx_mod_write_word(sp+0x44,0);psx_mod_write_word(sp+0x48,0);psx_mod_write_word(p+0xbc,100);psx_mod_write_word(p+0xc0,0);psx_mod_write_word(p+0xc4,0);};
    for(int i=0;i<3;++i){psx_mod_write_word(p+4+4*i,0);psx_mod_write_word(cam+0x14+4*i,0);psx_mod_write_half(cam+12+2*i,i==2?4096:0);}
    psx_mod_write_byte(p+0x2e,2);seed();
    modern=false;writes=0;callback(&c,0x8003c500);assert(writes==0);modern=true;
    ttk::f.held[ttk::original_aim]=true;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());ttk::f.held[ttk::original_aim]=false;
    g_precise_mode=1;callback(&c,0x8003c500);assert(writes==0);g_precise_mode=0;
    ready=false;callback(&c,0x8003c500);assert(writes==0);ready=true;
    c.gpr[4]=p+0x8a4;callback(&c,0x8003c500);assert(writes==0);c.gpr[4]=p;
    for(unsigned weapon:{9u,10u,13u,14u}){c.gpr[7]=weapon;callback(&c,0x8003c500);assert(writes==0);}c.gpr[7]=4;
    pokeb(0x8003c500);callback(&c,0x8003c500);assert(writes==0);pokeb(0x8003c500);
    unsigned char stack_before[4096],scratch_before[1024];
    std::memcpy(stack_before,ram+(sp&0x1fffff)-4096,4096);std::memcpy(scratch_before,scratch,1024);
    callback(&c,0x8003c500);
    assert(!std::memcmp(stack_before,ram+(sp&0x1fffff)-4096,4096));assert(!std::memcmp(scratch_before,scratch,1024));
    assert(c.gpr[6]==0x801e0000 && c.gte_data[0]==0);
    int x=(int32_t)psx_mod_read_word(c.gpr[6]),z=psx_mod_read_word(c.gpr[6]+8);
    assert(x<0 && std::abs(100+1000.0*x/z)<1); // parallax converges at wall
    assert(psx_mod_read_word(sp+0x40)==100); // muzzle offset preserved
    seed();psx_mod_write_word(sp+0x40,500);callback(&c,0x8003c500);
    assert(psx_mod_read_word(sp+0x40)==100); // precision-camera origin cannot teleport the muzzle
    seed();cover=true;callback(&c,0x8003c500);
    assert(psx_mod_read_word(sp+0x40)==42); // retract from near cover, never teleport forward
    psx_mod_write_byte(p+0x3b8,2);psx_mod_write_byte(p+0x3b9,4);
    psx_mod_write_word(p+0x224,0);assert(ttk_aim_reticle()); // walking is still armed
    ttk::f.held[ttk::original_aim]=true;assert(!ttk_aim_reticle());ttk::f.held[ttk::original_aim]=false;
    psx_mod_write_byte(p+0x3b8,0);assert(!ttk_aim_reticle());
    psx_mod_write_byte(p+0x3b8,1);assert(!ttk_aim_reticle()); // held inventory object
    // D08Q3: flight has no ground lease. Both gun callers still converge
    // on the camera ray, including vertical aim, with independent reticle
    // preference and unchanged guard/original-aim rejection.
    ready=false;jet_ready=true;cover=false;
    psx_mod_write_byte(p+0x3b8,2);
    for(unsigned weapon:{4u,5u,8u,11u,28u,29u}) {
        c.gpr[7]=weapon;c.gpr[31]=weapon==4?0x8004f7b4:0x8004fa88;
        psx_mod_write_byte(p+0x3b9,weapon);
        for(int pitch:{-2048,0,2048}) {
            psx_mod_write_half(cam+14,pitch);seed();callback(&c,0x8003c500);
            assert(c.gpr[6]==0x801e0000 && ttk_aim_reticle());
            const int vy=(int32_t)psx_mod_read_word(c.gpr[6]+4);
            assert(pitch==0?vy==0:vy*pitch>0);
        }
    }
    psx_mod_write_half(cam+14,0);c.gpr[7]=4;c.gpr[31]=0x8004f7b4;
    setenv("DNTTK_CROSSHAIR","0",1);assert(!ttk_aim_reticle());
    seed();callback(&c,0x8003c500);assert(c.gpr[6]==0x801e0000);
    setenv("DNTTK_CROSSHAIR","1",1);
    ttk::f.held[ttk::original_aim]=true;seed();writes=0;callback(&c,0x8003c500);
    assert(writes==0 && !ttk_aim_reticle());ttk::f.held[ttk::original_aim]=false;
    pokeb(0x8003c500);seed();writes=0;callback(&c,0x8003c500);
    assert(writes==0 && !ttk_aim_reticle());pokeb(0x8003c500);
    modern=false;seed();writes=0;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());modern=true;
    jet_ready=false;seed();writes=0;callback(&c,0x8003c500);assert(writes==0 && !ttk_aim_reticle());
    ready=true;psx_mod_write_byte(p+0x3b9,4);
    std::puts("PASS: D08Q3 flight shots, vertical aim, crosshair preference and guarded fallback");
    // The ordinary shotgun/rifle/Gatling/RPG handler has a distinct call site.
    for(unsigned weapon:{5u,6u,7u,8u,11u}) {
        c.gpr[31]=0x8004fa88;c.gpr[7]=weapon;seed();callback(&c,0x8003c500);
        assert(c.gpr[6]==0x801e0000);
    }
    c.gpr[31]=0x8004fa84;seed();writes=0;callback(&c,0x8003c500);assert(writes==0);
    c.gpr[31]=0x8004fa88;pokeb(0x8004fa80);seed();writes=0;callback(&c,0x8003c500);assert(writes==0);pokeb(0x8004fa80);
    c.gpr[31]=0x8004f7b4;c.gpr[7]=4;
    seed();c.gpr[31]=0x8004e7e4;c.gpr[7]=12;c.gpr[5]=sp+0x30;c.gpr[6]=sp+0x20;
    psx_mod_write_word(sp+0x10,321);psx_mod_write_word(sp+0x14,99);
    callback(&c,0x8003c500);assert(c.gpr[6]==0x801e0000);
    assert(psx_mod_read_word(sp+0x10)==321 && psx_mod_read_word(sp+0x14)==99);
    c.gpr[5]=sp+0x40;c.gpr[6]=sp+0x20;writes=0;callback(&c,0x8003c500);assert(writes==0);
    c.gpr[31]=0x8004f7b4;c.gpr[7]=4;seed();
    // Independent marker controls cannot alter supported free-view shot vectors.
    cover=false;psx_mod_write_byte(p+0x3b8,2);
    setenv("DNTTK_CROSSHAIR","0",1);assert(!ttk_aim_reticle());
    seed();callback(&c,0x8003c500);int free_x=(int32_t)psx_mod_read_word(c.gpr[6]);
    setenv("DNTTK_RED_DOT","0",1);seed();callback(&c,0x8003c500);
    assert((int32_t)psx_mod_read_word(c.gpr[6])==free_x);
    setenv("DNTTK_AIM_ASSIST","original-lock",1);
    psx_mod_write_word(p+0x288,0x800e0000);psx_mod_write_word(p+0x2b0,1);
    psx_mod_write_word(0x800e00ec,20);psx_mod_write_word(0x800e00f0,0);psx_mod_write_word(0x800e00f4,500);
    actor_visible=true;seed();callback(&c,0x8003c500);
    int assist_x=(int32_t)psx_mod_read_word(c.gpr[6]);
    assert(assist_x<free_x && std::abs(100+500.0*assist_x/(int32_t)psx_mod_read_word(c.gpr[6]+8)-20)<1);
    setenv("DNTTK_CROSSHAIR","1",1);setenv("DNTTK_RED_DOT","1",1);
    seed();callback(&c,0x8003c500);assert((int32_t)psx_mod_read_word(c.gpr[6])==assist_x);
    block_actor=true;seed();callback(&c,0x8003c500);assert((int32_t)psx_mod_read_word(c.gpr[6])==free_x);
    block_actor=false;block_muzzle=true;seed();callback(&c,0x8003c500);assert((int32_t)psx_mod_read_word(c.gpr[6])==free_x);
    block_muzzle=false;psx_mod_write_word(0x800e00ec,300); // outside six-degree cone
    seed();callback(&c,0x8003c500);assert((int32_t)psx_mod_read_word(c.gpr[6])==free_x);
    psx_mod_write_word(0x800e00ec,20);psx_mod_write_word(0x800e0000,2); // dead target
    seed();callback(&c,0x8003c500);assert((int32_t)psx_mod_read_word(c.gpr[6])==free_x);
    // Point-blank overlap must begin behind the whole actor, then hit forwards.
    unsetenv("DNTTK_AIM_ASSIST");actor_visible=false;close_actor=true;seed();
    psx_mod_write_word(cam+0x1c,-1000);psx_mod_write_word(p+0xc4,450);
    callback(&c,0x8003c500);
    std::printf("Point-blank sphere: forward direction=%d, origin=(%d,%d)\n",
        (int32_t)psx_mod_read_word(c.gpr[6]+8),(int32_t)psx_mod_read_word(sp+0x40),(int32_t)psx_mod_read_word(sp+0x48));std::fflush(stdout);
    assert((int32_t)psx_mod_read_word(c.gpr[6]+8)>4000);
    assert(psx_mod_read_word(sp+0x40)==0 && psx_mod_read_word(sp+0x48)==0);
    // Camera obstruction behind Duke cannot reverse a forward shot.
    close_actor=false;behind_cover=true;seed();callback(&c,0x8003c500);
    assert((int32_t)psx_mod_read_word(c.gpr[6]+8)>0);
    behind_cover=false;psx_mod_write_word(cam+0x1c,0);
    // Only the guarded marker's pending quad may collapse; aim fields untouched.
    // D07D: ownership comes from the enqueue frame alone (S4 actor, marker
    // frame RA), with no entry lease, camera link or gameplay gate.
    CPUState marker{};marker.gpr[29]=sp-0x48;marker.gpr[31]=0x80033db0;marker.gpr[20]=p;
    marker.gpr[5]=marker.gpr[16]=0x801d0000;psx_mod_write_word(sp-0x48+0x40,0x8003543c);
    psx_mod_write_byte(0x801d0003,9);psx_mod_write_byte(0x801d0007,0x2c);
    auto quad=[&](){for(unsigned offset:{8u,16u,24u,32u})psx_mod_write_word(0x801d0000+offset,0x00640064);};
    auto hidden=[&](){for(unsigned offset:{8u,16u,24u,32u})if(psx_mod_read_word(0x801d0000+offset))return false;return true;};
    setenv("DNTTK_RED_DOT","1",1);quad();writes=0;callback(&marker,0x8002bc18);assert(writes==0 && !hidden());
    setenv("DNTTK_RED_DOT","0",1);ready=false;g_precise_mode=1;
    callback(&marker,0x8002bc18);assert(hidden());
    assert(psx_mod_read_word(p+0x288)==0x800e0000 && psx_mod_read_word(p+0x2b0)==1);
    ready=true;g_precise_mode=0;
    marker.gpr[20]=0x800e0000;quad();writes=0;callback(&marker,0x8002bc18);assert(writes==0); // another actor
    marker.gpr[20]=p;psx_mod_write_word(sp-0x48+0x40,0x80035440);writes=0;callback(&marker,0x8002bc18);assert(writes==0); // foreign frame
    psx_mod_write_word(sp-0x48+0x40,0x8003543c);g_ls_mode=1;writes=0;callback(&marker,0x8002bc18);assert(writes==0);
    g_ls_mode=0;modern=false;callback(&marker,0x8002bc18);assert(writes==0); // Vanilla keeps the dot
    modern=true;pokeb(0x80033b00);writes=0;callback(&marker,0x8002bc18);assert(writes==0); // changed marker code
    pokeb(0x80033b00);callback(&marker,0x8002bc18);assert(hidden());
    // Authenticated energy constructor completion: preserve physical origin,
    // eliminate implicit retargeting and keep only the free-view single beam.
    ready=false;jet_ready=true; // beam completion must also accept flight
    c.gpr[31]=0x80058b80;c.gpr[7]=10;c.gpr[5]=p+0x80c;c.gpr[6]=p+0x144;
    psx_mod_write_word(p+0x80c,100);psx_mod_write_word(p+0x810,0);psx_mod_write_word(p+0x814,0);
    ttk::f.active=true;callback(&c,0x8003c500);
    assert(c.gpr[5]!=p+0x80c && psx_mod_read_word(p+0x80c)==100);
    CPUState beam{};beam.gpr[29]=sp-0x110;beam.gpr[5]=beam.gpr[29]+0x10;
    beam.gpr[16]=0x800da978;beam.gpr[23]=p;beam.gpr[20]=3;
    psx_mod_write_word(0x800da988,p);psx_mod_write_byte(0x800da9a8,15);
    psx_mod_write_word(0x800da990,p+0x80c);psx_mod_write_word(0x800da98c,0x800e0000);
    writes=0;beam.gpr[31]=0x8007337c;callback(&beam,0x8001ca4c);assert(writes==0);
    beam.gpr[31]=0x80073380;callback(&beam,0x8001ca4c);
    assert(psx_mod_read_word(0x800da98c)==0 && psx_mod_read_word(0x800da990)!=p+0x80c && beam.gpr[20]==1);
    writes=0;callback(&beam,0x8001ca4c);assert(writes==0); // consumed lease
    setenv("DNTTK_WEAPON_AIM","original",1);seed();writes=0;callback(&c,0x8003c500);assert(writes==0);
    std::puts("weapon aim guards, parallax, muzzle obstruction and original fallback passed");
}
