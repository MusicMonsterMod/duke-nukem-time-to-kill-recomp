// D60: a lock that wants a FAMILY JEWEL takes any jewel Duke holds, in any
// order (Modernized). Included in modern_controls.cpp after door_use.inc,
// before jewel_colours.inc. Note 516.
//
// The original (level 6's nuke-symbol wheel, read from the LEVEL06 overlay,
// handler 0x800cc338, kind 7): the lock counts the jewels set in at
// object+0x35 (0, 1, 2) and always wants item 14 + count. Mode 2 answers
// 14, 15 or 16; mode 0 accepts only that item's found bit (player+0x38c,
// 0x390, 0x394 bit 0) or that item already in Duke's hand (+0x3b8 = 1, +0x3b9
// = item); mode 3 switches the wheel to its model with count + 1 jewels,
// plays the sound and counts up, and the third completes the device (objects
// 20 and 21). So a jewel found out of order, or the third jewel when an
// earlier one is missing, is refused: the game's own rule.
//
// Here the game still makes the whole use with the item it wants (W): when
// Duke presses action at such a lock without W but with another found jewel
// J, J's flag is lent to W for that use. The low halfwords of W's and J's
// flags trade places and J records the lend (bits 0x0c00 = W - 13), so the
// lock sees W in Duke's inventory and the original press, line-up,
// animation 47, the push and mode 3 all run unchanged. Once W is used up
// (bit 0 cleared as Duke takes it in hand) the flags trade back: J is the
// used jewel, W keeps what it had, and the hand (+0x3b9, +0x3bb) holds J, so
// a use given back returns J. A press the lock refuses, or a use that stops
// before animation 47, trades back unchanged.
//
// Each used jewel records the order it went in (bits 0x3000 = 1..3), which
// the wheel's sockets follow (jewel_colours.inc). Used jewels from older
// saves take their order in item order, as the game set them. Everything
// lives in Duke's own flag words (guest RAM), so a savestate taken at any
// moment, a lend included, carries on. The bits are free in the mission
// flags: the game tests bit 0 (found) and the host's D08A19 marker 0x4000.
// Vanilla runs none of this (hook() and select_weapon are Modernized only).
// The flag rules are in jewel_device_rules.h.
namespace jr=jewel_rules;
static unsigned jewel_lends,jewel_returns,jewel_undos,jewel_places;
static int jewel_preferred=-1;         // D08A18 Enter: the browsed jewel
static uint64_t jewel_preferred_update,jewel_updates;
static uint32_t jewel_preferred_loads;
static uint32_t jewel_flag_address(unsigned item) {return player+0x354+4*item;}
static void jewel_read(uint16_t* f) {for(unsigned i=0;i<jr::items;++i)f[i]=psx_mod_read_half(jewel_flag_address(jr::first_item+i));}
static void jewel_write(const uint16_t* f) {
    for(unsigned i=0;i<jr::items;++i) {
        const uint32_t a=jewel_flag_address(jr::first_item+i);
        if(psx_mod_read_half(a)!=f[i])psx_mod_write_half(a,f[i]);
    }
}
// For jewel_colours.inc: the jewel lent to item w (-1), a used jewel's place.
static int jewel_lent_from(unsigned w) {uint16_t f[jr::items];jewel_read(f);return jr::lent_from(f,w);}
static unsigned jewel_order(unsigned item) {return jr::order(psx_mod_read_half(jewel_flag_address(item)));}
// The jewel item lock in front of Duke (after the game's finder) and the
// item it wants, or false.
static bool jewel_lock_wants(CPUState* cpu,uint32_t object,int32_t& wanted) {
    if(object<0x80010000u || object>=0x80200000u || (object&3))return false;
    const uint32_t table=psx_mod_read_word(0x800d2660),handlers=psx_mod_read_word(0x800dd84c);
    if(table<0x80010000u || table>=0x80200000u || handlers<0x80010000u || handlers>=0x80200000u)return false;
    const uint32_t type_flags=psx_mod_read_word(table+28*psx_mod_read_half(object+0x2c));
    if(!(type_flags&0x01000000) || (type_flags&0x08c00000))return false;   // the item path of 51890
    const uint32_t handler=psx_mod_read_word(handlers+4*psx_mod_read_byte(object+0x19));
    if(handler<0x80010000u || handler>=0x80200000u)return false;
    wanted=-1;
    return original_call(cpu,handler,object,&wanted,2) && jr::is_jewel(wanted);
}
// D08A18 Enter on a browsed jewel at a lock that wants a jewel: can the lock
// take it now? Lent for the lock's own mode 0 only, then traded straight back
// (no game frame runs in between). A yes keeps the jewel preferred for the E
// use Enter starts.
static bool jewel_mission_usable(CPUState* cpu,uint32_t handler,uint32_t object,int wanted,int item) {
    uint16_t f[jr::items];jewel_read(f);
    if(!jr::is_jewel(wanted) || !jr::is_jewel(item) || !(f[item-jr::first_item]&jr::found_bit) || jr::lend_active(f))return false;
    int32_t blocked=1;
    if(item!=wanted) {uint16_t g[jr::items];std::copy(f,f+jr::items,g);jr::trade(g,(unsigned)wanted,(unsigned)item,true);jewel_write(g);}
    const bool ok=original_call(cpu,handler,object,&blocked,0);
    jewel_write(f);
    if(!ok || blocked)return false;
    jewel_preferred=item;jewel_preferred_update=jewel_updates;jewel_preferred_loads=psx_mod_savestate_loads();
    return true;
}
// 0x80051890 entry (the idle Action) on a fresh Cross: lend a jewel when the
// lock in front wants one Duke does not hold. The finder's contact is put
// back; the press then runs its own finder and checks.
static void jewel_device_action(CPUState* cpu) {
    if(cpu->gpr[4]!=player || !player_identity_ready() || !gameplay_context() ||
       psx_mod_read_byte(player+0x27d) || (psx_mod_read_word(player+0x224)&0x241))return;
    const uint32_t word=door_action_word();
    if(!word || (psx_mod_read_word(word)&3)!=1)return;
    uint16_t f[jr::items];jewel_read(f);
    if(jr::lend_active(f) || jr::choice(f,jr::first_item,-1)<0)return;   // no jewel found
    const uint32_t contact=psx_mod_read_word(player+0x174);
    const uint16_t contact_side=psx_mod_read_half(player+0x178);
    int32_t wanted=-1;
    const bool lock=original_call(cpu,0x80077f28,0) && jewel_lock_wants(cpu,psx_mod_read_word(player+0x174),wanted);
    psx_mod_write_word(player+0x174,contact);psx_mod_write_half(player+0x178,contact_side);
    if(!lock)return;
    const bool fresh=jewel_preferred_loads==psx_mod_savestate_loads() && jewel_updates-jewel_preferred_update<180;
    const int j=jr::choice(f,(unsigned)wanted,fresh?jewel_preferred:-1);
    jewel_preferred=-1;
    if(j<0 || j==wanted)return;
    jr::trade(f,(unsigned)wanted,(unsigned)j,true);jewel_write(f);++jewel_lends;
    if(std::getenv("DNTTK_JEWEL_TRACE"))
        std::fprintf(stderr,"[TTK jewel] lock wants %d, jewel %d lent\n",(int)wanted,j);
}
// Once per player update (select_weapon, after D08A19's marker): trade a
// lend back once the lock's item is used up, or undo it when the use did not
// start; give each newly used jewel its place.
static void jewel_device_update() {
    if(!player_identity_ready())return;
    ++jewel_updates;
    uint16_t f[jr::items];jewel_read(f);
    uint8_t held=psx_mod_read_byte(player+0x3b9),request=psx_mod_read_byte(player+0x3bb);
    const uint8_t held0=held,request0=request;
    const unsigned events=jr::update(f,psx_mod_read_byte(player+0x27d)!=0,held,request);
    if(!events)return;
    jewel_write(f);
    if(held!=held0)psx_mod_write_byte(player+0x3b9,held);
    if(request!=request0)psx_mod_write_byte(player+0x3bb,request);
    if(events&jr::returned)++jewel_returns;
    if(events&jr::undone)++jewel_undos;
    if(events&jr::placed)++jewel_places;
    if(std::getenv("DNTTK_JEWEL_TRACE"))
        std::fprintf(stderr,"[TTK jewel] update %s%s%s: flags %04x %04x %04x\n",events&jr::returned?"returned ":"",
                     events&jr::undone?"undone ":"",events&jr::placed?"placed":"",f[0],f[1],f[2]);
}
// The mission inventory shows the jewels as they are, a lend in progress
// included.
static void jewel_mission_view(uint16_t* mission) {jr::mission_view(mission);}
