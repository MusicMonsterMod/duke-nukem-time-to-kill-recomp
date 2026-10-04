#include <cstdlib>
#include <string_view>
static unsigned precision_timeline_invalidations;
extern "C" void pgxp_invalidate_all(void) {++precision_timeline_invalidations;}
extern "C" {int g_pgxp_mesh_active=0;}
// D11B near-clip math: GTE-exact projection, safety classification and DPCS.
#include "near_clip.h"
extern "C" int pgxp_mesh_geometry(void) {const char* s=std::getenv("DNTTK_GEOMETRY_PRECISION");return s && std::string_view(s)=="corrected";}
extern "C" int pgxp_mesh_textures(void) {const char* s=std::getenv("DNTTK_TEXTURE_PRECISION");return s && std::string_view(s)=="corrected";}
extern "C" void pgxp_mesh_register_boundary(void) {}
#include <cstdint>
#include <set>
// D17N fixture: addresses whose packet words carry an exact projection.
static std::set<uint32_t> precise_words;
extern "C" int pgxp_mesh_vertex(uint32_t addr,uint32_t,int32_t* x,int32_t* y,uint16_t* z) {
    *x=*y=0;*z=1;return precise_words.count(addr)!=0;
}
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
struct HostCorner {uint32_t word;int32_t x,y;float z,u,v,depth;};
static std::vector<HostCorner> host_corners;
extern "C" {uint8_t* g_psx_ram=ram;uint32_t g_dirty_ram_code_gen=1;}
extern "C" uint32_t psx_mod_alloc_guest_memory(uint32_t,uint32_t){return 0x801e0000;}
extern "C" uint8_t psx_mod_read_byte(uint32_t a){return ram[a&0x1fffff];}
extern "C" uint16_t psx_mod_read_half(uint32_t a){return psx_mod_read_byte(a)|(psx_mod_read_byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a){return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
extern "C" void psx_mod_write_byte(uint32_t a,uint8_t v){ram[a&0x1fffff]=v;}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v){for(int i=0;i<4;++i)ram[(a+i)&0x1fffff]=v>>(8*i);}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t,PSXModFunctionEntryCallback cb){near_hook=cb;return 1;}
extern "C" int psx_mod_gpu_host_vertex(uint32_t,uint32_t,int32_t,int32_t,float,float,float){return 1;}
extern "C" int psx_mod_gpu_host_vertex_depth(uint32_t,uint32_t w,int32_t x,int32_t y,float z,float u,float v,float d){++host_vertices;host_corners.push_back({w,x,y,z,u,v,d});return 1;}
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
    // A new/empty OT must not discard metadata for a queued packet buffer.
    // Individual writes and actual state restores own precision invalidation.
    const unsigned invalidations=precision_timeline_invalidations;
    near_hook(cpu,address);
    assert(precision_timeline_invalidations==invalidations);
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
    g_pgxp_mesh_active=1;
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
        ++g_dirty_ram_code_gen;host_vertices=0;host_corners.clear();
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
    auto object_case=[&](uint32_t ra,int extent,bool expect_taken,bool occupied_ot=false) {
        auto c=reset();c.gpr[31]=ra;c.gpr[18]=0x80180000+extent*0x60;
        psx_mod_write_byte(mesh+6,3);word(mesh+0x10,verts);word(mesh+0x14,list);
        word(verts,(uint16_t)-extent|uint32_t(uint16_t(-extent))<<16);word(verts+4,5000);
        word(verts+8,extent|uint32_t(uint16_t(-extent))<<16);word(verts+12,5000);
        word(verts+16,uint32_t(extent)<<16);word(verts+20,5000);
        word(list,0x00010024);word(list+4,0x00020100);word(list+8,0);
        word(list+12,0x0200001f);word(list+16,0x1f00);word(list+20,0xff);
        if(occupied_ot) {
            // A sprite before the first mesh leaves the OT nonempty. Starting
            // from a loaded scene must still get a fresh, bounded clip budget.
            word(bm,1);
            ttk::near_clip_load_frame_state({0,0xffffffffu,0});
            CPUState boundary{};
            observed_hook(&boundary,0x80026164);
            assert(ttk::near_clip_frame_state().host_bytes==0xffffffffu); // wrong caller
            boundary.gpr[31]=0x800268f4;
            observed_hook(&boundary,0x80026164);
            const auto fresh=ttk::near_clip_frame_state();
            assert(fresh.first_cursor==psx_mod_read_word(ctx) && fresh.host_bytes==0);
        }
        observed_hook(&c,0x80010000);
        assert((c.gpr[4]!=mesh)==expect_taken);
        assert((host_vertices>0)==expect_taken);
        if(occupied_ot) {
            const auto frame=ttk::near_clip_frame_state();
            assert(frame.host_bytes>0);
            // Worker state restoration must retain usage; another mesh in
            // the same populated OT must not refresh an exhausted budget.
            ttk::near_clip_load_frame_state({frame.first_cursor,0xffffffffu,frame.held_frame});
            const auto count=host_vertices;c.gpr[4]=mesh;
            observed_hook(&c,0x80010000);
            assert(c.gpr[4]==mesh && host_vertices==count);
            ttk::near_clip_load_frame_state(frame);
            assert(ttk::near_clip_frame_state().host_bytes==frame.host_bytes);
        }
    };
    object_case(0x80032288,100,true); // compact static prop beyond near radius
    object_case(0x80032288,100,true,true); // first mesh need not see an empty OT
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
    // D17L: distant opaque static top faces retain native integer vertices,
    // affine UVs, quad topology and AVSZ4 ordering while gaining host depth.
    // Vertical, sloped, lower, translucent and foreign-caller faces stay native.
    auto top_case=[&](int kind,uint32_t caller,bool expected,int bias=0) {
        auto c=reset();c.gpr[31]=caller;c.gpr[18]=0x80190000;
        psx_mod_write_byte(mesh+6,8);word(mesh+0x10,verts);word(mesh+0x14,list);
        const int16_t xyz[8][3]={{-400,-100,4600},{400,-100,4600},
            {-400,-100,5400},{400,-100,5400},{-400,200,4600},
            {400,200,4600},{-400,200,5400},{400,200,5400}};
        for(int i=0;i<8;++i) {
            int y=xyz[i][1];
            if(kind==1 && i==2) y=0; // sloped/nonplanar
            if(kind==2 && i==4) y=-200; // surface is not mesh top
            word(verts+8*i,(uint16_t)xyz[i][0]|uint32_t(uint16_t(y))<<16);
            word(verts+8*i+4,(uint16_t)xyz[i][2]);
        }
        // Double sided, ordinary modulated FT4. A bias in either direction
        // must remain exactly native, without the enhanced -4 prop bias.
        const uint32_t bits=(kind>=10?0u:1u)|(kind==3?16u:0u)|(bias>0?8u:bias<0?4u:0u);
        word(list,0x0001002c);word(list+4,0x03020100);
        word(list+8,0);word(list+12,(bits<<25)|0x001f);
        word(list+16,0x1f1f1f00);word(list+20,0xff);
        if(kind==4) word(ctx+0x48,0x02000000); // context fade/semitransparency
        if(kind==5) word(ctx+0x78,1); // original far-depth rejection
        if(kind==6) c.gte_ctrl[30]=128; // use AVSZ4 scale, not an average guess
        if(kind==10) word(ctx+0x50,0x400); // mirrored native NCLIP
        if(kind==12) c.gte_ctrl[26]=64; // screen-snapped zero area
        if(kind==8) word(verts+7*8,psx_mod_read_word(verts+6*8)); // duplicate corner
        if(kind==9) psx_mod_write_byte(mesh+6,7); // incomplete box
        if(kind==7) word(ctx+8,0x80161000); // whole-mesh budget fallback
        observed_hook(&c,0x80010000);
        assert((host_vertices!=0)==expected);
        if(expected) {
            assert(host_vertices==6);
            const int expected_slot=(kind==6?78:156)+bias;
            for(int i=0;i<2048;++i) assert((psx_mod_read_word(ot+i*8)!=0)==(i==expected_slot));
            const int index[6]={0,1,2,2,1,3};
            ttk::NearGte g{};for(int i=0;i<3;++i)g.r[i][i]=4096;g.h=256;
            const float uv[4][2]={{0,0},{31,0},{0,31},{31,31}};
            for(int i=0;i<6;++i) {
                const auto& v=host_corners[i];const int j=index[i];
                assert(v.word==ttk::near_project(g,xyz[j][0],xyz[j][1],xyz[j][2]).sxy);
                const auto projected=ttk::near_project(g,xyz[j][0],xyz[j][1],xyz[j][2]);
                assert(v.z==(pgxp_mesh_textures()?float(xyz[j][2]):1.0f) && v.depth>0);
                assert(v.x==(pgxp_mesh_geometry()?projected.x16:(int32_t)(int16_t)v.word*65536));
                assert(v.y==(pgxp_mesh_geometry()?projected.y16:(int32_t)(int16_t)(v.word>>16)*65536));
                assert(v.u==uv[j][0] && v.v==uv[j][1]);
            }
        } else if(kind!=5 && kind!=10 && kind!=12) assert(c.gpr[4]==mesh);
    };
    for(int bias:{-1,0,1}) top_case(0,0x80032288,true,bias);
    for(int kind:{1,2,3,4,5,7,8,9,10,12}) top_case(kind,0x80032288,false);
    top_case(6,0x80032288,true);top_case(11,0x80032288,true);
    top_case(0,0x80033778,false);top_case(0,0x800351f4,false);
    live_eye=false;top_case(0,0x80032288,false);live_eye=true;
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
// D17N: with Corrected textures, a world polygon whose corners all have exact
// projections is drawn whole instead of going to the screen-space subdivision.
// Bit 21 is cleared on one corner for that polygon only, then restored.
static void whole_polygon_contracts() {
    constexpr uint32_t ctx=0x800d67a8,buffer=0x80170000,record=0x80171000;
    constexpr uint32_t near_bit=1u<<21;
    const char* keep=std::getenv("DNTTK_WORLD_SUBDIVISION");
    const bool corrected=pgxp_mesh_textures() && !(keep && std::string_view(keep)=="1");
    // packet_contracts leaves a caller instruction altered; restore it.
    psx_mod_write_word(0x80032280,psx_mod_read_word(0x80032280)^1);++g_dirty_ram_code_gen;
    auto setup=[&](bool quad,int16_t width) {
        CPUState c{};c.gpr[6]=ctx;c.gpr[14]=buffer;c.gpr[16]=record;
        c.gpr[31]=quad?0x80011ab4:0x80011894;
        psx_mod_write_word(record,quad?0x07050301:0x00050301);
        precise_words.clear();
        for(uint32_t i:{1u,3u,5u,7u}) {
            const int16_t x=(i==7?width:int16_t(i*10)),y=int16_t(i*5);
            psx_mod_write_word(buffer+8*i,uint16_t(x)|uint32_t(uint16_t(y))<<16);
            psx_mod_write_word(buffer+8*i+4,near_bit|0x1800);
            precise_words.insert(buffer+8*i);
        }
        return c;
    };
    auto flags=[&](uint32_t i){return psx_mod_read_word(buffer+8*i+4);};
    // Quad: cleared on the first corner only while its fetch runs.
    auto c=setup(true,70);near_hook(&c,0x8001160c);
    assert(flags(1)==(corrected?0x1800u:(near_bit|0x1800)));
    for(uint32_t i:{3u,5u,7u}) assert(flags(i)==(near_bit|0x1800));
    // The next fetch restores it before deciding for its own polygon.
    c.gpr[31]=0;near_hook(&c,0x8001160c);assert(flags(1)==(near_bit|0x1800));
    // Triangle with the same rules.
    c=setup(false,70);near_hook(&c,0x800114ec);
    assert(flags(1)==(corrected?0x1800u:(near_bit|0x1800)));
    // A new mesh rewrites the buffer: a pending restore must not land on it.
    psx_mod_write_word(buffer+12,0x1234);CPUState entry{};near_hook(&entry,0x80011020);
    near_hook(&c,0x800114ec);assert(flags(1)==0x1234);
    // Original subdivision is kept when any rule fails.
    auto kept=[&](bool quad,int16_t width,auto change) {
        auto k=setup(quad,width);change(k);
        near_hook(&k,quad?0x8001160c:0x800114ec);
        assert(flags(1)==(near_bit|0x1800));
        k.gpr[31]=0;near_hook(&k,0x8001160c);
    };
    kept(true,70,[&](CPUState&){psx_mod_write_word(buffer+8*5+4,0x1800);}); // already whole
    kept(true,70,[&](CPUState&){precise_words.erase(buffer+8*7);}); // no exact projection
    kept(true,1100,[](CPUState&){}); // beyond the GPU primitive width
    kept(true,70,[](CPUState& k){k.gpr[31]=0x80011894;}); // wrong caller
    kept(false,70,[](CPUState& k){k.gpr[6]=0x800d6800;}); // foreign context
    // Altered renderer code must refuse the bypass.
    const uint32_t code=psx_mod_read_word(0x80011acc);
    psx_mod_write_word(0x80011acc,code^1);++g_dirty_ram_code_gen;
    kept(true,70,[](CPUState&){});
    psx_mod_write_word(0x80011acc,code);++g_dirty_ram_code_gen;
    std::puts("ttk-near-test whole-polygon contracts PASS");
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
    if(argc==2) {packet_contracts(argv[1]);whole_polygon_contracts();}
    std::puts("ttk-near-test PASS");
}
