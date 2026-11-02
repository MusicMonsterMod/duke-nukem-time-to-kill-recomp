// D26K: time in the backtick console is not player inactivity. While the
// console holds input (D26J: an active frame with no new input), the guest
// pad is neutral in run mode (Caps Lock or Shift as the speed modifier), so
// the native idle routine 0x80053038 counted player+0x240 down from 3000 by
// the frame step 0x800d21fc and, at zero, ran its armed expiry: the original
// holster 0x8004d4a4, then the fidgets 64-69. In walk mode the Walk bit keeps
// 0x8001e070 reporting activity and the timer at 3000.
// While the console holds, the countdown keeps the value it had when the
// console opened; a reset (activity, 3000) still applies and is kept from
// then on. Closing resumes the countdown from the kept value, so the original
// idle holster comes after the same remaining time as without the console.
// A holster or fidget already under way when the console opens finishes as
// the original's. Checked from the authenticated player update (0x8005a210
// from 0x80041b34), which runs every update, never in render replays.
static bool console_idle_held;
static uint64_t console_idle_epoch;
static unsigned console_idle_value, console_idle_keeps;
static void console_idle_hold() {
    const auto& f=input_snapshot(Context::Gameplay);
    if(!input_console_holding() || !player_identity_ready()) {console_idle_held=false;return;}
    const unsigned timer=psx_mod_read_half(player+0x240);
    if(console_idle_held && console_idle_epoch==f.epoch && timer<console_idle_value) {
        psx_mod_write_half(player+0x240,console_idle_value);++console_idle_keeps;return;
    }
    console_idle_held=true;console_idle_epoch=f.epoch;console_idle_value=timer;
}
