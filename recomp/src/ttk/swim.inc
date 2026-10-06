// D08M — modern swimming / wading (host locomotion layer).
// Included from modern_controls.cpp after terrain.inc.
//
// Iteration 9 — root/input bridge (see documentation/55-swim-controls-research.md):
//   • Never invent +0x20c, never zero +0x1f8 on entry, never world-XYZ locomotion.
//   • Free swim: face_view upright; WASD→D-pad + stick tables; suppress Square;
//     soft clamped body-Y for Space↑/Ctrl↓; optional look×W when deep.
//   • Optional late rewrite of +0xfc/+0x100 from camera-relative WASD.
//   • Wade/ledge: 8003e2d0(97|98) + anti-bounce holdoff/reassert.
//
// D08O — deep free-swim polish:
//   • A/D strafe: 8005201c's strafe branches read the layout's L2/R2 words
//     (800d1a68 = left → 88/91/92, 800d1b80 = right → 89/90/93). D-pad
//     Left/Right are turn buttons (71/70) and stick X is also turn, so the
//     host resolves the strafe pad bits from the layout tables instead.
//   • Ctrl descends from the surface: gate on floor margin (+0x1c8), not on
//     "already >0x100 under the surface".
//   • Free swim Space = ascend only; no host Space ledge exit (E / original
//     mantle owns leaving the water). Wade jump 8003e2d0(97|98) unchanged.
//
// D08O.2 — native swim state machine (800455bc, player+0x22c):
//   • state 4 = surface (anims 122 idle, 125 fwd, 126 back, 123/124 strafe).
//     80044c18 pins body Y to the surface every frame, so soft-Y can never
//     dive from here. The original dive is 80045564 → state 5 / anim 133;
//     Ctrl calls it directly (Square would also require "no ledge ahead").
//   • state 5 = underwater (127 idle, 128–130 thrust, 133 dive). Square/Cross
//     = thrust along body yaw + body pitch (+0x1e/+0x26, +900 up … −900 down;
//     D-pad Up/Down are pitch). Host sets yaw/pitch camera-relatively from
//     WASD / Space / Ctrl and injects Square; the original integrator moves
//     Duke and hands back to state 4 when he breaks the surface.
//
// D08O.3 — wade ledge assist (subway platforms from shallow water):
//   • The directed jump 98 rises only ~560 units; the subway ledge is ~635
//     above the water floor. It lands only from a ~700–1250-unit run-up; from
//     the wall the 98 is cancelled on frame one (splash), and from 300–600
//     units it hits the wall face (107) and falls back (105).
//   • Fix: when the standing probe (repeated 0/320/…/1280 units along the
//     view) sees a ledge ≥256 above the feet, the ordinary directed jump 98
//     is launched toward it with a higher takeoff velocity (applied at the
//     first ballistic update like the bed fix's delayed_vertical; 98
//     integrates ≈vy/96 per tick so −9000 rises ≈840 vs its own ≈560), XZ
//     speed scaled to the measured distance, and — within one probe step of
//     the wall — XZ held at zero until the animated foot clears the lip, since
//     98's first stride into the wall face is what bumped (107) / cancelled.
//     A first version used vertical 97 → 98; the user found that hop far too
//     high, so the arc is 98's own, only ≈280 taller.

