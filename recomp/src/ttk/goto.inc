// D61 goto (backtick console, Modernized): put Duke anywhere in any level,
// standing on the floor, ready to play. Contract: documentation/559-d61-level-atlas.md.
//
//   goto [LEVEL] room R [DEGREES] [quiet]    a standing point in room R
//   goto [LEVEL] object N [DIST] [DEGREES] [quiet]
//                                            beside level object N (table
//                                            800D2550, as warp object), facing it
//   goto X Y Z [DEGREES] [quiet]             the floor nearest that point
//   goto LEVEL at X Y Z [DEGREES] [quiet]    the same in another level
//   goto NAME [quiet]                        a landmark (assets/atlas/landmarks.txt)
//   goto LEVEL                               the start of a level (hidden ones too)
//   ... face DEG, quiet, safe                in any order after a target
//   goto here | goto landmarks | goto face [+|-]DEGREES | goto rooms | goto fix | goto help
//
// LEVEL is a level number from "levels": another level is entered first with
// level select's own travel (the restart path, so a normal pistol start), and
// Duke is placed once that level has been in play for a second. Within the
// current level Duke keeps everything he has.
//
// Placement (game thread, the player update): the target's room becomes Duke's
// cell (+0x2e) so the game's own segment query (0x8006D980, weapon_aim.cpp's
// trace) is seeded there, then vertical rays find every surface in the column
// from the room's top down. Duke stands on the lowest one with STAND headroom
// above it (what play uses, and never inside a block, the D53D LEVEL09 case),
// his position 498 above it as when standing (+0x1c8, the ground under him,
// is that surface). A room target tries a grid of columns from the room's
// centre outward. Facing: the given compass angle (0 = +Z, 90 = +X), else
// towards the object, else (a room) the longest clear view at eye height,
// else (a point) as Duke faced; the view yaw follows.
// quiet: the game's own invisibility timer (+0x89c, enemy AI 0x800A5CBC)
// for goto_quiet_ticks (2400: 8 s at the drain of 300 a second, note 132), so enemies ignore Duke while a shot is framed (Duke
// is drawn translucent and the gadget timer shows while it runs). safe:
// the invincibility timer (+0x8A0) for 20 s, as its pickup, so a lava floor or an enemy
// already at Duke does no harm (the gadget timer shows).
// Results are logged as "[TTK goto] arrived ..." or "[TTK goto] refused ..."
// (tools/local/atlas.py waits on them).
struct GotoRequest {
    enum Kind {Room,Object,Point} kind=Room;
    bool on=false,face=false,quiet=false,safe=false;
    long level=-1;int room=-1,n=0,distance=600,degrees=0;int32_t x=0,y=0,z=0;
    char label[40]={};
};
static std::mutex goto_mutex;static GotoRequest goto_request;
static int goto_settle=0;static unsigned goto_wait=0;
static std::atomic<int> goto_turn{INT32_MIN};
static std::atomic<bool> goto_fix_request{false};
static bool swim_teleport_exit(CPUState* cpu);   // swim.inc (D61: out of the water)
static constexpr int goto_stand=0x500,goto_offset=498,goto_quiet_ticks=2400,goto_safe_ticks=6000,goto_wait_frames=60*60;
static constexpr uint32_t goto_rooms=0x800da454,goto_room_count=0x800da47c,goto_level_index=0x800be570;
static std::atomic<unsigned> goto_arrivals{0},goto_refusals{0};
// The indices the level select skips (note 101): developer "level force" only.
static bool goto_hidden_level(long n) {return n==4 || (n>=13 && n<=20) || n>=30;}
// Where goto may stand Duke: authenticated Modernized gameplay, or (developer,
// a hidden index whose overlay is not authenticated) plain one-player
// gameplay with Duke's camera linked, not paused.
static bool goto_ready() {
    if(player_identity_ready())return true;
    return goto_hidden_level(ttk::level_current()) && gameplay_context() &&
        psx_mod_read_word(camera+0xa4)==player && psx_mod_read_word(player+0x7d4)==camera;
}

