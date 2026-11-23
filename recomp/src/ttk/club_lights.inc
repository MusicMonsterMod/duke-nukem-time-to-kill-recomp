// D57J: nightclub rooms (documentation/541-d57j-club-lighting.md). Included
// by level_lights.inc after the lamp file code, before the console.
//
// The LA strip club (levels 0, 5 and 9, found by its fixtures' models in
// prop_lights.cpp: the ceiling neon bar with its spot cans, the pole stages'
// red canopies, the back bar) read in RELIT as a room with every light on:
// its lamp file lights it with broad scan lamps (the painted glow of the
// whole room, radius up to 6000) and sign lamps of 3500-4200 at every table
// and neon tube, which overlap into one even wash (lab: all lamps off took
// the view from mean 37 to 15; no single lamp more than 3 of it). A rule for
// every room that holds one of those fixtures, so every copy of the club in
// every level (and a custom level that uses them) takes it, with no
// per-level data:
// - Relit dims the painted light further there (club_ambient, against the
//   indoor 60): the room is dark, the fixtures carry it.
// - The scan lamps (the even glow) are taken out of those rooms.
// - Every other lamp there (the signs, neon, tables, the bar) keeps its
//   colour, more saturated, and reaches club_reach as far: a pool round its
//   fixture with dark between.
// - New lamps (not written by "lamp save"): a warm amber-pink key under each
//   pole canopy (the brightest light in the room, on the dancer) and a gel
//   pool down the aim of each spot can on the neon bars (magenta outside,
//   a cool blue inside: the two-colour scheme against the warm key).
// prop_lights.cpp draws the haze round them (Enhanced and Relit only).
// Lighting ORIGINAL draws no lamps (the club as the game drew it); Vanilla
// and Classic never run this. "lamp club off" (session) reloads the lamps
// without the rule: the look before D57J, for comparisons.
// Developer: DNTTK_CLUB=0 (start with it off), DNTTK_CLUB_AMBIENT=<percent>,
// DNTTK_CLUB_REACH=<percent>, DNTTK_CLUB_SATURATE=<exponent>,
// DNTTK_CLUB_KEY="r g b strength radius", DNTTK_CLUB_KEY_AT="x y z" (canopy
// model space), DNTTK_CLUB_SPOT="strength radius distance" (along the aim).
static float club_ambient=0.30f,club_reach=0.55f,club_saturate=1.6f;
static float club_key_rgb[3]={1.0f,0.68f,0.55f},club_key_k=4.0f,club_key_radius=2000;   // Bastard Amber-like (Rosco 02), the brightest in the room
static float club_key_at[3]={-38,600,27};   // 600 under the rim, 760 over the stage
static float club_spot_k=1.3f,club_spot_radius=1500,club_spot_dist=1300;
// D57L (documentation/544-d57l-apartment-mood-lighting.md): a room holding the
// hanging lamp (prop type 229, by model) is an apartment room (levels 0 and
// 9, room 8). In RELIT its painted light is dimmed further (apt_ambient: the
// lamps carry it, a hazy room rather than an even wash), and a faint sodium
// glow stands just inside each of its portals that open onto an outdoor
// room (its street window). The lamps' looks are in prop_lights.cpp.
// Developer: DNTTK_APARTMENT_AMBIENT=<percent>,
// DNTTK_APARTMENT_WINDOW="r g b strength radius inside above".
static float apt_ambient=0.45f;
static float apt_window_rgb[3]={1.0f,0.50f,0.18f},apt_window_k=0.35f,apt_window_radius=1400,apt_window_in=250,apt_window_up=300;
static struct ClubState {
    unsigned level=~0u;bool done=false;uint64_t sig=0;
    std::vector<uint8_t> room;                      // per room: 1 = a nightclub room
    unsigned fixtures=0,shaped=0,added=0;
    uint32_t loaded_tick=0,next_try=0;              // a level's props can link a little after its rooms: none found, look again
    std::vector<LevelLamp> dropped;                 // the file's scan lamps taken out (lamp save writes them back)
    std::vector<uint8_t> apt;                       // D57L: per room: 1 = an apartment room
    unsigned apts=0,windows=0;
} club_st;
static void club_tune_once() {
    static bool done=false;
    if(done) return;
    done=true;
    if(const char* e=std::getenv("DNTTK_CLUB");e && e[0]=='0') club_lights_enabled.store(false);
    if(const char* e=std::getenv("DNTTK_CLUB_AMBIENT")) club_ambient=std::clamp((float)std::atof(e),10.0f,100.0f)/100.0f;
    if(const char* e=std::getenv("DNTTK_CLUB_REACH")) club_reach=std::clamp((float)std::atof(e),20.0f,100.0f)/100.0f;
    if(const char* e=std::getenv("DNTTK_CLUB_SATURATE")) club_saturate=std::clamp((float)std::atof(e),1.0f,4.0f);
    if(const char* e=std::getenv("DNTTK_CLUB_KEY")) std::sscanf(e,"%f %f %f %f %f",&club_key_rgb[0],&club_key_rgb[1],&club_key_rgb[2],&club_key_k,&club_key_radius);
    if(const char* e=std::getenv("DNTTK_CLUB_KEY_AT")) std::sscanf(e,"%f %f %f",&club_key_at[0],&club_key_at[1],&club_key_at[2]);
    if(const char* e=std::getenv("DNTTK_CLUB_SPOT")) std::sscanf(e,"%f %f %f",&club_spot_k,&club_spot_radius,&club_spot_dist);
    if(const char* e=std::getenv("DNTTK_APARTMENT");e && e[0]=='0') apartment_lights_enabled.store(false);
    if(const char* e=std::getenv("DNTTK_APARTMENT_AMBIENT")) apt_ambient=std::clamp((float)std::atof(e),10.0f,100.0f)/100.0f;
    if(const char* e=std::getenv("DNTTK_APARTMENT_WINDOW")) std::sscanf(e,"%f %f %f %f %f %f %f",&apt_window_rgb[0],&apt_window_rgb[1],&apt_window_rgb[2],&apt_window_k,&apt_window_radius,&apt_window_in,&apt_window_up);
}
static void club_reset() {club_st.loaded_tick=light_ticks;club_st.next_try=0;club_st.done=false;club_st.room.clear();club_st.dropped.clear();club_st.fixtures=club_st.shaped=club_st.added=0;
                         club_st.apt.clear();club_st.apts=club_st.windows=0;}
