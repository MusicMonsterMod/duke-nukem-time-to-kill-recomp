// D10F1 clear lens (Modernized independent camera, third person only).
//
// The native solve (0x8003aa48) clears a 512 x 240 window at the anchor, but
// 0x80039c7c puts the lens camera[+0x42]/2 (193) behind that window, and the
// 768 minimum pushes a compressed anchor back out along Duke's heading,
// through what it hit (documentation/379-d10f-close-surface-camera.md).
// After that solve, at the follow easing (0x8002a038 from 0x8003ade4, the
// view-space delta at sp+0x18), this takes the camera along the orbit's own
// axis instead:
// - the lens is traced from the pivot with the original segment query
//   (0x8006d980) to its preferred point and a small ring around it; the
//   nearest world hit, less a margin, shortens the whole boom toward the
//   pivot (the shoulder shrinks with it). An actor the native draw will fade
//   from that lens (its root nearer than 1024 in depth, 0x80034e18: crates,
//   breakables) is passed over, so that fade still handles it; a large one
//   (the apartment wardrobe, root far from its doors) stops the lens;
// - inward changes are immediate; outward ones hold briefly, then ease in
//   host time, so a doorway clearing cannot pop the camera across a room;
// - the state's native follow lag (rates at 0x800c0d4c) is kept; the lens it
//   leaves is traced again and pulled in front of any world hit;
// - hanging from monkey bars (D08J4), the grates are objects the query does
//   not report: the lens stays below the hanging grate's plane, closer than
//   the floor if it must, so looking down shows the floor and not the grill.
// The result goes to the easing as one view-space delta at rate 1. Native
// fades, room/height queries, the saved distance and shoulder, yaw/pitch and
// aim are unchanged. Developer A/B: DNTTK_CLEAR_LENS=0 keeps the native solve.
struct ClearLens {
    bool valid=false,live=false;
    uint64_t solved=0;                // camera update (cameras) last solved
    double lens=0,goal=0,reach=0,ms=0,hold_ms=0;
    int32_t view[3]{};
    // The last probe's inputs and result: an unchanged camera reuses it for
    // one update (each original query costs about 0.2 ms here; a door that
    // closes is seen one update later).
    int32_t key[8]{};double clear=1;unsigned reused=0;
    // Actors this update found to stop the lens: later rays skip the back trace.
    uint32_t blockers[4]{};unsigned blocker_count=0;
    unsigned updates=0,probes=0,hits=0,actors=0,pulls=0,holds=0,declines=0,reuses=0;
    uint64_t ns=0,peak_ns=0;          // solve time (diagnostics)
};
static ClearLens clear_lens;
static bool clear_lens_enabled() {
    static const bool enabled=[] {const char* t=std::getenv("DNTTK_CLEAR_LENS");return !t || std::strcmp(t,"0");}();
    return enabled;
}
// Developer tuning (lens distances in world units, times in milliseconds).
static double clear_lens_param(const char* name,double fallback,double lo,double hi) {
    const char* t=std::getenv(name);char* end=nullptr;const double v=t?std::strtod(t,&end):fallback;
    return t && end && !*end && std::isfinite(v) && v>=lo && v<=hi ? v : fallback;
}
static double clear_lens_floor() {static const double v=clear_lens_param("DNTTK_CAMERA_FLOOR",450,96,1600);return v;}
static double clear_lens_hold() {static const double v=clear_lens_param("DNTTK_CAMERA_RETURN_HOLD",100,0,2000);return v;}
static double clear_lens_return() {static const double v=clear_lens_param("DNTTK_CAMERA_RETURN",350,0,4000);return v;}
static constexpr double clear_lens_ring=48, clear_lens_margin=48;
// Native actor fade depth (0x400) less a band for the root joint's offset.
static constexpr double clear_lens_fade_depth=896;
// Monkey bars: the lens keeps this far below the grate origin (D08J4 grates
// sit about 240 above the pivot), and never nearer the pivot than the minimum.
static constexpr double clear_lens_grate_gap=96, clear_lens_minimum=96;
// D08J3A: attached to a climb (ladder, pole, chain, climbing wall), a ledge
// hang or shimmy, or the monkey bars, the climbed surface is within a body
// width of the pivot. Viewed from that side, a lens held at the ground floor
// (450) sits behind it: the frame showed only the surface, Duke hidden. There
// the floor is the minimum, so the lens stays in front of the surface and the
// native fade covers Duke when it comes close.
static bool clear_lens_attached() {
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    return climb_state_early() || ceiling_hang_state() || ((mode==6 || mode==7) && mantle_state_early());
}
static double clear_lens_attached_floor() {static const double v=clear_lens_param("DNTTK_CLIMB_CAMERA_FLOOR",clear_lens_minimum,clear_lens_minimum,1600);return v;}
// Fraction of from->to that stays below the hanging grate; 1 off the bars.
static double clear_lens_under(const double* from,const double* to) {
    if(!ceiling_hang_state()) return 1;
    const uint32_t grate=psx_mod_read_word(player+0x17c);
    if(grate<0x80010000u || grate>0x801ffff0u) return 1;
    const double plane=(int32_t)psx_mod_read_word(grate+8)+clear_lens_grate_gap; // y grows downward
    if(to[1]>=plane || from[1]<=plane) return 1;
    return (from[1]-plane)/(from[1]-to[1]);
}
// Fraction of from->to clear for a lens at to; -1 when the query is
// unavailable. An actor stops the query; one the native draw fades from to
// is passed over (the query restarts just past it, at most three times).
static bool clear_lens_trace_on() {static const bool on=std::getenv("DNTTK_CLEAR_LENS_TRACE")!=nullptr;return on;}
static double clear_lens_fraction(CPUState* cpu,const double* from,const double* to) {
    const double length=std::sqrt((to[0]-from[0])*(to[0]-from[0])+(to[1]-from[1])*(to[1]-from[1])+(to[2]-from[2])*(to[2]-from[2]));
    if(length<1) return 1;
    double start[3]={from[0],from[1],from[2]};
    uint32_t ignore=0;
    for(int pass=0;pass<4;++pass) {
        double hit[3];int kind=0;uint32_t actor=0;
        if(!camera_segment_query(cpu,start,to,hit,kind,ignore,&actor)) return -1;
        ++clear_lens.probes;
        if(kind==0) return 1;
        double along=0;
        for(int i=0;i<3;++i) along+=(hit[i]-from[i])*(to[i]-from[i])/length;
        if(kind==1) {++clear_lens.hits;return std::clamp(along/length,0.0,1.0);}
        double depth=clear_lens_fade_depth;
        if(actor) {depth=0;for(int i=0;i<3;++i) depth+=((int32_t)psx_mod_read_word(actor+4+4*i)-to[i])*(from[i]-to[i])/length;}
        if(clear_lens_trace_on())
            std::fprintf(stderr,"[clear lens] actor %08x type %u at %.0f/%.0f depth %.0f\n",actor,actor?psx_mod_read_half(actor+0x2c):0,along,length,depth);
        // The lens must also be outside it: traced back from the lens, the
        // same actor's far side comes first, a margin away (a lens inside a
        // large model such as the wardrobe draws its inside, black).
        bool outside=false;
        const bool known=std::find(clear_lens.blockers,clear_lens.blockers+clear_lens.blocker_count,actor)!=clear_lens.blockers+clear_lens.blocker_count;
        if(depth<clear_lens_fade_depth && !known) {
            double back[3];int back_kind=0;uint32_t back_actor=0;
            if(!camera_segment_query(cpu,to,from,back,back_kind,0,&back_actor)) return -1;
            ++clear_lens.probes;
            double gap=0;for(int i=0;i<3;++i) gap+=(back[i]-to[i])*(back[i]-to[i]);
            outside=back_kind==2 && back_actor==actor && std::sqrt(gap)>=clear_lens_margin;
            if(clear_lens_trace_on())
                std::fprintf(stderr,"[clear lens]   back kind %d actor %08x gap %.0f\n",back_kind,back_actor,std::sqrt(gap));
        }
        if(!outside) {
            if(actor && !known && clear_lens.blocker_count<4) clear_lens.blockers[clear_lens.blocker_count++]=actor;
            ++clear_lens.hits;return std::clamp(along/length,0.0,1.0);
        }
        ++clear_lens.actors;
        if(along+16>=length) return 1;
        ignore=actor;
        for(int i=0;i<3;++i) start[i]=from[i]+(to[i]-from[i])*(along+16)/length;
    }
    return 1;
}
// Native easing 0x8002a038(delta, rate).
static int32_t clear_lens_ease(int32_t delta,int32_t rate) {
    if(rate<=0) return delta;
    if(std::abs(delta)<rate) return delta;
    if(delta==rate) return 1;
    return int32_t((int64_t(delta)*4096/rate)>>12);
}
// Once per camera update, at its first easing call. sp: the 0x8003ade4 frame.
static bool clear_lens_solve(CPUState* cpu,uint32_t sp) {
    clear_lens.blocker_count=0;
    double target[3];
    if(!pivot(target)) return false;
    const double half=(int16_t)psx_mod_read_half(camera+0x42)/2.0;
    if(half<16 || half>1024) return false;
    // View rows as the easing uses them (orbit_constraint wrote them).
    double row[3][3];
    for(int r=0;r<3;++r)for(int k=0;k<3;++k) row[r][k]=(int16_t)psx_mod_read_half(camera+6*r+2*k)/4096.0;
    const double* forward=row[2];
    double current[3];
    for(int i=0;i<3;++i) current[i]=(int32_t)psx_mod_read_word(sp+0x28+4*i);
    // Preferred lens: the D10 boom and shoulder behind the pivot.
    double lens[3];
    for(int i=0;i<3;++i) lens[i]=target[i]-forward[i]*(radius+half)+row[0][i]*shoulder_offset;
    double reach=0;
    for(int i=0;i<3;++i) reach+=(lens[i]-target[i])*(lens[i]-target[i]);
    reach=std::sqrt(reach);
    if(reach<1) return false;
    // The lens and a ring beside it (both sides, and the vertical side away
    // from the pivot: a ceiling looking down, the floor looking up).
    const int32_t key[8]={(int32_t)target[0],(int32_t)target[1],(int32_t)target[2],
        (int32_t)psx_mod_read_half(camera+12),(int32_t)psx_mod_read_half(camera+14),(int32_t)psx_mod_read_half(camera+16),
        (int32_t)std::lround(radius),(int32_t)std::lround(shoulder_offset)};
    double clear=1;
    if(clear_lens.valid && std::equal(key,key+8,clear_lens.key) && clear_lens.reused<1) {
        clear=clear_lens.clear;++clear_lens.reused;++clear_lens.reuses;
    } else {
        const double up=lens[1]<target[1]?-1:1; // row[1] points down the view
        const double ring[4][2]={{0,0},{1,0},{-1,0},{0,up}};
        for(const auto& o:ring) {
            double point[3];
            for(int i=0;i<3;++i) point[i]=lens[i]+(row[0][i]*o[0]+row[1][i]*o[1])*clear_lens_ring;
            const double t=clear_lens_fraction(cpu,target,point);
            if(t<0) return false;
            clear=std::min(clear,t);
        }
        std::copy(key,key+8,clear_lens.key);clear_lens.clear=clear;clear_lens.reused=0;
    }
    const double floor=std::min(clear_lens_attached()?clear_lens_attached_floor():clear_lens_floor(),reach);
    double goal=std::clamp(clear*reach-(clear<1?clear_lens_margin:0),floor,reach);
    const double under=clear_lens_under(target,lens);
    if(under<1) goal=std::min(goal,std::max(clear_lens_minimum,under*reach));
    const double now=input_clock_ms();
    const double dt=clear_lens.valid?std::clamp(now-clear_lens.ms,0.0,250.0):0;
    clear_lens.ms=now;
    // The preferred distance itself may change (Alt-wheel, menu): follow it
    // while unobstructed, as the native camera does.
    if(!clear_lens.valid || goal<=clear_lens.lens) {
        if(clear_lens.valid && goal<clear_lens.lens-1) {clear_lens.hold_ms=now+clear_lens_hold();++clear_lens.holds;}
        clear_lens.lens=goal;
    } else if(now>=clear_lens.hold_ms) {
        const double tau=clear_lens_return()/3;
        clear_lens.lens+=tau>0?(goal-clear_lens.lens)*(1-std::exp(-dt/tau)):goal-clear_lens.lens;
        if(goal-clear_lens.lens<1) clear_lens.lens=goal;
    }
    if(clear_lens.lens>=reach-1 && goal>=reach-1) clear_lens.lens=reach; // unobstructed: the native boom
    clear_lens.valid=true;clear_lens.goal=goal;clear_lens.reach=reach;
    // Anchor whose lens lies on the pivot->preferred-lens ray at that distance.
    const double scale=clear_lens.lens/reach;
    double anchor[3];
    for(int i=0;i<3;++i) anchor[i]=target[i]+(lens[i]-target[i])*scale+forward[i]*half;
    // Native follow lag, per view axis, from this state's rates.
    const uint32_t rates=0x800c0d4c+8*(cpu->gpr[19]&0xff);
    double eased[3],delta[3]={};
    for(int r=0;r<3;++r) {
        double v=0;for(int k=0;k<3;++k) v+=row[r][k]*(anchor[k]-current[k]);
        eased[r]=clear_lens_ease((int32_t)std::lround(v),(int16_t)psx_mod_read_half(rates+2*r));
    }
    for(int k=0;k<3;++k) for(int r=0;r<3;++r) delta[k]+=row[r][k]*eased[r];
    // The lens the lag leaves must be clear as well. Within the probed ring
    // and no farther out than the cleared lens, it already is.
    double placed[3],length=0,off=0;
    for(int i=0;i<3;++i) {
        placed[i]=current[i]+delta[i]-forward[i]*half;length+=(placed[i]-target[i])*(placed[i]-target[i]);
        const double aimed=target[i]+(lens[i]-target[i])*scale;
        off+=(placed[i]-aimed)*(placed[i]-aimed);
    }
    length=std::sqrt(length);
    const bool covered=std::sqrt(off)<clear_lens_ring/2 && length<=clear_lens.lens+1;
    const double placed_under=clear_lens_under(target,placed);
    if((length>floor && !covered) || placed_under<1) {
        double t=length>floor && !covered?clear_lens_fraction(cpu,target,placed):1;
        if(t<0) return false;
        if(t<1) t=std::max(floor,t*length-clear_lens_margin)/length;
        if(placed_under<1) t=std::min(t,std::max(clear_lens_minimum,placed_under*length)/length);
        if(t<1) {
            const double keep=t;
            if(keep<1) {
                for(int i=0;i<3;++i) delta[i]=target[i]+(placed[i]-target[i])*keep+forward[i]*half-current[i];
                clear_lens.lens=std::min(clear_lens.lens,keep*length);clear_lens.hold_ms=now+clear_lens_hold();
                ++clear_lens.pulls;
            }
        }
    }
    for(int r=0;r<3;++r) {
        double v=0;for(int k=0;k<3;++k) v+=row[r][k]*delta[k];
        clear_lens.view[r]=(int32_t)std::lround(v);
        psx_mod_write_word(sp+0x18+4*r,(uint32_t)clear_lens.view[r]);
    }
    ++clear_lens.updates;
    return true;
}
// 0x8002a038 entry from 0x8003ade4's three easing calls; the caller has
// checked the third-person orbit lease.
static void clear_lens_follow(CPUState* cpu,uint32_t ra,uint32_t sp) {
    const int axis=ra==0x8003af40?0:ra==0x8003af60?1:ra==0x8003af80?2:-1;
    if(axis<0 || sp!=orbit_sp-0xb8 || constraint_update!=cameras) return;
    if(clear_lens.solved!=cameras) {
        // A gap (first person, a lost lease, a load) starts from the preferred boom.
        if(clear_lens.solved+1!=cameras) clear_lens.valid=false;
        clear_lens.solved=cameras;
        const auto t0=std::chrono::steady_clock::now();
        clear_lens.live=clear_lens_solve(cpu,sp);
        const uint64_t ns=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-t0).count();
        clear_lens.ns+=ns;clear_lens.peak_ns=std::max(clear_lens.peak_ns,ns);
        if(!clear_lens.live) {++clear_lens.declines;clear_lens.valid=false;}
    }
    if(!clear_lens.live) return;
    cpu->gpr[4]=(uint32_t)clear_lens.view[axis];cpu->gpr[5]=1;
}
static const char* clear_lens_debug_json() {
    static char text[416];
    std::snprintf(text,sizeof text,"{\"enabled\":%s,\"live\":%s,\"lens\":%.1f,\"goal\":%.1f,\"reach\":%.1f,\"floor\":%.0f,\"attached\":%s,\"updates\":%u,\"probes\":%u,\"hits\":%u,\"actors\":%u,\"pulls\":%u,\"holds\":%u,\"declines\":%u,\"reuses\":%u,\"us\":%llu,\"peak_us\":%llu}",
        clear_lens_enabled()?"true":"false",clear_lens.live?"true":"false",clear_lens.lens,clear_lens.goal,clear_lens.reach,clear_lens_floor(),clear_lens_attached()?"true":"false",
        clear_lens.updates,clear_lens.probes,clear_lens.hits,clear_lens.actors,clear_lens.pulls,clear_lens.holds,clear_lens.declines,clear_lens.reuses,
        (unsigned long long)(clear_lens.ns/1000),(unsigned long long)(clear_lens.peak_ns/1000));
    return text;
}
