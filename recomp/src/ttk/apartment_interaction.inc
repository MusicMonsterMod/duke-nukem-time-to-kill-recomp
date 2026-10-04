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
// its contact list. For the authenticated light switch only, aim the first ray
// at its model centre within a small view cone/reach. Keep the original model
// intersection, finder ordering, callback, interaction animation and secret code.
static unsigned switch_queries;
static void apartment_switch_query(CPUState* cpu) {
    const uint32_t sp=cpu->gpr[29],object=cpu->gpr[5];
    if(cpu->gpr[31]!=0x80077e18 || cpu->gpr[4]!=sp+0x18 ||
       cpu->gpr[6]!=sp+0x38 || cpu->gpr[7]!=sp+0x48 ||
       cpu->gpr[20]!=player+0x1c || sp>0x801ffe00 ||
       psx_mod_read_word(sp+0x84)!=0x80077fa4 ||
       psx_mod_read_word(sp+0xb0)!=0x800518c0 ||
       object!=psx_mod_read_word(0x800d25b0) || !apartment_object(object,595,24) ||
       !input_snapshot(Context::Gameplay).active || !player_identity_ready() || !first_map() ||
       psx_mod_read_word(0x800be570)!=0 || psx_mod_read_word(0x800c27bc)!=1 ||
       psx_mod_read_byte(object+0x2e)!=psx_mod_read_byte(player+0x2e))return;
    const double dx=(int32_t)psx_mod_read_word(sp+0x58)-(int32_t)psx_mod_read_word(player+4);
    const double dz=(int32_t)psx_mod_read_word(sp+0x60)-(int32_t)psx_mod_read_word(player+12);
    const double dy=(int32_t)psx_mod_read_word(sp+0x5c)-(int32_t)psx_mod_read_word(player+8);
    const double distance=std::hypot(dx,dz),heading=view_yaw();
    if(distance<1 || distance>768 || std::abs(dy)>768 ||
       (dx*std::sin(heading)+dz*std::cos(heading))/distance<std::cos(35*tau/360))return;
    for(unsigned i=0;i<3;++i)psx_mod_write_word(sp+0x28+4*i,psx_mod_read_word(sp+0x58+4*i));
    // The switch model is a zero-depth plane. Cross it rather than ending
    // exactly on it, which can round to the near side at oblique angles.
    psx_mod_write_word(sp+0x28,(int32_t)psx_mod_read_word(sp+0x58)+(int32_t)std::lround(dx*64/distance));
    psx_mod_write_word(sp+0x30,(int32_t)psx_mod_read_word(sp+0x60)+(int32_t)std::lround(dz*64/distance));
    ++switch_queries;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))std::fprintf(stderr,"ttk-switch-query object=%08x distance=%.1f\n",object,distance);
}
