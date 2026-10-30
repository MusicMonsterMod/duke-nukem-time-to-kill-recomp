// LEVEL00's bed covers an already-existing pickup. The original proximity scan
// ignores that concealment and can collect it during the pillow landing animation.
// Leave the original object and inventory untouched; route ONLY this concealed
// pickup call through the authenticated dispatcher's unknown-type no-op branch.
static bool apartment_object(uint32_t object,unsigned type,unsigned kind) {
    return object>=0x800e0000 && object<=0x801f0000 && !(object&3) &&
        psx_mod_read_half(object+0x2c)==type && psx_mod_read_byte(object+0x34)==kind;
}
static uint32_t ignored_pickup;
static unsigned concealed_pickups;
static void apartment_pickup(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    const uint32_t object=cpu->gpr[5],bed=psx_mod_read_word(0x800d257c);
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x8007fe6c || !f.active ||
       !player_identity_ready() || !first_map() || psx_mod_read_word(0x800be570)!=0 ||
       psx_mod_read_word(0x800c27bc)!=1 || (psx_mod_read_word(0x800dd878)&0x400) ||
       !apartment_object(bed,5,11) || !apartment_object(object,58,0) ||
       psx_mod_read_byte(object+0x15)!=1 ||
       psx_mod_read_byte(object+0x2e)!=psx_mod_read_byte(bed+0x2e))return;
    if(!ignored_pickup)ignored_pickup=psx_mod_alloc_guest_memory(0x40,16);
    if(!ignored_pickup)return;
    psx_mod_write_half(ignored_pickup+0x2c,0xffff);
    cpu->gpr[5]=ignored_pickup;++concealed_pickups;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))
        std::fprintf(stderr,"ttk-concealed-pickup object=%08x bed=%08x\n",object,bed);
}
// The original finder tests three thin forward rays against objects already in
// its contact list. For the authenticated light switch and for mirrors (D35N1
// below) only, aim the first ray at the model centre within a small view
// cone/reach. Keep the original model intersection, finder ordering, callback,
// interaction animation and secret code.
// The first ray (0x80076330 from 0x80077e18) of the E use finder's object
// test (0x80077ccc via 0x80077f28, from the idle Action 0x80051890), in a
// one-player Modernized game, for an object in Duke's room.
static bool use_finder_first_ray(CPUState* cpu) {
    const uint32_t sp=cpu->gpr[29],object=cpu->gpr[5];
    return cpu->gpr[31]==0x80077e18 && cpu->gpr[4]==sp+0x18 &&
       cpu->gpr[6]==sp+0x38 && cpu->gpr[7]==sp+0x48 &&
       cpu->gpr[20]==player+0x1c && sp<=0x801ffe00 &&
       psx_mod_read_word(sp+0x84)==0x80077fa4 &&
       psx_mod_read_word(sp+0xb0)==0x800518c0 &&
       input_snapshot(Context::Gameplay).active && player_identity_ready() &&
       psx_mod_read_word(0x800c27bc)==1 &&
       psx_mod_read_byte(object+0x2e)==psx_mod_read_byte(player+0x2e);
}
// Aim that ray through the object's model centre when it lies within reach
// and a 35-degree view cone; the original model intersection still decides.
static bool use_finder_aim(CPUState* cpu,double reach,double* range=nullptr) {
    const uint32_t sp=cpu->gpr[29];
    const double dx=(int32_t)psx_mod_read_word(sp+0x58)-(int32_t)psx_mod_read_word(player+4);
    const double dz=(int32_t)psx_mod_read_word(sp+0x60)-(int32_t)psx_mod_read_word(player+12);
    const double dy=(int32_t)psx_mod_read_word(sp+0x5c)-(int32_t)psx_mod_read_word(player+8);
    const double distance=std::hypot(dx,dz),heading=view_yaw();
    if(distance<1 || distance>reach || std::abs(dy)>768 ||
       (dx*std::sin(heading)+dz*std::cos(heading))/distance<std::cos(35*tau/360))return false;
    for(unsigned i=0;i<3;++i)psx_mod_write_word(sp+0x28+4*i,psx_mod_read_word(sp+0x58+4*i));
    // Switch and mirror models are zero-depth planes. Cross them rather than
    // ending exactly on them, which can round to the near side at oblique angles.
    psx_mod_write_word(sp+0x28,(int32_t)psx_mod_read_word(sp+0x58)+(int32_t)std::lround(dx*64/distance));
    psx_mod_write_word(sp+0x30,(int32_t)psx_mod_read_word(sp+0x60)+(int32_t)std::lround(dz*64/distance));
    if(range)*range=distance;
    return true;
}
static unsigned switch_queries;
static void apartment_switch_query(CPUState* cpu) {
    const uint32_t object=cpu->gpr[5];
    if(!use_finder_first_ray(cpu) || object!=psx_mod_read_word(0x800d25b0) ||
       !apartment_object(object,595,24) || !first_map() ||
       psx_mod_read_word(0x800be570)!=0)return;
    double distance;
    if(!use_finder_aim(cpu,768,&distance))return;
    ++switch_queries;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))std::fprintf(stderr,"ttk-switch-query object=%08x distance=%.1f\n",object,distance);
}
// D35N1/D35N2: mirrors (interaction classes 11 and 15, which select pose 278
// and its quote at 0x80051644) are authored behind furniture in several rooms
// (a slot-6 counter, the Old West dresser), so the 512-unit finder ray never
// reaches them and E does nothing. Extend only their first ray to 896 units;
// the original callback, line-up, pose, quote and repeat rules run unchanged.
static unsigned mirror_queries;
static bool mirror_object(uint32_t object) {
    const uint32_t table=psx_mod_read_word(0x800d2660);
    const unsigned type=psx_mod_read_half(object+0x2c);
    // 1062 types of 28 bytes (0x8001b634, as push.inc's object_type_count).
    if(table<0x80010000u || table>=0x80200000u || (table&3) || type>=0x7428u/28u)return false;
    const unsigned use_class=psx_mod_read_byte(table+28*type+4);
    return (use_class==11 || use_class==15) && (psx_mod_read_word(table+28*type)&0x00400000u);
}
static void mirror_use_query(CPUState* cpu) {
    const uint32_t object=cpu->gpr[5];
    if(object<0x80010000u || object>=0x80200000u || (object&3) ||
       !use_finder_first_ray(cpu) || !mirror_object(object))return;
    double distance;
    if(!use_finder_aim(cpu,896,&distance))return;
    ++mirror_queries;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))std::fprintf(stderr,"ttk-mirror-query object=%08x type=%u distance=%.1f\n",
        object,psx_mod_read_half(object+0x2c),distance);
}
