#include "limb_presentation.h"
#include "bio_mask_mesh.h"
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
static bool live_eye=false,modern=true,live_third=false,duke=false,interaction=false,test_viewmodel=false;
static ttk::LimbOwner test_owner=ttk::LimbOwner::DisplayHand;
static int test_part=0x0f;
static double body_coverage=1;
static float observed_coverage=1;
static unsigned host_vertices;
struct HostCorner {uint32_t word;int32_t x,y;float z,u,v,depth;};
static std::vector<HostCorner> host_corners;
extern "C" {uint8_t* g_psx_ram=ram;uint32_t g_dirty_ram_code_gen=1;}
extern "C" uint32_t psx_mod_alloc_guest_memory(uint32_t size,uint32_t){static uint32_t next=0x801d0000;const auto result=next;next+=size;assert(next<=0x80200000);return result;}
// D17R1 host packet arena (GPU DMA aperture).
static uint32_t dma_next=0x80f00000,dma_allocations;
extern "C" uint32_t psx_mod_alloc_gpu_dma_memory(uint32_t n,uint32_t align){dma_next=(dma_next+align-1)&~(align-1);const auto p=dma_next;dma_next+=n;assert(dma_next<=0x81000000);++dma_allocations;return p;}
static int32_t test_margin=85;
extern "C" int32_t psx_mod_widescreen_x_margin(void){return test_margin;}
// GPU DMA aperture (0x80f00000, mod_memory.h), apart from main RAM as in the runtime.
static uint8_t aperture[0x100000];
static uint8_t& byte_at(uint32_t a){return (a&0xffffff)>=0xf00000?aperture[a&0xfffff]:ram[a&0x1fffff];}
extern "C" uint8_t psx_mod_read_byte(uint32_t a){return byte_at(a);}
extern "C" uint16_t psx_mod_read_half(uint32_t a){return psx_mod_read_byte(a)|(psx_mod_read_byte(a+1)<<8);}
extern "C" uint32_t psx_mod_read_word(uint32_t a){return psx_mod_read_half(a)|(uint32_t(psx_mod_read_half(a+2))<<16);}
extern "C" void psx_mod_write_byte(uint32_t a,uint8_t v){byte_at(a)=v;}
extern "C" void psx_mod_write_word(uint32_t a,uint32_t v){for(int i=0;i<4;++i)byte_at(a+i)=v>>(8*i);}
extern "C" int psx_mod_register_function_entry_plugin(const char*,uint32_t,PSXModFunctionEntryCallback cb){near_hook=cb;return 1;}
extern "C" int psx_mod_gpu_host_vertex(uint32_t,uint32_t,int32_t,int32_t,float,float,float){return 1;}
extern "C" int psx_mod_gpu_host_vertex_depth(uint32_t,uint32_t w,int32_t x,int32_t y,float z,float u,float v,float d){++host_vertices;host_corners.push_back({w,x,y,z,u,v,d});return 1;}
extern "C" int psx_mod_gpu_host_vertex_coverage(uint32_t a,uint32_t w,int32_t x,int32_t y,float z,float u,float v,float d,float coverage){observed_coverage=coverage;return psx_mod_gpu_host_vertex_depth(a,w,x,y,z,u,v,d);}