static bool goto_room_box(int room,int32_t lo[3],int32_t hi[3]) {
    const uint32_t table=psx_mod_read_word(goto_rooms),rooms=psx_mod_read_word(goto_room_count);
    if(room<0 || !rooms || rooms>256 || unsigned(room)>=rooms || table<0x80010000u || table>0x80200000u-rooms*88)return false;
    const uint32_t a=table+88u*unsigned(room);
    for(int k=0;k<3;++k){lo[k]=(int32_t)psx_mod_read_word(a+4*k);hi[k]=(int32_t)psx_mod_read_word(a+0x20+4*k);}
    return hi[0]>lo[0] && hi[1]>lo[1] && hi[2]>lo[2];
}
// Solid placed objects (a platform block, a crate, a dumpster: not world to
// the segment query, the D53D LEVEL09 case) stand on the floor too: when one
// whose box covers Duke's spot (with his radius) overlaps the space he would
// stand in, he stands on its top instead, again for one stacked on it. The
// rule for solid is spawn_objects_clear's (type flags set, no flag 8).
// False when that leaves less than goto_stand below the ceiling.
static bool goto_props(double x,double z,double& floor,int& headroom) {
    const double ceiling=floor-headroom,radius=160;
    const uint32_t rooms=psx_mod_read_word(goto_rooms),count=psx_mod_read_word(goto_room_count);
    if(!count || count>256 || rooms<0x80010000u || rooms>0x80200000u-count*88)return true;
    for(int pass=0;pass<4;++pass) {
        double top=floor;
        for(unsigned room=0;room<count;++room) {
            uint32_t node=psx_mod_read_word(rooms+88*room+0x48);
            for(unsigned n=0;node && n<1024;++n) {
                if(node<0x80010000u || node>0x801ffff0u || (node&3))break;
                const uint32_t other=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
                const uint32_t flags=object_type_flags(other);
                if(other==player || !flags || (flags&8))continue;
                for(unsigned i=0,boxes=object_box_count(other);i<boxes;++i) {
                    PushBounds q;if(!push_bounds(other,q,i))continue;
                    if(x+radius<q.x0 || x-radius>q.x1 || z+radius<q.z0 || z-radius>q.z1)continue;
                    if(q.y0>=floor-1 || q.y1<=floor-goto_stand)continue;   // below his feet, or above his head
                    top=std::min(top,double(q.y0));
                }
            }
        }
        if(top>=floor)break;
        floor=top;
    }
    headroom=int(floor-ceiling);
    return headroom>=goto_stand;
}
// Every surface in the column (x, z) of room, top down, seeded in that room.
// Returns the lowest with goto_stand clear above it nearest `near` (or the
// lowest of all when near is NaN); false when none.
static bool goto_column(CPUState* cpu,int room,double x,double z,double near,double& floor,int& headroom) {
    int32_t lo[3],hi[3];
    if(!goto_room_box(room,lo,hi))return false;
    const uint8_t cell=psx_mod_read_byte(player+0x2e);
    psx_mod_write_byte(player+0x2e,uint8_t(room));
    double from=lo[1]+16,prev=lo[1];bool found=false;double best_score=1e30;
    for(int k=0;k<128 && from<hi[1];++k) {
        const double a[3]={x,from,z},b[3]={x,double(hi[1])+256,z};double hit[3];int kind=0;
        if(!atlas_segment_query(cpu,a,b,hit,kind) || kind!=1 || hit[1]<from-1)break;
        const double y=hit[1];
        // A hit at the ray's start: it began inside solid (a room box can
        // reach above the real ceiling; the query reports the boundary at
        // once). Step down through it; open space begins below.
        if(y<from+2){from+=128;prev=from;continue;}
        if(y-prev>=goto_stand) {
            const double score=std::isnan(near)?-y:std::fabs(y-near);
            if(score<best_score){best_score=score;floor=y;headroom=int(y-prev);found=true;}
        }
        prev=y;from=y+48;
    }
    psx_mod_write_byte(player+0x2e,cell);
    return found && goto_props(x,z,floor,headroom);
}
// The room whose actor list (+0x4C) holds Duke, or -1: what the game draws
// and hears him by. His cell (+0x2E) alone is not enough.
static int goto_listed_room() {
    const uint32_t rooms=psx_mod_read_word(goto_rooms),count=psx_mod_read_word(goto_room_count);
    if(!count || count>256 || rooms<0x80010000u || rooms>0x80200000u-count*88)return -1;
    for(unsigned r=0;r<count;++r) {
        uint32_t node=psx_mod_read_word(rooms+88*r+0x4c);
        for(unsigned n=0;node && n<2048;++n) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3))break;
            if(psx_mod_read_word(node+8)==player)return int(r);
            node=psx_mod_read_word(node+4);
        }
    }
    return -1;
}
// hull_call with a second argument.
static bool goto_call2(CPUState* source,uint32_t address,uint32_t a0,uint32_t a1) {
    const uint32_t top=source->gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail)return false;
    uint32_t saved[1024],scratch[256];
    const uint32_t base=(top-4096)&~3u;
    for(unsigned i=0;i<1024;++i)saved[i]=psx_mod_read_word(base+4*i);
    for(unsigned i=0;i<256;++i)scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=0x800000fc;cpu.gpr[4]=a0;cpu.gpr[5]=a1;
    guest_busy=true;psx_mod_host_call_begin();psx_dispatch_call(&cpu,address,0x800000fc);psx_mod_host_call_end();guest_busy=false;
    for(unsigned i=0;i<1024;++i)if(psx_mod_read_word(base+4*i)!=saved[i])psx_mod_write_word(base+4*i,saved[i]);
    for(unsigned i=0;i<256;++i)if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i])psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    return cpu.pc==0 && !g_psx_call_bail;
}
// D61 fix (user report 2026-11-25, UI slot 5, "goto room 17"): writing Duke's
// position and cell left him in his old room's actor list, so the game drew
// and heard him by that room: turning away from it hid his body and weapon,
// and his shots sounded far off. The game's own actor relink 0x80095D9C
// (actor, position*) finds the room at the position (0x8005D790, the cell as
// hint), takes the actor out of its cell's list (+0x4C, 0x8001C9AC) and into
// the new room's (0x8001C6DC), cell included. It moves an actor only when
// that room differs from the cell, so the cell is first set back to the room
// whose list holds Duke. (0x80095CB0 is the placed-object version, list
// +0x48: not Duke's.)
static bool goto_relink(CPUState* cpu,const char* why) {
    const int before=goto_listed_room(),want=psx_mod_read_byte(player+0x2e);
    if(before>=0)psx_mod_write_byte(player+0x2e,uint8_t(before));
    const bool ok=goto_call2(cpu,0x80095d9c,player,player+4);
    const int after=goto_listed_room(),cell=psx_mod_read_byte(player+0x2e);
    if(!ok || after<0)psx_mod_write_byte(player+0x2e,uint8_t(want));
    std::fprintf(stderr,"[TTK goto] relink (%s): listed in room %d -> %d, cell %d%s\n",why,before,after,cell,ok && after==cell?"":" (MISMATCH)");
    return ok && after==cell;
}
// D61 (user report 2026-11-25, TIME TO KILL: the club music went on playing
// after goto room 17). Walking, Duke's update (0x800412A4) relinks him and,
// when his room changes, calls the room-transition handler 0x8006C138(old,
// new): the new room's ambience (flags 0x20/0x10, its ambient zone +0x34),
// then the level's own cues keyed on the rooms left and entered (music,
// sounds). A teleport crossed no boundary, so nothing left the old room. The
// game's own teleport, the respawn at a re-entry point (0x80042DE8), ends
// with 0x8006C138(0, new room) and the music reset 0x8003C240: goto and warp
// end the same way (not its invincibility or ammo).
static void goto_arrival_events(CPUState* cpu,const char* why) {
    if(std::getenv("DNTTK_GOTO_EVENTS") && !std::strcmp(std::getenv("DNTTK_GOTO_EVENTS"),"0"))return;   // developer A/B
    const int room=psx_mod_read_byte(player+0x2e);
    const bool a=goto_call2(cpu,0x8006c138,0,uint32_t(room));
    const bool b=goto_call2(cpu,0x8003c240,0,0);
    std::fprintf(stderr,"[TTK goto] arrival events (%s): room %d%s\n",why,room,a && b?"":" (a call failed)");
}
static void goto_place(CPUState* cpu,const char* what,int room,double x,double floor,double z,double facing,int headroom,bool quiet,bool safe) {
    psx_mod_write_word(player+4,uint32_t(int32_t(std::lround(x))));
    psx_mod_write_word(player+8,uint32_t(int32_t(std::lround(floor-goto_offset))));
    psx_mod_write_word(player+12,uint32_t(int32_t(std::lround(z))));
    psx_mod_write_word(player+0x1c8,uint32_t(int32_t(std::lround(floor))));
    psx_mod_write_byte(player+0x2e,uint8_t(room));
    yaw=facing;
    psx_mod_write_half(player+0x1c,uint16_t(int(std::lround(facing/(2*3.14159265358979)*4096))&4095));
    goto_relink(cpu,"goto");
    room=psx_mod_read_byte(player+0x2e);
    swim_teleport_exit(cpu);   // out of the swim state (it drops him onto this floor)
    goto_arrival_events(cpu,"goto");
    psx_mod_write_word(player+4,uint32_t(int32_t(std::lround(x))));
    psx_mod_write_word(player+8,uint32_t(int32_t(std::lround(floor-goto_offset))));
    psx_mod_write_word(player+12,uint32_t(int32_t(std::lround(z))));
    if(quiet && (int16_t)psx_mod_read_half(player+0x89c)<goto_quiet_ticks)psx_mod_write_half(player+0x89c,goto_quiet_ticks);
    // safe: the game's own invincibility timer (+0x8A0, Duke's damage handler
    // 0x800A4510, note 132) for 6000 units, 20 s, as the invincibility pickup
    // (lava kills in about a second: time to walk out), a fall, an enemy at Duke.
    if(safe && (int16_t)psx_mod_read_half(player+0x8a0)<goto_safe_ticks)psx_mod_write_half(player+0x8a0,goto_safe_ticks);
    const int degrees=(int(std::lround(facing*180/3.14159265358979))%360+360)%360;
    ++goto_arrivals;
    std::fprintf(stderr,"[TTK goto] arrived level %ld room %d at %.0f %.0f %.0f facing %d floor %.0f headroom %d%s%s (%s)\n",
                 ttk::level_current(),room,x,floor-goto_offset,z,degrees,floor,headroom,quiet?" quiet":"",safe?" safe":"",what);
    char text[48];std::snprintf(text,sizeof text,"ROOM %d",room);input_notice(text);
}
static void goto_refuse(const char* why) {
    ++goto_refusals;
    std::fprintf(stderr,"[TTK goto] refused %s\n",why);
    char text[80];std::snprintf(text,sizeof text,"Goto: %s",why);input_notice(text);
}
static double goto_compass(int degrees) {return degrees*3.14159265358979/180.0;}
// The longest clear view at eye height from a standing point (16 directions):
// a room arrival faces into the room, not into the nearest wall.
static double goto_open_view(CPUState* cpu,int room,double x,double floor,double z,double fallback) {
    const uint8_t cell=psx_mod_read_byte(player+0x2e);
    psx_mod_write_byte(player+0x2e,uint8_t(room));
    double best=-1,facing=fallback;
    for(int k=0;k<16;++k) {
        const double a=k*3.14159265358979/8,eye=floor-900;
        const double from[3]={x,eye,z},to[3]={x+std::sin(a)*12000,eye,z+std::cos(a)*12000};double hit[3];int kind=0;
        if(!atlas_segment_query(cpu,from,to,hit,kind))continue;
        const double d=kind?std::hypot(hit[0]-x,hit[2]-z):12000;
        if(d>best+1){best=d;facing=a;}
    }
    psx_mod_write_byte(player+0x2e,cell);
    return facing;
}
static void goto_resolve(CPUState* cpu,const GotoRequest& g) {
    if(g.kind==GotoRequest::Object) {
        const uint32_t o=(g.n>=0 && g.n<64)?psx_mod_read_word(0x800d2550+4*g.n):0;
        if(o<0x80010000u || o>=0x801fff00u){goto_refuse("no such object");return;}
        const double ox=(int32_t)psx_mod_read_word(o+4),oy=(int32_t)psx_mod_read_word(o+8),oz=(int32_t)psx_mod_read_word(o+12);
        // The given side first, then every 30 degrees around the object, at the
        // given distance, then half and double it.
        for(const int dist:{g.distance,g.distance/2,g.distance*2}) for(int turn=0;turn<360;turn+=30) {
            const double a=goto_compass((g.face?g.degrees:0)+turn);
            const double x=ox+std::sin(a)*dist,z=oz+std::cos(a)*dist;
            const int room=warp_cell(int32_t(x),int32_t(oy),int32_t(z));
            double floor;int head;
            if(room>=0 && goto_column(cpu,room,x,z,oy,floor,head)) {
                goto_place(cpu,g.label,room,x,floor,z,std::atan2(ox-x,oz-z),head,g.quiet,g.safe);return;
            }
        }
        // Else the standing point of the object's own room nearest it, facing it.
        const int home=warp_cell(int32_t(ox),int32_t(oy),int32_t(oz));
        int32_t lo[3],hi[3];
        if(home>=0 && goto_room_box(home,lo,hi)) {
            std::vector<std::pair<double,std::pair<double,double>>> cols;
            for(double x=lo[0]+512;x<hi[0];x+=1024) for(double z=lo[2]+512;z<hi[2];z+=1024)
                cols.push_back({(x-ox)*(x-ox)+(z-oz)*(z-oz),{x,z}});
            std::sort(cols.begin(),cols.end());
            for(auto& c:cols) {
                double floor;int head;const double x=c.second.first,z=c.second.second;
                if(goto_column(cpu,home,x,z,oy,floor,head)) {
                    goto_place(cpu,g.label,home,x,floor,z,std::atan2(ox-x,oz-z),head,g.quiet,g.safe);return;
                }
            }
        }
        goto_refuse("no floor beside that object");return;
    }
    if(g.kind==GotoRequest::Point) {
        // That column, then rings around it (a pickup or a ledge edge can
        // leave the exact column without a standing surface).
        bool inside=false;
        for(const int r:{0,256,512,768,1024,1536}) for(int k=0;k<(r?8:1);++k) {
            const double a=k*3.14159265358979/4,x=g.x+std::sin(a)*r,z=g.z+std::cos(a)*r;
            const int room=warp_cell(int32_t(x),g.y,int32_t(z));
            if(room<0)continue;
            inside=true;
            double floor;int head;
            if(!goto_column(cpu,room,x,z,g.y,floor,head) || std::fabs(floor-g.y)>4096)continue;
            const double facing=g.face?goto_compass(g.degrees):yaw;
            goto_place(cpu,g.label,room,x,floor,z,facing,head,g.quiet,g.safe);return;
        }
        goto_refuse(inside?"no floor near that point":"that point is outside every room");return;
    }
    int32_t lo[3],hi[3];
    if(!goto_room_box(g.room,lo,hi)){goto_refuse("no such room in this level (goto rooms)");return;}
    const double cx=(lo[0]+hi[0])/2.0,cz=(lo[2]+hi[2])/2.0;
    // Columns on the world grid (1024), nearest the centre first.
    std::vector<std::pair<double,std::pair<double,double>>> cols;
    for(double x=lo[0]+512;x<hi[0];x+=1024) for(double z=lo[2]+512;z<hi[2];z+=1024)
        cols.push_back({(x-cx)*(x-cx)+(z-cz)*(z-cz),{x,z}});
    std::sort(cols.begin(),cols.end());
    if(cols.size()>48)cols.resize(48);
    for(auto& c:cols) {
        double floor;int head;const double x=c.second.first,z=c.second.second;
        if(goto_column(cpu,g.room,x,z,std::nan(""),floor,head)) {
            const double facing=g.face?goto_compass(g.degrees):goto_open_view(cpu,g.room,x,floor,z,yaw);
            goto_place(cpu,g.label,g.room,x,floor,z,facing,head,g.quiet,g.safe);return;
        }
    }
    goto_refuse("no standing point found in that room");
}
static void goto_update(CPUState* cpu) {
    if(goto_fix_request.exchange(false) && goto_ready()) {
        const int before=goto_listed_room();
        const bool ok=goto_relink(cpu,"fix");
        char text[64];std::snprintf(text,sizeof text,ok?"Goto fix: Duke in room %d (was %d)":"Goto fix failed",psx_mod_read_byte(player+0x2e),before);
        input_notice(text);
    }
    const int turn=goto_turn.exchange(INT32_MIN);
    if(turn!=INT32_MIN && goto_ready()) {
        // "goto face": absolute compass degrees, or a relative turn (encoded +100000).
        const double a=turn>=100000-720 && turn<=100000+720?yaw+goto_compass(turn-100000):goto_compass(turn);
        yaw=std::remainder(a,2*3.14159265358979);
        psx_mod_write_half(player+0x1c,uint16_t(int(std::lround(yaw/(2*3.14159265358979)*4096))&4095));
        std::fprintf(stderr,"[TTK goto] facing %d\n",(int(std::lround(yaw*180/3.14159265358979))%360+360)%360);
    }
    GotoRequest g;
    {
        std::lock_guard<std::mutex> lock(goto_mutex);
        if(!goto_request.on){goto_settle=0;return;}
        if(goto_request.level>=0 && ttk::level_current()!=goto_request.level) {
            goto_settle=0;
            if(++goto_wait>goto_wait_frames){goto_request.on=false;goto_wait=0;goto_refuse("the level did not load");}
            return;
        }
        // A level just entered: let it run a second first (its objects and
        // enemies start, Duke lands at the level start).
        if(goto_request.level>=0 && ++goto_settle<60)return;
        g=goto_request;goto_request.on=false;goto_settle=0;goto_wait=0;
    }
    if(psx_mod_read_half(0x800be55c)){goto_refuse("Duke is dying");return;}
    if(!goto_ready() || psx_mod_read_word(0x800c27bc)!=1){goto_refuse("unavailable here (one-player gameplay only)");return;}
    goto_resolve(cpu,g);
    // A level entered for this goto starts as the game starts it: a pistol
    // start (Desert Eagle ammo +0x2d6, 200 from a new game, a savestate load
    // or another level, D61 tests). Say so loudly if one ever arrives without
    // it (the D53D lab arrivals, from a copy of a player save, did; not
    // reproduced here).
    if(g.level>=0 && !(int16_t)psx_mod_read_half(player+0x2d6)) {
        std::fprintf(stderr,"[TTK goto] warning: arrived in level %ld without pistol ammo\n",g.level);
        input_notice("Goto: arrived without the pistol");
    }
}

