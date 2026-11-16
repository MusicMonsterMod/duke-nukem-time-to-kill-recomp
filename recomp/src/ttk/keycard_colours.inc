// D08A46: Duke 3D-style coloured key cards (Modernized with Polymorph;
// DNTTK_KEYCARD_COLOURS duke3d by default, original keeps every card blue).
// Included in modern_controls.cpp after night_vision_look.inc. Note 500.
//
// Every key-slot key in TTK is one object: a flat textured quad (model 420,
// one FT4) whose 32 x 24 4-bit texture is a blue radiation-symbol card drawn
// with a 16-colour blue ramp. The mission slot decides the colour, as Duke
// 3D's blue, red and yellow cards: type 153 (inventory item 6) stays blue,
// type 154 (item 7) is red and type 155 (item 8) yellow. The held card (Duke's
// equipment 1 or insertion pose 266, item +0x3b9 7 or 8) takes the same colour. The red and yellow
// ramps (ours, index for index as bright as the blue one) sit in the second
// texture bank; the card's polygons keep their retail texels and read only
// their palette from there (near_clip, PSX_MOD_HOST_CLUT_BANK). Door logic,
// pickups and the native card are untouched; Classic and Vanilla keep blue.
namespace {
// Bank coordinates of the two palettes (16 entries each, rows below the
// security TV atlas and the prop-light textures).
constexpr int keycard_clut_x[2]={256,272},keycard_clut_y=480;
// Red and yellow, derived from the card's blue ramp: red (B, 0.55 R, 0.45 R),
// yellow (B, 0.78 B + 0.12 G, 0.5 R), each in the 15-bit format with the
// mask bit set as the original's.
constexpr uint16_t keycard_ramps[2][16]={
    {0x9d3f,0x90bf,0x8c7d,0x8877,0x801c,0x801b,0x801a,0x8019,0x8019,0x8017,0x8015,0x8012,0x800d,0x8009,0x8004,0x8000},
    {0xa37f,0x975f,0x8f1d,0x8a77,0x82fc,0x82db,0x82ba,0x8299,0x8299,0x8257,0x8235,0x81d2,0x814d,0x80e9,0x8064,0x8000},
};
constexpr unsigned keycard_type_first=153;   // 153 blue, 154 red, 155 yellow
uint64_t keycard_frames[2]={0,0};
}

bool keycard_colours_on() {
    if(!input_modernized() || ttk_world_classic()) return false;
    const char* v=ttk_live_env("DNTTK_KEYCARD_COLOURS");
    return !(v && !std::strcmp(v,"original"));
}
void keycard_paint_bank(uint16_t* bank) {
    for(int c=0;c<2;++c) for(int i=0;i<16;++i) bank[(size_t)keycard_clut_y*1024+keycard_clut_x[c]+i]=keycard_ramps[c][i];
}
// 1 red, 2 yellow, 0 blue or not a card: the colour of a mission key slot.
static int keycard_colour_of_item(unsigned item) {return item==7?1:item==8?2:0;}
// The card texture as the game holds it now (model of type 153), so a level
// whose VRAM places it elsewhere is followed: CLUT, page and UV rectangle.
static bool keycard_source(uint16_t& clut,uint16_t& page,uint8_t rect[4]) {
    const uint32_t types=psx_mod_read_word(0x800d2660),models=psx_mod_read_word(0x800ddb18);
    if(types<0x80010000u || types>=0x801f0000u || models<0x80010000u || models>=0x801f0000u) return false;
    const unsigned model=psx_mod_read_half(types+28*keycard_type_first+16);
    if(model>=1024) return false;
    const uint32_t mesh=psx_mod_read_word(models+4*model);
    if(mesh<0x80010000u || mesh>0x801ffc00u || (mesh&3)) return false;
    const uint32_t list=psx_mod_read_word(mesh+0x14);
    if(list<0x80010000u || list>0x801ffc00u || (list&3)) return false;
    const uint32_t header=psx_mod_read_word(list);
    if((header&0xffff)!=0x2c || (header>>16)!=1) return false;   // the card: one FT4
    const uint32_t t1=psx_mod_read_word(list+8),t2=psx_mod_read_word(list+12),t3=psx_mod_read_word(list+16);
    clut=(uint16_t)(t1>>16);page=(uint16_t)(t2>>16);
    const uint8_t us[4]={(uint8_t)t1,(uint8_t)t2,(uint8_t)t3,(uint8_t)(t3>>16)},vs[4]={(uint8_t)(t1>>8),(uint8_t)(t2>>8),(uint8_t)(t3>>8),(uint8_t)(t3>>24)};
    rect[0]=*std::min_element(us,us+4);rect[1]=*std::min_element(vs,vs+4);
    rect[2]=*std::max_element(us,us+4);rect[3]=*std::max_element(vs,vs+4);
    return ((page>>7)&3)==0;   // 4-bit, as the palettes are 16 entries
}
// For the object frame being drawn: the card's own CLUT, page and UV
// rectangle and the bank palette to use (0: leave it blue).
bool keycard_frame(bool duke_frame,KeycardRecolour& out) {
    out={};
    if(!keycard_colours_on()) return false;
    int colour=0;
    const uint32_t o=nv_object;
    if(duke_frame || (o && o==player)) {
        // Measured at a reader: equipment +0x3b8 is 1 and +0x3b9 the item
        // (6..10) while the card is held (upper 47), then equipment 0 with the
        // item kept for the insertion (upper +0x74 = 266).
        if(psx_mod_read_byte(player+0x3b8)==1 || psx_mod_read_half(player+0x74)==266)
            colour=keycard_colour_of_item(psx_mod_read_byte(player+0x3b9));
    } else if(o>=0x80010000u && o<=0x801ffc00u && !(o&3)) {
        const unsigned type=psx_mod_read_half(o+0x2c);
        if(type==keycard_type_first+1) colour=1; else if(type==keycard_type_first+2) colour=2;
    }
    if(!colour || !keycard_source(out.clut,out.page,out.rect)) return false;
    out.bank_clut=(uint16_t)((keycard_clut_y<<6)|(keycard_clut_x[colour-1]>>4));
    ++keycard_frames[colour-1];
    return true;
}
void keycard_colour_stats(uint64_t out[2]) {out[0]=keycard_frames[0];out[1]=keycard_frames[1];}
