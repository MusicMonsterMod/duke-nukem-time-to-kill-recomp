// D27A: Shift-run is silent in Modernized. The player update's walk/run toggle
// (0x8004178c..0x800419f0, guarded) turns the run button event into
// player+0x224 bit 1 and clicks, sound 0x0001 through the non-positional call
// 0x8006bbd8(1), whenever the new state differs from the one it last stored
// at player+0x27b (read only there). Modern Shift holds the original run
// button, so every press and release clicked.
//
// At the entry of 0x800412a4 (Duke's update), the host works out the state the
// toggle is about to store, exactly as the original does, and stores it at
// +0x27b first: the original then sees no change and stays silent, and still
// sets +0x224 bit 1 itself, so running is untouched. The analog path and the
// original's own toggle mode (event 0, config & 3 == 1) are left alone. The
// D08A10 heartbeat makes its own 0x8006bbd8(1) call and is unaffected. Vanilla
// never gets here (player_identity_ready() needs Modernized).
// DNTTK_RUN_CLICK=original keeps the click (diagnostics).
static unsigned run_click_syncs,run_click_events;
static int run_click_last_event=-1;
static bool run_click_original() {
    static const bool keep=[]{const char* v=std::getenv("DNTTK_RUN_CLICK");return v && !std::strcmp(v,"original");}();
    return keep;
}
static void run_click_presync() {
    if(run_click_original() || !player_identity_ready())return;
    // 0x8004181c..0x80041850: the event is 2 (released) unless play is live.
    const unsigned index=psx_mod_read_byte(player+0x233);
    int event=2;
    if(psx_mod_read_half(0x800bda74)==0 && psx_mod_read_half(0x800bcbb0)!=0)
        event=(int)psx_mod_read_word(0x800bda8c+4*index+0x90);
    run_click_last_event=event;
    if(event!=1 && event!=2)return;
    ++run_click_events;
    // 1 (held) runs when config bit 0 is set, 2 (released) when it is clear.
    const uint32_t config=psx_mod_read_word(0x800d1cf0+4*index);
    if(config<0x80000000u || config>0x801ffffcu)return;
    const uint8_t want=((event==1)==((psx_mod_read_word(config)&1)!=0))?2:0;
    if(psx_mod_read_byte(player+0x27b)!=want) {psx_mod_write_byte(player+0x27b,want);++run_click_syncs;}
}
static const char* run_click_debug_json() {
    static char buffer[128];
    std::snprintf(buffer,sizeof buffer,"{\"silent\":%s,\"syncs\":%u,\"events\":%u,\"last_event\":%d}",
        run_click_original()?"false":"true",run_click_syncs,run_click_events,run_click_last_event);
    return buffer;
}
