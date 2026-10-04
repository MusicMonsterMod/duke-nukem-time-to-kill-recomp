// D17P draw distance (Modernized "extended"; Vanilla and "original" are
// unchanged). Three original limits produce black bands at the far end of long
// views such as the subway in UI slot 8; Vanilla, and the original hardware,
// behave the same.
//
// 1. Far limits. Each frame the camera render (0x8002e48c) copies the level's
//    view limits into the render context 0x800d67a8: +0x6c fade start (level
//    header 0x800da450), +0x70 mesh far limit (fade start + 0x13d0), +0x74
//    room/prop/actor far limit (the alternate 0x800da34c during the portal walk
//    and in some rooms, otherwise +0x70) and +0x78 the mesh depth limit
//    (+0x74 / 4). Mesh vertices fade to the fog colour over 0x1000 units past
//    the fade start, a face with a corner past +0x70 is drawn as a black quad
//    (0x80011cf4) and rooms, props and actors past +0x74 are not drawn. Far
//    away the fade is squeezed into a pixel or two, so the cut-off reads as a
//    hard black box. "extended" doubles each limit (at most 0xffff, the GTE
//    depth range; the mesh path's own guards and the 2048-entry OT clamp
//    already accept it) and moves the fade start by the same amount as the
//    mesh limit, so the original fade still ends just short of the new limit.
//    Only the render context is changed, between the render's writes and their
//    readers: the portal walk (0x8006276c), the room walk (0x80062b48) and the
//    props/actors pass (0x8002ffec) are entered right after them. Values are
//    derived from the level globals, never from the context, so applying twice
//    is harmless. The globals also drive gameplay distance checks
//    (0x800993d0, 0x800a5b84) and are untouched.
// 2. Portal rectangles (widen_portals below).
// 3. Thin distant faces. A ceiling or floor cell a pixel tall can have
//    integer corners whose NCLIP determinant is zero or of the wrong sign, so
//    the mesh renderer (0x80011894/0x80011ab4) skips it. With exact projection
//    tracking (geometry or texture precision Corrected), the runtime's precise
//    culling gives NCLIP the exact sign instead (gte_nclip_culling_set). With
//    both precisions Original nothing is tracked and NCLIP stays native.
static constexpr uint32_t view_limits=0x800d67a8, level_fade=0x800da450, level_alternate=0x800da34c;
struct DistanceGuard {uint32_t address,size;const char* digest;};
static const DistanceGuard distance_guards[]={
    {0x8002e610,0xd0,"08bea76836167641001e18d7469f0a3fb15442e9e990c99e383317dce7297d16"},
    {0x80062b48,0x2c0,"2e0537eefa1b92abf3563535274f4fa675ffbce808f0c5644f0c77f099771d76"},
};
static std::array<std::vector<uint32_t>,2> distance_words;
static uint64_t distance_frames=0,distance_refusals=0,portal_widened=0;
static int32_t distance_original=0,distance_extended=0;
static bool draw_distance_extended() {
    static const bool v=[]{const char* t=std::getenv("DNTTK_DRAW_DISTANCE");return t && !std::strcmp(t,"extended");}();
    return v;
}
// Developer: DNTTK_DRAW_DISTANCE_FAR=N replaces the doubled limit (4096..65535).
static int32_t draw_distance_override() {
    static const int32_t v=[]{const char* t=std::getenv("DNTTK_DRAW_DISTANCE_FAR");
        const long n=t?std::atol(t):0;return n>=4096 && n<=0xffff ? (int32_t)n : 0;}();
    return v;
}
static int32_t extend_limit(int32_t far) {
    const int32_t override=draw_distance_override();
    return override ? std::max(override,far) : std::min(far*2,0xffff);
}
// Portal rectangles. The portal walk (0x8006276c) lists each visible room
// with the screen rectangle of the opening it is seen through (0x800d68b0,
// 12-byte entries: x, y, w, h, room; count 0x800d68a8). The room walk
// (0x80062b48) clips that room's mesh against it: a face whose corners all lie
// beyond one edge is skipped (0x8001160c). The integer rectangles of distant
// openings fall a pixel or two short of the surfaces seen through them, so a
// whole strip of the next section's ceiling or floor is skipped and the cleared
// background shows as a thin black line across the corridor. Each rectangle
// grows by a small margin, within the view rectangle 0x800d2210; nearer walls
// around the opening are drawn later and still cover anything behind them.
// The props/actors pass (0x8002ffec) reads the same, wider list.
// Developer: DNTTK_PORTAL_MARGIN=0..8 native pixels (default 2).
static int32_t portal_margin() {
    static const int32_t v=[]{const char* t=std::getenv("DNTTK_PORTAL_MARGIN");
        const long n=t?std::atol(t):2;return n>=0 && n<=8 ? (int32_t)n : 2;}();
    return v;
}
static void widen_portals() {
    const int32_t m=portal_margin();
    const uint32_t count=psx_mod_read_word(0x800d68a8);
    if(!m || count>256) return;
    const int32_t rx=(int16_t)psx_mod_read_half(0x800d2210),ry=(int16_t)psx_mod_read_half(0x800d2212);
    const int32_t rx2=rx+(int16_t)psx_mod_read_half(0x800d2214),ry2=ry+(int16_t)psx_mod_read_half(0x800d2216);
    for(uint32_t i=0;i<count;++i) {
        const uint32_t e=0x800d68b0+12*i;
        const int32_t x=(int16_t)psx_mod_read_half(e),y=(int16_t)psx_mod_read_half(e+2);
        const int32_t w=(int16_t)psx_mod_read_half(e+4),h=(int16_t)psx_mod_read_half(e+6);
        if(w<=0 || h<=0) continue;
        const int32_t x0=std::max(rx,x-m),y0=std::max(ry,y-m),x1=std::min(rx2,x+w+m),y1=std::min(ry2,y+h+m);
        if(x1<=x0 || y1<=y0 || (x0==x && y0==y && x1==x+w && y1==y+h)) continue;
        psx_mod_write_half(e,(uint16_t)(int16_t)x0);psx_mod_write_half(e+2,(uint16_t)(int16_t)y0);
        psx_mod_write_half(e+4,(uint16_t)(int16_t)(x1-x0));psx_mod_write_half(e+6,(uint16_t)(int16_t)(y1-y0));
        ++portal_widened;
    }
}
static void draw_distance_apply(uint32_t address,uint32_t ra) {
    const bool rooms=address==0x8006276c && ra==0x8002e650;
    const bool walk=address==0x80062b48 && ra==0x8002e688;
    const bool scene=address==0x8002ffec && ra==0x8002e6dc;
    if((!rooms && !walk && !scene) || !draw_distance_extended()) return;
    if(!code_identity(distance_guards,distance_words)) {++distance_refusals;return;}
    // Developer: DNTTK_PRECISE_CULLING=0 keeps native NCLIP decisions.
    static const bool culling=[]{const char* t=std::getenv("DNTTK_PRECISE_CULLING");
        const bool on=!t || std::strcmp(t,"0");gte_nclip_culling_set(on);return on;}();(void)culling;
    const int32_t fade=(int32_t)psx_mod_read_word(level_fade);
    const int32_t alternate=(int32_t)psx_mod_read_word(level_alternate);
    const int32_t mesh=fade+0x13d0;
    const int32_t far=(int32_t)psx_mod_read_word(view_limits+0x74);
    if(fade<=0 || mesh>=0xffff || alternate<=0 || alternate>=0xffff) {++distance_refusals;return;}
    int32_t limit;
    if(far==mesh || far==extend_limit(mesh)) limit=extend_limit(mesh);
    else if(far==alternate || far==extend_limit(alternate)) limit=extend_limit(alternate);
    else {++distance_refusals;return;}
    const int32_t mesh_limit=extend_limit(mesh);
    psx_mod_write_word(view_limits+0x6c,(uint32_t)(fade+(mesh_limit-mesh)));
    psx_mod_write_word(view_limits+0x70,(uint32_t)mesh_limit);
    psx_mod_write_word(view_limits+0x74,(uint32_t)limit);
    psx_mod_write_word(view_limits+0x78,(uint32_t)(limit>>2));
    if(far!=limit) distance_original=far;
    distance_extended=limit;
    if(walk) widen_portals();
    if(scene) ++distance_frames;
}
static const char* draw_distance_debug_json() {
    static char buffer[320];
    uint64_t checks=0,flips=0;gte_nclip_culling_stats(&checks,&flips);
    std::snprintf(buffer,sizeof buffer,"{\"mode\":\"%s\",\"frames\":%llu,\"refusals\":%llu,\"original\":%d,\"extended\":%d,"
        "\"portals_widened\":%llu,\"nclip_checks\":%llu,\"nclip_flips\":%llu}",
        draw_distance_extended()?"extended":"original",(unsigned long long)distance_frames,(unsigned long long)distance_refusals,
        distance_original,distance_extended,(unsigned long long)portal_widened,(unsigned long long)checks,(unsigned long long)flips);
    return buffer;
}
