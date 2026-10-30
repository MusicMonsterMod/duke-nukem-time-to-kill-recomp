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
//    hard black box. "extended" raises each limit to 0xffff, the GTE depth
//    range (D17AQ; D17P doubled it, which still cut long views such as the
//    level-6 moat short; the mesh path's own guards and the 2048-entry OT
//    clamp already accept it) and moves the fade start by the same amount as
//    the mesh limit, so the original fade still ends just short of the new
//    limit.
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
// Developer: DNTTK_DRAW_DISTANCE_FAR=N replaces the maximum limit (4096..65535).
static int32_t draw_distance_override() {
    static const int32_t v=[]{const char* t=std::getenv("DNTTK_DRAW_DISTANCE_FAR");
        const long n=t?std::atol(t):0;return n>=4096 && n<=0xffff ? (int32_t)n : 0;}();
    return v;
}
static int32_t extend_limit(int32_t far) {
    const int32_t override=draw_distance_override();
    return override ? std::max(override,far) : std::max(far,0xffff);
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
// A full-height boundary into a taller sky room is an open upper edge.
// Its authored portal ends at the source room ceiling, but the independent
// camera can see the neighbour above that edge. Extend only that direct
// neighbour's top; downstream door/window apertures remain intact.
static bool open_upper_portal(uint32_t target) {
    static const bool enabled=[] {const char* e=std::getenv("DNTTK_OPEN_PORTAL_TOP");return !e || std::strcmp(e,"0");}();
    if(!enabled) return false;
    const uint32_t table=psx_mod_read_word(0x800da454),source=psx_mod_read_word(camera+0x90);
    if(source>=256 || target>=256 || source==target || table<0x80010000u || table>0x801fa800u || (table&3)) return false;
    const uint32_t a=table+source*88,b=table+target*88;
    if(!(psx_mod_read_word(b+12)&8)) return false;
    const int32_t top=(int32_t)psx_mod_read_word(a+0x14);
    if((int32_t)psx_mod_read_word(b+0x14)>=top) return false;
    const unsigned count=psx_mod_read_byte(a+0x30);
    const uint32_t portals=psx_mod_read_word(a+0x38);
    if(!count || count>32 || portals<0x80010000u || portals>0x80200000u-count*36 || (portals&3)) return false;
    const int32_t origin=(int32_t)psx_mod_read_word(a+4);
    for(unsigned i=0;i<count;++i) {
        const uint32_t p=portals+i*36;
        if((int16_t)psx_mod_read_half(p+14)!=(int32_t)target) continue;
        int32_t lo=INT32_MAX,hi=INT32_MIN;
        for(unsigned j=0;j<4;++j) {
            const int32_t y=origin+(int16_t)psx_mod_read_half(p+8*j+2);
            lo=std::min(lo,y);hi=std::max(hi,y);
        }
        if(lo==top && hi>lo) return true;
    }
    return false;
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
        const int32_t x0=std::max(rx,x-m),y0=open_upper_portal(psx_mod_read_word(e+8))?ry:std::max(ry,y-m),x1=std::min(rx2,x+w+m),y1=std::min(ry2,y+h+m);
        if(x1<=x0 || y1<=y0 || (x0==x && y0==y && x1==x+w && y1==y+h)) continue;
        psx_mod_write_half(e,(uint16_t)(int16_t)x0);psx_mod_write_half(e+2,(uint16_t)(int16_t)y0);
        psx_mod_write_half(e+4,(uint16_t)(int16_t)(x1-x0));psx_mod_write_half(e+6,(uint16_t)(int16_t)(y1-y0));
        ++portal_widened;
    }
}
// D17AQ open sky. Outdoor rooms (flags bit 3, the sky bit) have bounds that
// reach far above their walls, and the jetpack can fly up to them. From there
// Duke sees over the walls, but the portal walk only lists rooms seen through
// portals and clips them to each opening's rectangle, so neighbouring yards
// are missing and portal-seen ones are cut mid-screen and drop out as the view
// turns (Vanilla behaves the same at that height). While the camera room is a
// sky room, a sky room Duke can see over the walls is drawn through the screen
// rectangle of its pieces that he can see (a listed one widened to it, any
// other one whose bounds are in front of the camera, on screen and inside the
// room far limit (+0x74) appended with it). The room walk's own piece test
// then skips the pieces outside it; with the whole view rectangle every piece
// of every added room was drawn, which slowed the game from 30 to about 24
// frames a second over the slot-10 moat. Host
// depth (D17AM) keeps them behind nearer walls. "Over the walls" uses the
// mesh pieces' bounds (room +0x32 count, +0x40 piece pointers, piece +4 int16
// min x/y/z and +0xc max, relative to the room's minimum corner; the room walk
// tests them with 0x800365e8): a sight line from the eye to the top of one of
// the room's pieces must leave the camera room above the highest camera-room
// piece standing where it crosses the room's edge, or where none stands. On
// the ground the walls fail that test and nothing changes, so the extra cost
// (the game's own walk draws the added rooms) is paid only when it shows.
// Only the room walk, the props/actors pass and the object pass 0x800374cc
// (which gets the original list back, below) read this list, all inside the camera render, so this changes
// drawing only; the portal walk's own room bitmap is untouched. The walk
// stops at 32 entries (0x8006286c), so the list never grows past 31.
// Developer: DNTTK_OPEN_SKY=0 keeps the portal-walk list.
static uint64_t open_sky_frames=0,open_sky_opened=0,open_sky_added=0;
static uint32_t open_sky_original=0,open_sky_count=0;
static bool open_sky_enabled() {
    static const bool v=[]{const char* t=std::getenv("DNTTK_OPEN_SKY");return !t || std::strcmp(t,"0");}();
    return v;
}
// Conservative view test of one room's bounds: true unless all eight corners
// lie behind the camera or their projection misses the view rectangle.
static bool open_sky_in_view(uint32_t room_record,const int16_t m[9],const int32_t eye[3],int32_t h,
                             int32_t rx,int32_t ry,int32_t rx2,int32_t ry2,int32_t far) {
    int32_t lo[3],hi[3];
    for(int i=0;i<3;++i) {lo[i]=(int32_t)psx_mod_read_word(room_record+4*i);hi[i]=(int32_t)psx_mod_read_word(room_record+0x20+4*i);}
    if(lo[0]>hi[0] || lo[1]>hi[1] || lo[2]>hi[2]) return false;
    double nearest=0;
    for(int i=0;i<3;++i) {const double d=eye[i]<lo[i]?lo[i]-eye[i]:eye[i]>hi[i]?eye[i]-hi[i]:0;nearest+=d*d;}
    if(nearest>(double)far*far) return false;
    double x0=1e9,y0=1e9,x1=-1e9,y1=-1e9;bool front=false,behind=false;
    for(int c=0;c<8;++c) {
        const double d[3]={(double)((c&1)?hi[0]:lo[0])-eye[0],(double)((c&2)?hi[1]:lo[1])-eye[1],(double)((c&4)?hi[2]:lo[2])-eye[2]};
        double v[3];
        for(int r=0;r<3;++r) v[r]=(m[3*r]*d[0]+m[3*r+1]*d[1]+m[3*r+2]*d[2])/4096.0;
        if(v[2]<=16) {behind=true;continue;}
        front=true;
        const double sx=h*v[0]/v[2],sy=h*v[1]/v[2];
        x0=std::min(x0,sx);x1=std::max(x1,sx);y0=std::min(y0,sy);y1=std::max(y1,sy);
    }
    if(!front) return false;
    if(behind) return true; // straddles the camera plane: keep it
    return x1>=rx && x0<=rx2 && y1>=ry && y0<=ry2;
}
struct OpenSkyBox {int32_t lo[3],hi[3];};
static bool open_sky_pieces(uint32_t room_record,std::vector<OpenSkyBox>& out) {
    out.clear();
    const unsigned count=psx_mod_read_byte(room_record+0x32);
    const uint32_t list=psx_mod_read_word(room_record+0x40);
    if(!count) return true;
    if(count>255 || list<0x80010000u || list>0x80200000u-count*4 || (list&3)) return false;
    int32_t origin[3];
    for(int i=0;i<3;++i) origin[i]=(int32_t)psx_mod_read_word(room_record+4*i);
    for(unsigned i=0;i<count;++i) {
        const uint32_t piece=psx_mod_read_word(list+4*i);
        if(piece<0x80010000u || piece>0x801ffff0u || (piece&1)) return false;
        OpenSkyBox b;
        for(int k=0;k<3;++k) {
            b.lo[k]=origin[k]+(int16_t)psx_mod_read_half(piece+4+2*k);
            b.hi[k]=origin[k]+(int16_t)psx_mod_read_half(piece+12+2*k);
        }
        if(b.lo[0]>b.hi[0] || b.lo[1]>b.hi[1] || b.lo[2]>b.hi[2]) return false;
        out.push_back(b);
    }
    return true;
}
// One sight line from the eye (inside the camera room) to a target point.
static bool open_sky_clear(const int32_t eye[3],const double target[3],const int32_t lo[3],const int32_t hi[3],
                           const std::vector<OpenSkyBox>& walls) {
    double t=1.0;
    for(int k=0;k<3;k+=2) {
        const double d=target[k]-eye[k];
        if(d>0 && target[k]>hi[k]) t=std::min(t,(hi[k]-eye[k])/d);
        else if(d<0 && target[k]<lo[k]) t=std::min(t,(lo[k]-eye[k])/d);
    }
    if(t>=1.0) return true; // the target is above or inside the camera room's footprint
    const double p[3]={eye[0]+t*(target[0]-eye[0]),eye[1]+t*(target[1]-eye[1]),eye[2]+t*(target[2]-eye[2])};
    double top=1e18;
    for(const auto& w:walls)
        if(p[0]>=w.lo[0]-32 && p[0]<=w.hi[0]+32 && p[2]>=w.lo[2]-32 && p[2]<=w.hi[2]+32) top=std::min(top,(double)w.lo[1]);
    return p[1]<top+128; // Y grows downward: above the highest wall there
}
struct OpenSkyView {int16_t m[9];int32_t eye[3],h,rect[4];};
// Grow r (x0,y0,x1,y1) by the projection of one box; a box reaching behind
// the camera takes the whole view rectangle.
static void open_sky_project(const OpenSkyBox& b,const OpenSkyView& v,double r[4]) {
    for(int c=0;c<8;++c) {
        const double d[3]={(double)((c&1)?b.hi[0]:b.lo[0])-v.eye[0],(double)((c&2)?b.hi[1]:b.lo[1])-v.eye[1],(double)((c&4)?b.hi[2]:b.lo[2])-v.eye[2]};
        double q[3];
        for(int k=0;k<3;++k) q[k]=(v.m[3*k]*d[0]+v.m[3*k+1]*d[1]+v.m[3*k+2]*d[2])/4096.0;
        if(q[2]<=16) {for(int k=0;k<4;++k) r[k]=v.rect[k];return;}
        const double sx=v.h*q[0]/q[2],sy=v.h*q[1]/q[2];
        r[0]=std::min(r[0],sx);r[1]=std::min(r[1],sy);r[2]=std::max(r[2],sx);r[3]=std::max(r[3],sy);
    }
}
// The screen rectangle of the pieces seen over the walls, clamped to the view
// with an 8-pixel margin; false when none is seen or it misses the view.
static bool open_sky_seen(const OpenSkyView& v,const int32_t lo[3],const int32_t hi[3],const std::vector<OpenSkyBox>& walls,
                          const std::vector<OpenSkyBox>& pieces,int32_t out[4]) {
    double r[4]={1e9,1e9,-1e9,-1e9};
    for(const auto& b:pieces) {
        const double y=b.lo[1];
        const double points[5][3]={{(b.lo[0]+b.hi[0])/2.0,y,(b.lo[2]+b.hi[2])/2.0},{(double)b.lo[0],y,(double)b.lo[2]},
            {(double)b.hi[0],y,(double)b.lo[2]},{(double)b.lo[0],y,(double)b.hi[2]},{(double)b.hi[0],y,(double)b.hi[2]}};
        for(const auto& q:points) if(open_sky_clear(v.eye,q,lo,hi,walls)) {open_sky_project(b,v,r);break;}
    }
    if(r[0]>r[2]) return false;
    out[0]=(int32_t)std::max<double>(v.rect[0],std::floor(r[0])-8);out[1]=(int32_t)std::max<double>(v.rect[1],std::floor(r[1])-8);
    out[2]=(int32_t)std::min<double>(v.rect[2],std::ceil(r[2])+8);out[3]=(int32_t)std::min<double>(v.rect[3],std::ceil(r[3])+8);
    return out[2]>out[0] && out[3]>out[1];
}
static void open_sky_write(uint32_t e,const int32_t r[4]) {
    psx_mod_write_half(e,(uint16_t)(int16_t)r[0]);psx_mod_write_half(e+2,(uint16_t)(int16_t)r[1]);
    psx_mod_write_half(e+4,(uint16_t)(int16_t)(r[2]-r[0]));psx_mod_write_half(e+6,(uint16_t)(int16_t)(r[3]-r[1]));
}
static void open_sky_rooms() {
    if(!open_sky_enabled()) return;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c),source=psx_mod_read_word(camera+0x90);
    if(!rooms || rooms>128 || source>=rooms || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return;
    if(!(psx_mod_read_word(table+source*88+12)&8)) return;
    uint32_t count=psx_mod_read_word(0x800d68a8);
    if(count>31) return;
    const uint16_t rx=psx_mod_read_half(0x800d2210),ry=psx_mod_read_half(0x800d2212);
    const uint16_t rw=psx_mod_read_half(0x800d2214),rh=psx_mod_read_half(0x800d2216);
    if((int16_t)rw<=0 || (int16_t)rh<=0) return;
    OpenSkyView v;int32_t lo[3],hi[3],r[4];
    for(int i=0;i<9;++i) v.m[i]=(int16_t)psx_mod_read_half(camera+0x20+2*i);
    for(int i=0;i<3;++i) v.eye[i]=(int32_t)psx_mod_read_word(camera+0x14+4*i);
    const int32_t projection=(int16_t)psx_mod_read_half(camera+0x42);
    v.h=projection>=64 && projection<=1024 ? projection : 92;
    v.rect[0]=(int16_t)rx;v.rect[1]=(int16_t)ry;v.rect[2]=(int16_t)rx+(int16_t)rw;v.rect[3]=(int16_t)ry+(int16_t)rh;
    const uint32_t home=table+source*88;
    for(int i=0;i<3;++i) {lo[i]=(int32_t)psx_mod_read_word(home+4*i);hi[i]=(int32_t)psx_mod_read_word(home+0x20+4*i);}
    static std::vector<OpenSkyBox> walls,pieces;
    if(!open_sky_pieces(home,walls)) return;
    bool listed[128]={};
    for(uint32_t i=0;i<count;++i) {
        const uint32_t e=0x800d68b0+12*i,room=psx_mod_read_word(e+8);
        if(room>=rooms) continue;
        listed[room]=true;
        if(room==source || !(psx_mod_read_word(table+room*88+12)&8)) continue;
        if(!open_sky_pieces(table+room*88,pieces) || !open_sky_seen(v,lo,hi,walls,pieces,r)) continue;
        const int32_t x=(int16_t)psx_mod_read_half(e),y=(int16_t)psx_mod_read_half(e+2);
        const int32_t w=(int16_t)psx_mod_read_half(e+4),hh=(int16_t)psx_mod_read_half(e+6);
        if(w>0 && hh>0) {r[0]=std::min(r[0],x);r[1]=std::min(r[1],y);r[2]=std::max(r[2],x+w);r[3]=std::max(r[3],y+hh);}
        if(r[0]==x && r[1]==y && r[2]==x+w && r[3]==y+hh) continue;
        open_sky_write(e,r);
        ++open_sky_opened;
    }
    const int32_t far=(int32_t)psx_mod_read_word(view_limits+0x74);
    for(uint32_t room=0;room<rooms && count<31;++room) {
        const uint32_t a=table+room*88;
        if(listed[room] || !(psx_mod_read_word(a+12)&8)) continue;
        if(!open_sky_in_view(a,v.m,v.eye,v.h,v.rect[0]-16,v.rect[1]-16,v.rect[2]+16,v.rect[3]+16,far)) continue;
        if(!open_sky_pieces(a,pieces) || pieces.empty() || !open_sky_seen(v,lo,hi,walls,pieces,r)) continue;
        const uint32_t e=0x800d68b0+12*count;
        open_sky_write(e,r);
        psx_mod_write_word(e+8,room);++count;++open_sky_added;
    }
    open_sky_original=psx_mod_read_word(0x800d68a8);open_sky_count=count;
    psx_mod_write_word(0x800d68a8,count);
    ++open_sky_frames;
}
// The object pass 0x800374cc (ra 0x8002e6e4, after the room walk and the
// props/actors pass) runs each listed room's object handlers. For the added
// rooms it drew nothing visible over the slot-10 moat but cost 2 game frames
// a second, so it gets the portal walk's own list back.
// Developer: DNTTK_OPEN_SKY_OBJECTS=1 keeps the added rooms for it.
#include "host_visibility.inc"
static void open_sky_object_pass(uint32_t ra) {
    if(host_vis_object_pass(ra)) {open_sky_count=0;return;}
    static const bool keep=[]{const char* t=std::getenv("DNTTK_OPEN_SKY_OBJECTS");return t && !std::strcmp(t,"1");}();
    if(keep || ra!=0x8002e6e4u || !open_sky_count || psx_mod_read_word(0x800d68a8)!=open_sky_count) return;
    psx_mod_write_word(0x800d68a8,open_sky_original);open_sky_count=0;
}
static void draw_distance_apply(CPUState* cpu,uint32_t address,uint32_t ra) {
    const bool rooms=address==0x8006276c && ra==0x8002e650;
    const bool walk=address==0x80062b48 && ra==0x8002e688;
    const bool scene=address==0x8002ffec && ra==0x8002e6dc;
    if(scene) host_vis_scene();
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
    // D17AR2: the host world pass supersedes the portal widening and open sky.
    if(walk && !host_vis_walk(cpu)) {widen_portals();open_sky_rooms();}
    if(scene) ++distance_frames;
}
static const char* enemy_reach_debug_json();
static const char* draw_distance_debug_json() {
    static char buffer[2048];
    uint64_t checks=0,flips=0;gte_nclip_culling_stats(&checks,&flips);
    std::snprintf(buffer,sizeof buffer,"{\"mode\":\"%s\",\"frames\":%llu,\"refusals\":%llu,\"original\":%d,\"extended\":%d,"
        "\"portals_widened\":%llu,\"nclip_checks\":%llu,\"nclip_flips\":%llu,"
        "\"open_sky\":{\"frames\":%llu,\"opened\":%llu,\"added\":%llu},\"host_visibility\":%s,\"enemies\":%s}",
        draw_distance_extended()?"extended":"original",(unsigned long long)distance_frames,(unsigned long long)distance_refusals,
        distance_original,distance_extended,(unsigned long long)portal_widened,(unsigned long long)checks,(unsigned long long)flips,
        (unsigned long long)open_sky_frames,(unsigned long long)open_sky_opened,(unsigned long long)open_sky_added,host_visibility_debug_json(),enemy_reach_debug_json());
    return buffer;
}