static bool apt_room(int room) {return room>=0 && (size_t)room<club_st.apt.size() && club_st.apt[(size_t)room];}
static float apt_ambient_now() {club_tune_once();return apt_ambient;}
static float club_ambient_now() {club_tune_once();return club_ambient;}
static bool club_room(int room) {return room>=0 && (size_t)room<club_st.room.size() && club_st.room[(size_t)room];}
// The colour pushed away from white, its brightest channel kept.
static void club_saturate_rgb(float rgb[3]) {
    const float m=std::max({rgb[0],rgb[1],rgb[2]});
    if(m<=0) return;
    for(int c=0;c<3;++c) rgb[c]=m*std::pow(rgb[c]/m,club_saturate);
}
// Model space to world, as the game turns props (yaw +0x1c, 4096 a turn).
static void club_place(double out[3],const double p[3],double yaw,const float m[3]) {
    const double ca=std::cos(yaw),sa=std::sin(yaw);
    out[0]=p[0]+m[0]*ca+m[2]*sa;out[1]=p[1]+m[1];out[2]=p[2]-m[0]*sa+m[2]*ca;
}
static void club_add(const double p[3],const float rgb[3],float k,float radius,const char* note) {
    LevelLamp l{};
    for(int i=0;i<3;++i) {l.p[i]=std::round(p[i]);l.rgb[i]=rgb[i];}
    l.radius=radius;l.k=k;l.style=lamp_steady;l.note=note;l.room=-2;l.club=2;l.wall_checked=true;
    level_lamps.push_back(l);++club_st.added;
}
// Once the level's rooms are built (game thread, level_lamps_update).
static void club_apply() {
    club_tune_once();
    club_st.done=true;club_st.level=level_lamps_level;club_st.sig=shadow_mesh_sig;
    const bool club_on=club_lights_enabled.load(std::memory_order_relaxed),apt_on=apartment_lights_enabled.load(std::memory_order_relaxed);
    if((!club_on && !apt_on) || !prop_light_kind) return;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return;
    struct Fixture {int kind;double p[3],yaw;uint32_t room;};
    std::vector<Fixture> fx,pendants;std::vector<uint32_t> seen;
    club_st.room.assign(rooms,0);club_st.apt.assign(rooms,0);
    for(uint32_t r=0;r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
        uint32_t node=psx_mod_read_word(table+88*r+head_at);
        for(unsigned guard=0;node && guard<1024;++guard) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            if(o<0x80010000u || o>0x801fff00u || (o&3) || psx_mod_read_byte(o+0x14)!=1) continue;
            if(std::find(seen.begin(),seen.end(),o)!=seen.end()) continue;
            seen.push_back(o);
            const int kind=prop_light_kind(psx_mod_read_word(o+0x44));
            const bool club=club_on && prop_light_club_kind(kind),apt=apt_on && prop_light_hanging(kind);   // D57M: the bare bulb too
            if(!club && !apt) continue;
            Fixture f{kind,{0,0,0},(int16_t)psx_mod_read_half(o+0x1c)*(2*3.14159265358979/4096),r};
            for(int k=0;k<3;++k) f.p[k]=(int32_t)psx_mod_read_word(o+4+4*k);
            if(apt) {pendants.push_back(f);club_st.apt[r]=1;continue;}
            fx.push_back(f);club_st.room[r]=1;
        }
    }
    club_st.fixtures=(unsigned)fx.size();
    // D57L: each apartment room's window glow: a portal into an outdoor room
    // (room flags & 8). Portals: +0x30 count, +0x38 pointer, 36 bytes each,
    // four corners of int16 x y z (8 bytes apart, from the room's origin),
    // the room it leads to at +14.
    for(const Fixture& f:pendants) {
        if(std::count_if(pendants.begin(),pendants.begin()+(&f-pendants.data()),[&](const Fixture& g){return g.room==f.room;})) continue;   // once a room
        ++club_st.apts;
        const uint32_t rec=table+88*f.room;
        const unsigned cnt=psx_mod_read_byte(rec+0x30);const uint32_t pp=psx_mod_read_word(rec+0x38);
        if(!cnt || cnt>32 || pp<0x80010000u || pp>0x801fff00u-36*cnt) continue;
        double org[3];for(int k=0;k<3;++k) org[k]=(int32_t)psx_mod_read_word(rec+4*k);
        for(unsigned i=0;i<cnt;++i) {
            const uint32_t q=pp+36*i;
            const int to=(int16_t)psx_mod_read_half(q+14);
            if(to<0 || (uint32_t)to>=rooms || !(psx_mod_read_word(table+88*(uint32_t)to+12)&8)) continue;
            double v[4][3],c[3]={0,0,0};
            for(int j=0;j<4;++j) for(int k=0;k<3;++k) {v[j][k]=org[k]+(int16_t)psx_mod_read_half(q+8*j+2*k);c[k]+=v[j][k]/4;}
            const double a[3]={v[1][0]-v[0][0],v[1][1]-v[0][1],v[1][2]-v[0][2]},b[3]={v[2][0]-v[0][0],v[2][1]-v[0][1],v[2][2]-v[0][2]};
            double nrm[3]={a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};
            const double len=std::sqrt(nrm[0]*nrm[0]+nrm[1]*nrm[1]+nrm[2]*nrm[2]);
            if(len<1) continue;
            // Into the room: the side its hanging lamp is on.
            double side=0;for(int k=0;k<3;++k) {nrm[k]/=len;side+=nrm[k]*(f.p[k]-c[k]);}
            if(side<0) for(double& x:nrm) x=-x;
            double p[3];for(int k=0;k<3;++k) p[k]=c[k]+nrm[k]*apt_window_in;
            p[1]-=apt_window_up;   // y grows down: nearer the top of the window
            club_add(p,apt_window_rgb,apt_window_k,apt_window_radius,"apartment window (street light)");
            --club_st.added;++club_st.windows;   // counted as a window, not a club lamp
        }
    }
    if(club_st.apts) std::fprintf(stderr,"[TTK apartment] level %u: %u apartment rooms, %u window glows\n",level_lamps_level,club_st.apts,club_st.windows);
    if(fx.empty()) {if(club_st.windows) {level_lamp_selected=-1;signal_cache_clear();}return;}
    // The file's lamps in club rooms: the scan lamps out, the rest to pools.
    for(size_t i=0;i<level_lamps.size();) {
        LevelLamp& l=level_lamps[i];
        if(l.room==-2) l.room=level_lamp_room(l);
        if(!club_room(l.room) || l.club) {++i;continue;}
        if(!l.note.compare(0,9,"scan room")) {club_st.dropped.push_back(l);level_lamps.erase(level_lamps.begin()+(long)i);continue;}
        for(int c=0;c<3;++c) l.file_rgb[c]=l.rgb[c];
        l.file_radius=l.radius;l.club=1;
        l.radius=std::max(600.0,std::round(l.radius*club_reach));
        club_saturate_rgb(l.rgb);for(float& c:l.rgb) c=std::round(c*100)/100;
        ++club_st.shaped;++i;
    }
    for(const Fixture& f:fx) {
        double p[3];
        if(f.kind==prop_light_canopy) {
            club_place(p,f.p,f.yaw,club_key_at);club_add(p,club_key_rgb,club_key_k,club_key_radius,"club key (pole canopy)");
        } else if(f.kind==prop_light_neon_bar) {
            for(int c=0;c<4;++c) {
                const float m[3]={club_can_x[c]+club_can_aim[0]*club_spot_dist,club_can_y+club_can_aim[1]*club_spot_dist,club_can_z+club_can_aim[2]*club_spot_dist};
                club_place(p,f.p,f.yaw,m);club_add(p,club_gel[c],club_spot_k,club_spot_radius,"club spot (neon bar can)");
            }
        }
    }
    level_lamp_selected=-1;signal_cache_clear();
    unsigned n=0;for(uint8_t v:club_st.room) n+=v;
    std::fprintf(stderr,"[TTK club] level %u: %u fixtures in %u rooms; %zu scan lamps out, %u lamps to pools, %u lamps added\n",
                 level_lamps_level,club_st.fixtures,n,club_st.dropped.size(),club_st.shaped,club_st.added);
}
// level_lamps_update, before the wall checks: once a level (or reload), and
// again when the rooms turn out to have been the previous level's.
static void club_update() {
    if(!shadow_mesh_tris || shadow_mesh_copy.empty()) return;
    if(club_st.done && club_st.level==level_lamps_level && club_st.sig!=shadow_mesh_sig && club_st.fixtures+club_st.room.size()>0) {
        level_lamps_load(level_lamps_level);   // resets club
    }
    if(!club_st.done) {club_apply();level_lamp_snapshot();}
    // None yet: every 30 ticks for 30 s after the load (one walk of the
    // room lists, about 400 objects).
    else if(!club_st.fixtures && !club_st.apts && light_ticks-club_st.loaded_tick<900 && light_ticks>=club_st.next_try) {
        club_st.next_try=light_ticks+30;club_apply();
        if(club_st.fixtures || club_st.windows) level_lamp_snapshot();
    }
}
// lamp save: the file's own values (the rule is applied at every load).
static void club_file_values(LevelLamp& l) {
    if(l.club!=1) return;
    for(int c=0;c<3;++c) l.rgb[c]=l.file_rgb[c];
    l.radius=l.file_radius;
}
static void club_write_dropped(std::ostream& f) {
    char row[200];
    for(const LevelLamp& l:club_st.dropped) {
        std::snprintf(row,sizeof row,"%.0f %.0f %.0f  %.2f %.2f %.2f  %.0f  %.2f  %s",l.p[0],l.p[1],l.p[2],l.rgb[0],l.rgb[1],l.rgb[2],l.radius,l.k,lamp_style_text(l).c_str());
        f<<row;if(!l.note.empty()) f<<"  # "<<l.note;f<<'\n';
    }
}
// "lamp apartment [off|on]" (D57L).
static void apt_console(const char* rest) {
    while(*rest==' ') ++rest;
    if(!std::strcmp(rest,"off") || !std::strcmp(rest,"on")) {
        apartment_lights_enabled.store(rest[1]=='n');
        level_lamps_load(level_lamps_level);level_lamp_snapshot();
        level_lamp_notice("Apartment lighting %s (session): lamps reloaded",rest[1]=='n'?"on":"off");
        return;
    }
    const char* why="";const float t=relit_room>=0?relit_room_target(relit_room,&why):0;
    level_lamp_notice("Apartment %s: %u rooms, %u window glows. Camera room %d %s (ambient %.0f%%)",apartment_lights_enabled.load()?"on":"off",
                      club_st.apts,club_st.windows,relit_room,apt_room(relit_room)?"is an apartment room":"is not",t*100);
}
static void club_console(const char* rest) {
    while(*rest==' ') ++rest;
    if(!std::strcmp(rest,"off") || !std::strcmp(rest,"on")) {
        club_lights_enabled.store(rest[1]=='n');
        level_lamps_load(level_lamps_level);level_lamp_snapshot();
        level_lamp_notice("Club lighting %s (session): lamps reloaded",rest[1]=='n'?"on":"off");
        return;
    }
    unsigned n=0;for(uint8_t v:club_st.room) n+=v;
    const char* why="";const float t=relit_room>=0?relit_room_target(relit_room,&why):0;
    level_lamp_notice("Club %s: %u fixtures in %u rooms, %zu scan lamps out, %u to pools, %u added. Camera room %d %s (ambient %.0f%%)",
                      club_lights_enabled.load()?"on":"off",club_st.fixtures,n,club_st.dropped.size(),club_st.shaped,club_st.added,relit_room,club_room(relit_room)?"is a club room":"is not",t*100);
}
