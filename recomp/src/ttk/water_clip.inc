// D17AF: native water tiles have their own FT4 builder, outside the ordinary
// mesh renderer. Recover the authored grid corners before GTE saturation and
// clip emitted oversized tiles, preserving their native painter slot/material.
// Included inside near_clip.cpp's anonymous namespace after object().
uint64_t water_seen=0,water_taken=0,water_guard_reject=0,water_grid_reject=0;
bool water_render_identity() {
    struct Guard {uint32_t address,size;const char* digest;};
    static const Guard guards[]={{0x8002ef90u,0xcc0u,
        "52db23011c61c794e5d5f5923fdf405242612f7ff8251d5e1734d151fc0b1daa"}};
    static std::array<std::vector<uint32_t>,1> words;
    static IdentityMemo memo;
    if(!memo.valid(input_host_frame(),g_dirty_ram_code_gen))
        memo.set(input_host_frame(),g_dirty_ram_code_gen,
            code_identity(guards,words,psx_mod_read_word,g_psx_ram));
    return memo.ok;
}
bool water_live() {
    return input_modernized() && env_int("DNTTK_WATER_CLIP",1,0,1) && enabled() &&
        (first_person_view_live() || widescreen_near_clip_live()) &&
        identity() && water_render_identity();
}
bool water_valid(uint32_t p,unsigned n) {return p>=0x80010000u && p<=0x80200000u-n && !(p&3);}
// D17AM4: each grid vertex (16-byte records: SXY, SZ, flags) is flagged 0x10
// when its SZ is under H/2 (0x8002f508/0x8002f5b0/0x8002f65c, after reading
// H through 0x800b4d2c), and a tile with any such corner is skipped before it
// is built (0x8002f70c..0x8002f71c), so it never reaches the clip below. In
// the eye view that is the tile under Duke and those beside him: the water
// around him vanished while he walked through it. Its screen-rectangle bits
// come from saturated projections and are meaningless too. While this path
// is live, a near vertex's flags are cleared once stored, so the original
// builds the tile and water_clip() clips it (or drops it, never drawing its
// saturated packet). The current row is cleared up to the vertex being
// flagged; its last vertex, and the previous row, are cleared again when the
// row's tiles are built. Far and screen culling of other tiles is unchanged.
uint64_t water_near_cleared=0,water_near_dropped=0;
void water_clear_near(uint32_t row,unsigned count) {
    if(!water_valid(row,16*count)) return;
    for(unsigned i=0;i<count;++i) {
        const uint32_t flags=row+16*i+8;
        if(psx_mod_read_word(flags)&0x10) {psx_mod_write_word(flags,0);++water_near_cleared;}
    }
}
unsigned water_row_vertices(uint32_t region) {
    int span=(int16_t)psx_mod_read_half(region+8)-(int16_t)psx_mod_read_half(region);
    if(span<0) span+=0x3ff;
    return (unsigned)std::clamp((span>>10)+1,0,33);
}
void water_near_flags(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    const unsigned done=ra==0x8002f508u?0:ra==0x8002f5b0u?1:ra==0x8002f65cu?2:3;
    if(done==3 || !water_live()) return;
    const uint32_t sp=cpu->gpr[29],region=cpu->gpr[21],row=cpu->gpr[30],triple=cpu->gpr[18];
    if(!water_valid(sp,0x94) || !water_valid(region,16) || triple<row || ((triple-row)&15)) return;
    const unsigned count=water_row_vertices(region);
    water_clear_near(psx_mod_read_word(sp+0x60),count);
    water_clear_near(row,std::min(count,(triple-row)/16+done));
}
void water_clip(CPUState* cpu) {
    if(cpu->gpr[31]!=0x8002fbb4u) return;
    ++water_seen;
    if(!water_live()) {++water_guard_reject;return;}
    auto valid=water_valid;
    const uint32_t packet=cpu->gpr[5],region=cpu->gpr[21],sp=cpu->gpr[29];
    const uint32_t row0=cpu->gpr[17],row1=cpu->gpr[18];
    const unsigned column=cpu->gpr[20];
    if(!valid(packet,40) || !valid(region,16) || !valid(sp,0x94) ||
       !valid(row0,32) || !valid(row1,32) || column>=32 ||
       psx_mod_read_word(sp+0x90)+8!=region) {++water_guard_reject;return;}
    // The rest of both rows, for this row's later tiles (see above).
    const unsigned count=water_row_vertices(region);
    water_clear_near(psx_mod_read_word(sp+0x60),count);
    water_clear_near(cpu->gpr[30],count);
    // A tile the original skips (a corner nearer than H/2) is drawn only
    // through the clip, never as its saturated native packet.
    const uint32_t records[]={row0,row0+16,row1,row1+16};
    bool skipped=false;
    for(uint32_t r:records) skipped|=psx_mod_read_word(r+4)*2<=(uint32_t)(cpu->gte_ctrl[26]&0xffff);
    auto drop=[&] {
        if(!skipped) return;
        psx_mod_write_word(packet,psx_mod_read_word(packet)&0xffffffu);++water_near_dropped;
    };
    const uint32_t command=psx_mod_read_word(packet+4);
    if((command&0xfd000000u)!=0x2c000000u || (psx_mod_read_word(packet)>>24)!=9) {drop();return;}
    Frame f{};if(!begin(f,cpu,true) || budget(f)<takeover_reserve+24*0x28) {drop();return;}
    // s1 is the previous Z row and s2 the current row. Signed 16-bit wrapping
    // matches the original SVECTOR stores, including the water-height phase.
    const int x=(int16_t)(psx_mod_read_half(region)+1024*column);
    const int y=(int16_t)(psx_mod_read_half(region+2)+psx_mod_read_half(0x800c3cd8u));
    const int z=(int16_t)psx_mod_read_half(sp+0x58);
    f.verts.resize(4);
    bool unsafe=false;int idx[]={0,1,2,3};
    for(unsigned i=0;i<4;++i) {
        f.verts[i]=near_project(f.g,(int16_t)(x+(i&1)*1024),y,(int16_t)(z-(i<2?1024:0)));
        // Fail closed if register/grid interpretation no longer matches the
        // actual native builder. Never unproject an already saturated corner.
        if(f.verts[i].sxy!=psx_mod_read_word(records[i]) ||
           f.verts[i].sz!=psx_mod_read_word(records[i]+4)) {++water_grid_reject;drop();return;}
        unsafe|=!f.verts[i].safe;
    }
    if(!unsafe && !oversize(f,idx,4)) return;
    // Painter depth only; texture perspective retains positive Z. A tile the
    // original skips has corners at SZ 0 behind the eye, so its native slot
    // is far too near and painter order would lay it over nearer scenery: it
    // is depth-tested at its surface instead, 32 units deep, so scenery just
    // under the waterline stays over it as the original paints it (the slot-10
    // channel walkway's grating lies about 10-15 units below the surface).
    f.sky=!skipped;
    Poly p{};p.source=packet;p.count=4;p.textured=true;p.actor=true;
    if(skipped) p.depth_push=32;
    p.native_order=true;p.native_slot=clamp((int32_t)cpu->gpr[4],0,0x7ff);
    p.command=0x34000000u|(command&0x03000000u);
    p.clut=psx_mod_read_half(packet+14);p.tpage=psx_mod_read_half(packet+22);
    for(unsigned i=0;i<4;++i) {
        p.idx[i]=i;p.colors[i]=command&0xffffffu;
        p.uv[i]=psx_mod_read_half(packet+12+8*i);
    }
    trace_begin(f,cpu);trace_vertices(f);
    draw(f,p);
    ++water_taken;
    // Retain a zero-word link at the native packet's original location. The
    // original insertion and ring advance remain intact; only its raster work
    // has moved to clipped host packets in the same native OT slot.
    psx_mod_write_word(packet,psx_mod_read_word(packet)&0xffffffu);
}