static constexpr int32_t k_water_none = 0x40000000;
// Original 8005201c: <0x200 shallow/wade, [0x200,0x281) mid walk (controller
// early-outs), ≥0x281 deep Square/176. Anim 70/71 in shallow water are tank
// turns, not 3D swim — the crystal-2 turret corridor is depth 0x100.
static constexpr int32_t k_free_swim_depth = 0x281;
static constexpr int32_t k_swim_y_step = 96;
static constexpr int32_t k_swim_root_step = 48;
static constexpr int32_t k_swim_floor_margin = 0x1e0; // standing body≈+0x1f4 above feet
static constexpr int32_t k_stick_deflect = 0x60; // ≥ run threshold path (0x51)
static constexpr double k_wade_hz_walk = 3600.0;
static constexpr double k_wade_hz_run = 4800.0;
static constexpr uint32_t k_pad_words = 0x800d1440;
static constexpr uint32_t k_pad_word_stride = 21;
static constexpr int32_t k_ledge_assist_rise = -256;  // ledge at least this far above the feet (Y-down)
static constexpr int32_t k_ledge_assist_clear = 32;   // foot this far above the ledge top before XZ resumes (landing tolerates ~100 below)
static constexpr int k_ledge_assist_holdoff = 40;     // swim_update calls (2 per tick) to keep +0x20c quiet
static constexpr int32_t k_ledge_vy = -9000;          // 98's own takeoff ≈ −6900 (rise ≈560, g≈506/tick, Δy≈vy/96); this rises ≈840
static constexpr int32_t k_ledge_hold_distance = 320; // probe offsets up to this: XZ held until the lip is cleared
static constexpr double k_ledge_speed_hold = 3400.0;  // XZ once released near the apex when the jump started at the wall
static constexpr double k_ledge_speed_hold_far = 4000.0; // … when the ledge was found one probe step out
static constexpr double k_ledge_speed_base = 1900.0;  // XZ from takeoff otherwise (flight ≈40 ticks → travel ≈ speed/4)
static constexpr double k_ledge_speed_per_unit = 3.5; // + per unit of probe distance
static constexpr double k_ledge_speed_max = 6400.0;  // ≈ 98's own 6007
static uint64_t swim_frame, swim_epoch;
static unsigned swim_mantles, wade_jumps, swim_dives, ledge_assists;
static int swim_water_holdoff;
static int swim_boost_frames;
static Vec2 swim_boost_dir{};
static bool swim_boost_running;
static double swim_boost_want; // >0: exact ledge-assist speed instead of walk/run top-up
static bool swim_ledge_pending, swim_ledge_hold_xz;
static int32_t swim_ledge_top;
static Vec2 swim_ledge_dir{};
static double swim_ledge_speed;
static uint64_t swim_ledge_epoch;

static bool swim_trace() {
    static const bool enabled=std::getenv("DNTTK_SWIM_TRACE")!=nullptr;
    return enabled;
}

static int32_t water_surface_y() {
    return (int32_t)psx_mod_read_word(player+0x834);
}
static bool water_present() {
    return psx_mod_read_word(player+0x834)!=(uint32_t)k_water_none;
}
static int32_t water_depth_units() {
    if(!water_present())return 0;
    return (int32_t)psx_mod_read_word(player+0x1c8)-water_surface_y();
}
static bool water_flagged() {
    return water_present() && psx_mod_read_word(player+0x20c)!=0;
}
static bool water_submerged() {
    if(!water_present())return false;
    return (int32_t)psx_mod_read_word(player+8)>water_surface_y();
}
static bool in_water() {
    return water_flagged();
}
static int32_t water_body_delta() {
    return (int32_t)psx_mod_read_word(player+8)-water_surface_y();
}
static bool wade_anim(unsigned animation) {
    return animation==63 || (animation>=72 && animation<=79) ||
           animation==178 || animation==181;
}
static bool free_swim_anim(unsigned animation) {
    return animation==70 || animation==80 || animation==81 ||
           animation==175 || animation==177;
}
static bool airborne_jump_anim(unsigned animation) {
    return animation==97 || animation==98 || animation==103;
}
static bool free_swimming() {
    if(!water_flagged())return false;
    const unsigned animation=psx_mod_read_half(player+0x60);
    if(airborne_jump_anim(animation))return false;
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    if(mode==4 || mode==5)return true;
    if(water_depth_units()<k_free_swim_depth)return false;
    if(free_swim_anim(animation))return true;
    if(wade_anim(animation))return false;
    return true;
}
static bool wading() {
    if(!water_flagged() || free_swimming())return false;
    return !airborne_jump_anim(psx_mod_read_half(player+0x60));
}
static bool shallow_land_water() {
    if(!wading())return false;
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    if(mode==4 || mode==5)return false;
    return water_depth_units()<k_free_swim_depth;
}
bool wade_full_speed_ready() {
    return input_modernized() && gameplay_context() && shallow_land_water();
}
// Native swim state machine (800455bc) — player+0x22c 4 = surface, 5 = under.
static constexpr int32_t k_swim_pitch_max = 900;      // original ±clamp
static constexpr int32_t k_swim_pitch_vertical = 900;
static constexpr int32_t k_swim_pitch_diagonal = 640;
static bool swim_surface_anim(unsigned animation) {
    return animation>=122 && animation<=126;
}
static bool swim_underwater_anim(unsigned animation) {
    return animation==127 || (animation>=128 && animation<=130) || animation==133;
}
static bool swim_native_surface() {
    return water_flagged() && psx_mod_read_byte(player+0x22c)==4 &&
           swim_surface_anim(psx_mod_read_half(player+0x60));
}
static bool swim_native_underwater() {
    return water_flagged() && psx_mod_read_byte(player+0x22c)==5 &&
           swim_underwater_anim(psx_mod_read_half(player+0x60));
}
static bool swim_lease_ready() {
    if(!input_modernized() || !gameplay_context() || !independent_camera())return false;
    if(!input_snapshot(Context::Gameplay).active)return false;
    if((psx_mod_read_word(player)&0x20002) || (psx_mod_read_word(player+0x224)&0x20000000))return false;
    if(psx_mod_read_word(camera+0xa4)!=player || psx_mod_read_word(player+0x7d4)!=camera)return false;
    if(!identity() || !water_flagged())return false;
    if(traversal_state_ready())return false;
    return true;
}
bool swim_input_ready() {
    if(!swim_lease_ready() || !free_swimming())return false;
    if(traversal_input_ready() || airborne_input_ready())return false;
    // Underwater the D-pad is pitch; thrust is injected by swim_thrust_input_ready.
    if(swim_native_underwater())return false;
    return true;
}
// Underwater (state 5): any WASD / Space / Ctrl becomes Square thrust; the host
// has already pointed body yaw/pitch where that thrust should go.
bool swim_thrust_input_ready() {
    if(!swim_lease_ready() || !swim_native_underwater())return false;
    if(traversal_input_ready() || airborne_input_ready())return false;
    return true;
}
// Suppress Square for wade jumps AND free-swim vertical — Square is thrust/176,
// not modern ascend.
bool swim_host_owns_jump() {
    return swim_lease_ready() && (wading() || free_swimming());
}

