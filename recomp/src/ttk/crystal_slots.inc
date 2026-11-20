// D08A43: an energy crystal put into a level device shows in it until it is
// taken back (Modernized; Vanilla never runs it). Included in
// modern_controls.cpp after spawn.inc. Note 519.
//
// The game's crystals (types 176/177/178, items 11/13/12) sit in holders.
// The generic holder (handler 0x80092e98, kind 17) shows a returned crystal
// itself: it moves the crystal object to the holder (194 up), finds its room
// (0x8005d790) and links it into that room's object list (room +0x48,
// 0x8001c6dc). A level's own crystal device does not. OBEY OR DIE's conveyor
// power slot (LEVEL05 overlay, handler 0x800cd878 subtype 1, type 728) takes
// any crystal in Duke's hand, stores its state in +0x35 (1 red, 2 green,
// 3 blue), switches its own model to the type's model 1 (the ring turns
// green, 0x80092bdc) and clears the hand, but the crystal object stays out of
// every room list: the crystal vanishes. Taking it back is the game's crystal
// take (0x80091cec): the state in +0x35 names the crystal (0x80092d78(level,
// state - 1) gives its slot in the level object table 0x800d2550), the take
// unlinks it from its room's list (0x8001c9ac, a no-op when it is in none),
// hands it to Duke, calls the device's handler in mode 3 (the slot powers
// off) and clears +0x35.
//
// Rule (game-wide, from the game's own data): for each crystal of the level
// (0x80092d78) that is neither found nor in Duke's hand, a level object whose
// +0x35 names that crystal, which is not a crystal, not a generic holder, and
// whose type has more than one model (a device with states), holds it; one
// whose model the game has switched from its first is preferred. The
// crystal object is then linked into that device's room at the device: for a
// wall plate (its thinnest axis horizontal) lying along the plate's normal,
// pushed into the plate's centre from the face that looks into its room with
// 60 units showing; for a flat device standing on its top. Taken back, the
// crystal is set upright again for the holders. The game's
// take unlinks it as before. Everything lives in guest RAM, so savestates and
// older saves carry on; a crystal this rule placed whose device let go of it
// without the take is unlinked again. A crystal still in the holder it
// started in (an inventory cheat gave its flag) leaves that holder for the
// device, the holder emptied as by the take. Developer: DNTTK_CRYSTAL_SLOTS=0 off,
// DNTTK_CRYSTAL_OUT=N how much of a plate's crystal shows (default 60).
static uint64_t crystal_slot_links=0,crystal_slot_unlinks=0,crystal_slot_released=0;
static uint32_t crystal_slot_map_level=0xffffffffu,crystal_slot_map_loads=0,crystal_slot_map_table=0;
static uint32_t crystal_slot_map[3]={};
struct CrystalShown {uint32_t object=0;int32_t p[3]={};};
static CrystalShown crystal_shown[3];
static constexpr uint32_t crystal_rooms_table=0x800da454,crystal_generic_holder=0x80092e98;
// The game routine with both arguments set (cheat_call leaves a1 alone when 0).
static bool crystal_call(CPUState* cpu,uint32_t address,uint32_t a0,uint32_t a1,uint32_t* result) {
    CPUState copy=*cpu;
    const uint32_t top=copy.gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail)return false;
    uint8_t stack[1024];
    for(unsigned i=0;i<sizeof stack;++i)stack[i]=psx_mod_read_byte(top-1024+i);
    copy.pc=0;copy.gpr[29]=top-256;copy.gpr[31]=0x800000fc;copy.gpr[4]=a0;copy.gpr[5]=a1;
    guest_busy=true;psx_mod_host_call_begin();psx_dispatch_call(&copy,address,0x800000fc);psx_mod_host_call_end();guest_busy=false;
    if(result)*result=copy.gpr[2];
    for(unsigned i=0;i<sizeof stack;++i)psx_mod_write_byte(top-1024+i,stack[i]);
    return copy.pc==0 && !g_psx_call_bail;
}
static bool crystal_ram(uint32_t a,uint32_t n){return a>=0x80010000u && a+n<=0x80200000u && !(a&3);}
static bool crystal_in_room(uint32_t room_entry,uint32_t object) {
    uint32_t node=psx_mod_read_word(room_entry+0x48);
    for(unsigned guard=0;node && guard<1024;++guard) {
        if(!crystal_ram(node,12))return false;
        if(psx_mod_read_word(node+8)==object)return true;
        node=psx_mod_read_word(node+4);
    }
    return false;
}
static unsigned crystal_item(unsigned type){return type==176?11:type==177?12:13;}
// Where a device shows its crystal (the crystal object's position).
static bool crystal_slot_spot(uint32_t device,uint32_t crystal,uint32_t rooms,int32_t out[3],int16_t angle[3]) {
    angle[0]=angle[1]=angle[2]=0;
    const uint32_t mesh=psx_mod_read_word(device+0x44),cmesh=psx_mod_read_word(crystal+0x44);
    if(!crystal_ram(mesh,12) || !crystal_ram(cmesh,12))return false;
    const uint32_t b=psx_mod_read_word(mesh+8),cb=psx_mod_read_word(cmesh+8);
    if(!crystal_ram(b,16) || !crystal_ram(cb,16))return false;
    auto bound=[](uint32_t at,double lo[3],double hi[3]) {
        for(int k=0;k<3;++k){lo[k]=(int16_t)psx_mod_read_half(at+2*k);hi[k]=(int16_t)psx_mod_read_half(at+8+2*k);}
    };
    double lo[3],hi[3],clo[3],chi[3];bound(b,lo,hi);bound(cb,clo,chi);
    for(int k=0;k<3;++k) if(hi[k]<lo[k] || chi[k]<clo[k])return false;
    // Model to world: yaw +0x1c (4096 a turn, 0 = +Z, 1024 = +X).
    const double a=(int16_t)psx_mod_read_half(device+0x1c)*(2*3.14159265358979/4096),ca=std::cos(a),sa=std::sin(a);
    auto rot=[&](const double* v,double* w){w[0]=v[0]*ca+v[2]*sa;w[1]=v[1];w[2]=-v[0]*sa+v[2]*ca;};
    double pos[3];for(int k=0;k<3;++k) pos[k]=(int32_t)psx_mod_read_word(device+4+4*k);
    double mc[3],wc[3];for(int k=0;k<3;++k) mc[k]=0.5*(lo[k]+hi[k]);rot(mc,wc);
    double centre[3];for(int k=0;k<3;++k) centre[k]=pos[k]+wc[k];
    const double crystal_mid=0.5*(clo[1]+chi[1]);   // crystal centre above its origin (y down)
    const double ex=hi[0]-lo[0],ey=hi[1]-lo[1],ez=hi[2]-lo[2];
    if(ey<=ex && ey<=ez) {           // flat: on top
        out[0]=int32_t(std::lround(centre[0]));out[2]=int32_t(std::lround(centre[2]));
        out[1]=int32_t(std::lround(pos[1]+lo[1]-chi[1]));
        return true;
    }
    // A plate: the face of its thin horizontal axis that looks into its room.
    const int axis=ex<=ez?0:2;const double half=0.5*(axis==0?ex:ez);
    double nm[3]={0,0,0};nm[axis]=1;double n[3];rot(nm,n);
    const int cell=(int8_t)psx_mod_read_byte(device+0x2e);
    double sign=1;
    if(cell>=0 && uint32_t(cell)<rooms) {
        const uint32_t r=psx_mod_read_word(crystal_rooms_table)+88*uint32_t(cell);
        double d=0;
        for(int k=0;k<3;k+=2) d+=n[k]*(0.5*((int32_t)psx_mod_read_word(r+4*k)+(int32_t)psx_mod_read_word(r+0x20+4*k))-centre[k]);
        sign=d<0?-1:1;
    }
    // The crystal lies along the plate's normal, pushed into its central hole
    // with crystal_out units of its length showing. A quarter turn about z
    // (+0x20) lays its long axis (model y) along world x, about x (+0x1e)
    // along z; its centre, crystal_mid down its axis from the origin, turns
    // with it.
    static const double crystal_out=[]{const char* t=std::getenv("DNTTK_CRYSTAL_OUT");return t?std::atof(t):60.0;}();
    const double length=chi[1]-clo[1];
    double off[3]={0,0,0};
    if(std::fabs(n[0])>=std::fabs(n[2])) {angle[2]=1024;off[0]=-crystal_mid;}
    else {angle[1]=1024;off[2]=crystal_mid;}
    double body[3];
    for(int k=0;k<3;++k) body[k]=centre[k]+sign*n[k]*(half-(0.5*length-crystal_out));
    out[0]=int32_t(std::lround(body[0]-off[0]));
    out[1]=int32_t(std::lround(centre[1]));
    out[2]=int32_t(std::lround(body[2]-off[2]));
    return true;
}
static void crystal_unlink(CPUState* cpu,uint32_t crystal,uint32_t rooms) {
    const int cell=(int8_t)psx_mod_read_byte(crystal+0x2e);
    if(cell<0 || uint32_t(cell)>=rooms)return;
    const uint32_t entry=psx_mod_read_word(crystal_rooms_table)+88*uint32_t(cell);
    if(crystal_call(cpu,0x8001c9ac,entry+0x48,crystal,nullptr))++crystal_slot_unlinks;
}

