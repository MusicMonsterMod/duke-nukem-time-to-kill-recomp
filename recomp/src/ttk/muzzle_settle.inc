// D47: a shot from rest smokes at the barrel, not at Duke's shoulder.
// Included by modern_controls.cpp after dynamic_lights.inc.
//
// The muzzle point player +0xbc..+0xc4 (where rounds launch, the gun call's
// smoke and D41B's muzzle light) is written by 0x80032364 at the start of
// Duke's actor draw 0x800348d8: the muzzle joint (+0xc8) of the pose just
// evaluated plus the weapon's offset (table 0x800c45b0; Duke's position when
// the joint is unset or the point is outside the rooms); the draw then calls
// 0x80032bd8 (a0 the actor) unconditionally. The gun call
// 0x8003c500 (ra 0x8004f7b4 / 0x8004fa88) runs earlier in the frame, in the
// player update, so a shot fired while the gun is still in its rest pose
// (the first shot of a press after the gun has settled) reads the previous
// frame's rest-pose muzzle: with the shotgun, about 250 units up and 250 to
// Duke's left, at the shoulder. The fire pose is drawn the same frame, so
// held shots (gun already up) are right. Vanilla does the same (original
// behaviour, lab note 422).
//
// Modernized: the gun call notes the muzzle it saw and the effects already
// alive; when Duke's draw has written the new muzzle (the 0x80032bd8 entry
// after it), the effects that shot added at the old muzzle (its smoke) and
// Duke's muzzle light from that launch move onto the new one (only when the
// shot left such an effect: the Combat Shotgun's puff). Effects are
// drawn later in the same camera render (0x8009ce44 after the object loop
// 0x800374cc), so the smoke is never drawn at the shoulder. A game frame is
// composition (where the 120/180 Hz redraw workers copy the game), render,
// then update: the shot's smoke reaches the workers before the next live
// draw corrects it, so the pending shot is kept in guest memory (carried by
// the workers' copies) and each replayed Duke draw makes the same move in
// its own copy. The muzzle light is host state published with the image, so
// redraws show it moved from the following game frame. The rounds keep
// their origin and direction (hits unchanged); the game's records are only
// moved, never added or removed. DNTTK_MUZZLE_SETTLE=0 turns it off.
static const Guard muzzle_settle_guards[] = {
    {0x80032364, 0x73c, "b66ac289e4216f2109d13cdf313dde77e98336e9c0ae46592e3eafba2d3465ca"},
    {0x8003c500, 400, "b7d03cf51600ef4a6bc471c986f8fd8f7dcf6daac4e1ea504af9562fa8049c67"},
    {0x800348d8, 3112, "fa5b20b17a5964109586d0a40c2eeebdfa92c5c083ff8f0713aa56cdc409ed1f"},
};
constexpr uint32_t settle_fx_pool=0x800c5894u, settle_fx_count=0x800c5898u, settle_fx_active=0x800deb50u, settle_fx_stride=0x54u;
// Guest block (visible to the workers' copies): +0 settle_pending while a shot
// waits (a copy taken before the block existed holds anything else), +4 the old muzzle
// (3 words), +0x10 count, +0x14 one 0x14-byte entry per effect alive at the
// gun call (+0 address, +4 type, +8 position).
constexpr uint32_t settle_max_fx=64u, settle_entry=0x14u, settle_pending=0x44343750u;
static uint32_t settle_block;
static uint64_t settle_frame;
static uint64_t settle_shots,settle_moves,settle_fx_moved,settle_lights_moved,settle_expired;
static std::atomic<uint64_t> settle_replays{0};   // worker redraws that moved a shot's smoke
static double settle_last[3];
static bool muzzle_settle_enabled() {
    const char* v=ttk_live_env("DNTTK_MUZZLE_SETTLE");
    return input_modernized() && !(v && v[0]=='0' && !v[1]);
}
static bool muzzle_settle_identity() {
    static thread_local std::array<std::vector<uint32_t>,std::size(muzzle_settle_guards)> expected;
    static thread_local IdentityMemo memo;
    const uint64_t frame=input_host_frame();
    if(!memo.valid(frame,g_dirty_ram_code_gen)) memo.set(frame,g_dirty_ram_code_gen,code_identity(muzzle_settle_guards,expected,psx_mod_read_word,g_psx_ram));
    return memo.ok;
}
// The live effects (+0 next, +8 type, +0x1c position), at most 64.
template<class F> static void settle_walk_fx(F&& f) {
    const uint32_t pool=psx_mod_read_word(settle_fx_pool),count=std::min<uint32_t>(psx_mod_read_word(settle_fx_count),settle_max_fx);
    uint32_t a=psx_mod_read_word(settle_fx_active);
    for(unsigned k=0;a && k<settle_max_fx;++k) {
        if(a<pool || a>=pool+count*settle_fx_stride || (a-pool)%settle_fx_stride) return;
        f(a);a=psx_mod_read_word(a);
    }
}
static void settle_read3(uint32_t a,int32_t* v) {for(int i=0;i<3;++i) v[i]=(int32_t)psx_mod_read_word(a+4*i);}
// 0x8003c500 entry: Duke's gun call (a0 the player). Live game only.
static void muzzle_settle_shot(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    if(cpu->gpr[4]!=player || (ra!=0x8004f7b4u && ra!=0x8004fa88u) || psx_mod_replay_active()) return;
    if(settle_block && psx_mod_read_word(settle_block)==settle_pending) {++settle_expired;psx_mod_write_word(settle_block,0);}
    if(!muzzle_settle_enabled() || !muzzle_settle_identity()) return;
    if(!settle_block) settle_block=psx_mod_alloc_guest_memory(0x14+settle_max_fx*settle_entry,16);
    if(!settle_block) return;
    for(int i=0;i<3;++i) psx_mod_write_word(settle_block+4+4*i,psx_mod_read_word(player+0xbc+4*i));
    uint32_t n=0;
    settle_walk_fx([&](uint32_t a){
        const uint32_t e=settle_block+0x14+n*settle_entry;
        psx_mod_write_word(e,a);psx_mod_write_word(e+4,psx_mod_read_word(a+8));
        for(int i=0;i<3;++i) psx_mod_write_word(e+8+4*i,psx_mod_read_word(a+0x1c+4*i));
        ++n;
    });
    psx_mod_write_word(settle_block+0x10,n);
    psx_mod_write_word(settle_block,settle_pending);
    settle_frame=input_host_frame();++settle_shots;
}
// 0x80032bd8 entry from Duke's draw (ra 0x8003492c, a0 the player): the
// muzzle writer 0x80032364 has just written this frame's muzzle. Runs in the
// live draw and in each worker's replayed draw (its own copy).
static void muzzle_settle_drawn(CPUState* cpu) {
    if(!settle_block || cpu->gpr[4]!=player || cpu->gpr[31]!=0x8003492cu || psx_mod_read_word(settle_block)!=settle_pending) return;
    const bool live=!psx_mod_replay_active();
    psx_mod_write_word(settle_block,0);
    if(live && input_host_frame()-settle_frame>12) {++settle_expired;return;}
    int32_t old[3],now[3];settle_read3(settle_block+4,old);settle_read3(player+0xbc,now);
    double d[3],len=0;for(int i=0;i<3;++i) {d[i]=now[i]-old[i];len+=d[i]*d[i];}
    len=std::sqrt(len);
    if(len<8 || len>1500) return;
    // Effects the shot added at the old muzzle (not alive, or elsewhere, at the gun call).
    const uint32_t n=std::min(psx_mod_read_word(settle_block+0x10),settle_max_fx);
    unsigned moved=0;
    settle_walk_fx([&](uint32_t a){
        int32_t p[3];settle_read3(a+0x1c,p);const uint32_t type=psx_mod_read_word(a+8);
        for(uint32_t i=0;i<n;++i) {
            const uint32_t e=settle_block+0x14+i*settle_entry;int32_t q[3];settle_read3(e+8,q);
            if(psx_mod_read_word(e)==a && psx_mod_read_word(e+4)==type && !std::memcmp(q,p,sizeof p)) return;
        }
        double r=0;for(int i=0;i<3;++i) r+=double(p[i]-old[i])*(p[i]-old[i]);
        if(r>48.0*48.0) return;
        for(int i=0;i<3;++i) psx_mod_write_word(a+0x1c+4*i,(uint32_t)(p[i]+(int32_t)std::lround(d[i])));
        ++moved;
    });
    if(!live) {if(moved) ++settle_replays;return;}
    // Only a shot that left its puff at the stale muzzle (the shotgun) is
    // moved: the Desert Eagle, Gatling and RPG leave none there, and the RPG's
    // muzzle alternates between its tubes from frame to frame, so their
    // lights stay at the round's origin.
    if(!moved) return;
    ++settle_moves;settle_fx_moved+=moved;std::copy(d,d+3,settle_last);
    // D41B's muzzle light from this launch (lit at the round's origin).
    unsigned lit=0;
    for(unsigned i=0;i<light_src_count;++i) {
        LightSrc& s=light_srcs[i];
        if(s.kind!=light_kind_muzzle || s.age!=0) continue;
        double r=0;for(int k=0;k<3;++k) r+=(s.p[k]-old[k])*(s.p[k]-old[k]);
        if(r>48.0*48.0) continue;
        for(int k=0;k<3;++k) s.p[k]+=d[k];
        ++lit;
    }
    if(lit) {settle_lights_moved+=lit;lights_publish();}
    if(std::getenv("DNTTK_MUZZLE_TRACE")) std::fprintf(stderr,"ttk-muzzle-settle from=%d,%d,%d to=%d,%d,%d effects=%u lights=%u\n",
        old[0],old[1],old[2],now[0],now[1],now[2],moved,lit);
}
static const char* muzzle_settle_debug_json() {
    static char b[320];
    std::snprintf(b,sizeof b,"{\"enabled\":%s,\"shots\":%llu,\"moves\":%llu,\"effects\":%llu,\"lights\":%llu,\"expired\":%llu,\"replays\":%llu,\"last\":[%.0f,%.0f,%.0f]}",
        muzzle_settle_enabled()?"true":"false",(unsigned long long)settle_shots,(unsigned long long)settle_moves,(unsigned long long)settle_fx_moved,
        (unsigned long long)settle_lights_moved,(unsigned long long)settle_expired,(unsigned long long)settle_replays.load(),settle_last[0],settle_last[1],settle_last[2]);
    return b;
}