static void swim_face_view_upright() {
    face_view();
    psx_mod_write_half(player+0x1e,0);
    psx_mod_write_half(player+0x26,0);
}

// 8005201c reads the forward magnitude word through 800d1b48[player+0x233].
// 800d1600 (stick X) is a turn axis in TTK — never fed from A/D.
static void swim_feed_stick(const InputFrame& f) {
    const unsigned idx=psx_mod_read_byte(player+0x233)&3u;
    const uint32_t p_a=psx_mod_read_word(0x800d1b48+idx*4);
    if(!p_a || (p_a&3))return;
    if(p_a<0x80000000u || p_a>0x801ff000u)return;
    const int32_t fwd=(int32_t)std::lround(f.move_y*k_stick_deflect);
    psx_mod_write_word(p_a,fwd);
}

// Pad word pointer → PSX pad bit. Words live at 800d1440+4*(bit+1+21*pad).
static int swim_pad_bit_from_word(uint32_t ptr,unsigned pad) {
    if(ptr<k_pad_words || (ptr&3))return -1;
    const int32_t index=(int32_t)((ptr-k_pad_words)/4)-(int32_t)(k_pad_word_stride*pad)-1;
    return (index>=0 && index<16)?index:-1;
}

// Strafe pad masks for the active layout (fallback L2/R2 = default layout).
void swim_strafe_pads(uint16_t& left,uint16_t& right) {
    left=0x100;right=0x200;
    const unsigned idx=psx_mod_read_byte(player+0x233)&3u;
    const int l=swim_pad_bit_from_word(psx_mod_read_word(0x800d1a68+idx*4),idx);
    const int r=swim_pad_bit_from_word(psx_mod_read_word(0x800d1b80+idx*4),idx);
    if(l>=0 && r>=0 && l!=r) {
        left=(uint16_t)(1u<<l);
        right=(uint16_t)(1u<<r);
    }
}

static void swim_rewrite_root(const InputFrame& f) {
    // Body faces view; local X ≈ strafe, local Z ≈ forward for 8005201c.
    if(!f.move_x && !f.move_y) {
        psx_mod_write_half(player+0xfc,0);
        psx_mod_write_half(player+0x100,0);
        return;
    }
    psx_mod_write_half(player+0xfc,(int16_t)std::lround(f.move_x*k_swim_root_step));
    psx_mod_write_half(player+0xfe,0);
    psx_mod_write_half(player+0x100,(int16_t)std::lround(f.move_y*k_swim_root_step));
}

// Mid-depth wade (depth 0x200..0x281, crystal-2 flooded corridor / ledge):
// the gait dispatcher (800486c8 and siblings) swaps every land gait for the
// wade clips 80/81, whose state handler 800539f8 applies the animation root
// (~22 units per frame, D-pad Forward only) after 800788e0 collision. The
// root is rewritten at that handler's entry (modern_controls.cpp), the same
// place the land run/walk handlers are owned; nothing here can help because
// the track advance (80059d18..80059d88) re-extracts and rotates the root
// after the player-update hook and before the handler.
static bool mid_wade_anim(unsigned animation) { return animation==80 || animation==81; }
static bool mid_wade_state() {
    return wading() && psx_mod_read_byte(player+0x22c)==1 &&
        mid_wade_anim(psx_mod_read_half(player+0x60)) &&
        water_depth_units()>=0x200;
}
bool mid_wade_input_ready() {
    return mid_wade_state() && swim_lease_ready() && locomotion_input_ready() &&
        !(psx_mod_read_word(player+0x224)&0x202002c1);
}

