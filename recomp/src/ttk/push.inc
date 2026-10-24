// D08T pushable objects (Modernized). Original rules, verified on the first
// map's alley dumpster (documentation/64-d08t-pushable-objects.md):
//  * Object type flags live at *(0x800d2660) + 28 * object[+0x2c].
//    0x08000000 = pushable, 0xc0 = climbable. The dumpster (type 165) has both.
//  * Idle 63, holstered, a fresh Cross against a pushable object: 0x80051890
//    lines Duke up (LineUpWithObject, 0x113) and starts grab 121, remembering
//    the object at player+0x290.
//  * Grab 121 at track 3 (0x80048c74): Cross still held + Up pushes (120),
//    + Down pulls (119); the object moves along one world axis (0x800910a0).
//    Releasing Cross lets go.
//  * Walking or pressing Up into a climbable object with Cross held climbs it
//    instead (0x80051cf0 -> 139/140, mode 8). That branch is why E used to
//    mount the dumpster instead of grabbing it.
// Modernized contract (D08T1): pushability adds a verb, it never remaps one.
// E is always the normal interaction/mantle: E alone at a pushable object does
// nothing (as at any climbable object) and W + E into a climbable one climbs it
// through the original 0x80051cf0 branch. Holding Grab / Manipulate grabs any
// pushable object, W/S push/pull relative to the camera and releasing Grab lets
// go. The original grab, alignment, push/pull motion and climb animations stay
// authoritative for the original interaction. D08T3 below replaces the fixed
// shove with supported, input-driven travel in Modernized.
static constexpr uint32_t object_pushable=0x08000000u, object_climbable=0xc0u;
// 0x8001b634 allocates the type table as 0x7428 bytes: 1062 types of 28 bytes.
// D08T2: later eras use types above 512 (medieval Duke-symbol block: 924).
static constexpr unsigned object_type_count=0x7428u/28u;
static unsigned push_masks, push_idle_masks;
static uint32_t object_type_flags(uint32_t object) {
    if(object<0x80010000u || object>=0x80200000u || (object&3))return 0;
    const uint32_t table=psx_mod_read_word(0x800d2660);
    const unsigned type=psx_mod_read_half(object+0x2c);
    if(table<0x80010000u || table>=0x80200000u || (table&3) || type>=object_type_count)return 0;
    return psx_mod_read_word(table+28*type);
}
static bool push_animation(unsigned animation) {return animation>=119 && animation<=121;}
// Camera-only lease widening (state()): no movement, facing or gait ownership.
static bool push_state_early() {
    return push_animation(psx_mod_read_half(player+0x60)) &&
        psx_mod_read_byte(player+0x22c)==0 && psx_mod_read_byte(player+0x22d)==0 &&
        psx_mod_read_byte(player+0x3b8)==0 &&
        (object_type_flags(psx_mod_read_word(player+0x290))&object_pushable);
}
bool push_grab_ready() {
    return input_modernized() && gameplay_context() && !(psx_mod_read_word(player)&0x20002) &&
        psx_mod_read_word(camera+0xa4)==player && psx_mod_read_word(player+0x7d4)==camera &&
        push_state_early() && identity();
}
// D08T4: 0x80051890 grabs only when bit 0 of byte 4 of Duke's floor record
// (*(player+0x48)) is set; the levels clear it where a pushable object is
// placed as fixed scenery (level 5 street dumpster), and the original Action
// then does nothing there. Unaligned: the records are 7 bytes long.
static bool push_floor_allows() {
    const uint32_t floor=psx_mod_read_word(player+0x48);
    return floor>=0x80010000u && floor<0x80200000u-4 && (psx_mod_read_byte(floor+4)&1);
}
// D08T7: authored mobility for the verified X travel lanes. Position along
// the lane is deliberately not identity: a loaded boundary save must work on
// its first grab. Overlay + type + lane/rotation distinguish fixed scenery,
// including LEVEL05's otherwise identical dumpster. No floor bits are changed.
// Keep this data separate from acquisition and collision for future placements.
struct PushLane {uint32_t tag;unsigned type;int32_t y,z;unsigned yaw;};
static constexpr PushLane push_lanes[]={
    {3,165,-9728,13829,2048}, // LEVEL00 alley
    {9,924,5632,33268,3072}, // LEVEL06 symbol blocks, recent slots 9-11
    {9,924,5632,32244,3072},
    {9,924,5632,30196,3072},
};
static bool push_lane_allows(uint32_t object) {
    if(!input_modernized() || !object_type_flags(object) || !identity())return false;
    for(const auto& lane:push_lanes)
        if(identity_tag==lane.tag && psx_mod_read_half(object+0x2c)==lane.type &&
           int32_t(psx_mod_read_word(object+8))==lane.y &&
           int32_t(psx_mod_read_word(object+12))==lane.z &&
           (psx_mod_read_half(object+0x1c)&4095)==lane.yaw)return true;
    return false;
}
static void push_lane_acquire(CPUState* cpu);
// Duke is touching a pushable object on normal ground (idle, gait or wall bump)
// where the original grab can start.
bool push_contact_ready() {
    if(!player_identity_ready() || (psx_mod_read_word(player)&0x20002) ||
       psx_mod_read_byte(player+0x22c)!=0 || psx_mod_read_byte(player+0x22d)!=0)return false;
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(!land_gait_anim(animation) && animation!=94 && animation!=95)return false;
    return (object_type_flags(psx_mod_read_word(player+0x174))&object_pushable) && (push_floor_allows() || push_lane_allows(psx_mod_read_word(player+0x174)));
}
// Original idle with the weapon stowed: a fresh Cross here reaches 0x80051890.
bool push_idle_ready() {return interaction_ready() && psx_mod_read_half(player+0x60)==63;}
// Camera-relative W/A/S/D projected on Duke's facing (the push axis after the
// original line-up): toward the object is original Up (push), away is Down.
uint16_t push_pad(float move_x,float move_y) {
    if((!move_x && !move_y) || !push_grab_ready())return 0;
    const auto v=movement(view_yaw(),move_x,move_y);
    const double facing=psx_mod_read_half(player+0x1c)*tau/4096;
    const double along=v.x*std::sin(facing)+v.z*std::cos(facing);
    return along>0.38?16:along<-0.38?64:0;
}
// Clear this tick's Action (Cross) bit; the next tick sees a fresh press.
static bool push_mask_cross() {
    const uint32_t cross=psx_mod_read_word(0x800d1b50+4*psx_mod_read_byte(player+0x233));
    if(cross<0x80010000u || cross>=0x80200000u || (cross&3))return false;
    const uint32_t word=psx_mod_read_word(cross);
    if(!(word&1))return false;
    psx_mod_write_word(cross,word&~1u);
    return true;
}
// Only a touched object that is pushable and has no other Action use (switch,
// pickup or use flags 0x01c00000, which the original checks after the grab).
static bool push_only_object(uint32_t object) {
    const uint32_t flags=object_type_flags(object);
    return (flags&object_pushable) && !(flags&0x01c00000u);
}
// 0x80051cf0 entry: the original walk/Up-into-obstacle action. Grab's own Cross
// must never climb a pushable object (it asked to hold it); E's Cross still does,
// exactly as at any climbable object.
static void push_obstacle_action(CPUState* cpu,uint32_t ra) {
    const auto& f=input_snapshot(Context::Gameplay);
    if(cpu->gpr[4]!=player || (ra!=0x80052544 && ra!=0x8005349c && ra!=0x8005384c) ||
       !f.active || !input_push_grab_owns_cross() || !player_identity_ready())return;
    if(!(object_type_flags(psx_mod_read_word(player+0x174))&object_pushable))return;
    if(push_mask_cross())++push_masks;
}
// 0x80051890 entry: the original idle Action, whose fresh Cross grabs a touched
// pushable object (anim 121). In captured Modernized play only Grab may do that:
// E's Cross is masked here, so E at a pushable object does what it does at any
// other climbable object (nothing standing still; W + E climbs).
static void push_idle_action(CPUState* cpu,uint32_t ra) {
    const auto& f=input_snapshot(Context::Gameplay);
    if(cpu->gpr[4]!=player || (ra!=0x800467fc && ra!=0x80052c1c) ||
       !f.active || !player_identity_ready())return;
    if(input_push_grab_owns_cross()) {push_lane_acquire(cpu);return;}
    if(!push_only_object(psx_mod_read_word(player+0x174)))return;
    if(push_mask_cross())++push_idle_masks;
}

