// D17BL the room the eye is in (Modernized; Vanilla never reaches it).
//
// The camera render (0x8002e48c) passes the camera's room word (camera +0x90)
// to the portal walk (0x8006276c: a0 room, a1 camera, a2 view rectangle,
// a3 list) and every visible room is found from there. The game keeps that
// word from its own camera update, which can trail the eye we draw from: in
// the first-person ladder climb through a ceiling hatch (UI slot 12, level 8)
// the eye was already above the lower room's box, inside the shaft room only,
// for up to 20 game frames while the word still named the lower room. The walk
// from a room the eye is not in finds no opening facing it, so nothing of the
// world was drawn and the whole view was the sky fill with the ladder over it.
// Classic and the host world pass, 60 and 180, all showed it; third person and
// Vanilla did not (their eye stays inside the room).
//
// Rule (every level, no per-level data): when the eye has gone through one of
// the camera room's portals (eye_room_through_portal), the walk starts in the
// room beyond it; when the eye lies outside the room's box (room +0 min, +0x20
// max) and a room joined to it through the level's portals (one or two steps)
// contains the eye, the walk starts there: the nearest step first, then the
// smallest box. Only the walk's start room
// changes, in the call's register and in the host pass; the camera's room word
// and the game's own state are untouched, so play, savestates and the camera
// update see the original value. An eye inside its room, or in no joined
// room at all (a wall-clipped camera), keeps the game's room.
// Developer: DNTTK_EYE_ROOM=0 keeps the game's room; DNTTK_EYE_ROOM_TRACE=1
// prints each walk's start room (in, out) and eye.
static uint64_t eye_room_checks=0,eye_room_moves=0,eye_room_unfound=0;
static uint32_t eye_room_last_from=0xffffffffu,eye_room_last_to=0xffffffffu;
static bool eye_room_inside(uint32_t a,const int32_t eye[3]) {
    for(int k=0;k<3;++k) {
        const int32_t lo=(int32_t)psx_mod_read_word(a+4*k),hi=(int32_t)psx_mod_read_word(a+0x20+4*k);
        if(eye[k]<lo || eye[k]>hi) return false;
    }
    return true;
}
// The eye has gone through one of the room's portals: it is on the far side
// of the opening's plane (the side away from the room's box centre, towards
// the target's) and within the opening's extent widened by 128 units across
// it (along its normal by the distance past it), and the
// target's box holds it. Room boxes overlap (a lower room's box can reach
// above its ceiling hatch), so the box alone cannot tell.
static int32_t eye_room_through_portal(uint32_t table,uint32_t rooms,uint32_t room,const int32_t eye[3]) {
    const uint32_t a=table+room*88;
    const unsigned count=psx_mod_read_byte(a+0x30);
    const uint32_t portals=psx_mod_read_word(a+0x38);
    if(!count || count>64 || portals<0x80010000u || portals>0x80200000u-count*36 || (portals&3)) return -1;
    double origin[3],centre[3];
    for(int k=0;k<3;++k) {origin[k]=(int32_t)psx_mod_read_word(a+4*k);centre[k]=0.5*(origin[k]+(int32_t)psx_mod_read_word(a+0x20+4*k));}
    for(unsigned i=0;i<count;++i) {
        const uint32_t p=portals+i*36;
        const int32_t target=(int16_t)psx_mod_read_half(p+14);
        if(target<0 || (uint32_t)target>=rooms || (uint32_t)target==room) continue;
        const uint32_t b=table+(uint32_t)target*88;
        if(!eye_room_inside(b,eye)) continue;
        double c[4][3],lo[3],hi[3];
        for(unsigned j=0;j<4;++j) for(int k=0;k<3;++k) c[j][k]=origin[k]+(int16_t)psx_mod_read_half(p+8*j+2*k);
        for(int k=0;k<3;++k) {lo[k]=std::min(std::min(c[0][k],c[1][k]),std::min(c[2][k],c[3][k]));hi[k]=std::max(std::max(c[0][k],c[1][k]),std::max(c[2][k],c[3][k]));}
        // Corners are a 2x2 grid: 0-1 and 0-2 span the opening.
        const double u[3]={c[1][0]-c[0][0],c[1][1]-c[0][1],c[1][2]-c[0][2]},v[3]={c[2][0]-c[0][0],c[2][1]-c[0][1],c[2][2]-c[0][2]};
        const double n[3]={u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]};
        const double len=std::sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
        if(len<1) continue;
        double tc[3];
        for(int k=0;k<3;++k) tc[k]=0.5*((int32_t)psx_mod_read_word(b+4*k)+(int32_t)psx_mod_read_word(b+0x20+4*k));
        double dr=0,dt=0,de=0;
        for(int k=0;k<3;++k) {dr+=n[k]*(centre[k]-c[0][k]);dt+=n[k]*(tc[k]-c[0][k]);de+=n[k]*(eye[k]-c[0][k]);}
        dr/=len;dt/=len;de/=len;
        if(!((dr>0 && dt<0) || (dr<0 && dt>0))) continue; // sides unclear
        if(!(dr>0 ? de<-1 : de>1)) continue;               // still on the room's side
        bool within=true;
        // Along the opening's normal the eye is |de| past it by design.
        for(int k=0;k<3 && within;++k) {
            const double m=128+std::fabs(de*n[k]/len);
            within=eye[k]>=lo[k]-m && eye[k]<=hi[k]+m;
        }
        if(within) return target;
    }
    return -1;
}
// Rooms behind an opening of `room` that the eye is right at (within `near`
// of its plane, either side, inside its extent widened by 128), or over it
// within 768 of the room behind it: the portal
// walk cannot see through an opening nearer than the near plane, so the host
// pass draws those rooms through the whole view (D17BL revision 2: the eye
// level with a ceiling hatch showed the sky fill and the ladder below).
static unsigned eye_room_near_openings(uint32_t table,uint32_t rooms,uint32_t room,const int32_t eye[3],double near,uint32_t* out,unsigned cap) {
    const uint32_t a=table+room*88;
    const unsigned count=psx_mod_read_byte(a+0x30);
    const uint32_t portals=psx_mod_read_word(a+0x38);
    if(!count || count>64 || portals<0x80010000u || portals>0x80200000u-count*36 || (portals&3)) return 0;
    double origin[3];for(int k=0;k<3;++k) origin[k]=(int32_t)psx_mod_read_word(a+4*k);
    unsigned n=0;
    for(unsigned i=0;i<count && n<cap;++i) {
        const uint32_t p=portals+i*36;
        const int32_t target=(int16_t)psx_mod_read_half(p+14);
        if(target<0 || (uint32_t)target>=rooms || (uint32_t)target==room) continue;
        double c[4][3],lo[3],hi[3];
        for(unsigned j=0;j<4;++j) for(int k=0;k<3;++k) c[j][k]=origin[k]+(int16_t)psx_mod_read_half(p+8*j+2*k);
        for(int k=0;k<3;++k) {lo[k]=std::min(std::min(c[0][k],c[1][k]),std::min(c[2][k],c[3][k]));hi[k]=std::max(std::max(c[0][k],c[1][k]),std::max(c[2][k],c[3][k]));}
        const double u[3]={c[1][0]-c[0][0],c[1][1]-c[0][1],c[1][2]-c[0][2]},v[3]={c[2][0]-c[0][0],c[2][1]-c[0][1],c[2][2]-c[0][2]};
        const double nn[3]={u[1]*v[2]-u[2]*v[1],u[2]*v[0]-u[0]*v[2],u[0]*v[1]-u[1]*v[0]};
        const double len=std::sqrt(nn[0]*nn[0]+nn[1]*nn[1]+nn[2]*nn[2]);
        if(len<1) continue;
        double de=0;for(int k=0;k<3;++k) de+=nn[k]*(eye[k]-c[0][k]);de/=len;
        // Within `near` of the plane, or inside the target's box within 256
        // of it: room boxes overlap around an opening (the room under a
        // ceiling hatch reaches above the opening's plane, up to its ceiling),
        // so an eye there sees that room's walls whichever start room it has.
        const bool in_target=std::fabs(de)<=256 && eye_room_inside(table+(uint32_t)target*88,eye);
        // Or the target's box within 768 of the eye while the eye is over the
        // opening (revision 4: from the shaft above, the lower room's top band
        // around the hatch, above the opening's plane, is in plain view).
        double box2=0;
        for(int k=0;k<3;++k) {
            const double lo2=(int32_t)psx_mod_read_word(table+(uint32_t)target*88+4*k),hi2=(int32_t)psx_mod_read_word(table+(uint32_t)target*88+0x20+4*k);
            const double d=eye[k]<lo2?lo2-eye[k]:eye[k]>hi2?eye[k]-hi2:0;box2+=d*d;
        }
        const bool over_target=box2<=768.0*768.0;
        if(std::fabs(de)>near && !in_target && !over_target) continue;
        bool within=in_target;
        if(!within) {
            within=true;
            for(int k=0;k<3 && within;++k) {const double m=128+std::fabs(de*nn[k]/len);within=eye[k]>=lo[k]-m && eye[k]<=hi[k]+m;}
        }
        if(within) out[n++]=(uint32_t)target;
    }
    return n;
}
static uint32_t eye_room_for(uint32_t cam,uint32_t room) {
    static const bool enabled=[]{const char* t=std::getenv("DNTTK_EYE_ROOM");return !t || std::strcmp(t,"0");}();
    if(!enabled) return room;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || room>=rooms || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return room;
    if(cam<0x80010000u || cam>0x801fff00u || (cam&3)) return room;
    int32_t eye[3];
    for(int k=0;k<3;++k) eye[k]=(int32_t)psx_mod_read_word(cam+0x14+4*k);
    ++eye_room_checks;
    const uint32_t original=room;
    // Through portals first (at most three openings in one frame).
    for(int step=0;step<3;++step) {
        const int32_t next=eye_room_through_portal(table,rooms,room,eye);
        if(next<0) break;
        room=(uint32_t)next;
    }
    if(eye_room_inside(table+room*88,eye)) {
        if(room!=original) {++eye_room_moves;eye_room_last_from=original;eye_room_last_to=room;}
        return room;
    }
    // Outside the box: a room joined in one or two steps whose box holds the
    // eye, the nearest step first, then the smallest box.
    uint32_t frontier[64];unsigned nf=1;frontier[0]=room;
    uint32_t seen[128];unsigned ns=1;seen[0]=room;
    for(int step=0;step<2;++step) {
        uint32_t next[64];unsigned nn=0;
        uint32_t best=0xffffffffu;double best_volume=0;
        for(unsigned f=0;f<nf;++f) {
            const uint32_t a=table+frontier[f]*88;
            const unsigned count=psx_mod_read_byte(a+0x30);
            const uint32_t portals=psx_mod_read_word(a+0x38);
            if(!count || count>64 || portals<0x80010000u || portals>0x80200000u-count*36 || (portals&3)) continue;
            for(unsigned i=0;i<count;++i) {
                const int32_t target=(int16_t)psx_mod_read_half(portals+i*36+14);
                if(target<0 || (uint32_t)target>=rooms) continue;
                bool known=false;
                for(unsigned s=0;s<ns && !known;++s) known=seen[s]==(uint32_t)target;
                if(known || ns>=128) continue;
                seen[ns++]=(uint32_t)target;
                if(nn<64) next[nn++]=(uint32_t)target;
                const uint32_t b=table+(uint32_t)target*88;
                if(!eye_room_inside(b,eye)) continue;
                double volume=1;
                for(int k=0;k<3;++k) volume*=(double)((int32_t)psx_mod_read_word(b+0x20+4*k)-(int32_t)psx_mod_read_word(b+4*k))+1;
                if(best==0xffffffffu || volume<best_volume) {best=(uint32_t)target;best_volume=volume;}
            }
        }
        if(best!=0xffffffffu) {
            ++eye_room_moves;eye_room_last_from=original;eye_room_last_to=best;
            return best;
        }
        std::copy(next,next+nn,frontier);nf=nn;
    }
    ++eye_room_unfound;
    return original;
}
// Portal walk entry from the camera render (ra 0x8002e650).
static void eye_room_portal_walk(CPUState* cpu) {
    const uint32_t cam=cpu->gpr[5];
    if(cam<0x80010000u || cam>0x801fff00u || (cam&3)) return;
    if(cpu->gpr[4]!=psx_mod_read_word(cam+0x90)) return; // not the camera's own room word
    const uint32_t before=cpu->gpr[4];
    cpu->gpr[4]=eye_room_for(cam,before);
    static const bool trace=[]{const char* t=std::getenv("DNTTK_EYE_ROOM_TRACE");return t && !std::strcmp(t,"1");}();
    if(trace) std::fprintf(stderr,"[TTK eye room] %u -> %u eye %d %d %d\n",before,cpu->gpr[4],
        (int32_t)psx_mod_read_word(cam+0x14),(int32_t)psx_mod_read_word(cam+0x18),(int32_t)psx_mod_read_word(cam+0x1c));
}
static const char* eye_room_debug_json() {
    static char buffer[192];
    std::snprintf(buffer,sizeof buffer,"{\"checks\":%llu,\"moves\":%llu,\"unfound\":%llu,\"last_from\":%d,\"last_to\":%d}",
        (unsigned long long)eye_room_checks,(unsigned long long)eye_room_moves,(unsigned long long)eye_room_unfound,
        (int)eye_room_last_from,(int)eye_room_last_to);
    return buffer;
}
