// D08X hold-E airborne ledge grab (Modernized). Original rules
// (documentation/73-d08x-ledge-grab.md):
//  * The running/standing-directional airborne handler 0x80055904 first calls
//    the reach 0x800557f8. An animation event (+0xb8 & 0x2000) arms the reach
//    once per jump (+0x224 |= 0x800000); while armed, a held Cross with the
//    weapon stowed (upper-table bit 8 clear) switches to reach 109, and every
//    reach update runs the surface acquisition 0x80055208 BEFORE the flight's
//    own collision. It catches a ladder (mode 3), a climbable object (mode 7)
//    or a ledge with headroom (mode 6, grab 147/148 -> hang 149 -> pull-up 140).
//  * The event comes late in the jump. A standing or short running jump at a
//    wall with a ledge in reach hits it first (impact 107) and bounces off, in
//    Vanilla as well; Up + Cross only grab when the wall is far enough away.
// Modernized contract: while E asks for a reach (held, pending or already
// reaching) through an owned jump (98/103/104 in mode 9), arm the original
// reach at once instead of waiting for the event. The original then enters 109
// on the held Cross and its own acquisition decides whether there is anything
// to catch; with nothing in reach Duke flies, lands or bounces exactly as
// before. Jumps without E, Vanilla and precision aim are unchanged.
static constexpr uint32_t reach_armed=0x800000u;
static unsigned ledge_reach_arms;
// Called from the owned-jump ballistic hook (0x8003ebf4 from 0x80055934), whose
// guards already require Duke, mode 9, an original jump animation and identity.
static void ledge_reach_arm(const InputFrame& f) {
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(animation!=98 && animation!=103 && animation!=104)return;
    // The hook's guards already prove Duke, mode 9, identity and the normal
    // camera; the first-person eye jump owns no flight lease, so none is required.
    if(!f.active || f.held[original_aim] || !input_airborne_reach_held())return;
    const uint32_t flags=psx_mod_read_word(player+0x224);
    if(flags&(reach_armed|0x241u))return;
    psx_mod_write_word(player+0x224,flags|reach_armed);
    ++ledge_reach_arms;
}
// D08X follow-up (boxes room): a running jump with E held into a climbable
// object (a crate, flags 0xc0) hits it before the reach's acquisition can see
// it: the contact (+0x174) is only set by the flight collision, after the
// acquisition ran, and the next update is already the bounce 107. The ground
// obstacle action (0x80051cf0) mantles the same object with 134..139, chosen
// by the ledge height +0x1c4 above the floor +0x1c8; those mantles end by
// raising the floor by +0x1c4 (0x8004c728). So on the first updates of an E
// bounce off a climbable object whose top is within that range above Duke's
// feet, start the matching original mantle from where he is in the air.
static constexpr uint32_t object_mantle=0xc0u;
static int32_t stand_offset=507;   // floor (+0x1c8) minus root Y while standing
static unsigned air_mantles, bounce_updates;
// 0x80073ae8 for box index 0, for every object kind: the object's box list
// (+0x44), a multi-box model (type flag 0x2000: model +0x68 -> +8) or the
// model's single box (+0x40, +4). Eight halfwords: x, y, z, pad min and max.
static bool object_box(uint32_t object,int16_t* box) {
    auto valid=[](uint32_t a){return a>=0x80010000u && a<0x80200000u;};
    uint32_t base=0;
    const uint32_t list=psx_mod_read_word(object+0x44);
    if(list) {
        if(!valid(list) || (list&3))return false;
        base=psx_mod_read_word(list+8);
    } else {
        const uint32_t model=psx_mod_read_word(object+0x40);
        if(!valid(model) || (model&3))return false;
        if(object_type_flags(object)&0x2000u) {
            const uint32_t entry=psx_mod_read_word(model+0x68);
            if(!valid(entry) || (entry&3))return false;
            base=psx_mod_read_word(entry+8);
        } else base=model+4;
    }
    if(!valid(base) || (base&1))return false;
    for(int i=0;i<8;++i)box[i]=(int16_t)psx_mod_read_half(base+2*i);
    return box[4]>box[0] && box[6]>box[2] && box[5]>box[1];
}
static bool object_top(uint32_t object,int32_t& top) {
    int16_t b[8];
    if(!object_box(object,b))return false;
    top=(int32_t)psx_mod_read_word(object+8)+std::min(b[1],b[5]);
    return true;
}
// Another object sitting on top of this one (a crate stack): no room to stand.
static bool object_covered(uint32_t object,int32_t top) {
    const double ox=(int32_t)psx_mod_read_word(object+4),oz=(int32_t)psx_mod_read_word(object+12);
    uint32_t node=psx_mod_read_word(player+0x220);
    for(int guard=0;node && guard<64;++guard) {
        if(node<0x80010000u || node>=0x80200000u || (node&3))break;
        const uint32_t next=psx_mod_read_word(node+4),other=psx_mod_read_word(node+8);
        node=next;
        int16_t b[8];
        if(other==object || !object_box(other,b))continue;
        const int32_t bottom=(int32_t)psx_mod_read_word(other+8)+std::max(b[1],b[5]);
        const double half=std::max(b[4]-b[0],b[6]-b[2])/2.0;
        if(std::abs(bottom-top)<=96 &&
           std::hypot((int32_t)psx_mod_read_word(other+4)-ox,(int32_t)psx_mod_read_word(other+12)-oz)<half)return true;
    }
    return false;
}
static unsigned mantle_animation(int32_t rel) {
    // Jump mantles only (the ground action keeps the original table).
    // 0x80051e7c: +0x1c4 (negative = above the feet) picks the mantle height;
    // taller climbable objects take the full climb 139.
    if(rel<-0x33f)return 139;   // mid-air, 138's band only completes as the full climb
    if(rel>=-0x1bf)return 134;
    if(rel>=-0x23f)return 135;
    if(rel>=-0x2bf)return 136;
    if(rel>=-0x33f)return 137;
    return 138;
}
static void ledge_reach_mantle(CPUState* cpu) {
    const unsigned animation=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    const int32_t y=(int32_t)psx_mod_read_word(player+8);
    // Learn the standing hip height from ordinary settled ground.
    if(mode==0 && psx_mod_read_byte(player+0x22d)==0 && land_gait_anim(animation)) {
        const int32_t offset=(int32_t)psx_mod_read_word(player+0x1c8)-y;
        if(offset>=400 && offset<=640)stand_offset=offset;
    }
    if(animation==107 && mode==9) ++bounce_updates; else bounce_updates=0;
    // Any original jump or fall in flight, the reach, or the start of a bounce.
    const bool airborne=mode==9 && (animation==97 || animation==98 || animation==103 || animation==104 ||
        animation==108 || animation==109 || (animation==107 && bounce_updates<=4));
    if(!airborne)return;
    const auto& f=input_snapshot(Context::Gameplay);
    if(!f.active || f.held[original_aim] || !input_airborne_reach_held() || !player_identity_ready() ||
       psx_mod_read_byte(player+0x3b8)!=0 || (psx_mod_read_word(player)&0x20002) ||
       (psx_mod_read_word(player+0x224)&0x241u))return;
    const double px=(int32_t)psx_mod_read_word(player+4),pz=(int32_t)psx_mod_read_word(player+12);
    // Direction of travel (the flight velocity), else the body heading.
    double mx=(int32_t)psx_mod_read_word(player+0x1f4),mz=(int32_t)psx_mod_read_word(player+0x1fc);
    // A bounce (107) has already reversed the velocity: use the heading there.
    if(animation==107 || std::hypot(mx,mz)<200) {
        const double heading=(psx_mod_read_half(player+0x1c)&4095)*tau/4096;
        mx=std::sin(heading);mz=std::cos(heading);
    }
    const double ml=std::hypot(mx,mz);mx/=ml;mz/=ml;
    const int32_t feet=y+stand_offset;
    // A climbable object (0xc0) whose rotated box Duke is within reach of,
    // heading toward it (up to ~75 degrees off), its top in mantle range.
    double face_x=0,face_z=0;
    auto candidate=[&](uint32_t object,int32_t& rel) {
        if(!(object_type_flags(object)&object_mantle))return false;
        int16_t b[8];
        if(!object_box(object,b))return false;
        rel=(int32_t)psx_mod_read_word(object+8)+std::min(b[1],b[5])-feet;
        // Below 0x100 he lands anyway; 0x100..0x3ff are the height mantles; a
        // little more (a crate next to the one he stands on, hit just after
        // takeoff) is the original full climb 139 after the ground line-up.
        if(rel>-0x100 || rel<-0x4c0)return false;
        if(object_covered(object,rel+feet))return false;
        const double a=(psx_mod_read_half(object+0x1c)&4095)*tau/4096,c=std::cos(a),s=std::sin(a);
        const double wx=px-(int32_t)psx_mod_read_word(object+4),wz=pz-(int32_t)psx_mod_read_word(object+12);
        const double lx=c*wx-s*wz,lz=s*wx+c*wz;   // inverse of RotMatrixY
        const double nx=std::clamp(lx,double(b[0]),double(b[4])),nz=std::clamp(lz,double(b[2]),double(b[6]));
        const double ex=lx-nx,ez=lz-nz,d=std::hypot(ex,ez);
        if(std::getenv("DNTTK_MANTLE_DEBUG"))std::fprintf(stderr,"mantle-cand %08x anim=%u rel=%d local=%.0f,%.0f box=%d..%d,%d..%d d=%.0f\n",object,animation,rel,lx,lz,b[0],b[4],b[2],b[6],d);
        // Duke's own collision radius keeps his centre ~370 off a face he touches.
        if(d<1 || d>450)return false;   // inside its footprint (on top) or out of reach
        // Nearest face point back in world space; Duke must be moving toward it.
        const double tx=-(c*ex+s*ez)/d,tz=-(-s*ex+c*ez)/d;
        if(tx*mx+tz*mz<0.25)return false;
        face_x=tx;face_z=tz;
        return true;
    };
    int32_t rel=0;
    uint32_t object=psx_mod_read_word(player+0x174);
    if(std::getenv("DNTTK_MANTLE_DEBUG"))std::fprintf(stderr,"mantle-air anim=%u feet=%d 174=%08x 220=%08x\n",animation,feet,object,psx_mod_read_word(player+0x220));
    if(!candidate(object,rel)) {
        object=0;
        uint32_t node=psx_mod_read_word(player+0x220);
        for(int guard=0;node && guard<64;++guard) {
            if(node<0x80010000u || node>=0x80200000u || (node&3))break;
            const uint32_t next=psx_mod_read_word(node+4),o=psx_mod_read_word(node+8);
            if(candidate(o,rel)) {object=o;break;}
            node=next;
        }
        if(!object)return;
    }
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-air-mantle object=%08x anim=%u feet=%d rel=%d mantle=%u\n",object,animation,feet,rel,mantle_animation(rel));
    const unsigned climb=mantle_animation(rel);
    // The ground obstacle action lines Duke up against the touched box before
    // every mantle (0x80051dc8 / 0x80051fb4: 0x8007ec4c with 0x113): it places
    // him at the face, squares him to it and checks the angle. Do the same with
    // the box found here (index 0); if the original refuses, no mantle.
    const uint32_t keep174=psx_mod_read_word(player+0x174);
    const uint16_t keep178=psx_mod_read_half(player+0x178);
    const uint16_t keep1c=psx_mod_read_half(player+0x1c),keep24=psx_mod_read_half(player+0x24);
    const uint32_t keepx=psx_mod_read_word(player+4),keepz=psx_mod_read_word(player+12);
    // Face the box first, as the run that touches it would.
    const int heading=int(std::lround(std::atan2(face_x,face_z)*4096/tau))&4095;
    psx_mod_write_half(player+0x1c,uint16_t(heading));psx_mod_write_half(player+0x24,uint16_t(heading));
    psx_mod_write_word(player+0x174,object);psx_mod_write_half(player+0x178,0);
    int32_t lined=0;
    if(!original_call(cpu,0x8007ec4c,0x113,&lined) || !(lined&0xffff)) {
        psx_mod_write_word(player+0x174,keep174);psx_mod_write_half(player+0x178,keep178);
        psx_mod_write_half(player+0x1c,keep1c);psx_mod_write_half(player+0x24,keep24);
        psx_mod_write_word(player+4,keepx);psx_mod_write_word(player+12,keepz);
        return;
    }
    psx_mod_write_half(player+0x1c,psx_mod_read_half(player+0x24));
    psx_mod_write_word(player+0x1c8,feet);psx_mod_write_word(player+0x1c4,rel);
    for(uint32_t v:{0x1f4u,0x1f8u,0x1fcu})psx_mod_write_word(player+v,0);
    psx_mod_write_byte(player+0x22c,0);psx_mod_write_byte(player+0x22d,0);
    psx_mod_write_half(player+0x60,climb);
    psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    bounce_updates=5;++air_mantles;
}