// Standing probe ahead of the view. On success `rise` is the ledge floor
// relative to the animated foot (+0x10): negative = above the feet.
// The standing probe reaches only ~350 units. To see a ledge from a normal
// run-up distance, repeat it from points shifted along the direction; the
// world position is written back before returning (a query, not a move).
static bool swim_probe_from(CPUState* cpu,Vec2 dir,int32_t ahead,int32_t& result,int32_t& floor) {
    const uint32_t x=psx_mod_read_word(player+4),z=psx_mod_read_word(player+12);
    if(ahead) {
        psx_mod_write_word(player+4,(uint32_t)((int32_t)x+(int32_t)std::lround(dir.x*ahead)));
        psx_mod_write_word(player+12,(uint32_t)((int32_t)z+(int32_t)std::lround(dir.z*ahead)));
    }
    const bool ok=terrain_probe(cpu,0x800797f4,dir,result,floor);
    if(ahead) {psx_mod_write_word(player+4,x);psx_mod_write_word(player+12,z);}
    return ok;
}

static bool swim_climb_ahead(CPUState* cpu,int32_t* rise=nullptr) {
    if(!cpu)return false;
    const float samples[][2]={{0,1},{-0.4f,0.9f},{0.4f,0.9f},{0,0.7f},{0,0.45f}};
    for(auto& s:samples) {
        auto dir=movement(view_yaw(),s[0],s[1]);
        int32_t result=0,floor=0;
        const bool ok=swim_probe_from(cpu,dir,0,result,floor);
        if(swim_trace())std::fprintf(stderr,"ttk-climb dir=%.2f/%.2f ok=%d result=%d floor=%d foot=%d y=%d\n",s[0],s[1],ok,result,floor,
            (int32_t)psx_mod_read_word(player+0x10),(int32_t)psx_mod_read_word(player+8));
        if(!ok)continue;
        if((result==0 || result==5) && floor>=-1792 && floor<=1280) {
            if(rise)*rise=floor;
            return true;
        }
    }
    return false;
}

// Ledge search along the view: a step the standing probe refuses (result 1)
// whose floor sits 256–1792 above the feet, within a run-up distance.
// `blocked` reports a refused step right in front (a plain wall, no ledge).
static bool swim_ledge_ahead(CPUState* cpu,Vec2 dir,int32_t& rise,int32_t& distance,bool* blocked=nullptr) {
    if(!cpu)return false;
    if(blocked)*blocked=false;
    for(int32_t ahead: {0,320,640,960,1280}) {
        int32_t result=0,floor=0;
        const bool ok=swim_probe_from(cpu,dir,ahead,result,floor);
        if(swim_trace())std::fprintf(stderr,"ttk-ledge probe ahead=%d ok=%d result=%d floor=%d\n",ahead,ok,result,floor);
        if(!ok)return false;
        if(result==1 && floor<=k_ledge_assist_rise && floor>=-1792) {rise=floor;distance=ahead;return true;}
        if(ahead==0 && result==1 && blocked)*blocked=true;
        if(result!=0 || floor<-128 || floor>256)return false; // not flat wading floor: stop scanning
    }
    return false;
}


static void swim_boost_horizontal(Vec2 dir,bool directed,bool running) {
    if(!directed)return;
    const bool exact=swim_boost_want>0;
    const double want=exact?swim_boost_want:(running?k_wade_hz_run:k_wade_hz_walk);
    const double vx=(int32_t)psx_mod_read_word(player+0x1f4);
    const double vz=(int32_t)psx_mod_read_word(player+0x1fc);
    const double mag=std::hypot(vx,vz);
    // D08W: the original launch (0x8003e2d0) aims along Duke's facing. Keep its
    // speed when it is already faster than the wade minimum, but always along
    // the held camera-relative direction, as a dry-ground D08C jump does; the
    // old rescale kept the facing, so strafe and backward wade jumps went forward.
    const double speed=exact?want:std::max(mag,want);
    psx_mod_write_word(player+0x1f4,(int32_t)std::lround(dir.x*speed));
    psx_mod_write_word(player+0x1fc,(int32_t)std::lround(dir.z*speed));
    swim_boost_dir=dir;
    swim_boost_running=running;
}