// Landmarks: "name level x y z [degrees]" or "name level room R [degrees]",
// '#' comments (recomp/assets/atlas/landmarks.txt; DNTTK_ATLAS_DIR overrides).
struct GotoLandmark {std::string name;long level=-1;bool room=false;int32_t x=0,y=0,z=0;int r=0,degrees=0;bool face=false;std::string note;};
static std::string goto_atlas_dir() {
    if(const char* e=std::getenv("DNTTK_ATLAS_DIR"); e && *e) return e;
    std::error_code ec;
    if(*TTK_ATLAS_DIR && std::filesystem::is_directory(TTK_ATLAS_DIR,ec)) return TTK_ATLAS_DIR;
    std::string dir="atlas";
    if(auto base=SDL_GetBasePath()) {
        dir=std::string(base)+"atlas";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    }
    return dir;
}
static std::vector<GotoLandmark> goto_landmarks() {
    std::vector<GotoLandmark> out;
    std::FILE* f=std::fopen((goto_atlas_dir()+"/landmarks.txt").c_str(),"r");
    if(!f)return out;
    char line[256];
    while(std::fgets(line,sizeof line,f)) {
        std::string note;
        if(char* h=std::strchr(line,'#')){note=h+1;*h=0;while(!note.empty() && (note.back()=='\n' || note.back()=='\r'))note.pop_back();
            while(!note.empty() && note.front()==' ')note.erase(0,1);}
        char name[40];long level;char a[16];
        GotoLandmark m;int used=0;
        if(std::sscanf(line,"%39s %ld %15s%n",name,&level,a,&used)<3)continue;
        m.name=name;m.level=level;m.note=note;
        if(!std::strcmp(a,"room")) {
            m.room=true;int deg;const int got=std::sscanf(line+used,"%d %d",&m.r,&deg);
            if(got<1)continue;
            if(got==2){m.face=true;m.degrees=deg;}
        } else {
            m.x=std::atoi(a);int deg;const int got=std::sscanf(line+used,"%d %d %d",&m.y,&m.z,&deg);
            if(got<2)continue;
            if(got==3){m.face=true;m.degrees=deg;}
        }
        out.push_back(m);
    }
    std::fclose(f);
    return out;
}

