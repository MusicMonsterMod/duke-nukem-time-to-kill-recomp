// D11B near-clip math: GTE-exact projection, safety classification and DPCS.
#include "near_clip.h"
#include "sky_render.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>
static uint8_t ram[0x200000];
static PSXModFunctionEntryCallback near_hook;
static bool live_eye=false;
static unsigned host_vertices;
extern "C" {uint8_t* g_psx_ram=ram;uint32_t g_dirty_ram_code_gen=1;}
extern "C" uint32_t psx_mod_alloc_guest_memory(uint32_t,uint32_t){return 0x801e0000;}
extern "C" uint8_t psx_mod_read_byte(uint32_t a){return ram[a&0x1fffff];}
extern "C" uint16_t psx_mod_read_half(uint32_t a){return psx_mod_read_byte(a)|(psx_mod_read_byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a){return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
extern "C" void psx_mod_write_byte(uint32_t a,uint8_t v){ram[a&0x1fffff]=v;}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v){for(int i=0;i<4;++i)ram[(a+i)&0x1fffff]=v>>(8*i);}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t,PSXModFunctionEntryCallback cb){near_hook=cb;return 1;}
extern "C" int psx_mod_gpu_host_vertex(uint32_t,uint32_t,int32_t,int32_t,float,float,float){return 1;}
extern "C" int psx_mod_gpu_host_vertex_depth(uint32_t,uint32_t,int32_t,int32_t,float,float,float,float){++host_vertices;return 1;}
extern "C" int psx_mod_replay_active(void){return 0;}
extern "C" uint32_t psx_mod_savestate_loads(void){return 0;}
namespace ttk {bool frame_trace_on(){return false;} void frame_trace_account(uint32_t,long){}}
namespace ttk {bool first_person_view_live(){return live_eye;} bool widescreen_near_clip_live(){return false;} bool first_person_duke_drawing(){return false;} bool first_person_actor_drawing(){return false;} bool first_person_weapon_drawing(){return false;} uint64_t input_host_frame(){return 0;}}
static int16_t sx(uint32_t p){return (int16_t)p;} static int16_t sy(uint32_t p){return (int16_t)(p>>16);}
// A whole-hook observation oracle: compare this digest across separate runs
// with the diagnostic gate disabled/enabled. Covers all fixture RAM and CPU
// changes, not just the count or locations of traced triangles.
static uint64_t packet_digest=14695981039346656037ull;
static void observed_hook(CPUState* cpu,uint32_t address) {
    near_hook(cpu,address);
    auto hash=[](const void* p,size_t n) {
        const auto* b=static_cast<const uint8_t*>(p);
        for(size_t i=0;i<n;++i) {packet_digest^=b[i];packet_digest*=1099511628211ull;}
    };
    hash(ram,sizeof ram);hash(cpu->gpr,sizeof cpu->gpr);
    hash(cpu->gte_ctrl,sizeof cpu->gte_ctrl);hash(&host_vertices,sizeof host_vertices);
}
// Optional owned-EXE integration: exercise the registered, identity-guarded
// hook and inspect emitted ordering-table packets, not a duplicate sort helper.
static void packet_contracts(const char* executable) {
    std::ifstream file(executable,std::ios::binary);
    const std::vector<unsigned char> exe((std::istreambuf_iterator<char>(file)),{});
    assert(exe.size()>0x800 && std::memcmp(exe.data(),"PS-X EXE",8)==0);
    uint32_t base=0;std::memcpy(&base,exe.data()+0x18,4);
    assert((base&0x1fffff)+exe.size()-0x800<=sizeof ram);
    std::memcpy(ram+(base&0x1fffff),exe.data()+0x800,exe.size()-0x800);
    // D17O: only the three resident sky matrix calls escape world-object
    // interpolation. Check the real code bytes and invalidation after restore.
    ttk::SkyRenderIdentity sky;
    assert(sky.valid(0,g_dirty_ram_code_gen,ram));
    for(uint32_t ra:{0x80038c68u,0x80038d7cu,0x80038e18u}) {
        assert(ttk::sky_transform_call(0x800d6eb0u,ra));
        assert(!ttk::sky_transform_call(0x800d7010u,ra));
        assert(!ttk::sky_transform_call(0x800d6eb0u,ra+4));
    }
    assert(!ttk::sky_transform_call(0x800d6eb0u,0x8003886cu));
    assert(!ttk::sky_transform_call(0x800d6eb0u,0x80038f5cu));
    const uint32_t sky_call=psx_mod_read_word(0x80038d74);
    psx_mod_write_word(0x80038d74,sky_call^1);++g_dirty_ram_code_gen;
    assert(!sky.valid(0,g_dirty_ram_code_gen,ram));
    psx_mod_write_word(0x80038d74,sky_call);++g_dirty_ram_code_gen;
    assert(sky.valid(0,g_dirty_ram_code_gen,ram));
    psx_mod_write_word(0x80038d74,sky_call^1);
    assert(!sky.valid(1,g_dirty_ram_code_gen,ram)); // already-dirty code, next frame
    psx_mod_write_word(0x80038d74,sky_call);++g_dirty_ram_code_gen;
    constexpr uint32_t mesh=0x80140000,verts=mesh+0x80,list=mesh+0x100;
    constexpr uint32_t ctx=0x800d67a8,ot=0x800d27a0,bm=0x800d26a0;
    auto word=[](uint32_t a,uint32_t v){psx_mod_write_word(a,v);};
    auto reset=[&] {
        std::memset(ram+(ot&0x1fffff),0,2048*8);
        std::memset(ram+(bm&0x1fffff),0,0x100);
        std::memset(ram+(ctx&0x1fffff),0,0x80);
        std::memset(ram+(mesh&0x1fffff),0,0x200);
        word(ctx,0x80160000);word(ctx+4,0x80160000);word(ctx+8,0x80180000);
        word(ctx+0x78,0xffff);word(ctx+0x54,0x808080);word(ctx+0x58,0x80150000);
        word(0x80150000,0);word(0x80150004,0x0010001f);word(0x80150008,0x1f1f1f00);
        ++g_dirty_ram_code_gen;host_vertices=0;
        CPUState cpu{};cpu.gpr[4]=mesh;cpu.gpr[5]=ot;cpu.gpr[6]=ctx;cpu.gpr[7]=bm;
        cpu.gte_ctrl[0]=4096;cpu.gte_ctrl[2]=4096;cpu.gte_ctrl[4]=4096;
        cpu.gte_ctrl[26]=256;cpu.gte_ctrl[29]=341;cpu.gte_ctrl[30]=256;
        return cpu;
    };
    live_eye=true;
    auto cpu=reset();cpu.gpr[31]=0x80037344;
    word(mesh+0x14,3);word(mesh+0x18,0x40);
    // Source depths 1024,2048,4096. All clipped/subdivided children must
    // retain the original world slot 128, behind a native decal at slot 80.
    word(mesh+0x1c,0x0001003f);word(mesh+0x20,0x34808080);
    word(mesh+0x24,0x00020001);word(mesh+0x28,0x34808080);
    word(mesh+0x2c,0x00040100);word(mesh+0x30,0x34808080);
    word(mesh+0x40,0x00010060);word(mesh+0x44,0x00020100);
    word(mesh+0x48,0x02000000);word(mesh+0x4c,0xff);
    observed_hook(&cpu,0x80011020);
    assert(cpu.gpr[4]!=mesh && host_vertices>3);
    assert(psx_mod_read_word(ot+128*8)!=0);
    for(unsigned i=0;i<2048;++i)if(i!=128)assert(psx_mod_read_word(ot+i*8)==0);
    auto object_case=[&](uint32_t ra,int extent,bool expect_taken) {
        auto c=reset();c.gpr[31]=ra;c.gpr[18]=0x80180000+extent*0x60;
        psx_mod_write_byte(mesh+6,3);word(mesh+0x10,verts);word(mesh+0x14,list);
        word(verts,(uint16_t)-extent|uint32_t(uint16_t(-extent))<<16);word(verts+4,5000);
        word(verts+8,extent|uint32_t(uint16_t(-extent))<<16);word(verts+12,5000);
        word(verts+16,uint32_t(extent)<<16);word(verts+20,5000);
        word(list,0x00010024);word(list+4,0x00020100);word(list+8,0);
        word(list+12,0x0200001f);word(list+16,0x1f00);word(list+20,0xff);
        observed_hook(&c,0x80010000);
        assert((c.gpr[4]!=mesh)==expect_taken);
        assert((host_vertices>0)==expect_taken);
    };
    object_case(0x80032288,100,true); // compact static prop beyond near radius
    object_case(0x80032288,128,true); // inclusive 256-unit extent
    object_case(0x80032288,129,false); // larger scenery stays original
    object_case(0x80033778,100,false); // dancer's articulated mesh
    object_case(0x800351f4,100,false); // actor joint
    object_case(0x80032288,400,false); // distant large scenery stays original
    // Two opaque faces of a compact prop stay in one ordering bucket so
    // host/native draw state cannot alternate for each face. Translucent
    // faces must keep their independent ordering.
    for(bool semi:{false,true}) {
        auto c=reset();c.gpr[31]=0x80032288;
        psx_mod_write_byte(mesh+6,6);word(mesh+0x10,verts);word(mesh+0x14,list);
        for(int face=0;face<2;++face) {
            const uint32_t v=verts+face*24;
            word(v,(uint16_t)-100|uint32_t(uint16_t(-100))<<16);word(v+4,5000+face*200);
            word(v+8,100|uint32_t(uint16_t(-100))<<16);word(v+12,5000+face*200);
            word(v+16,100u<<16);word(v+20,5000+face*200);
            const uint32_t record=list+4+face*16;
            word(record,face?0x00050403:0x00020100);word(record+4,0);
            word(record+8,(semi?0x22000000:0x02000000)|0x1f);word(record+12,0x1f00);
        }
        word(list,0x00020024);word(list+36,0xff);
        observed_hook(&c,0x80010000);assert(host_vertices==6);
        unsigned buckets=0;for(unsigned i=0;i<2048;++i)buckets+=psx_mod_read_word(ot+i*8)!=0;
        assert(buckets==(semi?2u:1u));
    }
    live_eye=false;object_case(0x80032288,100,false); // Vanilla/original view
    live_eye=true;
    // Changed caller instruction must invalidate permission, even with the
    // expected return address and a previously authenticated code image.
    word(0x80032280,psx_mod_read_word(0x80032280)^1);++g_dirty_ram_code_gen;
    object_case(0x80032288,100,false);
    live_eye=false;
    std::printf("packet_digest=%016llx\n",(unsigned long long)packet_digest);
    std::puts("ttk-near-test owned packet contracts PASS");
}
int main(int argc,char** argv) {
    ttk::NearGte g{};
    for(int i=0;i<3;++i) g.r[i][i]=4096;
    g.h=256;g.fc[0]=16*200;g.fc[1]=0;g.fc[2]=16*50;
    // Ordinary vertex: q = 256/1000 in 16.16, SX = floor(100*q) = 25, SY = 12.
    auto a=ttk::near_project(g,100,50,1000);
    assert(a.safe && a.sz==1000 && sx(a.sxy)==25 && sy(a.sxy)==12);
    // Divide saturation (2*SZ <= H) and a vertex behind the eye are unsafe.
    assert(!ttk::near_project(g,10,10,128).safe);
    assert(!ttk::near_project(g,10,10,-500).safe);
    assert(ttk::near_project(g,10,10,-500).sz==0);
    assert(ttk::near_project(g,10,10,129).safe);
    // Far off to the side: exact GTE value saturates at -0x400/0x3ff, so unsafe.
    auto side=ttk::near_project(g,6000,0,1000);
    assert(!side.safe && sx(side.sxy)==0x3ff);
    // Translation and view position.
    g.tr[2]=2000;auto t=ttk::near_project(g,0,0,0);
    assert(t.safe && t.sz==2000 && t.view[2]==2000);
    // DPCS: IR0 0 keeps the color, 4096 reaches the far color (FC/16), code byte kept.
    assert(ttk::near_dpcs(g,0x34102030u,0)==0x34102030u);
    const uint32_t f=ttk::near_dpcs(g,0x34102030u,4096);
    assert((f&0xff)==200 && ((f>>8)&0xff)==0 && ((f>>16)&0xff)==50 && (f>>24)==0x34);
    // A floor and a shelf one unit apart must keep their depth order even
    // when each mesh snaps different corners to integer screen pixels.
    // Plane y+z=2000, then a parallel surface nearer the eye by one unit.
    const double pa[3]={-500,500,1500},pb[3]={500,500,1500},pc[3]={0,1000,1000};
    const double qa[3]={-500,499,1500},qb[3]={500,499,1500},qc[3]={0,999,1000};
    const auto floor=ttk::near_depth_plane(pa,pb,pc),shelf=ttk::near_depth_plane(qa,qb,qc);
    g.ofx=0;g.ofy=0;
    for(int y=80;y<256;++y) {
        const double expected=2000.0/(1+y/256.0);
        const double z=ttk::near_raster_depth(floor,g,0,y,1500);
        const double front=ttk::near_raster_depth(shelf,g,0,y,1500);
        assert(std::abs(z-expected)<1e-9 && front<z);
    }
    // Reciprocal depth is affine in raster space: different subdivisions
    // produce the same interior value, even after independently snapped edges.
    const double ys[3]={85,86,255};
    double q=0;
    for(int i=0;i<3;++i) q+=(1.0/3)/ttk::near_raster_depth(floor,g,0,ys[i],1500);
    const double center=ttk::near_raster_depth(floor,g,0,(85+86+255)/3.0,1500);
    assert(std::abs(1/q-center)<1e-9);
    // Degenerate, near-plane and horizon cases cannot create invalid depth.
    const auto degenerate=ttk::near_depth_plane(pa,pa,pc);
    assert(ttk::near_raster_depth(degenerate,g,0,100,1500)==1500);
    assert(ttk::near_raster_depth(floor,g,0,-256,1500)==1500);
    assert(ttk::near_raster_depth(floor,g,0,-255.99,1500)==1500);
    // Translation of the projection centre leaves the same physical ray.
    g.ofx=123*65536;g.ofy=42*65536;
    assert(std::abs(ttk::near_raster_depth(floor,g,123,142,1500)-2000.0/(1+100/256.0))<1e-9);
    if(argc==2)packet_contracts(argv[1]);
    std::puts("ttk-near-test PASS");
}