static void swim_reassert_boost() {
    if(swim_boost_frames<=0)return;
    if(!airborne_jump_anim(psx_mod_read_half(player+0x60))) {
        swim_boost_frames=0;swim_boost_want=0;return;
    }
    swim_boost_horizontal(swim_boost_dir,true,swim_boost_running);
    --swim_boost_frames;
}

static bool swim_start_airborne_jump(CPUState* cpu,const InputFrame& f,Vec2 dir,bool directed) {
    const unsigned jump_animation=directed?98:97;
    const uint32_t water=psx_mod_read_word(player+0x20c);
    if(water)psx_mod_write_word(player+0x20c,0);
    if(!original_call(cpu,0x8003e2d0,jump_animation)) {
        if(water)psx_mod_write_word(player+0x20c,water);
        return false;
    }
    psx_mod_write_half(player+0x60,jump_animation);
    psx_mod_write_half(player+0x68,0);
    psx_mod_write_half(player+0x6a,0);
    // Longer holdoff + XZ reassert reduces lip bounce-back on subway ledges.
    if(water)swim_water_holdoff=22;
    swim_boost_want=0;
    swim_boost_horizontal(dir,directed,input_running());
    if(directed)swim_boost_frames=12;
    takeoff_intent=dir;
    takeoff_epoch=f.epoch;takeoff_frame=f.sequence;
    takeoff_valid=true;takeoff_obstacle=false;takeoff_direction=directed;
    intent=dir;had_movement=directed;intent_epoch=f.epoch;movement_frame=f.sequence;
    ground_frame=f.sequence;ground_epoch=f.epoch;
    flight_intent=directed?dir:Vec2{0,0};
    flight_epoch=f.epoch;
    flight_valid=true;
    // D08Z: wade jumps keep their own ledge/boost horizontal control; no
    // manual air steering (it only acts on a seeded cap).
    air_cap=0;
    return true;
}

// Ledge assist, per ballistic update of the directed jump (8003ebf4 hook,
// before the original integrates): hold the XZ speed at the distance-scaled
// value — zero while still below the lip when the jump started at the wall —
// and release once the animated foot has cleared the ledge top.
static void swim_ledge_ballistic(const InputFrame& f) {
    if(!swim_ledge_pending)return;
    const unsigned animation=psx_mod_read_half(player+0x60);
    const int32_t vy=(int32_t)psx_mod_read_word(player+0x1f8);
    const int32_t foot=(int32_t)psx_mod_read_word(player+0x10);
    const bool clear=foot<=swim_ledge_top-k_ledge_assist_clear;
    if(swim_trace())std::fprintf(stderr,"ttk-ledge ballistic anim=%u/%u st=%u/%u vy=%d vz=%d y=%d foot=%d delayed=%d\n",animation,psx_mod_read_half(player+0x68),
        psx_mod_read_byte(player+0x22c),psx_mod_read_byte(player+0x22d),vy,(int32_t)psx_mod_read_word(player+0x1fc),(int32_t)psx_mod_read_word(player+8),foot,delayed_takeoff);
    if(animation!=98 || f.epoch!=swim_ledge_epoch || psx_mod_read_byte(player+0x22c)!=9 || (vy>=0 && !clear)) {
        if(swim_trace())std::fprintf(stderr,"ttk-ledge abort anim=%u vy=%d foot=%d top=%d\n",animation,vy,foot,swim_ledge_top);
        swim_ledge_pending=false;swim_ledge_hold_xz=false;return;
    }
    if(!clear && swim_ledge_hold_xz) {
        psx_mod_write_word(player+0x1f4,0);psx_mod_write_word(player+0x1fc,0);
        return;
    }
    swim_boost_want=swim_ledge_speed;
    swim_boost_horizontal(swim_ledge_dir,true,false);
    if(clear) {
        swim_boost_frames=12;
        swim_ledge_pending=false;swim_ledge_hold_xz=false;++ledge_assists;
        if(swim_trace())std::fprintf(stderr,"ttk-ledge clear vy=%d foot=%d top=%d speed=%.0f\n",vy,foot,swim_ledge_top,swim_ledge_speed);
    }
}

