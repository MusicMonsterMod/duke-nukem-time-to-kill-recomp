// D08A6 selected gadget on the HUD (approved design A, Modernized only). The
// gadget chosen with [ / ] (the one Enter / U uses) always has a box at the
// original item slot, the place the status bar 0x8008ba30 stacks the jetpack
// box over the ammo box. Everything is the original's own drawing: either the
// status bar draws the gadget there itself, or the box is drawn by the same
// calls it makes for a gadget box, from its entry (after the D14 shift):
//   0x8008b98c number to text (charge * 100 / capacity)
//   0x8008b454 text at x + 0x800d22a4/16, y + 2, flags 0x310 (red HUD digits)
//   0x8008b678 icon record at x + 1, y, then the 46x16 box record 0x800c44a4
// Original elements (layout 0x800dd778 + 8 * element, state 0x800dd7b8):
//   element 4 jetpack (169,69) while player+0x358 bit 1 is on;
//   element 5 Bio Mask / goggles (82,89), left of ammo, while +0x35c or +0x360
//   bit 1 is on (the two never are on together; the original turns the other
//   off). Their slide-in offsets are set from the layout when they appear.
// So: a selected gadget that is on is the original box, with element 5 moved to
// the slot when it is the Bio Mask or goggles; an off gadget, or the medkit
// (the original has no medkit box), is drawn here with the health cross for the
// medkit. An off gadget's digits are the same glyph packets at colour 0x31
// instead of 0x80 (38%: PSX texture modulation). A jetpack that is on but not
// selected moves one box row up (box height + 4, the original's own step);
// Bio Mask / goggles keep their own place left of ammo. Layout changes live in
// the widescreen_hud_begin/restore window, so saves never see them.
static constexpr uint32_t gadget_box=0x800c44a4,glyph_cursor=0x800d67a8;
static uint32_t gadget_icon(unsigned item) {
    return item==1?0x800c44b4:item==2?0x800c44c4:item==3?0x800c4504:0x800c44e4;
}
static bool gadget_owned(unsigned item) {
    if(item!=1 && item!=2 && item!=3 && item!=5) return false;
    const unsigned v=psx_mod_read_half(player+0x354+4*item);
    return (v&1) && ((v&2) || (int16_t)psx_mod_read_half(player+0x356+4*item)>0);
}
// Original HUD calls on one saved stack and scratchpad (see original_call).
static bool gadget_box_draw(CPUState* source,unsigned item,int x,int y,bool lit) {
    const uint32_t top=source->gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail) return false;
    uint32_t saved[1024],scratch[256];
    const uint32_t base=(top-4096)&~3u,text=top-0x80;
    for(unsigned i=0;i<1024;++i) saved[i]=psx_mod_read_word(base+4*i);
    for(unsigned i=0;i<256;++i) scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    bool ok=true;
    auto call=[&](uint32_t address,uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3) {
        if(!ok) return;
        CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=0x800000fc;
        cpu.gpr[4]=a0;cpu.gpr[5]=a1;cpu.gpr[6]=a2;cpu.gpr[7]=a3;
        psx_mod_write_word(top-0x100+16,100);  // 0x8008b678 scale argument
        guest_busy=true;psx_dispatch_call(&cpu,address,0x800000fc);guest_busy=false;
        ok=cpu.pc==0 && !g_psx_call_bail;
    };
    const int16_t charge=(int16_t)psx_mod_read_half(player+0x356+4*item);
    const unsigned capacity=psx_mod_read_half(0x800c2712+4*item);
    const int32_t value=capacity?charge*100/(int32_t)capacity:charge;
    call(0x8008b98c,text,(uint32_t)value,0,0);
    unsigned glyphs=0;
    for(unsigned i=0;i<12 && psx_mod_read_byte(text+i);++i) ++glyphs;
    uint32_t packet=psx_mod_read_word(glyph_cursor);
    const int16_t step=(int16_t)psx_mod_read_half(0x800d22a4);
    call(0x8008b454,text,(uint32_t)(x+step/16),(uint32_t)(y+2),0x310);
    if(ok && !lit) {
        // Same walk as the glyph writer 0x8008b210: 0x28-byte packets, wrapping
        // to the buffer start when the next one would pass its end.
        const uint32_t start=psx_mod_read_word(glyph_cursor+4),end=psx_mod_read_word(glyph_cursor+8);
        for(unsigned n=0;n<glyphs;++n) {
            if(end<packet+0x28) packet=start;
            if(packet<0x80010000 || packet>0x801fffd8 || psx_mod_read_byte(packet+7)!=0x2c) break;
            for(unsigned c=4;c<7;++c) psx_mod_write_byte(packet+c,0x31);
            packet+=0x28;
        }
    }
    call(0x8008b678,gadget_icon(item),(uint32_t)(x+1),(uint32_t)y,0);
    call(0x8008b678,gadget_box,(uint32_t)x,(uint32_t)y,0);
    for(unsigned i=0;i<1024;++i) if(psx_mod_read_word(base+4*i)!=saved[i]) psx_mod_write_word(base+4*i,saved[i]);
    for(unsigned i=0;i<256;++i) if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i]) psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    return ok;
}
static void gadget_hud(CPUState* cpu) {
    if(!hud_applied || psx_mod_read_byte(player+0x233)!=0) return;
    // The selection as the original menu's remembered ID (shortcuts.inc keeps
    // it equal to selected_item): high-refresh redraws replay this function in
    // worker processes on a copy of guest memory, where host state is stale.
    const unsigned item=psx_mod_read_half(0x800c3f94);
    if(!gadget_owned(item)) return;
    const uint32_t jet=hud_layout+32,vision=hud_layout+40;
    const int16_t x=(int16_t)psx_mod_read_half(jet),y=(int16_t)psx_mod_read_half(jet+2);
    const bool on=item!=5 && (psx_mod_read_half(player+0x354+4*item)&2);
    if(item!=1 && (psx_mod_read_half(player+0x358)&2)) {
        const int row=psx_mod_read_byte(gadget_box+3)+4;
        psx_mod_write_half(jet+2,(uint16_t)(int16_t)(y-row));
    }
    if(item==1 && on) {
        // The original jetpack box is in the slot. Skip its rise out of the
        // ammo box (state +34, the slide offset; 0 is its resting value): the
        // dim box was already there, so the lit one replaces it in place.
        psx_mod_write_half(0x800dd7b8+34,0);
        return;
    }
    // Element 5 shows the goggles when they are on, else the Bio Mask.
    const unsigned vision_item=(psx_mod_read_half(player+0x360)&2)?3:2;
    if(on && item==vision_item) {
        psx_mod_write_half(vision,(uint16_t)x);
        psx_mod_write_half(vision+2,(uint16_t)y);
        return;
    }
    gadget_box_draw(cpu,item,x,y,item==5 || on);
}
