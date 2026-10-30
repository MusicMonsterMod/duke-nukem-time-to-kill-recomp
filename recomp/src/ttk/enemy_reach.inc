// D17AQ dormant enemies (Modernized "extended" draw distance; Vanilla and
// "original" are unchanged). Included after draw_distance.inc.
//
// The original creates spawner enemies within its own fog edge and removes
// them past it. Level load stores fade start + 0x13d0 (the original mesh far
// limit, 25552 in level 6) at 0x800da474; the spawner setup 0x8009934c squares
// it into 0x800dd9e0 and loads a per-level cap of spawned enemies (0x800c56a8;
// live count 0x800c56a4). Each frame the spawner tick 0x80099530 (called with
// Duke's position, single player only) visits four spawners (records at
// *0x800de720, 0x30 bytes: +0 type, +4 actor, +8 position, +0x14 flags, bit 0
// spawned, 0x16 skip): a spawned one goes to the removal check 0x800992fc, an
// empty one to the spawn check 0x8009926c, which also replaces the farthest
// enemy (0x80099048) when the cap is full and creates through 0x80098f3c. Both
// ask the range test 0x80099164 (a0 Duke's position, a1 the record), which
// answers "too far" when the squared distance from 0x800675d4 exceeds the
// radius; that distance saturates to 0x7fffffff once an axis differs by 32768.
// Extended draw distance shows twice that range and more, so enemies appeared
// out of nothing in plain view.
//
// As in Duke 3D, enemies are now there before they come into view but do not
// react until Duke is near or they are hurt:
// - The range test answers with the exact distance against the extended mesh
//   limit to spawn and that plus 4096 to remove, by pointing a0 at the
//   record's own position (distance 0, in range) or at a far point (saturated,
//   out of range). No game variable is written. The cap, the live count and
//   the nearest-first replacement are the original's, so no more enemies exist
//   than the original allows (a failed creation disables its spawner for good,
//   flag 0x10), and a nearer spawner still replaces the farthest, dormant one.
// - Dormant: a spawned enemy farther than the original radius that has not
//   lost health since it was spawned. The actor loop (0x80025ac8) runs each
//   actor's prep 0x80025944 and then its class controller, handler table
//   *0x800dd850 indexed by byte +0x15 (class 0 is the empty handler
//   0x800632b8). For a dormant enemy the prep hook gives it class 0 and the
//   empty handler's hook puts its class straight back, so its controller does
//   not run that frame and nothing else ever sees the swap (0x8006cefc, right
//   after the loop, restores any left over; a savestate taken inside that
//   short window is healed at the first tick after loading it, from the type
//   table *0x800d2660, 28-byte entries, byte 6 the class the actor was created
//   with; only then, so a class the game itself clears is left alone). Within the
//   original radius it is
//   the original enemy; shot or caught in a blast it wakes for good.
// Developer: DNTTK_ENEMY_REACH=0 keeps the original spawner range.
static const DistanceGuard reach_guards[]={
    {0x80099164,0x58,"66356f6fc46dbb1e47c505564f0f50d792c179430bb225a19fd595b73bb5e9c9"},
    {0x800991bc,0x190,"0822d02fea0d9fd09ef33ced347246665fd22a61965ad462610c459629855395"},
    {0x8009934c,0xb4,"e55a581f179a5f7858131d7f7f0da5864c54e10e112e60533a6de590bfea8f97"},
    {0x80099530,0xe4,"d81fb9a6de284246bebd1087651b0f470557352c586900d55cfb93e3667ffbc7"},
    {0x80025ac8,0x1d4,"ce0ba458f4fc8518425e9b5b3144d9cfb83e1cf82d06c7af45a1a8c9d59f0206"},
};
static std::array<std::vector<uint32_t>,5> reach_words;
static uint64_t reach_spawn_in=0,reach_spawn_out=0,reach_kept=0,reach_removed=0,reach_refusals=0;
static uint64_t reach_held=0,reach_woken=0,reach_healed=0;
static uint32_t reach_far_point=0;
struct ReachActor {uint32_t record,actor;int16_t health;bool woken,dormant;};
static std::vector<ReachActor> reach_actors;
struct ReachSwap {uint32_t actor;uint8_t cls;};
static std::vector<ReachSwap> reach_swaps;
static bool enemy_reach_live() {
    static const bool enabled=[]{const char* t=std::getenv("DNTTK_ENEMY_REACH");return !t || std::strcmp(t,"0");}();
    return enabled && draw_distance_extended() && code_identity(reach_guards,reach_words);
}
static double reach_extended() {
    const int32_t fade=(int32_t)psx_mod_read_word(level_fade);
    return fade>0 && fade+0x13d0<0xffff ? extend_limit(fade+0x13d0) : 0;
}
static double reach_distance2(uint32_t a,uint32_t b) {
    double d=0;
    for(int i=0;i<3;++i) {const double x=(double)(int32_t)psx_mod_read_word(a+4*i)-(int32_t)psx_mod_read_word(b+4*i);d+=x*x;}
    return d;
}
// The range test, by caller: spawn (0x800991ec, 0x80099288) or removal
// (0x80099310). The three types the original never tests (0x3d, 0x385, 0x3e)
// and every other caller are left to it.
static void reach_range(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    const bool spawn=ra==0x800991ecu || ra==0x80099288u,remove=ra==0x80099310u;
    if((!spawn && !remove) || !enemy_reach_live()) return;
    const uint32_t duke=cpu->gpr[4],record=cpu->gpr[5];
    if(duke<0x80010000u || duke>0x801ffff0u || (duke&3) || record<0x80010000u || record>0x801fffe8u || (record&3)) return;
    const uint32_t type=psx_mod_read_word(record);
    if(type==0x3d || type==0x385 || type==0x3e) return;
    const double reach=reach_extended()+(remove?4096.0:0.0);
    if(reach<=4096) {++reach_refusals;return;}
    const bool in=reach_distance2(duke,record+8)<=reach*reach;
    if(in) cpu->gpr[4]=record+8;
    else {
        if(!reach_far_point) reach_far_point=psx_mod_alloc_guest_memory(16,16);
        if(!reach_far_point) {++reach_refusals;return;}
        for(int i=0;i<3;++i) psx_mod_write_word(reach_far_point+4*i,0x40000000u);
        cpu->gpr[4]=reach_far_point;
    }
    if(spawn) ++(in?reach_spawn_in:reach_spawn_out); else ++(in?reach_kept:reach_removed);
}
// Spawner tick (a0 Duke's position), before the actor loop: decide which
// spawned enemies are dormant this frame.
static void reach_tick(CPUState* cpu) {
    if(!enemy_reach_live()) {reach_actors.clear();return;}
    const uint32_t duke=cpu->gpr[4],records=psx_mod_read_word(0x800de720),count=psx_mod_read_word(0x800c56a0);
    const uint32_t r2=psx_mod_read_word(0x800dd9e0);
    if(duke<0x80010000u || duke>0x801ffff0u || (duke&3) || !count || count>512 ||
       records<0x80010000u || records>0x80200000u-count*0x30 || (records&3) || !r2) {reach_actors.clear();return;}
    static uint32_t loads=psx_mod_savestate_loads();
    const bool heal=loads!=psx_mod_savestate_loads();
    loads=psx_mod_savestate_loads();
    if(heal) reach_swaps.clear();
    std::vector<ReachActor> next;
    for(uint32_t i=0;i<count;++i) {
        const uint32_t record=records+0x30*i,actor=psx_mod_read_word(record+4);
        if(!(psx_mod_read_word(record+0x14)&1) || actor<0x80010000u || actor>0x801ffc00u || (actor&3)) continue;
        if(heal && !psx_mod_read_byte(actor+0x15)) {
            const uint32_t types=psx_mod_read_word(0x800d2660),type=psx_mod_read_half(actor+0x2c);
            const uint8_t cls=types>=0x80010000u && types<0x801f0000u && type<4096 ? psx_mod_read_byte(types+28*type+6) : 0;
            if(cls) {psx_mod_write_byte(actor+0x15,cls);++reach_healed;}
        }
        const int16_t health=(int16_t)psx_mod_read_half(actor+0x32);
        ReachActor a{record,actor,health,false,false};
        for(const auto& old:reach_actors) if(old.actor==actor && old.record==record) {
            a.woken=old.woken || health<old.health;
            if(!old.woken && a.woken) ++reach_woken;
            break;
        }
        a.dormant=!a.woken && reach_distance2(duke,actor+4)>(double)r2;
        next.push_back(a);
    }
    reach_actors.swap(next);
}
static bool reach_dormant(uint32_t actor) {
    for(const auto& a:reach_actors) if(a.actor==actor) return a.dormant;
    return false;
}
static void reach_restore() {
    for(const auto& s:reach_swaps) if(!psx_mod_read_byte(s.actor+0x15)) psx_mod_write_byte(s.actor+0x15,s.cls);
    reach_swaps.clear();
}
// Actor prep (a0 the actor), from the actor loop only.
static void reach_hold(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31],actor=cpu->gpr[4];
    if((ra!=0x80025be0u && ra!=0x80025c58u) || reach_actors.empty() || !reach_dormant(actor)) return;
    const uint8_t cls=psx_mod_read_byte(actor+0x15);
    if(!cls) return;
    reach_swaps.push_back({actor,cls});
    psx_mod_write_byte(actor+0x15,0);
    ++reach_held;
}
// The empty class handler (a0 the actor) and the call after the loop.
static void reach_release(CPUState* cpu) {
    if(reach_swaps.empty()) return;
    const uint32_t actor=cpu->gpr[4];
    for(size_t i=0;i<reach_swaps.size();++i) if(reach_swaps[i].actor==actor) {
        psx_mod_write_byte(actor+0x15,reach_swaps[i].cls);
        reach_swaps.erase(reach_swaps.begin()+i);
        return;
    }
}
static const char* enemy_reach_debug_json() {
    static char buffer[320];
    unsigned dormant=0;for(const auto& a:reach_actors) dormant+=a.dormant;
    std::snprintf(buffer,sizeof buffer,"{\"spawn_in\":%llu,\"spawn_out\":%llu,\"kept\":%llu,\"removed\":%llu,\"refusals\":%llu,"
        "\"held\":%llu,\"woken\":%llu,\"healed\":%llu,\"spawned\":%u,\"dormant\":%u,\"live\":%u,\"cap\":%u}",
        (unsigned long long)reach_spawn_in,(unsigned long long)reach_spawn_out,(unsigned long long)reach_kept,
        (unsigned long long)reach_removed,(unsigned long long)reach_refusals,(unsigned long long)reach_held,
        (unsigned long long)reach_woken,(unsigned long long)reach_healed,(unsigned)reach_actors.size(),dormant,psx_mod_read_word(0x800c56a4),psx_mod_read_word(0x800c56a8));
    return buffer;
}