// D08X: the original object hang (mode 7, 149..151 after the reach catches a
// climbable object) offers no pull-up when the space above is blocked (a crate
// under another crate or a ceiling); only Square lets go. In Modernized, S lets
// go too, and holding W that has not started a climb within ~40 updates lets go
// on its own, so Duke never stays stuck there.
static unsigned hang_stall, hang_releases;
static bool hang_release;
static void object_hang_update() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    const bool was=hang_release;
    hang_release=false;
    if(psx_mod_read_byte(player+0x22c)!=7 || animation<149 || animation>151 || !traversal_input_ready()) {hang_stall=0;return;}
    const auto& f=input_snapshot(Context::Gameplay);
    if(f.move_y>0.5f) {if(hang_stall<1000)++hang_stall;}
    else hang_stall=0;
    // The object hang is the original pole-style hang: A/D walk Duke round the
    // object. Caught on a crate-type climbable (0xc0: a crate, the dumpster) by
    // a jump, it hangs him from the side of a crate stack and turns him in
    // circles, so let go at once; the bounce and fall play as without the reach.
    const bool crate=(object_type_flags(psx_mod_read_word(player+0x17c))&object_mantle)==object_mantle;
    hang_release=crate || f.move_y<-0.5f || hang_stall>40;
    if(hang_release && !was)++hang_releases;
}
bool object_hang_release_ready() {return hang_release;}
// D08X confidence: the reach's acquisition (0x80055208) probes for a ledge at a
// fixed point 425 above Duke's root (0x8007b2f4), so a ledge a little above the
// jump's apex is missed. While E asks for a reach (109, or the first updates of
// a bounce 107), retry that same original acquisition with Duke raised 160,
// 320 and 480 (about an arm's length); the original decides the catch, its type and hang. A miss restores
// everything. Vanilla and jumps without E never see it.
static unsigned lift_catches,turn_catches;
static int32_t last_lift;
// D08Y gap jumps (documentation/77-d08y-gap-jump-research.md): a jump that
// meets a level-geometry ledge with the feet a little below its top (the far
// side of a slightly-too-long gap) has no original outcome but the bounce 107:
// the ledge is too low for the hang acquisition (its probe point is ~930 above
// the feet) and the flight collision leaves the ledge height (+0x1c4) at 0, so
// the object jump mantle above cannot see it either. Retrying the same original
// acquisition with Duke lowered finds it: a lowered ledge catch (mode 6) proves
// a ledge with headroom in front of him and leaves its top in +0x1c8, and is
// turned into the original height mantle (134..137, 139) from where he is,
// exactly as the jump mantle does for crates.
static unsigned drop_mantles;
static int32_t last_drop_rel;
// A jump mantle starts where the catch placed Duke (at the face, up to ~400
// units ahead of where he flies). Jumping him there in one update read as a
// pop, so the mantle starts where he is and glides him to that point over its
// first updates (the original mantle's own root motion still runs).
static bool mantle_gliding;
static int32_t glide_x,glide_z;
static unsigned glide_updates,glides;
static void mantle_glide_start(uint32_t x,uint32_t z) {
    glide_x=(int32_t)x;glide_z=(int32_t)z;glide_updates=0;mantle_gliding=true;++glides;
}
static void mantle_glide() {
    if(!mantle_gliding)return;
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(animation<134 || animation>139 || ++glide_updates>8) {mantle_gliding=false;return;}
    const int32_t x=(int32_t)psx_mod_read_word(player+4),z=(int32_t)psx_mod_read_word(player+12);
    const double dx=glide_x-x,dz=glide_z-z,d=std::hypot(dx,dz);
    if(d<=8) {mantle_gliding=false;return;}
    const double step=std::min(d,std::max(d/3,60.0));
    psx_mod_write_word(player+4,uint32_t(x+(int32_t)std::lround(dx*step/d)));
    psx_mod_write_word(player+12,uint32_t(z+(int32_t)std::lround(dz*step/d)));
}
// One retry of the original acquisition: Duke lifted (dy < 0, D08X: a ledge a
// little above the probe point) or lowered (dy > 0, D08Y: a ledge below it),
// heading turned by `turn`.
struct ReachTry {int16_t dy,turn;};
// Each isolated acquisition costs about a quarter of a millisecond, so one
// player update tries at most a few of these (all of them every update froze
// frames). The core tries cover every catch seen in testing and come round
// every three updates; the lifted turned tries come round every twelve.
static constexpr ReachTry core_rising[]={{0,290},{0,-290},{-160,0},{-320,0},{-480,0},{0,580},{0,-580}};
static constexpr ReachTry core_falling[]={{0,290},{0,-290},{-160,0},{240,0},{-320,0},{360,0},{-480,0},{120,0},{0,580},{0,-580},{480,0}};
static constexpr ReachTry reach_extra[]={{-160,290},{-160,-290},{-320,290},{-320,-290},{-480,290},{-480,-290},
    {-160,580},{-160,-580},{-320,580},{-320,-580},{-480,580},{-480,-580}};
