// D17BF2/D17BF1 far hits (Modernized "extended" draw distance; Vanilla and
// "original" are unchanged). Included after sleeping_enemies.inc. Note 564.
//
// Extended draw distance shows enemies far past where the original let them
// exist (D17AQ keeps real enemies out to the extended limit, D17BF draws the
// rest asleep). Two things stopped Duke reaching them:
//
// 1. Round range. Rounds move in 0x8006ed5c: each update adds the step (+0x46,
//    speed +0x44 times the frame scale 0x800d21fc) to the travel counter +0x40
//    and ends the round once the counter passes its kind's range, word +0x1c of
//    the projectile type table 0x800c4bc0 (64-byte entries, kind byte +0x30):
//    32767 for bullets, 25000 for several others, 3072 for flames. The counter
//    does nothing else (every type's falloff distance +0x20 is 0, so damage
//    never depends on it). Rocket kinds move in 0x80070568, which has no range
//    check (only the life +0x32). For Duke's own rounds of a kind whose range
//    is at least 25000, the round sweep 0x8006dd2c, called from 0x8006ed5c
//    (ra 0x8006efc0) after that update's range check, takes the part of this
//    update's step that would exceed the far range out of the counter again:
//    it grows by step * range / far, so the round's own range is used up over
//    the far range: the extended view limit times 1.16 (a target at the edge
//    of a 60-degree half view is 1/cos(30) farther than the view depth). Speed,
//    spread, damage and the hit tests are the original's; enemy rounds and
//    short kinds (flames) keep their range.
// 2. Sleeping copies (D17BF) had no body. Before any of Duke's projectiles is
//    swept (rounds, rockets, bolts: 0x8006dd2c from 0x8006ed5c, 0x80070568 and
//    0x8006ddb8), its path this update is tested against each copy; a path
//    that passes within reach of one creates the real enemy there first, with
//    the creation the spawner itself uses (0x80098f3c on the copy's row), so
//    the original sweep that follows meets it and hits it with its own model
//    test, walls and damage. At the cap (0x800c56a4 of 0x800c56a8) the
//    farthest dormant enemy that is not hurt or held by a script gives way
//    (0x80098d70, the removal the replacement uses); if none can, the round
//    passes and the copy sleeps on. A copy made real by a near miss is an
//    ordinary dormant D17AQ enemy (it stands still and may give way again);
//    one the round hurts wakes for good (enemy_reach.inc keeps it).
// Developer: DNTTK_ROUND_RANGE=original keeps the original range;
// DNTTK_FAR_WAKE=0 leaves copies untouchable; console "farhits".
static constexpr uint32_t projectile_table=0x800da978,projectile_end=0x800dcd78,projectile_types=0x800c4bc0;
static const DistanceGuard far_guards[]={
    {0x8006ed5c,0xc0,"b87345c23d52f862d1b5ade375c84be0afb4fd3718f023564695f9371bccd2f9"},   // the round update's range check
    {0x8006efa0,0x30,"bcee69d8d6c62aaca61bb1a5397ef1a9c340dfd994b3f3358226ce946d4a406b"},   // its sweep call
    {0x80098f3c,0x40,"ae43313fa6f82a0c9ecc9e7dc1c3040d88f8846f592242e49654a14b2046a154"},   // spawner creation
    {0x80098d70,0x40,"cd1f161338f229d66741ebc56f66ddd01fbef9b36323fe6a97275e6fead35842"},   // spawner removal
};
static std::array<std::vector<uint32_t>,4> far_words;
static std::atomic<int> far_override{-1};   // console: -1 default, 0 off, 1 on
static uint64_t far_stretched=0,far_rounds=0,far_wakes=0,far_wake_fail=0,far_wake_full=0,far_wake_yield=0,far_tests=0,far_refusals=0;
static int32_t far_last_range=0;
static uint32_t far_last_yield=0;
static bool far_range_enabled() {
    static const bool v=[]{const char* t=std::getenv("DNTTK_ROUND_RANGE");return !t || std::strcmp(t,"original");}();
    const int o=far_override.load(std::memory_order_relaxed);
    return (o<0?v:o!=0) && input_modernized() && draw_distance_extended();
}
static bool far_wake_enabled() {
    static const bool v=[]{const char* t=std::getenv("DNTTK_FAR_WAKE");return !t || std::strcmp(t,"0");}();
    const int o=far_override.load(std::memory_order_relaxed);
    return (o<0?v:o!=0) && sleeping_live() && enemy_reach_live();
}
static bool far_projectile(uint32_t p) {
    return p>=projectile_table && p<projectile_end && !((p-projectile_table)%0x60) && psx_mod_read_word(p+0x10)==player;
}
// The far range: the extended view limit, out to the edge of the view.
static int32_t far_range() {
    const double limit=reach_extended();
    return (int32_t)std::lround((limit>0?limit:65535.0)*1.16);
}
static void far_stretch(uint32_t p) {
    const uint32_t kind=psx_mod_read_byte(p+0x30);
    if(kind>=0x25) return;
    const int32_t range=(int32_t)psx_mod_read_word(projectile_types+64*kind+0x1c),far=far_range();
    if(range<25000 || range>=far) return;
    const int32_t step=psx_mod_read_half(p+0x46),travel=(int32_t)psx_mod_read_word(p+0x40);
    if(step<=0 || travel<step) return;
    // This update added step; keep step*range/far of it.
    const int32_t keep=(int32_t)((int64_t)step*range/far);
    psx_mod_write_word(p+0x40,(uint32_t)(travel-step+keep));
    far_last_range=far;++far_stretched;
}
// Shortest distance between segment p0-p1 and the vertical segment
// x=c[0], z=c[2], y in [ylo,yhi]; squared.
static double far_segment_column2(const double p0[3],const double p1[3],const double c[3],double ylo,double yhi) {
    // Sample-free: minimise over t in [0,1] the horizontal distance, then the
    // vertical gap at that t; refine with a few golden-section steps (the
    // function is convex in t for a column, the clamp keeps it unimodal).
    auto f=[&](double t){
        double q[3];for(int k=0;k<3;++k) q[k]=p0[k]+(p1[k]-p0[k])*t;
        const double dx=q[0]-c[0],dz=q[2]-c[2];
        const double dy=q[1]<ylo?ylo-q[1]:q[1]>yhi?q[1]-yhi:0;
        return dx*dx+dz*dz+dy*dy;
    };
    double a=0,b=1;const double g=0.6180339887498949;
    double x1=b-g*(b-a),x2=a+g*(b-a),f1=f(x1),f2=f(x2);
    for(int i=0;i<40;++i) {
        if(f1<f2) {b=x2;x2=x1;f2=f1;x1=b-g*(b-a);f1=f(x1);}
        else {a=x1;x1=x2;f1=f2;x2=a+g*(b-a);f2=f(x2);}
    }
    return std::min({f(0),f(1),f1,f2});
}
static bool far_wake_row(CPUState* cpu,uint32_t row) {
    const uint32_t duke=player+4;far_last_yield=0;
    if((int32_t)(int16_t)psx_mod_read_half(0x800c56a4)>=(int32_t)(int16_t)psx_mod_read_half(0x800c56a8)) {
        const uint32_t other=reach_yielding(duke,true);
        if(!other || other==row) {++far_wake_full;return false;}
        if(!cheat_call(cpu,0x80098d70,other) || (psx_mod_read_word(other+0x14)&1)) {++far_wake_fail;return false;}
        ++far_wake_yield;far_last_yield=other;
    }
    const uint32_t before=psx_mod_read_word(row+0x14);
    if(!cheat_call(cpu,0x80098f3c,row) || !(psx_mod_read_word(row+0x14)&1)) {
        psx_mod_write_word(row+0x14,before&~0x10u);   // a failed creation never disables the spawner for good
        ++far_wake_fail;return false;
    }
    const uint32_t actor=psx_mod_read_word(row+4);
    if(actor<0x80010000u || actor>0x801ffc00u || (actor&3)) {++far_wake_fail;return false;}
    reach_note_created(row,actor);
    ++far_wakes;
    if(far_wakes<=64) std::fprintf(stderr,"[TTK far hits] wake row %08x (gave way: %08x) flags %x->%x frame %u projectile %08x type %u actor %08x at %d,%d,%d\n",row,far_last_yield,before,psx_mod_read_word(row+0x14),psx_mod_read_word(0x800bcba4),cpu->gpr[4],psx_mod_read_word(row),actor,
        (int32_t)psx_mod_read_word(actor+4),(int32_t)psx_mod_read_word(actor+8),(int32_t)psx_mod_read_word(actor+12));
    return true;
}
static void far_wake(CPUState* cpu,uint32_t p) {
    const uint32_t n=sleep_table?psx_mod_read_word(sleep_table):0;
    if(!n || n>sleep_max) return;
    double p0[3],p1[3];
    for(int k=0;k<3;++k) {p0[k]=(int32_t)psx_mod_read_word(p+0x14+4*k);p1[k]=(int32_t)psx_mod_read_word(p+4+4*k);}
    // The path this update, lengthened by half a step at each end: the copy's
    // reach is generous, the original sweep decides the actual hit.
    double lo[3],hi[3];
    for(int k=0;k<3;++k) {lo[k]=std::min(p0[k],p1[k]);hi[k]=std::max(p0[k],p1[k]);}
    unsigned woke=0;
    for(uint32_t i=0;i<n && woke<2;++i) {
        const uint32_t e=sleep_table+16+16*i,row=psx_mod_read_word(e),a=psx_mod_read_word(e+4);
        if(!sleep_ram(row,0x30) || !a || (psx_mod_read_word(row+0x14)&0x17)) continue;
        const double rad=std::max<double>(256,(int16_t)psx_mod_read_half(a+0x5a));
        double c[3];for(int k=0;k<3;++k) c[k]=(int32_t)psx_mod_read_word(a+4+4*k);
        // Body column: from the actor's position up two radii (y grows down)
        // and half a radius below; reach 1.25 radii.
        const double ylo=c[1]-2*rad,yhi=c[1]+0.5*rad,reach=1.25*rad;
        if(c[0]+reach<lo[0] || c[0]-reach>hi[0] || c[2]+reach<lo[2] || c[2]-reach>hi[2] || yhi+reach<lo[1] || ylo-reach>hi[1]) continue;
        ++far_tests;
        if(far_segment_column2(p0,p1,c,ylo,yhi)>reach*reach) continue;
        if(far_wake_row(cpu,row)) ++woke;
    }
}
// Round sweep entry (a0 the projectile).
static void far_hits_sweep(CPUState* cpu) {
    const uint32_t p=cpu->gpr[4],ra=cpu->gpr[31];
    if(psx_mod_replay_active() || guest_busy || g_precise_mode || g_ls_mode || g_psx_call_bail || !far_projectile(p)) return;
    const bool round=ra==0x8006efc0u,rocket=ra==0x8007067cu,other=ra==0x8006dec4u;
    if(!round && !rocket && !other) return;
    if(!code_identity(far_guards,far_words)) {++far_refusals;return;}
    if(round) {++far_rounds;if(far_range_enabled()) far_stretch(p);}
    if(far_wake_enabled() && psx_mod_read_word(0x800c27bc)==1) far_wake(cpu,p);
}
// Developer "farhits probe": which copies Duke can see from where he stands
// (the game's own segment query from his eye to each copy's body), written to
// the log (row:distance:clear) with the count in the debug JSON.
static std::atomic<bool> far_probe_request{false};
static char far_probe_text[1024]="";
static unsigned far_probe_clear=0;
static void far_probe(CPUState* cpu) {
    if(!far_probe_request.exchange(false)) return;
    std::string out;unsigned clear=0;
    const uint32_t n=sleep_table?psx_mod_read_word(sleep_table):0;
    double eye[3];for(int k=0;k<3;++k) eye[k]=(int32_t)psx_mod_read_word(player+4+4*k);
    eye[1]-=1000;
    for(uint32_t i=0;i<n && i<sleep_max;++i) {
        const uint32_t e=sleep_table+16+16*i,row=psx_mod_read_word(e),a=psx_mod_read_word(e+4);
        if(!sleep_ram(row,0x30) || !a) continue;
        const double rad=std::max<double>(256,(int16_t)psx_mod_read_half(a+0x5a));
        double c[3],hit[3];for(int k=0;k<3;++k) c[k]=(int32_t)psx_mod_read_word(a+4+4*k);
        c[1]-=rad;
        int kind=0;
        const bool ok=atlas_segment_query(cpu,eye,c,hit,kind);
        double d=0,h=0;for(int k=0;k<3;++k) {d+=(c[k]-eye[k])*(c[k]-eye[k]);h+=(c[k]-hit[k])*(c[k]-hit[k]);}
        const bool seen=ok && (kind==0 || h<=rad*rad*4);
        clear+=seen;
        char item[48];std::snprintf(item,sizeof item,"%s%u:%.0f:%d",out.empty()?"":";",(row-psx_mod_read_word(0x800de720))/0x30,std::sqrt(d),seen?1:0);
        if(out.size()+std::strlen(item)<sizeof far_probe_text-1) out+=item;
    }
    std::snprintf(far_probe_text,sizeof far_probe_text,"%s",out.c_str());far_probe_clear=clear;
    std::fprintf(stderr,"[TTK far hits] probe from %.0f,%.0f,%.0f: %u of %u copies in clear view: %s\n",eye[0],eye[1],eye[2],clear,n,far_probe_text);
}
static bool far_hits_console(const char* line,void (*say)(const char*)) {
    if(!std::strcmp(line,"farhits probe")) {far_probe_request.store(true);say("Far hits: probing the sleeping enemies in view (see the log)");return true;}
    if(std::strcmp(line,"farhits") && std::strncmp(line,"farhits ",8)) return false;
    const char* v=line[7]?line+8:"";
    if(!std::strcmp(v,"on")) far_override.store(1);
    else if(!std::strcmp(v,"off")) far_override.store(0);
    else if(!std::strcmp(v,"default")) far_override.store(-1);
    else if(*v) {say("Usage: farhits [on|off|default] (Duke's rounds reach the far view; far sleeping enemies wake when hit; session only)");return true;}
    char text[200];
    std::snprintf(text,sizeof text,"Far hits %s: round range %d, %llu far wakes (session only). Default on with Extended draw distance.",
        far_range_enabled()||far_wake_enabled()?"on":"off",far_range_enabled()?far_range():0,(unsigned long long)far_wakes);
    say(text);return true;
}
static const char* far_hits_debug_json() {
    static char buffer[320];
    std::snprintf(buffer,sizeof buffer,"{\"range\":%s,\"wake\":%s,\"far_range\":%d,\"rounds\":%llu,\"stretched\":%llu,\"tests\":%llu,\"wakes\":%llu,\"yield\":%llu,\"full\":%llu,\"fail\":%llu,\"refusals\":%llu,\"probe_clear\":%u}",
        far_range_enabled()?"true":"false",far_wake_enabled()?"true":"false",far_last_range,(unsigned long long)far_rounds,(unsigned long long)far_stretched,
        (unsigned long long)far_tests,(unsigned long long)far_wakes,(unsigned long long)far_wake_yield,(unsigned long long)far_wake_full,(unsigned long long)far_wake_fail,(unsigned long long)far_refusals,far_probe_clear);
    return buffer;
}