// A level's name for replies: the game's own, or for the hidden indices the
// community's (documentation/559): True Grit and Modern 4 are named XXX.
static std::string goto_level_name(long n) {
    switch(n) {
    case 4: return "TRUE GRIT (hidden)";
    case 13: return "MODERN 4 (hidden)";
    case 30: return "BOSS 4 (hidden)";
    default: break;
    }
    char title[48]={};ttk::level_title(unsigned(n),title,sizeof title);
    std::string t=title;
    if(goto_hidden_level(n))t+=" (hidden)";
    return t;
}
static void goto_help(void (*say)(const char*)) {
    // D26L4: tabbed rows; the console lines them up (elastic tab stops).
    say("GOTO puts Duke anywhere, standing and ready. Examples:");
    say("  goto room 12\troom 12 of this level (goto rooms: how many)");
    say("  goto 3 room 12\troom 12 of level 3 (type levels for numbers)");
    say("  goto 13\tthe start of level 13, hidden levels too");
    say("  goto gag-lava-south\ta saved spot (goto landmarks: the list)");
    say("  goto object 17\tbeside level object 17");
    say("  goto 45000 -300 -20600\ta map point x y z (goto 3 at x y z: in level 3)");
    say("Add to any of them:");
    say("  face 90\tface that compass direction (0 to 359)");
    say("  quiet\tinvisible for 8 seconds");
    say("  safe\tinvincible for 20 seconds");
    say("Also: goto here (where Duke is), goto face 90 (turn now), goto fix");
    say("Hidden levels (cut levels, test maps, 2-player arenas): type levels for the list");
}
static bool goto_console_command(const char* line,void (*say)(const char*),bool& close) {
    if(std::strcmp(line,"goto") && std::strncmp(line,"goto ",5))return false;
    if(!input_modernized()) {say("Goto: Modernized profile only (Vanilla is unchanged)");return true;}
    const char* rest=line[4]?line+5:"";
    while(*rest==' ')++rest;
    char reply[192];
    if(!*rest || !std::strcmp(rest,"help")) {goto_help(say);return true;}
    if(!std::strcmp(rest,"landmarks")) {
        const auto marks=goto_landmarks();
        if(marks.empty()){say("No landmarks (assets/atlas/landmarks.txt)");return true;}
        say("Landmarks (goto NAME):");
        // D26L4: grouped by level, one aligned block (level, name, note).
        long last=-1;
        for(const auto& m:marks) {
            char label[64]="";
            if(m.level!=last) {
                char title[48];
                if(ttk::level_title(unsigned(m.level),title,sizeof title))std::snprintf(label,sizeof label,"%ld %s",m.level,title);
                else std::snprintf(label,sizeof label,"LEVEL %ld",m.level);
                last=m.level;
            }
            std::snprintf(reply,sizeof reply,"%s\t%s\t%s",label,m.name.c_str(),m.note.c_str());say(reply);
        }
        return true;
    }
    const long current=ttk::level_current();
    if(current<0) {say("Goto: start or load a game first");return true;}
    if(psx_mod_read_half(0x800be55c)) {say("Goto: Duke is dying - wait for the level to restart");return true;}   // identity is checked on arrival (the console pauses gameplay)
    if(!std::strcmp(rest,"here")) {
        const int32_t x=(int32_t)psx_mod_read_word(player+4),y=(int32_t)psx_mod_read_word(player+8),z=(int32_t)psx_mod_read_word(player+12);
        const int deg=(int(std::lround(yaw*180/3.14159265358979))%360+360)%360;
        const int listed=goto_listed_room(),cell=psx_mod_read_byte(player+0x2e);
        std::snprintf(reply,sizeof reply,"level %ld room %d at %d %d %d facing %d (landmark line: NAME %ld %d %d %d %d)%s",
                      current,cell,x,y,z,deg,current,x,y,z,deg,listed==cell?"":" - drawn by another room: type goto fix");
        say(reply);std::fprintf(stderr,"[TTK goto] here %s\n",reply);return true;
    }
    if(!std::strcmp(rest,"fix")) {   // relink Duke into the room he stands in (a save from an old warp)
        goto_fix_request.store(true);say("Relinking Duke to the room he stands in");close=true;return true;
    }
    if(!std::strcmp(rest,"rooms")) {
        const uint32_t rooms=psx_mod_read_word(goto_room_count);
        std::snprintf(reply,sizeof reply,"Level %ld has rooms 0-%u (goto room R); Duke is in room %u",current,rooms?rooms-1:0,psx_mod_read_byte(player+0x2e));
        say(reply);return true;
    }
    if(!std::strncmp(rest,"face ",5)) {   // "goto face N": turn now (with a target after it, a goto)
        const char* v=rest+5;char* end=nullptr;const long d=std::strtol(v,&end,10);
        if(end!=v && !*end) {
            if(d<-720 || d>720){say("Try: goto face 90 (a compass direction), or goto face +45 to turn");return true;}
            goto_turn.store(*v=='+' || *v=='-'?int(100000+d):int(d));
            say("Turning");return true;
        }
    }
    // Words: optional level number, then room/object/point/landmark; face N,
    // quiet and safe anywhere.
    char buf[160];std::snprintf(buf,sizeof buf,"%s",rest);
    std::vector<std::string> w;
    for(char* t=std::strtok(buf," ");t;t=std::strtok(nullptr," "))w.push_back(t);
    GotoRequest g;g.on=true;
    auto number=[](const std::string& s,long& v){char* e=nullptr;v=std::strtol(s.c_str(),&e,10);return !s.empty() && !*e;};
    long v;
    // The extras may stand anywhere in the line: face N, quiet, safe.
    for(size_t k=0;k<w.size();) {
        if(w[k]=="quiet" || w[k]=="safe"){(w[k]=="quiet"?g.quiet:g.safe)=true;w.erase(w.begin()+long(k));continue;}
        if(w[k]=="face" && k+1<w.size() && number(w[k+1],v)){g.face=true;g.degrees=int(v);w.erase(w.begin()+long(k),w.begin()+long(k)+2);continue;}
        if(w[k]=="face"){say("Try: goto room 12 face 90 (a compass direction, 0 to 359)");return true;}
        ++k;
    }
    const bool facing_word=g.face;
    size_t i=0;
    if(w.empty()) {say("Goto: where to? Type goto help for examples");return true;}
    // "goto N": the start of level N (the hidden indices too), as level select.
    if(w.size()==1 && number(w[0],v)) {
        if(v==current){std::snprintf(reply,sizeof reply,"Already in level %ld. Try goto room 12, or goto rooms",v);say(reply);return true;}
        if(v<0 || v>31){say("Goto: no such level. Type levels for the list");return true;}
        if(const char* refusal=ttk::level_travel(v,goto_hidden_level(v))) {
            std::snprintf(reply,sizeof reply,"Goto: cannot travel now: %s",refusal);say(reply);return true;
        }
        std::snprintf(reply,sizeof reply,"Travelling to %ld %s...",v,goto_level_name(v).c_str());say(reply);
        std::fprintf(stderr,"[TTK goto] request level %ld start\n",v);
        close=true;return true;
    }
    const bool lead=w.size()>=2 && number(w[0],v) && (w[1]=="room" || w[1]=="object" || w[1]=="at");
    if(lead){number(w[0],g.level);i=1;}
    if(i<w.size() && w[i]=="at")++i;   // "goto L at X Y Z": a point in another level
    std::snprintf(g.label,sizeof g.label,"%s",rest);
    if(i<w.size() && w[i]=="room") {
        if(i+1>=w.size() || !number(w[i+1],v)){say("Try: goto room 12, or goto 3 room 12 (goto rooms: how many)");return true;}
        g.kind=GotoRequest::Room;g.room=int(v);
        if(i+2<w.size()){if(!number(w[i+2],v) || facing_word){say("Try: goto room 12 face 90");return true;}g.face=true;g.degrees=int(v);}
    } else if(i<w.size() && w[i]=="object") {
        if(i+1>=w.size() || !number(w[i+1],v)){say("Try: goto object 17, or goto 3 object 17");return true;}
        g.kind=GotoRequest::Object;g.n=int(v);
        if(i+2<w.size() && number(w[i+2],v))g.distance=int(std::clamp(v,200L,4000L));
        if(i+3<w.size() && number(w[i+3],v) && !facing_word){g.face=true;g.degrees=int(v);}
    } else if(w.size()-i>=3 && number(w[i],v)) {
        long x,y,z;
        if(!number(w[i],x) || !number(w[i+1],y) || !number(w[i+2],z)){say("Try: goto 45000 -300 -20600 (x y z)");return true;}
        g.kind=GotoRequest::Point;g.x=int32_t(x);g.y=int32_t(y);g.z=int32_t(z);
        if(i+3<w.size() && number(w[i+3],v) && !facing_word){g.face=true;g.degrees=int(v);}
    } else if(w.size()-i==1) {
        bool found=false;
        for(const auto& m:goto_landmarks()) if(spawn_same(m.name.c_str(),w[i].c_str())) {
            found=true;g.level=m.level;
            if(!facing_word){g.face=m.face;g.degrees=m.degrees;}
            if(m.room){g.kind=GotoRequest::Room;g.room=m.r;}
            else {g.kind=GotoRequest::Point;g.x=m.x;g.y=m.y;g.z=m.z;}
            break;
        }
        if(!found){std::snprintf(reply,sizeof reply,"Goto: no landmark '%s'. Type goto landmarks for the list",w[i].c_str());say(reply);return true;}
    } else {say("Goto: not understood. Type goto help for examples");return true;}
    if(g.level==current)g.level=-1;
    if(g.level>=0) {
        if(const char* refusal=ttk::level_travel(g.level,goto_hidden_level(g.level))) {   // hidden indices: developer, as level force
            std::snprintf(reply,sizeof reply,"Goto: cannot travel now: %s",refusal);say(reply);return true;
        }
        std::snprintf(reply,sizeof reply,"Travelling to %ld %s, then %s...",g.level,goto_level_name(g.level).c_str(),g.label);say(reply);
    } else {
        std::snprintf(reply,sizeof reply,"Going to %s...",g.label);say(reply);
    }
    {std::lock_guard<std::mutex> lock(goto_mutex);goto_wait=0;goto_settle=0;goto_request=g;}
    std::fprintf(stderr,"[TTK goto] request %s\n",rest);
    close=true;
    return true;
}
// D26L2 (console_syntax.cpp): the run-time names the highlighter colours. A
// word only begins a name while it is still being typed (prefix).
static int console_name_match(const std::string& word,const char* name,bool prefix) {
    const size_t n=std::strlen(name);
    if(word.size()>n)return 0;
    for(size_t i=0;i<word.size();++i)if(std::tolower((unsigned char)word[i])!=std::tolower((unsigned char)name[i]))return 0;
    return word.size()==n ? 1 : prefix ? 2 : 0;
}
int console_spawn_name(const std::string& rest,bool prefix) {
    int best=0;
    for(const auto& item:spawn_items)best=std::max(best,console_name_match(rest,item.name,prefix)==1?3:console_name_match(rest,item.name,prefix));
    for(const auto& c:spawn_crystals)best=std::max(best,console_name_match(rest,c.name,prefix)==1?3:console_name_match(rest,c.name,prefix));
    return best==3 ? 1 : best;
}
int console_landmark_name(const std::string& word,bool prefix) {
    // landmarks.txt is read again at most every two seconds (the console
    // classifies every visible row each time it redraws).
    static std::vector<std::string> names;
    static uint32_t read_at;
    static bool read;
    const uint32_t now=SDL_GetTicks();
    if(!read || now-read_at>2000) {
        names.clear();
        for(const auto& m:goto_landmarks())names.push_back(m.name);
        read=true;read_at=now;
    }
    int best=0;
    for(const auto& n:names){const int m=console_name_match(word,n.c_str(),prefix);if(m==1)return 1;best=std::max(best,m);}
    return best;
}