static constexpr unsigned reach_core_budget=4,reach_extra_budget=1;
static unsigned reach_core_cursor,reach_extra_cursor;
static bool reach_session;
static void ledge_reach_retry(CPUState* cpu) {
    const unsigned animation=psx_mod_read_half(player+0x60);
    const auto& f=input_snapshot(Context::Gameplay);
    const unsigned upper=psx_mod_read_half(player+0x74);
    const bool eligible=psx_mod_read_byte(player+0x22c)==9 &&
        (animation==109 || (animation==107 && bounce_updates>=1 && bounce_updates<=4)) &&
        f.active && !f.held[original_aim] && input_airborne_reach_held() && player_identity_ready() &&
        psx_mod_read_byte(player+0x3b8)==0 && !(psx_mod_read_word(player)&0x20002) &&
        !(psx_mod_read_word(player+0x224)&0x241u) && upper<280 && !(psx_mod_read_word(0x800c2824+4*upper)&8);
    if(!eligible) {reach_session=false;return;}
    if(!reach_session) {reach_session=true;reach_core_cursor=reach_extra_cursor=0;}
    // The lowered tries only on the way down: the band they cover (feet below
    // a ledge top the rising arc already passed) is only met descending.
    const bool falling=animation!=109 || (int32_t)psx_mod_read_word(player+0x1f8)>0;
    const ReachTry* core=falling?core_falling:core_rising;
    const unsigned core_n=falling?std::size(core_falling):std::size(core_rising);
    ReachTry plan[reach_core_budget+reach_extra_budget];unsigned n=0;
    for(unsigned k=0;k<reach_core_budget;++k)plan[n++]=core[(reach_core_cursor+k)%core_n];
    for(unsigned k=0;k<reach_extra_budget;++k)plan[n++]=reach_extra[(reach_extra_cursor+k)%std::size(reach_extra)];
    reach_core_cursor+=reach_core_budget;reach_extra_cursor+=reach_extra_budget;
    static uint32_t saved[0x8a4/4];
    for(unsigned i=0;i<0x8a4/4;++i)saved[i]=psx_mod_read_word(player+4*i);
    auto restore=[&]{for(unsigned i=0;i<0x8a4/4;++i)if(psx_mod_read_word(player+4*i)!=saved[i])psx_mod_write_word(player+4*i,saved[i]);};
    const int32_t y=(int32_t)psx_mod_read_word(player+8);
    const uint16_t keep1c=psx_mod_read_half(player+0x1c),keep24=psx_mod_read_half(player+0x24);
    for(unsigned t=0;t<n;++t) {
        const ReachTry r=plan[t];
        psx_mod_write_word(player+8,uint32_t(y+r.dy));
        // Angle forgiveness (D08X): the facing test (0x8007eaf0) wants the body
        // heading (+0x24) within 45 degrees of the wall normal and the probe
        // points along +0x1c; in Modernized the body follows the camera in
        // flight, so try it turned up to about 51 degrees either way.
        psx_mod_write_half(player+0x1c,uint16_t((keep1c+r.turn)&4095));
        psx_mod_write_half(player+0x24,uint16_t((keep24+r.turn)&4095));
        int32_t caught=0;
        if(!original_call(cpu,0x80055208,0,&caught)) {restore();return;}
        const unsigned mode=psx_mod_read_byte(player+0x22c);
        if(!caught || mode==9) {restore();continue;}
        if(r.dy<=0) {
            // D08X: the original catch, its type and hang stand.
            const int32_t lift=-r.dy;
            ++lift_catches;last_lift=lift;if(r.turn)++turn_catches;
            // The facing test squared +0x24 to the wall; the probe yaw follows.
            psx_mod_write_half(player+0x1c,psx_mod_read_half(player+0x24));
            // Back to his real height; ledge_hang_settle() brings him flush.
            psx_mod_write_word(player+8,uint32_t(y));
            if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
                std::fprintf(stderr,"ttk-lift-catch lift=%d turn=%d anim=%u mode=%u y=%d\n",lift,r.turn,psx_mod_read_half(player+0x60),
                    mode,(int32_t)psx_mod_read_word(player+8));
            return;
        }
        const int32_t top=(int32_t)psx_mod_read_word(player+0x1c8);
        const int32_t feet=y+stand_offset,rel=top-feet;
        if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
            std::fprintf(stderr,"ttk-drop-catch drop=%d turn=%d mode=%u anim=%u top=%d feet=%d rel=%d\n",r.dy,r.turn,mode,
                psx_mod_read_half(player+0x60),top,feet,rel);
        // D08Y: only a ledge (mode 6) whose top is 0x60..0x4c0 above the feet;
        // a lower lip is left to the flight's own landing (and, should he bounce
        // off it, to ledge_step_up). Anything else is left exactly as it was.
        if(mode!=6 || rel>-0x60 || rel<-0x4c0) {restore();continue;}
        // Squared to the wall by the facing test; keep the catch's X/Z (at the
        // face), the real height, and start the mantle as the ground does.
        const uint16_t heading=psx_mod_read_half(player+0x24);
        const uint32_t x=psx_mod_read_word(player+4),z=psx_mod_read_word(player+12);
        restore();
        mantle_glide_start(x,z);
        psx_mod_write_half(player+0x1c,heading);psx_mod_write_half(player+0x24,heading);
        psx_mod_write_word(player+0x1c8,feet);psx_mod_write_word(player+0x1c4,rel);
        for(uint32_t v:{0x1f4u,0x1f8u,0x1fcu})psx_mod_write_word(player+v,0);
        psx_mod_write_byte(player+0x22c,0);psx_mod_write_byte(player+0x22d,0);
        psx_mod_write_half(player+0x60,mantle_animation(rel));
        psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
        bounce_updates=5;++drop_mantles;last_drop_rel=rel;
        return;
    }
}

