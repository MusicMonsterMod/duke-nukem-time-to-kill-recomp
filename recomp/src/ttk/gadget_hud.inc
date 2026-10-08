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
// the slot when it is the Bio Mask or goggles; an off gadget, the medkit or
// held steroids (the original has no box for either) is drawn here, the medkit
// and steroids with their own icons (D08A7, D08A4, below). Running steroids
// (D08A8) are drawn here too, lit, in the slot when selected, else stacked above
// the slot after an unselected jetpack; element 2 then shows only armour.
// Running power-up coins (D08A17) stack above those, a row each. An off gadget's digits are the same glyph packets at colour 0x31
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
// D08A4 steroids icon: the user's own art, assets/ui/items/hud-steroids-15col.png
// (tools/local/reduce_icon_15col.py from hud-steroids.png; test_ui_art.py keeps
// these tables equal to it), in the same empty band: the cell right of the
// medkit's at (964,205), the palette on the next row at (1008,207).
static const uint16_t k_steroids_clut[16]={
    0x0000u,0x4a31u,0x5694u,0x62f7u,0x35d1u,0x254eu,0x0007u,0x004bu,
    0x0cb2u,0x2576u,0x0028u,0x044cu,0x002bu,0x0049u,0x088du,0x39f5u};
static const uint32_t k_steroids_cell[32]={  // 16 rows, 2 words a row, texel 0 lowest
    0x32110000u,0x00001223u,0x33210000u,0x00004122u,0x23310000u,0x00005111u,0x23140000u,0x00005411u,
    0x98760000u,0x00006789u,0x98ba0000u,0x0000dc89u,0x995e0000u,0x00005599u,0x44450000u,0x00005444u,
    0xff950000u,0x00004f4fu,0xff950000u,0x0000444fu,0xfff50000u,0x0000f44fu,0xff5d0000u,0x00005444u,
    0x98c60000u,0x0000ae89u,0x98b60000u,0x0000db89u,0x98ba0000u,0x0000db89u,0x98e00000u,0x00000e89u};
// D08A17 power-up icons: the user's own art (D08A16), assets/ui/items/
// hud-invincibility-15col.png, hud-invisibility-15col.png and
// hud-double-duke-15col.png (test_ui_art.py keeps these tables equal to them),
// in the same empty band: cells right of the steroids' at (968,205), (972,205)
// and (976,205), palettes on rows 208-210 at x 1008.
static const uint16_t k_invincibility_clut[16]={
    0x0000u,0x0653u,0x05f0u,0x0611u,0x05cfu,0x016cu,0x073bu,0x0084u,
    0x071au,0x06d8u,0x179eu,0x075cu,0x0f7eu,0x0696u,0x3fbeu,0x2bbeu};
static const uint32_t k_invincibility_cell[32]={  // 16 rows, 2 words a row, texel 0 lowest
    0x33210000u,0x00002443u,0x34531000u,0x00035553u,0x68572600u,0x00127599u,0xb99471a0u,0x0d274ddcu,
    0xb1fe578eu,0x9d75fa1bu,0x36efc72fu,0xd47caeb1u,0x3effe47cu,0xd75ffee1u,0xdaaaed71u,0xd73affa9u,
    0x1551d573u,0x97345542u,0xc7988c31u,0x93abba5cu,0xe98cbc1bu,0xb1cfef6eu,0xfa9ff69fu,0xebbee8ffu,
    0xff8bcde0u,0x0e8ab8eeu,0xd5775a00u,0x00a5572du,0x774d9000u,0x00069277u,0xaaae0000u,0x0000eaacu};
static const uint16_t k_invisibility_clut[16]={
    0x0000u,0x4c2cu,0x402au,0x3008u,0x7032u,0x0801u,0x6831u,0x6030u,
    0x78f5u,0x7433u,0x7874u,0x582fu,0x79f9u,0x7977u,0x1c04u,0x7894u};
static const uint32_t k_invisibility_cell[32]={  // 16 rows, 2 words a row, texel 0 lowest
    0x11210000u,0x00002222u,0x22321000u,0x00023332u,0x66352400u,0x00125377u,0x97725180u,0x0b252bbau,
    0x41dc356cu,0x7b5e8819u,0x24cdf52du,0xb35a8c41u,0x2cd8c2e9u,0xbe3ddcc1u,0xbf88cb51u,0xb52f8d8bu,
    0x1331b3e2u,0x7e123332u,0xae766921u,0x7284983au,0xc76a4f19u,0x91adc84cu,0xdfbdd47du,0xc49cc6ddu,
    0xdd649bc0u,0x0c6846ccu,0xb3ee3f00u,0x00f3ee2bu,0x5e2b7000u,0x000472e5u,0xfffc0000u,0x0000c8fau};