static void swim_try_wade_jump(CPUState* cpu,const InputFrame& f) {
    if(!input_jump_pending())return;
    const unsigned animation=psx_mod_read_half(player+0x60);
    const bool standing=wade_anim(animation) || animation==70 || animation==175 ||
        animation==176 || animation==177 || mid_wade_anim(animation);
    int32_t rise=0,distance=0;bool blocked=false;
    // D08W: the ledge and climb assists jump toward what lies ahead of the view,
    // so they serve Space alone or a jump with W held. Strafing (A/D, also after
    // running forward) or backing (S) is the ordinary directed jump below; the
    // climb probe accepts flat water ahead, which used to turn every strafe or
    // backward jump in the subway into a forward one.
    const bool toward_view=(!f.move_x && !f.move_y) || f.move_y>0.05f;
    if(standing && !(psx_mod_read_word(player+0x224)&0x202002c1) && !f.held[crouch] &&
       toward_view && swim_ledge_ahead(cpu,movement(view_yaw(),0,1),rise,distance,&blocked)) {
        // Ledge well above the feet: the ordinary directed jump toward it,
        // a little more lift, XZ held while below the lip when at the wall.
        if(!input_take_jump())return;
        auto dir=movement(view_yaw(),0,1);
        const int32_t foot=(int32_t)psx_mod_read_word(player+0x10);
        if(swim_start_airborne_jump(cpu,f,dir,true)) {
            // Close to the wall 98's first stride would bump it (107) before
            // the feet are up, so XZ waits for the lip in the near buckets.
            swim_ledge_pending=true;swim_ledge_top=foot+rise;swim_ledge_hold_xz=distance<=k_ledge_hold_distance;
            // Speed scaled to the measured distance so Duke lands just past
            // the lip instead of flying into whatever stands behind it.
            swim_ledge_dir=dir;
            swim_ledge_speed=swim_ledge_hold_xz?(distance==0?k_ledge_speed_hold:k_ledge_speed_hold_far):
                std::clamp(k_ledge_speed_base+k_ledge_speed_per_unit*distance,k_ledge_speed_base,k_ledge_speed_max);
            swim_ledge_epoch=f.epoch;
            delayed_vertical=k_ledge_vy;delayed_takeoff=true;terrain_epoch=f.epoch;
            // The generic wade boost must not fight the assist's speed/hold.
            swim_boost_frames=0;swim_boost_want=swim_ledge_speed;
            swim_water_holdoff=std::max(swim_water_holdoff,k_ledge_assist_holdoff);
            ++standing_takeoffs;++wade_jumps;
            if(swim_trace())std::fprintf(stderr,"ttk-ledge launch rise=%d distance=%d foot=%d top=%d hold=%d\n",rise,distance,foot,swim_ledge_top,swim_ledge_hold_xz);
        }
        return;
    }
    if(toward_view && swim_climb_ahead(cpu,&rise)) {
        if(!input_take_jump())return;
        auto dir=movement(view_yaw(),f.move_x,f.move_y>0.05f?f.move_y:1.0f);
        if(swim_start_airborne_jump(cpu,f,dir,true)) {
            ++standing_takeoffs;++wade_jumps;
        }
        return;
    }
    if(!standing)return;
    if(psx_mod_read_word(player+0x224)&0x202002c1)return;
    if(!input_take_jump())return;
    // A directed 98 straight into a plain wall is cancelled by the original on
    // its first frame (Duke just splashes); give the vertical hop instead.
    const bool directed=(f.move_x || f.move_y) && !(blocked && f.move_y>0.05f);
    auto dir=directed?movement(view_yaw(),f.move_x,f.move_y):Vec2{0,0};
    if(swim_start_airborne_jump(cpu,f,dir,directed)) {
        ++standing_takeoffs;++wade_jumps;
    }
}

// Soft Y only while original water flag is set. Never clears fall VY, never
// invents +0x20c, never rises through the free surface.
static void swim_apply_vertical(const InputFrame& f) {
    if(!water_flagged())return;
    int32_t dy=0;
    if(f.held[jump] && !f.held[crouch]) dy=-k_swim_y_step;
    else if(f.held[crouch] && !f.held[jump]) dy=k_swim_y_step;
    else if(water_body_delta()>0xa0 && (f.move_x || f.move_y)) {
        dy=(int32_t)std::lround(-std::sin(pitch)*f.move_y*k_swim_y_step);
    }
    if(!dy)return;
    int32_t y=(int32_t)psx_mod_read_word(player+8);
    const int32_t surf=water_surface_y();
    int32_t next=y+dy;
    if(dy<0 && next<surf) next=surf;
    if(dy>0) {
        // Descend from the surface or mid-water, but keep a floor margin so
        // Ctrl cannot pin Duke into unrecoverable ground (Space always recovers).
        const int32_t floor_y=(int32_t)psx_mod_read_word(player+0x1c8);
        const int32_t limit=std::min(floor_y-k_swim_floor_margin,surf+0x1800);
        if(next>limit) next=limit;
        if(next<=y)return;
    }
    psx_mod_write_word(player+8,next);
}

