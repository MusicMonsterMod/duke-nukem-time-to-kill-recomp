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
// (the original has no medkit box), is drawn here, the medkit with its own icon
// (D08A7, below). An off gadget's digits are the same glyph packets at colour 0x31
// instead of 0x80 (38%: PSX texture modulation). A jetpack that is on but not
// selected moves one box row up (box height + 4, the original's own step);
// Bio Mask / goggles keep their own place left of ammo. Layout changes live in
// the widescreen_hud_begin/restore window, so saves never see them.
static constexpr uint32_t gadget_box=0x800c44a4,glyph_cursor=0x800d67a8;
// D08A7 medkit icon: the user's own art, assets/ui/items/gadget-medkit-15col.png
// (tests/local/test_ui_art.py keeps these tables equal to it), as a 4bpp cell and
// palette in space the disc's HUD sheet (/DATA/FONTS.RAW at VRAM 960,0) leaves
// empty: the cell at (960,205), 4 VRAM words x 16 rows, and the palette row at
// (1008,206). Rows 205-222 stay empty in play; the game fills x 960-991 from
// row 223 down at run time (live VRAM survey, D08A7). The health box keeps the cross (record 0x800c44e4). The medkit
// record is the cross record with this cell and palette, on the saved stack.
// Each time the medkit box is drawn, the HUD ordering-table slot also gets a
// CPU-to-VRAM load of the cell and palette (GP0 A0, then a texture-cache clear),
// put in front of the icon. It goes through the game's own linked-list DMA, so
// it never interleaves with a transfer, and nothing has to survive a level load,
// a savestate (old or new) or a D17 replay worker: every drawn frame loads it.
static constexpr uint16_t medkit_cell_x=960,medkit_cell_y=205,medkit_clut_x=1008,medkit_clut_y=206;
static const uint16_t k_medkit_clut[16]={
    0x0000u,0x35cfu,0x4653u,0x14bau,0x001au,0x254bu,0x084fu,0x675bu,
    0x5af8u,0x6b7du,0x73beu,0x7bffu,0x56deu,0x463eu,0x2d9cu,0x52b6u};
static const uint32_t k_medkit_cell[32]={  // 16 rows, 2 words a row, texel 0 lowest
    0x00000000u,0x00000000u,0x43210000u,0x00001234u,0x66250000u,0x00005266u,0x00870000u,0x00008800u,
    0xed3cba97u,0x77abceecu,0xedcc7f89u,0x7827ccdcu,0xeabbf1f9u,0x721fbb9eu,0x4abbf2f8u,0x8f2fba94u,
    0x43eaf227u,0x722fbe34u,0x443af227u,0x722fb344u,0x49aaf227u,0x722faa94u,0xeaba8227u,0x722fab9eu,
    0xde9a88f8u,0xff889cedu,0xe3c91512u,0x21519ceeu,0x00000000u,0x00000000u,0x00000000u,0x00000000u};
static uint32_t gadget_icon(unsigned item,uint32_t medkit_record) {
    return item==1?0x800c44b4:item==2?0x800c44c4:item==3?0x800c4504:medkit_record;
}
// Medkit record: the cross record 0x800c44e4 (CLUT id, w, h, x, y, flags) with the
// medkit cell and palette.
static void medkit_record_write(uint32_t record) {
    for(unsigned i=0;i<16;i+=4) psx_mod_write_word(record+i,psx_mod_read_word(0x800c44e4+i));
    psx_mod_write_half(record,(uint16_t)((medkit_clut_y<<6)|(medkit_clut_x>>4)));
    psx_mod_write_word(record+4,medkit_cell_x);
    psx_mod_write_word(record+8,medkit_cell_y);
}
// The load packet: tag, A0 cell (3 + 32 words), A0 palette (3 + 8), cache clear.
static constexpr unsigned medkit_load_words=1+35+11+1;
static uint32_t medkit_load_packet() {
    // Same ring as the HUD sprite packets 0x8008b678 takes: cursor, start, end;
    // back to the start when the packet would pass the end.
    uint32_t packet=psx_mod_read_word(glyph_cursor);
    const uint32_t start=psx_mod_read_word(glyph_cursor+4),end=psx_mod_read_word(glyph_cursor+8);
    if(end<packet+4*medkit_load_words) packet=start;
    if(packet<0x80010000 || packet+4*medkit_load_words>0x80200000 || end<start+4*medkit_load_words) return 0;
    psx_mod_write_word(glyph_cursor,packet+4*medkit_load_words);
    uint32_t a=packet;
    auto put=[&](uint32_t v) { psx_mod_write_word(a,v);a+=4; };
    put((medkit_load_words-1)<<24|0xffffffu);
    put(0xa0000000u);put((uint32_t)medkit_cell_y<<16|medkit_cell_x);put(16u<<16|4u);
    for(uint32_t v:k_medkit_cell) put(v);
    put(0xa0000000u);put((uint32_t)medkit_clut_y<<16|medkit_clut_x);put(1u<<16|16u);
    for(unsigned i=0;i<16;i+=2) put((uint32_t)k_medkit_clut[i+1]<<16|k_medkit_clut[i]);
    put(0x01000000u);
    return packet;
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
    const uint32_t medkit_record=top-0x40;
    if(item==5) medkit_record_write(medkit_record);
    call(0x8008b678,gadget_icon(item,medkit_record),(uint32_t)(x+1),(uint32_t)y,0);
    call(0x8008b678,gadget_box,(uint32_t)x,(uint32_t)y,0);
    // The original's ordering-table add 0x8002bc18 (slot 0, the status bar's)
    // puts a packet at the head of its slot, so the load added last runs first.
    if(ok && item==5)
        if(const uint32_t load=medkit_load_packet()) call(0x8002bc18,0,load,0,0);
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