// Choosing the crystal (user, 2026-11-20: "just be able to use any of the 3
// crystals in that slot"). A crystal device asks for the first crystal Duke
// has, red, green, blue (its handler's mode 2), so with more than one it always
// took red. Enter on a browsed crystal in the mission inventory (D08A18) at
// such a device sets the other found crystals aside for the use: their found
// bit moves to bit 0x0800 of their own flag word (guest RAM, so a savestate in
// between keeps them), the device then asks for the browsed one and the normal
// E use runs. They come back as soon as the chosen crystal is in Duke's hand,
// or 120 game frames later if the use never starts, and after any load.
static constexpr uint16_t crystal_aside_bit=0x0800;
static unsigned crystal_aside_item=0;static uint64_t crystal_aside_until=0;static uint32_t crystal_aside_loads=0;
static uint64_t crystal_aside_frames=0,crystal_choices=0;
static uint32_t crystal_flag(unsigned item){return player+0x354+4*item;}
static void crystal_aside_restore() {
    for(unsigned i=11;i<=13;++i) {
        const uint16_t v=psx_mod_read_half(crystal_flag(i));
        if(v&crystal_aside_bit)psx_mod_write_half(crystal_flag(i),uint16_t((v&~crystal_aside_bit)|1));
    }
    crystal_aside_item=0;
}
static bool crystal_mission_usable(CPUState* cpu,uint32_t handler,uint32_t object,int wanted,int item) {
    if(wanted<11 || wanted>13 || item<11 || item>13 || wanted==item)return false;
    if(!(psx_mod_read_half(crystal_flag(unsigned(item)))&1))return false;
    for(unsigned i=11;i<=13;++i) {
        if(int(i)==item)continue;
        const uint16_t v=psx_mod_read_half(crystal_flag(i));
        if(v&1)psx_mod_write_half(crystal_flag(i),uint16_t((v&~1)|crystal_aside_bit));
    }
    int32_t now=-1,blocked=1;
    const bool ok=original_call(cpu,handler,object,&now,2) && now==item && original_call(cpu,handler,object,&blocked,0) && blocked==0;
    if(!ok) {crystal_aside_restore();return false;}
    crystal_aside_item=unsigned(item);crystal_aside_until=crystal_aside_frames+120;crystal_aside_loads=psx_mod_savestate_loads();
    ++crystal_choices;
    return true;
}
// Each game frame: give the set-aside crystals back once the use has the chosen one.
static void crystal_aside_update() {
    ++crystal_aside_frames;
    bool any=false;
    for(unsigned i=11;i<=13;++i) any|=(psx_mod_read_half(crystal_flag(i))&crystal_aside_bit)!=0;
    if(!any)return;
    const bool held=crystal_aside_item && psx_mod_read_byte(player+0x3b8)==1 && psx_mod_read_byte(player+0x3b9)==crystal_aside_item;
    if(held || !crystal_aside_item || crystal_aside_frames>crystal_aside_until || crystal_aside_loads!=psx_mod_savestate_loads())
        crystal_aside_restore();
}
static void crystal_slots_update(CPUState* cpu) {
    if(player_identity_ready())crystal_aside_update();
    static const bool enabled=[]{const char* t=std::getenv("DNTTK_CRYSTAL_SLOTS");return !t || std::strcmp(t,"0");}();
    if(!enabled || !player_identity_ready() || psx_mod_read_word(0x800c27bc)!=1)return;
    const uint32_t level=psx_mod_read_word(0x800be570);
    if(!spawn_crystal_level(level))return;
    const uint32_t table=0x800d2550,rooms_table=psx_mod_read_word(crystal_rooms_table),rooms=psx_mod_read_word(0x800da47c);
    const uint32_t types=psx_mod_read_word(0x800d2660),handlers=psx_mod_read_word(0x800dd84c),models=psx_mod_read_word(0x800ddb18);
    if(!rooms || rooms>256 || !crystal_ram(rooms_table,rooms*88) || !crystal_ram(types,28*1062) || !crystal_ram(handlers,4) || !crystal_ram(models,4))return;
    // The level's crystal slots, from the game's own routine, once per level and load.
    const uint32_t first=psx_mod_read_word(table);
    if(crystal_slot_map_level!=level || crystal_slot_map_loads!=psx_mod_savestate_loads() || crystal_slot_map_table!=first) {
        for(uint32_t s=0;s<3;++s) {
            uint32_t slot=0;
            if(!crystal_call(cpu,0x80092d78,level,s,&slot) || slot>=64)return;
            crystal_slot_map[s]=slot;
        }
        crystal_slot_map_level=level;crystal_slot_map_loads=psx_mod_savestate_loads();crystal_slot_map_table=first;
        for(auto& c:crystal_shown)c=CrystalShown{};
    }
    for(uint32_t s=0;s<3;++s) {
        const uint32_t crystal=psx_mod_read_word(table+4*crystal_slot_map[s]);
        if(!crystal_ram(crystal,0x60))continue;
        const unsigned type=psx_mod_read_half(crystal+0x2c);
        if(type<176 || type>178)continue;
        const unsigned item=crystal_item(type);
        const bool found=psx_mod_read_half(player+0x354+4*item)&1;
        const bool held=psx_mod_read_byte(player+0x3b8)==1 && psx_mod_read_byte(player+0x3b9)==item;
        const int ccell=(int8_t)psx_mod_read_byte(crystal+0x2e);
        const bool linked=ccell>=0 && uint32_t(ccell)<rooms && crystal_in_room(rooms_table+88*uint32_t(ccell),crystal);
        CrystalShown& shown=crystal_shown[s];
        const bool ours=shown.object==crystal && linked &&
            (int32_t)psx_mod_read_word(crystal+4)==shown.p[0] && (int32_t)psx_mod_read_word(crystal+8)==shown.p[1] &&
            (int32_t)psx_mod_read_word(crystal+12)==shown.p[2];
        // Taken back: upright again for the next holder (crystals stand at 0, 0, 0).
        if((found || held) && (psx_mod_read_half(crystal+0x1e) || psx_mod_read_half(crystal+0x20)) &&
           !psx_mod_read_half(crystal+0x1c)) {
            for(int k=1;k<3;++k){psx_mod_write_half(crystal+0x1c+2*k,0);psx_mod_write_half(crystal+0x24+2*k,0);}
        }
        uint32_t device=0;
        // A device whose model the game switched (0x80092bdc on insert) first.
        for(int pass=0;pass<2 && !device && !found && !held;++pass) {
            for(uint32_t i=0;i<64 && !device;++i) {
                const uint32_t o=psx_mod_read_word(table+4*i);
                if(o==crystal || !crystal_ram(o,0x60) || psx_mod_read_byte(o+0x35)!=s+1)continue;
                const unsigned t=psx_mod_read_half(o+0x2c);
                if(t>=1062 || (t>=176 && t<=178))continue;
                const uint32_t handler=psx_mod_read_word(handlers+4*psx_mod_read_byte(o+0x19));
                if(handler==crystal_generic_holder || !psx_mod_read_half(types+28*t+0x12))continue;
                const uint32_t model0=psx_mod_read_word(models+4*psx_mod_read_half(types+28*t+0x10));
                if(pass==0 && psx_mod_read_word(o+0x44)==model0)continue;
                device=o;
            }
        }
        if(!device) {
            // A crystal this rule placed whose device let go of it without the take.
            if(ours && !found && !held) {crystal_unlink(cpu,crystal,rooms);shown=CrystalShown{};}
            else if(!linked || found || held) shown=CrystalShown{};
            continue;
        }
        if(linked && !ours) {
            // Still in the holder it started in (an inventory cheat such as
            // dnstuff sets the found flag without taking it): it is in the
            // device now, so the holder lets it go as the take would (+0x35 0).
            uint32_t holder=0;
            for(uint32_t i=0;i<64 && !holder;++i) {
                const uint32_t o=psx_mod_read_word(table+4*i);
                if(o==device || !crystal_ram(o,0x60) || psx_mod_read_byte(o+0x35)!=s+1)continue;
                if(psx_mod_read_word(handlers+4*psx_mod_read_byte(o+0x19))!=crystal_generic_holder)continue;
                const double dx=(int32_t)psx_mod_read_word(o+4)-(int32_t)psx_mod_read_word(crystal+4);
                const double dz=(int32_t)psx_mod_read_word(o+12)-(int32_t)psx_mod_read_word(crystal+12);
                if(dx*dx+dz*dz<600.0*600.0)holder=o;
            }
            if(!holder)continue;   // the game has it somewhere else: leave it
            psx_mod_write_byte(holder+0x35,0);++crystal_slot_released;
        }
        int32_t p[3];int16_t ang[3];
        if(!crystal_slot_spot(device,crystal,rooms,p,ang))continue;
        const int dcell=(int8_t)psx_mod_read_byte(device+0x2e);
        if(dcell<0 || uint32_t(dcell)>=rooms)continue;
        if(ours && p[0]==shown.p[0] && p[1]==shown.p[1] && p[2]==shown.p[2] && ccell==dcell && psx_mod_read_half(crystal+0x20)==uint16_t(ang[2]) && psx_mod_read_half(crystal+0x1e)==uint16_t(ang[1]))continue;
        if(linked)crystal_unlink(cpu,crystal,rooms);
        for(int k=0;k<3;++k)psx_mod_write_word(crystal+4+4*k,uint32_t(p[k]));
        // Its angles (+0x1c yaw, +0x1e about x, +0x20 about z; the drawn copy +0x24..).
        for(int k=0;k<3;++k){psx_mod_write_half(crystal+0x1c+2*k,uint16_t(ang[k]));psx_mod_write_half(crystal+0x24+2*k,uint16_t(ang[k]));}
        psx_mod_write_byte(crystal+0x2e,uint8_t(dcell));
        const uint32_t entry=rooms_table+88*uint32_t(dcell);
        uint32_t head=0;
        if(!crystal_call(cpu,0x8001c6dc,psx_mod_read_word(entry+0x48),crystal,&head) || !crystal_ram(head,12))continue;
        psx_mod_write_word(entry+0x48,head);
        // As the holder does: types with byte 6 set are on the second list too.
        if(psx_mod_read_byte(types+28*type+6)) {
            uint32_t h2=0;
            if(crystal_call(cpu,0x8001c6dc,psx_mod_read_word(0x800c5694),crystal,&h2) && crystal_ram(h2,12))psx_mod_write_word(0x800c5694,h2);
        }
        shown.object=crystal;std::copy(p,p+3,shown.p);
        ++crystal_slot_links;
        std::fprintf(stderr,"[TTK crystal slot] crystal %08x (type %u) shown in %08x (type %u) at %d %d %d room %d\n",
                     crystal,type,device,psx_mod_read_half(device+0x2c),p[0],p[1],p[2],dcell);
    }
}