extern "C" int psx_mod_replay_active(void){return 0;}
extern "C" uint32_t psx_mod_savestate_loads(void){return 0;}
namespace ttk {bool frame_trace_on(){return false;} void frame_trace_account(uint32_t,long){}}
static bool lower_body=false;
namespace ttk {void first_person_mesh_begin(CPUState*){} void third_person_mesh_begin(CPUState*){} bool input_modernized(){return modern;} bool first_person_view_live(){return live_eye;} bool third_person_camera_live(){return modern && live_third;} double third_person_body_coverage(){return body_coverage;} bool widescreen_near_clip_live(){return false;} bool third_person_duke_drawing(){return duke;} bool first_person_duke_drawing(){return duke;} bool first_person_actor_drawing(){return duke;} LimbPresentation first_person_limb_presentation(){return test_viewmodel?limb_presentation(test_owner,test_part):lower_body?limb_presentation(LimbOwner::Body,5,true):interaction?limb_presentation(LimbOwner::Interaction,0x0f):LimbPresentation{};} bool first_person_interaction_drawing(){return interaction;} bool first_person_weapon_drawing(){return false;} uint64_t input_host_frame(){return 0;}}
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
    hash(ram,sizeof ram);hash(aperture,sizeof aperture);hash(cpu->gpr,sizeof cpu->gpr);
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
    // D17Y: an absent animation bank must not become framebuffer texturing.
    // Exercise the native third-person path (no near takeover), preserving a
    // valid descriptor even when its texture words happen to be all zero.
    {
        constexpr uint32_t anim=0x80151000,valid=0x80152000,missing=0x800c5784;
        auto setup=[&] {
            auto c=reset();word(mesh+0x10,verts);word(mesh+0x14,list);
            word(ctx+0x60,anim);word(anim+8,missing);word(anim+24,valid);
            word(missing,0);word(missing+4,0);word(missing+8,0);
            word(valid,0);word(valid+4,0);word(valid+8,0);
            word(list,0x0002002c);
            for(unsigned i=0;i<2;++i) {
                word(list+4+i*16,0x03020100);word(list+8+i*16,i<<16);
                word(list+12+i*16,0x4000001f);word(list+16+i*16,0x1f1f1f00);
            }
            word(list+36,0xff);return c;
        };
        auto c=setup();observed_hook(&c,0x80010000);
        assert(c.gpr[4]!=mesh && host_vertices==0);
        const uint32_t filtered=psx_mod_read_word(c.gpr[4]+0x14);
        assert(psx_mod_read_word(filtered)==0x0001002c);
        assert(psx_mod_read_word(filtered+8)==0x00010000);
        assert(psx_mod_read_word(filtered+20)==0xff);
        assert(psx_mod_read_word(list)==0x0002002c); // source stays intact
        c=setup();modern=false;observed_hook(&c,0x80010000);assert(c.gpr[4]==mesh);modern=true;
        c=setup();word(missing+4,1);observed_hook(&c,0x80010000);assert(c.gpr[4]==mesh);
        c=setup();word(anim+8,valid);observed_hook(&c,0x80010000);assert(c.gpr[4]==mesh);
        c=setup();word(list+36,0x00010099);observed_hook(&c,0x80010000);assert(c.gpr[4]==mesh);
        c=setup();c.gpr[6]+=4;observed_hook(&c,0x80010000);assert(c.gpr[4]==mesh);
        std::puts("D17Y missing animation: filtered copy, valid zero descriptor, Vanilla and malformed-list guards PASS");
    }
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
    // D17R1: host packets live in their own arena; the game's ring cursor
    // and its packets are untouched.
    assert(psx_mod_read_word(ot+128*8)>=0x80f00000u && psx_mod_read_word(ot+128*8)<0x80f80000u);
    assert(psx_mod_read_word(ctx)==0x80160000);
    for(unsigned i=0;i<2048;++i)if(i!=128)assert(psx_mod_read_word(ot+i*8)==0);
    // D17X: both native halves of a warped world quad retain the original
    // four-corner maximum depth, even when one half has only near vertices.
    {
        auto c=reset();c.gpr[31]=0x80037344;
        word(mesh+0x14,4);word(mesh+0x18,0x50);
        const uint32_t packed[4]={0x0001ff3f,0x0001ff01,0x0001013f,0x00020101};
        for(int i=0;i<4;++i) {word(mesh+0x1c+8*i,packed[i]);word(mesh+0x20+8*i,0x34808080);}
        word(mesh+0x50,0x00010061);word(mesh+0x54,0x03020100);
        word(mesh+0x58,0x02000000);word(mesh+0x5c,0xff);
        observed_hook(&c,0x80011020);assert(host_vertices>=6);
        for(unsigned i=0;i<2048;++i) assert((psx_mod_read_word(ot+i*8)!=0)==(i==64));
    }
    // D17AE/D17AK scene depth: a native-kept world quad's own packet, written
    // to the game's ring by the original renderer, gets each corner's view
    // depth at the drawing-chain close. Translucent packets and Vanilla keep
    // painter order; the packet and ordering table are not modified.
    for(bool modernized:{true,false}) {
        modern=modernized;
        auto c=reset();c.gpr[31]=0x80037344;
        word(mesh+0x14,4);word(mesh+0x18,0x50);
        const uint32_t packed[4]={0x0004013f,0x00040101,0x00043f3f,0x00043f01};
        for(int i=0;i<4;++i) {word(mesh+0x1c+8*i,packed[i]);word(mesh+0x20+8*i,0x34808080);}
        word(mesh+0x50,0x00010061);word(mesh+0x54,0x03020100);
        word(mesh+0x58,0x02000000);word(mesh+0x5c,0xff);
        observed_hook(&c,0x80011020);
        assert(c.gpr[4]==mesh && host_vertices==0);
        ttk::NearGte g{};for(int i=0;i<3;++i) g.r[i][i]=4096;g.h=256;
        uint32_t xy[4];
        for(int i=0;i<4;++i) {
            const auto p=ttk::near_project(g,(int16_t)((packed[i]&0xff)<<10),(int16_t)((packed[i]&0xff00)>>1),(int16_t)(((packed[i]>>16)&0xff)<<10));
            assert(p.safe && p.sz==4096);xy[i]=p.sxy;
        }
        constexpr uint32_t ring=0x80160000;
        for(uint32_t at:{ring,ring+0x34}) {
            word(at,0x0c000000);word(at+4,(at==ring?0x3c:0x3e)<<24|0x808080);
            for(int i=0;i<4;++i) {if(i) word(at+4+12*i,0x808080);word(at+8+12*i,xy[i]);word(at+12+12*i,0);}
        }
        word(ctx,ring+0x68);
        CPUState close{};observed_hook(&close,0x8001fba0);
        if(!modernized) {assert(host_vertices==0);continue;}
        assert(host_vertices==4);
        for(int i=0;i<4;++i) assert(host_corners[i].word==xy[i] && host_corners[i].depth==4096.0f && host_corners[i].u<0);
        for(unsigned i=0;i<2048;++i) assert(psx_mod_read_word(ot+i*8)==0);
        // A second close without new packets adds nothing.
        observed_hook(&close,0x8001fba0);assert(host_vertices==4);
    }
    modern=true;
    std::puts("D17AE scene depth: native world packet corners carry view depth; translucent and Vanilla keep painter order PASS");
    // D17Z: a flat wall with skewed UVs must retain the native texture
    // interpolation across the 1-2 diagonal, including subdivision vertices.
    // The alternative diagonal moves V by 4.5 texels at this quad's centre.
    if(pgxp_mesh_textures()) for(int shift:{0,2048}) {
        auto c=reset();c.gpr[31]=0x80037344;c.gte_ctrl[5]=shift;
        word(mesh+0x14,4);word(mesh+0x18,0x50);
        c.gte_ctrl[6]=uint32_t(-128);
        const uint32_t packed[4]={0x0001003f,0x00010001,0x0001023f,0x00010201};
        for(int i=0;i<4;++i) {word(mesh+0x1c+8*i,packed[i]);word(mesh+0x20+8*i,0x34808080);}
        word(mesh+0x50,0x00010061);word(mesh+0x54,0x03020100);
        word(mesh+0x58,0x02000000);word(mesh+0x5c,0xff);
        word(0x80150000,0x407f);word(0x80150004,0x00104040);
        word(0x80150008,0x6e40777e);
        observed_hook(&c,0x80011020);assert(host_vertices>=6);
        bool clipped=false;
        for(const auto& v:host_corners) {
            clipped|=v.x==480*65536;
            // Before translation: x=-1024..1024, y=-128..128, z=1024.
            const double x=(v.x/65536.0-shift/4.0+256.0)/512.0;
            const double y=(v.y/65536.0+32.0)/64.0;
            const double u=x+y<=1 ? 127-63*x-y : 126-62*x;
            const double t=x+y<=1 ? 64+55*y : 73-9*x+46*y;
            assert(std::abs(v.u-u)<0.01 && std::abs(v.v-t)<0.01);
            assert(std::abs(v.depth-1024)<0.01);
        }
        assert(clipped==(shift!=0));
        for(unsigned i=0;i<2048;++i) assert((psx_mod_read_word(ot+i*8)!=0)==(i==32));
        // The Modernized-only hook cannot alter Vanilla's mesh or packets.
        c=reset();modern=false;observed_hook(&c,0x80011020);
        assert(c.gpr[4]==mesh && host_vertices==0);modern=true;
        std::puts("D17Z planar world UV interpolation and source ordering PASS");
    }
    // D17M: overhead horizontal world faces in the prop-depth band write depth,
    // preserving their maximum-SZ slot, corners and native quad diagonal.
    // No takeover outside the band, in Original textures, or in Vanilla.
    const char* bridge_env=std::getenv("DNTTK_PLATFORM_DEPTH");
    const char* subdiv_env=std::getenv("DNTTK_WORLD_SUBDIVISION");
    const bool bridge=pgxp_mesh_textures() && !(bridge_env && std::string_view(bridge_env)=="0") &&
        !(subdiv_env && std::string_view(subdiv_env)=="1");
    auto world_bridge=[&](bool quad,int shift,uint32_t ra,int far_limit,bool exhausted,bool expect,int kind=0) {
        auto c=reset();c.gpr[31]=ra;c.gte_ctrl[7]=shift;c.gte_ctrl[6]=-512;
        if(kind==1) c.gte_ctrl[6]=256; // floor below the eye
        word(mesh+0x14,8);word(mesh+0x18,0x80);
        uint32_t packed[4]={0x0002013f,0x00020101,0x0003013f,0x00030101};
        if(kind>=4) for(auto& v:packed) v=(v&~0xff00u)|uint32_t(kind==5?3:2)<<8;
        for(int i=0;i<4;++i) {word(mesh+0x1c+8*i,packed[i]);word(mesh+0x20+8*i,0x34808080);}
        for(int i=0;i<4;++i) {word(mesh+0x1c+8*(i+4),packed[i]&~0xff00u);word(mesh+0x20+8*(i+4),0x34808080);}
        if(kind==6) word(mesh+0x1c+8*6,(packed[2]&~0xff00u)|0x100); // upper corners at mixed heights
        if(kind==3) word(mesh+0x1c+8*6,(packed[2]&~0xff00u)+1); // missing upper corner
        if(kind==2) word(mesh+0x1c+16,packed[2]+0x100); // sloped/nonplanar
        word(mesh+0x80,quad?0x00010061:0x00010060);word(mesh+0x84,quad?0x03020100:0x00020100);
        word(mesh+0x88,0x02000000);word(mesh+0x8c,0xff);word(ctx+0x78,far_limit);
        if(exhausted) {word(bm,1);auto state=ttk::near_clip_frame_state();state.host_bytes=0xffffffffu;ttk::near_clip_load_frame_state(state);}
        observed_hook(&c,0x80011020);
        assert((host_vertices!=0)==expect);
        if(expect) {
            assert(host_vertices==(quad?6u:3u));
            for(unsigned i=0;i<2048;++i) assert((psx_mod_read_word(ot+i*8)!=0)==(i==96));
            const int index[6]={0,1,2,2,1,3};
            ttk::NearGte g{};for(int i=0;i<3;++i)g.r[i][i]=4096;g.h=256;g.tr[1]=-512;
            const float uv[4][2]={{0,0},{31,0},{0,31},{31,31}};
            for(unsigned i=0;i<host_vertices;++i) {
                const auto& v=host_corners[i];const int j=index[i];const uint32_t w=packed[j];
                const auto projected=ttk::near_project(g,(int16_t)((w&255)<<10),(int16_t)(((w&0xff00)<<15)>>16),(int16_t)(((w>>16)&255)<<10));
                assert(v.word==projected.sxy && v.z==projected.sz && v.depth>0);
                assert(v.u==uv[j][0] && v.v==uv[j][1]);
            }
        }
    };
    for(bool quad:{false,true}) {
        world_bridge(quad,0,0x80062db8,0xffff,false,bridge);
        world_bridge(quad,6144,0x80062db8,0xffff,false,false);
        world_bridge(quad,0,0x80032288,0xffff,false,false);
        world_bridge(quad,0,0x80062db8,768,false,false);
        world_bridge(quad,0,0x80062db8,0xffff,true,false);
        world_bridge(quad,0,0x80062db8,0xffff,false,false,1);
        world_bridge(quad,0,0x80062db8,0xffff,false,false,2);
        world_bridge(quad,0,0x80062db8,0xffff,false,false,3);
        world_bridge(quad,0,0x80062db8,0xffff,false,bridge,4);
        world_bridge(quad,0,0x80062db8,0xffff,false,false,5);
        world_bridge(quad,0,0x80062db8,0xffff,false,false,6);
        live_eye=false;world_bridge(quad,0,0x80062db8,0xffff,false,false);live_eye=true;
    }
    // Searching farther for platforms must not extend the old takeover
    // boundary for unrelated wide world faces.
    cpu=reset();cpu.gpr[31]=0x80062db8;
    word(mesh+0x14,3);word(mesh+0x18,0x40);
    word(mesh+0x1c,0x00040038);word(mesh+0x20,0x34808080);
    word(mesh+0x24,0x00040008);word(mesh+0x28,0x34808080);
    word(mesh+0x2c,0x00040100);word(mesh+0x30,0x34808080);
    word(mesh+0x40,0x00010060);word(mesh+0x44,0x00020100);
    word(mesh+0x48,0x02000000);word(mesh+0x4c,0xff);
    observed_hook(&cpu,0x80011020);
    assert(cpu.gpr[4]==mesh && host_vertices==0);
    auto object_case=[&](uint32_t ra,int extent,bool expect_taken,bool occupied_ot=false,uint32_t pickup_flags=0,int z=5000,bool semi=false,int projection=256) {
        auto c=reset();c.gte_ctrl[26]=projection;c.gpr[31]=ra;c.gpr[18]=0x80180000+extent*0x60;
        word(0x800d2660,0x80190000);word(c.gpr[18]+0x2c,3);word(0x80190000+28*3,pickup_flags);
        psx_mod_write_byte(mesh+6,3);word(mesh+0x10,verts);word(mesh+0x14,list);
        word(verts,(uint16_t)-extent|uint32_t(uint16_t(-extent))<<16);word(verts+4,z);
        word(verts+8,extent|uint32_t(uint16_t(-extent))<<16);word(verts+12,z);
        word(verts+16,uint32_t(extent)<<16);word(verts+20,z);
        word(list,0x00010024);word(list+4,0x00020100);word(list+8,0);
        word(list+12,semi?0x2200001f:0x0200001f);word(list+16,0x1f00);word(list+20,0xff);
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
        const auto gte_before=c;
        observed_hook(&c,0x80010000);
        assert(std::memcmp(c.gte_ctrl,gte_before.gte_ctrl,sizeof c.gte_ctrl)==0);
        assert((c.gpr[4]!=mesh)==expect_taken);
        assert((host_vertices>0)==expect_taken);
        if(pickup_flags==0x200 && expect_taken) {
            assert(host_vertices==3); // no subdivision or compact regrouping
            const unsigned slot=unsigned((int64_t(341)*3*z)>>12)>>3;
            for(unsigned i=0;i<2048;++i) assert((psx_mod_read_word(ot+i*8)!=0)==(i==slot));
            for(const auto& v:host_corners) assert(v.z==z && v.depth>0);
        }
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
    // D17AH: sky layers retain native painter composition while keeping
    // perspective UV/depth metadata. Adjacent callers and changed sky code
    // must remain ordinary depth-tested geometry; Vanilla never takes over.
    {
        live_eye=true;
        const char* old_painter=std::getenv("DNTTK_SKY_PAINTER");
        const std::string saved_painter=old_painter?old_painter:"";
        setenv("DNTTK_SKY_PAINTER","1",1);
        for(int hp:{256,160,92}) for(uint32_t ra:{0x80038c94u,0x80038cf0u,0x80038da8u,0x80038e44u}) {
            for(bool semi:{false,true}) {
                object_case(ra,100,true,false,0,500,semi,hp);
                assert(psx_mod_read_word(ot+8)==0); // not the deferred weapon layer
                const auto painter=host_corners;
                for(const auto& v:painter) assert(v.z>0 && v.depth<0);
                setenv("DNTTK_SKY_PAINTER","0",1);
                object_case(ra,100,true,false,0,500,semi,hp);
                assert(host_corners.size()==painter.size());
                for(size_t j=0;j<painter.size();++j) {
                    const auto& a=painter[j];const auto& b=host_corners[j];
                    assert(a.word==b.word && a.x==b.x && a.y==b.y);
                    assert(a.z==b.z && a.u==b.u && a.v==b.v && b.depth>0);
                }
                setenv("DNTTK_SKY_PAINTER","1",1);
            }
            object_case(ra+4,100,true,false,0,500,false,hp);
            for(const auto& v:host_corners) assert(v.depth>0);
        }
        const uint32_t original=psx_mod_read_word(0x80038d74);
        psx_mod_write_word(0x80038d74,original^1);++g_dirty_ram_code_gen;
        object_case(0x80038c94,100,true,false,0,500);
        for(const auto& v:host_corners) assert(v.depth>0);
        psx_mod_write_word(0x80038d74,original);++g_dirty_ram_code_gen;
        modern=false;live_eye=false;object_case(0x80038c94,100,false,false,0,500);modern=true;live_eye=true;
        if(old_painter)setenv("DNTTK_SKY_PAINTER",saved_painter.c_str(),1);else unsetenv("DNTTK_SKY_PAINTER");
        std::puts("PASS: D17AH sky painter depth, unchanged geometry/UVs, caller/identity/Vanilla guards");
    }
    // D14B: exercise real emitted mesh packets, including UV/depth metadata.
    // Display owners, Q and door legs are independent of world H; world/contact
    // owners still respond to it. No shared GTE register is changed by projection.
    {
        live_eye=true;test_viewmodel=true;
        for(auto owner:{ttk::LimbOwner::DisplayHand,ttk::LimbOwner::DisplayArm,
                       ttk::LimbOwner::Interaction,ttk::LimbOwner::Body,ttk::LimbOwner::Kick,ttk::LimbOwner::Legs}) {
            test_owner=owner;
            for(int part:{1,2,3,4,5,6,7,8,21}) for(int margin:{0,85,192}) {
                test_part=part;test_margin=margin;
                const bool fixed=owner==ttk::LimbOwner::DisplayHand || owner==ttk::LimbOwner::DisplayArm ||
                    owner==ttk::LimbOwner::Kick || owner==ttk::LimbOwner::Legs;
                object_case(0x80034dd0,100,true,false,0,500);
                const auto baseline=host_corners;assert(!baseline.empty());
                for(int h:{160,177,191,209,256,277}) {
                    object_case(0x80034dd0,100,true,false,0,500,false,h);
                    assert(host_corners.size()==baseline.size());
                    bool moved=false;
                    for(unsigned i=0;i<baseline.size();++i) {
                        const auto& a=baseline[i];const auto& b=host_corners[i];
                        assert(a.z==b.z && a.u==b.u && a.v==b.v && a.depth==b.depth);
                        moved|=a.x!=b.x || a.y!=b.y;
                        if(fixed) assert(a.word==b.word && a.x==b.x && a.y==b.y);
                    }
                    if(!fixed) assert(moved==(h!=256));
                }
            }
        }
        test_part=0x0f;test_owner=ttk::LimbOwner::DisplayHand;test_viewmodel=false;test_margin=85;
        ttk::near_clip_load_frame_state({0,0,0,0});
        std::puts("PASS: D14B/D36J4 fixed display/Q/door projection, world/contact exclusion, UV/depth and GTE preservation at three aspects");
    }
    // D17AB: a mirror can leave the native OT empty while each arm/weapon
    // mesh queues host packets. More than four meshes must not wrap the arena
    // and overwrite the first mesh before the draw-boundary flush.
    {
        live_eye=true;test_viewmodel=true;
        ttk::near_clip_load_frame_state({0,0,0,0});
        unsigned region=0,used=0;
        std::vector<uint8_t> retained;
        uint32_t retained_base=0;
        for(unsigned i=0;i<6;++i) {
            object_case(0x80034dd0,100+i,true,false,0,500);
            const auto state=ttk::near_clip_frame_state();
            if(!i) {
                region=state.host_region;
                retained_base=0x80f00000+region*128*1024;
                for(unsigned j=0;j<state.host_bytes;++j) retained.push_back(psx_mod_read_byte(retained_base+j));
            }
            assert(state.host_region==region && state.host_bytes>used);
            for(unsigned j=0;j<retained.size();++j) assert(psx_mod_read_byte(retained_base+j)==retained[j]);
            assert(psx_mod_read_word(bm)==0 && psx_mod_read_word(ot+8)==0);
            used=state.host_bytes;
        }
        // Native HUD arrives before the deferred weapon flush.
        word(bm,0x80000000u);word(ot,0x801f0000);word(ot+4,0x801f0000);
        word(0x801f0000,0x03000000u);
        ttk::near_clip_viewmodel_flush();
        assert(psx_mod_read_word(ot)==0x801f0000 && psx_mod_read_word(ot+8));
        unsigned packets=0;uint32_t node=psx_mod_read_word(ot+8);
        while(node) {
            assert(++packets<10000);
            assert((node&0xffffff)>=((retained_base)&0xffffff));
            assert((node&0xffffff)<((retained_base+used)&0xffffff));
            assert((psx_mod_read_word(node)>>24)==9);
            node=psx_mod_read_word(node)&0xffffff;
            if(node)node|=0x80000000u;
        }
        assert(packets>=6);
        test_viewmodel=false;ttk::near_clip_load_frame_state({0,0,0,0});
        std::puts("PASS: D17AB pending viewmodel packets survive empty OT across six meshes");
    }
    // D10B: body coverage travels with real emitted packets; a fully opaque
    // body remains native. Doors use only the authenticated occluder caller.
    live_eye=false;live_third=true;duke=true;body_coverage=0.375;
    object_case(0x80034dd0,100,true);
    assert(observed_coverage==0.375f);
    body_coverage=1;object_case(0x80034dd0,100,false);
    modern=false;body_coverage=0.375;object_case(0x80034dd0,100,false);modern=true;
    duke=false;body_coverage=1;
    {
        auto c=reset();c.gpr[4]=0x80140000;c.gpr[31]=0x80032010;
        observed_hook(&c,0x8002ee50);assert(c.gpr[4]!=0x80140000);
        c.gpr[4]=0x80140000;c.gpr[31]=0x80032014;
        observed_hook(&c,0x8002ee50);assert(c.gpr[4]==0x80140000);
        c.gpr[31]=0x80032010;modern=false;
        observed_hook(&c,0x8002ee50);assert(c.gpr[4]==0x80140000);modern=true;
    }
    live_third=false;
    {
        auto c=reset();c.gpr[4]=0x80140000;c.gpr[31]=0x80032010;
        observed_hook(&c,0x8002ee50);
        assert(c.gpr[4]==0x80140000); // D10B1: native third-person prop overlap test
        live_eye=true;
        observed_hook(&c,0x8002ee50);
        assert(c.gpr[4]!=0x80140000); // Earlier first-person fix remains active
    }
    std::puts("PASS: D10B1 native third-person occluder input preserved; first-person suppression retained");
    std::puts("PASS: D10B body packet coverage, native opaque body, solid-door caller and Vanilla gates");
    // D35D: native interaction hands use ordinary near clipping/depth, not
    // the weapon overlay. Other first-person body joints remain native.
    duke=true;
    object_case(0x80034dd0,100,false,false,0,100);
    interaction=true;object_case(0x80034dd0,100,true,false,0,100);
    // D36I: admitted contact arms retain world depth/OT ordering. They must
    // never acquire the weapon layer's negative depth or deferred slot 1.
    assert(psx_mod_read_word(ot+8)==0);
    for(const auto& v:host_corners) assert(v.depth>0 && v.z>=16);
    interaction=false;
    lower_body=true;object_case(0x80034dd0,100,true,false,0,100);
    assert(psx_mod_read_word(ot+8)==0);
    for(const auto& v:host_corners) assert(v.depth>0 && v.z>=16);
    lower_body=false;duke=false;
    object_case(0x80032288,100,true); // compact static prop beyond near radius
    object_case(0x80032288,100,true,true); // first mesh need not see an empty OT
    object_case(0x80032288,128,true); // inclusive 256-unit extent
    object_case(0x80032288,129,false); // larger scenery stays original
    object_case(0x80033778,100,false); // dancer's articulated mesh
    object_case(0x800351f4,100,false); // actor joint
    object_case(0x80032288,400,false); // distant large scenery stays original
    object_case(0x80032288,160,bridge,false,0x20000000); // compact walk-over pickup
    object_case(0x80032288,161,false,false,0x20000000); // bounded at 320 units
    object_case(0x80032288,160,false,false,0x10000000); // a different type flag
    object_case(0x80033778,160,false,false,0x20000000); // foreign caller
    live_eye=false;object_case(0x80032288,160,false,false,0x20000000);live_eye=true;
    // D17M1: distant tall ladder panels retain their source ordering but
    // participate in platform depth. Other climb types/callers stay native.
    const bool ladder_bridge=bridge && (!std::getenv("DNTTK_LADDER_DEPTH") || std::string_view(std::getenv("DNTTK_LADDER_DEPTH"))!="0");
    object_case(0x80032288,400,ladder_bridge,false,0x200);
    object_case(0x80032288,400,false,false,0x200,8192);
    object_case(0x80032288,400,ladder_bridge,false,0x200,8191);
    object_case(0x80032288,400,false,false,0x200,5000,true);
    object_case(0x80032288,400,false,false,0x100);
    object_case(0x80032288,400,false,false,0x600);
    object_case(0x80033778,400,false,false,0x200);
    live_eye=false;object_case(0x80032288,400,false,false,0x200);live_eye=true;
    // D17AA: host pieces of a safe, opaque, noncompact scenery face must
    // retain AVSZ3/4 source ordering. Otherwise the foreground prop bias can
    // draw a block after an intervening native wall. Preserve native bias,
    // far rejection, and the existing actor/translucent/compact exclusions.
    for(bool quad:{false,true}) for(int bias:{-1,0,1}) for(int kind=0;kind<5;++kind) {
        auto c=reset();c.gpr[31]=kind==1?0x800351f4:0x80032288;
        const int extent=kind==3?100:512;
        const int z0=kind==4?100:1800,z1=kind==4?300:kind==3?2000:2200;
        const int xyz[4][3]={{-extent,-128,z0},{extent,-128,z1},
                             {-extent,128,z0},{extent,128,z1}};
        psx_mod_write_byte(mesh+6,quad?4:3);word(mesh+0x10,verts);word(mesh+0x14,list);
        for(unsigned i=0;i<(quad?4u:3u);++i) {
            word(verts+8*i,uint16_t(xyz[i][0])|uint32_t(uint16_t(xyz[i][1]))<<16);
            word(verts+8*i+4,xyz[i][2]);
        }
        const unsigned bits=1u|(bias<0?4u:bias>0?8u:0u)|(kind==2?16u:0u);
        word(list,quad?0x0001002c:0x00010024);word(list+4,quad?0x03020100:0x00020100);
        word(list+8,0);word(list+12,(bits<<25)|31);word(list+16,0x1f1f1f00);word(list+20,0xff);
        observed_hook(&c,0x80010000);assert(host_vertices>=3);
        const bool eligible=kind==0 && pgxp_mesh_textures() &&
            !(subdiv_env && std::string_view(subdiv_env)=="1");
        if(eligible) {
            const int sum=quad?2*(z0+z1):2*z0+z1;
            const int source_slot=(((quad?256:341)*sum)>>12)/8+bias;
            for(unsigned i=0;i<2048;++i) assert((psx_mod_read_word(ot+i*8)!=0)==(int(i)==source_slot));
            assert(host_corners.front().depth>0); // still carries host depth
        }
    }
    // D17AC: the retail phone back sits inside its mounting wall. Its
    // allowance changes depth only, never packet XY, UV, perspective or OT.
    // Foreign shapes/callers, other panels, transparency, back-side viewing,
    // actors and Vanilla must not acquire the larger contact allowance.
    {
        const int xyz[10][3]={{-105,0,-136},{105,0,-136},{-105,0,136},{105,0,136},
            {-105,-395,-136},{105,-395,-136},{-105,-395,136},{105,-395,136},
            {105,-321,-136},{105,-321,136}};
        std::vector<HostCorner> reference;
        unsigned reference_slot=0;
        for(int kind=0;kind<9;++kind) {
            auto c=reset();c.gpr[31]=kind==2?0x800351f4:0x80032288;
            c.gte_ctrl[0]=0;c.gte_ctrl[1]=kind==5?uint16_t(-4096):4096;
            c.gte_ctrl[3]=kind==5?4096:uint16_t(-4096);c.gte_ctrl[4]=0;
            c.gte_ctrl[7]=1500;
            psx_mod_write_byte(mesh+6,10);word(mesh+0x10,verts);word(mesh+0x14,list);
            for(int i=0;i<10;++i) {
                word(verts+i*8,uint16_t(xyz[i][0]+(kind==1 && i==8))|uint32_t(uint16_t(xyz[i][1]))<<16);
                word(verts+i*8+4,uint16_t(xyz[i][2]));
            }
            word(list,0x0001002c);word(list+4,kind==3?0x09080705:0x06020400);
            word(list+8,0);word(list+12,(kind==4?0x22000000:0x02000000)|31);
            word(list+16,0x1f1f1f00);word(list+20,0xff);
            if(kind==6) word(ctx+0x48,0x02000000);
            if(kind==7) {duke=true;interaction=true;}
            if(kind==8) {modern=false;live_eye=false;}
            observed_hook(&c,0x80010000);
            duke=false;interaction=false;modern=true;live_eye=true;
            if(kind==8) {assert(host_corners.empty());continue;}
            assert(host_corners.size()==6);
            const char* phone_env=std::getenv("DNTTK_PHONE_BACK_DEPTH");
            const bool enabled=!(phone_env && std::string_view(phone_env)=="0");
            for(const auto& v:host_corners) {
                const float offset=kind==0 && enabled?16.0f:kind==7?0.0f:2.0f;
                assert(std::abs(v.depth-(v.z-offset))<0.02f);
                if(kind==0 && enabled) {
                    assert(v.depth<1595); // rear panel survives mounting wall
                    assert(v.depth>1580); // nearer occluder still wins
                }
            }
            unsigned slot=0;for(unsigned i=0;i<2048;++i) if(psx_mod_read_word(ot+i*8)) slot=i;
            if(kind==0) {reference=host_corners;reference_slot=slot;}
            if(kind==1) {
                assert(slot==reference_slot);
                for(unsigned i=0;i<6;++i) {
                    const auto& a=reference[i];const auto& b=host_corners[i];
                    assert(a.word==b.word && a.x==b.x && a.y==b.y && a.z==b.z && a.u==b.u && a.v==b.v);
                }
            }
        }
        std::puts("D17AC phone panel: bounded depth, unchanged raster/UV/order and exclusion guards PASS");
        // D17AL: beyond the host takeover the phone's own native packets get
        // scene depth. They must carry the same contact allowance as its host
        // pieces (16 for the rear panel, 2 for other faces), so it cannot
        // vanish into its wall at a distance and pop in at the handover.
        // Modified shapes keep 2; Vanilla keeps painter order.
        const bool phone_enabled=!(std::getenv("DNTTK_PHONE_BACK_DEPTH") && std::string_view(std::getenv("DNTTK_PHONE_BACK_DEPTH"))=="0");
        for(int kind=0;kind<3;++kind) {
            modern=kind!=2;
            auto c=reset();c.gpr[31]=0x80032288;
            c.gte_ctrl[0]=0;c.gte_ctrl[1]=4096;c.gte_ctrl[3]=uint16_t(-4096);c.gte_ctrl[4]=0;
            c.gte_ctrl[7]=4000;c.gte_ctrl[26]=1024;
            psx_mod_write_byte(mesh+6,10);word(mesh+0x10,verts);word(mesh+0x14,list);
            for(int i=0;i<10;++i) {
                word(verts+i*8,uint16_t(xyz[i][0]+(kind==1 && i==8))|uint32_t(uint16_t(xyz[i][1]))<<16);
                word(verts+i*8+4,uint16_t(xyz[i][2]));
            }
            word(list,0x0002002c);word(list+4,0x06020400);word(list+8,0);word(list+12,0x02000000|31);
            word(list+16,0x07030501);word(list+20,0);word(list+24,0x02000000|31);word(list+28,0xff);
            observed_hook(&c,0x80010000);
            assert(c.gpr[4]==mesh && host_vertices==0); // distant: native packets
            ttk::NearGte g{};g.r[0][2]=4096;g.r[1][1]=4096;g.r[2][0]=-4096;g.tr[2]=4000;g.h=1024;
            constexpr uint32_t ring=0x80160000;
            const int faces[2][4]={{0,4,2,6},{1,5,3,7}};
            float z[2][4];
            for(int f=0;f<2;++f) {
                const uint32_t at=ring+40*f;
                word(at,0x09000000);word(at+4,0x2c808080);
                for(int k=0;k<4;++k) {
                    const int i=faces[f][k];
                    const auto p=ttk::near_project(g,(int16_t)(xyz[i][0]+(kind==1 && i==8)),(int16_t)xyz[i][1],(int16_t)xyz[i][2]);
                    assert(p.safe && p.sz>=3072);
                    word(at+8+8*k,p.sxy);word(at+12+8*k,0);z[f][k]=(float)p.view[2];
                }
            }
            word(ctx,ring+80);
            CPUState close{};observed_hook(&close,0x8001fba0);
            if(kind==2) {assert(host_vertices==0);continue;}
            assert(host_vertices==8);
            for(int f=0;f<2;++f) for(int k=0;k<4;++k) {
                const auto& v=host_corners[4*f+k];
                const float offset=f==0 && kind==0 && phone_enabled?16.0f:2.0f;
                assert(v.word==psx_mod_read_word(ring+40*f+8+8*k));
                assert(std::abs(v.depth-(z[f][k]-offset))<0.02f);
            }
            for(unsigned i=0;i<2048;++i) assert(psx_mod_read_word(ot+i*8)==0);
        }
        modern=true;
        std::puts("D17AL scene-depth prop contact: native phone back 16, other prop faces 2, shape/Vanilla guards PASS");
    }
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
    // D17X: a warped quad meeting a floor must retain the PS1 1-2
    // diagonal and a valid plane for each half, including per-corner UVs.
    {
        auto c=reset();c.gpr[31]=0x80032288;
        psx_mod_write_byte(mesh+6,4);word(mesh+0x10,verts);word(mesh+0x14,list);
        const int xyz[4][3]={{-100,-100,5000},{100,-100,5000},
                             {-100,100,5000},{100,100,5050}};
        for(int i=0;i<4;++i) {
            word(verts+i*8,uint16_t(xyz[i][0])|uint32_t(uint16_t(xyz[i][1]))<<16);
            word(verts+i*8+4,xyz[i][2]);
        }
        word(list,0x0001002c);word(list+4,0x03020100);
        word(list+8,0);word(list+12,0x0200001f);
        word(list+16,0x1f1f1f00);word(list+20,0xff);
        observed_hook(&c,0x80010000);assert(host_corners.size()==6);
        const char* split=std::getenv("DNTTK_WARPED_QUADS");
        const bool native=!(split && std::string_view(split)=="0");
        const int order[2][6]={{0,1,3,0,3,2},{0,1,2,2,1,3}};
        for(int i=0;i<6;++i) {
            const int source=order[native][i];const auto& v=host_corners[i];
            assert(v.z==xyz[source][2]);
            assert(v.u==(source%2?31:0) && v.v==(source>=2?31:0));
            if(native && i<3) assert(std::abs(v.depth-4998.0f)<0.01f);
        }
    }
    // D17W: a textured face and a flat face on the same corners (the level 3
    // minecart). The original paints the earlier record last, so that one
    // must win the host depth test: the other is pushed behind its plane.
    for(bool textured_first:{true,false}) {
        auto c=reset();c.gpr[31]=0x80032288;
        psx_mod_write_byte(mesh+6,3);word(mesh+0x10,verts);word(mesh+0x14,list);
        word(verts,(uint16_t)-100|uint32_t(uint16_t(-100))<<16);word(verts+4,5000);
        word(verts+8,100|uint32_t(uint16_t(-100))<<16);word(verts+12,5000);
        word(verts+16,100u<<16);word(verts+20,5000);word(verts+24,0x00405060);
        uint32_t at=list;
        for(int face=0;face<2;++face) {
            // One group per face: FT3 (16-byte record) or F3 (8 bytes, palette 0).
            const bool tex=(face==0)==textured_first;
            word(at,tex?0x00010024:0x00010020);word(at+4,0x00020100);word(at+8,0);
            if(tex) {word(at+12,0x02000000|0x1f);word(at+16,0x1f00);}
            at+=tex?20:12;
        }
        word(at,0xff);
        observed_hook(&c,0x80010000);
        assert(host_corners.size()==6);
        // Corners 0-2 belong to the first record, 3-5 to the second.
        for(int i=0;i<3;++i) {
            assert(host_corners[i].x==host_corners[3+i].x && host_corners[i].y==host_corners[3+i].y);
            assert(host_corners[i].depth<host_corners[3+i].depth);
            assert(host_corners[3+i].depth<host_corners[i].depth*1.001f);
        }
        // Flat pieces are never depth-tested: both faces share one slot and
        // the face the original paints under (the later record) goes in last, so the
        // OT draws it first.
        unsigned slots=0;uint32_t head=0;
        for(unsigned i=0;i<2048;++i) if(const uint32_t h=psx_mod_read_word(ot+i*8)) {++slots;head=h;}
        assert(slots==1);
        assert((psx_mod_read_word(head+4)>>24)==(textured_first?0x30u:0x34u));
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
        if(kind==7) { // whole-mesh budget fallback: this frame's host arena is spent
            word(bm,1);auto state=ttk::near_clip_frame_state();
            state.host_bytes=0xffffffffu;ttk::near_clip_load_frame_state(state);
        }
        if(kind==13) word(ctx+8,0x80161000); // D17R1: a full game ring no longer matters
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
    top_case(6,0x80032288,true);top_case(11,0x80032288,true);top_case(13,0x80032288,true);
    top_case(0,0x80033778,false);top_case(0,0x800351f4,false);
    live_eye=false;top_case(0,0x80032288,false);live_eye=true;
    live_eye=false;object_case(0x80032288,100,false); // Vanilla/original view
    live_eye=true;
    // Changed caller instruction must invalidate permission, even with the
    // expected return address and a previously authenticated code image.
    word(0x80032280,psx_mod_read_word(0x80032280)^1);++g_dirty_ram_code_gen;
    object_case(0x80032288,100,false);
    // D17AF: water tile near clip - takeover with painter depth, grid mismatch
    // and safe pass-through. Requires game code for identity(); guard rejections
    // are in water_clip_contracts(). Restore the code region XOR'd by the
    // preceding identity-failure test so identity() and water_render_identity()
    // pass for the water hook.
    word(0x80032280,psx_mod_read_word(0x80032280)^1);++g_dirty_ram_code_gen;
    {
        setenv("DNTTK_WATER_CLIP","1",1);
        constexpr uint32_t wpkt=0x80165000,wrgn=0x80165080,wstk=0x80165100;
        constexpr uint32_t wrw0=0x80165200,wrw1=0x80165220;
        auto half=[](uint32_t a,uint16_t v){psx_mod_write_byte(a,v&0xff);psx_mod_write_byte(a+1,v>>8);};
        // FT4 opaque packet: size byte=9 (10 words), command 0x2c.
        word(wpkt,9u<<24);word(wpkt+4,0x2c808080u);
        for(int i=0;i<4;++i)word(wpkt+12+i*8,0x40404040u); // UV words
        half(wpkt+14,0x0100u);half(wpkt+22,0x0019u); // CLUT, tpage
        // Region X/Y base and stack link; zero the water Y-offset field in RAM.
        word(0x800c3cd8u,0);
        half(wrgn,0);half(wrgn+2,100);          // X_base=0, Y_base=100
        word(wstk+0x90,wrgn-8);half(wstk+0x58,1200u); // near z=176, far z=1200
        // Compute expected SXY/SZ for four corners via near_project with H=256.
        // Corner (x, y, z): 0=(0,100,176) 1=(1024,100,176) 2=(0,100,1200) 3=(1024,100,1200).
        // Corner 1 saturates on X (unsafe); corner 3 is safe.
        ttk::NearGte g{};for(int i=0;i<3;++i)g.r[i][i]=4096;g.h=256;
        auto v0=ttk::near_project(g,0,100,176),v1=ttk::near_project(g,1024,100,176);
        auto v2=ttk::near_project(g,0,100,1200),v3=ttk::near_project(g,1024,100,1200);
        assert(!v1.safe && v2.safe && v3.safe); // confirm saturation
        word(wrw0+ 0,v0.sxy);word(wrw0+ 4,v0.sz); // records[0]: row0
        word(wrw0+16,v1.sxy);word(wrw0+20,v1.sz); // records[1]: row0+16
        word(wrw1+ 0,v2.sxy);word(wrw1+ 4,v2.sz); // records[2]: row1
        word(wrw1+16,v3.sxy);word(wrw1+20,v3.sz); // records[3]: row1+16
        auto wc=reset();
        wc.gpr[31]=0x8002fbb4u;wc.gpr[5]=wpkt;wc.gpr[21]=wrgn;
        wc.gpr[29]=wstk;wc.gpr[17]=wrw0;wc.gpr[18]=wrw1;wc.gpr[20]=0;wc.gpr[4]=500;
        observed_hook(&wc,0x8002bc18);
        assert(!host_corners.empty()); // takeover: corner 1 unsafe
        for(const auto& v:host_corners) assert(v.depth<0); // painter depth
        assert((psx_mod_read_word(wpkt)>>24)==0); // native packet link zeroed
        // Grid mismatch: corrupt one record SXY -> grid_reject, no host corners.
        word(wpkt,9u<<24); // restore packet header for repeated call
        word(wrw0,v0.sxy^1);
        host_corners.clear();observed_hook(&wc,0x8002bc18);
        assert(host_corners.empty());
        word(wrw0,v0.sxy); // restore
        // Safe pass-through: far tile at z=3100 (near 2076), small screen span.
        half(wstk+0x58,3100u);
        auto u0=ttk::near_project(g,0,100,2076),u1=ttk::near_project(g,1024,100,2076);
        auto u2=ttk::near_project(g,0,100,3100),u3=ttk::near_project(g,1024,100,3100);
        assert(u0.safe && u1.safe && u2.safe && u3.safe);
        word(wrw0+ 0,u0.sxy);word(wrw0+ 4,u0.sz);
        word(wrw0+16,u1.sxy);word(wrw0+20,u1.sz);
        word(wrw1+ 0,u2.sxy);word(wrw1+ 4,u2.sz);
        word(wrw1+16,u3.sxy);word(wrw1+20,u3.sz);
        word(wpkt,9u<<24);
        host_corners.clear();observed_hook(&wc,0x8002bc18);
        assert(host_corners.empty()); // safe, not oversized -> pass-through
        unsetenv("DNTTK_WATER_CLIP");
        std::puts("D17AF water_clip: painter depth takeover, grid mismatch, safe pass-through PASS");
    }
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
static void biomask_packet_contracts() {
    // Animation overrides and the mask must coexist in the same 1 MiB aperture.
    assert(psx_mod_alloc_gpu_dma_memory(320*1024,16));
    constexpr uint32_t ot=0x800d27a0,bitmap=0x800d26a0,weapon=0x801f0000,hud=0x801f0100;
    std::vector<uint32_t> colors;
    auto draw=[&](double bob,uint32_t light=0x808080){
        colors.clear();
        ttk::near_clip_load_frame_state({0,0,0,0});host_corners.clear();
        psx_mod_write_word(bitmap,0xc0000000u);
        psx_mod_write_word(ot,hud);psx_mod_write_word(ot+4,hud);psx_mod_write_word(hud,0x03000000);
        psx_mod_write_word(ot+8,weapon);psx_mod_write_word(ot+12,weapon);psx_mod_write_word(weapon,0x03000000);
        ttk::near_clip_biomask(bob,light);
        assert(psx_mod_read_word(ot)==hud && psx_mod_read_word(hud)==0x03000000);
        assert(psx_mod_read_word(ot+8)==weapon);
        uint32_t node=psx_mod_read_word(weapon)&0xffffff;unsigned n=0;
        while(node){
            assert(++n<=2400);const uint32_t p=node|0x80000000u;
            assert((psx_mod_read_word(p)>>24)==9);
            assert((psx_mod_read_word(p+4)>>24)==0x34);
            colors.push_back(psx_mod_read_word(p+4)&0xffffff);
            assert((psx_mod_read_word(p+12)>>16)==0x7fff);
            assert((psx_mod_read_word(p+24)>>16)==0x7e00);
            node=psx_mod_read_word(p)&0xffffff;
        }
        // Art revisions can simplify geometry; reject an empty/mostly lost mesh
        // without imposing the previous ribbed asset's polygon count.
        constexpr auto source_triangles=sizeof(ttk::bio_mask_mesh)/sizeof(ttk::bio_mask_mesh[0]);
        assert(n>=source_triangles/4 && n<=source_triangles && host_corners.size()==n*3);
        for(const auto& c:host_corners){assert(c.z>0 && c.depth<0);assert(c.u>=0 && c.u<=255 && c.v>=0 && c.v<=127);}
        return host_corners.front().y;
    };
    const auto allocations_before_draw=dma_allocations;
    const auto resting=draw(0),moving=draw(20);
    assert(dma_allocations==allocations_before_draw); // No late mask reservation after load.
    assert(std::abs((moving-resting)/65536.0-3.0)<.001);
    assert(draw(0)==resting);
    const auto neutral=colors;
    for(uint32_t light:{0u,0x404040u,0x2080ffu,0xffffffu}) {
        assert(draw(0,light)==resting);
        for(size_t i=0;i<colors.size();++i)for(int k=0;k<3;++k)
            assert(((colors[i]>>(8*k))&255)==std::min(255u,(((neutral[i]>>(8*k))&255)*((light>>(8*k))&255))>>7));
    }
    draw(0);assert(colors==neutral); // No tint survives the next draw/load.
    std::puts("PASS: Bio Mask native-light modulation, dark/neutral/tinted/bright, preserves geometry");
    std::puts("PASS: Bio Mask real mesh packets follow weapons, preserve HUD, carry external atlas and precise bob");
}
static void water_clip_contracts() {
    // D17AF: guard rejections verified without the game executable.
    constexpr uint32_t wpkt=0x80170000,wrgn=0x80170080,wstk=0x80170100;
    constexpr uint32_t wrw0=0x80170200,wrw1=0x80170220;
    psx_mod_write_word(wpkt,9u<<24);psx_mod_write_word(wpkt+4,0x2c808080u);
    for(int i=0;i<4;++i)psx_mod_write_word(wpkt+12+i*8,0x40404040u);
    psx_mod_write_byte(wpkt+14,0x00);psx_mod_write_byte(wpkt+15,0x01); // CLUT
    psx_mod_write_byte(wpkt+22,0x19);psx_mod_write_byte(wpkt+23,0x00); // tpage
    psx_mod_write_byte(wrgn,0);psx_mod_write_byte(wrgn+1,0);
    psx_mod_write_byte(wrgn+2,100);psx_mod_write_byte(wrgn+3,0);
    psx_mod_write_word(wstk+0x90,wrgn-8);
    psx_mod_write_byte(wstk+0x58,0xb0);psx_mod_write_byte(wstk+0x59,0x04); // z=1200
    CPUState c{};
    c.gte_ctrl[0]=c.gte_ctrl[2]=c.gte_ctrl[4]=4096;
    c.gte_ctrl[26]=256;c.gte_ctrl[29]=341;c.gte_ctrl[30]=256;
    c.gpr[5]=wpkt;c.gpr[21]=wrgn;c.gpr[29]=wstk;
    c.gpr[17]=wrw0;c.gpr[18]=wrw1;c.gpr[20]=0;c.gpr[4]=0;
    // Wrong RA: water_clip checks gpr[31]==0x8002fbb4u before water_seen.
    c.gpr[31]=0x8002fbb0u;host_corners.clear();
    observed_hook(&c,0x8002bc18);assert(host_corners.empty());
    c.gpr[31]=0x8002fbb4u;
    // DNTTK_WATER_CLIP=0 (explicitly disabled) -> guard reject.
    const char* old_wc=std::getenv("DNTTK_WATER_CLIP");
    const std::string saved_wc=old_wc?old_wc:"";
    setenv("DNTTK_WATER_CLIP","0",1);
    host_corners.clear();observed_hook(&c,0x8002bc18);assert(host_corners.empty());
    setenv("DNTTK_WATER_CLIP","1",1);
    // !modern -> guard reject.
    modern=false;host_corners.clear();observed_hook(&c,0x8002bc18);
    assert(host_corners.empty());modern=true;
    // !live_eye (widescreen_near_clip_live is always false in test) -> reject.
    live_eye=false;host_corners.clear();observed_hook(&c,0x8002bc18);
    assert(host_corners.empty());live_eye=true;
    if(old_wc)setenv("DNTTK_WATER_CLIP",saved_wc.c_str(),1);else unsetenv("DNTTK_WATER_CLIP");
    std::puts("PASS: D17AF water_clip RA mismatch, DNTTK_WATER_CLIP=0, !modern, !live_eye guards reject");
}
static void frozen_packet_contracts() {
    constexpr uint32_t ot=0x800d27a0,bitmap=0x800d26a0,weapon=0x801f0000,hud=0x801f0100;
    const auto allocations=dma_allocations;
    for(int margin:{0,85,256,427})for(unsigned page:{0x19u,0x39u,0x59u,0x79u,0x19fu}) {
        test_margin=margin;ttk::near_clip_load_frame_state({0,0,0,0});
        for(unsigned i=0;i<2048*8;i+=4)psx_mod_write_word(ot+i,0);
        psx_mod_write_word(bitmap,0xc0000000u);
        psx_mod_write_word(ot,hud);psx_mod_write_word(ot+4,hud);psx_mod_write_word(hud,0x03000000);
        psx_mod_write_word(ot+8,weapon);psx_mod_write_word(ot+12,weapon);
        psx_mod_write_word(weapon,0x09000000);psx_mod_write_word(weapon+4,0x34808080);
        psx_mod_write_word(weapon+24,page<<16);psx_mod_write_word(weapon+12,0);
        ttk::near_clip_frozen_tint();
        const auto p=psx_mod_read_word(ot+12);
        assert(p!=weapon && psx_mod_read_word(p)==(24u<<24));
        assert(psx_mod_read_word(p+20)==((page&~0x60u)<<16));
        assert(psx_mod_read_word(p+88)==(page<<16));
        assert(psx_mod_read_word(p+32)==0x2a400808u && psx_mod_read_word(p+52)==0x2a400808u);
        assert(int16_t(psx_mod_read_word(p+36))==-256-margin);
        assert(int16_t(psx_mod_read_word(p+60))==256+margin);
        assert(psx_mod_read_word(ot)==hud && psx_mod_read_word(hud)==0x03000000);
        assert((psx_mod_read_word(weapon)&0xffffff)==(p&0xffffff));
    }
    test_margin=85;assert(dma_allocations==allocations);
    std::puts("PASS: D35J5 tint packet page restoration, all blend modes, widescreen and HUD ordering");
}
int main(int argc,char** argv) {
    // A load-first session must own its world and mask packet arenas before
    // savestate restore can import any unclaimed allocation tail. Animation
    // gets the remaining 320 KiB below, without a duplicate mask allocation.
    assert(dma_allocations==2);
    assert(dma_next==0x80f00000u+(512u+188u)*1024u+16u);
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
    biomask_packet_contracts();
    frozen_packet_contracts();
    water_clip_contracts();
    std::puts("ttk-near-test PASS");
}