static const uint16_t k_double_duke_clut[16]={
    0x0000u,0x39f1u,0x31aeu,0x252au,0x56d8u,0x0842u,0x8000u,0x6b7cu,
    0x4e95u,0x5f3au,0x77beu,0x77bdu,0x14a6u,0x6f9du,0x73bdu,0x7bdeu};
static const uint32_t k_double_duke_cell[32]={  // 16 rows, 2 words a row, texel 0 lowest
    0x11210000u,0x00002221u,0x12311000u,0x00013331u,0x44352400u,0x00126344u,0x98426170u,0x08162889u,
    0x91bb354au,0x885c7719u,0x14fe952du,0x82597b91u,0x1bbdb2c9u,0x8c3deba1u,0x8777b868u,0x8517db78u,
    0x133883c1u,0x8c123332u,0x7c444911u,0x81799739u,0xf8499989u,0x919dad9au,0xe78ed48du,0xf49ff4bbu,
    0xbe4998f0u,0x0a4794fbu,0x83553700u,0x0073c528u,0x5c284000u,0x000482c5u,0x797a0000u,0x0000f799u};

// Gadgets whose box icon is the project's own art rather than a disc record.
struct GadgetArt {unsigned item;uint16_t cell_x,cell_y,clut_x,clut_y;const uint32_t* cell;const uint16_t* clut;};
static constexpr GadgetArt gadget_art[]={
    {5,medkit_cell_x,medkit_cell_y,medkit_clut_x,medkit_clut_y,k_medkit_cell,k_medkit_clut},
    {4,964,205,1008,207,k_steroids_cell,k_steroids_clut},
};
// D08A17: the three coins run timed effects with no on-screen countdown in the
// original. Each pickup case of the dispatcher 0x80081a48 writes 6000 (20 s at
// the drain's 300 a second) to its own player halfword: type 1047
// invincibility +0x8a0 (0x80082848), 1045 invisibility +0x89c (0x8008287c),
// 1046 Double Duke +0x89e (0x800828b8); a second coin sets it back to 6000.
// Duke's update drains each by the frame step 0x800d21fc and zeroes it at the
// end (0x80042208..0x80042294). Death and a level change zero all three (the
// reset 0x8003fd00..0x8003fd08); Continue then gives 1500 invincibility
// (0x80042f58), which shows as a 25% box. dnkroz is its own flag 0x800c3cc6.
// Item numbers here are the coin types, so own_art() never matches them.
struct PowerUp {uint16_t timer;GadgetArt art;};
static constexpr int32_t powerup_full=6000;
static constexpr PowerUp powerups[]={
    {0x8a0,{1047,968,205,1008,208,k_invincibility_cell,k_invincibility_clut}},
    {0x89c,{1045,972,205,1008,209,k_invisibility_cell,k_invisibility_clut}},
    {0x89e,{1046,976,205,1008,210,k_double_duke_cell,k_double_duke_clut}},
};
static const GadgetArt* own_art(unsigned item) {
    for(const auto& art:gadget_art) if(art.item==item) return &art;
    return nullptr;
}
static uint32_t gadget_icon(unsigned item) {
    return item==1?0x800c44b4:item==2?0x800c44c4:item==3?0x800c4504:0;
}
// Own-art record: the cross record 0x800c44e4 (CLUT id, w, h, x, y, flags) with
// the art's cell and palette.
static void art_record_write(uint32_t record,const GadgetArt& art) {
    for(unsigned i=0;i<16;i+=4) psx_mod_write_word(record+i,psx_mod_read_word(0x800c44e4+i));
    psx_mod_write_half(record,(uint16_t)((art.clut_y<<6)|(art.clut_x>>4)));
    psx_mod_write_word(record+4,art.cell_x);
    psx_mod_write_word(record+8,art.cell_y);
}
// The load packet: tag, A0 cell (3 + 32 words), A0 palette (3 + 8), cache clear.
static constexpr unsigned art_load_words=1+35+11+1;
static uint32_t art_load_packet(const GadgetArt& art) {
    // Same ring as the HUD sprite packets 0x8008b678 takes: cursor, start, end;
    // back to the start when the packet would pass the end.
    uint32_t packet=psx_mod_read_word(glyph_cursor);
    const uint32_t start=psx_mod_read_word(glyph_cursor+4),end=psx_mod_read_word(glyph_cursor+8);
    if(end<packet+4*art_load_words) packet=start;
    if(packet<0x80010000 || packet+4*art_load_words>0x80200000 || end<start+4*art_load_words) return 0;
    psx_mod_write_word(glyph_cursor,packet+4*art_load_words);
    uint32_t a=packet;
    auto put=[&](uint32_t v) { psx_mod_write_word(a,v);a+=4; };
    put((art_load_words-1)<<24|0xffffffu);
    put(0xa0000000u);put((uint32_t)art.cell_y<<16|art.cell_x);put(16u<<16|4u);
    for(unsigned i=0;i<32;++i) put(art.cell[i]);
    put(0xa0000000u);put((uint32_t)art.clut_y<<16|art.clut_x);put(1u<<16|16u);
    for(unsigned i=0;i<16;i+=2) put((uint32_t)art.clut[i+1]<<16|art.clut[i]);
    put(0x01000000u);
    return packet;
}
static bool gadget_owned(unsigned item) {
    if(item==4) return steroids_owned();  // D08A4 held, D08A8 running
    if(item!=1 && item!=2 && item!=3 && item!=5) return false;
    const unsigned v=psx_mod_read_half(player+0x354+4*item);
    return (v&1) && ((v&2) || (int16_t)psx_mod_read_half(player+0x356+4*item)>0);
}
// Original HUD calls on one saved stack and scratchpad (see original_call).
// The icon is a disc record, or the project's art when art is given.
static bool hud_box_draw(CPUState* source,uint32_t icon,const GadgetArt* art,int32_t value,int x,int y,bool lit) {
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
    const uint32_t own_record=top-0x40;
    if(art) art_record_write(own_record,*art);
    call(0x8008b678,art?own_record:icon,(uint32_t)(x+1),(uint32_t)y,0);
    call(0x8008b678,gadget_box,(uint32_t)x,(uint32_t)y,0);
    // The original's ordering-table add 0x8002bc18 (slot 0, the status bar's)
    // puts a packet at the head of its slot, so the load added last runs first.
    if(ok && art)
        if(const uint32_t load=art_load_packet(*art)) call(0x8002bc18,0,load,0,0);
    for(unsigned i=0;i<1024;++i) if(psx_mod_read_word(base+4*i)!=saved[i]) psx_mod_write_word(base+4*i,saved[i]);
    for(unsigned i=0;i<256;++i) if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i]) psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    return ok;
}
static bool gadget_box_draw(CPUState* source,unsigned item,int x,int y,bool lit) {
    const int16_t charge=(int16_t)psx_mod_read_half(player+0x356+4*item);
    const unsigned capacity=psx_mod_read_half(0x800c2712+4*item);
    const int32_t value=capacity?charge*100/(int32_t)capacity:charge;
    return hud_box_draw(source,gadget_icon(item),own_art(item),value,x,y,lit);
}
static unsigned powerup_draws;
static void gadget_hud(CPUState* cpu) {
    steroids_hud_restore();
    if(!hud_applied || psx_mod_read_byte(player+0x233)!=0) return;
    // The selection as the original menu's remembered ID (shortcuts.inc keeps
    // it equal to selected_item): high-refresh redraws replay this function in
    // worker processes on a copy of guest memory, where host state is stale.
    const unsigned item=psx_mod_read_half(0x800c3f94);
    const bool selected=gadget_owned(item);
    // D08A8: running steroids are drawn here (lit, the pill icon, the same
    // percent the original showed in the armour element), never by element 2.
    const bool steroids=steroids_running();
    if(steroids) steroids_hud_hide();
    const uint32_t jet=hud_layout+32,vision=hud_layout+40;
    const int16_t x=(int16_t)psx_mod_read_half(jet),y=(int16_t)psx_mod_read_half(jet+2);
    const int row=psx_mod_read_byte(gadget_box+3)+4;
    // Boxes for gadgets that are on but not selected stack upwards from the
    // slot, a box row each: the jetpack first (the original's own element),
    // then running steroids.
    const bool jet_on=psx_mod_read_half(player+0x358)&2;
    int rows=selected?1:0;  // the selected box takes the slot
    if(selected && item!=1 && jet_on) {
        psx_mod_write_half(jet+2,(uint16_t)(int16_t)(y-row));
        ++rows;
    } else if(!selected && jet_on) rows=1;  // the original jetpack box in the slot
    if(steroids && (!selected || item!=4)) gadget_box_draw(cpu,4,x,y-row*rows++,true);
    // D08A17: running power-ups, lit, a box row each above the rest, with the
    // percent of the full 20 s, as the steroids box shows its own.
    for(const auto& p:powerups) {
        const int32_t left=(int16_t)psx_mod_read_half(player+p.timer);
        if(left>0 && hud_box_draw(cpu,0,&p.art,left*100/powerup_full,x,y-row*rows++,true)) ++powerup_draws;
    }
    if(!selected) return;
    const bool on=item!=5 && (psx_mod_read_half(player+0x354+4*item)&2);
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
    // The medkit and steroids (held or running) are always lit.
    gadget_box_draw(cpu,item,x,y,item==5 || item==4 || on);
}
static const char* powerups_debug_json() {
    static char buffer[160];
    std::snprintf(buffer,sizeof buffer,"{\"draws\":%u,\"invincibility\":%d,\"invisibility\":%d,\"double_duke\":%d}",powerup_draws,
        (int16_t)psx_mod_read_half(player+0x8a0),(int16_t)psx_mod_read_half(player+0x89c),(int16_t)psx_mod_read_half(player+0x89e));
    return buffer;
}
