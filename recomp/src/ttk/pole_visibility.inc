// D35F1: a pole can extend out of its owning room. The native object pass
// visits only visible rooms, so looking up the attached chain can remove its
// entire mesh when the lower room leaves the portal list. Submit just that
// attachment through the native static-object renderer, only when its room
// is absent. Keep the portal list, object membership and world draws intact.
static uint32_t pole_visibility_object;
static void pole_visibility_update() {
    const unsigned mode=psx_mod_read_byte(player+0x22c),pose=psx_mod_read_half(player+0x60);
    const uint32_t object=psx_mod_read_word(player+0x17c);
    if(mode==3 && pole_object(object_type_flags(object))) pole_visibility_object=object;
    else if(mode!=8 || (pose!=191 && pose!=196)) pole_visibility_object=0;
}
static void pole_visibility_draw(CPUState* source,uint32_t ra) {
    if(ra!=0x8002e6dc || source->gpr[4]!=camera || !independent_camera() ||
       !orbit_valid || psx_mod_read_word(camera+0xa4)!=player ||
       !identity()) return;
    const unsigned mode=psx_mod_read_byte(player+0x22c),pose=psx_mod_read_half(player+0x60);
    const bool exiting=mode==8 && (pose==191 || pose==196);
    if(mode!=3 && !exiting) return;
    const uint32_t object=exiting?pole_visibility_object:psx_mod_read_word(player+0x17c);
    if(!pole_object(object_type_flags(object)) || psx_mod_read_byte(object+0x14)!=1) return;
    const int room=(int8_t)psx_mod_read_byte(object+0x2e);
    const uint32_t count=psx_mod_read_word(0x800d68a8);
    if(room<0 || count==0 || count>256) return;
    for(uint32_t i=0;i<count;++i)
        if(psx_mod_read_word(0x800d68b8+12*i)==(uint32_t)room) return;
    const uint32_t top=source->gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 ||
       g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    uint32_t saved[1024],scratch[256];
    const uint32_t base=(top-4096)&~3u;
    for(unsigned i=0;i<1024;++i)saved[i]=psx_mod_read_word(base+4*i);
    for(unsigned i=0;i<256;++i)scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=0x800000fc;
    // Native rectangle format x/y/width/height, shared by the camera and
    // portal list. The normal object renderer retains its sphere, box,
    // polygon clipping, texture transparency and world-depth ordering.
    cpu.gpr[4]=camera;cpu.gpr[5]=object;cpu.gpr[6]=0x800d2210;
    guest_busy=true;psx_dispatch_call(&cpu,0x80031d10,0x800000fc);guest_busy=false;
    for(unsigned i=0;i<1024;++i)if(psx_mod_read_word(base+4*i)!=saved[i])psx_mod_write_word(base+4*i,saved[i]);
    for(unsigned i=0;i<256;++i)if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i])psx_mod_write_word(0x1f800000+4*i,scratch[i]);
}