// D08Y ledge forgiveness, with or without E, armed or not: a running or
// directional jump (98/103/104, or its E reach 109) that meets a geometry ledge with the feet just
// under its top (a lip of up to 0x100) has only the original bounce 107, even
// though the jump nearly made it. On the first updates of that bounce, if the
// original acquisition (lowered, with the reach's held bit presented for the
// isolated call) finds a ledge with headroom in front of him and its top at
// most 0x100 above his feet, start the smallest original mantle 134 instead.
// Lifting him in flight does not help: the bounce is already decided in the
// update the ledge first comes in reach. Taller ledges are left to the E jump
// mantle (ledge_reach_retry) or the original bounce. An E jump whose lowered
// catch found a lip under 0x60 (left to the landing) is covered here too.
static unsigned step_ups,flight_updates;
static int32_t last_step_rel;
static unsigned last_flight_anim;
static void ledge_step_up(CPUState* cpu) {
    const unsigned animation=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    if(mode==9 && (animation==98 || animation==103 || animation==104 || animation==109)) {last_flight_anim=animation;flight_updates=0;return;}
    if(mode!=9 || animation!=107 || !last_flight_anim) {last_flight_anim=0;return;}
    if(++flight_updates>3) {last_flight_anim=0;return;}
    const auto& f=input_snapshot(Context::Gameplay);
    if(!f.active || f.held[original_aim] || !player_identity_ready() || (psx_mod_read_word(player)&0x20002) ||
       (psx_mod_read_word(player+0x224)&0x241u))return;
    const uint32_t held=psx_mod_read_word(0x800d1b50+4*psx_mod_read_byte(player+0x233));
    if(held<0x80010000u || held>=0x80200000u || (held&3))return;
    const uint32_t buttons=psx_mod_read_word(held);
    static uint32_t saved[0x8a4/4];
    for(unsigned i=0;i<0x8a4/4;++i)saved[i]=psx_mod_read_word(player+4*i);
    auto restore=[&]{for(unsigned i=0;i<0x8a4/4;++i)if(psx_mod_read_word(player+4*i)!=saved[i])psx_mod_write_word(player+4*i,saved[i]);};
    const int32_t y=(int32_t)psx_mod_read_word(player+8),feet=y+stand_offset;
    const uint16_t keep1c=psx_mod_read_half(player+0x1c);
    // Two isolated acquisitions per update (see ledge_reach_retry): the five
    // heights come round over the three bounce updates checked.
    static constexpr int32_t step_drops[]={400,560,720,480,640,400};
    for(unsigned k=0;k<2;++k) {
        const int32_t drop=step_drops[(2*(flight_updates-1)+k)%std::size(step_drops)];
        psx_mod_write_word(player+8,uint32_t(y+drop));
        // During the bounce +0x1c may already be turned; probe along the body.
        psx_mod_write_half(player+0x1c,psx_mod_read_half(player+0x24));
        int32_t caught=0;
        psx_mod_write_word(held,buttons|1u);
        const bool called=original_call(cpu,0x80055208,0,&caught);
        psx_mod_write_word(held,buttons);
        if(!called) {restore();return;}
        const bool ledge=caught && psx_mod_read_byte(player+0x22c)==6;
        const int32_t rel=ledge?(int32_t)psx_mod_read_word(player+0x1c8)-feet:1;
        const uint16_t heading=psx_mod_read_half(player+0x24);
        const uint32_t x=psx_mod_read_word(player+4),z=psx_mod_read_word(player+12);
        restore();
        if(std::getenv("DNTTK_TRAVERSAL_TRACE") && caught)
            std::fprintf(stderr,"ttk-step-try drop=%d ledge=%d rel=%d\n",drop,ledge,rel);
        if(!ledge || rel>0x40 || rel<-0x100)continue;
        mantle_glide_start(x,z);
        psx_mod_write_half(player+0x1c,heading);psx_mod_write_half(player+0x24,heading);
        psx_mod_write_word(player+0x1c8,feet);psx_mod_write_word(player+0x1c4,std::min(rel,-1));
        for(uint32_t v:{0x1f4u,0x1f8u,0x1fcu})psx_mod_write_word(player+v,0);
        psx_mod_write_byte(player+0x22c,0);psx_mod_write_byte(player+0x22d,0);
        psx_mod_write_half(player+0x60,134);
        psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
        ++step_ups;last_step_rel=rel;last_flight_anim=0;
        if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
            std::fprintf(stderr,"ttk-step-up drop=%d rel=%d equip=%u\n",drop,rel,psx_mod_read_byte(player+0x3b8));
        return;
    }
    psx_mod_write_half(player+0x1c,keep1c);
}

