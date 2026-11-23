// D41L: props that are light sources (prop_lights.h). See
// documentation/436-d41l-saloon-light.md.
//
// Time to Kill paints its room light into vertex colours; a saloon's
// woodburner and candle chandelier give none. In Modernized (Polymorph):
//
// - The iron woodburner (28 vertices, a 540 x 460 body on a pipe, the grate
//   on the door face at model +z) burns: embers glow behind its grate (an
//   additive quad over it) with a faint bloom in front, and a low fire light
//   flickers on the floor and walls before it.
// - The candle chandelier (twelve radial arm planes, 896 across, a candle at
//   each arm's tip) is lit: a small flame on every candle, each with a soft
//   halo, a warm light among the candles for the ceiling and arms, and a
//   broad soft fill that reaches the room below.
//
// D57F: an energy crystal (types 176-178) set into a crystal holder other
// than the one it starts in (the time machine's three holders in OBEY OR
// DIE) shines there as in its first holder: a light in the
// colour, reach and strength of the level lamp bound to it there (D41D1 sign
// lamp), at the same offset; it goes when the crystal is taken.
//
// Both are found by their model data, so every copy in every level takes
// part. The lights join the D41A list (DYNAMIC LIGHTS on) after the level
// lamps and the D53 screens, nearest to Duke first, gated by the room
// Polymorph shows; the flames and glows are drawn by the near-clip object
// path from the second texture bank (x 256..335, painted here). Classic and
// Vanilla: nothing changes.
#include "prop_lights.h"
#include "prop_state.h"
#include "near_clip.h"
#include "duke_flicker.h"
#include "live_env.h"
#include "pc_input.h"
#include "cpu_state.h"
#include "gpu.h"
#if defined(PSX_SDL3)
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#endif
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace ttk {
namespace {
constexpr uint32_t players=0x800d7198u,player_index=0x800d2654u;
constexpr double pi=3.14159265358979;

// ---- models -------------------------------------------------------------
bool mesh_header(uint32_t mesh,unsigned count,uint32_t& vp,uint32_t& list) {
    if(mesh<0x80010000u || mesh>0x801ffff0u || (mesh&3)) return false;
    if(psx_mod_read_byte(mesh+6)!=count) return false;
    vp=psx_mod_read_word(mesh+0x10);list=psx_mod_read_word(mesh+0x14);
    return vp>=0x80010000u && vp<=0x801fff00u && list>=0x80010000u && list<=0x801fff00u && !(vp&3) && !(list&3);
}
// The woodburner: first corner (-270, 1435, -230), pipe top (109, -1535, -18).
bool stove_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,28,vp,list) && psx_mod_read_word(vp)==0x059bfef2u && psx_mod_read_half(vp+4)==0xff1au &&
           psx_mod_read_word(vp+8*24)==0xfa01006du && psx_mod_read_half(vp+8*24+4)==0xffeeu;
}
// The chandelier: twelve FT4 arm planes, first corner (-896, 235, 0), the
// chain's foot (0, -235, -25).
bool chandelier_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,28,vp,list) && psx_mod_read_word(list)==0x000c002cu && psx_mod_read_word(vp)==0x00ebfc80u &&
           psx_mod_read_half(vp+4)==0 && psx_mod_read_word(vp+8*26)==0xff150000u && psx_mod_read_half(vp+8*26+4)==0xffe7u;
}
// D57G: the iron-ring chandelier (prop type 372; RESISTANCE IS FEUDAL):
// 51 vertices, a pentagonal ring (y 570..950) on five chains to the apex,
// a candle flame at each corner (part of the model). List header
// 0x0014002c, first vertex (-4, 948, 828), the apex (0, -955, -67).
bool ring_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,51,vp,list) && psx_mod_read_word(list)==0x0014002cu && psx_mod_read_word(vp)==0x03b4fffcu &&
           psx_mod_read_half(vp+4)==0x033cu && psx_mod_read_word(vp+8*50)==0xfc450000u && psx_mod_read_half(vp+8*50+4)==0xffbdu;
}
// D57F: the time machine's hanging top (prop type 192; levels 0 and 5): 40
// vertices, an octagonal body (y -763..-87) on four prongs down to y 666.
// List header 0x0018002c, first vertex (-590, -763, -244), the last
// (162, 666, -518).
// D41X: the fire barrel (prop type 220; levels 0 and 5): 20 vertices, the
// drum (y 374 at the floor up to its open rim at -122, 320 x 276) with a
// fire cross standing in it up to y -374. The original paints its glow into
// the room's vertex colours; it gives no light of its own. First vertex
// (0, -122, 0), the last (80, 374, -138).
bool barrel_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,20,vp,list) && psx_mod_read_word(vp)==0xff860000u && psx_mod_read_half(vp+4)==0 &&
           psx_mod_read_word(vp+8*19)==0x01760050u && psx_mod_read_half(vp+8*19+4)==0xff76u;
}
// D41Y: the construction barricade (prop type 128; level 0 rooms 20 and 23):
// 9 vertices, an orange-and-white A-frame (feet at y 255, z +-119, top rail
// to y -138) with its warning lamp on top at one end, a 16 x 16 quad
// (x -205..-92, y -255..-138, z 2). The lamp's texture is animated: the
// game's own blink, a lit frame and a dark frame of 600 ms, all barricades in
// step. First vertex (-205, 255, -119), the last (-205, -255, 2).
bool barricade_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,9,vp,list) && psx_mod_read_word(vp)==0x00ffff33u && psx_mod_read_half(vp+4)==0xff89u &&
           psx_mod_read_word(vp+8*8)==0xff01ff33u && psx_mod_read_half(vp+8*8+4)==0x0002u;
}
// D41Z: the caged wall lamp (prop types 665 and 667 pale, 851 red; levels 0,
// 5 and 9, one model record per type and level): 26 vertices, a bulb 86 wide
// and 200 tall behind four bars, standing out from its wall plate (model z
// 31) to z -31; model -z faces the room. Its texture is steady (no animated
// face). First vertex (0, 100, -3), the last (-43, -99, 31).
bool cage_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,26,vp,list) && psx_mod_read_word(vp)==0x00640000u && psx_mod_read_half(vp+4)==0xfffdu &&
           psx_mod_read_word(vp+8*25)==0xff9dffd5u && psx_mod_read_half(vp+8*25+4)==0x001fu;
}
// D41AA: the subway train (prop type 77; level 0): 69 vertices, a car 9470
// long (model x -9361..110, y -1794 roof..46 rails, z +-898). Its nose (x
// +110, a sloped cab) leads when it runs; both ends carry a bumper cluster of
// three lamps (amber, red, amber) and the flat tail end (x -9361) a cyan
// beacon on the roof. Its texture is steady (no animated face). First vertex
// (-2, 46, -899), the last (-341, -357, 683).
bool train_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,69,vp,list) && psx_mod_read_word(vp)==0x002efffeu && psx_mod_read_half(vp+4)==0xfc7du &&
           psx_mod_read_word(vp+8*68)==0xfe9bfeabu && psx_mod_read_half(vp+8*68+4)==0x02abu;
}
// D57C: the subway entrance globe lamp (prop type 315; level 0, the two on
// the posts at the top of the "Subway" stairs): a sphere of 25 vertices,
// 222 wide and 200 tall about its own centre, its texture a bright yellow
// glass (lamp_colour also requires most of it lit: a plain ball with the
// same mesh would not pass). First vertex (0, -100, 0), the last (29, 100,
// 29).
bool globe_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,25,vp,list) && psx_mod_read_word(vp)==0xff9c0000u && psx_mod_read_half(vp+4)==0x0000u &&
           psx_mod_read_word(vp+8*24)==0x0064001du && psx_mod_read_half(vp+8*24+4)==0x001du;
}
// D57I: the LA street lamp post (prop type 160; levels 0, 5 and 9, the LA
// street block): 26 vertices, a pole (model z -611..-467, foot y 1462 on the
// street, top -1461) whose arm reaches out over the road to the lamp head
// (z 421..611, y -1462..-1360). Its lens is the head's underside, two faces
// painted with one white-hot bulb, meeting at (0, -1360, 523). First vertex
// (-71, 1462, -611), the last (75, -1446, 611).
bool street_lamp_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,26,vp,list) && psx_mod_read_word(vp)==0x05b6ffb9u && psx_mod_read_half(vp+4)==0xfd9du &&
           psx_mod_read_word(vp+8*25)==0xfa5a004bu && psx_mod_read_half(vp+8*25+4)==0x0263u;
}
// D57J: the LA strip club's fixtures (levels 0, 5 and 9). The ceiling neon
// bar (prop type 193): 48 vertices, a magenta tube along model x -2000..2000
// at y 0 (the ceiling) and four spot cans hanging under it (y 102..277) at x
// +-550 and +-1600, their lenses (x, 232, -160) aimed along model -z and 25
// degrees down. First vertex (-1600, 0, 0), the last (1550, 102, 0).
bool neon_bar_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,48,vp,list) && psx_mod_read_word(vp)==0x0000f9c0u && psx_mod_read_half(vp+4)==0x0000u &&
           psx_mod_read_word(vp+8*47)==0x0066060eu && psx_mod_read_half(vp+8*47+4)==0x0000u;
}
// The pole stage's canopy (prop type 117): a red hexagonal lampshade, 13
// vertices, its open rim at model y 0 (radius about 500 about (-38, 27)) and
// its top at y -448, a ring of bulbs painted round its band. First vertex
// (-31, -448, -41), the last (416, 0, 307).
bool canopy_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,13,vp,list) && psx_mod_read_word(vp)==0xfe40ffe1u && psx_mod_read_half(vp+4)==0xffd7u &&
           psx_mod_read_word(vp+8*12)==0x000001a0u && psx_mod_read_half(vp+8*12+4)==0x0133u;
}
// The back bar's bottle shelves (prop type 4, over the counter): 26
// vertices, 3072 x 582 x 988. First vertex (-512, 291, 71), the last (1365,
// -291, 71).
bool club_bar_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,26,vp,list) && psx_mod_read_word(vp)==0x0123fe00u && psx_mod_read_half(vp+4)==0x0047u &&
           psx_mod_read_word(vp+8*25)==0xfedd0555u && psx_mod_read_half(vp+8*25+4)==0x0047u;
}
// D57D: the shaded table lamp (prop type 228; level 0: on the box table in
// the strip club's back room behind the stage, and in a furnished room off
// the street): 20 vertices, a square stem (model y 38..196, 108 across) under
// a four-sided shade, its open rim at y 21 (278 across) narrowing to the top
// at y -196 (98 across), painted a pale warm cream (248 248 176 at its
// brightest). First vertex (-54, 196, -54), the last (-49, -196, 49).
bool table_lamp_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,20,vp,list) && psx_mod_read_word(vp)==0x00c4ffcau && psx_mod_read_half(vp+4)==0xffcau &&
           psx_mod_read_word(vp+8*19)==0xff3cffcfu && psx_mod_read_half(vp+8*19+4)==0x0031u;
}
// D57K: the hanging ceiling lamp (prop type 229; level 0 room 8, the
// furnished apartment off the LA street): 16 vertices, 272 x 940 x 274; a
// cord from the ceiling (model y -470) down to a shallow four-sided cream
// shade (apex y 379, rim y 451, 272 across) with a white bulb hanging under
// it (y 448..470). First vertex (13, 470, 13), the last (136, 451, 137).
bool pendant_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,16,vp,list) && psx_mod_read_word(vp)==0x01d6000du && psx_mod_read_half(vp+4)==0x000du &&
           psx_mod_read_word(vp+8*15)==0x01c30088u && psx_mod_read_half(vp+8*15+4)==0x0089u;
}
// D57M: the bare bulb dangling on its cord (prop type 929; level 5 room 8,
// the LA apartment in Obey or Die): 16 vertices, 84 x 604 x 84; a cord of two
// crossed quads 22 wide from the ceiling (model y -302) down to y 181, and the
// bulb, two crossed quads 84 wide at y 181..302. First vertex (-42, 181, 0),
// the last (0, 181, -11).
bool bulb_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,16,vp,list) && psx_mod_read_word(vp)==0x00b5ffd6u && psx_mod_read_half(vp+4)==0x0000u &&
           psx_mod_read_word(vp+8*15)==0x00b50000u && psx_mod_read_half(vp+8*15+4)==0xfff5u;
}
// D57H: the red candle on its tall iron stand (prop type 361; level 8): 59
// vertices, 190 x 730 x 190; a four-legged foot (model y 279..365, 190
// across), a square iron pole (22 across) up to y -240, a drip tray, the red
// candle (a hexagonal block 60 across, y -260..-339) and its flame, four
// crossed quads of the game's animated flame texture (y -336..-365). The
// prop stands at y -365 over its floor, so the flame is about 700 up. First
// vertex (-11, 297, -11), the last (0, -365, 11).
bool candle_stand_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,59,vp,list) && psx_mod_read_word(vp)==0x0129fff5u && psx_mod_read_half(vp+4)==0xfff5u &&
           psx_mod_read_word(vp+8*58)==0xfe930000u && psx_mod_read_half(vp+8*58+4)==0x000bu;
}
// D57B: the secret DeLorean (prop type 238; GOLD AND GUNS room 37, in a
// dark cave): 67 vertices, 756 x 463 x 2112. Its nose is at model -z (z
// -1032: the front panel, a pair of square headlamps at each end of a black
// DMC grille, x +-200..+-290, y 26..141), its tail at +z (z 1080: the time
// machine's vents and an orange louvered panel, y -57..53), its floor at y
// 182, the roof y -233. First vertex (-293, 141, -1032), the last (230, -57,
// 1020).
bool delorean_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,67,vp,list) && psx_mod_read_word(vp)==0x008dfedbu && psx_mod_read_half(vp+4)==0xfbf8u &&
           psx_mod_read_word(vp+8*66)==0xffc700e6u && psx_mod_read_half(vp+8*66+4)==0x03fcu;
}
// D57N: the atomic health pickup (type 581; two model records, one per
// way it is turned): 53 vertices, a red and blue atom in cyan orbits, first
// vertex (0, -60, 0), the last at y 81 (x and z differ between the two).
bool atomic_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,53,vp,list) && psx_mod_read_word(vp)==0xffc40000u && psx_mod_read_half(vp+4)==0 &&
           psx_mod_read_half(vp+8*52+2)==0x0051u;
}
bool lit_mesh(uint32_t mesh) {return delorean_mesh(mesh) || candle_stand_mesh(mesh) || barrel_mesh(mesh) || barricade_mesh(mesh) || cage_mesh(mesh) || train_mesh(mesh) || globe_mesh(mesh) || street_lamp_mesh(mesh) || table_lamp_mesh(mesh) || pendant_mesh(mesh) || bulb_mesh(mesh);}
bool portal_top_mesh(uint32_t mesh) {
    uint32_t vp,list;
    return mesh_header(mesh,40,vp,list) && psx_mod_read_word(list)==0x0018002cu && psx_mod_read_word(vp)==0xfd05fdb2u &&
           psx_mod_read_half(vp+4)==0xff0cu && psx_mod_read_word(vp+8*39)==0x029a00a2u && psx_mod_read_half(vp+8*39+4)==0xfdfau;
}
int kind_of(uint32_t mesh) {
    if(portal_top_mesh(mesh)) return prop_light_portal_top;
    if(stove_mesh(mesh)) return prop_light_stove;
    if(chandelier_mesh(mesh)) return prop_light_chandelier;
    if(ring_mesh(mesh)) return prop_light_ring;
    if(barricade_mesh(mesh)) return prop_light_barricade;
    if(cage_mesh(mesh)) return prop_light_cage;
    if(train_mesh(mesh)) return prop_light_train;
    if(globe_mesh(mesh)) return prop_light_globe;
    if(street_lamp_mesh(mesh)) return prop_light_street;
    if(table_lamp_mesh(mesh)) return prop_light_table_lamp;   // D57D
    if(pendant_mesh(mesh)) return prop_light_pendant;   // D57K
    if(bulb_mesh(mesh)) return prop_light_bulb;   // D57M
    if(candle_stand_mesh(mesh)) return prop_light_candle_stand;   // D57H
    if(delorean_mesh(mesh)) return prop_light_delorean;   // D57B
    if(atomic_mesh(mesh)) return prop_light_atomic;   // D57N
    if(neon_bar_mesh(mesh)) return prop_light_neon_bar;   // D57J
    if(canopy_mesh(mesh)) return prop_light_canopy;
    if(club_bar_mesh(mesh)) return prop_light_club_bar;
    return prop_light_none;
}
bool visuals() {
    return prop_lights_enabled.load(std::memory_order_relaxed) && input_modernized() && !ttk_world_classic();
}
uint32_t now_ms() {return ttk_draw_clock_ms();}   // D23N1: held during a redraw job

