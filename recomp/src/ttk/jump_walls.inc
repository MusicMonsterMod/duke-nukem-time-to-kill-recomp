// D08Z1 jump wall contact (Modernized, DNTTK_JUMP_WALLS=original restores the
// original). documentation/134-d08z1-jump-wall-slide.md.
// Original: the airborne handler 0x80055904 integrates the flight
// (0x8003ebf4: delta +0xfc/+0xfe/+0x100 from the velocity +0x1f4/+0x1f8/+0x1fc)
// and sweeps it (0x8007a98c). A wall (result 1, finalizer 0x80079f4c: normal
// +0x194, reflected direction +0x1b4, angle +0x1c0 between the reversed
// horizontal motion and the wall normal, 0x400 = 90 degrees) within 45 degrees
// of head-on, or moving away, is the bounce: 0x8003ef08 zeroes a rising
// vertical velocity, sends Duke back along the reflection at half speed and
// plays 107 (rumble, sound 0x2008). 45..90 degrees deflects him away along the
// reflection at 0.25..0.75 of his speed (0x8003ef78). A ceiling hit while
// rising (result 3) stops the rise and rescales the horizontal speed by that
// stale angle (0x8003efd0), or bounces (107) off a steep one. Either way the
// jump's travel is gone.
// Modernized (EDuke32 clipmove feel): before each airborne update the host
// runs the original integration and sweep in isolation on a copy of Duke's
// state (restored afterwards, as the D08X reach retries do). If they report a
// wall, only the velocity component into the wall is removed; on a ceiling
// while rising the rise stops and the horizontal speed stays. Up to three
// passes resolve a corner (a second wall); one still in the way then stops the
// horizontal travel. The original update then runs with that velocity and its
// own sweep checks the slide, so Duke keeps his height, arc and the motion
// along the wall, with no bounce, stagger, rumble or contact sound. A hit the
// pre-pass cannot resolve (moving away from the wall, an unknown result) is
// left to the original. Ledge grabs and mantles that started on the first
// updates of a bounce use the recent contact instead (jump_wall_recent()).
static bool jump_walls_slide() {
    static const bool original=[]{const char* v=std::getenv("DNTTK_JUMP_WALLS");return v && !std::strcmp(v,"original");}();
    return !original;
}
static unsigned jump_wall_slides,jump_wall_corners,jump_wall_corner_turns,jump_wall_catches,jump_wall_stops,jump_ceiling_stops,jump_wall_unsolved,jump_wall_passes,jump_wall_nudges;
// Outward drift left after a slide (velocity units: about 1.4 world units per
// 30 Hz update), so a slide along a wall does not keep touching it.
static constexpr double wall_separation=128.0;
// A corner turn always moves at least this fast (velocity units), so even a
// 90-degree turn carries Duke past the edge instead of hanging on it.
static constexpr double corner_min_speed=1200.0;
// The velocity and flight timer the pre-pass left for this update's original
// integration, for the safety net below.
static double jump_wall_last_vx,jump_wall_last_vz;
static int32_t jump_wall_last_vy;
static uint32_t jump_wall_last_t;
static bool jump_wall_last_valid;
static bool jump_wall_anim_pending;
static uint16_t jump_wall_anim,jump_wall_track,jump_wall_track2;
// Player updates since the last slide (0 in the update it happened; counted
// at the start of each player update, like the bounce's own updates).
static unsigned jump_wall_age=0xffff;
static uint64_t jump_wall_loads;
// The horizontal velocity before the slide: where he was heading into the wall.
static double jump_wall_vx,jump_wall_vz;
static void jump_walls_update() {
    const uint64_t loads=psx_mod_savestate_loads();
    if(loads!=jump_wall_loads) {jump_wall_loads=loads;jump_wall_age=0xffff;return;}
    if(psx_mod_read_byte(player+0x22c)!=9) {jump_wall_age=0xffff;return;}
    if(jump_wall_age<0xffff)++jump_wall_age;
}
// A slide 'lo'..'hi' player updates ago in this flight (a bounce's 107 updates).
static bool jump_wall_recent(unsigned lo,unsigned hi) {
    return jump_walls_slide() && jump_wall_age>=lo && jump_wall_age<=hi;
}
// Entry of 0x8003ebf4 from the airborne handler (ra 0x80055934), after the
// ttk.modern.controls hook has made this update's takeoff redirect, air
// steering and delayed vertical: the velocity the original is about to use.
static void jump_walls_presolve(CPUState* cpu) {
    if(cpu->gpr[4]==player && cpu->gpr[31]==0x80055934)jump_wall_last_valid=false;
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x80055934 || !jump_walls_slide() ||
       psx_mod_read_byte(player+0x22c)!=9 || (psx_mod_read_word(player)&0x20002) ||
       !player_identity_ready())return;
    static uint32_t saved[0x8a4/4];
    for(unsigned i=0;i<0x8a4/4;++i)saved[i]=psx_mod_read_word(player+4*i);
    auto restore=[&]{for(unsigned i=0;i<0x8a4/4;++i)if(psx_mod_read_word(player+4*i)!=saved[i])psx_mod_write_word(player+4*i,saved[i]);};
    const double vx0=(int32_t)saved[0x1f4/4],vz0=(int32_t)saved[0x1fc/4];
    double vx=vx0,vz=vz0;int32_t vy=(int32_t)saved[0x1f8/4];
    bool wall=false,changed=false;
    // One isolated integration and sweep with the current velocity; restores Duke.
    auto sweep=[&](int32_t& result,double& nx,double& nz)->bool {
        int32_t ignored=0;result=0;++jump_wall_passes;
        const bool ok=original_call(cpu,0x8003ebf4,0,&ignored) && original_call(cpu,0x8007a98c,0,&result);
        nx=(int32_t)psx_mod_read_word(player+0x194);nz=(int32_t)psx_mod_read_word(player+0x19c);
        restore();
        return ok;
    };
    auto apply=[&]{
        saved[0x1f4/4]=(uint32_t)(int32_t)std::lround(vx);
        saved[0x1f8/4]=(uint32_t)vy;
        saved[0x1fc/4]=(uint32_t)(int32_t)std::lround(vz);
        restore();changed=true;
    };
    // Up to two clips against the faces the sweep reports (a wall, or the
    // crease of two), then, if a corner still blocks, a search around it.
    double first_nx=0,first_nz=0,last_nx=0,last_nz=0;
    bool blocked=false;
    for(unsigned pass=0;pass<=2;++pass) {
        int32_t result=0;double nx=0,nz=0;
        if(!sweep(result,nx,nz))return;
        const double nh=std::hypot(nx,nz);
        if(result==1) {
            if(nh<64) {++jump_wall_unsolved;break;}
            if(pass==2) {blocked=true;last_nx=nx/nh;last_nz=nz/nh;break;}
            if(!wall) {first_nx=nx/nh;first_nz=nz/nh;}
            last_nx=nx/nh;last_nz=nz/nh;
            // A wall: remove the motion into it (the normal faces Duke) and
            // leave a little outward drift so the next sweep does not touch
            // it again. Moving along it (or off it) and still touching: push
            // off it a little harder.
            const double into=(vx*nx+vz*nz)/nh;
            if(into>-wall_separation) {
                if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
                    std::fprintf(stderr,"ttk-jump-wall touch anim=%u n=%.0f,%.0f v=%.0f,%d,%.0f pass=%u\n",psx_mod_read_half(player+0x60),
                        nx,nz,vx,vy,vz,pass);
                ++jump_wall_nudges;
            }
            if(wall)++jump_wall_corners;
            const double out=into<=-wall_separation?wall_separation-into:wall_separation*(2+2*pass);
            vx+=out*nx/nh;vz+=out*nz/nh;wall=true;apply();
        } else if(result==3 && vy<0) {
            // A ceiling while rising: stop the rise; a sloped one also takes
            // the motion into it.
            vy=0;
            const double into=nh>=64?(vx*nx+vz*nz)/nh:0;
            if(into<0) {vx-=into*nx/nh;vz-=into*nz/nh;}
            ++jump_ceiling_stops;apply();
        } else break;
    }
    if(blocked) {
        // A convex corner (the edge of a monument base, a pillar): the faces'
        // own normals do not clear it. Try the takeoff direction turned away
        // from the corner a little more each time (15-degree steps, the side
        // the first clip slid to first), keeping the speed's share along the
        // new heading, then a push straight off the faces. The first heading
        // the original sweep clears is taken; none, and Duke is pushed off.
        ++jump_wall_corners;
        const double speed=std::hypot(vx0,vz0);
        const double hx=speed>1?vx0/speed:-first_nx,hz=speed>1?vz0/speed:-first_nz;
        // Turn first toward the side the first clip slid to: rotating the
        // heading by +a moves it along (-hz, hx), and the slide lies on the
        // side of the heading where the wall normal points.
        const double side=(hx*first_nz-hz*first_nx)>=0?1.0:-1.0;
        bool found=false;
        auto attempt=[&](double dx,double dz,double v)->bool {
            vx=dx*v;vz=dz*v;apply();
            int32_t result=0;double nx=0,nz=0;
            if(!sweep(result,nx,nz))return false;
            return result!=1;
        };
        for(int k=1;k<=6 && !found;++k)
            for(double sign:{side,-side}) {
                const double a=sign*k*tau/24,c=std::cos(a),s=std::sin(a);
                const double dx=hx*c-hz*s,dz=hx*s+hz*c;
                if(attempt(dx,dz,std::max(speed*c,corner_min_speed))) {found=true;break;}
            }
        if(!found) {
            const double ox=first_nx+last_nx,oz=first_nz+last_nz,ol=std::hypot(ox,oz);
            if(ol>0.1 && attempt(ox/ol,oz/ol,corner_min_speed))found=true;
            else if(attempt(last_nx,last_nz,corner_min_speed))found=true;
        }
        if(found)++jump_wall_corner_turns; else ++jump_wall_stops;
        if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
            std::fprintf(stderr,"ttk-jump-wall corner found=%d v=%.0f,%.0f\n",found,vx,vz);
    }
    jump_wall_last_vx=vx;jump_wall_last_vz=vz;jump_wall_last_vy=vy;
    jump_wall_last_t=saved[0x200/4];jump_wall_last_valid=true;
    if(!changed)return;
    if(wall) {
        // Remember where he was heading for the ledge helpers.
        if(jump_wall_age>4 || jump_wall_age==0xffff) {jump_wall_vx=vx0;jump_wall_vz=vz0;}
        jump_wall_age=0;++jump_wall_slides;
    }
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-jump-wall anim=%u v=%.0f,%.0f -> %.0f,%d,%.0f wall=%d\n",psx_mod_read_half(player+0x60),vx0,vz0,
            (double)(int32_t)psx_mod_read_word(player+0x1f4),(int32_t)psx_mod_read_word(player+0x1f8),
            (double)(int32_t)psx_mod_read_word(player+0x1fc),wall);
}
// Safety net: a corner the pre-pass could not clear (every heading still
// touches it) reaches the original bounce 0x8003ef08 (from the wall case,
// 0x800559f8) or deflection 0x8003ef78 (0x800559e8). Both rerun the
// integration 0x8003ebf4 after replacing the velocity; at that entry (its
// caller's return address at sp+0x24) put back the pre-pass velocity minus
// the part into the wall the sweep reported, plus a push off it, with the
// vertical speed and flight timer this update started with (no second
// gravity tick, no lost rise). The handler then writes the bounce 107; the
// next animation runner call (0x80059db0, the upper body's in the same player
// update) puts the flight animation back before any update evaluates it. The original's
// rumble and contact sound have already played for this one update.
static void jump_walls_bounce_rerun(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    if(cpu->gpr[4]!=player || (ra!=0x8003ef68 && ra!=0x8003efc0) || !jump_wall_last_valid || !jump_walls_slide())return;
    const uint32_t caller=psx_mod_read_word(cpu->gpr[29]+0x24);
    if(!((ra==0x8003ef68 && caller==0x800559f8) || (ra==0x8003efc0 && caller==0x800559e8)) || !player_identity_ready())return;
    jump_wall_last_valid=false;
    double nx=(int32_t)psx_mod_read_word(player+0x194),nz=(int32_t)psx_mod_read_word(player+0x19c);
    const double nh=std::hypot(nx,nz);
    double vx=jump_wall_last_vx,vz=jump_wall_last_vz;
    if(nh>=64) {
        nx/=nh;nz/=nh;
        const double into=vx*nx+vz*nz;
        if(into<0) {vx-=into*nx;vz-=into*nz;}
        vx+=nx*corner_min_speed/4;vz+=nz*corner_min_speed/4;
    }
    psx_mod_write_word(player+0x1f4,(uint32_t)(int32_t)std::lround(vx));
    psx_mod_write_word(player+0x1fc,(uint32_t)(int32_t)std::lround(vz));
    psx_mod_write_word(player+0x1f8,(uint32_t)jump_wall_last_vy);
    psx_mod_write_word(player+0x200,jump_wall_last_t);
    if(ra==0x8003ef68) {
        jump_wall_anim_pending=true;
        jump_wall_anim=psx_mod_read_half(player+0x60);
        jump_wall_track=psx_mod_read_half(player+0x68);jump_wall_track2=psx_mod_read_half(player+0x6a);
    }
    jump_wall_age=0;++jump_wall_catches;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-jump-wall catch %s anim=%u v=%.0f,%d,%.0f\n",ra==0x8003ef68?"bounce":"deflect",psx_mod_read_half(player+0x60),vx,jump_wall_last_vy,vz);
}
static void jump_walls_runner(CPUState* cpu) {
    // The first animation runner call after the catch (the upper body's,
    // 0x8005a5a8, in the same player update: the airborne handler runs after
    // the lower body's), before any update evaluates 107.
    if(!jump_wall_anim_pending)return;
    const uint32_t a0=cpu->gpr[4];
    if(a0<player+0x60 || a0>player+0x100)return;
    jump_wall_anim_pending=false;
    if(psx_mod_read_half(player+0x60)!=107 || psx_mod_read_byte(player+0x22c)!=9)return;
    psx_mod_write_half(player+0x60,jump_wall_anim);
    psx_mod_write_half(player+0x68,jump_wall_track);psx_mod_write_half(player+0x6a,jump_wall_track2);
}
static const char* jump_walls_debug_json() {
    static char buffer[256];
    std::snprintf(buffer,sizeof buffer,"{\"slide\":%s,\"slides\":%u,\"corners\":%u,\"turns\":%u,\"catches\":%u,\"stops\":%u,\"ceilings\":%u,\"unsolved\":%u,\"nudges\":%u,\"passes\":%u,\"age\":%u}",
        jump_walls_slide()?"true":"false",jump_wall_slides,jump_wall_corners,jump_wall_corner_turns,jump_wall_catches,jump_wall_stops,jump_ceiling_stops,jump_wall_unsolved,jump_wall_nudges,
        jump_wall_passes,jump_wall_age);
    return buffer;
}