// D08T3: movement belongs to the held input, not the one-shot animation.
// Keep the original line-up, axis and clips. The dispatcher still runs, but
// receives zero root motion and no bone-driven object displacement.
static bool object_box(uint32_t object,int16_t* box);
static bool ladder_box(uint32_t object,int16_t* box);
static uint32_t push_manual_object;
static uint64_t push_manual_epoch;
static double push_sound_distance;
static int push_blocked_pad;
static unsigned push_manual_moves,push_manual_blocks;
static bool push_manual_enabled() {
    const char* value=std::getenv("DNTTK_PUSH");
    return !value || std::strcmp(value,"original");
}
struct PushBounds {int32_t x0,x1,y0,y1,z0,z1;};
static bool push_bounds(uint32_t object,PushBounds& b) {
    int16_t box[8];
    if(!object_box(object,box)) {
        // Ladder collision panels have zero depth. They still stop a block.
        if(!(object_type_flags(object)&0x200) || !ladder_box(object,box))return false;
        if(box[0]==box[4]){box[0]-=16;box[4]+=16;}
        if(box[2]==box[6]){box[2]-=16;box[6]+=16;}
    }
    // Original pushables line up on cardinal axes. Rotate their collision
    // box, rather than assuming the dumpster and symbol cube have one size.
    const double a=int16_t(psx_mod_read_half(object+0x1c))*tau/4096;
    const double c=std::cos(a),s=std::sin(a);
    double x0=1e9,x1=-1e9,z0=1e9,z1=-1e9;
    for(int x:{int(box[0]),int(box[4])})for(int z:{int(box[2]),int(box[6])}) {
        const double rx=x*c+z*s,rz=z*c-x*s;
        x0=std::min(x0,rx);x1=std::max(x1,rx);z0=std::min(z0,rz);z1=std::max(z1,rz);
    }
    const int32_t x=int32_t(psx_mod_read_word(object+4)),y=int32_t(psx_mod_read_word(object+8)),z=int32_t(psx_mod_read_word(object+12));
    b={x+int32_t(std::lround(x0)),x+int32_t(std::lround(x1)),y+box[1],y+box[5],z+int32_t(std::lround(z0)),z+int32_t(std::lround(z1))};
    return true;
}
// D08T5: LEVEL00 alley dumpster. The user's reference save (UI slot 7,
// 2026-10-23) places its centre at (7292,-9728,13829), yaw 2048.
// Authenticate the overlay and placed object's lane, not its RAM address.
// Clamp only travel toward the ladder. Older saves beyond the limit can retreat.
static int push_ladder_limit(uint32_t object,int dx) {
    if(dx<=0 || !first_map() || psx_mod_read_half(object+0x2c)!=165 ||
       int32_t(psx_mod_read_word(object+8))!=-9728 ||
       int32_t(psx_mod_read_word(object+12))!=13829 ||
       (psx_mod_read_half(object+0x1c)&4095)!=2048)return dx;
    const int64_t remaining=7292-int64_t(int32_t(psx_mod_read_word(object+4)));
    return int(std::max<int64_t>(0,std::min<int64_t>(dx,remaining)));
}
static bool push_step_clear(CPUState* cpu,uint32_t object,int dx,int dz) {
    PushBounds b;if(!push_bounds(object,b))return false;
    const HullState saved=hull_save();const uint32_t feet=psx_mod_read_word(player+0x1c8);
    HullScratch scratch(cpu->gpr[29]);
    auto finish=[&](bool ok){hull_restore(saved);psx_mod_write_word(player+0x1c8,feet);return ok;};
    // At the leading/trailing corners and centre, the block must retain the
    // same support and enough room for its full height. No gravity or puzzle
    // grid snapping: stop at the last supported position.
    psx_mod_write_word(player+0x1c8,b.y1);
    for(int x:{b.x0+16,(b.x0+b.x1)/2,b.x1-16})for(int z:{b.z0+16,(b.z0+b.z1)/2,b.z1-16}) {
        int32_t clearance=0;
        if(!original_call(cpu,0x8007765c,scratch.point(0,x+dx,b.y1-64,z+dz),&clearance))return finish(false);
        const int32_t step=int32_t(psx_mod_read_word(player+0x1c4));
        if(std::abs(int64_t(step))>48 || clearance<b.y1-b.y0-32 || clearance==0xffff)return finish(false);
    }
    // Duke must also have supported floor behind him when pulling, and
    // standing clearance. This room query does not collide with the held box.
    psx_mod_write_word(player+0x1c8,feet);
    int32_t clearance=0;
    const int32_t px=int32_t(psx_mod_read_word(player+4)),pz=int32_t(psx_mod_read_word(player+12));
    if(!original_call(cpu,0x8007765c,scratch.point(0,px+dx,int32_t(feet)-64,pz+dz),&clearance) ||
       std::abs(int64_t(int32_t(psx_mod_read_word(player+0x1c4))))>64 || clearance<0x370 || clearance==0xffff)return finish(false);
    // Sweep the leading face at foot and head height as well as checking
    // destination floors: thin walls cannot be crossed between samples.
    for(int x:{b.x0+16,b.x1-16})for(int z:{b.z0+16,b.z1-16})for(int y:{b.y0+32,b.y1-32}) {
        int32_t hit=0;
        const uint32_t from=scratch.point(0,x,y,z),to=scratch.point(1,x+dx,y,z+dz);
        if(!original_call(cpu,0x800774cc,from,&hit,to) || hit)return finish(false);
    }
    int32_t hit=0;
    const uint32_t from=scratch.point(0,px,int32_t(feet)-0x370,pz),to=scratch.point(1,px+dx,int32_t(feet)-0x370,pz+dz);
    if(!original_call(cpu,0x800774cc,from,&hit,to) || hit)return finish(false);
    // Duke's nearby list is culled for his body and can discover an obstacle
    // after the much wider held block has already crossed it. The room lists
    // used by 0x80095cb0 contain the complete placed objects (88-byte rooms,
    // list at +0x48), including flat ladder panels across a room boundary.
    auto list_clear=[&](uint32_t node) {
        for(unsigned n=0;node && n<1024;++n) {
            if(node<0x80010000 || node>0x801ffff0 || (node&3))return false;
            const uint32_t other=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            const uint32_t flags=object_type_flags(other);
            if(other==object || other==player || !flags || (flags&8))continue;
            PushBounds q;if(!push_bounds(other,q))continue;
            if(px+dx-220<q.x1 && px+dx+220>q.x0 && pz+dz-220<q.z1 && pz+dz+220>q.z0 && int32_t(feet)-0x370<q.y1-8 && int32_t(feet)>q.y0+8)return false;
            if(b.x0+dx<q.x1-8 && b.x1+dx>q.x0+8 && b.z0+dz<q.z1-8 && b.z1+dz>q.z0+8 && b.y0<q.y1-8 && b.y1>q.y0+8)return false;
        }
        return !node;
    };
    if(!list_clear(psx_mod_read_word(player+0x220)))return finish(false);
    const uint32_t rooms=psx_mod_read_word(0x800da454),count=psx_mod_read_word(0x800da47c);
    if(count>128 || (count && (rooms<0x80010000 || rooms>0x80200000-count*88 || (rooms&3))))return finish(false);
    for(unsigned room=0;room<count;++room)
        if(!list_clear(psx_mod_read_word(rooms+88*room+0x48)))return finish(false);
    return finish(true);
}
// Mirror only the original grab branch (0x800518e0..0x800519b4), replacing
// its floor permission with the placement policy. Original line-up and the
// manual mover's support/collision checks must both succeed. Roll back all
// player query/alignment fields on refusal; the room's floor data stays intact.
static void push_lane_acquire(CPUState* cpu) {
    const uint32_t object=psx_mod_read_word(player+0x174);
    if(guest_busy || push_floor_allows() || !push_manual_enabled() ||
       !push_contact_ready() || !push_lane_allows(object) || !push_only_object(object) ||
       psx_mod_read_half(player+0x60)!=63 || psx_mod_read_byte(player+0x3b8) ||
       psx_mod_read_word(player+0x17c) || (psx_mod_read_word(player+0x224)&0x241))return;
    const unsigned layout=psx_mod_read_byte(player+0x233);
    if(layout>=4)return;
    const uint32_t cross=psx_mod_read_word(0x800d1b50+4*layout);
    if(cross<0x80010000 || cross>0x801ffffc || (cross&3) ||
       (psx_mod_read_word(cross)&3)!=1)return;
    push_mask_cross(); // original idle action must not process the same press
    uint32_t saved[0x8a4/4];
    for(unsigned i=0;i<0x8a4/4;++i)saved[i]=psx_mod_read_word(player+4*i);
    int32_t lined=0;
    const bool aligned=original_call(cpu,0x8007ec4c,0x113,&lined) && (lined&0xffff);
    // The extra acquisition permission belongs to these X lanes. The original
    // line-up computes target facing at +0x24; it still rejects oblique grabs.
    const bool along=((((psx_mod_read_half(player+0x24)+512)&4095)/1024)&1)!=0;
    const bool clear=aligned && along && push_step_clear(cpu,object,0,0);
    if(!clear) {
        for(unsigned i=0;i<0x8a4/4;++i)psx_mod_write_word(player+4*i,saved[i]);
    } else {
        psx_mod_write_half(player+0x60,121);
        psx_mod_write_word(player+0x290,object);
        psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    }
    if(std::getenv("DNTTK_PUSH_TRACE"))std::fprintf(stderr,
        "ttk-push acquire object=%08x aligned=%d along=%d clear=%d\n",object,aligned,along,clear);
}
static void push_manual_release(bool orphan=false) {
    if((push_manual_object || orphan) && push_animation(psx_mod_read_half(player+0x60))) {
        // The original hold's release branch finishes the interaction. Restart
        // all channels so no walking clip survives into the next action.
        for(unsigned ch=0;ch<4;++ch) {
            const uint32_t a=player+0x60+0x14*ch;
            psx_mod_write_half(a,121);psx_mod_write_half(a+8,0);psx_mod_write_half(a+10,0);
        }
        psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x2000u);
        for(unsigned a:{0xfcu,0xfeu,0x100u})psx_mod_write_half(player+a,0);
    }
    push_manual_object=0;push_sound_distance=0;push_blocked_pad=0;
}
static void push_manual_prepare(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    if(push_manual_object && push_manual_epoch!=f.epoch)push_manual_release();
    if(!push_manual_enabled() || !f.active || !input_push_grab_owns_cross() || !push_grab_ready()) {
        const unsigned animation=psx_mod_read_half(player+0x60);
        push_manual_release(push_manual_enabled() && push_grab_ready() && (animation==119 || animation==120));return;
    }
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(!push_manual_object) {
        if(animation!=121 || psx_mod_read_half(player+0x68)!=3)return;
        push_manual_object=psx_mod_read_word(player+0x290);push_manual_epoch=f.epoch;push_sound_distance=0;
    }
    if(push_manual_object!=psx_mod_read_word(player+0x290)){push_manual_release();return;}
    const uint16_t pad=push_pad(f.move_x,f.move_y);
    const unsigned want=pad==push_blocked_pad?121:pad==16?120:pad==64?119:121;
    for(unsigned ch=0;ch<4;++ch) {
        const uint32_t a=player+0x60+0x14*ch;
        if(psx_mod_read_half(a)!=want) {
            psx_mod_write_half(a,want);psx_mod_write_half(a+8,0);psx_mod_write_half(a+10,0);
        } else if(want!=121 && psx_mod_read_half(a+8)==1) {
            const unsigned clip=psx_mod_read_half(0x800c2d64+8*want+2*ch);
            if(clip>=2048)continue;
            const uint32_t rec=psx_mod_read_word(0x800d8328+4*clip),first=rec&0xffff,count=rec>>16;
            // Loop the working part of the disc's clip, before its release.
            if(count>=6 && psx_mod_read_half(a+6)>=first+count-2) {
                const unsigned next=first+2;
                psx_mod_write_half(a+4,psx_mod_read_half(a+6));psx_mod_write_half(a+6,next);
                const unsigned ticks=psx_mod_read_word(psx_mod_read_word(0x800d8324)+8*next)>>16;
                psx_mod_write_half(a+2,ticks?ticks:50);
            }
        }
    }
}
static void push_manual_dispatch(CPUState* cpu,uint32_t ra) {
    if(cpu->gpr[4]!=player || ra!=0x8004b634 || !push_manual_enabled())return;
    const auto& f=input_snapshot(Context::Gameplay);
    // The animation update between prepare and dispatch can finish the grab.
    // Claim it here before the original dispatcher consumes an already-held
    // direction and starts its unowned, one-shot shove.
    if(!push_manual_object) {
        if(!f.active || !input_push_grab_owns_cross() || !push_grab_ready() ||
           psx_mod_read_half(player+0x60)!=121 || psx_mod_read_half(player+0x68)!=3)return;
        push_manual_object=psx_mod_read_word(player+0x290);push_manual_epoch=f.epoch;push_sound_distance=0;
    }
    if(!f.active || !input_push_grab_owns_cross() || !push_grab_ready() || push_manual_epoch!=f.epoch) {push_manual_release();return;}
    for(unsigned a:{0xfcu,0xfeu,0x100u})psx_mod_write_half(player+a,0);
    psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)&~0x2000u);
    psx_mod_write_half(player+0xb8,psx_mod_read_half(player+0xb8)&~0x1800u);
    const uint16_t pad=push_pad(f.move_x,f.move_y);
    // Hold has no original directional transition; prepare chooses the clip.
    for(uint32_t table:{0x800d1ce8u,0x800d14f8u}) {
        const uint32_t a=psx_mod_read_word(table+4*psx_mod_read_byte(player+0x233));
        if(a>=0x80010000 && a<0x80200000 && !(a&3))psx_mod_write_word(a,psx_mod_read_word(a)&~1u);
    }
    if(!pad){push_blocked_pad=0;return;}
    const unsigned axis=((psx_mod_read_half(player+0x1c)+512)&4095)/1024;
    const int sign=(axis==0 || axis==1?1:-1)*(pad==16?1:-1);
    // Simulation ticks, not presentation frames (about 900 units/second).
    const int ticks=std::min(32u,psx_mod_read_word(0x800d21fc));
    const int amount=std::max(1,ticks*3);
    int moved=0;
    while(moved<amount) {
        int step=std::min(16,amount-moved)*sign;
        if(axis&1)step=push_ladder_limit(push_manual_object,step);
        if(!step){++push_manual_blocks;break;}
        const int dx=(axis&1)?step:0,dz=(axis&1)?0:step;
        if(!push_step_clear(cpu,push_manual_object,dx,dz)){++push_manual_blocks;break;}
        for(uint32_t object:{player,push_manual_object}) {
            psx_mod_write_word(object+4,psx_mod_read_word(object+4)+dx);
            psx_mod_write_word(object+12,psx_mod_read_word(object+12)+dz);
        }
        moved+=std::abs(step);++push_manual_moves;
    }
    push_blocked_pad=moved?0:pad;
    if(moved) {
        int32_t ignored=0;hull_call(cpu,0x80095cb0,push_manual_object,&ignored);
        push_sound_distance+=moved;
        if(push_sound_distance>=180) {
            CPUState sound=*cpu;sound.gpr[5]=push_manual_object+4;sound.gpr[6]=0;
            hull_call(&sound,0x8006b270,0x7008,&ignored);push_sound_distance=0;
        }
    }
    if(std::getenv("DNTTK_PUSH_TRACE"))std::fprintf(stderr,"ttk-push object=%08x pad=%u amount=%d moved=%d blocks=%u\n",push_manual_object,pad,amount,moved,push_manual_blocks);
}