// Tuning (developer): DNTTK_PROP_LIGHT_<NAME>="r g b strength radius".
struct Tune {float rgb[3],k,radius;};
Tune fire_light{{1.0f,0.46f,0.15f},1.5f,2800};          // the stove's fire, before its door
Tune candle_light{{1.0f,0.80f,0.52f},0.9f,1600};        // among the candles: ceiling and arms
Tune fill_light{{1.0f,0.74f,0.46f},0.5f,5000};          // the chandelier's broad fill below
// D41X: the fire barrel's open fire, in the flame above the rim (developer
// DNTTK_PROP_LIGHT_BARREL; DNTTK_PROP_BARREL_Y its model height).
Tune barrel_light{{1.0f,0.52f,0.18f},1.3f,3000};
float barrel_at[3]={0,-300,0};
// D41Y: the barricade's warning lamp: a small amber light at the lamp, on
// and off with the game's blink (developer DNTTK_PROP_LIGHT_BARRICADE), and
// a soft halo over the lamp face while it is lit (DNTTK_PROP_BARRICADE_HALO=
// "size level"). The light switches on with the lit frame and dies in
// barricade_decay ms after it, as a bulb.
Tune barricade_light{{1.0f,0.66f,0.10f},0.7f,1100};
constexpr float barricade_lamp_at[3]={-148,-196,0};
float barricade_halo_size=240,barricade_halo_level=90,barricade_decay=90;
// The lamp's level now per barricade model (any thread reads it for the
// halo): written by the game thread's frame hook from the D55 reading.
struct BlinkSlot {std::atomic<uint32_t> mesh{0};std::atomic<float> level{0};std::atomic<float> rgb[3]{{1.0f},{0.77f},{0.0f}};};
BlinkSlot blink_slots[8];
BlinkSlot* blink_slot(uint32_t mesh,bool add) {
    for(BlinkSlot& b:blink_slots) if(b.mesh.load(std::memory_order_relaxed)==mesh) return &b;
    if(add) for(BlinkSlot& b:blink_slots) if(!b.mesh.load(std::memory_order_relaxed)) {b.mesh.store(mesh,std::memory_order_relaxed);return &b;}
    return nullptr;
}
// D41Z: the caged wall lamp: a steady light just before its bulb in the
// bulb's own colour (cage_colour: pale warm white, or red), and a soft halo
// over the bulb. Developer: DNTTK_PROP_LIGHT_CAGE="r g b strength radius"
// (r g b: the colour when the texture cannot be read), DNTTK_PROP_CAGE_AT=
// "z" its model depth, DNTTK_PROP_CAGE_HALO="size level".
Tune cage_light{{1.0f,0.90f,0.62f},0.8f,1600};
float cage_at[3]={0,0,-90};
float cage_halo_size=300,cage_halo_level=60;
// The colour per lamp model (any thread reads it for the halo): written by
// the game thread's prop scan.
struct CageSlot {std::atomic<uint32_t> mesh{0};std::atomic<float> rgb[3]{{1.0f},{0.90f},{0.62f}};};
CageSlot cage_slots[16];
const CageSlot* cage_slot(uint32_t mesh) {
    for(const CageSlot& c:cage_slots) if(c.mesh.load(std::memory_order_relaxed)==mesh) return &c;
    return nullptr;
}
// The bulb's colour: the mean of the model's lit texels (brightest channel
// 0.4 and up, read from VRAM through each face's CLUT), scaled to its
// brightest channel and deepened (power 1.5) so a pale bulb gives a warm
// light and a red one a red light. False when the texture is not readable.
// D57C: lit (optional) gets the share of the texels that are lit.
bool lamp_colour(uint32_t mesh,unsigned vertices,float rgb[3],float* lit=nullptr) {
    uint32_t vp,cur;
    if(!mesh_header(mesh,vertices,vp,cur)) return false;
    double sum[3]={0,0,0},n=0,all=0;
    for(unsigned guard=0;guard<64;++guard) {
        const uint32_t header=psx_mod_read_word(cur);cur+=4;
        const uint32_t type=header&0xffff,count=header>>16;
        if(type==0xff) break;
        unsigned size=0;
        switch(type) {case 0x20: case 0x28: size=8;break;case 0x30: case 0x38: size=12;break;case 0x24: case 0x2c: size=16;break;case 0x34: case 0x3c: size=20;break;}
        if(!size || !count || count>64 || cur>0x801fff00u-count*size) break;
        for(uint32_t k=0;k<count;++k,cur+=size) {
            if(type!=0x24 && type!=0x2c && type!=0x34 && type!=0x3c) continue;
            const uint32_t t1=psx_mod_read_word(cur+4),t2=psx_mod_read_word(cur+8),t3=psx_mod_read_word(cur+12);
            const unsigned nc=(type&8)?4:3;
            const uint32_t clut=t1>>16,tp=(t2>>16)&0x1ff,uv[4]={t1&0xffff,t2&0xffff,t3&0xffff,t3>>16};
            int u0=255,u1=0,v0=255,v1=0;
            for(unsigned i=0;i<nc;++i) {u0=std::min<int>(u0,uv[i]&0xff);u1=std::max<int>(u1,uv[i]&0xff);v0=std::min<int>(v0,uv[i]>>8);v1=std::max<int>(v1,uv[i]>>8);}
            if((u1-u0+1)*(v1-v0+1)>64*64) continue;
            const int tx=(tp&0xf)*64,ty=((tp>>4)&1)*256,depth=(tp>>7)&3,cx=(clut&0x3f)*16,cy=(clut>>6)&0x1ff;
            auto at=[](int x,int y){return gpu_vram_peek(x&1023,y&511);};
            for(int v=v0;v<=v1;++v) for(int u=u0;u<=u1;++u) {
                uint16_t c;
                if(depth==0) c=at(cx+((at(tx+u/4,ty+v)>>(4*(u&3)))&0xf),cy);
                else if(depth==1) c=at(cx+((at(tx+u/2,ty+v)>>(8*(u&1)))&0xff),cy);
                else c=at(tx+u,ty+v);
                const double r=(c&31)/31.0,g=((c>>5)&31)/31.0,b=((c>>10)&31)/31.0;
                all+=1;
                if(std::max({r,g,b})<0.4) continue;
                sum[0]+=r;sum[1]+=g;sum[2]+=b;n+=1;
            }
        }
    }
    const double m=std::max({sum[0],sum[1],sum[2]});
    if(lit) *lit=all>0?(float)(n/all):0.0f;
    if(n<8 || m<=0) return false;
    for(int c=0;c<3;++c) rgb[c]=(float)std::pow(sum[c]/m,1.5);
    return true;
}
bool cage_colour(uint32_t mesh,float rgb[3]) {return lamp_colour(mesh,26,rgb);}
// D57C: the subway entrance globe: a light at the globe's centre in its
// glass's colour (lamp_colour: warm yellow), reaching down the posts to the
// pavement and the stair head, and a soft halo over the globe. Developer:
// DNTTK_PROP_LIGHT_GLOBE="r g b strength radius" (r g b: the colour when the
// texture cannot be read), DNTTK_PROP_GLOBE_HALO="size level".
Tune globe_light{{1.0f,0.82f,0.40f},1.0f,3600};
float globe_halo_size=460,globe_halo_level=80;
constexpr float globe_at[3]={0,0,0};
struct GlobeSlot {std::atomic<uint32_t> mesh{0};std::atomic<float> rgb[3]{{1.0f},{0.82f},{0.40f}};std::atomic<bool> lit{false};};
GlobeSlot globe_slots[8];
const GlobeSlot* globe_slot(uint32_t mesh) {
    for(const GlobeSlot& g:globe_slots) if(g.mesh.load(std::memory_order_relaxed)==mesh) return &g;
    return nullptr;
}
// D57I: the street lamp post: high-pressure sodium, a 90s city street at
// night. HPS (Philips SON, colour 220: about 1900-2000 K, x 0.540 y 0.420,
// Ra about 20) lies outside sRGB (it clips to 1.0 0.25 0); the eye's night
// adaptation sees a warm orange-amber with a little pink (the usual
// rendering is 255 183 76), so the light sits between the two. A lamp 2820
// above the street lights it barely under the renderer's (1 - d/R)^2
// falloff, so the pool comes from a light lower down, out over the road (a
// cobra head throws its light down and out over the road, IES type II/III),
// and the lens itself carries a soft halo. Steady: no flicker. Developer:
// DNTTK_PROP_LIGHT_STREET="r g b strength radius", DNTTK_PROP_STREET_AT=
// "y z" (model height and reach of the pool light), DNTTK_PROP_STREET_HALO=
// "size level r g b".
// Tuned on the level 0 street (Relit reaches 1.35x further): a pool round
// each post with dark road between them, not an orange wash. The light
// stands out over the road beyond the lens (as a cobra head aims), so the
// post's own pole, 1540 away, takes less of it.
Tune street_light{{1.0f,0.60f,0.20f},1.7f,2400};
float street_at[3]={0,650,1000};   // 810 above the street
constexpr float street_lens_at[3]={0,-1335,523};
float street_halo_size=560,street_halo_level=100,street_halo_rgb[3]={1.0f,0.72f,0.38f};
// D57D: the shaded table lamp: a tungsten bulb (about 2700 K) behind a
// cream shade. One steady light at the bulb, inside the shade just above its
// open rim, so the table top, the floor round it and the walls nearby take a
// warm pool; a soft glow on the shade. Developer:
// DNTTK_PROP_LIGHT_TABLE_LAMP="r g b strength radius",
// DNTTK_PROP_TABLE_LAMP_AT="y" (model height of the bulb),
// DNTTK_PROP_TABLE_LAMP_HALO="size level r g b".
Tune table_lamp_light{{1.0f,0.78f,0.50f},1.2f,2600};
float table_lamp_at[3]={0,-40,0};
constexpr float table_shade_at[3]={0,-90,0};
float table_halo_size=520,table_halo_level=70,table_halo_rgb[3]={1.0f,0.90f,0.62f};
// D57K: the hanging ceiling lamp: a bare tungsten bulb under a shallow
// shade, hung low over the room. One steady light just under the bulb, so
// the floor and the furniture under it take the pool and the ceiling, far
// above, much less; a soft glow round the bulb and the shade's underside.
// Weaker than the table lamp: its room's painted light is already bright
// (strength 1.2 bleached the wall behind it; 0.5 reads as a warm pool).
// Developer: DNTTK_PROP_LIGHT_PENDANT="r g b strength radius",
// DNTTK_PROP_PENDANT_AT="y" (model height of the light),
// DNTTK_PROP_PENDANT_HALO="size level r g b".
Tune pendant_light{{1.0f,0.78f,0.50f},0.5f,2200};
float pendant_at[3]={0,520,0};
constexpr float pendant_bulb_at[3]={0,455,0};
float pendant_halo_size=300,pendant_halo_level=40,pendant_halo_rgb[3]={1.0f,0.90f,0.66f};
// D57M: the bare bulb: its light just under the glass, and a glow on the
// bulb (model y 181..302, its centre y 241). Outside an apartment room the
// shaded lamp's warm tungsten. Developer: DNTTK_PROP_LIGHT_BULB="r g b
// strength radius", DNTTK_PROP_BULB_AT="y", DNTTK_PROP_BULB_HALO="size level r g b".
Tune bulb_light{{1.0f,0.78f,0.50f},0.5f,2200};
float bulb_at[3]={0,420,0};   // just under the glass: a softer pool on the ceiling
constexpr float bulb_glass_at[3]={0,245,0};
float bulb_halo_size=300,bulb_halo_level=100,bulb_halo_rgb[3]={1.0f,0.90f,0.66f};
// D57H: the red candle on its iron stand: one small warm candle light just
// over the flame (a candle's yellow-orange, as the level files' candle
// lamps: 1.00 0.69 0.22), the gentle candle flicker of the chandeliers
// (prop_candle_level, about 0.93..1.0), and a soft warm halo round the
// flame. A flame, not a fixture: it does not follow a room's switch.
// Developer: DNTTK_PROP_LIGHT_CANDLE_STAND="r g b strength radius",
// DNTTK_PROP_CANDLE_STAND_AT="y", DNTTK_PROP_CANDLE_STAND_HALO="size level".
Tune candle_stand_light{{1.0f,0.70f,0.30f},0.7f,1400};   // 0.9 1500 tried first: the wall beside it went hot in Relit
float candle_stand_at[3]={0,-390,0};   // just over the flame's tip (y -365)
constexpr float candle_flame_at[3]={0,-352,0};
float candle_stand_halo_size=200,candle_stand_halo_level=70;
// D57B: the DeLorean, found: its headlights are on (a warm-white halogen
// pool on the cave floor ahead of the nose, which also lights the nose and
// the front of the body) and its rear vents glow orange (a low warm light
// behind the tail). Steady. The renderer's lights are points, so the
// headlight is a light out ahead of the nose, low over the floor.
// Developer: DNTTK_PROP_LIGHT_DELOREAN_HEAD / _VENT="r g b strength radius",
// DNTTK_PROP_DELOREAN_HEAD_AT / _VENT_AT="x y z" (model space),
// DNTTK_PROP_DELOREAN_HALO="head_size head_level vent_size vent_level".
// Tuned at the user's UI slot 9 (in the cave, looking at the car): the
// headlight 1800 out ahead lit only the rock the car faces, so it stands just
// before the nose at hood height and reaches the bonnet and front wings too;
// at strength 1.0 the cave lost its gloom (Relit mean 31 -> 57), at 0.6 it
// keeps it (31 -> 45) while the body reads.
Tune delorean_head{{1.0f,0.92f,0.78f},0.6f,2200};
Tune delorean_vent{{1.0f,0.48f,0.20f},0.55f,2000};
float delorean_head_at[3]={0,-40,-1350},delorean_vent_at[3]={0,-120,1300};
constexpr float delorean_lamp_at[2][3]={{-245,84,-1050},{245,84,-1050}},delorean_vents_at[3]={0,0,1090};
float delorean_head_halo[2]={260,90},delorean_vent_halo[2]={320,55};
// D57A: lava gives light. Time to Kill draws its lava as world faces (GOLD
// AND GUNS: the channel at the bottom of the long hall's pit, level 3 rooms
// 49-53, a plane at y 6784): a dark orange, flowing texture (mean 71 34 1)
// under vertex colours painted to a full red (about 255 120 120), which is
// what makes it glow; the walls round it take none of that light. Found by
// rule, game-wide, once a level (and after each load): a world face is lava
// when it is flat (every corner at one height), every corner's painted red
// is at least lava_paint_min and well over its blue, and its texture is
// fiery (red the strongest channel, blue under 15% of it, green 25-75% of
// it). Its faces are gathered into pools (lava_pool across); each pool
// gives one warm light lava_lift above its surface, with a slow, gentle
// throb (lava_level: about 0.85..1.0, 3.6 s, each pool on its own phase).
// The lights go with the prop lights (the nearest, rooms shown).
// Developer: DNTTK_LAVA_LIGHT="r g b strength radius", DNTTK_LAVA_AT="lift
// pool" (height over the surface, pool size), DNTTK_LAVA_THROB="depth seconds",
// DNTTK_LAVA_TRACE=1 (the scan's faces, textures and pools).
// Tuned in the GOLD AND GUNS pit (its walls 6800 tall): radius 3600 lit the
// bottom of the walls only, unseen from the hall's rim; 9000 warms the walls
// up to the rim and, faintly, the cave ceiling, as a pool of lava would.
Tune lava_light{{1.0f,0.42f,0.10f},1.4f,10000};   // user 2026-11-23: "turned up a notch" (was 1.0 / 9000)
float lava_lift=900,lava_pool=12000,lava_throb_depth=0.15f,lava_throb_s=3.6f;
// At most lava_max pools light at once, the nearest; the last of them fades
// as the next one comes as near (over lava_handoff), so walking the hall
// hands the light on without a pop. Each pool light is a shadowed prop light
// (about 2.7 points of GPU at 180 Hz): six cost 16 points in the GOLD AND
// GUNS hall, two cost 5 (developer: DNTTK_LAVA_MAX="n handoff").
unsigned lava_max=2;float lava_handoff=4000;
constexpr unsigned lava_paint_min=200;
float lava_level(uint32_t ms,uint32_t seed) {
    const float t=ms/1000.0f,ph=(float)(seed%997)*0.37f;
    const float slow=0.5f+0.5f*std::sin(2*(float)pi*t/lava_throb_s+ph);
    return 1.0f-lava_throb_depth*slow+0.03f*(prop_light_noise(t*0.9,seed)-0.5f);
}
// D57L (documentation/544-d57l-apartment-mood-lighting.md): lamps follow their
// room's switch, and the LA apartment's mood.
// - The game dims a room by its light colour (room record +0x54, 80 80 80 a
//   plain room): the apartment's switch takes it 60 -> 10. A room under
//   room_dark_below is switched off: its electric fixtures (the hanging,
//   caged, globe and street lamps) go out with it, and a table lamp (it
//   plugs in) stays on, turned low and so deeper amber (a dimmed tungsten
//   bulb warms: about 2700 K full, 1600 K at 5%). Flames are not switched.
//   Game-wide, the game's own state: any room a level dims follows.
// - A room holding the hanging lamp is an apartment room (levels 0 and 9,
//   room 8): its hanging lamp is a dim rose pool instead of a bright
//   downward light and its table lamp warmer (the user's choice of the
//   mock-ups: L4 lit, D3 switched off). club_lights.inc dims its painted
//   light in Relit and adds the street's glow at its window.
// "lamp apartment off" (session) is the look before D57L. Developer:
// DNTTK_APARTMENT_PENDANT / _TABLE / DNTTK_DARK_TABLE="r g b strength radius",
// DNTTK_APARTMENT_PENDANT_HALO="size level r g b", DNTTK_DARK_TABLE_HALO="level r g b".
constexpr unsigned room_dark_below=0x40;
Tune apartment_pendant{{1.0f,0.45f,0.42f},0.40f,1700};
float apartment_pendant_halo[5]={300,60,1.0f,0.90f,0.66f};   // a warm-white bulb glow that reads against the rose (user 2026-11-22: "bring the halo back when the light is on")
Tune apartment_table{{1.0f,0.60f,0.32f},0.9f,2300};
// D57M: the bare bulb in an apartment room (level 5 room 8, its one light):
// a low, warm dimmed-tungsten pool, dark at the walls. Developer
// DNTTK_APARTMENT_BULB="r g b strength radius", DNTTK_APARTMENT_BULB_HALO="size level r g b".
Tune apartment_bulb{{1.0f,0.58f,0.34f},0.45f,2000};
float apartment_bulb_halo[5]={300,140,1.0f,0.86f,0.58f};   // hot: Relit's low ambient greys the bulb's own texture
Tune dark_table{{1.0f,0.55f,0.22f},0.80f,2200};
float dark_table_halo[4]={45,1.0f,0.62f,0.30f};
// The look each lamp's halo takes, worked out by the draw itself (1 an
// apartment lamp, 2 switched off, 0 as before): the scan records each lamp's
// prop, model, room and whether it is an apartment room, and the draw reads
// the room's light colour from RAM as it draws. (A look published by the
// light pass went stale whenever that pass did not run - the pause and save
// screens, and the image after each rescan - and the old halo showed round a
// switched-off lamp; user, 2026-11-22.)
struct LookSlot {std::atomic<uint32_t> o{0},mesh{0},room{0};std::atomic<bool> apt{false};};
LookSlot look_slots[16];
unsigned room_light(uint32_t table,uint32_t rooms,uint32_t room);
int slot_look(const LookSlot& l) {
    if(!apartment_lights_enabled.load(std::memory_order_relaxed)) return 0;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    const bool ok=rooms && rooms<=256 && table>=0x80010000u && table<=0x80200000u-rooms*88 && !(table&3);
    if(ok && room_light(table,rooms,l.room.load(std::memory_order_relaxed))<room_dark_below) return 2;
    return l.apt.load(std::memory_order_relaxed)?1:0;
}
int look_of(uint32_t o,uint32_t mesh) {
    int found=-1;
    for(const LookSlot& l:look_slots) {
        if(l.mesh.load(std::memory_order_relaxed)!=mesh) continue;
        const int v=slot_look(l);
        if(o && l.o.load(std::memory_order_relaxed)==o) return v;
        // The prop not known: the model's look if every copy agrees.
        if(found<0) found=v;else if(found!=v) found=0;
    }
    return o?0:std::max(found,0);
}
// D41AA: the subway train. Its lights ride with it (placed from its object
// record each image) and follow the train overlay's state (+0x35, D44B: 0
// parked, 1 pulling away, 2 at speed, 3 the crash, 4 stopped):
// - both bumper clusters glow, and the tail's cyan roof beacon is lit;
// - pulling away, the nose's headlight strikes up (a stutter, then full) and
//   throws a strong warm light ahead down the tunnel;
// - at the impact (2 -> 3) an orange flash at the nose with the game's
//   fireball, sparks strobing low at the leading end while it skids, the
//   beacon dips, the headlight blows and sputters out;
// - wrecked (4, as a later save finds it): the smashed nose cluster holds a
//   little dimmer after its sputter; the tail cluster and the beacon stay lit.
// A train model under another type (a custom level) keeps the parked look.
// Developer: DNTTK_PROP_LIGHT_TRAIN_LAMPS / _BEAM / _BEACON / _FLASH /
// _SPARKS ("r g b strength radius"), DNTTK_PROP_TRAIN_BEAM_AT="x y z", DNTTK_PROP_TRAIN_HALO="lamp beacon beam
// flash sparks" (halo levels), DNTTK_PROP_TRAIN_TRACE=1 (state changes).
Tune train_lamps{{1.0f,0.45f,0.06f},0.7f,1300};     // a bumper cluster (amber, red, amber)
Tune train_beam{{1.0f,0.80f,0.50f},1.6f,8000};      // the headlight, well ahead of the nose (it lights the tunnel, not its own cab)
Tune train_beacon{{0.14f,0.76f,1.0f},1.2f,3000};    // the cyan roof beacon (the D41D1 sign lamp's colour); D41AB: its strength at shade 0
// D41AB: a prop light's style. duke: Duke Nukem 3D's random light (SE 4,
// duke_flicker.h): hitag, lit shade (+0..15 a flash), unlit shade; the light
// and its halo take the level together. steady: 1. Developer:
// DNTTK_PROP_TRAIN_BEACON_STYLE="duke H ON OFF" or "steady".
struct PropStyle {bool duke;int hitag,on,off;};
// The Duke tick the game frame was composed at (composition_hook, from the
// draw clock): the frame's lights and the halos of all its images read the
// same tick, so a light and its halo flash together.
std::atomic<uint32_t> duke_tick_frame{0};
float prop_style_level(const PropStyle& p) {
    return p.duke?duke_random_light_level_at(duke_tick_frame.load(std::memory_order_relaxed),p.hitag,p.on,p.off):1.0f;
}
void tune_style(const char* name,PropStyle& p) {
    const char* e=std::getenv(name);
    if(!e) return;
    if(!std::strncmp(e,"steady",6)) {p.duke=false;return;}
    int v[3]={p.hitag,p.on,p.off};
    if(std::sscanf(e,"duke %d %d %d",&v[0],&v[1],&v[2])>=0) {p={true,v[0]<0?0:v[0],v[1],v[2]};}
}
PropStyle train_beacon_style{true,0,duke_default_on_shade,duke_default_off_shade};   // the tutorial map's SE 4: lit 0, unlit 20
Tune train_flash{{1.0f,0.58f,0.22f},3.2f,9000};     // the impact
Tune train_sparks{{1.0f,0.78f,0.45f},2.0f,2800};    // skid sparks
// Model points (y down, the nose at +x).
constexpr float train_lead_at[3]={170,-30,350},train_tail_at[3]={-9440,-30,-24},train_beacon_at[3]={-9100,-1700,15};
float train_beam_at[3]={3200,-300,0};
// The impact and the sparks stand outside the car on the side it skids
// toward (train_side: +1 model +z, -1 model -z, from its own motion after
// the impact), so they light the flank and floor there.
constexpr float train_flash_at[3]={400,-650,1300},train_sparks_at[3]={-600,0,1000};
// The three lamps of each cluster (texels u 72, 80, 88 at v 186 on faces 0
// and 2), stood just off the face for the halos; amber, red, amber.
constexpr float train_lead_lamps[3][3]={{117,-27,179},{99,-27,359},{81,-27,533}};
constexpr float train_tail_lamps[3][3]={{-9386,-27,184},{-9386,-27,-24},{-9386,-27,-234}};
float train_halo_lamp=110,train_halo_beacon=70,train_halo_beam=90,train_halo_flash=130,train_halo_sparks=170;
// The train now (game thread writes; the halos read it on any thread).
std::atomic<uint32_t> train_mesh_now{0},train_state_now{0},train_start_ms{0},train_crash_ms{0};
std::atomic<float> train_side{1};
struct TrainLook {float lead,tail,beacon,beam,flash,sparks;};
// A light blowing: two lit ticks, then three in four for a second, fading.
float train_sputter(uint32_t t,uint32_t ms_len,uint32_t seed) {
    if(t>=ms_len) return 0;
    const uint32_t tick=t/34;uint32_t h=(tick+seed*0x9e3779b9u)*2654435761u;h^=h>>15;
    return (tick<2 || ((h>>8)&3)!=0) ? 1.0f-(float)t/ms_len : 0.0f;
}
TrainLook train_look(uint32_t ms) {
    TrainLook l{1,1,1,0,0,0};
    const uint32_t st=train_state_now.load(std::memory_order_relaxed);
    if(st==1 || st==2) {
        // Pulling away: the headlight strikes up, stutters, then holds with
        // the car's hum.
        const uint32_t t=ms-train_start_ms.load(std::memory_order_relaxed);
        l.beam=t<90?1.0f:t<220?0.0f:t<280?0.8f:t<420?0.1f:t<700?0.4f+0.6f*(t-420)/280.0f:
               0.94f+0.06f*prop_light_noise(t/1000.0*14,77);
    } else if(st==3 || st==4) {
        const uint32_t t=ms-train_crash_ms.load(std::memory_order_relaxed);
        const double s=t/1000.0;
        // The impact: white-hot for a moment, then a fire's flutter dying away.
        if(t<3200) l.flash=t<70?1.0f:std::exp(-(float)(t-70)/750.0f)*(0.7f+0.3f*prop_light_noise(s*22,91));
        // Skid sparks: a hard strobe while it slews, gone in under a second.
        if(t<950) l.sparks=(prop_light_noise(s*48,57)>0.42f?1.0f:0.12f)*(1.0f-t/950.0f);
        l.beam=train_sputter(t,1100,7);
        // The beacon dips at the blow.
        if(t<420) l.beacon=0.35f+0.65f*prop_light_noise(s*30,33)*(t/420.0f);
        // The nose cluster is smashed: it buzzes and now and then drops out.
        // The user read a buzzing wreck lamp as a flicker fault (UI slot 12,
        // 2026-11-22): after its sputter the nose cluster holds steady, a
        // little dimmer. DNTTK_PROP_TRAIN_BUZZ=1 brings the buzz back.
        static const bool buzz=std::getenv("DNTTK_PROP_TRAIN_BUZZ") && std::atoi(std::getenv("DNTTK_PROP_TRAIN_BUZZ"))!=0;
        const float drop=buzz?prop_light_noise(s*1.4,19):0.0f;
        l.lead=t<1100?train_sputter(t,1100,11)+0.0f:
               !buzz?0.85f:drop>0.70f?0.45f*(prop_light_noise(s*34,23)>0.5f?1.0f:0.0f):0.78f+0.12f*prop_light_noise(s*9,29);
    }
    // D41AB: the beacon flickers as Duke 3D's random lights, in every state.
    l.beacon*=prop_style_level(train_beacon_style);
    return l;
}
// D57G: the iron ring's five candles (one light at the ring's centre, among
// the flames) and its fill on the hall below.
Tune ring_light{{1.0f,0.80f,0.52f},0.9f,2400};
Tune ring_fill{{1.0f,0.74f,0.46f},0.6f,6000};
// D56: pickups that glow (documentation/442-d56-necro-atomic-lights.md),
// by object type (+0x2c) of a pickup object (kind +0x14 = 4), so every copy
// in every level, dropped or spawned, takes part. The atomic health (581):
// a red and blue atom in cyan orbits; a cool cyan-blue light, a slow throb.
Tune atomic_light{{0.42f,0.82f,1.30f},0.85f,1700};
float atomic_halo[2]={380,90};   // D57N: size, level (developer DNTTK_ATOMIC_HALO="size level")
// D57F: an energy crystal away from its first holder, when the level binds
// no lamp of its type (a custom level): the D41D1 sign scan's colours.
Tune crystal_red{{1.00f,0.09f,0.36f},1.0f,1791},crystal_blue{{0.26f,0.19f,1.00f},1.0f,1758},crystal_green{{0.31f,1.00f,0.25f},1.0f,1710};
struct PickupLight {uint16_t type;const Tune* tune;float lift;};
const PickupLight pickup_lights[]={{581,&atomic_light,260}};
void tune(const char* name,Tune& t) {
    if(const char* e=std::getenv(name)) std::sscanf(e,"%f %f %f %f %f",&t.rgb[0],&t.rgb[1],&t.rgb[2],&t.k,&t.radius);
}

