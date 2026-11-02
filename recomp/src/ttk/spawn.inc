// D26E debug spawn (backtick console "spawn <item>"), the TTK counterpart of
// EDuke32's spawn: a new pickup object in front of Duke, made by the game's own
// creator. Contract: documentation/111-d26e-debug-spawn.md.
//
// 80095A74 CreateObject(type, cell hint, position*) takes a 96-byte object from
// the free list 800DDAFC, copies x/y/z/w, finds the map cell of the position
// (8005D790, the hint when it fails), links the object into that cell's list
// and calls 80095504, which sets +0x2c and attaches the type's model. It is what
// a destroyed container or enemy uses to drop its item (80096B98 calls it with
// its own position and sets the new object's +0x35 to 1); this does the same.
// 80095504 needs a model only when the type's flags (type table 800D2660,
// 28 bytes per type) have bit 2; the model comes from the per-level table
// *800DDB18 at the type's +0x10. A level loads only its own models, so a type
// without one here is refused instead of creating an invisible object.
struct SpawnItem {const char* name;uint16_t type;int8_t level;uint8_t group=0;}; // group: 0 item, 1 weapon, 2 ammo
// Names follow the game's own strings. Mission slots 153/154 change name per
// level (inventory screen); -1 = any level.
static constexpr SpawnItem spawn_items[]={
    {"subway security key",153,0},{"transport room id",154,0},
    {"warehouse key",153,5},{"gantry key",153,7},{"valve key",154,7},
    {"lab key",153,9},{"valve room key",154,9},
    {"key 1",153,-1},{"key 2",154,-1},
    {"generic crystal",761,-1},
    {"skeleton key",542,-1},{"scrap of paper",547,-1},{"old note",548,-1},{"torn paper",549,-1},
    {"family jewel",854,-1},{"family jewel 2",855,-1},{"family jewel 3",856,-1},
    {"steroids",638,-1},{"armor",320,-1},{"health",473,-1},{"atomic health",581,-1},
    {"medkit",580,-1},{"biomask",560,-1},{"goggles",573,-1},{"jetpack",753,-1},
    {"invulnerability",1047,-1},{"invisibility",1045,-1},{"double duke",1046,-1},
    // Weapons and ammo: the type cases of the pickup dispatcher 0x80081a48
    // that give a weapon (0x800818ec, or bit 0 set with the ammo for 12-14)
    // or only its ammo (0x80081720). Names follow the weapon slots
    // (documentation/33-controls-shortcuts.md). Upgraded slots (27-29) come
    // from the same pickups when Duke has the upgrade.
    {"throwing knife",45,-1,1},{"throwing axe",1,-1,1},{"crossbow",21,-1,1},
    {"desert eagle",59,-1,1},{"combat shotgun",66,-1,1},{"buffalo rifle",10,-1,1},
    {"gatling gun",41,-1,1},{"rpg",47,-1,1},{"flamethrower",37,-1,1},
    {"energy weapon",34,-1,1},{"freezer",641,-1,1},{"pipe bomb",58,-1,1},
    {"holy hand grenade",608,-1,1},{"dynamite",126,-1,1},
    {"crossbow ammo",609,-1,2},{"desert eagle ammo",33,-1,2},{"shotgun ammo",65,-1,2},
    {"buffalo rifle ammo",124,-1,2},{"gatling ammo",562,-1,2},{"rpg ammo",106,-1,2},
    {"flamethrower ammo",38,-1,2},{"energy weapon ammo",125,-1,2},{"freezer ammo",642,-1,2},
};
static constexpr uint32_t spawn_types=0x800d2660,spawn_models=0x800ddb18,spawn_free=0x800ddafc;
static constexpr unsigned spawn_type_count=0x7428/28,spawn_distance=600;
static std::atomic<int> spawn_request{-1};
static std::atomic<unsigned> spawn_requests;

