// D08A9 picked-up gadget becomes the [ / ] selection (Modernized, Duke 3D feel:
// EDuke32's P_AddInventory sets inven_icon on every inventory pickup, refills
// included). Vanilla keeps the original selection.
//
// Every gadget pickup goes through the dispatcher 0x80081a48 (only caller
// 0x8007fe64). Its cases for the jetpack (0x80082580), Bio Mask (0x80082624),
// goggles (0x800826c4) and medkit (0x80082768) take the pickup only while the
// amount is below the item table capacity, then set bit 0 and the full amount;
// steroids (0x800827e8) are held by D08A4 at the pickup's sound call. Only in
// multiplayer (0x800c27bc >= 2) do the jetpack, Bio Mask and goggles also start
// their activation (+0x8000) there. Mission items and keys are other cases and
// never touch items 1-5.
//
// So the items are compared around the dispatcher call: before at its entry,
// after at the next call of its caller (0x8001ca4c, ra 0x8007fe78). A gadget
// that became owned or gained amount was picked up. The selection is the
// original menu's remembered ID 0x800c3f94 (remember_item), which the D08A6 HUD
// box and the D17 replay workers read from guest memory. While any gadget is
// mid-activation it waits for the next item poll that finds none.
// The original inventory grant (dninventory, dnstuff, dnitems) does not go
// through the dispatcher and keeps the current selection, as in EDuke32.
struct PickupSnapshot {bool pending;uint32_t loads;uint16_t flags[6];int16_t amounts[6];};
static PickupSnapshot pickup_before;
static unsigned pickup_selection,pickup_selects,pickup_deferred,pickup_refills;
static bool pickup_activation_pending() {
    for(unsigned i=1;i<=5;++i) if(psx_mod_read_half(player+0x354+4*i)&0x8000) return true;
    return false;
}
static void pickup_select_apply() {
    if(!pickup_selection) return;
    if(!interaction_alive() || psx_mod_read_byte(player+0x233)!=0) {pickup_selection=0;return;}
    if(pickup_activation_pending()) return;
    const unsigned item=pickup_selection;pickup_selection=0;
    if(!usable_item(item)) return;
    ++pickup_selects;
    if(item!=selected_item || psx_mod_read_half(0x800c3f94)!=item) {remember_item(item);inventory_feedback(false);}
}
// Dispatcher entry (a0 Duke from 0x8007fe64).
static void pickup_select_begin(CPUState* cpu) {
    pickup_before.pending=false;
    if(cpu->gpr[4]!=player || cpu->gpr[31]!=0x8007fe6c || !player_identity_ready() ||
       psx_mod_read_byte(player+0x233)!=0) return;
    pickup_before.pending=true;pickup_before.loads=psx_mod_savestate_loads();
    for(unsigned i=1;i<=5;++i) {
        pickup_before.flags[i]=psx_mod_read_half(player+0x354+4*i);
        pickup_before.amounts[i]=(int16_t)psx_mod_read_half(player+0x356+4*i);
    }
}
// After the dispatcher returns (0x8001ca4c from 0x8007fe70).
static void pickup_select_end() {
    if(!pickup_before.pending) return;
    pickup_before.pending=false;
    if(pickup_before.loads!=psx_mod_savestate_loads()) return;
    for(unsigned i:{1u,2u,3u,4u,5u}) {
        const unsigned flags=psx_mod_read_half(player+0x354+4*i);
        const int16_t amount=(int16_t)psx_mod_read_half(player+0x356+4*i);
        const bool gained=(flags&1) && !(pickup_before.flags[i]&1);
        if(!gained && !((flags&1) && amount>pickup_before.amounts[i])) continue;
        if(!gained) ++pickup_refills;
        pickup_selection=i;
        if(!pickup_activation_pending()) pickup_select_apply(); else ++pickup_deferred;
        break;
    }
}
static const char* pickup_select_debug_json() {
    static char buffer[128];
    std::snprintf(buffer,sizeof buffer,"{\"selects\":%u,\"refills\":%u,\"deferred\":%u,\"waiting\":%u}",
        pickup_selects,pickup_refills,pickup_deferred,pickup_selection);
    return buffer;
}
