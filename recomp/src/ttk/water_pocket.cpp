// D17BD: PIG FACTORY's pool gets its four missing water surfaces (Modernized).
// Each room lists water boxes (room +0x3c, count +0x31, 24 bytes each). The
// two water passes (loops at 0x80030090 and 0x80037a48) call the surface
// builder 0x8002ef90 for every box whose first halfword has bit 0 clear. In
// room 62 of PIG FACTORY four boxes at the pool's surface height, all water
// Duke swims in, have bit 0 set and so no surface: two shafts off the pool
// (beside the brick pillar and opposite it), where the first-person view
// looks down into dry walls, and the two triangular ends of the sloping
// beach, cut by the room's diagonal far wall (depth hides the rest of each
// square tile). The builder reads only the box's extents, material and
// height. While the builder starts on the pool's own box (the list's first),
// the pockets' bits are cleared, so the same loop builds them next; each bit
// is set again as soon as the builder starts on its box. No save or load can
// happen in between (psx_mod_host_call_begin/end), and the per-frame
// 0x8001fba0 restores anything left over, so no save sees the change. Tiles
// then take the usual water path (D17AF clip, D17AN depth).
// DNTTK_WATER_POCKET=0 (developer) keeps the authored data.
#include "cpu_state.h"
#include "mod_plugins.h"
#include "pc_input.h"
#include "code_identity.h"
#include <cstdlib>
#include <cstring>

extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;

namespace {
constexpr uint32_t room_table=0x800da454u,room_count=0x800da47cu,level_tag=0x800ca968u;
constexpr uint32_t pig_factory=0xcu,pool_room=62;
struct Pocket {unsigned index;int16_t box[7];};
// x0 y0 z0 . x1 y1 z1, room-local; index in room 62's list.
const Pocket pockets[]={
    {1,{6144,3584,2048,0,7168,5632,4096}},  // opposite the pillar
    {2,{1024,3584,2048,0,2048,6656,4096}},  // beside the pillar
    {5,{1024,3584,5120,0,2048,3968,6144}},  // beach corner, pillar side
    {6,{6144,3584,5120,0,7168,3968,6144}},  // beach corner, opposite
};
constexpr unsigned pocket_count=sizeof pockets/sizeof pockets[0];
uint32_t cleared[pocket_count]={};uint64_t cleared_epoch=0;bool holding=false;
bool box_is(uint32_t box,const int16_t (&want)[7]) {
    static const unsigned at[]={0,1,2,4,5,6};   // x0 y0 z0 . x1 y1 z1
    for(unsigned i:at) if((int16_t)psx_mod_read_half(box+8+2*i)!=want[i]) return false;
    return true;
}
bool identity() {
    struct Guard {uint32_t address,size;const char* digest;};
    static const Guard guards[]={
        {0x8002ef90u,0xcc0u,"52db23011c61c794e5d5f5923fdf405242612f7ff8251d5e1734d151fc0b1daa"},
        {0x80030020u,0xe0u,"91b8cc3941786b13cc8963864efbd2fe65631d4ee50a10fb905bba57e784ef9f"},
        {0x800379ecu,0xc0u,"27c6dddd6f6305174f8476aea0507a760387d0cf312661850d11efe49d91484c"},
    };
    static std::array<std::vector<uint32_t>,3> expected;
    static ttk::IdentityMemo memo;
    const auto frame=ttk::input_host_frame();
    if(!memo.valid(frame,g_dirty_ram_code_gen))
        memo.set(frame,g_dirty_ram_code_gen,ttk::code_identity(guards,expected,psx_mod_read_word,g_psx_ram));
    return memo.ok;
}
// Whether `box` is the first of room 62's list (the pool); `out` gets each
// pocket's box when it matches with bit 0 set, else 0.
bool pool_list(uint32_t box,uint32_t (&out)[pocket_count]) {
    static const int16_t pool[7]={2048,3584,1024,0,6144,5632,6144};
    if(psx_mod_read_word(level_tag)!=pig_factory || psx_mod_read_word(room_count)<=pool_room) return false;
    const uint32_t room=psx_mod_read_word(room_table)+88*pool_room;
    if(room<0x80010000u || room>0x801fff00u || psx_mod_read_word(room+0x3c)!=box ||
       box<0x80010000u || box>0x801fff00u || (psx_mod_read_half(box)&1) || !box_is(box,pool)) return false;
    const unsigned listed=psx_mod_read_byte(room+0x31);
    bool any=false;
    for(unsigned k=0;k<pocket_count;++k) {
        const uint32_t p=box+24*pockets[k].index;
        out[k]=pockets[k].index<listed && (psx_mod_read_half(p)&1) &&
               psx_mod_read_half(p+4)==psx_mod_read_half(box+4) && box_is(p,pockets[k].box) ? p : 0;
        any|=out[k]!=0;
    }
    return any;
}
void restore(uint32_t& p) {
    // A savestate load replaced the RAM: it holds the authored bit already.
    if(p && cleared_epoch==psx_mod_savestate_loads()) psx_mod_write_half(p,psx_mod_read_half(p)|1);
    p=0;
    bool any=false;for(uint32_t c:cleared) any|=c!=0;
    if(!any && holding) {holding=false;psx_mod_host_call_end();}
}
void frame_restore(CPUState*,uint32_t) {for(auto& p:cleared) restore(p);}
void water_pocket(CPUState* cpu,uint32_t) {
    const uint32_t box=cpu->gpr[4],ra=cpu->gpr[31];
    bool pending=false,mine=false;
    for(auto& p:cleared) {pending|=p!=0;if(p && p==box) {restore(p);mine=true;}}
    if(mine) return;
    // Any other box: the loop has moved on, so nothing stays cleared.
    if(pending) {for(auto& p:cleared) restore(p);return;}
    static const bool on=[] {const char* s=std::getenv("DNTTK_WATER_POCKET");return !(s && !std::strcmp(s,"0"));}();
    if(!on || (ra!=0x800300c4u && ra!=0x80037a88u) || !ttk::input_modernized() || !identity()) return;
    uint32_t found[pocket_count];
    if(!pool_list(box,found)) return;
    if(!holding) {holding=true;psx_mod_host_call_begin();}
    for(unsigned k=0;k<pocket_count;++k) if(found[k]) {
        psx_mod_write_half(found[k],psx_mod_read_half(found[k])&~1u);cleared[k]=found[k];
    }
    cleared_epoch=psx_mod_savestate_loads();
}
}
PSX_MOD_CONSTRUCTOR(register_ttk_water_pocket) {
    psx_mod_register_function_entry_plugin("ttk.water.pocket",0x8002ef90u,water_pocket);
    psx_mod_register_function_entry_plugin("ttk.water.pocket.frame",0x8001fba0u,frame_restore);
}
