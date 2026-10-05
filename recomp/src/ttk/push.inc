// D08T pushable objects (Modernized). Original rules, verified on the first
// map's alley dumpster (documentation/64-d08t-pushable-objects.md):
//  * Object type flags live at *(0x800d2660) + 28 * object[+0x2c].
//    0x08000000 = pushable, 0xc0 = climbable. The dumpster (type 165) has both.
//  * Idle 63, holstered, a fresh Cross against a pushable object: 0x80051890
//    lines Duke up (LineUpWithObject, 0x113) and starts grab 121, remembering
//    the object at player+0x290.
//  * Grab 121 at track 3 (0x80048c74): Cross still held + Up pushes (120),
//    + Down pulls (119); the object moves along one world axis (0x800910a0).
//    Releasing Cross lets go.
//  * Walking or pressing Up into a climbable object with Cross held climbs it
//    instead (0x80051cf0 -> 139/140, mode 8). That branch is why E used to
//    mount the dumpster instead of grabbing it.
// Modernized contract (D08T1): pushability adds a verb, it never remaps one.
// E is always the normal interaction/mantle: E alone at a pushable object does
// nothing (as at any climbable object) and W + E into a climbable one climbs it
// through the original 0x80051cf0 branch. Holding Grab / Manipulate grabs any
// pushable object, W/S push/pull relative to the camera and releasing Grab lets
// go. The original grab, alignment, push/pull motion and climb animations stay
// authoritative; the host only chooses which original button reaches them.
static constexpr uint32_t object_pushable=0x08000000u, object_climbable=0xc0u;
// 0x8001b634 allocates the type table as 0x7428 bytes: 1062 types of 28 bytes.
// D08T2: later eras use types above 512 (medieval Duke-symbol block: 924).
static constexpr unsigned object_type_count=0x7428u/28u;
static unsigned push_masks, push_idle_masks;
static uint32_t object_type_flags(uint32_t object) {
    if(object<0x80010000u || object>=0x80200000u || (object&3))return 0;
    const uint32_t table=psx_mod_read_word(0x800d2660);
    const unsigned type=psx_mod_read_half(object+0x2c);
    if(table<0x80010000u || table>=0x80200000u || (table&3) || type>=object_type_count)return 0;
    return psx_mod_read_word(table+28*type);
}
static bool push_animation(unsigned animation) {return animation>=119 && animation<=121;}
// Camera-only lease widening (state()): no movement, facing or gait ownership.
static bool push_state_early() {
    return push_animation(psx_mod_read_half(player+0x60)) &&
        psx_mod_read_byte(player+0x22c)==0 && psx_mod_read_byte(player+0x22d)==0 &&
        psx_mod_read_byte(player+0x3b8)==0 &&
        (object_type_flags(psx_mod_read_word(player+0x290))&object_pushable);
}
bool push_grab_ready() {
    return input_modernized() && gameplay_context() && !(psx_mod_read_word(player)&0x20002) &&
        psx_mod_read_word(camera+0xa4)==player && psx_mod_read_word(player+0x7d4)==camera &&
        push_state_early() && identity();
}
// Duke is touching a pushable object on normal ground (idle, gait or wall bump).
bool push_contact_ready() {
    if(!player_identity_ready() || (psx_mod_read_word(player)&0x20002) ||
       psx_mod_read_byte(player+0x22c)!=0 || psx_mod_read_byte(player+0x22d)!=0)return false;
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(!land_gait_anim(animation) && animation!=94 && animation!=95)return false;
    return object_type_flags(psx_mod_read_word(player+0x174))&object_pushable;
}
// Original idle with the weapon stowed: a fresh Cross here reaches 0x80051890.
bool push_idle_ready() {return interaction_ready() && psx_mod_read_half(player+0x60)==63;}
// Camera-relative W/A/S/D projected on Duke's facing (the push axis after the
// original line-up): toward the object is original Up (push), away is Down.
uint16_t push_pad(float move_x,float move_y) {
    if((!move_x && !move_y) || !push_grab_ready())return 0;
    const auto v=movement(view_yaw(),move_x,move_y);
    const double facing=psx_mod_read_half(player+0x1c)*tau/4096;
    const double along=v.x*std::sin(facing)+v.z*std::cos(facing);
    return along>0.38?16:along<-0.38?64:0;
}
// Clear this tick's Action (Cross) bit; the next tick sees a fresh press.
static bool push_mask_cross() {
    const uint32_t cross=psx_mod_read_word(0x800d1b50+4*psx_mod_read_byte(player+0x233));
    if(cross<0x80010000u || cross>=0x80200000u || (cross&3))return false;
    const uint32_t word=psx_mod_read_word(cross);
    if(!(word&1))return false;
    psx_mod_write_word(cross,word&~1u);
    return true;
}
// Only a touched object that is pushable and has no other Action use (switch,
// pickup or use flags 0x01c00000, which the original checks after the grab).
static bool push_only_object(uint32_t object) {
    const uint32_t flags=object_type_flags(object);
    return (flags&object_pushable) && !(flags&0x01c00000u);
}
// 0x80051cf0 entry: the original walk/Up-into-obstacle action. Grab's own Cross
// must never climb a pushable object (it asked to hold it); E's Cross still does,
// exactly as at any climbable object.
static void push_obstacle_action(CPUState* cpu,uint32_t ra) {
    const auto& f=input_snapshot(Context::Gameplay);
    if(cpu->gpr[4]!=player || (ra!=0x80052544 && ra!=0x8005349c && ra!=0x8005384c) ||
       !f.active || !input_push_grab_owns_cross() || !player_identity_ready())return;
    if(!(object_type_flags(psx_mod_read_word(player+0x174))&object_pushable))return;
    if(push_mask_cross())++push_masks;
}
// 0x80051890 entry: the original idle Action, whose fresh Cross grabs a touched
// pushable object (anim 121). In captured Modernized play only Grab may do that:
// E's Cross is masked here, so E at a pushable object does what it does at any
// other climbable object (nothing standing still; W + E climbs).
static void push_idle_action(CPUState* cpu,uint32_t ra) {
    const auto& f=input_snapshot(Context::Gameplay);
    if(cpu->gpr[4]!=player || (ra!=0x800467fc && ra!=0x80052c1c) ||
       !f.active || input_push_grab_owns_cross() || !player_identity_ready())return;
    if(!push_only_object(psx_mod_read_word(player+0x174)))return;
    if(push_mask_cross())++push_idle_masks;
}