// Surface (state 4): Ctrl = the original dive transition (80045564 → state 5,
// anim 133). Same guards the original applies before its Square dive.
static void swim_try_dive(CPUState* cpu,const InputFrame& f) {
    if(!cpu || !f.held[crouch] || f.held[jump])return;
    if(psx_mod_read_word(player+0x224)&0x241)return;
    if(psx_mod_read_half(player+0x244))return;
    if(original_call(cpu,0x80045564,player+0x60))++swim_dives;
}

// Underwater (state 5): body yaw = camera-relative move direction (view yaw
// when idle); body pitch = look pitch along the move axis, or ±vertical for
// Space / Ctrl. The original thrust (Square, injected by input_pad) follows it.
static void swim_steer_underwater(const InputFrame& f) {
    const bool moving=f.move_x || f.move_y;
    double heading_rad=view_yaw();
    double forward_share=0.0;
    if(moving) {
        const auto dir=movement(view_yaw(),f.move_x,f.move_y);
        heading_rad=std::atan2(dir.x,dir.z);
        forward_share=f.move_y/std::hypot((double)f.move_x,(double)f.move_y);
    }
    const int heading=static_cast<int>(std::lround(heading_rad*4096/tau)) & 4095;
    psx_mod_write_half(player+0x1c,heading);
    psx_mod_write_half(player+0x24,heading);
    psx_mod_write_half(player+0x2b4,0);
    // Host view pitch is +down (forward.y = sin(pitch)); body pitch is +up.
    const double look_up=std::clamp(-pitch*4096/tau,(double)-k_swim_pitch_max,(double)k_swim_pitch_max);
    int32_t body_pitch;
    if(f.held[jump] && !f.held[crouch]) body_pitch=moving?k_swim_pitch_diagonal:k_swim_pitch_vertical;
    else if(f.held[crouch] && !f.held[jump]) body_pitch=moving?-k_swim_pitch_diagonal:-k_swim_pitch_vertical;
    else if(moving) body_pitch=(int32_t)std::lround(look_up*forward_share);
    else body_pitch=(int32_t)std::lround(look_up);
    psx_mod_write_half(player+0x1e,(int16_t)body_pitch);
    psx_mod_write_half(player+0x26,(int16_t)body_pitch);
}

// D08O1 - fire while swimming underwater. The original state-5 handlers
// (800455bc via table 80013ed8: idle 127 at 8004653c, thrust 128-130 at
// 80046994) test the Cross action word before Square: with a weapon out, held
// fire keeps Duke in, or returns him to, idle 127, so he stops to shoot. While
// the player fires and holds a swim direction, bit 0 of that word (this tick)
// is hidden from the swim handler only - its entry, called from 8004b594 - and
// put back at the next player-update step, 80055e80 (callback table 800c2754
// [3]), which 8005a210 always calls straight after; 800493a4 and the camera
// update also restore it if that step does not run (other hooks do not: the
// handler calls 0x80076330 before its fire test). The weapon still sees
// fire; the original thrust then carries Duke where swim.inc points him.
static unsigned swim_fire_masks, swim_fire_restores;
static uint32_t swim_fire_word;
static uint32_t swim_fire_value;
static bool swim_fire_hidden;
bool swim_weapon_ready() {
    return swim_lease_ready() && (swim_native_underwater() || swim_native_surface());
}
static void swim_fire_restore(uint32_t address=0) {
    if(!swim_fire_hidden)return;
    swim_fire_hidden=false;
    if(swim_trace())std::fprintf(stderr,"ttk-swim-fire restore at %08x word=%08x anim=%u\n",address,psx_mod_read_word(swim_fire_word),psx_mod_read_half(player+0x60));
    // Only undo our own write; a newer pad sample stays as the original made it.
    if(psx_mod_read_word(swim_fire_word)==(swim_fire_value&~1u))psx_mod_write_word(swim_fire_word,swim_fire_value);
    ++swim_fire_restores;
}
static void swim_fire_hide(CPUState* cpu,uint32_t ra) {
    swim_fire_restore(0x800455bc);
    const auto& f=input_snapshot(Context::Gameplay);
    if(cpu->gpr[4]!=player || ra!=0x8004b5bc || !f.held[fire] || f.held[holster])return;
    if(!(f.move_x || f.move_y || f.held[jump] || f.held[crouch]))return;
    if(psx_mod_read_byte(player+0x3b8)!=2 || !swim_thrust_input_ready())return;
    const uint32_t word=psx_mod_read_word(0x800d1b50+4*(psx_mod_read_byte(player+0x233)&3u));
    if(word<k_pad_words || word>=k_pad_words+4*k_pad_word_stride*4 || (word&3))return;
    const uint32_t value=psx_mod_read_word(word);
    if(!(value&1u))return;
    psx_mod_write_word(word,value&~1u);
    if(swim_trace())std::fprintf(stderr,"ttk-swim-fire hide %08x=%08x anim=%u square=%08x\n",word,value,psx_mod_read_half(player+0x60),psx_mod_read_word(psx_mod_read_word(0x800d1b88+4*(psx_mod_read_byte(player+0x233)&3u))));
    swim_fire_word=word;swim_fire_value=value;swim_fire_hidden=true;
    ++swim_fire_masks;
}