// D08X: settle a jump catch flush on the ledge. The original catch keeps the
// height at which the probe found the ledge (higher still after a lifted or
// turned retry), so the hands can hang in the air above the ledge until a
// shimmy (0x8004c8e4..) re-aligns Duke. Measured on the slot-12 sewer wall and
// the boxes-room ledge, the shimmy settles his root 454..463 below the ledge
// top (+0x1c8 in the hang). So when an E jump has just caught a ledge (mode 6
// entered from flight), ease the root to that height over a few updates.
static constexpr int32_t k_hang_drop=456;
static bool hang_settling;
static unsigned hang_settles;
static void ledge_hang_settle() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    if(mode!=6 || animation<147 || animation>149) {hang_settling=false;return;}
    if(!hang_settling) {
        // Only the catch itself: from flight, during an E reach, Modernized.
        if(psx_mod_read_byte(player+0x22d)!=9 || !input_airborne_reach_held() || !player_identity_ready())return;
        hang_settling=true;++hang_settles;
    }
    const int32_t y=(int32_t)psx_mod_read_word(player+8);
    const int32_t target=(int32_t)psx_mod_read_word(player+0x1c8)+k_hang_drop;
    const int32_t d=target-y;
    if(std::abs(d)<=4 || std::abs(d)>900)return;   // settled, or not this ledge
    const int32_t step=std::abs(d)<=24?d:(d>0?std::max(d/3,24):std::min(d/3,-24));
    psx_mod_write_word(player+8,uint32_t(y+step));
}
