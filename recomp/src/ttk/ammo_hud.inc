// D08A32 ammo box icons (Modernized only). The status bar 0x8008ba30 draws the
// ammo box (element 1, layout hud_layout + 8) with one icon for every weapon:
// record 0x800c4514 + 16 * (3 - ammo % 4), a 16x13 cartridge belt at
// (x + 2, y + 2) that steps one round per shot (0x8008bb5c), then the 46x16
// box at (x, y). The weapon is player +0x3ba; its ammo is +0x2c6 + 4 * slot,
// or +0x336 / +0x33a / +0x332 for an upgraded Gatling / RPG / flamethrower
// (+0x2c4 + 4 * slot bit 3; 0x8008ba50..0x8008baf0).
//
// Here each weapon shows its own icon (assets/ui/ammo/, ammo_icons.h), drawn
// as the gadget icons are (hud_box_draw, bare: the 16x16 cell at x + 1, y,
// its cell and palette loaded in front of it through OT slot 0), and the four
// belt records get width 0 for the status bar's call (a degenerate quad, as
// hud_border_begin does to the box) until widescreen_hud_restore. The Gatling
// gun keeps the game's belt (user decision). One cell and palette in the HUD
// sheet's empty band (D08A7): the cell at (984,205) between the D08A20 and
// D19U2 cells, the palette at (1008,212) below the D08A20 one.
//
// Four icons animate (user designs), on the game clock 0x800d21fc (300 a
// second; 18 units = 60 ms), from the ammo count the box shows:
//  - Crossbow: three bolts at rest; a shot sends the top bolt off (frames 1-3
//    at 0 / 60 / 120 ms, rest at 200 ms); at three or fewer bolts the stack
//    shows what is left (4 two, 10 one, 13 blank) and its shot frames differ.
//  - Desert Eagle: the top of one magazine following the clip, player +0x231
//    (the high byte of the halfword +0x230: 5632 for the full 22-round clip; 0 at the clip's last
//    shot, refilled to the reserve at the end of the game's reload, about
//    1.06 s later; live probe). Rest 0-4 by rounds, a shot 5 + 3 * (n - 1)
//    from a clip of n (5 at most), then at 0 with a reserve the empty
//    magazine drops (20, 21) until the refill raises a loaded one (22, 23
//    when four or more go in).
//  - Energy weapon: a shot plays 1-7 at 60 ms then 0; shots during the cycle
//    keep it going, it ends on 0 at the end of the cycle.
//  - Freezer: a shot flashes 1 (peak), 2, 3 and rest at 230 ms; shots within
//    that keep pulsing 1, 2, 3, 2 at 70 ms, then 3 and rest.
// Live frames choose the cell; redraws (D17 replay presentation, workers
// with stale host state) take the live frame's choice from RenderState.
#include "ammo_icons.h"
static constexpr uint32_t ammo_belt=0x800c4514;
static constexpr unsigned ammo_belts=4;
static bool ammo_belts_hidden=false;
static int ammo_view=-1;            // k_ammo_cells index of this frame's icon, -1 the game's belt
static int override_ammo_view=-1;   // the live frame's choice, in a redraw
static unsigned ammo_draws,ammo_shots,ammo_refills,ammo_reloads;
struct AmmoAnim {
    int slot=-1,ammo=0,clip=0,before=0;
    int32_t t=0,shot=0,start=0,end=0,refill=0,reload=0;
    bool shot_valid=false,refill_valid=false,reloading=false,reload_track=false;
    uint32_t loads=0;
};
static AmmoAnim ammo_anim;
static int ammo_slot_ammo(unsigned slot) {
    return (int16_t)psx_mod_read_half(player+0x2c6+4*slot);
}
static int ammo_clip_rounds() {
    // The round count is the high byte; the low byte is not part of it (the
    // game writes +0x230 as a byte elsewhere), so never round it up.
    return psx_mod_read_byte(player+0x231);
}
// The animation frame of an animated weapon at the current clock.
static int ammo_frame(const AmmoAnim& a) {
    const int32_t dt=a.t-a.shot;
    switch(a.slot) {
    case 3: {  // crossbow
        const int n=a.ammo;
        const int rest=n>=3?0:n==2?4:n==1?10:13;
        if(!a.shot_valid || dt>=60) return rest;
        static const int8_t seq4[]={1,2,3},seq3[]={1,2,6},seq2[]={7,8,9},seq1[]={11,12};
        const int8_t* seq=a.before>=4?seq4:a.before==3?seq3:a.before==2?seq2:seq1;
        const int len=a.before>=2?3:2;
        return seq[std::min(dt/18,(int32_t)len-1)];
    }
    case 4: {  // Desert Eagle
        const int c=a.clip,rest=std::min(c,4);
        const bool shot_last=a.shot_valid && (!a.refill_valid || a.shot>=a.refill);
        if(shot_last && dt<48) return 5+3*(std::clamp(a.before,1,5)-1)+std::min(dt/18,(int32_t)2);
        if(!shot_last && a.refill_valid && !a.reloading) {
            const int32_t dr=a.t-a.refill;
            if(dr<45) return c>=4?22:rest;
            if(dr<90) return c>=4?23:rest;
            return rest;
        }
        // The game's reload (upper track 17), also before the clip is empty
        // (it reloads a part clip by itself): the magazine drops once the
        // last shot's frames are done, and stays gone until the refill.
        if(a.reloading) {
            const int32_t from=std::max(a.reload,a.shot_valid?a.shot+48:a.reload);
            return a.t<from ? rest : a.t-from<45 ? 20 : 21;
        }
        // An empty clip with a reserve: the reload is coming; the magazine drops.
        if(c==0 && a.ammo>0) return !a.shot_valid || dt>=105 ? 21 : dt>=60 ? 20 : 0;
        return rest;
    }
    case 10:  // energy weapon
        if(!a.shot_valid || a.t>=a.end) return 0;
        {const int i=(int)(((a.t-a.start)/18)%8);return i<7?i+1:0;}
    case 11:  // freezer
        if(!a.shot_valid || dt>=69) return 0;
        if(dt<42) {static const int8_t loop[]={1,2,3,2};return loop[((a.t-a.start)/21)%4];}
        return 3;
    }
    return 0;
}
// Live frame: advance the clock, take shots and refills from the counts.
static int ammo_live_view() {
    if(!input_modernized() || !identity()) return -1;
    const unsigned slot=psx_mod_read_byte(player+0x3ba);
    if(slot>=sizeof k_ammo_first/sizeof k_ammo_first[0] || k_ammo_first[slot]<0) {ammo_anim.slot=-1;return -1;}
    AmmoAnim& a=ammo_anim;
    const int ammo=ammo_slot_ammo(slot),clip=slot==4?ammo_clip_rounds():0;
    if((int)slot!=a.slot || a.loads!=psx_mod_savestate_loads()) {
        a=AmmoAnim{};a.slot=(int)slot;a.ammo=ammo;a.clip=clip;a.loads=psx_mod_savestate_loads();
    } else {
        a.t+=(int32_t)std::min(30u,psx_mod_read_word(0x800d21fc));
        const int count=slot==4?clip:ammo,last=slot==4?a.clip:a.ammo;
        if(count<last) {
            if(slot==10) {
                if(a.shot_valid && a.t<a.end) a.end=a.start+144*((a.t-a.start)/144+1);
                else {a.start=a.t;a.end=a.t+144;}
            } else if(slot==11 && !(a.shot_valid && a.t-a.shot<69)) a.start=a.t;
            a.before=last;a.shot=a.t;a.shot_valid=true;++ammo_shots;
        } else if(slot==4 && count>last) {a.refill=a.t;a.refill_valid=true;a.reloading=false;++ammo_refills;}
        a.ammo=ammo;a.clip=clip;
        if(slot==4) {
            const bool track=psx_mod_read_half(player+0x74)==17;
            if(track && !a.reload_track) {a.reload=a.t;a.reloading=true;++ammo_reloads;}
            if(!track) a.reloading=false;  // the refill came, or the reload was left (switch, death)
            a.reload_track=track;
        }
    }
    const int n=k_ammo_count[slot]>1?std::clamp(ammo_frame(a),0,(int)k_ammo_count[slot]-1):0;
    return k_ammo_frames[k_ammo_first[slot]+n];
}
static void ammo_hud_restore() {
    if(!ammo_belts_hidden) return;
    // Only records still as this left them (a restored state may differ).
    for(unsigned i=0;i<ammo_belts;++i) {
        const uint32_t r=ammo_belt+16*i;
        if(psx_mod_read_byte(r+2)==0 && psx_mod_read_byte(r+3)==13) psx_mod_write_byte(r+2,16);
    }
    ammo_belts_hidden=false;
}
static void ammo_hud(CPUState* cpu) {
    const bool redraw=render_override && replay_redraw_active();
    ammo_view=redraw?override_ammo_view:ammo_live_view();
    if(ammo_view<0 || ammo_view>=(int)(sizeof k_ammo_cells/sizeof k_ammo_cells[0]) || !hud_applied || !identity()) return;
    for(unsigned i=0;i<ammo_belts;++i)
        if(psx_mod_read_word(ammo_belt+16*i)!=0x0d103b7fu || psx_mod_read_word(ammo_belt+16*i+4)!=1008) return;
    const AmmoIconCell& c=k_ammo_cells[ammo_view];
    const GadgetArt art={0,984,205,1008,212,c.cell,c.clut};
    const unsigned slot=psx_mod_read_byte(player+0x233);
    const int x=(int16_t)psx_mod_read_half(hud_layout+8+4*slot),y=(int16_t)psx_mod_read_half(hud_layout+10+4*slot);
    if(!hud_box_draw(cpu,0,&art,0,x+1,y,true,true)) return;
    for(unsigned i=0;i<ammo_belts;++i) psx_mod_write_byte(ammo_belt+16*i+2,0);
    ammo_belts_hidden=true;++ammo_draws;
}
static const char* ammo_hud_debug_json() {
    static char buffer[256];
    std::snprintf(buffer,sizeof buffer,"{\"draws\":%u,\"view\":%d,\"slot\":%d,\"ammo\":%d,\"clip\":%d,\"clock\":%d,\"shots\":%u,\"refills\":%u,\"reloads\":%u}",
        ammo_draws,ammo_view,ammo_anim.slot,ammo_anim.ammo,ammo_anim.clip,(int)ammo_anim.t,ammo_shots,ammo_refills,ammo_reloads);
    return buffer;
}