// Candle flames and halos (developer: DNTTK_PROP_FLAMES="w h level halo
// halo_level candle_at candle_y").
float flame_w=40,flame_h=84,flame_rgb=175,halo_size=240,halo_rgb=80;
// D57G: the iron ring's flames are the model's own (a small tongue on each
// candle block); a soft halo round each (developer: DNTTK_PROP_RING_HALO=
// "size level y") reads as candlelight from the hall below.
float ring_halo_size=420,ring_halo_rgb=70,ring_halo_y=500;
// D57F: the machine's top. The game lights it nearly white over a dark room,
// so the whole metal read as a lamp; its colours are scaled to
// portal_top_tone, and the light comes from its centre between the claws,
// over the pad: a live light there (portal_light) and a soft glow at that
// point (developer: DNTTK_PROP_PORTAL_TOP="tone size level y").
float portal_top_tone=0.6f,portal_top_halo=520,portal_top_level=95,portal_top_y=260;
// Its light, in the pad lamp's colour (the D41D1 sign lamp of type 179),
// from the centre between the claws onto them and down to the pad
// (developer: DNTTK_PROP_LIGHT_PORTAL).
Tune portal_light{{0.35f,0.63f,1.00f},0.9f,1900};
float portal_breath(uint32_t ms) {return 0.925f+0.075f*std::sin((float)(ms/1000.0*2*pi/3.0));}
// Model points (y down; the stove's door faces model +z).
// D41Q: each arm plane maps texture u 96 at its outer corner to u 127 at the
// column; the candle is texel column u 100 (centre 100.5), its tip the top
// row (v 64, y -212). So a candle stands 26.5/31 of the arm's length out:
// radius 766 on the 896 arm (D41L had 795, one texel outward).
float candle_at_u=(127-100.5f)/31,candle_y=-212;
constexpr float fire_at[3]={0,1180,560};                 // before the grate, low
constexpr float candle_at[3]={0,-120,0},fill_at[3]={0,700,0};
// D57G: the ring's flames stand at model y 334..562 on its top (y 566).
float ring_at[3]={0,450,0},ring_fill_at[3]={0,1800,0};
constexpr float grate_x=150,grate_y0=1160,grate_y1=1305,grate_z=233;
// The chandelier's outer arm corners (bottom row): the candle directions.
constexpr int arm_corner[12]={0,4,6,8,10,12,14,16,18,20,22,24};

// ---- props and their lights (game thread) --------------------------------
// D54: ref is the prop; when it breaks (prop_state.h) the light sputters out
// from broke_ms (standing 0) and the scans no longer find it.
struct Emitter {double p[3];const Tune* tune;bool fire;uint32_t seed;uint32_t room;float vis=1;PropRef ref;int standing=1;uint32_t broke_ms=0;bool pickup=false;
               bool crystal=false;Tune own{};bool portal=false;bool cage=false;   // D41Z: steady, as its texture
               uint32_t blink_mesh=0;uint32_t lit_ms=0;   // D41Y: a barricade lamp: its model, when it last showed lit   // D57F: own = the crystal's light (its level lamp's look)
               float train_k=-1;   // D41AA: a train light's level this image (-1: not a train light)
               int lamp=0;bool apartment=false,dark=false;   // D57L: table lamp / hanging lamp kind; in an apartment room; its room switched off
               bool steady=false;   // D57B: no flicker, and not an electric fixture of its room
               bool lava=false;};   // D57A: a pool of lava (lava_level)
