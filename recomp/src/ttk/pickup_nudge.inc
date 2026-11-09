// D08A41: placed pickups moved out of awkward spots (Modernized only).
// The original pickup loop (8007FD04) touches an item only when Duke's centre
// is within max(0x100, +0x5a) of it on X and on Z. Duke's hull keeps his
// centre off the walls, so an item pushed deep into a corner is only touched
// on some approaches. Each entry names one level item by level, type and its
// original X/Z; when that object is found at that exact spot (a fresh level
// or any savestate) it is moved by the offset and relinked to its cell with
// the original 0x80095cb0. A moved item no longer matches, so each object
// moves once. Vanilla never reaches this code.
struct PickupNudge {unsigned level;uint16_t type;int32_t x,z,dx,dz;const char* name;};
static constexpr PickupNudge pickup_nudges[]={
    // Level 0, the corner of the stone room (user's UI slot 8).
    {0,581,-2907,19306,128,-160,"atomic health"},
};
static unsigned pickup_nudge_ticks,pickup_nudge_moves;
static void pickup_nudge_update(CPUState* cpu) {
    // Rescan every 30 player updates: covers a fresh level, a restart after
    // death and savestate loads without tracking each.
    static const bool off=[]{const char* e=std::getenv("DNTTK_PICKUP_NUDGE");return e && *e=='0';}(); // A/B testing
    if(off || ++pickup_nudge_ticks%30)return;
    const unsigned level=psx_mod_read_word(0x800be570);
    bool wanted=false;
    for(const PickupNudge& n:pickup_nudges)wanted|=n.level==level;
    if(!wanted || !player_identity_ready() || psx_mod_read_word(0x800c27bc)!=1 || guest_busy ||
       cpu->gpr[29]<0x801f4000 || g_precise_mode || g_ls_mode || g_psx_call_bail)return;
    const uint32_t rooms=psx_mod_read_word(0x800da454),count=psx_mod_read_word(0x800da47c);
    if(count>128 || (count && (rooms<0x80010000 || rooms>0x80200000-count*88 || (rooms&3))))return;
    for(const PickupNudge& n:pickup_nudges) {
        if(n.level!=level)continue;
        for(unsigned room=0;room<count;++room) {
            uint32_t node=psx_mod_read_word(rooms+88*room+0x48);
            for(unsigned k=0;node && k<1024;++k) {
                if(node<0x80010000 || node>0x801ffff0 || (node&3))return;
                const uint32_t object=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
                if(!cheat_ram(object,96) || psx_mod_read_half(object+0x2c)!=n.type ||
                   (int32_t)psx_mod_read_word(object+4)!=n.x || (int32_t)psx_mod_read_word(object+12)!=n.z ||
                   psx_mod_read_byte(object+0x35)==0x63)continue;
                psx_mod_write_word(object+4,uint32_t(n.x+n.dx));
                psx_mod_write_word(object+12,uint32_t(n.z+n.dz));
                int32_t ignored=0;
                hull_call(cpu,0x80095cb0,object,&ignored);
                ++pickup_nudge_moves;
                std::fprintf(stderr,"[TTK pickup] level %u %s %08x moved %+d,%+d out of its corner\n",
                             level,n.name,object,n.dx,n.dz);
                goto next; // the scan above may now be stale; one object per entry
            }
        }
        next:;
    }
}