// A model is needed and loaded, or not needed (the original's own rule).
static bool spawn_model_ready(unsigned type) {
    const uint32_t types=psx_mod_read_word(spawn_types),models=psx_mod_read_word(spawn_models);
    if(type==0 || type>=spawn_type_count || !cheat_ram(types,28*spawn_type_count))return false;
    if(!(psx_mod_read_word(types+28*type)&2))return true;
    const unsigned model=psx_mod_read_half(types+28*type+16);
    if(!cheat_ram(models,4*(model+1)))return false;
    const uint32_t address=psx_mod_read_word(models+4*model);
    return address>=0x80010000u && address<0x80200000u;
}
static const char* spawn_name(unsigned type,unsigned level) {
    for(auto& item:spawn_items)if(item.type==type && (item.level<0 || unsigned(item.level)==level))return item.name;
    return nullptr;
}
// D26F custom pickups: the level crystals as walk-over items. Contract:
// documentation/117-d26f-crystals.md ("Custom pickups").
//
// The game's crystals (types 176/177/178) sit in holders and are taken with
// the action button by 80091CEC, which sets inventory item 11/12/13. Their type
// flags lack the walk-over pickup bit 0x20000000, so the pickup loop (8007FD04)
// never touches them and the dispatcher 80081A48 has no case for them. Type 761
// is a different, generic walk-over crystal that sets item 14.
// Console-only numbers (above the 1062 real types) create the real crystal
// object; spawn_crystal_update then performs the pickup the way the
// dispatcher's common tail does for a touched item.
struct SpawnCrystal {const char* name;uint16_t number,type,item,message;};
static constexpr SpawnCrystal spawn_crystals[]={
    {"red energy crystal",1761,176,11,113},
    {"blue energy crystal",2761,177,12,114},
    {"green energy crystal",3761,178,13,115},
};
// Only levels whose inventory names items 11-13 (0x80087D4C): 0, 5, 9.
static bool spawn_crystal_level(unsigned level){return level==0 || level==5 || level==9;}
static const SpawnCrystal* spawn_crystal(int number) {
    for(auto& c:spawn_crystals)if(c.number==number)return &c;
    return nullptr;
}
struct SpawnedCrystal {uint32_t object;const SpawnCrystal* crystal;};
static std::vector<SpawnedCrystal> spawned_crystals;
static uint32_t spawned_level,spawned_loads;
static bool spawn_item_found(unsigned item){return psx_mod_read_half(player+0x354+4*item)&1;}
// The level's own objects (table 800D2550, 64 pointers) include its real
// crystals. A tracked address that shows up there is never ours.
static bool spawn_level_object(uint32_t object) {
    for(unsigned i=0;i<64;++i)if(psx_mod_read_word(0x800d2550+4*i)==object)return true;
    return false;
}
// Touch range of the original pickup loop (8007FD04): |dx| and |dz| below the
// object's radius +0x5a when it exceeds 0x100, else below 0x100. Its object
// query box is 0x258 tall; |dy| is held to that.
static bool spawn_crystal_touched(uint32_t object) {
    const int32_t dx=(int32_t)psx_mod_read_word(object+4)-(int32_t)psx_mod_read_word(player+4);
    const int32_t dy=(int32_t)psx_mod_read_word(object+8)-(int32_t)psx_mod_read_word(player+8);
    const int32_t dz=(int32_t)psx_mod_read_word(object+12)-(int32_t)psx_mod_read_word(player+12);
    const int32_t radius=std::max<int32_t>(0x100,(int16_t)psx_mod_read_half(object+0x5a));
    return std::abs(dx)<radius && std::abs(dz)<radius && std::abs(dy)<0x258;
}
// The dispatcher's common tail (80082D04..80082E4C) for a touched item, minus
// the multiplayer respawn: message, sound, out of its cell, out of the active
// list, back to the free list.
static bool spawn_crystal_collect(CPUState* cpu,uint32_t object,const SpawnCrystal& c) {
    const unsigned index=psx_mod_read_byte(player+0x233);
    const uint32_t players=std::max<uint32_t>(1,psx_mod_read_word(0x800c27bc));
    const uint32_t cells=psx_mod_read_word(0x800da454);
    const int cell=int8_t(psx_mod_read_byte(object+0x2e));
    if(!cheat_ram(cells,4) || cell<0)return false;
    const uint32_t list[1]={object};
    uint32_t removed=0;
    if(!cheat_call(cpu,0x8001c9ac,cells+88*cell+0x48,object,list,1,&removed) || !removed)return false; // out of the cell
    const uint32_t flag=player+0x354+4*c.item;
    psx_mod_write_half(flag,psx_mod_read_half(flag)|1);           // as 80091FC4 stores a taken crystal
    psx_mod_write_half(0x800dcd90+2*index,c.message);             // pickup message id (80082D20)
    psx_mod_write_word(0x800dcd98+4*index,0x4b0/players);         // its display time (80082DD4)
    cheat_call(cpu,0x8006bbd8,0x7005);                            // 761's pickup sound
    if(psx_mod_read_byte(object+0x35)==2)                          // still bouncing: as 80082D4C
        cheat_call(cpu,0x8008f104,psx_mod_read_word(object+0x10));
    cheat_call(cpu,0x8001c9ac,0x800c5694,object,list,1);           // out of the active list (search: safe when absent)
    if((int16_t)psx_mod_read_half(object+0x30)==-1)cheat_call(cpu,0x80095c7c,object); // back to the free list
    else psx_mod_write_byte(object+0x35,0x63);
    return true;
}
static void spawn_crystal_update(CPUState* cpu) {
    if(spawned_crystals.empty())return;
    // Forget everything across a savestate load or level change: the pool is
    // rebuilt and an address may then hold an unrelated object.
    if(psx_mod_savestate_loads()!=spawned_loads || psx_mod_read_word(0x800be570)!=spawned_level) {
        spawned_crystals.clear();return;
    }
    // Collect only where cheat_call can run (same gate as the spawn itself), so
    // the flag is never set without the object leaving the world.
    if(!player_identity_ready() || psx_mod_read_word(0x800c27bc)!=1 || guest_busy ||
       cpu->gpr[29]<0x801f4000 || g_precise_mode || g_ls_mode || g_psx_call_bail)return;
    for(size_t i=0;i<spawned_crystals.size();) {
        const SpawnedCrystal s=spawned_crystals[i];
        if(!cheat_ram(s.object,96) || psx_mod_read_half(s.object+0x2c)!=s.crystal->type || spawn_level_object(s.object)) {
            spawned_crystals.erase(spawned_crystals.begin()+i);continue;
        }
        // The bounce engine ends with +0x35 = 0xff; a pickup's own update turns
        // that into 0 (800816BC). A crystal has no such update, so do it here.
        if(psx_mod_read_byte(s.object+0x35)==0xff)psx_mod_write_byte(s.object+0x35,0);
        if(spawn_crystal_touched(s.object)) {
            const bool ok=spawn_crystal_collect(cpu,s.object,*s.crystal);
            std::fprintf(stderr,"[TTK spawn] crystal %u object %08x collected item %u %s\n",
                         s.crystal->number,s.object,s.crystal->item,ok?"ok":"FAILED");
            spawned_crystals.erase(spawned_crystals.begin()+i);continue;
        }
        ++i;
    }
}
// D26H: out of Duke's pickup reach. The pickup loop's touch test is a square
// (|dx| and |dz| each below max(0x100, +0x5a), see spawn_crystal_touched), and
// +0x5a is the model's bounding radius set at creation: 579 for the
// flamethrower. 600 along a diagonal view is only 424 per axis, inside it.
// Move the new object out until its larger axis offset clears the reach by
// spawn_reach_margin: along the view, else up to spawn_turn_limit degrees to
// either side (nearest first; nearer an axis needs less distance). A spot is
// usable when the way there is open (original segment query 0x8006d980 at
// Duke's height, no world hit before the spot + spawn_wall_margin), no solid
// placed object (a dumpster: not world to that query) crosses the way or the
// spot, and a floor lies within spawn_drop_limit below Duke's feet there (no
// ledge). With none, it stays at spawn_distance, as before. 0x80095cb0 relinks
// it to its new cell.
static constexpr int32_t spawn_reach_margin=128,spawn_wall_margin=128,spawn_drop_limit=256;
static constexpr int spawn_turn_step=10,spawn_turn_limit=40;
static unsigned spawn_moves,spawn_blocked;
// The room lists (0x800da454, 88-byte rooms, list at +0x48) hold every placed
// object; solid ones are those push_step_clear also stops at (type flags set,
// no flag 8). The way from Duke to the spot + margin, widened by the margin,
// must miss each of their boxes that spans Duke's standing height.
static bool spawn_objects_clear(uint32_t object,double px,double pz,double tx,double tz,int32_t feet) {
    const uint32_t rooms=psx_mod_read_word(0x800da454),count=psx_mod_read_word(0x800da47c);
    if(count>128 || (count && (rooms<0x80010000 || rooms>0x80200000-count*88 || (rooms&3))))return false;
    for(unsigned room=0;room<count;++room) {
        uint32_t node=psx_mod_read_word(rooms+88*room+0x48);
        for(unsigned n=0;node && n<1024;++n) {
            if(node<0x80010000 || node>0x801ffff0 || (node&3))return false;
            const uint32_t other=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            const uint32_t flags=object_type_flags(other);
            if(other==object || other==player || !flags || (flags&8))continue;
            for(unsigned i=0,boxes=object_box_count(other);i<boxes;++i) {
                PushBounds q;if(!push_bounds(other,q,i))continue;
                if(q.y1<=feet-0x370 || q.y0>=feet)continue;
                // Segment against the widened box on the floor plane (slabs).
                double t0=0,t1=1;
                const double from[2]={px,pz},span[2]={tx-px,tz-pz};
                const double lo[2]={double(q.x0-spawn_wall_margin),double(q.z0-spawn_wall_margin)};
                const double hi[2]={double(q.x1+spawn_wall_margin),double(q.z1+spawn_wall_margin)};
                for(int k=0;k<2 && t0<=t1;++k) {
                    if(std::fabs(span[k])<1e-6){if(from[k]<lo[k] || from[k]>hi[k])t0=2;continue;}
                    double a=(lo[k]-from[k])/span[k],b=(hi[k]-from[k])/span[k];
                    if(a>b)std::swap(a,b);
                    t0=std::max(t0,a);t1=std::min(t1,b);
                }
                if(t0<=t1)return false;
            }
        }
        if(node)return false;
    }
    return true;
}
static bool spawn_clear_of_reach(CPUState* cpu,uint32_t object,double view) {
    const int32_t reach=std::max<int32_t>(0x100,(int16_t)psx_mod_read_half(object+0x5a));
    const double px=(int32_t)psx_mod_read_word(player+4),py=(int32_t)psx_mod_read_word(player+8),pz=(int32_t)psx_mod_read_word(player+12);
    const int32_t feet=(int32_t)psx_mod_read_word(player+0x1c8);
    for(int turn=0;turn<=spawn_turn_limit;turn=turn>0?-turn:spawn_turn_step-turn) {
        const double a=view+turn*3.141592653589793/180,s=std::sin(a),c=std::cos(a);
        const double need=std::max<double>(spawn_distance,(reach+spawn_reach_margin)/std::max(std::fabs(s),std::fabs(c)));
        if(!turn && need<=spawn_distance)return true;
        const double from[3]={px,py,pz},ahead[3]={px+s*(need+spawn_wall_margin),py,pz+c*(need+spawn_wall_margin)};
        const double spot[3]={px+s*need,py,pz+c*need},below[3]={spot[0],double(feet+spawn_drop_limit),spot[2]};
        double hit[3];int wall=0,floor=0;
        if(!view_segment_query(cpu,from,ahead,hit,wall) || wall==1 ||
           !view_segment_query(cpu,spot,below,hit,floor) || floor!=1 ||
           !spawn_objects_clear(object,px,pz,ahead[0],ahead[2],feet))continue;
        psx_mod_write_word(object+4,uint32_t(int32_t(std::lround(spot[0]))));
        psx_mod_write_word(object+12,uint32_t(int32_t(std::lround(spot[2]))));
        int32_t ignored=0;
        hull_call(cpu,0x80095cb0,object,&ignored);
        ++spawn_moves;
        std::fprintf(stderr,"[TTK spawn] reach %d: moved to %.0f ahead, %+d degrees\n",reach,need,turn);
        return true;
    }
    ++spawn_blocked;
    std::fprintf(stderr,"[TTK spawn] reach %d: no clear spot within %d degrees, kept at %u\n",reach,spawn_turn_limit,spawn_distance);
    return false;
}
static int spawn_pending=-1;
static uint64_t spawn_deadline;
static void spawn_update(CPUState* cpu) {
    // A typed request arrives while the console has just released the mouse,
    // so it does not need captured input, only a verified Modernized gameplay
    // player. Wait up to ~5 s (300 host frames) for one, then give up.
    spawn_crystal_update(cpu);
    const int fresh=spawn_request.exchange(-1);
    if(fresh>=0){spawn_pending=fresh;spawn_deadline=input_host_frame()+300;}
    if(spawn_pending<0)return;
    if(!player_identity_ready() || psx_mod_read_word(0x800c27bc)!=1 ||
       guest_busy || cpu->gpr[29]<0x801f4000 || g_precise_mode || g_ls_mode || g_psx_call_bail) {
        if(input_host_frame()>spawn_deadline){spawn_pending=-1;input_notice("Spawn unavailable here");}
        return;
    }
    const SpawnCrystal* crystal=spawn_crystal(spawn_pending);
    const int type=crystal?crystal->type:spawn_pending;spawn_pending=-1;
    const unsigned level=psx_mod_read_word(0x800be570);
    if(crystal && !spawn_crystal_level(level)) {input_notice("Spawn: no energy crystals in this level");return;}
    if(!spawn_model_ready(unsigned(type))) {input_notice("Spawn: not available in this level");return;}
    if(!psx_mod_read_word(spawn_free)) {input_notice("Spawn: no free object");return;}
    // In front of the camera (the modern view yaw, 0 = +Z, 1024 = +X), so the
    // item lands where the player looks even when Duke's body faces elsewhere.
    const double a=view_yaw();
    const int32_t x=(int32_t)psx_mod_read_word(player+4)+int32_t(std::lround(std::sin(a)*spawn_distance));
    const int32_t z=(int32_t)psx_mod_read_word(player+12)+int32_t(std::lround(std::cos(a)*spawn_distance));
    const int32_t feet=(int32_t)psx_mod_read_word(player+8);
    const uint32_t position[4]={uint32_t(x),uint32_t(feet),uint32_t(z),psx_mod_read_word(player+16)};
    const uint32_t cell=uint32_t(int32_t(int8_t(psx_mod_read_byte(player+0x2e))));
    uint32_t object=0;
    if(!cheat_call(cpu,0x80095a74,uint32_t(type),cell,position,4,&object) || !cheat_ram(object,96) ||
       psx_mod_read_half(object+0x2c)!=unsigned(type)) {
        input_notice("Spawn could not complete");return;
    }
    const bool clear=spawn_clear_of_reach(cpu,object,a);
    // Like a real drop (80096B98), the original fall then places the item: it
    // overrides this start height and rests where the game's own enemy drops
    // rest (checked: a pig cop's ammo drop and a spawned item settle at the same
    // height on the level 0 street). Any "half in the ground" look is the game's.
    psx_mod_write_byte(object+0x35,1); // as 80096B98 marks a dropped item: the original fall and bounce
    // A pickup type's own update (800814D4) starts that fall. A crystal type is
    // static scenery and never runs it, so run it once: with +0x35 == 1 it
    // registers the object with the bounce engine (8008F150, handle in +0x10,
    // the engine clears +0x35 on landing) or, failing that, rests it at the
    // floor (8005D358) - 50. Its type-specific spin skips 176-178.
    if(crystal)cheat_call(cpu,0x800814d4,object);
    char text[64];
    const char* name=spawn_name(unsigned(type),level);
    if(crystal) {
        if(spawned_crystals.empty() || spawned_loads!=psx_mod_savestate_loads() || spawned_level!=level) {
            spawned_crystals.clear();spawned_loads=psx_mod_savestate_loads();spawned_level=level;
        }
        spawned_crystals.push_back({object,crystal});
        std::snprintf(text,sizeof text,"Spawned %s%s",crystal->name,spawn_item_found(crystal->item)?" (already found)":"");
    } else if(type==761 && spawn_crystal_level(level))
        std::snprintf(text,sizeof text,"Spawned generic crystal (item 14)");
    else if(name)std::snprintf(text,sizeof text,"Spawned %s",name);
    else std::snprintf(text,sizeof text,"Spawned type %d",type);
    if(!clear)std::strncat(text," (close: way ahead blocked)",sizeof text-std::strlen(text)-1);
    for(char* c=text;*c;++c)*c=char(std::toupper((unsigned char)*c));
    input_notice(text);
    std::fprintf(stderr,"[TTK spawn] type %d object %08x at %d,%d,%d cell %d\n",type,object,(int32_t)psx_mod_read_word(object+4),
                 (int32_t)position[1],(int32_t)psx_mod_read_word(object+12),(int)int8_t(psx_mod_read_byte(object+0x2e)));
}
static bool spawn_same(const char* a,const char* b) {
    for(;*a && *b;++a,++b)if(std::tolower((unsigned char)*a)!=std::tolower((unsigned char)*b))return false;
    return !*a && !*b;
}
// Runtime event-pump thread, between guest frames: parse and queue only (the
// emulation thread checks identity and the player before creating anything).
bool spawn_console_command(const char* line,void (*say)(const char*),bool& close) {
    close=false;
    const bool list=!std::strcmp(line,"items") || !std::strcmp(line,"spawn");
    if(!list && std::strncmp(line,"spawn ",6))return false;
    if(!input_modernized()) {say("Spawn: Modernized profile only (Vanilla is unchanged)");return true;}
    if(psx_mod_read_half(0x800bcbb0)!=1 || psx_mod_read_half(0x800ce1b0)!=2) {
        say("Spawn: start or load a game first");return true;
    }
    const unsigned level=psx_mod_read_word(0x800be570);
    if(list) {
        char row[100]{};size_t used=0;
        // D08A33: weapons and their ammo get their own lists (user, 2026-11-02).
        static const char* const titles[]={"Spawnable here (spawn <name> or spawn <type>):",
            "Weapons here:","Ammo here:"};
        for(unsigned group=0;group<3;++group) {
            bool first=true;
            for(auto& item:spawn_items) {
                if(item.group!=group || (item.level>=0 && unsigned(item.level)!=level) || !spawn_model_ready(item.type))continue;
                if(first){say(titles[group]);first=false;}
                char entry[40];std::snprintf(entry,sizeof entry,"%s (%u)",item.name,item.type);
                if(used+std::strlen(entry)+2>=44) {say(row);used=0;row[0]=0;}
                used+=std::snprintf(row+used,sizeof row-used,"%s%s",used?", ":"  ",entry);
            }
            if(used)say(row);
            used=0;row[0]=0;
            if(group==0 && spawn_crystal_level(level))
                say("  Crystals: red energy crystal (1761), blue (2761), green (3761)");
        }
        // Every other pickup-class type (type byte 6 == 1) whose model this
        // level has loaded, by number: unnamed slots.
        const uint32_t types=psx_mod_read_word(spawn_types);
        bool first=true;
        for(unsigned type=1;cheat_ram(types,28*spawn_type_count) && type<spawn_type_count;++type) {
            if(psx_mod_read_byte(types+28*type+6)!=1 || !(psx_mod_read_word(types+28*type)&2) || !spawn_model_ready(type))continue;
            bool named=false;
            for(auto& item:spawn_items)named|=item.type==type && (item.level<0 || unsigned(item.level)==level);
            if(named)continue;
            if(first){say("Other pickups here (spawn <type>):");first=false;}
            char entry[8];std::snprintf(entry,sizeof entry,"%u",type);
            if(used+std::strlen(entry)+1>=44) {say(row);used=0;row[0]=0;}
            used+=std::snprintf(row+used,sizeof row-used,"%s%s",used?" ":"  ",entry);
        }
        if(used)say(row);
        return true;
    }
    const char* what=line+6;
    while(*what==' ')++what;
    int type=-1;
    char* end=nullptr;
    const long number=std::strtol(what,&end,10);
    if(end!=what && !*end) type=int(number);
    else {
        for(auto& c:spawn_crystals)if(spawn_same(what,c.name)){type=c.number;break;}
    }
    if(const SpawnCrystal* c=spawn_crystal(type)) {
        if(!spawn_crystal_level(level)) {say("Spawn: no energy crystals in this level (levels 0, 5 and 9)");return true;}
        if(!spawn_model_ready(c->type)) {say("Spawn: that crystal is not loaded in this level");return true;}
        spawn_request.store(type);++spawn_requests;
        char reply[80];std::snprintf(reply,sizeof reply,"Spawning %s...",c->name);
        say(reply);
        close=true;
        return true;
    }
    if(type<0) {
        for(auto& item:spawn_items)
            if(spawn_same(what,item.name) && (item.level<0 || unsigned(item.level)==level)){type=item.type;break;}
    }
    if(type<=0 || unsigned(type)>=spawn_type_count) {say("Spawn: unknown item - type items for the list");return true;}
    if(!spawn_model_ready(unsigned(type))) {say("Spawn: that item is not loaded in this level");return true;}
    if(type==761 && spawn_crystal_level(level))
        say("Note: 761 is the generic crystal (item 14); this level's crystals are 1761-3761");
    spawn_request.store(type);++spawn_requests;
    char reply[80];std::snprintf(reply,sizeof reply,"Spawning %s...",what);
    say(reply);
    close=true;
    return true;
}