uint64_t scan_sig=0;   // the level the emitters were found on (prop_state.h)
std::vector<Emitter> emitters;
uint32_t scan_level=~0u,scan_frame=0,frames=0,vis_ms=0,scan_loads=~0u;
size_t stoves=0,chandeliers=0,rings=0,barrels=0,barricades=0,cages=0,cages_bound=0,globes=0,street_lamps=0,table_lamps=0,pendants=0,bulbs=0,candle_stands=0,deloreans=0;
// D41AA: the train (one a level; game thread): its record, its room (it
// relinks as it runs) and the overlay state last seen (~0u: none yet).
struct Train {uint32_t o=0,mesh=0,room=0,state=~0u,room_frame=0;int32_t crash_p[3]={0,0,0};bool crashed_here=false;};
Train train;
Emitter train_lights[6];   // lamps, tail, beacon, beam, flash, sparks (persistent: the room fade)
bool cages_pending=false;   // D41Z: the level's lamps not loaded at the scan: scan again soon
// D56: glowing pickups, rescanned every 10 frames (dropped and spawned ones
// appear within a sixth of a second); one taken goes dark at once.
std::vector<Emitter> pickups;
uint32_t pickup_frame=0;
// D57F: crystals away from their first holder, rescanned with the pickups.
std::vector<Emitter> crystals;
// D56: a pickup's slow throb, about 0.8..1.0 (1.4 s a breath).
float pickup_level(uint32_t ms,uint32_t seed) {
    return 0.90f+0.10f*std::sin((float)(ms/1000.0*2*pi/1.4+(seed&255)*0.1));
}
void place(double out[3],const double prop[3],double yaw,const float m[3]) {
    // Model to world: yaw +0x1c (4096 a turn); model -z goes to (-sin, -cos).
    const double ca=std::cos(yaw),sa=std::sin(yaw);
    out[0]=prop[0]+m[0]*ca+m[2]*sa;out[1]=prop[1]+m[1];out[2]=prop[2]-m[0]*sa+m[2]*ca;
}
// D57B rev 2: the DeLorean's stands (the boxes it rests on, a prop model
// whose top meets the car's underside, under its body) float above the cave
// floor. Each such model gets a copy whose bottom corners go down to the
// floor under the lowest of its copies (the others sink into the floor,
// unseen); the car keeps its height. The floor: the nearest flat world face
// of the room under the stand's centre. Developer: DNTTK_DELOREAN_STANDS=0
// keeps them as they are; DNTTK_PROP_LIGHT_TRACE=1 reports them.
bool stands_enabled() {const char* e=std::getenv("DNTTK_DELOREAN_STANDS");return !(e && e[0]=='0');}
// The height (y, down positive) of the nearest flat face of the room at or
// under y0 at (x, z); false when there is none.
bool floor_below(uint32_t room,double x,double z,double y0,double& out) {
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(room>=rooms) return false;
    const uint32_t a=table+88*room;
    const double org[3]={(double)(int32_t)psx_mod_read_word(a),(double)(int32_t)psx_mod_read_word(a+4),(double)(int32_t)psx_mod_read_word(a+8)};
    const unsigned n=psx_mod_read_byte(a+0x32);const uint32_t list=psx_mod_read_word(a+0x40);
    if(!n || list<0x80010000u || list>0x80200000u-n*4 || (list&3)) return false;
    bool found=false;
    for(unsigned i=0;i<n;++i) {
        const uint32_t piece=psx_mod_read_word(list+4*i);
        if(piece<0x80010000u || piece>0x801f0000u || (piece&3)) continue;
        const unsigned count=psx_mod_read_half(piece+0x14);
        if(!count || count>2048 || piece+0x1c+8*count>0x80200000u) continue;
        uint32_t cursor=piece+psx_mod_read_word(piece+0x18);
        if(cursor<0x80010000u || cursor>0x801ffff0u) continue;
        for(unsigned guard=0;guard<=256;++guard) {
            const uint32_t header=psx_mod_read_word(cursor);cursor+=4;
            const uint32_t type=header&0xffff,m=header>>16;
            if(type==0xff || (type!=0x60 && type!=0x61) || !m || m>4096 || cursor+m*8>0x80200000u) break;
            for(uint32_t k=0;k<m;++k,cursor+=8) {
                const uint32_t word=psx_mod_read_word(cursor);const unsigned nc=type==0x60?3u:4u;
                double c[4][3];bool ok=true,flat=true;
                for(unsigned j=0;j<nc && ok;++j) {
                    const unsigned idx=(word>>(8*j))&0xff;
                    if(idx>=count) {ok=false;break;}
                    const uint32_t w0=psx_mod_read_word(piece+0x1c+8*idx);
                    c[j][0]=org[0]+(int16_t)(((w0&0xff)<<10)&0xffff);c[j][1]=org[1]+(int16_t)((((w0>>8)&0xff)<<7)&0xffff);c[j][2]=org[2]+(int16_t)((((w0>>16)&0xff)<<10)&0xffff);
                    if(j) flat&=c[j][1]==c[0][1];
                }
                if(!ok || !flat || c[0][1]<y0-16 || (found && c[0][1]>=out)) continue;
                // Inside the face (xz)? A GT4 is the triangles 0 1 2 and 1 3 2.
                auto inside=[&](int p,int q,int r) {
                    const double d1=(x-c[q][0])*(c[p][2]-c[q][2])-(c[p][0]-c[q][0])*(z-c[q][2]);
                    const double d2=(x-c[r][0])*(c[q][2]-c[r][2])-(c[q][0]-c[r][0])*(z-c[r][2]);
                    const double d3=(x-c[p][0])*(c[r][2]-c[p][2])-(c[r][0]-c[p][0])*(z-c[p][2]);
                    return !((d1<0 || d2<0 || d3<0) && (d1>0 || d2>0 || d3>0));
                };
                if(inside(0,1,2) || (nc==4 && inside(1,3,2))) {out=c[0][1];found=true;}
            }
        }
    }
    return found;
}
void build_stands(uint32_t car,uint32_t room) {
    if(!prop_stand_block) return;
    const uint32_t level=psx_mod_read_word(0x800be570u);
    psx_mod_write_word(prop_stand_block,~0u);psx_mod_write_word(prop_stand_block+4,0);
    if(!stands_enabled()) return;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(room>=rooms) return;
    double cp[3];for(int k=0;k<3;++k) cp[k]=(int32_t)psx_mod_read_word(car+4+4*k);
    const double car_bottom=cp[1]+182;   // the car's floor line (model y 182)
    struct Need {uint32_t mesh;double lowest;int32_t hi;};   // per model: how far down its bottom must go
    std::vector<Need> needs;
    for(uint32_t head_at:{0x48u,0x4cu}) {
        uint32_t node=psx_mod_read_word(table+88*room+head_at);
        for(unsigned guard=0;node && guard<1024;++guard) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            if(o==car || o<0x80010000u || o>0x801fff00u || (o&3) || psx_mod_read_byte(o+0x14)!=1) continue;
            const uint32_t mesh=psx_mod_read_word(o+0x44);
            if(mesh<0x80010000u || mesh>0x801ffff0u || (mesh&3)) continue;
            const unsigned nv=psx_mod_read_byte(mesh+6);const uint32_t vp=psx_mod_read_word(mesh+0x10);
            if(!nv || nv>64 || vp<0x80010000u || vp>0x801fff00u) continue;
            int32_t lo=INT32_MAX,hi=INT32_MIN;
            for(unsigned v=0;v<nv;++v) {const int32_t y=(int16_t)psx_mod_read_half(vp+8*v+2);lo=std::min(lo,y);hi=std::max(hi,y);}
            double p[3];for(int k=0;k<3;++k) p[k]=(int32_t)psx_mod_read_word(o+4+4*k);
            const double dx=p[0]-cp[0],dz=p[2]-cp[2];
            if(dx*dx+dz*dz>1300.0*1300.0 || std::fabs(p[1]+lo-car_bottom)>64) continue;   // not under the car's body, or its top not at the car's underside
            double floor;
            if(!floor_below(room,p[0],p[2],p[1]+hi,floor)) continue;
            const double gap=floor-(p[1]+hi);
            if(std::getenv("DNTTK_PROP_LIGHT_TRACE")) std::fprintf(stderr,"[TTK prop light] DeLorean stand %08x model %08x at %.0f %.0f %.0f: bottom %.0f, floor %.0f (gap %.0f)\n",o,mesh,p[0],p[1],p[2],p[1]+hi,floor,gap);
            if(gap<=8 || gap>2000) continue;
            bool known=false;
            for(Need& nd:needs) if(nd.mesh==mesh) {nd.lowest=std::max(nd.lowest,gap);known=true;}
            if(!known) needs.push_back({mesh,gap,hi});
        }
    }
    uint32_t used=0x40,count=0;
    for(const Need& nd:needs) {
        if(count>=6) break;
        const unsigned nv=psx_mod_read_byte(nd.mesh+6);const uint32_t vp=psx_mod_read_word(nd.mesh+0x10);
        const uint32_t bytes=0x18+nv*8+nv*4+16;   // header, corners, the colour words after them
        if(used+bytes>prop_stand_block_size) break;
        const uint32_t copy=prop_stand_block+used,verts=copy+0x18;used+=(bytes+15)&~15u;
        for(uint32_t i=0;i<0x18;i+=4) psx_mod_write_word(copy+i,psx_mod_read_word(nd.mesh+i));
        psx_mod_write_word(copy+0x10,verts);   // the face list stays the model's own
        for(uint32_t i=0;i<nv*12+16;i+=4) psx_mod_write_word(verts+i,psx_mod_read_word(vp+i));
        const int32_t drop=nd.hi+(int32_t)std::lround(nd.lowest)+8;   // 8 into the floor: no seam
        for(unsigned v=0;v<nv;++v) if((int16_t)psx_mod_read_half(vp+8*v+2)==nd.hi) psx_mod_write_half(verts+8*v+2,(uint16_t)(int16_t)drop);
        psx_mod_write_word(prop_stand_block+8+8*count,nd.mesh);psx_mod_write_word(prop_stand_block+12+8*count,copy);++count;
        if(std::getenv("DNTTK_PROP_LIGHT_TRACE")) std::fprintf(stderr,"[TTK prop light] DeLorean stands: model %08x bottom %d -> %d (copy %08x)\n",nd.mesh,nd.hi,drop,copy);
    }
    psx_mod_write_word(prop_stand_block+4,count);psx_mod_write_word(prop_stand_block,level);
}
uint32_t stand_swap(uint32_t mesh) {
    if(!prop_stand_block || !input_modernized() || ttk_world_classic()) return 0;
    if(psx_mod_read_word(prop_stand_block)!=psx_mod_read_word(0x800be570u)) return 0;
    const uint32_t n=std::min<uint32_t>(psx_mod_read_word(prop_stand_block+4),6);
    for(uint32_t i=0;i<n;++i) if(psx_mod_read_word(prop_stand_block+8+8*i)==mesh) {
        const uint32_t copy=psx_mod_read_word(prop_stand_block+12+8*i);
        return prop_stand_owns(copy)?copy:0;
    }
    return 0;
}
void scan_props() {
    uint32_t car=0,car_room=0;   // D57B rev 2
    emitters.clear();stoves=chandeliers=rings=barrels=barricades=cages=cages_bound=globes=street_lamps=table_lamps=pendants=bulbs=candle_stands=deloreans=0;scan_sig=0;cages_pending=false;
    const uint32_t train_was=train.o;train.o=0;
    unsigned cage_models=0,globe_models=0;
    for(BlinkSlot& b:blink_slots) {b.mesh.store(0,std::memory_order_relaxed);b.level.store(0,std::memory_order_relaxed);}
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return;
    std::vector<uint32_t> seen;
    for(uint32_t r=0;r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
        uint32_t node=psx_mod_read_word(table+88*r+head_at);
        for(unsigned guard=0;node && guard<1024;++guard) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            if(o<0x80010000u || o>0x801fff00u || (o&3)) continue;
            if(std::find(seen.begin(),seen.end(),o)!=seen.end()) continue;
            seen.push_back(o);
            if(psx_mod_read_byte(o+0x14)!=1 || prop_broken_flag(o)) continue;   // D54: broken props give no light
            const uint32_t mesh=psx_mod_read_word(o+0x44);
            // D41X: the fire barrel has no drawn extras (its fire is the
            // model's own), so it is found here only, not by kind_of on
            // every draw.
            const bool barrel=barrel_mesh(mesh);
            const int kind=barrel?prop_light_none:kind_of(mesh);
            if(!kind && !barrel) continue;
            if(prop_light_club_kind(kind)) continue;   // D57J: their lights are club lamps (club_lights.inc)
            double p[3];for(int k=0;k<3;++k) p[k]=(int32_t)psx_mod_read_word(o+4+4*k);
            const double yaw=(int16_t)psx_mod_read_half(o+0x1c)*(2*pi/4096);
            Emitter e{};e.room=r;e.seed=o;e.ref=prop_ref(o);
            if(kind==prop_light_train) {
                // D41AA: its lights follow it each image (train_update, publish).
                if(train_was!=o || !train.mesh) train.state=~0u;
                train.o=o;train.mesh=mesh;train.room=r;
            } else if(barrel) {
                place(e.p,p,yaw,barrel_at);e.tune=&barrel_light;e.fire=true;emitters.push_back(e);++barrels;
            } else if(kind==prop_light_barricade) {
                // D41Y: the light follows the lamp's frames (blink_update).
                place(e.p,p,yaw,barricade_lamp_at);e.tune=&e.own;e.own=barricade_light;e.blink_mesh=mesh;blink_slot(mesh,true);
                emitters.push_back(e);++barricades;
            } else if(kind==prop_light_cage) {
                // D41Z: a lamp the level's lamp file binds to this prop (a
                // D41D1 sign lamp: level 5's red ones) keeps that lamp alone.
                // The model's colour first: the halo uses it either way.
                CageSlot* slot=nullptr;
                for(unsigned i=0;i<cage_models && !slot;++i) if(cage_slots[i].mesh.load(std::memory_order_relaxed)==mesh) slot=&cage_slots[i];
                if(!slot && cage_models<16) {
                    slot=&cage_slots[cage_models++];
                    float rgb[3]={cage_light.rgb[0],cage_light.rgb[1],cage_light.rgb[2]};
                    cage_colour(mesh,rgb);
                    for(int c=0;c<3;++c) slot->rgb[c].store(rgb[c],std::memory_order_relaxed);
                    slot->mesh.store(mesh,std::memory_order_relaxed);
                }
                const int32_t at[3]={(int32_t)p[0],(int32_t)p[1],(int32_t)p[2]};
                const int bound=level_lamp_bound ? level_lamp_bound((uint16_t)psx_mod_read_half(o+0x2c),at,nullptr) : 0;
                if(bound<0) {cages_pending=true;continue;}
                if(bound==2) {++cages_bound;continue;}
                place(e.p,p,yaw,cage_at);e.own=cage_light;e.tune=&e.own;e.cage=true;
                if(slot) for(int c=0;c<3;++c) e.own.rgb[c]=slot->rgb[c].load(std::memory_order_relaxed);
                emitters.push_back(e);++cages;
            } else if(kind==prop_light_globe) {
                // D57C: the colour and the lit check, once per model.
                GlobeSlot* slot=nullptr;
                for(unsigned i=0;i<globe_models && !slot;++i) if(globe_slots[i].mesh.load(std::memory_order_relaxed)==mesh) slot=&globe_slots[i];
                if(!slot && globe_models<8) {
                    slot=&globe_slots[globe_models++];
                    float rgb[3]={globe_light.rgb[0],globe_light.rgb[1],globe_light.rgb[2]},lit=0;
                    const bool read=lamp_colour(mesh,25,rgb,&lit);
                    for(int c=0;c<3;++c) slot->rgb[c].store(rgb[c],std::memory_order_relaxed);
                    slot->lit.store(read && lit>=0.4f,std::memory_order_relaxed);
                    slot->mesh.store(mesh,std::memory_order_relaxed);
                    if(std::getenv("DNTTK_PROP_LIGHT_TRACE"))
                        std::fprintf(stderr,"[TTK prop light] globe model %08x colour %.2f %.2f %.2f lit %.2f (read %d)\n",mesh,rgb[0],rgb[1],rgb[2],lit,(int)read);
                }
                if(!slot || !slot->lit.load(std::memory_order_relaxed)) continue;
                const int32_t at[3]={(int32_t)p[0],(int32_t)p[1],(int32_t)p[2]};
                const int bound=level_lamp_bound ? level_lamp_bound((uint16_t)psx_mod_read_half(o+0x2c),at,nullptr) : 0;
                if(bound<0) {cages_pending=true;continue;}
                if(bound==2) continue;   // the level's lamp file lights it
                place(e.p,p,yaw,globe_at);e.own=globe_light;e.tune=&e.own;e.cage=true;   // steady, as the caged lamps
                for(int c=0;c<3;++c) e.own.rgb[c]=slot->rgb[c].load(std::memory_order_relaxed);
                emitters.push_back(e);++globes;
            } else if(kind==prop_light_street) {
                // D57I: a lamp the level file binds to the post keeps that lamp alone.
                const int32_t at[3]={(int32_t)p[0],(int32_t)p[1],(int32_t)p[2]};
                const int bound=level_lamp_bound ? level_lamp_bound((uint16_t)psx_mod_read_half(o+0x2c),at,nullptr) : 0;
                if(bound<0) {cages_pending=true;continue;}
                if(bound==2) continue;
                place(e.p,p,yaw,street_at);e.own=street_light;e.tune=&e.own;e.cage=true;   // steady
                emitters.push_back(e);++street_lamps;
            } else if(kind==prop_light_table_lamp) {
                // D57D: a lamp the level file binds to it keeps that lamp alone.
                const int32_t at[3]={(int32_t)p[0],(int32_t)p[1],(int32_t)p[2]};
                const int bound=level_lamp_bound ? level_lamp_bound((uint16_t)psx_mod_read_half(o+0x2c),at,nullptr) : 0;
                if(bound<0) {cages_pending=true;continue;}
                if(bound==2) continue;
                place(e.p,p,yaw,table_lamp_at);e.own=table_lamp_light;e.tune=&e.own;e.cage=true;   // steady
                e.lamp=kind;emitters.push_back(e);++table_lamps;
            } else if(kind==prop_light_pendant) {
                // D57K: as the table lamp: a lamp the level file binds keeps that lamp alone.
                const int32_t at[3]={(int32_t)p[0],(int32_t)p[1],(int32_t)p[2]};
                const int bound=level_lamp_bound ? level_lamp_bound((uint16_t)psx_mod_read_half(o+0x2c),at,nullptr) : 0;
                if(bound<0) {cages_pending=true;continue;}
                if(bound==2) continue;
                place(e.p,p,yaw,pendant_at);e.own=pendant_light;e.tune=&e.own;e.cage=true;   // steady
                e.lamp=kind;emitters.push_back(e);++pendants;
            } else if(kind==prop_light_bulb) {
                // D57M: as the hanging lamp.
                const int32_t at[3]={(int32_t)p[0],(int32_t)p[1],(int32_t)p[2]};
                const int bound=level_lamp_bound ? level_lamp_bound((uint16_t)psx_mod_read_half(o+0x2c),at,nullptr) : 0;
                if(bound<0) {cages_pending=true;continue;}
                if(bound==2) continue;
                place(e.p,p,yaw,bulb_at);e.own=bulb_light;e.tune=&e.own;e.cage=true;   // steady
                e.lamp=kind;emitters.push_back(e);++bulbs;
            } else if(kind==prop_light_candle_stand) {
                // D57H: a flame: the candle flicker (not cage: not steady, not switched).
                place(e.p,p,yaw,candle_stand_at);e.tune=&candle_stand_light;emitters.push_back(e);++candle_stands;
            } else if(kind==prop_light_delorean) {
                // D57B: the headlights and the vents.
                if(!car) {car=o;car_room=r;}
                place(e.p,p,yaw,delorean_head_at);e.tune=&delorean_head;e.steady=true;emitters.push_back(e);
                place(e.p,p,yaw,delorean_vent_at);e.tune=&delorean_vent;e.seed=o+1;emitters.push_back(e);++deloreans;
            } else if(kind==prop_light_portal_top) {
                // D57F: the light is the centre between the claws, not the metal.
                const float at[3]={0,portal_top_y,0};
                place(e.p,p,yaw,at);e.tune=&portal_light;e.portal=true;emitters.push_back(e);
            } else if(kind==prop_light_stove) {
                place(e.p,p,yaw,fire_at);e.tune=&fire_light;e.fire=true;emitters.push_back(e);++stoves;
            } else if(kind==prop_light_ring) {
                place(e.p,p,yaw,ring_at);e.tune=&ring_light;emitters.push_back(e);
                place(e.p,p,yaw,ring_fill_at);e.tune=&ring_fill;e.seed=o+1;emitters.push_back(e);++rings;
            } else {
                place(e.p,p,yaw,candle_at);e.tune=&candle_light;emitters.push_back(e);
                place(e.p,p,yaw,fill_at);e.tune=&fill_light;e.seed=o+1;emitters.push_back(e);++chandeliers;
            }
        }
    }
    if(prop_stand_block) {
        if(car) build_stands(car,car_room);
        else if(psx_mod_read_word(prop_stand_block)!=~0u) psx_mod_write_word(prop_stand_block,~0u);
    }
    for(unsigned i=cage_models;i<16;++i) cage_slots[i].mesh.store(0,std::memory_order_relaxed);
    for(unsigned i=globe_models;i<8;++i) globe_slots[i].mesh.store(0,std::memory_order_relaxed);
    if(!train.o) {train.mesh=0;train.state=~0u;train_mesh_now.store(0,std::memory_order_relaxed);}
    if(std::getenv("DNTTK_PROP_LIGHT_TRACE"))
        std::fprintf(stderr,"[TTK prop light] level %u: %zu stoves, %zu chandeliers, %zu rings, %zu fire barrels, %zu barricades, %zu caged lamps (%zu lit by the level file), %zu globes, %zu street lamps, %zu table lamps, %zu hanging lamps, %zu bare bulbs, %zu candle stands, %zu DeLoreans, %zu lights\n",scan_level,stoves,chandeliers,rings,barrels,barricades,cages,cages_bound,globes,street_lamps,table_lamps,pendants,bulbs,candle_stands,deloreans,emitters.size());
    if(std::getenv("DNTTK_PROP_LIGHT_TRACE")) for(const CageSlot& c:cage_slots) if(const uint32_t m=c.mesh.load(std::memory_order_relaxed))
        std::fprintf(stderr,"[TTK prop light] caged lamp model %08x colour %.2f %.2f %.2f\n",m,c.rgb[0].load(),c.rgb[1].load(),c.rgb[2].load());
    for(Emitter& e:emitters) if(e.blink_mesh || e.cage) e.tune=&e.own;   // after the moves
    // D57L: apartment rooms (holding a hanging lamp, D57M: or a bare bulb) and the halo slots.
    std::vector<uint32_t> apt;
    for(const Emitter& e:emitters) if(prop_light_hanging(e.lamp)) apt.push_back(e.room);
    unsigned slots=0;
    for(Emitter& e:emitters) {
        e.apartment=e.lamp && std::find(apt.begin(),apt.end(),e.room)!=apt.end();
        if(e.lamp && slots<16) {
            LookSlot& l=look_slots[slots++];
            l.o.store(e.seed,std::memory_order_relaxed);l.room.store(e.room,std::memory_order_relaxed);
            l.apt.store(e.apartment,std::memory_order_relaxed);l.mesh.store(psx_mod_read_word(e.seed+0x44),std::memory_order_relaxed);
        }
    }
    for(unsigned i=slots;i<16;++i) look_slots[i].mesh.store(0,std::memory_order_relaxed);
}
// D57L: the room's light colour (+0x54, its red byte; 0x80 a plain room).
unsigned room_light(uint32_t table,uint32_t rooms,uint32_t room) {
    if(room>=rooms) return 0x80;
    return psx_mod_read_byte(table+88*room+0x54);
}
// D57L: the light a lamp gives now (its room's switch, the apartment's look).
const Tune* tune_now(const Emitter& e) {
    if(!e.lamp || !apartment_lights_enabled.load(std::memory_order_relaxed)) return e.tune;
    if(e.dark) return e.lamp==prop_light_table_lamp?&dark_table:nullptr;   // the hanging lamp is out
    if(e.apartment) return e.lamp==prop_light_pendant?&apartment_pendant:e.lamp==prop_light_bulb?&apartment_bulb:&apartment_table;
    return e.tune;
}
// D41Y: each frame, every barricade model's lamp level from the frame the
// game shows (D55 signal reading: 1 lit, 0 dark), for the halos and lights.
void blink_update() {
    if(!prop_signal_now) return;
    for(BlinkSlot& b:blink_slots) {
        const uint32_t mesh=b.mesh.load(std::memory_order_relaxed);
        if(!mesh) continue;
        float level=1,rgb[3]={1.0f,0.77f,0.0f};
        if(!prop_signal_now(mesh,level,rgb)) level=1;   // no animation readable: steady
        b.level.store(level,std::memory_order_relaxed);
        if(level>0.5f) for(int c=0;c<3;++c) b.rgb[c].store(rgb[c],std::memory_order_relaxed);
    }
    if(static const bool trace=std::getenv("DNTTK_BLINK_TRACE")!=nullptr;trace)
        for(const BlinkSlot& b:blink_slots) if(const uint32_t m=b.mesh.load(std::memory_order_relaxed))
            std::fprintf(stderr,"[TTK prop light] frame %u barricade %08x lamp %.2f\n",frames,m,b.level.load(std::memory_order_relaxed));
}
// D57A: the level's lava pools (game thread; on a level change or a load,
// again while the level's textures are not yet in VRAM).
std::vector<Emitter> lava;
uint32_t lava_level_seen=~0u,lava_loads_seen=~0u,lava_frame=0;unsigned lava_tries=0;bool lava_pending=false;
// The mean colour (0..1) of a world texture entry's texels (its UV box,
// through its CLUT); false when nothing readable is there (not loaded yet).
// It reads the level's VRAM snapshot that the neon scan (or, in LIGHTING
// ORIGINAL, D57F's share) already takes a moment after each load, delivered
// without a GPU wait (near_clip_vram): reading VRAM here (gpu_vram_peek, or
// one gpu_vram_copy) stalled the game thread 24 ms on the GL readback.
std::shared_ptr<const std::vector<uint16_t>> lava_vram;
bool texture_mean(uint32_t t1,uint32_t t2,uint32_t t3,double out[3]) {
    if(!lava_vram || lava_vram->size()<1024*512) return false;
    const uint32_t clut=t1>>16,tp=(t2>>16)&0x1ff,uv[4]={t1&0xffff,t2&0xffff,t3&0xffff,t3>>16};
    int u0=255,u1=0,v0=255,v1=0;
    for(uint32_t q:uv) {u0=std::min<int>(u0,q&0xff);u1=std::max<int>(u1,q&0xff);v0=std::min<int>(v0,q>>8);v1=std::max<int>(v1,q>>8);}
    const int tx=(tp&0xf)*64,ty=((tp>>4)&1)*256,depth=(tp>>7)&3,cx=(clut&0x3f)*16,cy=(clut>>6)&0x1ff;
    const std::vector<uint16_t>& vram=*lava_vram;
    auto at=[&vram](int x,int y){return vram[(size_t)(y&511)*1024+(x&1023)];};
    double sum[3]={0,0,0};unsigned n=0,lit=0;
    for(int v=v0;v<=v1;v+=2) for(int u=u0;u<=u1;u+=2) {   // every other texel: plenty for a mean
        uint16_t c;
        if(depth==0) c=at(cx+((at(tx+u/4,ty+v)>>(4*(u&3)))&0xf),cy);
        else if(depth==1) c=at(cx+((at(tx+u/2,ty+v)>>(8*(u&1)))&0xff),cy);
        else c=at(tx+u,ty+v);
        sum[0]+=(c&31)/31.0;sum[1]+=((c>>5)&31)/31.0;sum[2]+=((c>>10)&31)/31.0;++n;lit+=(c&0x7fff)!=0;
    }
    if(!n || lit*4<n) return false;
    for(int k=0;k<3;++k) out[k]=sum[k]/n;
    return true;
}
bool fiery(const double m[3]) {
    return m[0]>=0.15 && m[0]>=m[1] && m[0]>=m[2] && m[2]<=0.15*m[0] && m[1]>=0.25*m[0] && m[1]<=0.75*m[0];
}
// D57N: the baked-glow walls laid upside down (see prop_lights.h). Every
// wall face (not flat) is grouped by its texture's page and palette; a
// group whose texture's painted brightness runs strongly along v (the mean
// of its brighter quarter of rows at least 1.6 times its darker quarter, its
// bright end fiery: red the strongest, blue at most 35% of it) and
// whose faces mostly (70%+, 12 or more) run one way in world height has its
// other faces flipped. Uses the level's VRAM snapshot (lava_vram).
void scan_glow_flips(uint32_t level) {
    if(!glow_flip_block) return;
    psx_mod_write_word(glow_flip_block,~0u);psx_mod_write_word(glow_flip_block+4,0);
    if(const char* e=std::getenv("DNTTK_GLOW_FLIP");e && e[0]=='0') return;
    if(!lava_vram) return;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c),textures=psx_mod_read_word(0x800d67a8u+0x58);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3) || textures<0x80010000u || textures>0x801ffff0u) return;
    const std::vector<uint16_t>& vram=*lava_vram;
    auto at=[&vram](int x,int y){return vram[(size_t)(y&511)*1024+(x&1023)];};
    struct Face {uint32_t record;uint32_t key;int dir;};   // key: page << 16 | clut; dir: +1 v grows downward
    std::vector<Face> faces;
    std::vector<std::pair<uint32_t,float>> gradient;   // per key: brighter quarter / darker quarter (0: not read, or not a fiery glow)
    std::vector<std::array<float,3>> glow_rgb;          // per key: the brighter quarter's colour
    for(uint32_t r=0;r<rooms;++r) {
        const uint32_t a=table+88*r;
        const double org1=(double)(int32_t)psx_mod_read_word(a+4);
        const unsigned n=psx_mod_read_byte(a+0x32);const uint32_t list=psx_mod_read_word(a+0x40);
        if(!n || list<0x80010000u || list>0x80200000u-n*4 || (list&3)) continue;
        for(unsigned i=0;i<n;++i) {
            const uint32_t piece=psx_mod_read_word(list+4*i);
            if(piece<0x80010000u || piece>0x801f0000u || (piece&3)) continue;
            const unsigned count=psx_mod_read_half(piece+0x14);
            if(!count || count>2048 || piece+0x1c+8*count>0x80200000u) continue;
            uint32_t cursor=piece+psx_mod_read_word(piece+0x18);
            if(cursor<0x80010000u || cursor>0x801ffff0u) continue;
            for(unsigned guard=0;guard<=256;++guard) {
                const uint32_t header=psx_mod_read_word(cursor);cursor+=4;
                const uint32_t type=header&0xffff,m=header>>16;
                if(type==0xff || (type!=0x60 && type!=0x61) || !m || m>4096 || cursor+m*8>0x80200000u) break;
                for(uint32_t k=0;k<m;++k,cursor+=8) {
                    const uint32_t word=psx_mod_read_word(cursor);const unsigned nc=type==0x60?3u:4u;
                    const int16_t tex=(int16_t)psx_mod_read_half(cursor+4);
                    const uint32_t e=textures+(uint32_t)(tex*12);
                    const uint32_t t1=psx_mod_read_word(e),t2=psx_mod_read_word(e+4),t3=psx_mod_read_word(e+8);
                    if((t2>>16)&0x200) continue;   // animated
                    const uint32_t uv[4]={t1&0xffff,t2&0xffff,t3&0xffff,t3>>16};
                    double y[4];bool ok=true;double ymin=1e30,ymax=-1e30;
                    for(unsigned j=0;j<nc;++j) {
                        const unsigned idx=(word>>(8*j))&0xff;
                        if(idx>=count) {ok=false;break;}
                        const uint32_t w0=psx_mod_read_word(piece+0x1c+8*idx);
                        y[j]=org1+(int16_t)((((w0>>8)&0xff)<<7)&0xffff);ymin=std::min(ymin,y[j]);ymax=std::max(ymax,y[j]);
                    }
                    if(!ok || ymax-ymin<256) continue;   // a wall: at least 256 tall
                    double vt=0,vb=0;unsigned nt=0,nb=0;
                    for(unsigned j=0;j<nc;++j) {if(y[j]==ymin) {vt+=uv[j]>>8;++nt;} if(y[j]==ymax) {vb+=uv[j]>>8;++nb;}}
                    if(!nt || !nb || vt/nt==vb/nb) continue;
                    const uint32_t key=(t2>>16&0x1ff)<<16|(t1>>16);
                    // The texture's painted gradient along v, once per key.
                    bool known=false;
                    for(const auto& g:gradient) if(g.first==key) {known=true;break;}
                    if(!known) {
                        int v0=255,v1=0,u0=255,u1=0;
                        for(unsigned j=0;j<nc;++j) {u0=std::min<int>(u0,uv[j]&0xff);u1=std::max<int>(u1,uv[j]&0xff);v0=std::min<int>(v0,uv[j]>>8);v1=std::max<int>(v1,uv[j]>>8);}
                        const uint32_t tp=(t2>>16)&0x1ff,clut=t1>>16;
                        const int tx=(tp&0xf)*64,ty=((tp>>4)&1)*256,depth=(tp>>7)&3,cx=(clut&0x3f)*16,cy=(clut>>6)&0x1ff;
                        const int rows=v1-v0+1,q=std::max(1,rows/4);
                        double band[2]={0,0},rgb[2][3]={{0,0,0},{0,0,0}};unsigned bn[2]={0,0};
                        for(int v=v0;v<=v1;++v) {
                            const int which=v<v0+q?0:v>v1-q?1:-1;
                            if(which<0) continue;
                            for(int u=u0;u<=u1;u+=2) {
                                uint16_t c;
                                if(depth==0) c=at(cx+((at(tx+u/4,ty+v)>>(4*(u&3)))&0xf),cy);
                                else if(depth==1) c=at(cx+((at(tx+u/2,ty+v)>>(8*(u&1)))&0xff),cy);
                                else c=at(tx+u,ty+v);
                                band[which]+=(c&31)+((c>>5)&31)+((c>>10)&31);++bn[which];
                                rgb[which][0]+=c&31;rgb[which][1]+=(c>>5)&31;rgb[which][2]+=(c>>10)&31;
                            }
                        }
                        float ratio=0;std::array<float,3> hot{0,0,0};
                        if(bn[0] && bn[1]) {
                            const double a0=band[0]/bn[0],a1=band[1]/bn[1];ratio=(float)(std::max(a0,a1)/std::max(1.0,std::min(a0,a1)));
                            const int b=a1>a0?1:0;for(int c=0;c<3;++c) hot[c]=(float)(rgb[b][c]/bn[b]/31.0);
                            // A glow: its bright end fiery (red the strongest, little blue).
                            if(!(hot[0]>=hot[1] && hot[2]<=0.35f*hot[0] && hot[1]<=0.8f*hot[0])) ratio=-ratio;
                        }
                        gradient.push_back({key,ratio});glow_rgb.push_back(hot);
                    }
                    faces.push_back({cursor,key,vb/nb>vt/nt?1:-1});
                }
            }
        }
    }
    std::vector<uint32_t> flips;unsigned groups=0;
    for(size_t gi=0;gi<gradient.size();++gi) {
        const auto& g=gradient[gi];
        if(std::getenv("DNTTK_LAVA_TRACE") && std::fabs(g.second)>=1.6f)
            std::fprintf(stderr,"[TTK glow flip] level %u texture page %x palette %x: gradient %.1f, bright end %.2f %.2f %.2f%s\n",level,g.first>>16,g.first&0xffff,std::fabs(g.second),
                         glow_rgb[gi][0],glow_rgb[gi][1],glow_rgb[gi][2],g.second<0?" (not fiery: not a glow)":"");
        if(g.second<1.6f) continue;
        unsigned down=0,up=0;
        for(const Face& f:faces) if(f.key==g.first) (f.dir>0?down:up)++;
        const unsigned total=down+up,major=std::max(down,up);
        if(total<12 || major<0.7*total || major==total) continue;   // 12+: a texture used widely (level 5's 7-face group is too thin to judge)
        const int odd=down>=up?-1:1;
        for(const Face& f:faces) if(f.key==g.first && f.dir==odd) flips.push_back(f.record);
        ++groups;
        if(std::getenv("DNTTK_PROP_LIGHT_TRACE") || std::getenv("DNTTK_LAVA_TRACE"))
            std::fprintf(stderr,"[TTK glow flip] level %u texture page %x palette %x: gradient %.1f, %u faces one way, %u the other: those flipped\n",level,g.first>>16,g.first&0xffff,g.second,major,total-major);
    }
    std::sort(flips.begin(),flips.end());flips.erase(std::unique(flips.begin(),flips.end()),flips.end());
    if(flips.size()>glow_flip_max) flips.resize(glow_flip_max);
    for(size_t i=0;i<flips.size();++i) psx_mod_write_word(glow_flip_block+8+4*(uint32_t)i,flips[i]);
    psx_mod_write_word(glow_flip_block+4,(uint32_t)flips.size());psx_mod_write_word(glow_flip_block,level);
    if(std::getenv("DNTTK_PROP_LIGHT_TRACE") || std::getenv("DNTTK_LAVA_TRACE"))
        std::fprintf(stderr,"[TTK glow flip] level %u: %zu wall faces of %u glow textures flipped upright\n",level,flips.size(),groups);
}
void scan_lava() {
    const auto t0=std::chrono::steady_clock::now();
    lava.clear();lava_pending=false;
    lava_vram=near_clip_vram(lava_level_seen);
    if(!lava_vram) {lava_pending=true;return;}   // the level's snapshot comes a moment after the load
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    const uint32_t textures=psx_mod_read_word(0x800d67a8u+0x58);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3) || textures<0x80010000u || textures>0x801ffff0u) {lava_pending=true;return;}
    struct Face {double c[3];uint32_t room;double area;};
    std::vector<Face> faces;
    std::vector<std::pair<int,int>> verdict;   // texture index, 1 fiery / 0 not / -1 unreadable
    unsigned candidates=0,unreadable=0;double tex_ms=0;
    for(uint32_t r=0;r<rooms;++r) {
        const uint32_t a=table+88*r;
        const double org[3]={(double)(int32_t)psx_mod_read_word(a),(double)(int32_t)psx_mod_read_word(a+4),(double)(int32_t)psx_mod_read_word(a+8)};
        const unsigned n=psx_mod_read_byte(a+0x32);const uint32_t list=psx_mod_read_word(a+0x40);
        if(!n || list<0x80010000u || list>0x80200000u-n*4 || (list&3)) continue;
        for(unsigned i=0;i<n;++i) {
            const uint32_t piece=psx_mod_read_word(list+4*i);
            if(piece<0x80010000u || piece>0x801f0000u || (piece&3)) continue;
            const unsigned count=psx_mod_read_half(piece+0x14);
            if(!count || count>2048 || piece+0x1c+8*count>0x80200000u) continue;
            uint32_t cursor=piece+psx_mod_read_word(piece+0x18);
            if(cursor<0x80010000u || cursor>0x801ffff0u) continue;
            for(unsigned guard=0;guard<=256;++guard) {
                const uint32_t header=psx_mod_read_word(cursor);cursor+=4;
                const uint32_t type=header&0xffff,m=header>>16;
                if(type==0xff || (type!=0x60 && type!=0x61) || !m || m>4096 || cursor+m*8>0x80200000u) break;
                for(uint32_t k=0;k<m;++k,cursor+=8) {
                    const uint32_t word=psx_mod_read_word(cursor);const unsigned nc=type==0x60?3u:4u;
                    double c[3]={0,0,0},y0=0,lo[2]={1e30,1e30},hi[2]={-1e30,-1e30};bool ok=true;
                    for(unsigned j=0;j<nc && ok;++j) {
                        const unsigned idx=(word>>(8*j))&0xff;
                        if(idx>=count) {ok=false;break;}
                        const uint32_t w0=psx_mod_read_word(piece+0x1c+8*idx),w1=psx_mod_read_word(piece+0x20+8*idx);
                        const unsigned R=w1&0xff,B=(w1>>16)&0xff;
                        if(R<lava_paint_min || R<B*3/2) {ok=false;break;}   // painted to glow, red
                        const double p[3]={org[0]+(int16_t)(((w0&0xff)<<10)&0xffff),org[1]+(int16_t)((((w0>>8)&0xff)<<7)&0xffff),org[2]+(int16_t)((((w0>>16)&0xff)<<10)&0xffff)};
                        if(j==0) y0=p[1];else if(p[1]!=y0) {ok=false;break;}   // flat
                        for(int q=0;q<3;++q) c[q]+=p[q]/nc;
                        lo[0]=std::min(lo[0],p[0]);hi[0]=std::max(hi[0],p[0]);lo[1]=std::min(lo[1],p[2]);hi[1]=std::max(hi[1],p[2]);
                    }
                    if(!ok) continue;
                    ++candidates;
                    const int16_t tex=(int16_t)psx_mod_read_half(cursor+4);
                    int v=-2;
                    for(const auto& t:verdict) if(t.first==tex) {v=t.second;break;}
                    if(v==-2) {
                        const uint32_t e=textures+(uint32_t)(tex*12);
                        double mean[3];
                        const auto tt=std::chrono::steady_clock::now();
                        v=texture_mean(psx_mod_read_word(e),psx_mod_read_word(e+4),psx_mod_read_word(e+8),mean)?(fiery(mean)?1:0):-1;
                        tex_ms+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-tt).count();
                        verdict.push_back({tex,v});
                        if(std::getenv("DNTTK_LAVA_TRACE") && v>=0)
                            std::fprintf(stderr,"[TTK lava] level %u texture %d: %.2f %.2f %.2f %s\n",lava_level_seen,tex,v>=0?mean[0]:0,v>=0?mean[1]:0,v>=0?mean[2]:0,v?"fiery":"not fiery");
                    }
                    if(v<0) {++unreadable;continue;}
                    if(!v) continue;
                    faces.push_back({{c[0],c[1],c[2]},r,(hi[0]-lo[0]+1)*(hi[1]-lo[1]+1)});
                }
            }
        }
    }
    if(!unreadable) scan_glow_flips(lava_level_seen);   // D57N: the same snapshot
    lava_vram.reset();
    if(unreadable) lava_pending=true;   // a candidate's texture not in VRAM yet: again (up to 20 tries)
    // Pools: one per room and height (D57N: pools by distance made the lit
    // pools change every few thousand units as Duke walked, so shadows, the
    // pit ledge's among them, slid; a room's pool stands still), split only
    // where a room's lava runs further than lava_pool from its pool's middle.
    struct Pool {double sum[3],w;uint32_t room;unsigned n;};
    std::vector<Pool> pools;
    for(const Face& f:faces) {
        Pool* hit=nullptr;
        for(Pool& p:pools) {
            const double dx=p.sum[0]/p.w-f.c[0],dz=p.sum[2]/p.w-f.c[2];
            if(p.room==f.room && std::fabs(p.sum[1]/p.w-f.c[1])<64 && dx*dx+dz*dz<lava_pool*lava_pool) {hit=&p;break;}
        }
        if(!hit) {pools.push_back({{0,0,0},0,f.room,0});hit=&pools.back();}
        for(int q=0;q<3;++q) hit->sum[q]+=f.c[q]*f.area;
        hit->w+=f.area;++hit->n;
    }
    for(const Pool& p:pools) {
        Emitter e{};e.room=p.room;e.lava=true;e.tune=&lava_light;
        e.p[0]=p.sum[0]/p.w;e.p[1]=p.sum[1]/p.w-lava_lift;e.p[2]=p.sum[2]/p.w;   // up is -y
        e.seed=(uint32_t)((int32_t)e.p[0]*31+(int32_t)e.p[2]*17);
        lava.push_back(e);
    }
    if(std::getenv("DNTTK_PROP_LIGHT_TRACE") || std::getenv("DNTTK_LAVA_TRACE"))
        std::fprintf(stderr,"[TTK lava] level %u: %u flat glowing faces, %zu lava faces in %zu pools%s (%.2f ms, textures %.2f)\n",lava_level_seen,candidates,faces.size(),pools.size(),lava_pending?" (textures not loaded: again)":"",
                     std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count(),tex_ms);
    if(std::getenv("DNTTK_LAVA_TRACE")) for(const Emitter& e:lava)
        std::fprintf(stderr,"[TTK lava]   pool room %u light at %.0f %.0f %.0f\n",e.room,e.p[0],e.p[1],e.p[2]);
}
void scan_pickups() {
    std::vector<Emitter> found;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(rooms && rooms<=256 && table>=0x80010000u && table<=0x80200000u-rooms*88 && !(table&3)) {
        for(uint32_t r=0;r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
            uint32_t node=psx_mod_read_word(table+88*r+head_at);
            for(unsigned guard=0;node && guard<1024;++guard) {
                if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
                const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
                if(o<0x80010000u || o>0x801fff00u || (o&3) || psx_mod_read_byte(o+0x14)!=4) continue;
                const uint16_t type=(uint16_t)psx_mod_read_half(o+0x2c);
                for(const PickupLight& k:pickup_lights) if(k.type==type) {
                    bool dup=false;for(const Emitter& e:found) dup|=e.ref.o==o;
                    if(dup) break;
                    Emitter e{};e.room=r;e.seed=o;e.ref=prop_ref(o);e.tune=k.tune;e.pickup=true;
                    for(int c=0;c<3;++c) e.p[c]=e.ref.p[c];
                    e.p[1]-=k.lift;
                    // Keep the room-gate fade of one already lit.
                    for(const Emitter& old:pickups) if(old.ref.o==o && old.ref.type==type) e.vis=old.vis;
                    found.push_back(e);break;
                }
            }
        }
    }
    if(std::getenv("DNTTK_PROP_LIGHT_TRACE") && found.size()!=pickups.size())
        std::fprintf(stderr,"[TTK prop light] level %u: %zu glowing pickups\n",scan_level,found.size());
    pickups.swap(found);
}
// D57F: every crystal seated in a crystal holder (linked in a room; one in
// Duke's hand is in no room) where no level lamp is bound to it.
void scan_crystals() {
    std::vector<Emitter> found;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    // The crystal holders (level object table 0x800d2550, kind +0x19 = 17,
    // the generic holder 0x80092e98): a seated crystal sits 194 units above
    // one. Devices (the conveyor slot) show their power their own way.
    std::vector<std::array<int32_t,3>> holders;
    for(uint32_t i=0;i<64;++i) {
        const uint32_t o=psx_mod_read_word(0x800d2550u+4*i);
        if(o<0x80010000u || o>0x801fff00u || (o&3) || psx_mod_read_byte(o+0x19)!=17) continue;
        holders.push_back({(int32_t)psx_mod_read_word(o+4),(int32_t)psx_mod_read_word(o+8),(int32_t)psx_mod_read_word(o+12)});
    }
    auto seated=[&](const int32_t p[3]) {
        for(const auto& h:holders) if(std::abs(p[0]-h[0])<=64 && std::abs(p[2]-h[2])<=64 && h[1]-p[1]>=120 && h[1]-p[1]<=400) return true;
        return false;
    };
    // The level's lamps not loaded yet (a level change): none until they are.
    static const int32_t nowhere[3]={0,0,0};
    const bool ok=level_lamp_bound && level_lamp_bound(0,nowhere,nullptr)>=0 && rooms && rooms<=256 && table>=0x80010000u && table<=0x80200000u-rooms*88 && !(table&3);
    for(uint32_t r=0;ok && r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
        uint32_t node=psx_mod_read_word(table+88*r+head_at);
        for(unsigned guard=0;node && guard<1024;++guard) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            if(o<0x80010000u || o>0x801fff00u || (o&3) || prop_broken_flag(o)) continue;
            const uint16_t type=(uint16_t)psx_mod_read_half(o+0x2c);
            if(type<176 || type>178) continue;
            bool dup=false;for(const Emitter& e:found) dup|=e.ref.o==o;
            if(dup) continue;
            Emitter e{};e.room=r;e.seed=o;e.ref=prop_ref(o);e.crystal=true;
            BoundLamp like{};
            const int bound=level_lamp_bound(type,e.ref.p,&like);
            if(bound==2 || !seated(e.ref.p)) continue;   // its own lamp lights it here; or not in a holder
            const Tune& fallback=type==176?crystal_red:type==177?crystal_blue:crystal_green;
            float off[3]={0,20,0};
            if(bound==1) {e.own=Tune{{like.rgb[0],like.rgb[1],like.rgb[2]},like.k,like.radius};for(int c=0;c<3;++c) off[c]=like.off[c];}
            else e.own=fallback;
            e.tune=&e.own;
            for(int c=0;c<3;++c) e.p[c]=e.ref.p[c]+off[c];
            for(const Emitter& old:crystals) if(old.ref.o==o && old.ref.type==type) e.vis=old.vis;
            found.push_back(e);
        }
    }
    if(std::getenv("DNTTK_PROP_LIGHT_TRACE") && found.size()!=crystals.size())
        std::fprintf(stderr,"[TTK prop light] level %u: %zu crystals away from their first holder lit\n",scan_level,found.size());
    crystals.swap(found);
    for(Emitter& e:crystals) e.tune=&e.own;   // after the move
}
// A pickup still lies there: linked in a room, still a pickup of the same
// type. Its light follows it (a dropped one falls to the floor).
bool pickup_present(const PropSnapshot& snap,Emitter& e) {
    if(!snap.linked.count(e.ref.o) || psx_mod_read_byte(e.ref.o+0x14)!=4 || psx_mod_read_half(e.ref.o+0x2c)!=e.ref.type) return false;
    const double lift=e.ref.p[1]-e.p[1];
    for(int c=0;c<3;++c) e.p[c]=(int32_t)psx_mod_read_word(e.ref.o+4+4*c);
    e.p[1]-=lift;for(int c=0;c<3;++c) e.ref.p[c]=(int32_t)e.p[c];e.ref.p[1]=(int32_t)(e.p[1]+lift);
    return true;
}
// D41AA: each game frame, the train's state (the overlay's +0x35 for type
// 77) and, while it runs, the room it is linked in.
uint32_t train_room_of(uint32_t o) {
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return 0;
    for(uint32_t r=0;r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
        uint32_t node=psx_mod_read_word(table+88*r+head_at);
        for(unsigned guard=0;node && guard<1024;++guard) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            if(psx_mod_read_word(node+8)==o) return r;
            node=psx_mod_read_word(node+4);
        }
    }
    return ~0u;
}
void train_update() {
    if(!train.o) return;
    if(psx_mod_read_byte(train.o+0x14)!=1 || psx_mod_read_word(train.o+0x44)!=train.mesh) {train.o=train.mesh=0;train.state=~0u;train_mesh_now.store(0);return;}
    const uint32_t st=psx_mod_read_half(train.o+0x2c)==77 ? psx_mod_read_byte(train.o+0x35) : 0;
    const uint32_t ms=now_ms();
    if(train.state==~0u) {
        // Found (a load, a level): already running or wrecked long since.
        train.crashed_here=false;train_side.store(1);
        train_start_ms.store(ms-60000);train_crash_ms.store(ms-60000);
    } else if(st!=train.state) {
        if(train.state==0 && st==1) train_start_ms.store(ms);
        if(train.state==2 && st==3) {
            train_crash_ms.store(ms);train.crashed_here=true;
            for(int k=0;k<3;++k) train.crash_p[k]=(int32_t)psx_mod_read_word(train.o+4+4*k);
        }
        if(std::getenv("DNTTK_PROP_TRAIN_TRACE")) std::fprintf(stderr,"[TTK prop light] train %08x state %u -> %u at frame %u ms %u\n",train.o,train.state,st,frames,ms);
    }
    // The side it skids toward: its motion since the impact, in model space
    // (model +z goes to world (sin yaw, cos yaw) in x, z).
    if(train.crashed_here && st>=3) {
        const double yaw=(int16_t)psx_mod_read_half(train.o+0x1c)*(2*pi/4096);
        const double dx=(int32_t)psx_mod_read_word(train.o+4)-train.crash_p[0],dz=(int32_t)psx_mod_read_word(train.o+12)-train.crash_p[2];
        const double side=dx*std::sin(yaw)+dz*std::cos(yaw);
        if(std::fabs(side)>=16) train_side.store(side>0?1.0f:-1.0f,std::memory_order_relaxed);
    }
    train.state=st;
    train_state_now.store(st,std::memory_order_relaxed);train_mesh_now.store(train.mesh,std::memory_order_relaxed);
    if((st>=1 && st<=3) || frames-train.room_frame>=30) {
        train.room_frame=frames;
        const uint32_t r=train_room_of(train.o);
        if(r!=~0u) train.room=r;
    }
}
void composition_hook(CPUState*,uint32_t) {
    if(psx_mod_replay_active()) return;
    if(!input_modernized()) {emitters.clear();pickups.clear();crystals.clear();lava.clear();lava_level_seen=~0u;scan_level=~0u;train.o=0;train_mesh_now.store(0);return;}   // Vanilla: nothing
    if(!glow_flip_block) {   // D57N: before any redraw worker is forked
        glow_flip_block=psx_mod_alloc_guest_memory(glow_flip_block_size,16);
        if(glow_flip_block) {psx_mod_write_word(glow_flip_block,~0u);psx_mod_write_word(glow_flip_block+4,0);}
    }
    if(!prop_stand_block) {   // D57B rev 2: before any redraw worker is forked
        prop_stand_block=psx_mod_alloc_guest_memory(prop_stand_block_size,16);
        if(prop_stand_block) {psx_mod_write_word(prop_stand_block,~0u);psx_mod_write_word(prop_stand_block+4,0);}
    }
    ++frames;
    duke_tick_frame.store(duke_tick(now_ms()),std::memory_order_relaxed);   // D41AB
    const uint32_t level=psx_mod_read_word(0x800be570u);
    // D54: a savestate load rescans too (a prop broken since, or standing again).
    const uint32_t loads=psx_mod_savestate_loads();
    const bool rescan=level!=scan_level || loads!=scan_loads;
    if(rescan) train.state=~0u;   // D41AA: a load: no start or impact from the jump
    if(rescan || frames-scan_frame>300 || (cages_pending && frames-scan_frame>=10)) {scan_level=level;scan_loads=loads;scan_frame=frames;scan_props();}
    if(rescan || frames-pickup_frame>=10) {pickup_frame=frames;if(rescan) {pickups.clear();crystals.clear();}scan_pickups();scan_crystals();}
    if(barricades) blink_update();
    train_update();
    // D57A: lava, once a level and after each load, once the level's VRAM
    // snapshot is there (and again while its textures load), every 30 images, up to 40 tries.
    if(level!=lava_level_seen || loads!=lava_loads_seen) {lava_level_seen=level;lava_loads_seen=loads;lava_tries=0;lava_frame=frames-60;lava_pending=true;lava.clear();}
    if(lava_pending && lava_tries<40 && frames-lava_frame>=30) {lava_frame=frames;++lava_tries;scan_lava();}
}
unsigned publish(PSXModLight* out,unsigned room,bool (*shown)(uint32_t)) {
    if(!room || (emitters.empty() && pickups.empty() && crystals.empty() && lava.empty() && !train.o) || !prop_lights_enabled.load(std::memory_order_relaxed) || !input_modernized()) return 0;
    const uint32_t pl=players+psx_mod_read_word(player_index)%2*0x8a4u;
    const double duke[3]={(double)(int32_t)psx_mod_read_word(pl+4),(double)(int32_t)psx_mod_read_word(pl+8),(double)(int32_t)psx_mod_read_word(pl+12)};
    const uint32_t ms=now_ms();
    // D54: a broken prop's light sputters out.
    const PropSnapshot& snap=prop_snapshot_take();
    if(prop_snapshot_usable(snap,scan_sig)) for(Emitter& e:emitters) if(e.standing && !prop_standing(snap,e.ref)) {
        e.standing=0;e.broke_ms=ms;
        std::fprintf(stderr,"[TTK prop light] %s light goes out: its prop %08x broke\n",e.fire?"fire":e.portal?"portal":e.blink_mesh?"barricade lamp":e.cage?"caged lamp":"candle",e.ref.o);
    }
    // D57L: each lamp's room switched off or not (the game's room light colour);
    // an electric fixture in a dark room gives no light (a table lamp turns low).
    {
        const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
        const bool ok=rooms && rooms<=256 && table>=0x80010000u && table<=0x80200000u-rooms*88 && !(table&3);
        const bool follow=apartment_lights_enabled.load(std::memory_order_relaxed);
        for(Emitter& e:emitters) e.dark=follow && ok && e.cage && room_light(table,rooms,e.room)<room_dark_below;
        if(static const bool trace=std::getenv("DNTTK_PROP_LIGHT_TRACE")!=nullptr;trace) {
            static unsigned last=~0u;unsigned dark=0;for(const Emitter& e:emitters) dark+=e.dark;
            if(dark!=last) {last=dark;std::fprintf(stderr,"[TTK prop light] %u lamps in switched-off rooms\n",dark);}
        }
    }
    // Room gate (level_lamp_room_lit): a light comes on at once, fades out over 0.25 s.
    const float step=std::min(1.0f,(ms-vis_ms)/250.0f);vis_ms=ms;
    struct Near {double d;Emitter* e;};
    std::vector<Near> near;
    // D56: a pickup taken (unlinked) gives no more light; between scans the
    // snapshot (all linked objects) tells.
    const bool snap_ok=snap.valid || !snap.linked.empty();
    auto consider=[&](Emitter& e) {
        if(e.pickup && snap_ok && !pickup_present(snap,e)) return;
        if(e.crystal && snap_ok && !prop_standing(snap,e.ref)) return;   // D57F: taken (or moved) since the scan
        if(e.dark && !e.lamp) return;   // D57L: an electric fixture in a switched-off room
        const Tune* const tune=tune_now(e);
        if(!tune) return;               // D57L: the hanging lamp, its room switched off
        const float target=!shown || shown(e.room)?1.0f:0.0f;
        e.vis=e.vis<target?target:std::max(target,e.vis-step);   // D41E: on at once, out over 0.25 s
        const double dx=e.p[0]-duke[0],dy=e.p[1]-duke[1],dz=e.p[2]-duke[2];
        const double d=std::sqrt(dx*dx+dy*dy+dz*dz)-tune->radius;
        // D41E: no range cut beyond the far limit; a light fades only when
        // nearer ones push it out of the budget (as the level lamps).
        if(d<32000 && e.vis>0.01f && (e.standing || ms-e.broke_ms<prop_dying_ticks*34u)) near.push_back({d,&e});
    };
    for(Emitter& e:emitters) consider(e);
    // D41AA: the train's lights, where it is now (it runs at 256 a tick).
    if(train.o && train.state!=~0u) {
        double p[3];for(int k=0;k<3;++k) p[k]=(int32_t)psx_mod_read_word(train.o+4+4*k);
        const double yaw=(int16_t)psx_mod_read_half(train.o+0x1c)*(2*pi/4096);
        const TrainLook look=train_look(ms);
        const float side=train_side.load(std::memory_order_relaxed);
        const float flash_at[3]={train_flash_at[0],train_flash_at[1],train_flash_at[2]*side},sparks_at[3]={train_sparks_at[0],train_sparks_at[1],train_sparks_at[2]*side};
        const struct {const Tune* t;const float* at;float k;} parts[6]={{&train_lamps,train_lead_at,look.lead},{&train_lamps,train_tail_at,look.tail},
            {&train_beacon,train_beacon_at,look.beacon},{&train_beam,train_beam_at,look.beam},{&train_flash,flash_at,look.flash},{&train_sparks,sparks_at,look.sparks}};
        for(int i=0;i<6;++i) {
            Emitter& e=train_lights[i];
            e.tune=parts[i].t;e.room=train.room;e.seed=train.o+i;e.train_k=parts[i].k;e.standing=1;
            place(e.p,p,yaw,parts[i].at);
            // The beam, flash and sparks exist only while they show; the
            // lamps stay in the list through their flicker.
            if(i>=3 && parts[i].k<=0) {e.vis=0;continue;}
            consider(e);
        }
        if(static const bool trace=std::getenv("DNTTK_PROP_TRAIN_TRACE")!=nullptr;trace) {
            static uint32_t last;
            if(ms-last>=100) {last=ms;
                std::fprintf(stderr,"[TTK prop light] train look ms %u state %u room %u shown %d: lead %.2f tail %.2f beacon %.2f beam %.2f flash %.2f sparks %.2f; vis %.2f %.2f %.2f (cap %u, %zu near)\n",
                ms,train.state,train.room,shown?(int)shown(train.room):-1,look.lead,look.tail,look.beacon,look.beam,look.flash,look.sparks,
                train_lights[0].vis,train_lights[1].vis,train_lights[2].vis,room,near.size());}
        }
    }
    if(prop_pickups_enabled.load(std::memory_order_relaxed)) for(Emitter& e:pickups) consider(e);
    if(prop_crystals_enabled.load(std::memory_order_relaxed)) for(Emitter& e:crystals) consider(e);
    if(lava_lights_enabled.load(std::memory_order_relaxed)) for(Emitter& e:lava) consider(e);   // D57A
    std::sort(near.begin(),near.end(),[](const Near& a,const Near& b){return a.d<b.d;});
    // D57A: the nearest lava_max pools only; the last fades toward the next one's distance.
    double lava_d[8];unsigned lava_n=0;
    for(const Near& q:near) if(q.e->lava && lava_n<8) lava_d[lava_n++]=q.d;
    unsigned lava_rank=0;
    unsigned n=0;
    const unsigned cap=std::min(room,6u);const double cut=near.size()>cap ? near[cap].d : 32000.0;
    for(const Near& q:near) {
        if(n>=cap) break;
        Emitter& e=*q.e;   // D41Y: a barricade lamp keeps when it was last lit
        float lava_k=1;
        if(e.lava) {
            const unsigned r=lava_rank++;
            if(r>=lava_max) continue;
            if(r+1==lava_max && r+1<lava_n) lava_k=(float)std::clamp((lava_d[r+1]-lava_d[r])/lava_handoff,0.0,1.0);
            if(lava_k<0.01f) continue;   // faded out: not even a dark light in the list (its cost is per light)
        }
        const Tune* const tune=e.train_k>=0?e.tune:tune_now(e);   // D57L
        float k=tune->k*e.vis*(float)std::clamp((cut-q.d)/4000,0.0,1.0);
        if(e.blink_mesh) {
            // D41Y: on with the lit frame, a bulb's quick fade after it.
            float on=0;
            if(const BlinkSlot* b=blink_slot(e.blink_mesh,false)) on=b->level.load(std::memory_order_relaxed);
            if(on>0.5f) e.lit_ms=ms;
            else on=e.lit_ms && ms-e.lit_ms<barricade_decay ? std::max(on,1.0f-(ms-e.lit_ms)/barricade_decay) : on;
            if(!e.standing) on=1;   // broken: sputters out from lit (D54)
            k*=on;   // dark: published at 0, so the light list does not change with the blink
        } else if(e.train_k>=0) k*=e.train_k;
        else
        k*=e.lava?lava_level(ms,e.seed)*lava_k:e.portal?portal_breath(ms):e.crystal||e.cage||e.steady?1.0f:e.pickup?pickup_level(ms,e.seed):e.fire?prop_fire_level(ms,e.seed):prop_candle_level(ms,e.seed);   // D57F: steady, as its sign lamp
        if(!e.standing) k*=prop_dying_level((ms-e.broke_ms)/34u,e.seed);
        PSXModLight& l=out[n++];
        l.x=(float)e.p[0];l.y=(float)e.p[1];l.z=(float)e.p[2];l.radius=tune->radius;
        l.r=tune->rgb[0]*k;l.g=tune->rgb[1]*k;l.b=tune->rgb[2]*k;l.flags=0;
    }
    return n;
}

