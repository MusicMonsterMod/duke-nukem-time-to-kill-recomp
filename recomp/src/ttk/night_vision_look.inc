// D08A44: Duke 3D-style night vision look (Modernized with Polymorph; NIGHT
// VISION LOOK, DNTTK_NIGHT_VISION_LOOK duke3d by default, original keeps
// TTK's). Included in modern_controls.cpp after sleeping_enemies.inc. Note 499.
//
// TTK's goggles: while player+0x224 bit 0x80000 is set, the player update
// calls 0x80040dd8, which stores one flat colour in 0x800c0bb8 (green 0x8000,
// or 0x7000 on alternate frames by the hblank counter's parity) and sets
// 0x800c270c. Every 3D renderer then draws that colour in place of its
// vertex light (the small-model face walk 0x8002d848 pins its colour to it
// through 0x800c270c), so textures show only their green and the darkness
// goes. 0x8002e850, after the frame's draw, clears 0x800c0bb8.
//
// Duke 3D (EDuke32 r10669, measured in note 499): the world keeps its light
// and shade and is shown through a green-lifted palette (SLIMEPAL); every
// enemy is drawn unshaded in palette 6, a pale-lime negative (dark texels
// brightest). Here:
// - The composition entry (0x80026164, before frame_replay publishes the
//   frame to the redraw workers) clears 0x800c270c and the goggles' colour,
//   so the world, Duke and his weapon draw with their own light.
// - The presented picture (and the HUD inside it) takes a colour lookup
//   built from our rule below (Polymorph present LUT, no extra pass).
// - Creatures (type table class 6, the hostile AI; Duke never) drawn through
//   the object loop are tagged for the renderer's remap (near_clip host
//   vertices, PSX_MOD_HOST_REMAP).
// Charge, sound, toggling and saved state stay native (D08A23, D08A25);
// Classic and Vanilla keep TTK's look.
namespace {
constexpr uint32_t nv_effect_colour=0x800c0bb8u, nv_effect_flag=0x800c270cu, nv_game_mode=0x800bcbb0u;
// The object loop's draw routines by object kind (table 0x800c0bf8), each
// called (camera, object); 0x8001ca4c is the loop's step after each.
constexpr uint32_t nv_draws[]={0x800632b0u,0x80031d10u,0x80032e78u,0x800348d8u,0x80031c14u};
uint32_t nv_object=0,nv_object_sp=0;
bool nv_object_glows=false;
uint64_t nv_cleared=0,nv_glow_objects=0;
uint8_t nv_lut[32*32*32*3];
bool nv_lut_built=false;
}

bool night_vision_look_duke3d() {
    if(!input_modernized() || ttk_world_classic()) return false;
    const char* v=ttk_live_env("DNTTK_NIGHT_VISION_LOOK");
    return !(v && !std::strcmp(v,"original"));
}
// The goggles' effect is on in the game's current state (a gameplay frame).
bool night_vision_look_live() {
    return night_vision_look_duke3d() && psx_mod_read_half(nv_game_mode)==1 &&
        (psx_mod_read_word(player+0x224)&0x80000u)!=0;
}
// Composition entry: the frame draws without TTK's flat green.
void night_vision_look_compose() {
    nv_object=0;nv_object_glows=false;
    if(!night_vision_look_live()) return;
    const uint32_t c=psx_mod_read_word(nv_effect_colour);
    if(c==0x8000u || c==0x7000u) psx_mod_write_word(nv_effect_colour,0);
    if(psx_mod_read_word(nv_effect_flag)) psx_mod_write_word(nv_effect_flag,0);
    ++nv_cleared;
}
// Object loop: which object is being drawn, and whether it glows.
void night_vision_object_entry(CPUState* cpu,uint32_t) {
    nv_object=cpu->gpr[5];nv_object_sp=cpu->gpr[29];nv_object_glows=false;
    if(!night_vision_look_live() || nv_object==player || nv_object<0x80010000u || nv_object>0x801ffc00u || (nv_object&3)) return;
    const uint32_t types=psx_mod_read_word(0x800d2660),type=psx_mod_read_half(nv_object+0x2c);
    if(types<0x80010000u || types>=0x801f0000u || type>=1024) return;
    nv_object_glows=psx_mod_read_byte(types+28*type+6)==6;
    nv_glow_objects+=nv_object_glows;
}
void night_vision_object_step(CPUState* cpu,uint32_t) {
    // A list step inside an object's draw (below its frame) is not the end.
    if(nv_object && cpu->gpr[29]<nv_object_sp) return;
    nv_object=0;nv_object_glows=false;
}
void night_vision_draw_end() {nv_object=0;nv_object_glows=false;}
bool night_vision_glow_object() {return nv_object_glows;}
// The world's colour: only green changes, lifted toward red and with the mid
// tones, as Duke 3D's night-vision palette shows the world (fitted to the
// user's EDuke32 recording, research/screencaps/nv-sound.mp4, and SLIMEPAL's
// behaviour; note 499): G' = G + max(0, w . (1, L, L^2, (R-G)+, (B-G)+,
// (G-R)+, R(1-B), B)), display values 0..1, L the luma.
const uint8_t* night_vision_lut() {
    if(nv_lut_built) return nv_lut;
    static constexpr double w[8]={0.0222,0.5779,-0.7633,0.8319,-0.5926,-0.1063,-0.2773,0.0093};
    // User, 2026-11-16: a tiny touch more green overlay so night vision is not
    // forgotten in well-lit places, not brighter. Red and blue scaled down,
    // green raised by exactly the luma they lose (dark stays dark).
    // Developer DNTTK_NV_TINT="r,b" (scales, 1,1 = none).
    double tr=0.93,tb=0.90;
    if(const char* t=std::getenv("DNTTK_NV_TINT")) std::sscanf(t,"%lf,%lf",&tr,&tb);
    tr=std::clamp(tr,0.5,1.0);tb=std::clamp(tb,0.5,1.0);
    for(int b=0;b<32;++b) for(int g=0;g<32;++g) for(int r=0;r<32;++r) {
        const double R=r/31.0,G=g/31.0,B=b/31.0,L=0.299*R+0.587*G+0.114*B;
        const double f=w[0]+w[1]*L+w[2]*L*L+w[3]*std::max(R-G,0.0)+w[4]*std::max(B-G,0.0)+w[5]*std::max(G-R,0.0)+w[6]*R*(1.0-B)+w[7]*B;
        double G2=G+std::max(0.0,f);
        const double R2=R*tr,B2=B*tb;
        G2=std::min(1.0,G2+(0.299*(R-R2)+0.114*(B-B2))/0.587);
        uint8_t* o=nv_lut+((b*32+g)*32+r)*3;
        o[0]=(uint8_t)std::lround(R2*255.0);o[1]=(uint8_t)std::lround(G2*255.0);o[2]=(uint8_t)std::lround(B2*255.0);
    }
    nv_lut_built=true;
    return nv_lut;
}
void night_vision_look_stats(uint64_t out[2]) {out[0]=nv_cleared;out[1]=nv_glow_objects;}
