// D35I: continuous wall-plane WASD, with the native per-axis attachment,
// clearance and transfer queries. Presentation choice does not own movement.
static bool wall_climb_state() {
    const unsigned a=psx_mod_read_half(player+0x60);
    return psx_mod_read_byte(player+0x22c)==3 && a>=198 && a<=205 &&
        (object_type_flags(psx_mod_read_word(player+0x17c))&0x700u)==0x100u;
}
static int wall_last_direction;
static uint64_t wall_epoch;
static unsigned wall_moves,wall_blocks;
// Native queries use world-space hand matrices evaluated before this handler.
// Translate their query-only copies after a successful axis, so a diagonal's
// second query starts at the new position. Never leave changed render matrices.
struct WallQueryPose {
    uint32_t matrices=0;unsigned count=0;uint32_t saved[64][3]{};
    WallQueryPose() {
        const uint32_t model=psx_mod_read_word(player+0x40);
        matrices=psx_mod_read_word(player+0x3c);
        if(model<0x80010000 || model>0x801fff00 || matrices<0x80010000 || matrices>0x801ff800)return;
        count=psx_mod_read_byte(model);
        if(count>64){count=0;return;}
        for(unsigned j=0;j<count;++j)for(unsigned k=0;k<3;++k)
            saved[j][k]=psx_mod_read_word(matrices+32*j+0x14+4*k);
    }
    void shift(int dx,int dy,int dz) {
        const int delta[]={dx,dy,dz};
        for(unsigned j=0;j<count;++j)for(unsigned k=0;k<3;++k) {
            const uint32_t a=matrices+32*j+0x14+4*k;
            psx_mod_write_word(a,psx_mod_read_word(a)+delta[k]);
        }
    }
    ~WallQueryPose() {
        for(unsigned j=0;j<count;++j)for(unsigned k=0;k<3;++k)
            psx_mod_write_word(matrices+32*j+0x14+4*k,saved[j][k]);
    }
};
static void wall_climb_dispatch(CPUState* cpu,uint32_t ra) {
    const char* compare=std::getenv("DNTTK_WALL_MOTION");
    if(compare && !std::strcmp(compare,"original"))return;
    if(cpu->gpr[4]!=player || ra!=0x8004b60c || psx_mod_replay_active() ||
       !input_modern_controls() || !wall_climb_state() || !traversal_input_ready())return;
    const auto& f=input_snapshot(Context::Gameplay);
    if(f.held[jump] || f.held[crouch] || (psx_mod_read_word(player+0x224)&0x20200241u))return;
    const unsigned upper=psx_mod_read_half(player+0x74);
    if(upper>=278 || (psx_mod_read_word(0x800c2824+4*upper)&8))return;
    WallQueryPose pose;if(!pose.count)return;
    const int horizontal=(f.move_x>0)-(f.move_x<0),vertical=(f.move_y>0)-(f.move_y<0);
    const int direction=horizontal+3*vertical;
    if(wall_epoch!=f.epoch || direction!=wall_last_direction) {
        // Native idle dispatch chooses the proper animation, without waiting
        // for the old directional stroke to finish. Zero input stops at once.
        psx_mod_write_half(player+0x60,198);
        psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
        wall_last_direction=direction;wall_epoch=f.epoch;
    }
    const double length=horizontal && vertical?std::sqrt(2.0):1.0;
    const double step=std::min(32u,psx_mod_read_word(0x800d21fc))/length;
    const double yaw=(psx_mod_read_half(player+0x1c)&4095)*tau/4096;
    const int dx=int(std::lround(std::cos(yaw)*horizontal*step));
    const int dz=int(std::lround(-std::sin(yaw)*horizontal*step));
    const int dy=int(std::lround(-vertical*step));
    auto axis=[&](unsigned dir,int x,int y,int z) {
        const auto state=hull_save();const uint32_t floor=psx_mod_read_word(player+0x1c8);
        psx_mod_write_half(player+0xfc,x);psx_mod_write_half(player+0xfe,y);psx_mod_write_half(player+0x100,z);
        int32_t result=-1;
        const bool called=original_call(cpu,0x8007d65c,dir,&result,0);
        if(!called || result!=0 || (object_type_flags(psx_mod_read_word(player+0x17c))&0x700u)!=0x100u) {
            hull_restore(state);psx_mod_write_word(player+0x1c8,floor);++wall_blocks;return false;
        }
        psx_mod_write_word(player+4,psx_mod_read_word(player+4)+x);
        psx_mod_write_word(player+8,psx_mod_read_word(player+8)+y);
        psx_mod_write_word(player+12,psx_mod_read_word(player+12)+z);
        pose.shift(x,y,z);++wall_moves;return true;
    };
    if(vertical)axis(vertical>0?0:1,0,dy,0);
    if(horizontal)axis(horizontal>0?3:2,dx,0,dz);
    // The original dispatcher still owns strokes, sounds, top/bottom exits
    // and release. Its animation-root motion must not add a second step.
    for(unsigned a:{0xfcu,0xfeu,0x100u})psx_mod_write_half(player+a,0);
    if(std::getenv("DNTTK_WALL_TRACE"))std::fprintf(stderr,"ttk-wall dir=%d ticks=%u moves=%u blocks=%u anim=%u\n",direction,psx_mod_read_word(0x800d21fc),wall_moves,wall_blocks,psx_mod_read_half(player+0x60));
}