// ---- the drawn extras (any thread: RAM reads and the clock only) ----------
uint8_t level8(float v) {return (uint8_t)std::clamp((int)std::lround(v),0,255);}
PropGlow billboard(float x,float y,float z,float w,float h,float anchor_y,float sway,int u,int v,int tw,int th,float r,float g,float b) {
    PropGlow q{};q.billboard=true;q.p[0][0]=x;q.p[0][1]=y;q.p[0][2]=z;q.w=w;q.h=h;q.anchor_y=anchor_y;q.sway=sway;
    q.u=u;q.v=v;q.tw=tw;q.th=th;q.rgb[0]=level8(r);q.rgb[1]=level8(g);q.rgb[2]=level8(b);
    return q;
}
// D57J: haze round the club's lights (Enhanced and Relit only: LIGHTING
// ORIGINAL keeps the club as the game drew it). The neon tube glows in the
// air along its length; each spot can's lens glows in its gel and throws a
// soft shaft down its aim (two crossed quads, the beam texture: brightest at
// the lens); the canopy's bulb ring glows and the inside of the shade is lit.
// Developer: DNTTK_CLUB_HAZE="neon can beam canopy" (levels, 0 off).
float club_haze_neon=34,club_haze_can=120,club_haze_beam=26,club_haze_canopy=60;
constexpr float club_tube_rgb[3]={1.0f,0.0f,0.87f};   // the tube's texels (248 0 216)
bool club_haze_on() {
    if(!club_lights_enabled.load(std::memory_order_relaxed)) return false;
    const char* t=ttk_live_env("DNTTK_LIGHTS");
    return !(t && !std::strcmp(t,"off"));
}
PropGlow quad(const float c[4][3],int u,int v,int tw,int th,float r,float g,float b) {
    PropGlow q{};std::memcpy(q.p,c,sizeof q.p);q.u=u;q.v=v;q.tw=tw;q.th=th;q.rgb[0]=level8(r);q.rgb[1]=level8(g);q.rgb[2]=level8(b);
    return q;
}
unsigned club_glows(int kind,PropGlow* out,unsigned max) {
    if(!club_haze_on()) return 0;
    unsigned n=0;
    if(kind==prop_light_neon_bar) {
        for(int i=-2;i<=2 && n<max && club_haze_neon>0;++i)
            out[n++]=billboard(800.0f*i,40,0,900,520,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                               club_haze_neon*club_tube_rgb[0],club_haze_neon*club_tube_rgb[1],club_haze_neon*club_tube_rgb[2]);
        for(int c=0;c<4;++c) {
            const float* g=club_gel[c];
            const float lx=club_can_x[c],ly=club_can_y,lz=club_can_z;
            if(n<max && club_haze_can>0) {
                const float k=club_haze_can;
                out[n++]=billboard(lx,ly+10,lz-30,300,300,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                                   k*(0.35f+0.65f*g[0]),k*(0.35f+0.65f*g[1]),k*(0.35f+0.65f*g[2]));
            }
            if(club_haze_beam<=0) continue;
            // The shaft: from the lens (80 wide) down the aim, 1900 long, 760 wide at its end.
            constexpr float len=1900,w0=40,w1=380;
            const float e[3]={lx+club_can_aim[0]*len,ly+club_can_aim[1]*len,lz+club_can_aim[2]*len};
            // Across the beam: model x, and the direction square to x and the aim.
            const float side2[3]={0,club_can_aim[2],-club_can_aim[1]};
            for(const float* sd:{(const float*)nullptr,side2}) {
                if(n>=max) break;
                const float s[3]={sd?sd[0]:1.0f,sd?sd[1]:0.0f,sd?sd[2]:0.0f};
                const float c4[4][3]={{lx-s[0]*w0,ly-s[1]*w0,lz-s[2]*w0},{lx+s[0]*w0,ly+s[1]*w0,lz+s[2]*w0},
                                      {e[0]-s[0]*w1,e[1]-s[1]*w1,e[2]-s[2]*w1},{e[0]+s[0]*w1,e[1]+s[1]*w1,e[2]+s[2]*w1}};
                const float k=club_haze_beam;
                out[n++]=quad(c4,prop_beam_u,0,prop_beam_w,prop_beam_h,k*g[0],k*g[1],k*g[2]);
            }
        }
    } else if(kind==prop_light_canopy && club_haze_canopy>0) {
        const float k=club_haze_canopy;
        // Inside the shade, lit from below by its lamp.
        if(n<max) out[n++]=billboard(-38,-60,27,1200,700,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,k,k*0.55f,k*0.38f);
        // The bulb ring: six small warm points round the band.
        for(int i=0;i<6 && n<max;++i) {
            const float a=(float)(i*pi/3+pi/6);
            out[n++]=billboard(-38+520*std::cos(a),-400,27+520*std::sin(a),170,170,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,1.4f*k,1.4f*k*0.78f,1.4f*k*0.52f);
        }
    }
    return n;
}
unsigned glows(uint32_t mesh,int kind,PropGlow* out,unsigned max) {
    if(!visuals()) return 0;
    const uint32_t ms=now_ms();unsigned n=0;
    if(kind==prop_light_stove && max>=2) {
        const float f=prop_fire_level(ms,mesh);
        // Embers behind the grate: the quad just in front of the door face.
        PropGlow& e=out[n++];e={};
        const float c[4][3]={{-grate_x,grate_y0,grate_z},{grate_x,grate_y0,grate_z},{-grate_x,grate_y1,grate_z},{grate_x,grate_y1,grate_z}};
        std::memcpy(e.p,c,sizeof c);e.u=prop_ember_u;e.v=0;e.tw=prop_ember_w;e.th=prop_ember_h;
        e.rgb[0]=e.rgb[1]=e.rgb[2]=level8(150*f);
        // A faint bloom before it.
        out[n++]=billboard(0,(grate_y0+grate_y1)/2,grate_z+90,560,420,0.5f,0,prop_halo_u,0,prop_halo_w,prop_halo_h,70*f,70*f,70*f);
    } else if(kind==prop_light_chandelier) {
        uint32_t vp,list;
        if(!mesh_header(mesh,28,vp,list)) return 0;
        for(int i=0;i<12 && n+2<=max;++i) {
            const uint32_t xy=psx_mod_read_word(vp+8*arm_corner[i]);
            const float vx=(float)(int16_t)xy,vz=(float)(int16_t)psx_mod_read_half(vp+8*arm_corner[i]+4);
            const float len=std::sqrt(vx*vx+vz*vz);if(len<1) continue;
            const float x=vx*candle_at_u,z=vz*candle_at_u;
            const uint32_t seed=mesh*13u+(uint32_t)i*101u;
            const float t=ms/1000.0f;
            // The flame stretches and leans a little, each on its own.
            const float stretch=0.88f+0.24f*prop_light_noise(t*6.0+i*3.1,seed);
            const float sway=10.0f*(prop_light_noise(t*3.0+i*1.7,seed+5)-0.5f);
            const float k=prop_candle_level(ms,seed);
            out[n]=billboard(x,candle_y+6,z,flame_w,flame_h*stretch,1.0f,sway,prop_flame_u,0,prop_flame_w,prop_flame_h,flame_rgb*k,flame_rgb*k,flame_rgb*k);
            out[n++].upright=true;   // D41Q: stands on its candle seen from below or above too
            out[n++]=billboard(x,candle_y-30,z,halo_size,halo_size,0.5f,0,prop_halo_u,0,prop_halo_w,prop_halo_h,halo_rgb*k,halo_rgb*k,halo_rgb*k);
        }
    } else if(kind==prop_light_portal_top) {
        // A slow breath, about 0.85..1.0 (3 s), in the pad lamp's colour.
        const float l=portal_top_level*portal_breath(ms);   // in step with its light
        if(n<max) out[n++]=billboard(0,portal_top_y,0,portal_top_halo,portal_top_halo,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,l*0.55f,l*0.80f,l);
    } else if(kind==prop_light_barricade) {
        // D41Y: a soft amber halo over the lamp while the game shows it lit.
        const BlinkSlot* b=blink_slot(mesh,false);
        const float on=b?b->level.load(std::memory_order_relaxed):0;
        if(on>0.05f && n<max) {
            const float l=barricade_halo_level*on;
            out[n++]=billboard(barricade_lamp_at[0],barricade_lamp_at[1],barricade_lamp_at[2],barricade_halo_size,barricade_halo_size,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                               l*b->rgb[0].load(std::memory_order_relaxed),l*b->rgb[1].load(std::memory_order_relaxed),l*b->rgb[2].load(std::memory_order_relaxed));
        }
    } else if(kind==prop_light_cage) {
        // D41Z: a soft warm-white halo over the bulb, just before its bars.
        // In the bulb's colour (a lamp the level file lights keeps the halo too).
        const CageSlot* c=cage_slot(mesh);
        const float r=c?c->rgb[0].load(std::memory_order_relaxed):cage_light.rgb[0],g=c?c->rgb[1].load(std::memory_order_relaxed):cage_light.rgb[1],
                    b=c?c->rgb[2].load(std::memory_order_relaxed):cage_light.rgb[2];
        if(n<max) out[n++]=billboard(0,0,-40,cage_halo_size,cage_halo_size,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                                     cage_halo_level*r,cage_halo_level*g,cage_halo_level*b);
    } else if(kind==prop_light_globe) {
        // D57C: a soft halo over the globe in its glass's colour.
        const GlobeSlot* g=globe_slot(mesh);
        if(g && g->lit.load(std::memory_order_relaxed) && n<max)
            out[n++]=billboard(globe_at[0],globe_at[1],globe_at[2],globe_halo_size,globe_halo_size,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                               globe_halo_level*g->rgb[0].load(std::memory_order_relaxed),globe_halo_level*g->rgb[1].load(std::memory_order_relaxed),
                               globe_halo_level*g->rgb[2].load(std::memory_order_relaxed));
    } else if(kind==prop_light_street) {
        // D57I: the sodium lens glows under the lamp head.
        if(n<max) out[n++]=billboard(street_lens_at[0],street_lens_at[1],street_lens_at[2],street_halo_size,street_halo_size,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                                     street_halo_level*street_halo_rgb[0],street_halo_level*street_halo_rgb[1],street_halo_level*street_halo_rgb[2]);
    } else if(kind==prop_light_table_lamp) {
        // D57D: the shade glows warm round the bulb (D57L: low and amber, its room switched off).
        const bool low=look_of(prop_glow_object,mesh)==2;
        const float lv=low?dark_table_halo[0]:table_halo_level;const float* rgb=low?dark_table_halo+1:table_halo_rgb;
        if(n<max) out[n++]=billboard(table_shade_at[0],table_shade_at[1],table_shade_at[2],table_halo_size,table_halo_size,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                                     lv*rgb[0],lv*rgb[1],lv*rgb[2]);
    } else if(kind==prop_light_pendant) {
        // D57K: the bulb glows warm under its shade (D57L: rose in the
        // apartment, none with its room switched off).
        const int look=look_of(prop_glow_object,mesh);
        const float* a=apartment_pendant_halo;
        if(look==1 && n<max) out[n++]=billboard(pendant_bulb_at[0],pendant_bulb_at[1],pendant_bulb_at[2],a[0],a[0],0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                                     a[1]*a[2],a[1]*a[3],a[1]*a[4]);
        else if(look==0 && n<max) out[n++]=billboard(pendant_bulb_at[0],pendant_bulb_at[1],pendant_bulb_at[2],pendant_halo_size,pendant_halo_size,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                                     pendant_halo_level*pendant_halo_rgb[0],pendant_halo_level*pendant_halo_rgb[1],pendant_halo_level*pendant_halo_rgb[2]);
    } else if(kind==prop_light_bulb) {
        // D57M: the bare bulb glows warm (low and warmer in an apartment
        // room, none with its room switched off).
        const int look=look_of(prop_glow_object,mesh);
        const float* a=apartment_bulb_halo;
        if(look==1 && n<max) out[n++]=billboard(bulb_glass_at[0],bulb_glass_at[1],bulb_glass_at[2],a[0],a[0],0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                                     a[1]*a[2],a[1]*a[3],a[1]*a[4]);
        else if(look==0 && n<max) out[n++]=billboard(bulb_glass_at[0],bulb_glass_at[1],bulb_glass_at[2],bulb_halo_size,bulb_halo_size,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                                     bulb_halo_level*bulb_halo_rgb[0],bulb_halo_level*bulb_halo_rgb[1],bulb_halo_level*bulb_halo_rgb[2]);
    } else if(kind==prop_light_train && train_mesh_now.load(std::memory_order_relaxed)==mesh) {
        // D41AA: a glow on each bumper lamp (amber, red, amber), the beacon,
        // the headlight while it shines, the impact and the skid sparks.
        const TrainLook l=train_look(ms);
        constexpr float lamp_rgb[3][3]={{1.0f,0.72f,0.10f},{1.0f,0.10f,0.05f},{1.0f,0.72f,0.10f}};
        for(int end=0;end<2;++end) {
            const float lv=(end?l.tail:l.lead)*train_halo_lamp;
            if(lv<1) continue;
            for(int i=0;i<3 && n<max;++i) {
                const float* at=end?train_tail_lamps[i]:train_lead_lamps[i];
                out[n++]=billboard(at[0],at[1],at[2],150,150,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,lv*lamp_rgb[i][0],lv*lamp_rgb[i][1],lv*lamp_rgb[i][2]);
            }
        }
        if(n<max && l.beacon>0) {const float v=train_halo_beacon*l.beacon;
            out[n++]=billboard(-9100,-1580,15,900,900,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,v*0.35f,v*0.85f,v);}
        if(n<max && l.beam>0) {const float v=train_halo_beam*l.beam;
            out[n++]=billboard(160,-60,300,1000,1000,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,v,v*0.86f,v*0.62f);}
        if(n<max && l.flash>0) {const float v=train_halo_flash*l.flash;
            out[n++]=billboard(train_flash_at[0],train_flash_at[1],train_flash_at[2]*train_side.load(std::memory_order_relaxed)*0.75f,3600,3600,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,v,v*0.62f,v*0.25f);}
        if(l.sparks>0) {
            // A few hot points jumping about the leading wheels.
            const double t=ms/1000.0;
            for(int i=0;i<4 && n<max;++i) {
                const float v=train_halo_sparks*l.sparks*(0.5f+0.5f*prop_light_noise(t*40,200+i));
                const float x=train_sparks_at[0]+800*(prop_light_noise(t*25,210+i)-0.5f),y=train_sparks_at[1]-120*prop_light_noise(t*31,220+i),z=(train_sparks_at[2]-80+160*prop_light_noise(t*27,230+i))*train_side.load(std::memory_order_relaxed);
                out[n++]=billboard(x,y,z,260,260,0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,v,v*0.8f,v*0.5f);
            }
        }
    } else if(prop_light_club_kind(kind)) {
        n=club_glows(kind,out,max);
    } else if(kind==prop_light_atomic) {
        // D57N: a soft cyan halo round the atom, breathing with its D56 light
        // (user, 2026-11-23: its glow was swallowed by the lava's).
        const float k=pickup_level(ms,mesh)*atomic_halo[1];
        if(n<max) out[n++]=billboard(0,5,0,atomic_halo[0],atomic_halo[0],0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                                     k*atomic_light.rgb[0]/1.30f,k*atomic_light.rgb[1]/1.30f,k*atomic_light.rgb[2]/1.30f);
    } else if(kind==prop_light_delorean && max>=3) {
        // D57B: a warm-white glow on each pair of headlamps, an orange one on the vents.
        for(const float* a:delorean_lamp_at)
            out[n++]=billboard(a[0],a[1],a[2],delorean_head_halo[0],delorean_head_halo[0],0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                               delorean_head_halo[1]*delorean_head.rgb[0],delorean_head_halo[1]*delorean_head.rgb[1],delorean_head_halo[1]*delorean_head.rgb[2]);
        out[n++]=billboard(delorean_vents_at[0],delorean_vents_at[1],delorean_vents_at[2],delorean_vent_halo[0],delorean_vent_halo[0],0.5f,0,prop_white_halo_u,0,prop_white_halo_w,prop_white_halo_h,
                           delorean_vent_halo[1]*delorean_vent.rgb[0],delorean_vent_halo[1]*delorean_vent.rgb[1],delorean_vent_halo[1]*delorean_vent.rgb[2]);
    } else if(kind==prop_light_candle_stand) {
        // D57H: a soft warm halo round the flame, flickering with its light.
        // The prop is not known here: one seed per model (the copies breathe together, as a model's flames do).
        const float k=prop_candle_level(ms,mesh*13u);
        if(n<max) out[n++]=billboard(candle_flame_at[0],candle_flame_at[1],candle_flame_at[2],candle_stand_halo_size,candle_stand_halo_size,0.5f,0,prop_halo_u,0,prop_halo_w,prop_halo_h,
                                     candle_stand_halo_level*k,candle_stand_halo_level*k,candle_stand_halo_level*k);
    } else if(kind==prop_light_ring) {
        uint32_t vp,list;
        if(!mesh_header(mesh,51,vp,list)) return 0;
        // Each candle is a block of vertices 14+8c..17+8c; the halo sits
        // over its middle, at the flame.
        for(int c=0;c<5 && n<max;++c) {
            float x=0,z=0;
            for(int v=14+8*c;v<18+8*c;++v) {x+=(float)(int16_t)psx_mod_read_half(vp+8*v);z+=(float)(int16_t)psx_mod_read_half(vp+8*v+4);}
            const float k=prop_candle_level(ms,mesh*13u+(uint32_t)c*101u);
            out[n++]=billboard(x/4,ring_halo_y,z/4,ring_halo_size,ring_halo_size,0.5f,0,prop_halo_u,0,prop_halo_w,prop_halo_h,ring_halo_rgb*k,ring_halo_rgb*k,ring_halo_rgb*k);
        }
    }
    return n;
}

