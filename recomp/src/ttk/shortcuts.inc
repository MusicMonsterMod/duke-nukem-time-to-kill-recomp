// Included in the guarded controls adapter. Slot/name proof: 33-controls-shortcuts.md.
static constexpr unsigned weapon_order[]={0,1,2,4,5,7,8,12,14,13,10,9,6,3,11};
static constexpr unsigned group_start[]={0,3,4,5,6,7,10,11,12,14,15};
static unsigned equipped=255,last_equipped=255,selected_item=5;
static uint64_t command_seen;
static bool guest_busy;
static unsigned base_weapon(unsigned slot) {return slot==27?9:slot==28?7:slot==29?8:slot;}
static bool usable_weapon(unsigned slot) {
    if(slot>14)return false;
    unsigned inventory=psx_mod_read_half(player+0x2c4+4*slot);
    if(!(inventory&1) || !(psx_mod_read_half(0x800c4590+44*slot)&1))return false;
    if(slot==0)return true;
    unsigned actual=slot;
    if(inventory&8)actual=slot==7?28:slot==8?29:slot==9?27:slot;
    return (int16_t)psx_mod_read_half(player+0x2c6+4*actual)>0 ||
        (slot==12 && (int32_t)psx_mod_read_word(player+0x254)>0);
}
// D08O1: in the swim states the original only completes a switch to weapons
// whose table flags carry 0x04 (3/4/5/6/7/12 and the 28 variant); it remaps
// others (1/2 -> 3, 8-11 -> 7, 13 -> 12) or leaves 14 pending, so underwater
// choices skip them.
static bool underwater_weapon(unsigned slot) {
    if(slot>14)return false;
    unsigned actual=slot;
    if(psx_mod_read_half(player+0x2c4+4*slot)&8)actual=slot==7?28:slot==8?29:slot==9?27:slot;
    return psx_mod_read_byte(0x800c4590+44*actual)&4;
}
// Original callable item path, on an isolated CPU/stack, never an input/fire macro.
static bool original_call(CPUState* source,uint32_t address,uint32_t a1,int32_t* result=nullptr) {
    uint32_t top=source->gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail)return false;
    uint32_t saved[1024],scratch[256];
    // Word copies: every guest access goes through the runtime's checked
    // path, so bytewise copies of these 5 KiB dominated a call's cost.
    const uint32_t base=(top-4096)&~3u;
    for(unsigned i=0;i<1024;++i)saved[i]=psx_mod_read_word(base+4*i);
    for(unsigned i=0;i<256;++i)scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=0x800000fc;
    cpu.gpr[4]=player;cpu.gpr[5]=a1;
    guest_busy=true;psx_dispatch_call(&cpu,address,0x800000fc);guest_busy=false;
    // Restore only what the call changed: guest writes are the costly part
    // (the runtime traces every store), and most of the 5 KiB is untouched.
    for(unsigned i=0;i<1024;++i)if(psx_mod_read_word(base+4*i)!=saved[i])psx_mod_write_word(base+4*i,saved[i]);
    for(unsigned i=0;i<256;++i)if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i])psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    if(result)*result=(int32_t)cpu.gpr[2];
    return cpu.pc==0 && !g_psx_call_bail;
}
static bool usable_item(unsigned item) {
    if(item<1 || item>5 || item==4)return false; // steroids activate on pickup
    if(psx_mod_read_half(0x800c2710+4*item)!=(item==1?1:item==5?2:3))return false;
    unsigned v=psx_mod_read_half(player+0x354+item*4);
    return (v&1) && !(v&0x8000) && ((v&2) || (int16_t)psx_mod_read_half(player+0x356+item*4)>0);
}
// Share the original menu's remembered ID. Its list rebuild maps this ID to
// the cursor at885e0; browsing writes it at893c8. Never rewrite its list/cursor.
static void inventory_feedback(bool announce) {
    uint16_t flags[6]{},capacities[6]{};int16_t amounts[6]{};
    for(unsigned i:{5u,1u,2u,3u}) {
        capacities[i]=psx_mod_read_half(0x800c2712+4*i);
        flags[i]=psx_mod_read_half(player+0x354+4*i);
        amounts[i]=(int16_t)psx_mod_read_half(player+0x356+4*i);
    }
    inventory_update(selected_item,flags,amounts,capacities,announce);
}
static void remember_item(unsigned item) {
    selected_item=item;
    if(psx_mod_read_half(0x800c3f94)!=item)psx_mod_write_half(0x800c3f94,item);
}
static void use_item(CPUState* cpu,unsigned item) {
    if(item==5) {
        // M: report charge only — never open the bracket switcher (EDuke32-style).
        unsigned owned=psx_mod_read_half(player+0x354+4*5);
        if(!(owned&1))return; // no medkit: silent, like EDuke32
        unsigned cap=psx_mod_read_half(0x800c2712+4*5);
        int16_t raw=(int16_t)psx_mod_read_half(player+0x356+4*5);
        unsigned charge=raw>0?(unsigned)raw:0;
        unsigned pct=cap?charge*100/cap:charge;
        char text[32];std::snprintf(text,sizeof text,"MEDKIT %u%%",pct);
        input_notice(text);
        if(!usable_item(5))return;
        remember_item(5);
    } else {
        if(!usable_item(item))return;
        remember_item(item);inventory_feedback(false);
    }
    // Same requests as inventory confirmation at 89474 / 89494 / 894cc.
    if(psx_mod_read_half(0x800c2712+4*item)==0) {
        psx_mod_write_byte(player+0x3bb,item);
        psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)|0xc);
        return;
    }
    // Jetpack cannot be switched in water; the normal-state lease excludes it.
    if(item==1 && psx_mod_read_word(player+0x834)!=0x40000000 &&
       (int32_t)psx_mod_read_word(player+8)>(int32_t)psx_mod_read_word(player+0x834))return;
    // Do not mutate a request unless nested execution is available.
    if(guest_busy || cpu->gpr[29]<0x801f4000 || cpu->gpr[29]>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail)return;
    for(unsigned i=1;i<=5;++i)if(psx_mod_read_half(player+0x354+4*i)&0x8000)return;
    auto address=player+0x354+4*item;
    unsigned value=psx_mod_read_half(address);
    if(psx_mod_read_half(0x800c2710+4*item)&1)value^=2;
    psx_mod_write_half(address,value|0x8000);
    original_call(cpu,0x800402d0,item);
    if(item==1 || item==2 || item==3) {
        const bool on=!!(psx_mod_read_half(player+0x354+4*item)&2);
        if(item==1)input_notice(on?"JETPACK ON":"JETPACK OFF");
        else if(item==2)input_notice(on?"BIO MASK ON":"BIO MASK OFF");
        else input_notice(on?"NIGHT VISION ON":"NIGHT VISION OFF");
    }
}
static void select_weapon(CPUState* cpu) {
    const auto& f=input_snapshot(Context::Gameplay);
    input_ack_commands(f.command_serial);
    if(!interaction_alive()) {equipped=last_equipped=255;command_seen=f.command_serial;return;}
    if(!f.active || !gameplay_context())return;
    // Only primary single-player ownership; no writes into player two's menu.
    if(psx_mod_read_byte(player+0x233)!=0)return;
    // D08Q2: Continue can reset the jetpack model/transition while retaining
    // its pending item bit. The original completion at 8004010c clears that
    // bit only while the open/close updater runs. Recover its closed, off
    // endpoint only; never finish an active transition or grant fuel/ownership.
    const unsigned jet=psx_mod_read_half(player+0x358);
    if((jet&0x8003)==0x8001 && movement_ready() &&
       (int16_t)psx_mod_read_half(player+0x32)>0 &&
       psx_mod_read_byte(player+0x22c)==0 && psx_mod_read_byte(player+0x22d)==0 &&
       !(psx_mod_read_word(player+0x224)&0x1000100c) &&
       psx_mod_read_half(player+0x84c)==0 && psx_mod_read_word(player+0x884)==1)
        psx_mod_write_half(player+0x358,jet&~0x8000u);
    unsigned menu_item=psx_mod_read_half(0x800c3f94);
    bool changed=false;
    if(menu_item!=selected_item && usable_item(menu_item)) {selected_item=menu_item;changed=true;}
    // Pending original activation is still visible; depletion chooses the next
    // eligible gadget and leaves U harmless when the inventory is empty.
    unsigned selected_flags=selected_item<=5?psx_mod_read_half(player+0x354+4*selected_item):0;
    if(!usable_item(selected_item) && !(selected_flags&0x8000)) {
        unsigned replacement=0;
        for(unsigned i:{5u,1u,2u,3u})if(usable_item(i)){replacement=i;break;}
        if(selected_item!=replacement){remember_item(replacement);changed=true;}
    }
    inventory_feedback(changed);
    unsigned equipment=psx_mod_read_byte(player+0x3b8),flags=psx_mod_read_word(player+0x224);
    unsigned actual=psx_mod_read_byte(player+0x3b9),current=base_weapon(actual);
    bool remote=equipment==1 && actual==0 && psx_mod_read_byte(player+0x3ba)==12 &&
        (int32_t)psx_mod_read_word(player+0x254)>0;
    if(remote)current=12;
    unsigned upper=psx_mod_read_half(player+0x74);
    bool weapon_idle=upper==5 || upper==20 || upper==29 || upper==35 || upper==39;
    if(equipment==0 && psx_mod_read_byte(player+0x3ba)==0)current=0;
    // Original boot selection holsters and retains the previous drawn weapon byte.
    // Observe successful original equipment changes, including inventory selection.
    if(!(flags&4) && (((equipment==2 || remote) && current<=14 && weapon_idle) ||
        (equipment==0 && psx_mod_read_byte(player+0x3ba)==0))) {
        if(equipped!=current) {last_equipped=equipped;equipped=current;}
    }
    if(!f.command_serial || command_seen==f.command_serial)return;
    command_seen=f.command_serial;
    // D12A: in the eye view Q is the quick kick (kick.inc), with any weapon,
    // moving or in the air; it never touches the original equipment or animation.
    bool eye_kick=false;
    for(unsigned n=0;n<f.command_count && n<32;++n) if(f.commands[n]==quick_kick && kick_request(true)) eye_kick=true;
    // D08Q: J in flight is the original inventory toggle (358 bit1 → 800402d0
    // → 8003ff6c closing), which the mode-10 handler turns into the 108 fall.
    // D08R: jetpack_input_ready() covers both schemes.
    // D08Q5: mode 10 completes the same weapon request (+0x3ba, flag 4) with
    // the normal upper-body holster/draw while the flight pose and handler
    // continue, so weapon choices are taken in flight too; other items and the
    // quick kick keep their ground lease.
    const bool jet_select=jetpack_input_ready();
    if(jet_select) {
        for(unsigned n=0;n<f.command_count && n<32;++n)
            if(f.commands[n]==jetpack || (f.commands[n]==item_use && selected_item==1)) {use_item(cpu,1);return;}
    }
    // D08O1: the original swim states switch weapons through the same request
    // (+0x3ba, flag 4) as the ground; there only weapon choices are taken,
    // items and the quick kick keep their ground lease.
    const bool swim_select=!jet_select && !movement_ready() && swim_weapon_ready();
    const bool weapons_only=swim_select || jet_select;
    if(!(movement_ready() || weapons_only) || (equipment!=0 && equipment!=2 && !remote) || f.held[original_aim] ||
       f.held[fire] || f.held[interact] || (flags&0x2c5))return;
    // Stable original upper-body idle loops. Draw, attack and charged-throw
    // animations retain ownership (4e840 dispatch / 50344 completion tables).
    if((equipment==2 || remote) && !weapon_idle)return;
    unsigned cursor=equipped,requested=255;
    for(unsigned n=0;n<f.command_count && n<32;++n) {
        int action=f.commands[n];
        if(weapons_only && !(action>=weapon_group_1 && action<=weapon_group_0) &&
           action!=weapon_previous && action!=weapon_next && action!=weapon_last) continue;
        if(action>=weapon_group_1 && action<=weapon_group_0) {
            unsigned group=action-weapon_group_1,begin=group_start[group],end=group_start[group+1];
            unsigned start=begin;
            for(unsigned i=begin;i<end;++i)if(weapon_order[i]==cursor)start=i+1==end?begin:i+1;
            for(unsigned j=0;j<end-begin;++j) {
                unsigned candidate=weapon_order[begin+(start-begin+j)%(end-begin)];
                if(usable_weapon(candidate) && (!swim_select || underwater_weapon(candidate))){requested=cursor=candidate;break;}
            }
        } else if(action==weapon_previous || action==weapon_next) {
            unsigned start=0;for(unsigned i=0;i<15;++i)if(weapon_order[i]==cursor)start=i;
            for(unsigned j=1;j<=15;++j) {
                unsigned candidate=weapon_order[(start+(action==weapon_next?j:15-j))%15];
                if(usable_weapon(candidate) && (!swim_select || underwater_weapon(candidate))){requested=cursor=candidate;break;}
            }
        } else if(action==weapon_last) {
            if(usable_weapon(last_equipped) && (!swim_select || underwater_weapon(last_equipped)))requested=cursor=last_equipped;
        } else if(action==item_previous || action==item_next) {
            constexpr unsigned items[]={5,1,2,3};unsigned start=0;
            for(unsigned i=0;i<4;++i)if(items[i]==selected_item)start=i;
            for(unsigned j=1;j<=4;++j){unsigned next=items[(start+(action==item_next?j:4-j))%4];
                if(usable_item(next)){remember_item(next);break;}}
            inventory_feedback(true);
        } else if(action==quick_kick) {
            if(eye_kick)break;
            // 517a4 checks equipment/selected bytes with mask 00ff00ff; the
            // stale drawn-weapon byte is intentionally irrelevant. Never disarm.
            if(requested==255 && equipment==0 && psx_mod_read_byte(player+0x3ba)==0 &&
               psx_mod_read_half(player+0x60)==63 && upper==63 && !f.move_x && !f.move_y && !f.held[jump])
                original_call(cpu,0x800517a4,0);
            break;
        } else {
            unsigned item=action==medkit?5:action==jetpack?1:action==biomask?2:action==goggles?3:action==item_use?selected_item:0;
            if(item) {use_item(cpu,item);break;} // one item activation per guest update
            // R: original steroids are pickup-activated.
        }
    }
    if(requested<=14 && !(psx_mod_read_word(player+0x224)&0xc) &&
       (requested!=equipped || equipment==0 && requested!=0)) {
        psx_mod_write_byte(player+0x3ba,requested);
        psx_mod_write_word(player+0x224,psx_mod_read_word(player+0x224)|4);++selections;
    }
}
