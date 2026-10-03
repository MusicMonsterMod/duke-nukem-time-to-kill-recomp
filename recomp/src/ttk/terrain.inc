// Small-drop, run-off and obstructed-takeoff handoff. Original queries, gravity and swept
// motion remain authoritative; this adapter never commits a player position.
static bool terrain_probe(CPUState* cpu,uint32_t function,Vec2 direction,int32_t& result,int32_t& floor) {
    if(guest_busy || cpu->gpr[29]<0x801f4000 || cpu->gpr[29]>0x801ffff0 ||
       g_precise_mode || g_ls_mode || g_psx_call_bail)return false;
    uint8_t saved[0x60];
    for(unsigned i=0;i<sizeof saved;++i)saved[i]=psx_mod_read_byte(player+0x174+i);
    auto heading=psx_mod_read_half(player+0x1c),offset=psx_mod_read_half(player+0x2b4);
    psx_mod_write_half(player+0x1c,int(std::lround(std::atan2(direction.x,direction.z)*4096/tau))&4095);
    psx_mod_write_half(player+0x2b4,0);
    // The standing probe measures both floor and head clearance from its cached
    // foot height. Use the original animated foot (+10), not the model origin
    // (+8), which can sit roughly 512 units above it. Restore the cache afterwards.
    if(function==0x800797f4)psx_mod_write_word(player+0x1c8,psx_mod_read_word(player+0x10));
    bool ok=original_call(cpu,function,4,&result);
    floor=(int32_t)psx_mod_read_word(player+0x1c4);
    psx_mod_write_half(player+0x1c,heading);psx_mod_write_half(player+0x2b4,offset);
    for(unsigned i=0;i<sizeof saved;++i)psx_mod_write_byte(player+0x174+i,saved[i]);
    return ok;
}
static bool terrain_step_probe(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    if(f.held[crouch] || f.held[interact] || (psx_mod_read_word(player+0x224)&0x202002c1) ||
       psx_mod_read_word(player+0x20c))return false;
    int32_t result=0,floor=0;
    return terrain_probe(cpu,0x8007926c,movement(view_yaw(),f.move_x,f.move_y),result,floor) &&
        result==6 && floor>256 && floor<=768;
}
static void terrain_update(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    const unsigned animation=psx_mod_read_half(player+0x60);
    static unsigned last_animation;
    const unsigned previous=last_animation;last_animation=animation;
    if(!f.active || !player_identity_ready() || (psx_mod_read_word(player)&0x20002) ||
       (psx_mod_read_word(player+0x224)&0x202002c1) || f.held[original_aim] || f.held[crouch]) {
        short_fall=delayed_takeoff=fall_velocity_pending=small_landing=false;landing_until=0;return;
    }
    if(terrain_epoch!=f.epoch) {short_fall=delayed_takeoff=fall_velocity_pending=small_landing=false;terrain_epoch=f.epoch;}
    small_landing=short_fall && animation>=72 && animation<=79;
    if(small_landing) {landing_until=f.sequence+12;landing_epoch=f.epoch;}
    if(short_fall && animation!=108 && animation!=105)short_fall=fall_velocity_pending=false;
    if(delayed_takeoff && animation!=98)delayed_takeoff=false;
    // D08L: a run off a larger ledge (only the original run dispatcher falls
    // there; walking stops at the edge) keeps the same lease when the fall
    // directly follows a ground stride, so it departs at running speed, not
    // the original fall initializer's 3058. Capped below a running jump's
    // 10085 and without its upward impulse, so it never outreaches a jump.
    const int32_t drop=(int32_t)psx_mod_read_word(player+0x1c4);
    const bool run_off=drop>768 && input_running() && previous>=72 && previous<=79;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE") && animation==108 && !short_fall)
        std::fprintf(stderr,"ttk-runoff seq=%llu drop=%d run=%d previous=%u had=%d gap=%lld\n",(unsigned long long)f.sequence,drop,input_running(),
            previous,had_movement,(long long)(f.sequence-movement_frame));
    if(animation==108 && !short_fall && had_movement && intent_epoch==f.epoch &&
       f.sequence>=movement_frame && f.sequence-movement_frame<=8 &&
       drop>256 && (drop<=768 || run_off) && !psx_mod_read_word(player+0x20c)) {
        short_fall=true;fall_velocity_pending=true;fall_walking=!input_running();large_fall=drop>768;
        fall_speed=stride_epoch==f.epoch && (f.move_x || f.move_y)?stride_speed:0;flight_valid=true;flight_epoch=f.epoch;flight_intent=intent;air_cap=0;
        if(large_fall)++run_offs; else ++step_drops;
    }
    if(std::getenv("DNTTK_TRAVERSAL_TRACE") && input_jump_pending())
        std::fprintf(stderr,"ttk-edge seq=%llu ground=%llu epochs=%llu/%llu anim=%u short=%d state=%u vy=%d object=%08x floor=%d move=%d had=%d gap=%llu\n",
            (unsigned long long)f.sequence,(unsigned long long)ground_frame,
            (unsigned long long)f.epoch,(unsigned long long)ground_epoch,animation,short_fall,
            psx_mod_read_byte(player+0x22c),(int32_t)psx_mod_read_word(player+0x1f8),psx_mod_read_word(player+0x174),
            (int32_t)psx_mod_read_word(player+0x1c4),bool(f.move_x || f.move_y),had_movement,(unsigned long long)(f.sequence-movement_frame));
    // Separate edge grace: at most six input sequences since verified ground,
    // and only an original small drop (not a launched jump or a large fall).
    // Reuse original jump velocity/animation and clearance, never move the actor.
    // D08Z manual jump style: any run-off (large drops included, nothing
    // required of the floor ahead) within a slightly longer grace.
    const bool manual=manual_jump();
    if(animation==108 && short_fall && (!large_fall || manual) && independent_camera() && input_jump_pending() &&
       ground_frame && ground_epoch==f.epoch && f.sequence>=ground_frame &&
       f.sequence-ground_frame<=(manual?manual_edge_grace:6) && psx_mod_read_byte(player+0x22c)==9 &&
       (int32_t)psx_mod_read_word(player+0x1f8)>=0 &&
       !psx_mod_read_word(player+0x174) && (f.move_x || f.move_y)) {
        int32_t result=0,floor=0,clearance=0;
        if(original_call(cpu,0x8007765c,player+4,&clearance) && clearance>=0x370 &&
           terrain_probe(cpu,0x800797f4,flight_intent,result,floor) && result==0 &&
           (manual || (floor>=0 && floor<=768))) {
            const unsigned jump_animation=fall_walking?98:103;
            if(original_call(cpu,0x8003e2d0,jump_animation)) {
                input_take_jump();
                intent=takeoff_intent=flight_intent;movement_frame=takeoff_frame=f.sequence;
                intent_epoch=takeoff_epoch=f.epoch;had_movement=true;
                takeoff_valid=takeoff_direction=true;takeoff_obstacle=false;
                short_fall=fall_velocity_pending=small_landing=flight_valid=false;
                landing_until=0;ground_frame=0;
                psx_mod_write_half(player+0x60,jump_animation);
                psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
                ++edge_jumps;if(large_fall)++coyote_jumps;
                return;
            }
        }
    }
    // Retain the vertical arc until the original forward clearance check accepts
    // a landing surface at/below the feet. A held original takeoff direction may
    // then enter the original directional handler, without a second upward impulse.
    if(animation!=97 || !takeoff_owned() || !takeoff_direction ||
       psx_mod_read_byte(player+0x22c)!=9 || (int32_t)psx_mod_read_word(player+0x1f8)>=0 ||
       !(f.move_x || f.move_y))return;
    // A head-height ray can report clear while the feet still intersect a
    // low obstacle. Retain the original refused query's absolute support top;
    // do not start horizontal flight until the existing vertical arc clears it.
    const int32_t rise=(int32_t)psx_mod_read_word(player+0x1c4);
    if(!takeoff_obstacle && rise<0 && rise>=-2048) {
        takeoff_top=(int32_t)psx_mod_read_word(player+0x1c8)+rise;
        takeoff_obstacle=true;
    }
    if(takeoff_obstacle && (int32_t)psx_mod_read_word(player+0x10)>takeoff_top)return;
    auto direction=movement(view_yaw(),f.move_x,f.move_y);
    if(direction.x*takeoff_intent.x+direction.z*takeoff_intent.z<0.8)return;
    int32_t result=0,floor=0;
    if(!terrain_probe(cpu,0x800797f4,takeoff_intent,result,floor) || result!=0 || floor<0)return;
    delayed_vertical=(int32_t)psx_mod_read_word(player+0x1f8);
    delayed_takeoff=true;terrain_epoch=f.epoch;flight_valid=false;
    psx_mod_write_half(player+0x60,98);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    ++delayed_jumps;
}
