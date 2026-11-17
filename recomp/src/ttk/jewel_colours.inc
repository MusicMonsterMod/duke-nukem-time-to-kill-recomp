// D08A45: the three Family Jewels of level 6 in the energy crystals' blue,
// green and red (Modernized with Polymorph; DNTTK_JEWEL_COLOURS=original keeps
// them pink). Included in modern_controls.cpp after keycard_colours.inc. Note
// 501.
//
// Types 854, 855 and 856 (inventory items 14, 15, 16) each have their own
// model (1595-1597 in level 6), all drawn with the same 13 textured faces and
// four 4-bit palettes, every one a single-hue pink ramp. A ramp of the same
// brightness at another hue recolours a palette entry for entry, as D08A46 did
// for the key cards: 854 blue, 855 green, 856 red. The recoloured ramps (ours:
// hue set to the crystal icon's, saturation lifted so the pale facets do not
// read pink again) sit in the second texture bank at y 481; a jewel face keeps
// its retail texels and reads only its palette from there (near_clip,
// PSX_MOD_HOST_CLUT_BANK). The test is the mesh being drawn, not the object
// type, so any object showing a jewel model takes its colour. Pickups, mission
// flags and the level's logic are untouched; Classic and Vanilla keep pink.
namespace {
constexpr int jewel_clut_x=256,jewel_clut_y=481;   // 12 palettes, x 256-447
constexpr unsigned jewel_type_first=854,jewel_palettes=4;
// [colour][palette in first-use order within the jewel model][entry].
constexpr uint16_t jewel_ramps[3][jewel_palettes][16]={
    {   // blue
        {0xf1c4,0xf5c4,0xf9e5,0xf9e5,0xfdc3,0xfdc3,0xfdc3,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
        {0xe9a4,0xed60,0xf160,0xf580,0xf580,0xf980,0xf980,0xf980,0xfd80,0xfd80,0xfd80,0xfd80,0x8000,0x8000,0x8000,0x8000},
        {0xd920,0xe140,0xe540,0xe940,0xed60,0xf160,0xf160,0xf580,0xf980,0xf980,0xfd80,0xfd80,0x8000,0x8000,0x8000,0x8000},
        {0xf68e,0xfeae,0xfe6c,0xfe6c,0xfe6c,0xfe8d,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
    },
    {   // green
        {0x9b24,0x9b43,0x9f44,0x9f44,0x9763,0x9763,0x9763,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
        {0x9ae3,0x8f00,0x8f20,0x8f40,0x8f40,0x8f40,0x8f40,0x8f40,0x8f60,0x8f60,0x8f60,0x8f60,0x8000,0x8000,0x8000,0x8000},
        {0x8a60,0x8aa0,0x8ec0,0x8ee0,0x8f00,0x8f20,0x8f20,0x8f40,0x8f40,0x8f40,0x8f60,0x8f60,0x8000,0x8000,0x8000,0x8000},
        {0xbb4c,0xbb6d,0xb36a,0xb36a,0xb36a,0xb76b,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
    },
    {   // red
        {0x90bc,0x90bd,0x94be,0x94be,0x8c9f,0x8c9f,0x8c9f,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
        {0x90ba,0x803b,0x803c,0x803d,0x803d,0x803e,0x803e,0x803e,0x803f,0x803f,0x803f,0x803f,0x8000,0x8000,0x8000,0x8000},
        {0x8036,0x8038,0x8039,0x803a,0x803b,0x803c,0x803c,0x803d,0x803e,0x803e,0x803f,0x803f,0x8000,0x8000,0x8000,0x8000},
        {0xb9dd,0xb9ff,0xb19f,0xb19f,0xb19f,0xb5bf,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
    },
};
uint64_t jewel_frames[3]={0,0,0};
// The mesh of type t's model, or 0 (the original's own tables, as SetObjectType).
uint32_t jewel_type_mesh(unsigned t) {
    const uint32_t types=psx_mod_read_word(0x800d2660),models=psx_mod_read_word(0x800ddb18);
    if(types<0x80010000u || types>=0x801f0000u || models<0x80010000u || models>=0x801f0000u) return 0;
    const unsigned model=psx_mod_read_half(types+28*t+16);
    if(model>=4096) return 0;
    const uint32_t mesh=psx_mod_read_word(models+4*model);
    return mesh>=0x80010000u && mesh<=0x801ffc00u && !(mesh&3)?mesh:0;
}
}

bool jewel_colours_on() {
    if(!input_modernized() || ttk_world_classic()) return false;
    const char* v=ttk_live_env("DNTTK_JEWEL_COLOURS");
    return !(v && !std::strcmp(v,"original"));
}
void jewel_paint_bank(uint16_t* bank) {
    for(int c=0;c<3;++c) for(unsigned k=0;k<jewel_palettes;++k) for(int i=0;i<16;++i)
        bank[(size_t)jewel_clut_y*1024+jewel_clut_x+16*(c*jewel_palettes+k)+i]=jewel_ramps[c][k][i];
}
// For the mesh being drawn: when it is a jewel's, its four palettes in
// first-use order (the order of the ramps above) and their bank replacements.
bool jewel_frame(uint32_t mesh,JewelRecolour& out) {
    out={};
    if(mesh<0x80010000u || mesh>0x801ffc00u || (mesh&3)) return false;
    int colour=-1;
    for(int c=0;c<3 && colour<0;++c) if(jewel_type_mesh(jewel_type_first+c)==mesh) colour=c;
    if(colour<0 || !jewel_colours_on()) return false;
    const uint32_t list=psx_mod_read_word(mesh+0x14);
    if(list<0x80010000u || list>0x801ffc00u || (list&3)) return false;
    unsigned found=0;
    uint32_t cursor=list;
    for(unsigned guard=0;;++guard) {
        if(guard>=16) return false;
        const uint32_t header=psx_mod_read_word(cursor);cursor+=4;
        const uint32_t type=header&0xffff,n=header>>16;
        if(type==0xff) break;
        const unsigned size=type==0x24 || type==0x2c?16:type==0x34 || type==0x3c?20:type==0x20 || type==0x28?8:type==0x30 || type==0x38?12:0;
        if(!size || n>64) return false;
        for(uint32_t k=0;k<n;++k,cursor+=size) {
            if(!(type&4)) continue;   // untextured
            const uint16_t clut=(uint16_t)(psx_mod_read_word(cursor+4)>>16),page=(uint16_t)(psx_mod_read_word(cursor+8)>>16);
            if((page>>7)&3) return false;   // the ramps are 16 entries: 4-bit only
            bool seen=false;for(unsigned j=0;j<found;++j) seen|=out.clut[j]==clut;
            if(seen) continue;
            if(found==jewel_palettes) return false;   // not the model measured
            out.clut[found++]=clut;
        }
    }
    if(found!=jewel_palettes) return false;
    for(unsigned k=0;k<jewel_palettes;++k)
        out.bank_clut[k]=(uint16_t)((jewel_clut_y<<6)|((jewel_clut_x+16*(colour*jewel_palettes+k))>>4));
    ++jewel_frames[colour];
    return true;
}
void jewel_colour_stats(uint64_t out[3]) {for(int c=0;c<3;++c) out[c]=jewel_frames[c];}