float tone(int kind) {return kind==prop_light_portal_top && visuals() ? portal_top_tone : 1.0f;}

// ---- textures -------------------------------------------------------------
uint16_t texel(float r,float g,float b) {
    const int R=std::clamp((int)std::lround(r*31),0,31),G=std::clamp((int)std::lround(g*31),0,31),B=std::clamp((int)std::lround(b*31),0,31);
    return (R|G|B)?(uint16_t)(0x8000|R|G<<5|B<<10):0;   // semi-transparent (STP); black is clear
}
float smooth(float a,float b,float x) {const float t=std::clamp((x-a)/(b-a),0.0f,1.0f);return t*t*(3-2*t);}
void paint(uint16_t* bank) {
    constexpr int base=256;   // page x 256 (prop_light_tpage), rows 0..31
    // Flame: a teardrop, white-gold core low in it, orange edges, a
    // transparent blue foot.
    for(int y=0;y<prop_flame_h;++y) for(int x=0;x<prop_flame_w;++x) {
        const float ny=(y+0.5f)/prop_flame_h,nx=(x+0.5f)/prop_flame_w*2-1;
        const float half=0.95f*std::pow(std::max(0.0f,(float)std::sin(pi*std::pow(ny,0.75f))),0.9f)*std::sqrt(ny);
        const float d=half>0.02f?std::fabs(nx)/half:2;
        const float body=std::pow(std::max(0.0f,1-d),0.8f)*smooth(1.0f,0.82f,ny);
        const float core=std::pow(std::max(0.0f,1-d*1.6f),1.2f)*smooth(0.25f,0.7f,ny)*smooth(1.0f,0.85f,ny);
        const float foot=std::max(0.0f,1-d)*smooth(0.8f,0.97f,ny)*0.35f;
        bank[(size_t)y*1024+base+prop_flame_u+x]=texel(body*0.95f+core*0.05f,body*0.45f+core*0.5f+foot*0.15f,body*0.08f+core*0.35f+foot*0.5f);
    }
    // Halo: a soft round warm glow.
    for(int y=0;y<prop_halo_h;++y) for(int x=0;x<prop_halo_w;++x) {
        const float dx=(x+0.5f)/prop_halo_w*2-1,dy=(y+0.5f)/prop_halo_h*2-1,r=std::sqrt(dx*dx+dy*dy);
        const float g=std::max(0.0f,std::exp(-r*r*4.0f)-std::exp(-4.0f))/(1-std::exp(-4.0f));
        bank[(size_t)y*1024+base+prop_halo_u+x]=texel(g*1.0f,g*0.62f,g*0.30f);
    }
    // D41S: the neutral halo (a falloff steeper at the rim than the warm one,
    // so a small glow reads as a point of light) and the shadow patch
    // (opaque black with STP: blended in mode 0 it halves the background).
    for(int y=0;y<prop_white_halo_h;++y) for(int x=0;x<prop_white_halo_w;++x) {
        const float dx=(x+0.5f)/prop_white_halo_w*2-1,dy=(y+0.5f)/prop_white_halo_h*2-1,r=std::sqrt(dx*dx+dy*dy);
        const float g=std::max(0.0f,std::exp(-r*r*5.0f)-std::exp(-5.0f))/(1-std::exp(-5.0f));
        bank[(size_t)y*1024+base+prop_white_halo_u+x]=texel(g,g,g);
    }
    for(int y=0;y<prop_shade_h;++y) for(int x=0;x<prop_shade_w;++x) bank[(size_t)y*1024+base+prop_shade_u+x]=0x8000;
    // D57J: the light shaft: a soft gaussian across, fading down its length.
    for(int y=0;y<prop_beam_h;++y) for(int x=0;x<prop_beam_w;++x) {
        const float ny=(y+0.5f)/prop_beam_h,nx=(x+0.5f)/prop_beam_w*2-1;
        const float across=std::exp(-nx*nx*4.0f)-std::exp(-4.0f);
        const float along=std::pow(1-ny,1.6f)*smooth(0.0f,0.08f,ny);
        const float g=std::max(0.0f,across/(1-std::exp(-4.0f))*along);
        bank[(size_t)y*1024+base+prop_beam_u+x]=texel(g,g,g);
    }
    // Embers: red-orange, hottest along the bottom, soft at the sides,
    // brighter in uneven clumps.
    for(int y=0;y<prop_ember_h;++y) for(int x=0;x<prop_ember_w;++x) {
        const float ny=(y+0.5f)/prop_ember_h,nx=(x+0.5f)/prop_ember_w*2-1;
        const float clump=0.75f+0.25f*std::sin(x*1.9f+std::sin(x*0.7f)*2.0f);
        const float g=smooth(1.0f,0.7f,std::fabs(nx))*(0.25f+0.75f*smooth(0.0f,0.85f,ny))*clump*smooth(1.0f,0.9f,ny);
        bank[(size_t)y*1024+base+prop_ember_u+x]=texel(g*1.0f,g*0.42f*(0.6f+0.4f*ny),g*0.10f);
    }
}
}  // namespace
}  // namespace ttk

