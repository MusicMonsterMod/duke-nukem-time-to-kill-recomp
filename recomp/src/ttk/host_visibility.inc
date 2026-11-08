// D17AR2 host-owned world visibility (on by default since D17AR3; developer
// DNTTK_HOST_VISIBILITY=0 turns it off; design:
// documentation/336-d17ar1-host-visibility-design.md). Modernized
// Extended draw distance with unified geometry only; Vanilla and "original"
// never reach it.
//
// The camera render (0x8002e48c) runs the portal walk (0x8006276c), which
// lists each room seen through a portal with the screen rectangle of the
// opening (0x800d68b0, 12-byte entries x, y, w, h, room; count 0x800d68a8),
// then the room walk (0x80062b48), which for each entry sets the room
// transform (0x80029660), the flat room colour and the clip rectangle, tests
// each mesh piece's bounds against the rectangle (0x800365e8) and draws the
// survivors (0x80011020). A room whose opening has left the screen, or that is
// seen over a wall rather than through a portal, is not listed: the pop-in and
// edge cut-offs of D17P/D17Y/D17AQ.
//
// Here, at the room walk's entry, the host chooses the pieces itself and draws
// them through the near-clip emission (near_clip_host_world_piece) with the
// state the room walk would have set:
// - rooms: a host portal walk from the camera room over the level's own room
//   graph (room +0x30 portal count, +0x38 36-byte portals: four corners of
//   int16 x/y/z (and a fourth half) relative to the room's minimum corner,
//   in 2x2 grid order like a PS1 quad, +6 flags, bit 1 a
//   closed portal the game skips too, +0xe target room) in floating point,
//   starting from a guard-banded view rectangle (wider than the screen, so a
//   room whose opening is just off screen still counts), with each room's
//   rectangle the union of the openings it is seen through. While the camera
//   room is a sky room, sky rooms seen over its walls join with the rectangle
//   of their pieces seen (the D17AQ open-sky test, guard-banded).
// - pieces: every piece of a selected room whose bounds (room +0x32 count,
//   +0x40 pointers, piece +4 int16 min and +0xc max relative to the room's
//   minimum corner) project into that room's rectangle, or reach behind the
//   eye; drawn nearest first, so distant pieces get the packet budget left
//   after the near ones and the props/actors share (near_clip.cpp
//   after_walk_reserve) instead of going back to the original renderer.
// - state: R = camera matrix (camera +0x20), TR = R * (room min - eye) as
//   0x80029660 computes it (ApplyMatrixLV, halving offsets of 32768 or more),
//   context +0x50 = 0x300 with bit 1 and +0x54 for a flat-coloured room (or
//   the global flat colour 0x800c0bb8), bit 7 (the rectangle test) set
//   against the host's room rectangle.
// Every room the portal walk listed is kept too, its rectangle the union of
// both, so the host never draws less than the game would (game_missed counts
// rooms the host walk alone did not reach). The portal walk's rectangles are
// read unwidened: D17P/D17Y portal widening and D17AQ open-sky list rewriting
// belong to Classic (draw_distance.inc) and run only when this pass does not
// (D17AR4).
// The game's walk then gets only the rooms of pieces the frame could not
// afford (normally none), drawing only those pieces; the props/actors pass
// (0x8002ffec) gets the host room list (game-listed rooms first, each
// rectangle widened to the host one within the view, at most 31), and the
// object pass (0x800374cc) gets the portal walk's own list back, since its
// handlers affect play (D17AQ). The portal walk itself is untouched.
// Developer: DNTTK_HOST_VIS_GUARD=0..100 (percent of the half view added on
// each side, default 15).
struct HostVisGuard {uint32_t address,size;const char* digest;};
static const HostVisGuard host_vis_guards[]={
    {0x80029660,0x18c,"5a9150b197569cf2f186bc6cf69f963b446b28541c7ff481eecc1fa120280261"},
    {0x800b444c,0x160,"704b7d8ebd6995ceb4134b7c68371678756cff1becec9850d930302d8aeabd02"},
    {0x8006276c,0x3dc,"c6544cdc51f5ba439a9e802458f8418e87a252021d487c3da59b21090fbe1c89"},
    // D17AZ: the object pass and its static-object renderer (reach draws).
    {0x800374cc,0x2ac,"0a623badc1e3360e74e61e7da97d2010ddc21cc1e89bd4d34c45a5f0cbe5c71a"},
    {0x80031d10,0x654,"8ed8575e0fd05c6291209498ae990fb0048fd0cfb15990e8cc7914cfa9fe96b2"},
};
static std::array<std::vector<uint32_t>,5> host_vis_words;
static uint64_t host_vis_frames=0,host_vis_refusals=0,host_vis_rooms=0,host_vis_extra=0,host_vis_tested=0,host_vis_selected=0,host_vis_left=0;
static uint64_t host_vis_redraw_only=0;
static uint64_t host_vis_missed=0,host_vis_widened=0,host_vis_select_us=0,host_vis_draw_us=0;
static uint32_t host_vis_last_missed=0,host_vis_last_missed_from=0;
// D17AR4: openings given an open upper edge (D17Y) by the host walk.
static uint64_t host_vis_open_tops=0;
// D17AR4 audit: entries checked, past the host rectangle, open upper edges,
// open upper edges past the host's top; the largest overshoot and last room.
static uint64_t host_vis_audit[4]={};static double host_vis_audit_max=0;static uint32_t host_vis_audit_room=0;
// D17AX: last frame's selected rooms (bit 8: drew a piece); fixed storage,
// since the debug server reads it from another thread.
static uint16_t host_vis_last_order[256];static uint32_t host_vis_last_order_n=0;
static uint32_t host_vis_last_rooms=0,host_vis_last_pieces=0,host_vis_last_game=0;
// Developer console "hostvis [on|off|default]" overrides the switch for the
// session. Redraw workers get the live frame's value with each job
// (RenderState), so high-refresh frames follow the toggle too.
static std::atomic<int> host_vis_override{-1};
// D40: without either, the live renderer decides (Classic: off).
static bool host_visibility_enabled() {
    static const int v=ttk_env_switch("DNTTK_HOST_VISIBILITY");
    const int o=host_vis_override.load(std::memory_order_relaxed);
    return o>=0 ? o!=0 : v>=0 ? v!=0 : !ttk_world_classic();
}
// D17BB: over-the-walls rooms come only from the camera room's open-air
// region: the sky rooms joined to it through the level's own portals, sky
// room to sky room. A castle's moat ring is one region and each walled
// courtyard another, so the moat never shows courtyard walls poking over the
// outer wall and a courtyard never shows the moat's (or another courtyard's)
// wall skins over its own: the two skins of a castle wall belong to different
// rooms and were built to be seen only from their own side. Developer:
// DNTTK_OVER_WALLS=0 (none), all (every sky room, as before D17BB), default
// region; console "hostvis walls off|all|region|default" (main process only).
// Classic's open-sky list (draw_distance.inc) follows the same rule (D17BC).
static std::atomic<int> host_vis_walls_override{-1};
static int host_vis_over_walls() { // 0 none, 1 region, 2 all
    static const int v=[]{const char* t=std::getenv("DNTTK_OVER_WALLS");
        if(!t || !*t) return 1;
        return !std::strcmp(t,"0") ? 0 : !std::strcmp(t,"all") ? 2 : 1;}();
    const int o=host_vis_walls_override.load(std::memory_order_relaxed);
    return o<0 ? v : o;
}
// The open-air region of a sky room: sky rooms reached through portals.
static void host_vis_sky_region(uint32_t table,uint32_t rooms,uint32_t source,std::vector<bool>& region) {
    region.assign(rooms,false);
    std::vector<uint32_t> stack{source};region[source]=true;
    while(!stack.empty()) {
        const uint32_t room=stack.back();stack.pop_back();
        const uint32_t a=table+room*88;
        const unsigned count=psx_mod_read_byte(a+0x30);
        const uint32_t portals=psx_mod_read_word(a+0x38);
        if(!count || count>64 || portals<0x80010000u || portals>0x80200000u-count*36 || (portals&3)) continue;
        for(unsigned i=0;i<count;++i) {
            const int32_t target=(int16_t)psx_mod_read_half(portals+i*36+14);
            if(target<0 || (uint32_t)target>=rooms || region[target]) continue;
            if(!(psx_mod_read_word(table+(uint32_t)target*88+12)&8)) continue;
            region[target]=true;stack.push_back((uint32_t)target);
        }
    }
}
static uint64_t host_vis_region_skipped=0;
static double host_vis_guard() {
    static const double v=[]{const char* t=std::getenv("DNTTK_HOST_VIS_GUARD");
        const long n=t?std::atol(t):15;return (n>=0 && n<=100 ? n : 15)/100.0;}();
    return v;
}
struct HostVisEntry {uint32_t room;int16_t r[4];};
// Saved game list (restored for the object pass) and the props pass list.
static HostVisEntry host_vis_game[32],host_vis_props[32];
static uint32_t host_vis_game_count=0,host_vis_props_count=0;
static bool host_vis_live=false;
// D17AT6: every room the host showed in the frame being drawn (game-listed
// rooms included). Small models such as blood pools are drawn only for rooms
// set in the portal walk's bitset (0x800e1f10, see host_visibility_room_shown).
static std::vector<bool> host_vis_shown;
static bool host_vis_shown_valid=false;
struct HostVisCamera {double m[9];double eye[3];double h;};
// D17AZ: the live frame's camera and guard-banded view (view-rect coordinates
// centred on the GTE offset), for the object reach test at the object pass.
static HostVisCamera host_vis_frame_cam;static double host_vis_frame_guard[4];
// View-space position (x right, y down, z forward) of a world point.
static void host_vis_view(const HostVisCamera& c,const double p[3],double v[3]) {
    const double d[3]={p[0]-c.eye[0],p[1]-c.eye[1],p[2]-c.eye[2]};
    for(int r=0;r<3;++r) v[r]=(c.m[3*r]*d[0]+c.m[3*r+1]*d[1]+c.m[3*r+2]*d[2])/4096.0;
}
static constexpr double host_vis_near=16.0;
// Screen bounds (x0,y0,x1,y1, view-rect coordinates) of a polygon in view
// space after clipping at the near plane; false when it lies behind it.
static bool host_vis_bounds(const double (*v)[3],int n,double b[4]) {
    b[0]=b[1]=1e18;b[2]=b[3]=-1e18;bool any=false;
    for(int i=0;i<n;++i) {
        const double* a=v[i];const double* c=v[(i+1)%n];
        auto add=[&](const double* p) {
            const double x=p[0]/p[2],y=p[1]/p[2];
            b[0]=std::min(b[0],x);b[2]=std::max(b[2],x);b[1]=std::min(b[1],y);b[3]=std::max(b[3],y);any=true;
        };
        if(a[2]>=host_vis_near) add(a);
        if((a[2]>=host_vis_near)!=(c[2]>=host_vis_near)) {
            const double t=(host_vis_near-a[2])/(c[2]-a[2]);
            const double q[3]={a[0]+t*(c[0]-a[0]),a[1]+t*(c[1]-a[1]),host_vis_near};
            add(q);
        }
    }
    return any;
}
// Box test: true when a box projects into rect r, or reaches behind the near
// plane without lying wholly outside one side of r's frustum.
static bool host_vis_box(const HostVisCamera& c,const double lo[3],const double hi[3],const double r[4]) {
    double v[8][3];bool behind=false;
    for(int k=0;k<8;++k) {
        const double p[3]={(k&1)?hi[0]:lo[0],(k&2)?hi[1]:lo[1],(k&4)?hi[2]:lo[2]};
        host_vis_view(c,p,v[k]);behind|=v[k][2]<host_vis_near;
    }
    bool front=false;for(auto& q:v) front|=q[2]>=host_vis_near;
    if(!front) return false;
    if(behind) {
        // Straddles the near plane: keep it unless all corners lie past one
        // side plane of the rectangle's frustum.
        for(int side=0;side<4;++side) {
            bool out=true;
            for(auto& q:v) {
                const double s=side==0?q[0]-r[0]/c.h*q[2]:side==1?q[1]-r[1]/c.h*q[2]:side==2?r[2]/c.h*q[2]-q[0]:r[3]/c.h*q[2]-q[1];
                out&=s<0;
            }
            if(out) return false;
        }
        return true;
    }
    double x0=1e18,y0=1e18,x1=-1e18,y1=-1e18;
    for(auto& q:v) {const double sx=c.h*q[0]/q[2],sy=c.h*q[1]/q[2];x0=std::min(x0,sx);x1=std::max(x1,sx);y0=std::min(y0,sy);y1=std::max(y1,sy);}
    return x1>=r[0] && x0<=r[2] && y1>=r[1] && y0<=r[3];
}
// 0x80029660 / ApplyMatrixLV: the room translation exactly as the game sets it.
static void host_vis_translation(const int16_t m[9],const int32_t room_min[3],const int32_t eye[3],int32_t tr[3]) {
    int32_t d[3];bool halve=false;
    for(int i=0;i<3;++i) {d[i]=(int32_t)((uint32_t)room_min[i]-(uint32_t)eye[i]);halve|=((d[i]<0?d[i]+0x7fff:d[i])>>15)!=0;}
    if(halve) for(int i=0;i<3;++i) d[i]=(d[i]+(d[i]<0))>>1;
    int32_t hi[3],lo[3];
    for(int i=0;i<3;++i) {
        if(d[i]<0) {hi[i]=-((-d[i])>>15);lo[i]=-((-d[i])&0x7fff);} else {hi[i]=d[i]>>15;lo[i]=d[i]&0x7fff;}
    }
    for(int r=0;r<3;++r) {
        int64_t a=0,b=0;
        for(int k=0;k<3;++k) {a+=(int64_t)m[3*r+k]*hi[k];b+=(int64_t)m[3*r+k]*lo[k];}
        tr[r]=(int32_t)((uint32_t)((int32_t)a)<<3)+(int32_t)(b>>12);
        if(halve) tr[r]=(int32_t)((uint32_t)tr[r]<<1);
    }
}
struct HostVisRoom {double r[4];bool on=false;unsigned visits=0;};
static void host_vis_union(double a[4],const double b[4]) {a[0]=std::min(a[0],b[0]);a[1]=std::min(a[1],b[1]);a[2]=std::max(a[2],b[2]);a[3]=std::max(a[3],b[3]);}
// D17AX developer rooms (DNTTK_HOST_VIS_ALL below): their pieces are drawn
// after every other piece and dropped, never handed to the game's walk, when
// the frame cannot afford them; the game's own packet area is sized for the
// original view, and overrunning it corrupted the GPU stream. They stay out
// of the props/actors list and the shown set (their objects belong to the
// game, and gameplay reads that list).
static std::vector<bool> host_vis_developer;
static uint64_t host_vis_developer_dropped=0;
// Room selection: host portal walk plus, from a sky room, sky rooms seen over
// the walls. Returns the selected rooms in discovery order.
static void host_vis_select(const HostVisCamera& c,uint32_t table,uint32_t rooms,uint32_t source,const double guard[4],
                            std::vector<HostVisRoom>& sel,std::vector<uint32_t>& order) {
    sel.assign(rooms,HostVisRoom{});order.clear();host_vis_developer.assign(rooms,false);
    std::vector<uint32_t> queue;
    std::copy(guard,guard+4,sel[source].r);sel[source].on=true;order.push_back(source);queue.push_back(source);
    for(size_t qi=0;qi<queue.size() && qi<4096;++qi) {
        const uint32_t room=queue[qi],a=table+room*88;
        const unsigned count=psx_mod_read_byte(a+0x30);
        const uint32_t portals=psx_mod_read_word(a+0x38);
        if(!count || count>64 || portals<0x80010000u || portals>0x80200000u-count*36 || (portals&3)) continue;
        double origin[3];for(int i=0;i<3;++i) origin[i]=(int32_t)psx_mod_read_word(a+4*i);
        for(unsigned i=0;i<count;++i) {
            const uint32_t p=portals+i*36;
            if(psx_mod_read_half(p+6)&2) continue;
            const int32_t target=(int16_t)psx_mod_read_half(p+14);
            if(target<0 || (uint32_t)target>=rooms || (uint32_t)target==room) continue;
            // Corners are stored as a 2x2 grid (like a PS1 quad); walk the
            // outline 0, 1, 3, 2 for the near-plane clip.
            static constexpr unsigned outline[4]={0,1,3,2};
            double v[4][3];
            for(unsigned k=0;k<4;++k) {
                const unsigned j=outline[k];
                const double w[3]={origin[0]+(int16_t)psx_mod_read_half(p+8*j),origin[1]+(int16_t)psx_mod_read_half(p+8*j+2),origin[2]+(int16_t)psx_mod_read_half(p+8*j+4)};
                host_vis_view(c,w,v[k]);
            }
            double b[4];
            if(!host_vis_bounds(v,4,b)) continue;
            for(int k=0;k<4;++k) b[k]*=c.h;
            const double* r=sel[room].r;
            double s[4]={std::max(b[0],r[0]),std::max(b[1],r[1]),std::min(b[2],r[2]),std::min(b[3],r[3])};
            if(s[2]<=s[0] || s[3]<=s[1]) continue;
            // D17Y: a full-height opening from the camera room into a taller
            // sky room shows that room above the opening's top edge too
            // (D17AR4: the host's own rule, not Classic's switch).
            if(room==source && upper_edge_open(a,table+(uint32_t)target*88,p)) {s[1]=r[1];++host_vis_open_tops;}
            auto& t=sel[target];
            if(t.on) {
                const double* q=t.r;
                if(s[0]>=q[0]-0.5 && s[1]>=q[1]-0.5 && s[2]<=q[2]+0.5 && s[3]<=q[3]+0.5) continue;
                host_vis_union(t.r,s);
            } else {std::copy(s,s+4,t.r);t.on=true;order.push_back((uint32_t)target);}
            if(++t.visits<=8) queue.push_back((uint32_t)target);
        }
    }
    // D17AX developer DNTTK_HOST_VIS_ALL: rooms of the level drawn through the whole
    // guard-banded view (pieces are still box-tested), to show what geometry
    // the level has beyond what the selection picks.
    // "1" every room, "sky" every sky room, or a comma list of rooms.
    static const std::vector<bool> all=[]{
        std::vector<bool> v;const char* t=std::getenv("DNTTK_HOST_VIS_ALL");
        if(!t || !*t || !std::strcmp(t,"0")) return v;
        v.assign(257,false);
        if(!std::strcmp(t,"1")) {v.assign(257,true);return v;}
        if(!std::strcmp(t,"sky")) {v[256]=true;return v;}
        for(const char* q=t;*q;) {char* e;const long n=std::strtol(q,&e,10);if(e==q) break;if(n>=0 && n<256) v[n]=true;q=*e?e+1:e;}
        return v;
    }();
    if(!all.empty()) {
        for(uint32_t room=0;room<rooms;++room) {
            if(!all[room] && !(all[256] && (psx_mod_read_word(table+room*88+12)&8))) continue;
            if(sel[room].on) {host_vis_union(sel[room].r,guard);continue;}
            std::copy(guard,guard+4,sel[room].r);sel[room].on=true;order.push_back(room);++host_vis_extra;
            host_vis_developer[room]=true;
        }
        return;
    }
    // Over the walls (the D17AQ open-sky test, guard-banded). It is part of
    // host selection, so the Classic switch DNTTK_OPEN_SKY does not reach it.
    const uint32_t home=table+source*88;
    const int over_walls=host_vis_over_walls();
    if(!(psx_mod_read_word(home+12)&8) || !over_walls) return;
    static std::vector<bool> region;
    if(over_walls==1) host_vis_sky_region(table,rooms,source,region);
    OpenSkyView v;
    for(int i=0;i<9;++i) v.m[i]=(int16_t)c.m[i];
    for(int i=0;i<3;++i) v.eye[i]=(int32_t)c.eye[i];
    v.h=(int32_t)c.h;
    for(int i=0;i<4;++i) v.rect[i]=(int32_t)std::lround(guard[i]);
    int32_t lo[3],hi[3],r[4];
    for(int i=0;i<3;++i) {lo[i]=(int32_t)psx_mod_read_word(home+4*i);hi[i]=(int32_t)psx_mod_read_word(home+0x20+4*i);}
    static std::vector<OpenSkyBox> walls,pieces;
    if(!open_sky_pieces(home,walls)) return;
    const int32_t far=(int32_t)psx_mod_read_word(view_limits+0x74);
    for(uint32_t room=0;room<rooms;++room) {
        const uint32_t a=table+room*88;
        if(room==source || !(psx_mod_read_word(a+12)&8)) continue;
        if(over_walls==1 && !region[room]) {++host_vis_region_skipped;continue;}
        if(!sel[room].on && !open_sky_in_view(a,v.m,v.eye,v.h,v.rect[0],v.rect[1],v.rect[2],v.rect[3],far)) continue;
        if(!open_sky_pieces(a,pieces) || pieces.empty() || !open_sky_seen(v,lo,hi,walls,pieces,r)) continue;
        const double s[4]={(double)r[0],(double)r[1],(double)r[2],(double)r[3]};
        if(sel[room].on) host_vis_union(sel[room].r,s);
        else {std::copy(s,s+4,sel[room].r);sel[room].on=true;order.push_back(room);++host_vis_extra;}
    }
}
static void host_vis_write_list(const HostVisEntry* e,uint32_t n) {
    for(uint32_t i=0;i<n;++i) {
        const uint32_t a=0x800d68b0+12*i;
        for(int k=0;k<4;++k) psx_mod_write_half(a+2*k,(uint16_t)e[i].r[k]);
        psx_mod_write_word(a+8,e[i].room);
    }
    psx_mod_write_word(0x800d68a8,n);
}
// Room walk entry (ra 0x8002e688), after the draw-distance limits. True when
// the host drew the world and the game's walk was emptied.
bool host_visibility_room_shown(uint32_t room) {
    return host_vis_shown_valid && room<host_vis_shown.size() && host_vis_shown[room];
}
static bool host_vis_walk(CPUState* cpu) {
    host_vis_shown_valid=false;
    host_vis_live=false;
    lights_upload(false,nullptr,nullptr); // D41A: no lit corners until this frame's camera is known
    // The session log records the switch (the environment and console are not).
    static int logged=-1;
    const int on=host_visibility_enabled();
    if(on!=logged) {std::fprintf(stderr,"[TTK hostvis] host world visibility %s\n",on?"on":"off");logged=on;}
    if(!on || !near_clip_host_world_live()) return false;
    if(!psx_mod_read_word(0x800c3c8c)) return false; // the walk draws nothing either
    if(!code_identity(host_vis_guards,host_vis_words)) {++host_vis_refusals;return false;}
    const uint32_t cam=cpu->gpr[4];
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(cam<0x80010000u || cam>0x801fff00u || (cam&3)) {++host_vis_refusals;return false;}
    const uint32_t source=psx_mod_read_word(cam+0x90);
    if(!rooms || rooms>256 || source>=rooms || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) {++host_vis_refusals;return false;}
    const uint32_t count=psx_mod_read_word(0x800d68a8);
    if(count>32) {++host_vis_refusals;return false;}
    const int16_t rx=(int16_t)psx_mod_read_half(0x800d2210),ry=(int16_t)psx_mod_read_half(0x800d2212);
    const int16_t rw=(int16_t)psx_mod_read_half(0x800d2214),rh=(int16_t)psx_mod_read_half(0x800d2216);
    if(rw<=0 || rh<=0) {++host_vis_refusals;return false;}
    const uint32_t* gte=cpu->gte_ctrl;
    HostVisCamera c;int16_t m[9];int32_t eye[3];
    for(int i=0;i<9;++i) {m[i]=(int16_t)psx_mod_read_half(cam+0x20+2*i);c.m[i]=m[i];}
    for(int i=0;i<3;++i) {eye[i]=(int32_t)psx_mod_read_word(cam+0x14+4*i);c.eye[i]=eye[i];}
    c.h=(uint16_t)gte[26];
    if(c.h<64) {++host_vis_refusals;return false;}
    // The view rectangle is centred on the GTE offset.
    const double ox=(int32_t)gte[24]/65536.0,oy=(int32_t)gte[25]/65536.0;
    const double view[4]={rx-ox,ry-oy,rx+rw-ox,ry+rh-oy};
    const double gx=(view[2]-view[0])*0.5*host_vis_guard(),gy=(view[3]-view[1])*0.5*host_vis_guard();
    const double guard[4]={view[0]-gx,view[1]-gy,view[2]+gx,view[3]+gy};
    host_vis_frame_cam=c;std::copy(guard,guard+4,host_vis_frame_guard);
    lights_upload(true,c.m,c.eye); // D41A
    host_vis_game_count=count;
    for(uint32_t i=0;i<count;++i) {
        const uint32_t e=0x800d68b0+12*i;host_vis_game[i].room=psx_mod_read_word(e+8);
        for(int k=0;k<4;++k) host_vis_game[i].r[k]=(int16_t)psx_mod_read_half(e+2*k);
    }
    static std::vector<HostVisRoom> sel;static std::vector<uint32_t> order;
    const auto t0=std::chrono::steady_clock::now();
    host_vis_select(c,table,rooms,source,guard,sel,order);
    const auto t1=std::chrono::steady_clock::now();
    // Every room the portal walk listed is drawn at least through its own
    // rectangle, so the host never shows less than the game would.
    for(uint32_t i=0;i<count;++i) {
        const uint32_t room=host_vis_game[i].room;
        if(room>=rooms) continue;
        const int16_t* g=host_vis_game[i].r;
        if(g[2]<=0 || g[3]<=0) continue;
        const double s[4]={g[0]-ox,g[1]-oy,g[0]+g[2]-ox,g[1]+g[3]-oy};
        host_vis_developer[room]=false; // a room the game lists is drawn as usual
        if(sel[room].on) {
            const double* q=sel[room].r;
            if(s[0]<q[0]-0.5 || s[1]<q[1]-0.5 || s[2]>q[2]+0.5 || s[3]>q[3]+0.5) ++host_vis_widened;
            host_vis_union(sel[room].r,s);
        } else {
            std::copy(s,s+4,sel[room].r);sel[room].on=true;order.push_back(room);
            ++host_vis_missed;host_vis_last_missed=room;
            host_vis_last_missed_from=source;
        }
    }
    // D17AR4 audit (developer DNTTK_HOST_VIS_AUDIT=1): would Classic's portal
    // rectangles (D17P margin, D17Y open upper edge) reach past the host's?
    static const bool audit=[]{const char* t=std::getenv("DNTTK_HOST_VIS_AUDIT");return t && !std::strcmp(t,"1");}();
    if(audit) {
        const int32_t mg=portal_margin();
        for(uint32_t i=0;i<count;++i) {
            const uint32_t room=host_vis_game[i].room;const int16_t* g=host_vis_game[i].r;
            if(room>=rooms || g[2]<=0 || g[3]<=0) continue;
            const bool top=open_upper_portal(room);
            const double c[4]={(double)std::max<int32_t>(rx,g[0]-mg)-ox,(double)(top?ry:std::max<int32_t>(ry,g[1]-mg))-oy,
                               (double)std::min<int32_t>(rx+rw,g[0]+g[2]+mg)-ox,(double)std::min<int32_t>(ry+rh,g[1]+g[3]+mg)-oy};
            const double* q=sel[room].r;
            const double out=std::max({q[0]-c[0],q[1]-c[1],c[2]-q[2],c[3]-q[3]});
            ++host_vis_audit[0];
            if(out>0.5) {++host_vis_audit[1];host_vis_audit_max=std::max(host_vis_audit_max,out);}
            if(top) {++host_vis_audit[2];if(q[1]-c[1]>0.5) {++host_vis_audit[3];host_vis_audit_room=room;}}
        }
    }
    // Context the walk sets before its loop (0x80062b84..0x80062bc0).
    psx_mod_write_word(view_limits+0x50,0x300);
    psx_mod_write_word(view_limits+0x10,0x1f800000u);psx_mod_write_word(view_limits+0x14,0x1f8002a0u);
    psx_mod_write_word(view_limits+0x60,0x800dd858u);
    uint32_t saved_ctrl[32],saved_gpr[32];
    std::memcpy(saved_ctrl,cpu->gte_ctrl,sizeof saved_ctrl);std::memcpy(saved_gpr,cpu->gpr,sizeof saved_gpr);
    for(int i=0;i<5;++i) cpu->gte_ctrl[i]=psx_mod_read_word(cam+0x20+4*i);
    static std::vector<uint32_t> left;left.clear();
    std::vector<bool> left_room(rooms,false);
    const uint32_t flat=psx_mod_read_word(0x800c0bb8);
    // Pieces in view, then drawn nearest first: the packet budget then runs
    // out, if ever, on the farthest pieces, not on whichever room came last.
    struct Pick {double distance;uint32_t piece,room;};
    static std::vector<Pick> picks;picks.clear();
    for(uint32_t room:order) {
        const uint32_t a=table+room*88;
        double origin[3];for(int i=0;i<3;++i) origin[i]=(int32_t)psx_mod_read_word(a+4*i);
        const unsigned n=psx_mod_read_byte(a+0x32);
        const uint32_t list=psx_mod_read_word(a+0x40);
        if(!n || list<0x80010000u || list>0x80200000u-n*4 || (list&3)) continue;
        for(unsigned i=0;i<n;++i) {
            const uint32_t piece=psx_mod_read_word(list+4*i);
            if(piece<0x80010000u || piece>0x801f0000u || (piece&3)) continue;
            double lo[3],hi[3],d2=0;
            for(int k=0;k<3;++k) {
                lo[k]=origin[k]+(int16_t)psx_mod_read_half(piece+4+2*k);hi[k]=origin[k]+(int16_t)psx_mod_read_half(piece+12+2*k);
                const double d=c.eye[k]<lo[k]?lo[k]-c.eye[k]:c.eye[k]>hi[k]?c.eye[k]-hi[k]:0;d2+=d*d;
            }
            ++host_vis_tested;
            if(lo[0]>hi[0] || lo[1]>hi[1] || lo[2]>hi[2] || !host_vis_box(c,lo,hi,sel[room].r)) continue;
            picks.push_back({d2,piece,room});
        }
    }
    // Developer rooms' pieces after all others, each group nearest first.
    std::stable_sort(picks.begin(),picks.end(),[](const Pick& x,const Pick& y){
        const bool a=host_vis_developer[x.room],b=host_vis_developer[y.room];
        return a!=b ? b : x.distance<y.distance;});
    static std::vector<bool> picked;picked.assign(rooms,false);for(const Pick& k:picks) picked[k.room]=true;
    unsigned pieces_drawn=0;uint32_t current=~0u,flags=0x300;
    // D17AR3: a redraw-only live frame is never shown (frame_replay.cpp); its
    // selection stands (rooms for the props and object passes), no piece is
    // emitted and none goes back to the game's walk. Drawing would also have
    // cleared the pieces' vertex light marks (a muzzle flash sets them); the
    // redraw workers copy this RAM, so without the clear the light stayed on.
    const bool redraw_only=live_frame_redraw_only();
    if(redraw_only) {
        for(const Pick& k:picks)
            if(!flat && !(psx_mod_read_word(table+k.room*88+12)&2)) near_clip_host_world_unlit(k.piece);
        pieces_drawn=(unsigned)picks.size();host_vis_redraw_only+=picks.size();picks.clear();
    }
    for(const Pick& k:picks) {
        if(k.room!=current) {
            // The room's state as the walk sets it (0x80062bd8..0x80062d00).
            current=k.room;
            const uint32_t a=table+k.room*88;
            int32_t room_min[3],tr[3];
            for(int i=0;i<3;++i) room_min[i]=(int32_t)psx_mod_read_word(a+4*i);
            host_vis_translation(m,room_min,eye,tr);
            for(int i=0;i<3;++i) cpu->gte_ctrl[5+i]=(uint32_t)tr[i];
            flags=0x300;
            if(flat) {flags|=2;psx_mod_write_word(view_limits+0x54,flat);}
            else if(psx_mod_read_word(a+12)&2) {flags|=2;psx_mod_write_word(view_limits+0x54,psx_mod_read_word(a+0x54));}
            const double* r=sel[k.room].r;
            const int16_t clip[4]={(int16_t)std::clamp<double>(std::floor(r[0]+ox),-32768,32767),(int16_t)std::clamp<double>(std::ceil(r[2]+ox),-32768,32767),
                                   (int16_t)std::clamp<double>(std::floor(r[1]+oy),-32768,32767),(int16_t)std::clamp<double>(std::ceil(r[3]+oy),-32768,32767)};
            for(int i=0;i<4;++i) psx_mod_write_half(view_limits+0x18+2*i,(uint16_t)clip[i]);
        }
        // Flag 0x80: the rectangle test (as the walk sets it for a piece partly
        // outside its room's rectangle), against the host's guard-banded room
        // rectangle. Faces whose exactly projected corners all lie past one
        // edge then cost no packets; faces near the eye keep host clipping.
        psx_mod_write_word(view_limits+0x50,flags|0x80);
        cpu->gpr[4]=k.piece;cpu->gpr[5]=0x800d27a0u;cpu->gpr[6]=view_limits;cpu->gpr[7]=0x800d26a0u;cpu->gpr[31]=0x80062db8u;
        ++host_vis_selected;
        if(near_clip_host_world_piece(cpu)) ++pieces_drawn;
        else if(host_vis_developer[k.room]) ++host_vis_developer_dropped;
        else {left.push_back(k.piece);left_room[k.room]=true;++host_vis_left;}
    }
    std::memcpy(cpu->gte_ctrl,saved_ctrl,sizeof saved_ctrl);std::memcpy(cpu->gpr,saved_gpr,sizeof saved_gpr);
    const auto t2=std::chrono::steady_clock::now();
    host_vis_select_us+=(uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count();
    host_vis_draw_us+=(uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();
    // The game's walk: only rooms with pieces left over, drawn whole-view.
    HostVisEntry walk[32];uint32_t walk_count=0;
    for(uint32_t room:order)
        if(left_room[room] && walk_count<32) walk[walk_count++]={room,{rx,ry,rw,rh}};
    // Props/actors pass: game-listed rooms first, then the host's.
    host_vis_props_count=0;
    std::vector<bool> listed(rooms,false);
    auto props_rect=[&](uint32_t room,int16_t out[4],const int16_t* game) {
        double b[4]={1e18,1e18,-1e18,-1e18};
        if(game) {b[0]=game[0];b[1]=game[1];b[2]=game[0]+game[2];b[3]=game[1]+game[3];}
        if(sel[room].on) {const double* r=sel[room].r;const double s[4]={r[0]+ox,r[1]+oy,r[2]+ox,r[3]+oy};host_vis_union(b,s);}
        const double x0=std::max<double>(rx,std::floor(b[0])),y0=std::max<double>(ry,std::floor(b[1]));
        const double x1=std::min<double>(rx+rw,std::ceil(b[2])),y1=std::min<double>(ry+rh,std::ceil(b[3]));
        if(x1<=x0 || y1<=y0) return false;
        out[0]=(int16_t)x0;out[1]=(int16_t)y0;out[2]=(int16_t)(x1-x0);out[3]=(int16_t)(y1-y0);
        return true;
    };
    for(uint32_t i=0;i<count && host_vis_props_count<31;++i) {
        const uint32_t room=host_vis_game[i].room;
        if(room>=rooms || listed[room]) continue;
        listed[room]=true;
        // A game-listed room keeps at least its own rectangle.
        HostVisEntry& e=host_vis_props[host_vis_props_count];e.room=room;
        if(!props_rect(room,e.r,host_vis_game[i].r)) std::copy(host_vis_game[i].r,host_vis_game[i].r+4,e.r);
        ++host_vis_props_count;
    }
    for(uint32_t room:order) {
        if(host_vis_props_count>=31) break;
        if(listed[room] || host_vis_developer[room]) continue;
        listed[room]=true;
        HostVisEntry& e=host_vis_props[host_vis_props_count];e.room=room;
        if(props_rect(room,e.r,nullptr)) ++host_vis_props_count;
    }
    host_vis_write_list(walk,walk_count);
    near_clip_host_walk(true,left);
    host_vis_live=true;
    host_vis_shown.assign(rooms,false);
    for(uint32_t room:order) if(room<rooms && !host_vis_developer[room]) host_vis_shown[room]=true;
    uint32_t listed_n=0;
    for(uint32_t room:order) if(listed_n<256) host_vis_last_order[listed_n++]=(uint16_t)(room|(picked[room]?0x100u:0u));
    host_vis_last_order_n=listed_n;
    host_vis_shown_valid=true;
    ++host_vis_frames;host_vis_rooms+=order.size();
    host_vis_last_rooms=(uint32_t)order.size();host_vis_last_pieces=pieces_drawn;host_vis_last_game=count;
    return true;
}
// Props/actors pass entry (ra 0x8002e6dc): the host room list.
static void host_vis_scene() {
    near_clip_host_walk(false,{});
    if(host_vis_live) host_vis_write_list(host_vis_props,host_vis_props_count);
}
// D17AZ: objects that reach out of their room. The object pass 0x800374cc
// visits only the rooms the portal walk listed: for each, the room's object
// list (+0x48, nodes +4 next, +8 object), its targetables (+0x4c) and the
// global list, drawing each through the renderer table 0x800c0bf8 by kind
// (+0x14) with the room's rectangle. An object belongs to one room (+0x2e),
// so one that reaches into rooms on screen vanished whole as soon as its own
// room's opening left the view: the slot-12 train (type 77, radius 9573,
// owned by the room behind the camera room), like a multi-sector sprite in
// EDuke32. Here, after the pass has its list back, each static object
// (kind 1, renderer 0x80031d10, the kind of nearly every object that reaches
// out of its room in all 21 levels) in a room the walk did not list is drawn
// through that renderer with the whole view rectangle when its bounding
// sphere (position +4, radius +0x5a) reaches out of its room's box into the
// box of a room the host shows, and into the guard-banded view. The renderer
// keeps its own sphere, box, far and near tests, clipping and depth; host
// depth hides what lies behind nearer walls. Only drawing changes: the lists,
// object membership and every handler stay the game's. Actors (kind 2) and
// the rest stay on the pass (D17AQ). Redraw workers run the same pass.
// Developer: DNTTK_OBJECT_REACH=0 turns it off.
static uint64_t host_vis_reach_drawn=0,host_vis_reach_tested=0,host_vis_reach_us=0;
static uint32_t host_vis_reach_last=0;
static bool host_vis_reach_enabled() {
    static const bool v=[]{const char* t=std::getenv("DNTTK_OBJECT_REACH");return !(t && !std::strcmp(t,"0"));}();
    return v;
}
static void host_vis_reach_draw(CPUState* source) {
    host_vis_reach_last=0;
    if(!host_vis_reach_enabled() || !host_vis_shown_valid) return;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || rooms!=host_vis_shown.size() || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return;
    if(psx_mod_read_word(0x800c0bf8+4)!=0x80031d10u) return;
    const uint32_t top=source->gpr[29];
    if(source->gpr[4]!=camera || guest_busy || top<0x801f4000 || top>0x801ffff0 ||
       g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    const auto t0=std::chrono::steady_clock::now();
    std::vector<bool> listed(rooms,false);
    for(uint32_t i=0;i<host_vis_game_count;++i) if(host_vis_game[i].room<rooms) listed[host_vis_game[i].room]=true;
    static std::vector<std::array<double,6>> boxes;boxes.resize(rooms);
    for(uint32_t room=0;room<rooms;++room) {
        const uint32_t a=table+room*88;
        for(int k=0;k<3;++k) {boxes[room][k]=(int32_t)psx_mod_read_word(a+4*k);boxes[room][3+k]=(int32_t)psx_mod_read_word(a+0x20+4*k);}
    }
    static std::vector<uint32_t> draw;draw.clear();
    for(uint32_t room=0;room<rooms && draw.size()<32;++room) {
        if(listed[room]) continue;
        const double* rlo=boxes[room].data();const double* rhi=rlo+3;
        uint32_t node=psx_mod_read_word(table+room*88+0x48);
        for(unsigned n=0;node && n<512 && draw.size()<32;++n,node=psx_mod_read_word(node+4)) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            const uint32_t o=psx_mod_read_word(node+8);
            if(o<0x80010000u || o>0x801fff00u || (o&3)) continue;
            if(psx_mod_read_byte(o+0x14)!=1 || (int8_t)psx_mod_read_byte(o+0x2e)!=(int)room) continue;
            const int rad=(int16_t)psx_mod_read_half(o+0x5a);
            if(rad<=0) continue;
            double lo[3],hi[3];bool out=false;
            for(int k=0;k<3;++k) {
                const double p=(int32_t)psx_mod_read_word(o+4+4*k);
                lo[k]=p-rad;hi[k]=p+rad;out|=lo[k]<rlo[k] || hi[k]>rhi[k];
            }
            if(!out) continue;
            ++host_vis_reach_tested;
            bool reach=false;
            for(uint32_t s=0;s<rooms && !reach;++s) {
                if(s==room || !host_vis_shown[s]) continue;
                const double* slo=boxes[s].data();const double* shi=slo+3;
                reach=lo[0]<shi[0] && hi[0]>slo[0] && lo[1]<shi[1] && hi[1]>slo[1] && lo[2]<shi[2] && hi[2]>slo[2];
            }
            if(reach && host_vis_box(host_vis_frame_cam,lo,hi,host_vis_frame_guard)) draw.push_back(o);
        }
    }
    host_vis_reach_us+=(uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-t0).count();
    if(draw.empty()) return;
    // The renderer runs on a copy of the CPU with the stack and scratchpad
    // restored afterwards, as D35F1's pole draw does.
    uint32_t saved[1024],scratch[256];
    const uint32_t base=(top-4096)&~3u;
    for(unsigned i=0;i<1024;++i) saved[i]=psx_mod_read_word(base+4*i);
    for(unsigned i=0;i<256;++i) scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    for(uint32_t o:draw) {
        CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=0x800000fc;
        cpu.gpr[4]=camera;cpu.gpr[5]=o;cpu.gpr[6]=0x800d2210;
        guest_busy=true;psx_dispatch_call(&cpu,0x80031d10,0x800000fc);guest_busy=false;
    }
    for(unsigned i=0;i<1024;++i) if(psx_mod_read_word(base+4*i)!=saved[i]) psx_mod_write_word(base+4*i,saved[i]);
    for(unsigned i=0;i<256;++i) if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i]) psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    host_vis_reach_drawn+=draw.size();host_vis_reach_last=(uint32_t)draw.size();
}
// Object pass entry (ra 0x8002e6e4): the portal walk's own list, then the
// objects that reach out of unlisted rooms (D17AZ).
static bool host_vis_object_pass(CPUState* cpu,uint32_t ra) {
    if(!host_vis_live || ra!=0x8002e6e4u) return false;
    host_vis_write_list(host_vis_game,host_vis_game_count);
    host_vis_reach_draw(cpu);
    host_vis_live=false;
    return true;
}
static const char* host_visibility_debug_json() {
    static char buffer[3072];
    uint64_t n[5];near_clip_host_counters(n);
    char list[1600];int used=0;list[0]=0;
    for(uint32_t i=0;i<host_vis_last_order_n && i<256 && used<(int)sizeof list-12;++i)
        used+=std::snprintf(list+used,sizeof list-used,"%s%u",i?",":"",(unsigned)host_vis_last_order[i]);
    std::snprintf(buffer,sizeof buffer,"{\"enabled\":%s,\"frames\":%llu,\"refusals\":%llu,\"rooms\":%llu,\"over_walls\":%llu,\"tested\":%llu,"
        "\"selected\":%llu,\"drawn\":%llu,\"left\":%llu,\"left_far\":%llu,\"left_near\":%llu,\"walk_nulled\":%llu,\"game_missed\":%llu,\"game_widened\":%llu,\"last_missed\":[%u,%u],\"select_us\":%llu,\"draw_us\":%llu,\"redraw_only_pieces\":%llu,\"last\":{\"rooms\":%u,\"pieces\":%u,\"game_rooms\":%u,\"props_rooms\":%u},"
        "\"open_tops\":%llu,\"audit\":[%llu,%llu,%llu,%llu,%.1f,%u],\"developer_dropped\":%llu,\"reach\":[%llu,%llu,%u,%llu],\"region_skipped\":%llu,\"list\":[%s]}",
        host_visibility_enabled()?"true":"false",(unsigned long long)host_vis_frames,(unsigned long long)host_vis_refusals,(unsigned long long)host_vis_rooms,
        (unsigned long long)host_vis_extra,(unsigned long long)host_vis_tested,(unsigned long long)host_vis_selected,(unsigned long long)n[0],
        (unsigned long long)n[1],(unsigned long long)n[3],(unsigned long long)n[4],(unsigned long long)n[2],(unsigned long long)host_vis_missed,(unsigned long long)host_vis_widened,host_vis_last_missed_from,host_vis_last_missed,(unsigned long long)host_vis_select_us,(unsigned long long)host_vis_draw_us,(unsigned long long)host_vis_redraw_only,host_vis_last_rooms,host_vis_last_pieces,host_vis_last_game,host_vis_props_count,(unsigned long long)host_vis_open_tops,
        (unsigned long long)host_vis_audit[0],(unsigned long long)host_vis_audit[1],(unsigned long long)host_vis_audit[2],(unsigned long long)host_vis_audit[3],host_vis_audit_max,host_vis_audit_room,(unsigned long long)host_vis_developer_dropped,(unsigned long long)host_vis_reach_drawn,(unsigned long long)host_vis_reach_tested,host_vis_reach_last,(unsigned long long)host_vis_reach_us,(unsigned long long)host_vis_region_skipped,list);
    return buffer;
}
bool hostvis_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"hostvis") && std::strncmp(line,"hostvis ",8)) return false;
    close=false;
    if(!std::strncmp(line,"hostvis walls ",14)) {
        const char* v=line+14;
        if(!std::strcmp(v,"off")) host_vis_walls_override=0;
        else if(!std::strcmp(v,"region")) host_vis_walls_override=1;
        else if(!std::strcmp(v,"all")) host_vis_walls_override=2;
        else if(!std::strcmp(v,"default")) host_vis_walls_override=-1;
        else {say("Usage: hostvis walls off|region|all|default (developer, session only)");return true;}
        const int m=host_vis_over_walls();
        say(m==0?"Over-the-walls rooms off.":m==2?"Over-the-walls rooms: every sky room.":"Over-the-walls rooms: own open-air region.");
        return true;
    }
    if(line[7]) {
        const char* v=line+8;
        if(!std::strcmp(v,"on")) host_vis_override=1;
        else if(!std::strcmp(v,"off")) host_vis_override=0;
        else if(!std::strcmp(v,"default")) host_vis_override=-1;
        else {say("Usage: hostvis [on|off|default|walls off|region|all|default] (developer, session only)");return true;}
    }
    say(host_visibility_enabled()?"Host world visibility on.":"Host world visibility off (developer).");
    return true;
}
