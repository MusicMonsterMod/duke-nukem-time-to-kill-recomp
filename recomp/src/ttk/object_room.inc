// Objects in the room they stand in (Modernized with EXTENDED draw distance,
// Polymorph; developer DNTTK_OBJECT_ROOM=0 turns it off). Ported on its own
// from the D23AL follow-up (note 512 section 4) onto the 18 November build.
// The object pass hands each renderer the list entry of the object's room byte
// (+0x2e), and the renderer rejects the object when its sphere leaves that
// entry's rectangle (0x80031584). A pickup whose room byte names the room
// beyond a doorway while it lies on this side (the user's UI slot 12, GOLD AND
// GUNS: the ammo by the mine door) then vanished as Duke stepped towards it,
// its room's rectangle shrinking to the doorway above it. When the object's
// position lies inside the box of other rooms the host shows, the renderer
// gets a copy of the entry whose rectangle also covers those rooms'
// rectangles; host depth still hides what lies behind their walls. Live
// frames and redraws alike (the copy goes in mod memory reserved before the
// redraw workers fork, render_state_prepare).
uint32_t object_room_scratch=0;
uint64_t object_room_widened=0;
static bool object_room_addr(uint32_t a,uint32_t n) {
    if(a&1) return false;
    if(a>=0x80010000u && a<=0x80200000u-n) return true;
    const uint32_t p=a&0x1fffffffu;
    return p>=0x1f000000u && p+n<=0x1f800000u && (a&0xe0000000u)!=0;
}
static bool object_room_on() {
    static int v=-1;if(v<0) {const char* e=std::getenv("DNTTK_OBJECT_ROOM");v=!(e && e[0]=='0');}
    return v && input_modernized() && draw_distance_extended();
}
static void object_room_rect(CPUState* cpu,uint32_t scratch) {
    const uint32_t e=cpu->gpr[6],obj=cpu->gpr[5];
    if(!scratch || !object_room_on() || !object_room_addr(obj,0x60)) return;
    if(!(e>=0x800d68b0u && e<0x800d68b0u+12*64 && !((e-0x800d68b0u)%12))) return;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || rooms!=host_vis_room_rect.size() || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return;
    const int own=(int8_t)psx_mod_read_byte(obj+0x2e);
    double p[3];for(int k=0;k<3;++k) p[k]=(int32_t)psx_mod_read_word(obj+4+4*k);
    double r[4]={(double)(int16_t)psx_mod_read_half(e),(double)(int16_t)psx_mod_read_half(e+2),0,0};
    r[2]=r[0]+(int16_t)psx_mod_read_half(e+4);r[3]=r[1]+(int16_t)psx_mod_read_half(e+6);
    bool widened=false;
    for(uint32_t room=0;room<rooms;++room) {
        if((int)room==own) continue;
        const auto& q=host_vis_room_rect[room];
        if(q[2]<q[0] || q[3]<q[1]) continue;
        const uint32_t a=table+room*88;
        bool in=true;
        for(int k=0;k<3 && in;++k) in=p[k]>=(int32_t)psx_mod_read_word(a+4*k) && p[k]<=(int32_t)psx_mod_read_word(a+0x20+4*k);
        if(!in) continue;
        if(q[0]<r[0] || q[1]<r[1] || q[2]>r[2] || q[3]>r[3]) {host_vis_union(r,q.data());widened=true;}
    }
    if(!widened) return;
    const double* v=host_vis_frame_guard;   // never past the guard-banded view
    const int32_t x0=(int32_t)std::floor(std::max(r[0],v[0])),y0=(int32_t)std::floor(std::max(r[1],v[1]));
    const int32_t x1=(int32_t)std::ceil(std::min(r[2],v[2])),y1=(int32_t)std::ceil(std::min(r[3],v[3]));
    if(x1<=x0 || y1<=y0) return;
    psx_mod_write_half(scratch,(uint16_t)(int16_t)x0);psx_mod_write_half(scratch+2,(uint16_t)(int16_t)y0);
    psx_mod_write_half(scratch+4,(uint16_t)(int16_t)(x1-x0));psx_mod_write_half(scratch+6,(uint16_t)(int16_t)(y1-y0));
    psx_mod_write_word(scratch+8,psx_mod_read_word(e+8));
    cpu->gpr[6]=scratch;++object_room_widened;
}
// Entry of 0x80031d10 / 0x80032e78 (registered after frame_replay's actor_draw).
void object_room_entry(CPUState* cpu,uint32_t) {
    if(!host_vis_shown_valid || cpu->gpr[4]!=host_vis_frame_cam_addr) return;
    if(!object_room_scratch && !psx_mod_replay_active()) object_room_scratch=psx_mod_alloc_guest_memory(16,16);
    object_room_rect(cpu,object_room_scratch);
}