static void swim_update(CPUState* cpu,bool late) {
    const auto& f=input_snapshot(Context::Gameplay);
    // The assist runs at the ballistic hook (no swim lease in flight); drop it
    // here if the jump never became airborne or the frame is gone.
    if(!late && swim_ledge_pending &&
       (!f.active || f.epoch!=swim_ledge_epoch || !identity() || (psx_mod_read_word(player)&0x20002) ||
        psx_mod_read_half(player+0x60)!=98 || psx_mod_read_byte(player+0x22c)!=9)) {
        swim_ledge_pending=false;swim_ledge_hold_xz=false;
    }
    if(!late && swim_trace() && (swim_ledge_pending || airborne_jump_anim(psx_mod_read_half(player+0x60))))
        std::fprintf(stderr,"ttk-swim anim=%u st=%u x=%d z=%d y=%d foot=%d vy=%d vx=%d vz=%d water=%08x flags=%08x pending=%d top=%d lease=%d\n",
            psx_mod_read_half(player+0x60),psx_mod_read_byte(player+0x22c),(int32_t)psx_mod_read_word(player+4),(int32_t)psx_mod_read_word(player+12),(int32_t)psx_mod_read_word(player+8),
            (int32_t)psx_mod_read_word(player+0x10),(int32_t)psx_mod_read_word(player+0x1f8),(int32_t)psx_mod_read_word(player+0x1f4),
            (int32_t)psx_mod_read_word(player+0x1fc),psx_mod_read_word(player+0x20c),psx_mod_read_word(player+0x224),
            swim_ledge_pending,swim_ledge_top,swim_lease_ready());
    if(!late && swim_trace() && airborne_jump_anim(psx_mod_read_half(player+0x60)))
        std::fprintf(stderr,"ttk-swim early anim=%u/%u st=%u vy=%d vz=%d y=%d foot=%d pending=%d delayed=%d\n",psx_mod_read_half(player+0x60),psx_mod_read_half(player+0x68),
            psx_mod_read_byte(player+0x22c),(int32_t)psx_mod_read_word(player+0x1f8),(int32_t)psx_mod_read_word(player+0x1fc),(int32_t)psx_mod_read_word(player+8),
            (int32_t)psx_mod_read_word(player+0x10),swim_ledge_pending,delayed_takeoff);
    if(!swim_lease_ready()) {
        if(swim_water_holdoff>0)--swim_water_holdoff;
        // In flight the water flag is clear, so the lease is not held; the XZ
        // reassert must still run there or it never runs at all.
        if(late && swim_boost_frames>0 && f.active && identity())swim_reassert_boost();
        else if(swim_boost_frames>0)--swim_boost_frames;
        return;
    }
    swim_frame=f.sequence;swim_epoch=f.epoch;

    if(swim_water_holdoff>0 && airborne_jump_anim(psx_mod_read_half(player+0x60))) {
        psx_mod_write_word(player+0x20c,0);
        --swim_water_holdoff;
    } else if(swim_water_holdoff>0) {
        swim_water_holdoff=0;
    }
    if(late)swim_reassert_boost();

    if(wading()) {
        if(cpu)swim_try_wade_jump(cpu,f);
        return;
    }

    if(swim_native_underwater()) {
        swim_steer_underwater(f);
        return;
    }
    if(swim_native_surface()) {
        // Surface: original D-pad/L2/R2 swim (injected by input_pad); body
        // upright facing the view; Ctrl dives; Space is inert; exit is mantle/E.
        swim_face_view_upright();
        swim_try_dive(cpu,f);
        return;
    }
    if(free_swimming()) {
        // Land-side water anims (70/80/81/175–177, mid depth) before the
        // original hands over to its swim states. Space ascends, Ctrl dips.
        swim_face_view_upright();
        swim_feed_stick(f);
        swim_rewrite_root(f);
        if(late)swim_apply_vertical(f);
        return;
    }
}
