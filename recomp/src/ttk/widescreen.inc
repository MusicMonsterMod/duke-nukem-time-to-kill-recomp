// D14 widescreen (Modernized only). On PSX the framework clamps the display
// aspect to 4:3 unless a trusted activation plugin selects another one after the
// mod plan is committed and before the window exists. The preloaded package
// dnttk.presentation.widescreen enables this plugin; it only acts when the
// launcher passes the Modernized profile's choice (DNTTK_WIDESCREEN=16:9, 16:10,
// 21:9 or auto), so Vanilla and "off" keep the original 4:3 picture. The
// runtime then renders natively wide (game.local.toml [widescreen]): the GTE
// projection is unchanged and the extra columns are real rendering, presented
// 1:1. The rest of this file makes TTK fill and use them.
static void widescreen_activate() {
    const char* mode=std::getenv("DNTTK_INPUT_MODE");
    const char* wide=std::getenv("DNTTK_WIDESCREEN");
    if(!mode || std::strcmp(mode,"modernized") || !wide || !std::strcmp(wide,"off")) return;
    // "auto" follows the window between 4:3 and 21:9 (a 16:9 window to start).
    if(!std::strcmp(wide,"auto")) {
        psx_mod_set_fixed_display_aspect(16,9);
        psx_mod_set_adaptive_display_aspect(21,9);
        return;
    }
    unsigned num=0,den=0;char tail=0;
    if(std::sscanf(wide,"%u:%u%c",&num,&den,&tail)==2) psx_mod_set_fixed_display_aspect(num,den);
}

// Visibility. TTK is a portal engine: the camera render (0x8002e48c) walks
// rooms through portals clipped against the root screen rectangle 0x800d2210
// (x,y,w,h), and rooms, props, actors and effects are culled against the
// rectangles derived from it. 0x8001f9f4 rewrites it every frame as
// (-256,-120,512,240) from the 512x240 screen mode. Widen it by the margin at
// the render's projection load (0x800b4d9c from 0x8002e4c8), just before the
// portal walk, so everything in the revealed columns is drawn; the next frame's
// rewrite restores it. DNTTK_WS_RECT=0 (developer) keeps the 4:3 rectangle.
static void widescreen_view_rect(uint32_t ra) {
    if(ra!=0x8002e4d0) return;
    static const bool off=[]{const char* t=std::getenv("DNTTK_WS_RECT");return t && !std::strcmp(t,"0");}();
    if(off) return;
    const int32_t m=psx_mod_widescreen_x_margin();
    if(m<=0 || m>512) return;
    const uint32_t r=0x800d2210;
    const int16_t x=(int16_t)psx_mod_read_half(r),w=(int16_t)psx_mod_read_half(r+4);
    if(x!=-256 || w!=512) return;
    psx_mod_write_half(r,(uint16_t)(int16_t)(x-m));
    psx_mod_write_half(r+4,(uint16_t)(int16_t)(w+2*m));
}

// D14: in third person the wider view shows near walls/floors the original
// renderer drops (a corner at or behind the eye) or tears. The D11B
// conservative near clip draws exactly those polygons; run it whenever the
// widescreen margins are live. DNTTK_WS_NEAR=0 (developer) keeps the original.
static bool widescreen_active() {return psx_mod_widescreen_x_margin()>0;}
bool widescreen_near_clip_live() {
    static const bool off=[]{const char* t=std::getenv("DNTTK_WS_NEAR");return t && !std::strcmp(t,"0");}();
    return !off && input_modernized() && widescreen_active();
}

// D14 HUD anchoring. The status bar layout 0x800dd778 holds (x,y) pairs
// relative to the screen centre (two per element, one per player slot), built
// once at level start by 0x8008c7b0 from the 512x240 view rectangle: health
// (-246,89), armour (-165,89), ammo (169,89), jetpack (169,69), Bio Mask /
// goggles (82,89) and so on. While the status bar 0x8008ba30 draws, left
// elements move left and right elements right by the widescreen margin so they
// sit in the wide corners; the screen-mode call 0x8001fc44 that follows it
// restores the game's values, so saves, menus and 4:3 never see the shift. One
// player only (split screen keeps its own layout). D08A6 moves gadget boxes in
// the same saved window (gadget_hud.inc), so the whole layout is saved.
static constexpr uint32_t hud_layout=0x800dd778;
static constexpr unsigned hud_pairs=16;
static uint32_t hud_saved[hud_pairs];
static bool hud_applied=false;
static void widescreen_hud_restore() {
    if(!hud_applied) return;
    for(unsigned i=0;i<hud_pairs;++i) psx_mod_write_word(hud_layout+4*i,hud_saved[i]);
    hud_applied=false;
}
static void widescreen_hud_begin(uint32_t ra) {
    widescreen_hud_restore();
    if(ra!=0x80026590 || psx_mod_read_word(0x800c27bc)>=2) return;
    for(unsigned i=0;i<hud_pairs;++i) hud_saved[i]=psx_mod_read_word(hud_layout+4*i);
    hud_applied=true;
    const int32_t m=psx_mod_widescreen_x_margin();
    if(m<=0 || m>512) return;
    for(unsigned i=0;i<hud_pairs;++i) {
        const int16_t x=(int16_t)psx_mod_read_half(hud_layout+4*i);
        const int16_t y=(int16_t)psx_mod_read_half(hud_layout+4*i+2);
        if(!x && !y) continue;  // unused slot
        if(x<-32) psx_mod_write_half(hud_layout+4*i,(uint16_t)(int16_t)(x-m));
        else if(x>32) psx_mod_write_half(hud_layout+4*i,(uint16_t)(int16_t)(x+m));
    }
}