PSX_MOD_CONSTRUCTOR(register_ttk_prop_lights) {
    ttk::tune("DNTTK_PROP_LIGHT_FIRE",ttk::fire_light);
    ttk::tune("DNTTK_PROP_LIGHT_CANDLES",ttk::candle_light);
    ttk::tune("DNTTK_PROP_LIGHT_FILL",ttk::fill_light);
    ttk::tune("DNTTK_PROP_LIGHT_BARREL",ttk::barrel_light);
    if(const char* e=std::getenv("DNTTK_PROP_BARREL_Y")) ttk::barrel_at[1]=(float)std::atof(e);
    ttk::tune("DNTTK_PROP_LIGHT_BARRICADE",ttk::barricade_light);
    if(const char* e=std::getenv("DNTTK_PROP_BARRICADE_HALO")) std::sscanf(e,"%f %f %f",&ttk::barricade_halo_size,&ttk::barricade_halo_level,&ttk::barricade_decay);
    ttk::tune("DNTTK_PROP_LIGHT_CAGE",ttk::cage_light);
    ttk::tune("DNTTK_PROP_LIGHT_GLOBE",ttk::globe_light);   // D57C
    ttk::tune("DNTTK_PROP_LIGHT_STREET",ttk::street_light);   // D57I
    ttk::tune("DNTTK_PROP_LIGHT_TABLE_LAMP",ttk::table_lamp_light);   // D57D
    if(const char* e=std::getenv("DNTTK_PROP_TABLE_LAMP_AT")) ttk::table_lamp_at[1]=(float)std::atof(e);
    if(const char* e=std::getenv("DNTTK_PROP_TABLE_LAMP_HALO")) std::sscanf(e,"%f %f %f %f %f",&ttk::table_halo_size,&ttk::table_halo_level,&ttk::table_halo_rgb[0],&ttk::table_halo_rgb[1],&ttk::table_halo_rgb[2]);
    ttk::tune("DNTTK_PROP_LIGHT_PENDANT",ttk::pendant_light);   // D57K
    if(const char* e=std::getenv("DNTTK_PROP_PENDANT_AT")) ttk::pendant_at[1]=(float)std::atof(e);
    ttk::tune("DNTTK_PROP_LIGHT_BULB",ttk::bulb_light);ttk::tune("DNTTK_APARTMENT_BULB",ttk::apartment_bulb);   // D57M
    if(const char* e=std::getenv("DNTTK_PROP_BULB_AT")) ttk::bulb_at[1]=(float)std::atof(e);
    ttk::tune("DNTTK_PROP_LIGHT_CANDLE_STAND",ttk::candle_stand_light);   // D57H
    ttk::tune("DNTTK_LAVA_LIGHT",ttk::lava_light);   // D57A
    if(const char* e=std::getenv("DNTTK_ATOMIC_HALO")) std::sscanf(e,"%f %f",&ttk::atomic_halo[0],&ttk::atomic_halo[1]);   // D57N
    if(const char* e=std::getenv("DNTTK_LAVA_AT")) std::sscanf(e,"%f %f",&ttk::lava_lift,&ttk::lava_pool);
    if(const char* e=std::getenv("DNTTK_LAVA_THROB")) std::sscanf(e,"%f %f",&ttk::lava_throb_depth,&ttk::lava_throb_s);
    if(const char* e=std::getenv("DNTTK_LAVA_MAX")) std::sscanf(e,"%u %f",&ttk::lava_max,&ttk::lava_handoff);
    ttk::tune("DNTTK_PROP_LIGHT_DELOREAN_HEAD",ttk::delorean_head);ttk::tune("DNTTK_PROP_LIGHT_DELOREAN_VENT",ttk::delorean_vent);   // D57B
    if(const char* e=std::getenv("DNTTK_PROP_DELOREAN_HEAD_AT")) std::sscanf(e,"%f %f %f",&ttk::delorean_head_at[0],&ttk::delorean_head_at[1],&ttk::delorean_head_at[2]);
    if(const char* e=std::getenv("DNTTK_PROP_DELOREAN_VENT_AT")) std::sscanf(e,"%f %f %f",&ttk::delorean_vent_at[0],&ttk::delorean_vent_at[1],&ttk::delorean_vent_at[2]);
    if(const char* e=std::getenv("DNTTK_PROP_DELOREAN_HALO")) std::sscanf(e,"%f %f %f %f",&ttk::delorean_head_halo[0],&ttk::delorean_head_halo[1],&ttk::delorean_vent_halo[0],&ttk::delorean_vent_halo[1]);
    if(const char* e=std::getenv("DNTTK_PROP_CANDLE_STAND_AT")) ttk::candle_stand_at[1]=(float)std::atof(e);
    if(const char* e=std::getenv("DNTTK_PROP_CANDLE_STAND_HALO")) std::sscanf(e,"%f %f",&ttk::candle_stand_halo_size,&ttk::candle_stand_halo_level);
    if(const char* e=std::getenv("DNTTK_PROP_BULB_HALO")) std::sscanf(e,"%f %f %f %f %f",&ttk::bulb_halo_size,&ttk::bulb_halo_level,&ttk::bulb_halo_rgb[0],&ttk::bulb_halo_rgb[1],&ttk::bulb_halo_rgb[2]);
    if(const char* e=std::getenv("DNTTK_APARTMENT_BULB_HALO")) std::sscanf(e,"%f %f %f %f %f",&ttk::apartment_bulb_halo[0],&ttk::apartment_bulb_halo[1],&ttk::apartment_bulb_halo[2],&ttk::apartment_bulb_halo[3],&ttk::apartment_bulb_halo[4]);
    ttk::tune("DNTTK_APARTMENT_PENDANT",ttk::apartment_pendant);ttk::tune("DNTTK_APARTMENT_TABLE",ttk::apartment_table);ttk::tune("DNTTK_DARK_TABLE",ttk::dark_table);   // D57L
    if(const char* e=std::getenv("DNTTK_APARTMENT_PENDANT_HALO")) std::sscanf(e,"%f %f %f %f %f",&ttk::apartment_pendant_halo[0],&ttk::apartment_pendant_halo[1],&ttk::apartment_pendant_halo[2],&ttk::apartment_pendant_halo[3],&ttk::apartment_pendant_halo[4]);
    if(const char* e=std::getenv("DNTTK_DARK_TABLE_HALO")) std::sscanf(e,"%f %f %f %f",&ttk::dark_table_halo[0],&ttk::dark_table_halo[1],&ttk::dark_table_halo[2],&ttk::dark_table_halo[3]);
    if(const char* e=std::getenv("DNTTK_PROP_PENDANT_HALO")) std::sscanf(e,"%f %f %f %f %f",&ttk::pendant_halo_size,&ttk::pendant_halo_level,&ttk::pendant_halo_rgb[0],&ttk::pendant_halo_rgb[1],&ttk::pendant_halo_rgb[2]);
    if(const char* e=std::getenv("DNTTK_PROP_STREET_AT")) std::sscanf(e,"%f %f",&ttk::street_at[1],&ttk::street_at[2]);
    if(const char* e=std::getenv("DNTTK_PROP_STREET_HALO")) std::sscanf(e,"%f %f %f %f %f",&ttk::street_halo_size,&ttk::street_halo_level,&ttk::street_halo_rgb[0],&ttk::street_halo_rgb[1],&ttk::street_halo_rgb[2]);
    if(const char* e=std::getenv("DNTTK_CLUB_HAZE")) std::sscanf(e,"%f %f %f %f",&ttk::club_haze_neon,&ttk::club_haze_can,&ttk::club_haze_beam,&ttk::club_haze_canopy);   // D57J
    if(const char* e=std::getenv("DNTTK_CLUB_GEL")) {   // D57J: "outer r g b inner r g b" (the cans at x +-1600, +-550)
        float v[6];
        if(std::sscanf(e,"%f %f %f %f %f %f",&v[0],&v[1],&v[2],&v[3],&v[4],&v[5])==6) for(int c=0;c<4;++c) for(int k=0;k<3;++k) ttk::club_gel[c][k]=v[(c==0||c==3?0:3)+k];
    }
    if(const char* e=std::getenv("DNTTK_PROP_GLOBE_HALO")) std::sscanf(e,"%f %f",&ttk::globe_halo_size,&ttk::globe_halo_level);
    if(const char* e=std::getenv("DNTTK_PROP_CAGE_AT")) ttk::cage_at[2]=(float)std::atof(e);
    if(const char* e=std::getenv("DNTTK_PROP_CAGE_HALO")) std::sscanf(e,"%f %f",&ttk::cage_halo_size,&ttk::cage_halo_level);
    ttk::tune("DNTTK_PROP_LIGHT_TRAIN_LAMPS",ttk::train_lamps);   // D41AA
    ttk::tune("DNTTK_PROP_LIGHT_TRAIN_BEAM",ttk::train_beam);
    ttk::tune("DNTTK_PROP_LIGHT_TRAIN_BEACON",ttk::train_beacon);
    ttk::tune_style("DNTTK_PROP_TRAIN_BEACON_STYLE",ttk::train_beacon_style);   // D41AB
    ttk::tune("DNTTK_PROP_LIGHT_TRAIN_FLASH",ttk::train_flash);
    ttk::tune("DNTTK_PROP_LIGHT_TRAIN_SPARKS",ttk::train_sparks);
    if(const char* e=std::getenv("DNTTK_PROP_TRAIN_BEAM_AT")) std::sscanf(e,"%f %f %f",&ttk::train_beam_at[0],&ttk::train_beam_at[1],&ttk::train_beam_at[2]);
    if(const char* e=std::getenv("DNTTK_PROP_TRAIN_HALO")) std::sscanf(e,"%f %f %f %f %f",&ttk::train_halo_lamp,&ttk::train_halo_beacon,&ttk::train_halo_beam,&ttk::train_halo_flash,&ttk::train_halo_sparks);
    ttk::prop_light_lit_mesh=ttk::lit_mesh;   // D41X barrel, D41Y barricade, D41Z caged lamp
    ttk::tune("DNTTK_PROP_LIGHT_ATOMIC",ttk::atomic_light);
    ttk::tune("DNTTK_PROP_LIGHT_RING",ttk::ring_light);
    ttk::tune("DNTTK_PROP_LIGHT_RING_FILL",ttk::ring_fill);
    ttk::tune("DNTTK_PROP_LIGHT_PORTAL",ttk::portal_light);
    ttk::tune("DNTTK_PROP_LIGHT_CRYSTAL_RED",ttk::crystal_red);
    ttk::tune("DNTTK_PROP_LIGHT_CRYSTAL_BLUE",ttk::crystal_blue);
    ttk::tune("DNTTK_PROP_LIGHT_CRYSTAL_GREEN",ttk::crystal_green);
    if(const char* e=std::getenv("DNTTK_PROP_RING_HALO")) std::sscanf(e,"%f %f %f",&ttk::ring_halo_size,&ttk::ring_halo_rgb,&ttk::ring_halo_y);
    if(const char* e=std::getenv("DNTTK_PROP_RING_AT")) std::sscanf(e,"%f %f",&ttk::ring_at[1],&ttk::ring_fill_at[1]);
    if(const char* e=std::getenv("DNTTK_PROP_FLAMES")) std::sscanf(e,"%f %f %f %f %f %f %f",&ttk::flame_w,&ttk::flame_h,&ttk::flame_rgb,&ttk::halo_size,&ttk::halo_rgb,&ttk::candle_at_u,&ttk::candle_y);
    psx_mod_register_function_entry_plugin("ttk.prop.lights",0x80026164u,ttk::composition_hook);
    ttk::prop_light_kind=ttk::kind_of;
    ttk::prop_light_visuals=ttk::visuals;
    ttk::prop_light_publish=ttk::publish;
    ttk::prop_mesh_swap=ttk::stand_swap;   // D57B rev 2
    ttk::prop_light_glows=ttk::glows;
    ttk::prop_light_tone=ttk::tone;
    if(const char* e=std::getenv("DNTTK_PROP_PORTAL_TOP")) std::sscanf(e,"%f %f %f %f",&ttk::portal_top_tone,&ttk::portal_top_halo,&ttk::portal_top_level,&ttk::portal_top_y);
    ttk::prop_light_paint_bank=ttk::paint;
}
