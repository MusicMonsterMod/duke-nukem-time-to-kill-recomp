// D08Z5 wall backflip (DNTTK_WALL_BACKFLIP=modern, the Modernized default;
// the launcher passes original for Vanilla).
// documentation/174-d08z5-wall-backflip.md.
//
// Original: the "backflip" off a car or a sloped obstacle is not a wall
// response (D08Z1's 107 never plays). A jump that lands on a face 40..70
// degrees steep (0x80077438: the floor normal 20..50 degrees above level)
// lands in mode 0, and the next ground step 0x80055e80 finds it too steep to
// stand on (0x800544e8; the only store of mode 2 is 0x80055fb4): mode 2 with
// the slip pose 145 when Duke faces up the slope, 143 when he faces down it
// (0x8005446c), and the slide sound 0x2006. The ground dispatcher runs the
// slide handler 0x80054568 for 143..146: Duke slides down the face, and when
// the floor ends under him (0x800789a8) 0x8003f060 starts the fall 108 (mode
// 9). Its flight start 0x8003e2d0 picks the launch from the table 0x800c27c8
// by the pose: 145 takes entry 10, 84 units straight back from Duke's facing,
// 143 entry 9, the same forwards, both with no rise, and rotates it into the
// velocity (0x80066e34, ra 0x8003e488): about 3040 velocity units down the
// slope. That launch carries him about 450..500 world units back over a fall
// off a car, after about 70 of slide.
//
// Modern: the slide stays (gravity on a face too steep to stand on), without
// the launch: for a fall started from the slide handler (ra 0x800546c4,
// 0x800546dc or 0x800546ec) the launch vector is zeroed before it is rotated,
// as the table's entry 0 does for other falls, so Duke drops where the slope
// ends. The slip pose and sound are the original's.
static bool wall_backflip_modern() {
    const char* v=ttk_live_env("DNTTK_WALL_BACKFLIP");
    return !v || std::strcmp(v,"original");
}
static unsigned wall_backflip_slides,wall_backflip_stops;
static bool wall_backflip_jump_pending;
// Entry of 0x8005446c right after the ground step's mode 2 store (ra
// 0x80055fc0): a slope slide starts this update. From the slide handler's
// jump branch (ra 0x800547fc) it picks that jump (below).
static void wall_backflip_slide_start(CPUState* cpu) {
    if(cpu->gpr[4]==player && cpu->gpr[31]==0x800547fc)wall_backflip_jump_pending=true;
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x80055fc0)return;
    ++wall_backflip_slides;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-wall-backflip slide anim=%u yaw=%u slope=%u v=%d,%d,%d\n",psx_mod_read_half(player+0x60),
            psx_mod_read_half(player+0x24),psx_mod_read_half(player+0x2bc),(int32_t)psx_mod_read_word(player+0x1f4),
            (int32_t)psx_mod_read_word(player+0x1f8),(int32_t)psx_mod_read_word(player+0x1fc));
}
static bool wall_backflip_fall_pending;
// Entry of 0x8003f060 (fall start): from the slide handler's three exits, the
// launch rotation below belongs to this fall.
static void wall_backflip_fall(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    wall_backflip_fall_pending=cpu->gpr[4]==player && (ra==0x800546c4 || ra==0x800546dc || ra==0x800546ec) &&
        wall_backflip_modern() && psx_mod_read_byte(player+0x22c)==2 && player_identity_ready();
}
// Entry of 0x80066e34 from the flight start 0x8003e2d0 (ra 0x8003e488): the
// table launch in Duke's frame at +0x1f4, about to be rotated into the velocity.
static void wall_backflip_launch(CPUState* cpu) {
    if(cpu->gpr[31]!=0x8003e488)return;
    const bool pending=wall_backflip_fall_pending;
    wall_backflip_fall_pending=false;
    if(!pending || cpu->gpr[5]!=player+0x1f4)return;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-wall-backflip stop anim=%u launch=%d,%d,%d\n",psx_mod_read_half(player+0x60),
            (int32_t)psx_mod_read_word(player+0x1f4),(int32_t)psx_mod_read_word(player+0x1f8),(int32_t)psx_mod_read_word(player+0x1fc));
    psx_mod_write_word(player+0x1f4,0);psx_mod_write_word(player+0x1f8,0);psx_mod_write_word(player+0x1fc,0);
    ++wall_backflip_stops;
}
// The backflip itself: Jump pressed while Duke slides (0x800547ac..0x80054834,
// Square word 0x800d1b88, no jetpack thrust) picks 99 (100 if the last jump
// was 99) when 0x8005446c finds Duke facing up the slope (more than 90
// degrees from +0x2bc), 98 when he faces down it. 99/100 fly backwards; the
// lower-body update then initializes it (0x800493a4, ra 0x8005a3a0 or
// 0x8005a4d0). The branch also runs after the handler's fall start in the
// same update, so the mode is the slide's 2 or already the fall's 9.
// Modern: that start becomes 98, the original's own jump off a slope, in
// Duke's facing. A standing jump's 99/100 (Down held at 0x8005492c) is left
// alone.
static unsigned wall_backflip_jumps,wall_backflip_climbs;
static bool wall_backflip_climb_pending;
static void wall_backflip_jump(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    if(cpu->gpr[4]!=player || (ra!=0x8005a3a0 && ra!=0x8005a4d0))return;
    const bool pending=wall_backflip_jump_pending;
    wall_backflip_jump_pending=false;
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(!pending || !wall_backflip_modern() || (animation!=99 && animation!=100) || psx_mod_read_half(player+0x68)!=0 ||
       !player_identity_ready())return;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-wall-backflip jump anim=%u mode=%u yaw=%u slope=%u\n",animation,psx_mod_read_byte(player+0x22c),psx_mod_read_half(player+0x24),psx_mod_read_half(player+0x2bc));
    psx_mod_write_half(player+0x60,98);psx_mod_write_half(player+0x23e,98);
    wall_backflip_climb_pending=true;
    ++wall_backflip_jumps;
}
// The original 98 from there goes nowhere: Duke takes off pressed into the
// face, so the airborne sweep 0x8007a98c reports it as floor (result 2) on
// the first update, even while he moves away from it, and the handler lands
// him with no rise (horizontal step only, 0x80054c04 picks a landing pose);
// the next ground step slides him again. The jump felt cancelled.
// Pressed as the slide reaches the bottom of the face, the handler has
// already started the fall and the original 98 is only a fall in the jump
// pose; Modern gives it the slope jump's rise.
// Modern, while that jump rises (98, mode 9, upward velocity):
// - first airborne update (the integration 0x8003ebf4 from the airborne
//   handler, ra 0x80055934): Duke keeps the jump's rise and drifts straight
//   up the slope (+0x2bc is downhill; +0x2be the face's tilt below level)
//   only so fast that the arc leaves the face: uphill speed * tan(face) =
//   k * rise. Gravity brings him back down on the face higher up (at best
//   4k(1-k) of the jump's height above the take-off), where it slides as
//   before: a well-timed jump gains a little height, as in Duke Nukem 3D;
// - every rising update: isolated original integrations and sweeps (as
//   D08Z1 does, restoring Duke). If the drift reports floor, this update
//   rises straight up, and if that does too, lifted by the smallest step (up
//   to 160 units) after which the original sweep finds the way free; the
//   drift resumes once clear. So the rise is not taken for a landing. Coming
//   down, the face lands and slides him as in the original.
// DNTTK_SLOPE_JUMP_K tunes k (diagnostics).
static double wall_backflip_climb_k() {
    const char* v=std::getenv("DNTTK_SLOPE_JUMP_K");
    const double k=v?std::atof(v):0.25;
    return k>0 && k<1 ? k : 0.25;
}
// The rise 98 takes from a slide (velocity units, up is negative).
static constexpr int32_t wall_backflip_rise=-6917;
static bool wall_backflip_climbing;
static uint32_t wall_backflip_drift[2];
static unsigned wall_backflip_climb_updates,wall_backflip_lifts;
static void wall_backflip_climb(CPUState* cpu) {
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x80055934)return;
    const bool pending=wall_backflip_climb_pending;
    wall_backflip_climb_pending=false;
    // From the bottom of the face the handler has already started the fall
    // (mode 9) and 98 then keeps the fall's velocity: give it the slope
    // jump's own rise.
    if(pending && psx_mod_read_half(player+0x60)==98 && psx_mod_read_byte(player+0x22c)==9 &&
       (int32_t)psx_mod_read_word(player+0x1f8)>=0)psx_mod_write_word(player+0x1f8,(uint32_t)wall_backflip_rise);
    const bool rising=psx_mod_read_half(player+0x60)==98 && psx_mod_read_byte(player+0x22c)==9 &&
        (int32_t)psx_mod_read_word(player+0x1f8)<0 && !(psx_mod_read_word(player)&0x20002) && player_identity_ready();
    if(pending && rising) {
        wall_backflip_climbing=true;wall_backflip_climb_updates=0;wall_backflip_drift[0]=wall_backflip_drift[1]=0;
        const int32_t vy=(int32_t)psx_mod_read_word(player+0x1f8);
        const int tilt=-(int16_t)psx_mod_read_half(player+0x2be);
        if(tilt>0 && tilt<1024) {
            constexpr double turn=6.283185307179586/4096;
            const double face=tilt*turn,uphill=((int16_t)psx_mod_read_half(player+0x2bc)+2048)*turn;
            const double speed=wall_backflip_climb_k()*-vy/std::tan(face);
            const int32_t vx=(int32_t)std::lround(speed*std::sin(uphill)),vz=(int32_t)std::lround(speed*std::cos(uphill));
            if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
                std::fprintf(stderr,"ttk-wall-backflip climb v=%d,%d,%d -> %d,%d,%d face=%.1f y=%d\n",(int32_t)psx_mod_read_word(player+0x1f4),vy,
                    (int32_t)psx_mod_read_word(player+0x1fc),vx,vy,vz,tilt*360.0/4096,(int32_t)psx_mod_read_word(player+8));
            psx_mod_write_word(player+0x1f4,(uint32_t)vx);psx_mod_write_word(player+0x1fc,(uint32_t)vz);
            wall_backflip_drift[0]=(uint32_t)vx;wall_backflip_drift[1]=(uint32_t)vz;
            ++wall_backflip_climbs;
        }
    }
    if(!wall_backflip_climbing)return;
    if(!rising || ++wall_backflip_climb_updates>40) {wall_backflip_climbing=false;return;}
    static uint32_t saved[0x8a4/4];
    for(unsigned i=0;i<0x8a4/4;++i)saved[i]=psx_mod_read_word(player+4*i);
    auto restore=[&]{for(unsigned i=0;i<0x8a4/4;++i)if(psx_mod_read_word(player+4*i)!=saved[i])psx_mod_write_word(player+4*i,saved[i]);};
    auto sweep=[&](bool drift,int32_t lift)->int32_t {
        int32_t ignored=0,result=-1;
        psx_mod_write_word(player+8,saved[8/4]-(uint32_t)lift);
        if(drift) {psx_mod_write_word(player+0x1f4,wall_backflip_drift[0]);psx_mod_write_word(player+0x1fc,wall_backflip_drift[1]);}
        else {psx_mod_write_word(player+0x1f4,0);psx_mod_write_word(player+0x1fc,0);}
        if(!original_call(cpu,0x8003ebf4,0,&ignored) || !original_call(cpu,0x8007a98c,0,&result))result=-1;
        restore();
        return result;
    };
    const bool trace=std::getenv("DNTTK_SLOPE_JUMP_TRACE");
    // The drift, straight up, then straight up lifted clear of the face.
    struct Try {bool drift;int32_t lift;};
    for(const Try t:{Try{true,0},Try{false,0},Try{false,24},Try{false,48},Try{false,96},Try{false,160}}) {
        const int32_t result=sweep(t.drift,t.lift);
        if(trace)std::fprintf(stderr,"ttk-wall-backflip sweep drift=%d lift=%d -> %d y=%d vy=%d update=%u\n",t.drift,t.lift,result,(int32_t)saved[8/4],(int32_t)saved[0x1f8/4],wall_backflip_climb_updates);
        if(result==2)continue;
        if(t.drift && t.lift==0)return;
        psx_mod_write_word(player+0x1f4,t.drift?wall_backflip_drift[0]:0);psx_mod_write_word(player+0x1fc,t.drift?wall_backflip_drift[1]:0);
        if(t.lift) {psx_mod_write_word(player+8,saved[8/4]-(uint32_t)t.lift);++wall_backflip_lifts;}
        return;
    }
}
static const char* wall_backflip_debug_json() {
    static char buffer[160];
    std::snprintf(buffer,sizeof buffer,"{\"modern\":%s,\"slides\":%u,\"stops\":%u,\"jumps\":%u,\"climbs\":%u,\"lifts\":%u}",wall_backflip_modern()?"true":"false",
        wall_backflip_slides,wall_backflip_stops,wall_backflip_jumps,wall_backflip_climbs,wall_backflip_lifts);
    return buffer;
}
