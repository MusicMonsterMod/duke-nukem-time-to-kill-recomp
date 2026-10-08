// D08Z3 modern landing (DNTTK_LANDING=modern, the Modernized default; the
// launcher passes original for Vanilla). documentation/136-d08z3-modern-landing.md.
//
// Original: the airborne handler lands through the selector 0x80054c04. It
// continues the gait (78/84/85/76, +0x228 bit 3) only after a directional or
// running flight (+0x23e 98/103/104/108) with a direction held at that very
// update, starts the next jump (96) with jump held, and otherwise plays the
// recovery 105. 105 (and the hard landing 106) run the landing-pose handler
// 0x80052fb8 from the ground dispatcher, which takes no input until the
// animation ends (105: about 37 fields at 60 Hz). A straight-up jump steered
// in the air, or a jump whose direction was let go before the touch-down,
// stands Duke still for about 0.6 s, in Vanilla too.
//
// Modern: 105 stays the landing pose while nothing is pressed. A held
// direction starts the walk or run from it at the ground dispatcher, in the
// same update, as the gait switch there does for 70/71 (frame 0, original
// handler, collision and stride); a jump press lets the next jump start.
// Takeoffs, flights, the landing choice and sound, fall damage, the hard
// landing 106 and water entries stay original.
static bool landing_modern() {
    const char* v=std::getenv("DNTTK_LANDING");
    return !v || std::strcmp(v,"original");
}
static unsigned landing_recoveries;
// Ground dispatcher 0x80048410 from the player update (ra 0x8004b634), before
// it runs this update's handler for +0x60.
static bool landing_recover(CPUState* cpu,uint32_t ra,const InputFrame& f) {
    if(cpu->gpr[4]!=player || ra!=0x8004b634 || !landing_modern() || psx_mod_read_half(player+0x60)!=105 ||
       psx_mod_read_byte(player+0x22c)!=0 || psx_mod_read_byte(player+0x22d)!=0)return false;
    const bool move=f.move_x || f.move_y;
    if(!f.active || (!move && !input_jump_pending()) || f.held[crouch] || f.held[original_aim] ||
       (psx_mod_read_word(player)&0x20002) || (psx_mod_read_word(player+0x224)&0x202002c1) ||
       psx_mod_read_word(camera+0xa4)!=player || psx_mod_read_word(player+0x7d4)!=camera ||
       shallow_land_water_early() || !player_identity_ready())return false;
    // A jump pressed on landing with no direction: the stance 63, where the
    // host's buffered press reaches the original standing jump (the buffer
    // only feeds the pad in a settled gait, and lapses within 105).
    const unsigned next=!move?63u:input_running()?76u:72u;
    if(std::getenv("DNTTK_TRAVERSAL_TRACE"))std::fprintf(stderr,"ttk-landing recover time=%u -> %u move=%.2f,%.2f jump=%d\n",
        (unsigned)psx_mod_read_half(player+0x6a),next,f.move_x,f.move_y,input_jump_pending()?1:0);
    psx_mod_write_half(player+0x60,next);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
    ++landing_recoveries;
    return true;
}
static const char* landing_debug_json() {
    static char buffer[96];
    std::snprintf(buffer,sizeof buffer,"{\"modern\":%s,\"recoveries\":%u}",landing_modern()?"true":"false",landing_recoveries);
    return buffer;
}
