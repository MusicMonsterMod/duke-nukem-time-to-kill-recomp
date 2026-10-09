// Typed debugging conveniences. Original TTK grant/toggle contracts are decoded
// in documentation/38-debug-cheats.md; the active controls identity gates calls.
struct HiddenEnemy {uint32_t row,type;int16_t health;bool active;};
static std::vector<HiddenEnemy> hidden_enemies;
static uint32_t hidden_table,hidden_count;
static bool monsters_hidden;
static bool cheat_ram(uint32_t a,unsigned n){return !(a&3) && a>=0x80010000 && a<=0x80200000-n;}
// Calls an original function on a copy of the CPU with a private stack and
// scratchpad. D26E: optional a1, a buffer copied into the saved stack area and
// passed as a2, and the v0 result.
static bool cheat_call(CPUState* cpu,uint32_t address,uint32_t actor,uint32_t a1=0,
                       const uint32_t* buffer=nullptr,unsigned words=0,uint32_t* result=nullptr) {
    CPUState copy=*cpu; // original_call normally supplies the player as a0.
    const uint32_t top=copy.gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail)return false;
    uint8_t stack[4096],scratch[1024];
    for(unsigned i=0;i<sizeof stack;++i)stack[i]=psx_mod_read_byte(top-4096+i);
    for(unsigned i=0;i<sizeof scratch;++i)scratch[i]=psx_mod_read_byte(0x1f800000+i);
    copy.pc=0;copy.gpr[29]=top-256;copy.gpr[31]=0x800000fc;copy.gpr[4]=actor;
    if(buffer && words && words<=32) {
        copy.gpr[5]=a1;
        const uint32_t at=top-192;
        for(unsigned i=0;i<words;++i)psx_mod_write_word(at+4*i,buffer[i]);
        copy.gpr[6]=at;copy.gpr[7]=0;
    }
    guest_busy=true;psx_dispatch_call(&copy,address,0x800000fc);guest_busy=false;
    if(result)*result=copy.gpr[2];
    for(unsigned i=0;i<sizeof stack;++i)psx_mod_write_byte(top-4096+i,stack[i]);
    for(unsigned i=0;i<sizeof scratch;++i)psx_mod_write_byte(0x1f800000+i,scratch[i]);
    return copy.pc==0 && !g_psx_call_bail;
}
static bool toggle_monsters(CPUState* cpu) {
    uint32_t table=psx_mod_read_word(0x800de720),count=psx_mod_read_word(0x800c56a0);
    uint32_t types=psx_mod_read_word(0x800d2660);
    if(!count || count>512 || !cheat_ram(table,count*48) || !cheat_ram(types,28*1024))return false;
    if(monsters_hidden && (table!=hidden_table || count!=hidden_count)) {
        hidden_enemies.clear();monsters_hidden=false;
    }
    if(!monsters_hidden) {
        // Validate the complete selection before mutating it. Handler6 is the
        // original hostile AI class; handler8 NPCs and other classes stay intact.
        std::vector<HiddenEnemy> candidates;
        for(unsigned i=0;i<count;++i) {
            uint32_t row=table+48*i,type=psx_mod_read_word(row),flags=psx_mod_read_word(row+20);
            if(type>=1024)return false;
            if(psx_mod_read_byte(types+type*28+6)!=6 || (flags&0x16))continue;
            uint32_t actor=psx_mod_read_word(row+4);
            if(flags&1) {
                if(!cheat_ram(actor,0x2cc) || psx_mod_read_half(actor+0x2c)!=type ||
                   psx_mod_read_byte(actor+0x15)!=6)return false;
                if((psx_mod_read_word(actor)&2) || (int16_t)psx_mod_read_half(actor+0x32)<=0)continue;
            }
            candidates.push_back({row,type,(int16_t)((flags&1)?psx_mod_read_half(actor+0x32):0),bool(flags&1)});
        }
        hidden_table=table;hidden_count=count;
        for(auto entry:candidates) {
            if(entry.active) {
                uint32_t actor=psx_mod_read_word(entry.row+4),pos[3];
                for(unsigned j=0;j<3;++j)pos[j]=psx_mod_read_word(actor+4+4*j);
                if(!cheat_call(cpu,0x80098d70,entry.row))return false;
                for(unsigned j=0;j<3;++j)psx_mod_write_word(entry.row+8+4*j,pos[j]);
            }
            psx_mod_write_word(entry.row+20,psx_mod_read_word(entry.row+20)|16);
            hidden_enemies.push_back(entry);
        }
        monsters_hidden=true;input_notice("Monsters: Off");
    } else {
        for(auto entry:hidden_enemies) {
            if(psx_mod_read_word(entry.row)!=entry.type || psx_mod_read_word(entry.row+4))continue;
            psx_mod_write_word(entry.row+20,psx_mod_read_word(entry.row+20)&~16u);
            if(entry.active && !(psx_mod_read_word(entry.row+20)&6) && cheat_call(cpu,0x80098f3c,entry.row)) {
                auto actor=psx_mod_read_word(entry.row+4);
                if(cheat_ram(actor,0x2cc) && psx_mod_read_half(actor+0x2c)==entry.type)
                    psx_mod_write_half(actor+0x32,entry.health);
            }
        }
        hidden_enemies.clear();monsters_hidden=false;input_notice("Monsters: On");
    }
    return true;
}
// D08Q4: Duke3D god mode also keeps health and the jetpack full. Health rises
// to the player's own maximum (object type +0xc, the cap 80040498/80096cac
// use) but is never lowered, so Atomic Health keeps its original 2x ceiling.
// User request (2026-10-06): god mode also gives the jetpack, the way the
// original inventory cheat 8003d738 grants item 1 (owned bit only), and keeps
// it while on (god mode survives level travel and savestates that lose the
// pack); it is topped up to the live item capacity at 800c2716.
// Runs at the 8005a210 step-table entry and again from the 80058120 poll,
// after the mode-10 handler's drain, so the HUD never shows 99%.
static void god_mode_update() {
    if(!psx_mod_read_half(0x800c3cc6) || !player_identity_ready() || psx_mod_read_word(0x800c27bc)!=1)return;
    const uint32_t types=psx_mod_read_word(0x800d2660),type=psx_mod_read_half(player+0x2c);
    if(type>=1024 || !cheat_ram(types,28*1024))return;
    const int16_t most=(int16_t)psx_mod_read_half(types+28*type+0xc),health=(int16_t)psx_mod_read_half(player+0x32);
    if(health>0 && most>0 && health<most)psx_mod_write_half(player+0x32,(uint16_t)most);
    const int16_t fuel=(int16_t)psx_mod_read_half(0x800c2716);
    if(fuel>0 && !(psx_mod_read_half(player+0x358)&1))psx_mod_write_half(player+0x358,psx_mod_read_half(player+0x358)|1);
    if((psx_mod_read_half(player+0x358)&1) && fuel>0 && (int16_t)psx_mod_read_half(player+0x35a)<fuel)
        psx_mod_write_half(player+0x35a,(uint16_t)fuel);
}
// D08G3: the original upgrade bit (8003df40 resolves 7/8/9 to 28/29/27) on the
// six weapons the original upgrade mask +0x85f covers, bit i -> weapon
// 4,5,7,8,9,10 (8003f4d8, 8008328c). The mask is what bonus-level awards set
// and what saves, pickups and the restart loadout reapply, so upgrades persist
// and weapons not yet owned arrive upgraded. Nothing new is granted, but an
// owned 7/8/9 reads ammo from its resolved record, which is unowned and empty
// without dnstuff (the weapon then counts as empty and is skipped). Carry the
// base ammo over, capped at the resolved record's capacity (800c4594).
static void upgrade_weapons() {
    static constexpr unsigned upgradable[]={4,5,7,8,9,10};
    static constexpr unsigned resolved[][2]={{7,28},{8,29},{9,27}};
    for(unsigned id:upgradable)psx_mod_write_half(player+0x2c4+4*id,psx_mod_read_half(player+0x2c4+4*id)|8);
    for(auto [base,up]:resolved) {
        if(!(psx_mod_read_half(player+0x2c4+4*base)&1))continue;
        const int16_t most=(int16_t)psx_mod_read_half(0x800c4594+44*up),ammo=(int16_t)psx_mod_read_half(player+0x2c6+4*base);
        const int16_t carried=std::min(ammo,most);
        psx_mod_write_half(player+0x2c4+4*up,psx_mod_read_half(player+0x2c4+4*up)|1);
        if((int16_t)psx_mod_read_half(player+0x2c6+4*up)<carried)psx_mod_write_half(player+0x2c6+4*up,(uint16_t)carried);
    }
    psx_mod_write_byte(player+0x85f,psx_mod_read_byte(player+0x85f)|0x3f);
}
static void cheats_update(CPUState* cpu) {
    god_mode_update();
    auto command=input_take_cheat();
    if(command==Cheat::None)return;
    if(!input_snapshot(Context::Gameplay).active || !player_identity_ready() ||
       psx_mod_read_word(0x800c27bc)!=1 || !movement_ready() || guest_busy ||
       cpu->gpr[29]<0x801f4000 || g_precise_mode || g_ls_mode || g_psx_call_bail) {
        input_notice("Cheat unavailable here - stand on solid ground");return;
    }
    bool ok=true;
    switch(command) {
    case Cheat::Monsters: if(!toggle_monsters(cpu))input_notice("Enemy toggle unavailable");return;
    case Cheat::God:
        psx_mod_write_half(0x800c3cc6,!psx_mod_read_half(0x800c3cc6));
        god_mode_update();
        input_notice(psx_mod_read_half(0x800c3cc6)?"God Mode: On":"God Mode: Off");return;
    case Cheat::Upgrade: upgrade_weapons();input_notice("Weapons Upgraded");return;
    case Cheat::Tlod: input_notice("Play Zaxtor's Oblivion!");return;
    case Cheat::Unlimited:
        psx_mod_write_half(0x800c3cc4,!psx_mod_read_half(0x800c3cc4));
        input_notice(psx_mod_read_half(0x800c3cc4)?"Unlimited ammo ON":"Unlimited ammo OFF");return;
    case Cheat::Stuff: case Cheat::Weapons: ok=original_call(cpu,0x8003d7bc,0);break;
    default:break;
    }
    if(ok && (command==Cheat::Stuff || command==Cheat::Inventory || command==Cheat::Items)) {
        ok=original_call(cpu,0x8003d738,0);
        // D08G4: running steroids stop and a full dose is held; full armour,
        // as the original full armour pickup sets it (0x80082470: below 10000,
        // +0x234 = 10000; the status bar shows /100).
        if(ok) {
            steroids_cheat_grant();
            if((int16_t)psx_mod_read_half(player+0x234)<10000) psx_mod_write_half(player+0x234,10000);
        }
    }
    if(ok && (command==Cheat::Stuff || command==Cheat::Keys || command==Cheat::Items))ok=original_call(cpu,0x8003d840,0);
    if(command==Cheat::Ammo)for(unsigned i=0;i<36;++i)
        if(psx_mod_read_half(player+0x2c4+4*i)&1)psx_mod_write_half(player+0x2c6+4*i,psx_mod_read_half(0x800c4594+44*i));
    if(command==Cheat::Health)psx_mod_write_half(player+0x32,10000);
    if(command==Cheat::Hyper) {
        psx_mod_write_half(player+0x364,psx_mod_read_half(player+0x364)|2);
        psx_mod_write_half(player+0x366,psx_mod_read_half(0x800c2722));
    }
    const char* message=command==Cheat::Stuff?"Giving Everything!":command==Cheat::Weapons?"Got All Weapons/Ammo":
        command==Cheat::Inventory?"Got All Inventory":command==Cheat::Items?"Giving Everything!":command==Cheat::Keys?"Got All Keys":
        command==Cheat::Ammo?"Owned weapon ammo refilled":command==Cheat::Hyper?"Steroids":"Health restored";
    input_notice(ok?message:"Cheat could not complete");
}
