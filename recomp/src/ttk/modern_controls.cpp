// Bounded SLUS-00583 adapter for the authenticated level overlays in
// control_guards.inc (LEVEL00, LEVEL01). Original routines retain all integration,
// collision responses and camera constraints. No final position writes.
#include "modern_controls.h"
#include "weapon_aim.h"
#include "aim_weapons.h"
#include "pc_input.h"
#include "inventory_hud.h"
#include "control_math.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "code_identity.h"
#include "near_clip.h"
#include "pgxp.h"
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include <map>
#include <string>

extern "C" {extern int g_precise_mode,g_ls_mode,g_psx_call_bail;}
namespace ttk {
static constexpr uint32_t player=0x800d7198, camera=0x800d6eb0;
// Mid-depth wade root per player tick (30 Hz): the 80/81 clip carries ~25
// units (uneven 6..44); the land run handler carries ~100 (its own floor is
// 80). Gains scale the clip rhythm; floor/ceiling keep it in the land band.
static constexpr double mid_wade_run_gain=4.0, mid_wade_run_floor=80.0, mid_wade_run_ceiling=120.0;
static constexpr double mid_wade_walk_gain=2.0, mid_wade_walk_floor=40.0, mid_wade_walk_ceiling=60.0;
struct Guard { uint32_t address, size; const char* digest; };
#include "control_guards.inc"
static uint64_t camera_frame, camera_epoch, ground_frame, ground_epoch;
static uint64_t moves, probes, cameras, refusals;
static Vec2 intent{};
static double gait_length;
static uint64_t gait_frame, gait_epoch;
static bool gait_running;
static double stride_speed,fall_speed;
static uint64_t stride_epoch,landing_until,landing_epoch;
static uint32_t walking_sp, probe_return, walking_size;
static bool had_movement;
static bool flight_valid;
static bool takeoff_valid, takeoff_direction;
static uint64_t takeoff_epoch, takeoff_frame;
static Vec2 takeoff_intent{};
static bool takeoff_obstacle;
static int32_t takeoff_top;
static unsigned standing_takeoffs;
static bool takeoff_owned() {
    const auto& f=input_snapshot(Context::Gameplay);
    return takeoff_valid && f.active && takeoff_epoch==f.epoch &&
        f.sequence>=takeoff_frame && f.sequence-takeoff_frame<=60;
}
static uint64_t movement_frame, flight_epoch;
static Vec2 flight_intent{};
static uint64_t jumps, speed_changes, selections;
static uint32_t weapon_update_sp;
static bool weapon_transition_boost;
static uint64_t intent_epoch;
static bool walking;
static bool step_dispatch, short_fall, large_fall, delayed_takeoff, fall_velocity_pending, fall_walking, small_landing;
static uint64_t terrain_epoch;
static int32_t delayed_vertical;
static unsigned step_drops, run_offs, delayed_jumps, edge_jumps, edge_postpones;

static bool orbit_valid;
static uint32_t orbit_sp;
static uint64_t looks, orbits, facings, arms, orientations;
static uint32_t presentation_vector;
static double yaw,pitch,radius,preferred_radius,distance_seen;
static uint64_t distance_epoch,distance_frame;
// D10: recenter swing, shoulder offset and persisted camera preferences.
static bool recentering,rest_pitch_valid;
static double rest_pitch,shoulder_offset;
static uint64_t recenter_seen,recenters;
static double saved_distance=-1,written_distance=-1;
static int written_shoulder=2,seen_shoulder=2,written_view=2,seen_view=2;
static uint64_t camera_pref_changed;
static LookAccumulator look;
// D11 first-person blend, 0 = third-person orbit, 1 = eye level (first_person.inc).
static double fp_blend;
static uint64_t fp_frame;
// D11: a jump that starts while the eye view is live keeps the camera-only
// lease through its original animations (Space-only jumps have no flight lease).
// D22B: in third person too; the orbit used to drop to the original camera.
static bool jump_camera;
// D17: set in a render replay worker, which has no live lease of its own.
static bool render_override=false, override_lease_cam=false, last_lease_cam=false;
static unsigned override_kick_frame=0;
// D12A first-person quick kick (kick.inc).
static bool kick_request(bool queue=false);
// D11B: between Duke's actor draw entry and the next object-list step.
static bool duke_drawing,actor_drawing;
static bool jump_animation(unsigned animation) {
    return animation==96 || animation==97 || animation==98 || animation==103 ||
        animation==104 || animation==105 || animation==109;
}
static bool first_person_anchor(const double* forward,double* anchor);
static bool independent_camera() {
    const char* mode=std::getenv("DNTTK_CAMERA_MODE");
    return !mode || !std::strcmp(mode,"independent");
}
static double sensitivity() {
    const char* text=std::getenv("DNTTK_MOUSE_SENSITIVITY");
    char* end=nullptr; double value=text?std::strtod(text,&end):0.12;
    if (!std::isfinite(value) || value<0.01 || value>2 || (text && (!end || *end))) value=0.12;
    return value*tau/360;
}
static bool pivot(double* xyz) {
    if (psx_mod_read_word(camera+0x50)!=player+0x7bc ||
        psx_mod_read_word(camera+0x4c)!=player+0x7bc) return false;
    for (int i=0;i<3;++i) xyz[i]=(int32_t)psx_mod_read_word(player+0x7bc+4*i);
    // Original normal-camera target offset. The D10 shoulder offset is applied
    // to the camera position in orbit_constraint, never to this look target.
    xyz[1]+=(int32_t)psx_mod_read_word(camera+0x88);
    return true;
}
// D10 persistence. The launcher owns the profile; the runtime only reports the
// last Alt-wheel distance and shoulder side through a small side file it names.
static double camera_saved_distance() {
    const char* text=std::getenv("DNTTK_CAMERA_DISTANCE");
    char* end=nullptr;double value=text?std::strtod(text,&end):0;
    if(!text || !end || *end || !std::isfinite(value) || value<768 || value>6144)return 0;
    return value;
}
static void camera_persist(uint64_t sequence,int shoulder,int view) {
    const char* path=std::getenv("DNTTK_CAMERA_STATE_FILE");
    if(!path || !*path)return;
    if(written_distance<0)written_distance=camera_saved_distance();
    if(written_shoulder==2) {
        const char* side=std::getenv("DNTTK_CAMERA_SHOULDER");
        written_shoulder=side && !std::strcmp(side,"right")?1:side && !std::strcmp(side,"left")?-1:0;
    }
    if(written_view==2) {
        const char* text=std::getenv("DNTTK_CAMERA_VIEW");
        written_view=text && !std::strcmp(text,"first");
    }
    const double distance=saved_distance>0?saved_distance:0;
    if(distance==written_distance && shoulder==written_shoulder && view==written_view)return;
    // Half a second after the last change: one write per adjustment, not per notch.
    if(sequence<camera_pref_changed+30)return;
    std::string temporary=std::string(path)+".tmp";
    if(FILE* file=std::fopen(temporary.c_str(),"w")) {
        const bool ok=std::fprintf(file,"{\"camera_distance\": %.0f, \"shoulder\": \"%s\", \"view\": \"%s\"}\n",distance,
                                   shoulder>0?"right":shoulder<0?"left":"center",view?"first":"third")>0;
        if(std::fclose(file)==0 && ok && std::rename(temporary.c_str(),path)==0) {
            written_distance=distance;written_shoulder=shoulder;written_view=view;return;
        }
    }
    std::remove(temporary.c_str());
    // Do not retry every frame after a failure; the next change tries again.
    written_distance=distance;written_shoulder=shoulder;written_view=view;
    std::fprintf(stderr,"[TTK camera] could not save camera preferences to %s\n",path);
}
static bool climb_state_early();
static void orbit_begin(uint32_t sp) {
    const auto& f=input_snapshot(Context::Gameplay);
    double target[3];
    if (!independent_camera() || !pivot(target)) {orbit_valid=false;look.reset();return;}
    // A slow original game update is not a camera ownership transition.
    // The dancers legitimately take five guest fields; the old four-field
    // timeout reseeded from an older camera and discarded accumulated mouse
    // motion. Loads, input epochs and an explicit lease release already
    // invalidate orbit_valid. Preserve the look across variable game cadence.
    if (!orbit_valid || camera_epoch!=f.epoch) {
        if(std::getenv("DNTTK_CAMERA_RESEED_TRACE"))
            std::fprintf(stderr,"[camera reseed] gap=%llu valid=%d epoch=%llu/%llu yaw=%.6f field=%llu\n",
                (unsigned long long)(f.sequence-camera_frame),orbit_valid?1:0,
                (unsigned long long)camera_epoch,(unsigned long long)f.epoch,yaw,
                (unsigned long long)f.sequence);
        double x=target[0]-(int32_t)psx_mod_read_word(camera+0x64);
        double y=target[1]-(int32_t)psx_mod_read_word(camera+0x68);
        double z=target[2]-(int32_t)psx_mod_read_word(camera+0x6c);
        // The solved camera distance may be shortened by a wall, death or a
        // transition. It is never a replacement for our established follow boom.
        if(radius<256 || radius>8192) {
            // 28e1c builds the original unconstrained boom from this local offset.
            double ox=(int32_t)psx_mod_read_word(camera+0x94);
            double oy=(int32_t)psx_mod_read_word(camera+0x98)-(int32_t)psx_mod_read_word(camera+0x88);
            double oz=(int32_t)psx_mod_read_word(camera+0x9c);
            double desired=std::sqrt(ox*ox+oy*oy+oz*oz);
            radius=(psx_mod_read_word(camera+0x80)&1) && desired>=256 && desired<=8192
                ? desired : std::sqrt(x*x+y*y+z*z);
        }
        if (radius<256 || radius>8192) {orbit_valid=false;look.reset();return;}
        // Collision can put the boom to one side of the requested view. On
        // capture/state recovery seed from the visible orientation, not the boom.
        double fx=(int16_t)psx_mod_read_half(camera+12);
        double fy=(int16_t)psx_mod_read_half(camera+14);
        double fz=(int16_t)psx_mod_read_half(camera+16);
        yaw=std::atan2(fx,fz);pitch=clamp_pitch(std::atan2(fy,std::hypot(fx,fz)));
        look.reset();orbit_valid=true;recentering=false;
    }
    // The first seed is the original follow view; recenter returns to its pitch.
    if(!rest_pitch_valid){rest_pitch=pitch;rest_pitch_valid=true;}
    if(preferred_radius==0) {
        if(saved_distance<0)saved_distance=camera_saved_distance();
        // A saved Alt-wheel preference starts in place, without a zoom on entry.
        if(saved_distance>0)radius=preferred_radius=saved_distance;
        else preferred_radius=std::clamp(radius,768.0,6144.0);
    }
    if(distance_epoch!=f.epoch) {distance_epoch=f.epoch;distance_seen=0;recenter_seen=0;distance_frame=f.sequence;}
    if(f.distance_total!=distance_seen) {
        preferred_radius=std::clamp(preferred_radius-192.0*(f.distance_total-distance_seen),768.0,6144.0);
        saved_distance=preferred_radius;camera_pref_changed=f.sequence;
    }
    distance_seen=f.distance_total;
    const unsigned ticks=unsigned(std::min<uint64_t>(f.sequence-distance_frame,4));
    const double blend=1-std::pow(0.75,ticks);
    if(ticks) {
        // Host vblank sequence, once per update; no collision feedback into preference.
        radius+=(preferred_radius-radius)*blend;distance_frame=f.sequence;
        // Shoulder side eases across; 0.22 of the boom, bounded for close/far.
        // D08J3: climbs centre the view whatever the shoulder setting; the
        // preference itself is untouched and returns when the climb ends.
        const int shoulder=climb_state_early()?0:f.shoulder;
        const double side=shoulder*std::clamp(0.22*radius,192.0,640.0);
        shoulder_offset+=(side-shoulder_offset)*blend;
    }
    auto delta=look.consume(f.epoch,f.total_x,f.total_y);
    if(f.recenter_total>recenter_seen) recentering=true;
    recenter_seen=f.recenter_total;
    // Mouse movement always wins over a swing in progress.
    if(delta.x || delta.z) recentering=false;
    if(recentering && ticks) {
        // Game yaw: +Z=0, +X=1024. Behind Duke means looking along his heading.
        const double heading=(psx_mod_read_half(player+0x1c)&4095)*tau/4096;
        const double dy=wrap_yaw(heading-yaw),dp=rest_pitch-pitch;
        yaw=wrap_yaw(yaw+dy*blend);pitch=clamp_pitch(pitch+dp*blend);
        if(std::abs(dy)<0.005 && std::abs(dp)<0.005) {yaw=wrap_yaw(heading);pitch=clamp_pitch(rest_pitch);recentering=false;}
        if(!recentering)++recenters;
    }
    const char* inverted=std::getenv("DNTTK_MOUSE_INVERT_Y");
    yaw=wrap_yaw(yaw+delta.x*sensitivity());
    pitch=clamp_pitch(pitch+delta.z*sensitivity()*(inverted && !std::strcmp(inverted,"1")?-1:1));
    if (delta.x || delta.z) ++looks;
    if(f.shoulder!=seen_shoulder){seen_shoulder=f.shoulder;camera_pref_changed=f.sequence;}
    if(int(f.first_person)!=seen_view){seen_view=f.first_person;camera_pref_changed=f.sequence;}
    camera_persist(f.sequence,f.shoulder,f.first_person);
    orbit_sp=sp;
}
static void view_matrix(uint32_t destination) {
    double sy=std::sin(yaw),cy=std::cos(yaw),sp=std::sin(pitch),cp=std::cos(pitch);
    double matrix[]={cy,0,-sy,-sy*sp,cp,-cy*sp,sy*cp,sp,cy*cp};
    for(int i=0;i<9;++i) psx_mod_write_half(destination+2*i,(int16_t)std::lround(matrix[i]*4096));
}
static void orbit_constraint(uint32_t delta) {
    double target[3]; if (!pivot(target)) {orbit_valid=false;return;}
    double sy=std::sin(yaw),cy=std::cos(yaw),sp=std::sin(pitch),cp=std::cos(pitch);
    double forward[]={sy*cp,sp,cy*cp};
    // World-to-view rows: right, down, forward. Set before all original
    // constraint queries, final camera position, room and height calculations.
    view_matrix(camera);
    double eye[3];
    const bool eye_view=fp_blend>0 && first_person_anchor(forward,eye);
    for(int i=0;i<3;++i) {
        int32_t current=psx_mod_read_word(camera+0x64+4*i);
        // Shoulder moves the eye along the view's right row; the look target and
        // the original constraint/collision solve are unchanged.
        const double right[]={cy,0,-sy};
        double orbit=target[i]-forward[i]*radius+right[i]*shoulder_offset;
        // D11: blend the requested anchor toward the eye; collision still solves it.
        if(eye_view) orbit+=(eye[i]-orbit)*fp_blend;
        int32_t desired=std::lround(orbit);
        psx_mod_write_word(delta+4*i,desired-current);
    }
    ++orbits;
}
#include "apartment_secret.inc"
// Runtime RAM image and RAM-code generation (memory.c). The plugin is linked
// into the runtime; the native tests define both beside their RAM mock.
extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;
static uint64_t identity_calls, identity_checks, identity_ns;
static constexpr uint32_t level_base=0x800ca968;
static constexpr size_t level_count=sizeof level_overlays/sizeof level_overlays[0];
// D22A: the authenticated level overlay of the last identity verdict (its
// LEVELxx tag), or 0 when the verdict failed.
static thread_local uint32_t identity_tag;
static int level_index(uint32_t tag) {
    for(size_t i=0;i<level_count;++i)if(level_overlays[i].tag==tag)return int(i);
    return -1;
}
// Only LEVEL00 carries the Modernized apartment pair.
static uint32_t (*level_reader(uint32_t tag))(uint32_t) {
    return tag==3?apartment_identity_word:psx_mod_read_word;
}
static bool identity() {
    static thread_local std::array<std::vector<uint32_t>,sizeof guards/sizeof guards[0]> expected;
    static thread_local std::array<std::vector<uint32_t>,sizeof state_guards/sizeof state_guards[0]> state_expected;
    static thread_local std::array<std::vector<uint32_t>,1> level_expected[level_count];
    static thread_local IdentityMemo memo;
    static thread_local uint32_t memo_tag;
    ++identity_calls;
    const uint64_t frame=input_host_frame();
    if(!memo.valid(frame,g_dirty_ram_code_gen)) {
        const auto t0=std::chrono::steady_clock::now();
        const uint32_t tag=psx_mod_read_word(level_base);
        const int level=level_index(tag);
        const bool ok=level>=0 && code_identity(guards,expected,psx_mod_read_word,g_psx_ram) &&
            masked_identity(state_guards,state_expected) &&
            code_identity(level_overlays[level].body,level_expected[level],level_reader(tag),g_psx_ram);
        memo.set(frame,g_dirty_ram_code_gen,ok);memo_tag=ok?tag:0;
        identity_ns+=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-t0).count();
        ++identity_checks;
    }
    identity_tag=memo_tag;
    if(!memo.ok)++refusals;
    // Name the guard that broke, once per transition, so a lost lease in a
    // playtest log says which original code changed.
    static bool reported;
    if(!memo.ok && !reported) {
        uint32_t address=0,live=0,want=0;
        const uint32_t tag=psx_mod_read_word(level_base);
        const int level=level_index(tag);
        const int index=code_identity_mismatch(guards,expected,address,live,want);
        const int state=index<0?masked_identity_mismatch(state_guards,state_expected,address,live,want):-1;
        if(index>=0)
            std::fprintf(stderr,"[TTK identity] guard %d (0x%08x, %u bytes) changed at 0x%08x: 0x%08x, expected 0x%08x\n",
                index,guards[index].address,guards[index].size,address,live,want);
        else if(state>=0)
            std::fprintf(stderr,"[TTK identity] state table 0x%08x (%u bytes, mask 0x%08x) changed at 0x%08x: 0x%08x, expected 0x%08x\n",
                state_guards[state].address,state_guards[state].size,state_guards[state].mask,address,live,want);
        else if(level<0)
            std::fprintf(stderr,"[TTK identity] level overlay tag 0x%08x at 0x%08x is not an authenticated level\n",tag,level_base);
        else {
            code_identity_mismatch(level_overlays[level].body,level_expected[level],address,live,want,level_reader(tag));
            std::fprintf(stderr,"[TTK identity] level %u overlay changed at 0x%08x: 0x%08x, expected 0x%08x\n",
                tag,address,live,want);
        }
        reported=true;
    }
    if(memo.ok)reported=false;
    return memo.ok;
}
// LEVEL00-only conveniences (the apartment) require that exact overlay.
static bool first_map() { return identity() && identity_tag==3; }
static bool gameplay_context() {
    // 1bb84 dispatches mode 0 frontend/attract, mode 1 game. 268a0
    // suspends player updates for inventory/pause. A demo player is not input ownership.
    return psx_mod_read_half(0x800bcbb0)==1 && !psx_mod_read_half(0x800be568) &&
        !psx_mod_read_half(0x800d2540);
}
static const char* last_refusal="";
const char* lease_refusal_reason() { return last_refusal; }
static void lease_refuse_trace(const char* why, bool ident) {
    last_refusal=why;
    char now[96];
    unsigned animation=psx_mod_read_half(player+0x60);
    unsigned mode=psx_mod_read_byte(player+0x22c), previous=psx_mod_read_byte(player+0x22d);
    uint32_t flags=psx_mod_read_word(player+0x224);
    uint32_t owner=psx_mod_read_word(camera+0xa4);
    std::snprintf(now,sizeof now,"%s a=%u %u/%u f=%08x cam=%08x id=%d ctx=%d",
        why,animation,mode,previous,flags,owner,ident?1:0,gameplay_context()?1:0);
    static char last[96];
    if(!std::strcmp(now,last))return;
    std::snprintf(last,sizeof last,"%s",now);
    std::fprintf(stderr,"[TTK lease] inactive (%s) anim=%u state=%u/%u flags=%08x cam=%08x ident=%d ctx=%d\n",
        why,animation,mode,previous,flags,owner,ident?1:0,gameplay_context()?1:0);
}
static bool water_present_early() {
    return psx_mod_read_word(player+0x834)!=0x40000000u;
}
static bool water_submerged_early() {
    if(!water_present_early())return false;
    return (int32_t)psx_mod_read_word(player+8)>(int32_t)psx_mod_read_word(player+0x834);
}
static bool in_water_early() {
    return water_submerged_early() ||
        (water_present_early() && psx_mod_read_word(player+0x20c)!=0);
}
// Waist-deep / mid water is original land gait (8005201c early-outs below 0x281
// and swim states 4/5). Treat it as solid-ground locomotion, not a swim path.
static bool shallow_land_water_early() {
    if(!in_water_early())return false;
    const unsigned mode=psx_mod_read_byte(player+0x22c);
    if(mode==4 || mode==5)return false;
    const int32_t depth=(int32_t)psx_mod_read_word(player+0x1c8)-
        (int32_t)psx_mod_read_word(player+0x834);
    return depth<0x281;
}
static bool land_gait_anim(unsigned animation) {
    return animation==63 || (animation>=72 && animation<=79) ||
        (animation>=178 && animation<=184);
}
// D22B: the original's own directional gait, selected by its idle dispatcher
// (0x8004c1dc..0x8004c2d4) and strafe pads: 82/83 walk and 84/85 run
// backward, 88-91 strafe (86/87 and 92/93 in mid water). Modernized never
// asks for them, but a pad held as an original move ends (a fall, roll or
// slide bridge) starts them; the lease then takes over at once instead of
// leaving the original controls in charge.
static bool original_step_anim(unsigned animation) {
    return (animation>=82 && animation<=85) || (animation>=88 && animation<=91);
}
// Landing poses: 94/95 and the heavier 106 (BLOOD BATHS, after a fall).
static bool landing_pose_anim(unsigned animation) { return animation==94 || animation==95 || animation==106; }
static bool wade_land_anim(unsigned animation) {
    return land_gait_anim(animation) || animation==70 || animation==71 ||
        animation==94 || animation==95;
}
// D08Q: original jetpack flight. 8004aaf8 enters mode 10 / anim 163 on Square
// with the jetpack on (+0x358 & 3) and fuel; 8004ade0 owns thrust, fuel and
// the hover lock (+0x224 & 0x08000000). Anims 163..170 are its poses.
static bool jetpack_anim(unsigned animation) { return animation>=163 && animation<=170; }
static bool jetpack_flying_early() {
    return psx_mod_read_byte(player+0x22c)==10 && jetpack_anim(psx_mod_read_half(player+0x60)) &&
        (psx_mod_read_half(player+0x358)&3)==3;
}
// D08R: Modernized jetpack scheme, fixed at launch (DNTTK_JETPACK). Classic
// keeps the modern flight controls but the original burst physics (jetpack.inc).
static bool jetpack_classic() {
    static const bool classic=[] {const char* v=std::getenv("DNTTK_JETPACK");return v && !std::strcmp(v,"classic");}();
    return classic;
}
// After the jetpack cuts out in the air (fuel, J) the original falls with 108
// in mode 9; keep the mouse camera through that fall until the landing.
static bool jet_fall_grace;
static bool push_state_early();
// D08V: original mantle / hang / pull-up (mode 8 anims 134..142, hang mode 6
// 147..153, shimmy mode 7 149..153), including the entry frames whose previous
// mode is still ground, fall or hang. The original dispatchers own motion and
// facing; the host keeps only the mouse camera (and directional buttons).
static bool mantle_state_early() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c), previous=psx_mod_read_byte(player+0x22d);
    const bool from=previous==0 || previous==6 || previous==7 || previous==8 || previous==9;
    // D08U: a ladder's top exit 190 and bottom step-off 185 (mode 8 after the
    // climb, never the 185 mount from the ground) are the same kind of handoff.
    // D08J5: so are a pole or chain's top exit 196 and step-off 191.
    return (mode==8 && animation>=134 && animation<=142 && from) ||
        (mode==8 && (animation==185 || animation==190 || animation==191 || animation==196) && (previous==3 || previous==8)) ||
        (mode==6 && animation>=147 && animation<=153 && from) ||
        (mode==7 && animation>=149 && animation<=153 && (previous==6 || previous==7 || previous==9));
}
// D08V: an original fall that no owned jump started (107/108, mode 9: stepping
// off an edge the run-off lease does not own, a failed grab, a drop into water).
// Camera only; no air steering.
static bool unowned_fall_early() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c), previous=psx_mod_read_byte(player+0x22d);
    // An owned short fall (108) keeps its flight lease. Any other 107/108, even
    // after an owned jump, is camera only; the camera hook then drops the stale
    // jump ownership (flight_valid) as for any animation outside the flight set.
    const bool owned=animation==108 && flight_valid && short_fall;
    return (animation==107 || animation==108) && mode==9 && (previous==9 || previous==0) && !owned;
}
// D08U: the host-blended top-of-ladder mount (ladder_top.inc).
static bool ladder_mount_early();
// D22B: the original dodge rolls, seen armed in PIG FACTORY and THE REAPER:
// 157 and 160 launch in the air (mode 9), 158/159 and 161/162 tumble and
// recover on the ground. They were missing from the lease, so each roll
// dropped the mouse camera and first person for about a second. The original
// owns the roll's motion; the host keeps the mouse camera and the selected
// view and sends no directions (committed_camera_ready).
static bool roll_state_early() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c), previous=psx_mod_read_byte(player+0x22d);
    return animation>=157 && animation<=162 && (mode==0 || mode==9) && (previous==0 || previous==9);
}
// D22B: the original steep-slope slide. The airborne/directional handler
// 0x80055904 enters mode 2 (anim 143 or 145) when 0x800544e8 finds a slope too
// steep to stand on; Duke then leaves the slope (99 or 108, mode 9) and lands
// as 105. FAMILY JEWELS trace: jump 96, 145 (2/9), 99 (9/9), 105 (0/0), idle.
// The latch carries the camera through that landing only.
static bool slide_landing;
static bool slide_state_early() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c), previous=psx_mod_read_byte(player+0x22d);
    const bool sliding=mode==2 || (mode==9 && previous==2) || (animation==99 && mode==9 && previous==9);
    if(sliding) slide_landing=true;
    else if(!(animation==105 && mode==0 && (previous==0 || previous==9))) slide_landing=false;
    return sliding || slide_landing;
}
// Original moves that run to their end on their own: the host keeps the mouse
// camera and the selected view but sends no directions.
static bool committed_move_early() { return roll_state_early() || slide_state_early(); }
// D08J3: ladders, poles, chains and climbing walls (mode 3, the attached set
// traversal_state_ready accepts), including their entry frames from a mount,
// grab or hang. The original keeps the normal camera and Duke's pivot there and
// only swaps in a high look-down boom; the mouse orbit replaces that boom. The
// original climb handlers still own motion and facing.
static bool climb_state_early() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    const unsigned mode=psx_mod_read_byte(player+0x22c), previous=psx_mod_read_byte(player+0x22d);
    return mode==3 && ((animation>=147 && animation<=156) || (animation>=185 && animation<=211)) &&
        (previous==0 || previous==3 || previous==6 || previous==7 || previous==8 || previous==9);
}
static bool traversal_camera_early() { return mantle_state_early() || unowned_fall_early() || ladder_mount_early() || committed_move_early() || climb_state_early(); }
static bool state(bool camera_only=false) {
    if(!gameplay_context())return false;
    unsigned animation=psx_mod_read_half(player+0x60);
    // Guarded idle/locomotion dispatch; wall bumps and this jump lease permit camera only.
    // Death can retain state bytes 0/0 and the normal camera pointer.
    const bool preparing=camera_only && (animation==96 || animation==97) && takeoff_owned();
    const bool flight=camera_only && flight_valid && (animation==98 || animation==103 || animation==104 || animation==105 || animation==109 || (animation==108 && short_fall)) &&
        flight_epoch==input_snapshot(Context::Gameplay).epoch;
    // D08M: keep independent mouselook while swimming. Full swim motion uses a
    // parallel lease (swim.inc); this only widens the camera-only gate.
    const bool swimming=camera_only && in_water_early();
    // Crystal-2 wade: original plays tank turns 70/71 in this water. Those used
    // to fail state() and kill mouse/Shift. Waist-deep water keeps the land lease.
    const bool wade_land=!camera_only && shallow_land_water_early() && (wade_land_anim(animation) ||
        original_step_anim(animation) || animation==86 || animation==87 || animation==92 || animation==93);
    // D08Q: jetpack flight (mode 10) and the fall after it cuts out keep the
    // camera-only lease, like swimming; the original handler owns the motion.
    const bool jet=camera_only && (jetpack_flying_early() ||
        (jet_fall_grace && animation==108 && (psx_mod_read_byte(player+0x22c)==9 || psx_mod_read_byte(player+0x22c)==10)));
    // D11/D22B unowned jump: camera only; original takeoff, flight and landing.
    const unsigned mode=psx_mod_read_byte(player+0x22c), previous=psx_mod_read_byte(player+0x22d);
    const bool eye_air=camera_only && jump_camera && jump_animation(animation) &&
        (mode==0 || mode==9) && (previous==0 || previous==9);
    // D08T grab/push/pull 119..121: mouse look only; the original owns motion.
    const bool pushing=camera_only && push_state_early();
    // D12A: an original boot request (112..115 before its initializer) in the
    // eye view; the initializer hook turns it into the quick kick (kick.inc).
    const bool eye_kick=camera_only && fp_blend>=0.5 && animation>=112 && animation<=115 &&
        psx_mod_read_half(player+0x68)==0;
    // D08V: mantles, hangs, pull-ups and unowned falls keep the mouse camera, and
    // so does ordinary gait while the previous-mode byte still says mantle/fall
    // (the first frames after a pull-up or landing).
    const bool mantle=camera_only && traversal_camera_early();
    // D22B: also the landing poses themselves (an unleased landing frame left a
    // jump taken from it without camera continuity) and after a slide.
    const bool settle=camera_only && (land_gait_anim(animation) || original_step_anim(animation) ||
        landing_pose_anim(animation)) && mode==0 && (previous==8 || previous==9 || previous==2);
    if ((psx_mod_read_word(player)&0x20002) || !(land_gait_anim(animation) || original_step_anim(animation) || wade_land ||
        (camera_only && (animation==70 || animation==71 || animation==80 || animation==81 || landing_pose_anim(animation) ||
                         animation==175 || animation==176 || animation==177 || animation==181 ||
                         (animation>=178 && animation<=184))) || preparing || flight || swimming || jet || eye_air || pushing || eye_kick || mantle || settle)) return false;
    const uint32_t inhibit=shallow_land_water_early()?0x20000000u:0x20000200u;
    return psx_mod_read_word(camera+0xa4)==player && psx_mod_read_word(player+0x7d4)==camera
        && !(psx_mod_read_word(player+0x224)&inhibit)
        && (swimming || wade_land || jet || eye_air || mantle || settle || ((flight || (preparing && animation==97)) ? ((psx_mod_read_byte(player+0x22c)==9 && (psx_mod_read_byte(player+0x22d)==9 || psx_mod_read_byte(player+0x22d)==0)) ||
                      (psx_mod_read_byte(player+0x22c)==0 && psx_mod_read_byte(player+0x22d)==9))
                   : psx_mod_read_byte(player+0x22c)==0 && psx_mod_read_byte(player+0x22d)==0));
}
static bool eligible(bool camera_only=false) {
    return input_modernized() && input_snapshot(Context::Gameplay).active && state(camera_only);
}
static bool lease_ready(bool camera_only=false) {
    const auto& f=input_snapshot(Context::Gameplay);
    return eligible(camera_only) && camera_frame && f.epoch==camera_epoch &&
        f.sequence>=camera_frame && f.sequence-camera_frame<=4;
}
bool movement_ready() { return lease_ready() && identity(); }
// D08Y: the original run handler queued a jump for the coming edge (+0x228
// bit 4, see the 0x800780b4 hook) and keeps it only while the jump button stays
// held and Duke still runs (0x80055ea4). The modern jump is a press, so the host
// holds the button for that queue (bounded by pc_input to a recent press).
bool edge_jump_queued() {
    const unsigned animation=psx_mod_read_half(player+0x60);
    return (animation==76 || animation==78) && psx_mod_read_byte(player+0x22c)==0 &&
        (psx_mod_read_word(player+0x228)&4u) && movement_ready();
}
// Preserve held locomotion intent through owned jumps and wall bumps so the original
// landing selector can choose its running continuation instead of idle recovery.
// This does not authorize movement/facing writes or air steering.
// D08Q: jetpack flight and its cut-out fall are camera-only leases with their
// own pad bridge (jetpack.inc / input_pad); they are not ground locomotion.
static bool jetpack_camera_only() {
    return jetpack_flying_early() || (jet_fall_grace && psx_mod_read_half(player+0x60)==108 && (psx_mod_read_byte(player+0x22c)==9 || psx_mod_read_byte(player+0x22c)==10));
}
bool locomotion_input_ready() { return lease_ready(true) && identity() && !jetpack_camera_only() && !push_state_early() && !traversal_camera_early(); }
// D08V: the camera-only lease is live through a mantle/hang/pull-up or an
// unowned fall. Input keeps the original directional buttons there.
bool traversal_camera_ready() { return lease_ready(true) && identity() && traversal_camera_early(); }
// D22B: rolls and slides are committed; feeding A/D (S) as the original strafe
// (Down) pads at a roll's end started the original strafe 90/91 (back-step 84)
// outside the lease.
bool committed_camera_ready() { return traversal_camera_ready() && committed_move_early(); }
bool directional_takeoff_ready() {
    return takeoff_owned() && takeoff_direction && psx_mod_read_half(player+0x60)==96 && lease_ready(true) && identity();
}
bool short_fall_input_ready() { return short_fall && terrain_epoch==input_snapshot(Context::Gameplay).epoch && airborne_input_ready(); }
bool airborne_input_ready() {
    return lease_ready(true) && flight_valid && psx_mod_read_byte(player+0x22c)==9 && identity() && !jetpack_camera_only();
}
bool player_identity_ready() { return input_modernized() && gameplay_context() && psx_mod_read_word(camera+0xa4)==player && psx_mod_read_word(player+0x7d4)==camera && identity(); }
// The original attached-traversal dispatchers own motion, facing and camera.
// Supply directional buttons only; never rotate their animation displacement.
static bool traversal_state_ready() {
    if(!input_modernized() || !gameplay_context() || psx_mod_read_byte(player+0x3b8)!=0 ||
       (psx_mod_read_word(player)&0x20002) || (psx_mod_read_word(player+0x224)&0x20000000) ||
       psx_mod_read_word(camera+0xa4)!=player || psx_mod_read_word(player+0x7d4)!=camera)return false;
    unsigned mode=psx_mod_read_byte(player+0x22c), previous=psx_mod_read_byte(player+0x22d);
    unsigned animation=psx_mod_read_half(player+0x60);
    // Ladders need a settled mode; D08V mantles/hangs also accept their entry
    // frames (previous mode ground, fall or hang).
    bool attached=(mode==3 && mode==previous && ((animation>=147 && animation<=156) || (animation>=185 && animation<=211))) ||
                  mantle_state_early();
    return attached && identity();
}
bool traversal_input_ready() {
    return input_snapshot(Context::Gameplay).active && traversal_state_ready();
}
bool interaction_alive() {
    // Retain E-owned holster intent through pause/inventory/script suspension.
    // This predicate grants no input or draw permission: restore_ready still
    // requires active, settled normal gameplay. Death, frontend, foreign actor
    // or overlay loss must cancel it.
    return input_modernized() && psx_mod_read_half(0x800bcbb0)==1 &&
        psx_mod_read_word(camera+0xa4)==player && psx_mod_read_word(player+0x7d4)==camera &&
        !(psx_mod_read_word(player)&0x20002) && identity();
}
bool interaction_restore_ready() {
    // Only normal, settled ownership can draw: never the attached/mount/exit
    // states, an equipment transition, or an active upper-body interaction.
    return movement_ready() && psx_mod_read_byte(player+0x3b8)==0 &&
        !(psx_mod_read_word(player+0x224)&4) && psx_mod_read_half(player+0x74)==psx_mod_read_half(player+0x60);
}
// The game's tap-or-hold button (Circle by default: holster, draw, use) keeps
// a per-frame press history; 0x80040584 sums the frame times of consecutive
// presses and at 100 enters the hold-to-select inventory mode (+0x224 0x200),
// which stops Duke until the button's release edge. The history word is
// *(0x800d1a90 + 4 * player+0x233), the same one that routine reads. Bit 0 set:
// the game's latest frame saw the button pressed.
bool hold_button_sampled() {
    const uint32_t index=psx_mod_read_byte(player+0x233);
    if(index>=4)return false;
    const uint32_t history=psx_mod_read_word(0x800d1a90+4*index);
    if(history<0x80000000u || history>=0x80200000u)return false;
    return (psx_mod_read_word(history)&1)!=0;
}
bool fire_draw_ready() {
    // Boot (selected ID zero) must retain its unarmed attack. Use the same
    // settled ground gate as E restoration; no drawing on ladders or in menus.
    return interaction_restore_ready() && psx_mod_read_byte(player+0x3ba)!=0;
}
bool interaction_holster_ready() {
    unsigned upper=psx_mod_read_half(player+0x74);
    // Do not spend the one-shot stow request inside a manual weapon switch,
    // draw/reload or attack. The pending E lease waits for original stable idle.
    return locomotion_input_ready() && psx_mod_read_byte(player+0x3b8)==2 &&
        !(psx_mod_read_word(player+0x224)&4) &&
        (upper==5 || upper==20 || upper==29 || upper==35 || upper==39);
}
bool weapon_drawn() {
    return player_identity_ready() && psx_mod_read_byte(player+0x3b8)==2;
}
bool weapon_holstered() {
    return player_identity_ready() && psx_mod_read_byte(player+0x3b8)==0;
}
bool interaction_ready() {
    unsigned upper=psx_mod_read_half(player+0x74);
    // Temporary equipment0 during a switch is not a completed E stow.
    // Original557f8 uses this same upper-table bit to gate airborne reaching.
    return (locomotion_input_ready() || traversal_input_ready()) && psx_mod_read_byte(player+0x3b8)==0 &&
        !(psx_mod_read_word(player+0x224)&4) && upper<280 &&
        !(psx_mod_read_word(0x800c2824+4*upper)&8);
}
static double view_yaw() {
    return std::atan2((int16_t)psx_mod_read_half(camera+12),
                      (int16_t)psx_mod_read_half(camera+16));
}
// Both options must opt in: original camera/aim retain original facing.
bool view_aim_input_ready() {
    const char* mode=std::getenv("DNTTK_WEAPON_AIM");
    unsigned weapon=psx_mod_read_byte(player+0x3b9);
    unsigned equipment=psx_mod_read_byte(player+0x3b8);
    return independent_camera() && mode && !std::strcmp(mode,"view") &&
        (equipment==0 || (equipment==2 && view_weapon_supported(weapon))) &&
        (movement_ready() || locomotion_input_ready() || jetpack_input_ready() || swim_weapon_ready());
}
static bool presentation_ready() {
    return view_aim_input_ready() && !input_snapshot(Context::Gameplay).held[original_aim] &&
        !(psx_mod_read_word(player+0x224)&0x80);
}
#include "shortcuts.inc"
#include "steroids.inc"
#include "steroids_beat.inc"
#include "run_click.inc"
#include "pickup_select.inc"
#include "crouch.inc"
#include "manual_jump.inc"
#include "terrain.inc"
#include "cheats.inc"
#include "apartment_interaction.inc"
#include "push.inc"
#include "ladder_top.inc"
#include "pole_climb.inc"
#include "ledge_reach.inc"
#include "ceiling_hang.inc"
#include "widescreen.inc"
#include "gadget_hud.inc"
#include "draw_distance.inc"
#include "spawn.inc"
static void face_view() {
    // Game yaw: +Z=0, +X=1024. Do not copy view pitch into the body's Euler angles.
    int heading=static_cast<int>(std::lround(view_yaw()*4096/tau)) & 4095;
    psx_mod_write_half(player+0x1c,heading);
    psx_mod_write_half(player+0x24,heading);
    ++facings;
}
#include "swim.inc"
#include "jetpack.inc"
#include "first_person.inc"
#include "kick.inc"

static void hook_body(CPUState* cpu, uint32_t address);
// Frame diagnostics (DNTTK_FRAME_TRACE=1): wall time between player updates
// (one game logic frame) and the CPU time spent in these hooks during it, with
// the costliest hook addresses, for frames over 25 ms or with over 3 ms of hooks.
// D08Y: the runtime CPU overclock (PSX_CPU_OVERCLOCK) is leased from the
// player update (gameplay in a level, Modernized); it lapses on its own a few
// fields after the updates stop (boot, menus, movies, loading).
extern "C" void psx_overclock_renew(void);
// D23F: the optional fast CPU timing (fast_timing.c) is leased the same way.
extern "C" void ttk_fast_timing_renew(void);
// Safety net: if emulation falls behind real time while overclocked (under 57
// host frames per second over a second), pause the lease for five seconds so
// the overclock can never be what starves audio. Logged once per pause.
// D23E/D23H: each second goes to replay_load_window (frame_replay.cpp), which
// sheds high-refresh presents on a sustained deficit and steps back up when
// emulation keeps up; the lease pauses only when nothing is left to shed.
// Seconds that span a gap (menus, loads: 3 s or more between player updates)
// and seconds while the overclock is paused are not judged.
static void overclock_lease() {
    using clk=std::chrono::steady_clock;
    static clk::time_point window_start,paused_until;static uint64_t window_frame;static unsigned pauses;
    const auto now=clk::now();const uint64_t frame=input_host_frame();
    if(window_start.time_since_epoch().count()==0 || frame<window_frame) {window_start=now;window_frame=frame;}
    const double dt=std::chrono::duration<double>(now-window_start).count();
    if(dt>=1.0) {
        const double rate=(frame-window_frame)/dt;
        const int verdict=replay_load_window(dt>=3.0 || now<paused_until ? 2 : rate<57.0 ? 1 : 0);
        if(verdict>0) {
            if(++pauses<=20)std::fprintf(stderr,"[TTK cpu] emulation behind real time (%.1f frames/s): high-refresh presents reduced\n",rate);
        } else if(verdict<0) {
            paused_until=now+std::chrono::seconds(5);
            if(++pauses<=20)std::fprintf(stderr,"[TTK cpu] emulation behind real time (%.1f frames/s): CPU overclock paused 5 s\n",rate);
        }
        window_start=now;window_frame=frame;
    }
    if(now>=paused_until)psx_overclock_renew();
}
static long frame_us;static std::map<uint32_t,long> frame_per;
bool frame_trace_on() {static const bool on=std::getenv("DNTTK_FRAME_TRACE")!=nullptr;return on;}
void frame_trace_account(uint32_t address,long us) {frame_us+=us;frame_per[address]+=us;}
static void hook(CPUState* cpu, uint32_t address) {
    const bool update=address==0x8005a210 && cpu->gpr[4]==player;
    if(update && input_modernized()) {overclock_lease();ttk_fast_timing_renew();}
    const bool trace=frame_trace_on();
    if(!trace) {hook_body(cpu,address);return;}
    using clk=std::chrono::steady_clock;
    static clk::time_point last_update;
    if(update) {
        const auto now=clk::now();
        if(last_update.time_since_epoch().count()) {
            const long wall=(long)std::chrono::duration_cast<std::chrono::microseconds>(now-last_update).count();
            static const long min_wall=std::getenv("DNTTK_FRAME_TRACE_MS")?std::atol(std::getenv("DNTTK_FRAME_TRACE_MS"))*1000:25000;
            if(wall>min_wall || frame_us>3000) {
                std::fprintf(stderr,"frame-trace wall=%ld hooks=%ld anim=%u mode=%u",wall,frame_us,psx_mod_read_half(player+0x60),psx_mod_read_byte(player+0x22c));
                std::vector<std::pair<long,uint32_t>> top;for(auto& kv:frame_per)top.push_back({kv.second,kv.first});
                std::sort(top.rbegin(),top.rend());
                for(size_t i=0;i<top.size() && i<4;++i)std::fprintf(stderr," %08x=%ld",top[i].second,top[i].first);
                std::fputc('\n',stderr);
            }
        }
        last_update=now;frame_us=0;frame_per.clear();
    }
    const auto t0=clk::now();hook_body(cpu,address);
    const long us=(long)std::chrono::duration_cast<std::chrono::microseconds>(clk::now()-t0).count();
    frame_trace_account(address,us);
}

static void hook_body(CPUState* cpu, uint32_t address) {
    // D08O1: hidden fire comes back at the next player-update step, an
    // animation start or the camera update, whichever runs first. Routines
    // the swim handler calls before its fire test (0x80076330) must not.
    if(address==0x80055e80 || address==0x800493a4 || address==0x8003ade4) swim_fire_restore(address);
    // D08O2A: the swim arm aim lives only inside Duke's model build.
    if(address==0x80055e80 || address==0x800493a4 || address==0x8003ade4 ||
       address==0x800455bc || address==0x800411b8) swim_aim_restore();
    // D08A8: steroids hidden from the status bar's armour element come back
    // after it, whatever the mode is by then.
    if(address==0x8001fc44 || address==0x800b4d9c) steroids_hud_restore();
    if (!input_modernized()) return;
    const auto& f=input_snapshot(Context::Gameplay);
    uint32_t ra=cpu->gpr[31], sp=cpu->gpr[29];
    if (sp<0x80010100 || sp>0x801ffff0 || (sp&3)) {head_restore();return;}
    if(address==0x8006276c || address==0x80062b48 || address==0x8002ffec) {draw_distance_apply(address,ra);return;}
    if(address==0x80076330) {apartment_switch_query(cpu);return;}
    if(address==0x80081a48) {pickup_select_begin(cpu);steroids_pickup(cpu);apartment_pickup(cpu);return;}
    if(address==0x80051cf0) {push_obstacle_action(cpu,ra);return;}
    if(address==0x80051890) {push_idle_action(cpu,ra);return;}
    if(address==0x800455bc) {
        // D08O1: the ground weapon-selection point (0x80058120) is not reached
        // in the original swim states; their handler entry takes its place.
        if(cpu->gpr[4]==player && ra==0x8004b5bc && swim_weapon_ready()) select_weapon(cpu);
        swim_fire_hide(cpu,ra);
        return;
    }
    if(address==0x80055e80) return;
    if(address==0x800411b8) {swim_aim_begin(cpu,ra);return;}
    if(address==0x8002b9f4) {
        // The original level teardown destroys these records. Forget host
        // ownership before their addresses can be reused by another scene.
        bool reset_caller=false;
        for(uint32_t r:{0x8001b510u,0x8001b8a0u,0x800222a8u,0x80025474u,0x800270d0u,0x80028594u})reset_caller|=ra==r;
        if(cpu->gpr[4]==1 && reset_caller && psx_mod_read_word(ra-8)==0x0c00ae7d) {
            hidden_enemies.clear();monsters_hidden=false;hidden_table=hidden_count=0;
        }
        return;
    }
    if (address==0x800493a4) {
        if(std::getenv("DNTTK_TRAVERSAL_TRACE") && cpu->gpr[4]==player && psx_mod_read_half(player+0x60)==96)
            std::fprintf(stderr,"ttk-standing ra=%08x track=%u active=%d dir=%.2f/%.2f frame=%llu/%llu epoch=%llu/%llu state=%u/%u flags=%08x player=%08x identity=%d aim=%d crouch=%d\n",ra,psx_mod_read_half(player+0x68),f.active,f.move_x,f.move_y,(unsigned long long)f.sequence,(unsigned long long)camera_frame,(unsigned long long)f.epoch,(unsigned long long)camera_epoch,psx_mod_read_byte(player+0x22c),psx_mod_read_byte(player+0x22d),psx_mod_read_word(player+0x224),psx_mod_read_word(player),player_identity_ready(),f.held[original_aim],f.held[crouch]);
        kick_convert(cpu,ra);
        // Original lower-body initializer, after the game accepted jump from
        // normal ground. Keep its 96 preparation and clearance choice intact.
        if(cpu->gpr[4]!=player || (ra!=0x8005a3a0 && ra!=0x8005a4d0) || psx_mod_read_half(player+0x60)!=96 ||
           psx_mod_read_half(player+0x68)!=0 || !f.active || !independent_camera() ||
           !ground_frame || ground_epoch!=f.epoch || f.sequence<ground_frame || f.sequence-ground_frame>4 ||
           !player_identity_ready() || (psx_mod_read_word(player)&0x20002) ||
           psx_mod_read_byte(player+0x22c)!=0 || psx_mod_read_byte(player+0x22d)!=0 ||
           (psx_mod_read_word(player+0x224)&0x202002c1) || f.held[crouch] || f.held[original_aim])return;
        if(takeoff_owned())return;
        takeoff_intent=movement(view_yaw(),f.move_x,f.move_y);
        takeoff_epoch=f.epoch;takeoff_frame=f.sequence;takeoff_valid=true;takeoff_obstacle=false;takeoff_direction=f.move_x || f.move_y;
        if(takeoff_direction)++standing_takeoffs;
    } else if (address==0x8005a210) {
        if(cpu->gpr[4]==player && cpu->gpr[5]==0x800c2754 && ra==0x80041b34) {
            // Attached cameras remain original-owned. Resume capture through
            // the authenticated player update, without needing an active input
            // frame or a normal locomotion camera to bootstrap it.
            // Escape sets initial_capture and releases. Recapture must not wait
            // for 0x8003ade4 (absent on alternate camera) or for walk-only
            // state() (anim 70/71 in the turret wade are turns, not gaits).
            if(input_wants_initial_capture() && gameplay_context() && identity())
                input_offer_gameplay_capture();
            if(shallow_land_water() && f.active && !f.held[original_aim] && identity())
                face_view();
            weapon_update_sp=sp;weapon_transition_boost=false;quick_takeoff_boost=false;
            cheats_update(cpu);
            spawn_update(cpu);
            steroids_beat(cpu); // D08A10
            terrain_update(cpu);
            stance_request(cpu);
            swim_update(cpu,false);
            jetpack_update();
            ladder_top_update(cpu);
            pole_climb_update();
            ladder_leap_update();
            ledge_reach_mantle(cpu);
            ledge_reach_retry(cpu);
            ledge_step_up(cpu);
            mantle_glide();
            ledge_hang_settle();
            object_hang_update();
            ceiling_hang_update();
            unsigned animation=psx_mod_read_half(player+0x60);
            // Space may precede direction. An authenticated neutral preparation
            // keeps its input/camera lease, but supplies no Forward until the
            // first direction arrives. Never reset the original animation timer.
            if(animation==96 && takeoff_owned() && !takeoff_direction &&
               (f.move_x || f.move_y) && locomotion_input_ready() &&
               !(psx_mod_read_word(player+0x224)&0x202002c1) &&
               !f.held[crouch] && !f.held[original_aim]) {
                takeoff_intent=movement(view_yaw(),f.move_x,f.move_y);
                takeoff_direction=true;++standing_takeoffs;
            }
            // Walking's 53404 handler has no jump branch. Start the same
            // original 96 preparation on a fresh buffered press, before the
            // original animation initializer/events. Running selection stays original.
            const unsigned jet=psx_mod_read_half(player+0x358);
            const bool thrust=(jet&3)==3 && (int16_t)psx_mod_read_half(player+0x35a)>=
                (int32_t)psx_mod_read_word(0x800d21fc);
            if((animation==63 || (animation>=72 && animation<=75)) &&
               independent_camera() && movement_ready() && (f.move_x || f.move_y) &&
               !(psx_mod_read_word(player+0x224)&0x202002c1) && !thrust &&
               !f.held[crouch] && !f.held[original_aim] && input_take_jump()) {
                // Shallow-water standing jumps (no WASD) are handled in swim_try_wade_jump.
                takeoff_intent=movement(view_yaw(),f.move_x,f.move_y);
                takeoff_epoch=f.epoch;takeoff_frame=f.sequence;takeoff_valid=true;takeoff_obstacle=false;takeoff_direction=true;++standing_takeoffs;
                psx_mod_write_half(player+0x60,96);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
            }
        }
    } else if(address==0x80059db0) {
        stance_advance(cpu);
        manual_quick_takeoff(cpu,ra);
        // Only original upper-body equip/stow tracks, with their normal event
        // runner intact. Consume the extra time once per player update: the
        // original runner may call back repeatedly with a remaining delta.
        const unsigned upper=psx_mod_read_half(player+0x74);
        const bool transition=upper==6 || upper==7 || upper==13 || upper==21 ||
            upper==22 || upper==30 || upper==31 || upper==36 || upper==40;
        // E-owned stow must finish before original reach (upper-table bit8).
        // Boost on the ground as well as in flight so armed run→jump→E can
        // clear the weapon before contact and feel like the holstered grab.
        const bool e_stow=input_auto_stow_pending() &&
            (airborne_input_ready() || movement_ready() || locomotion_input_ready());
        // Two-handed stow finishes through original generic blend0/1 AFTER
        // equipment becomes zero. Its table bit8 still blocks the reach at558a0.
        // Keep the E-owned budget through that tail; never skip its events.
        const bool stow_tail=e_stow && psx_mod_read_byte(player+0x3b8)==0 && (upper==0 || upper==1);
        if(!weapon_transition_boost && (transition || stow_tail) && (movement_ready() || e_stow) &&
           cpu->gpr[4]==player+0x74 && cpu->gpr[5]==player && ra==0x8005a5a8 &&
           cpu->gpr[7]==1 && cpu->gpr[6]==weapon_update_sp-0x2e) {
            int delta=(int16_t)psx_mod_read_half(cpu->gpr[6]);
            if(delta>0 && delta<=4096) {
                // 4x finishes E stow inside the flight window without skipping
                // original upper events (8x left upper stuck on jump poses).
                psx_mod_write_half(cpu->gpr[6],e_stow?delta*4:delta+delta/2);weapon_transition_boost=true;
            }
        }
    } else if (address==0x80048410) {
        // Switch only the two ordinary walk/run gait pairs, at the original
        // locomotion dispatcher. Restart their original animation just as the
        // transition routine does; retain root delta and all collision handling.
        unsigned animation=psx_mod_read_half(player+0x60);
        const bool wade_turn=locomotion_input_ready() && (animation==70 || animation==71);
        if(cpu->gpr[4]!=player || ra!=0x8004b634 ||
           !(movement_ready() || wade_turn) ||
           f.held[original_aim] || f.held[jump] || !(f.move_x || f.move_y))return;
        step_dispatch=false;
        if(!input_running() && (animation==63 || animation==72 || animation==74 || animation==76 || animation==78))
            step_dispatch=terrain_step_probe(cpu);
        const bool run=input_running() || step_dispatch;
        if((animation==70 || animation==71) && (f.move_x || f.move_y)) {
            psx_mod_write_half(player+0x60,run?76:72);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
            ++speed_changes;
        }
        if(step_dispatch && animation==63) {
            psx_mod_write_half(player+0x60,76);psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
        }
        if((run && (animation==72 || animation==74)) || (!run && (animation==76 || animation==78))) {
            // Immediate frame-0 restart (original transition). Plant-aligned delay
            // felt laggy and still clunked; soft mid-stride phase reuse froze pose.
            // Shift-run plant SFX silencing is deferred low-priority (D27).
            psx_mod_write_half(player+0x60,run?animation+4:animation-4);
            psx_mod_write_half(player+0x68,0);psx_mod_write_half(player+0x6a,0);
            ++speed_changes;
        }
    } else if (address==0x8003ebf4) {
        if(std::getenv("DNTTK_TRAVERSAL_TRACE") && cpu->gpr[4]==player)
            std::fprintf(stderr,"ttk-jump ra=%08x anim=%u state=%u/%u flags=%08x had=%d gap=%llu epoch=%llu/%llu\n",ra,psx_mod_read_half(player+0x60),psx_mod_read_byte(player+0x22c),psx_mod_read_byte(player+0x22d),psx_mod_read_word(player+0x224),had_movement,(unsigned long long)(f.sequence-movement_frame),(unsigned long long)f.epoch,(unsigned long long)intent_epoch);

        // Directional-jump ballistic update: initial takeoff is 9/0 before 9/9.
        // Latch the last supported movement
        // direction once, before original gravity, swept collision and integration.
        // Subsequent impacts retain their own velocity; no air steering/reinjection.
        // D08Z: the E reach 109 keeps the same ballistic update; only the
        // manual style's air steering acts on it.
        const bool reach_steer=psx_mod_read_half(player+0x60)==109 && manual_jump();
        if (cpu->gpr[4]!=player || ra!=0x80055934 || !f.active ||
            (psx_mod_read_half(player+0x60)!=98 && psx_mod_read_half(player+0x60)!=103 && psx_mod_read_half(player+0x60)!=104 &&
             !reach_steer && !(psx_mod_read_half(player+0x60)==108 && short_fall && terrain_epoch==f.epoch)) ||
            (psx_mod_read_word(player)&0x20002) ||
            (psx_mod_read_word(player+0x224)&0x20000200) ||
            psx_mod_read_byte(player+0x22c)!=9 ||
            (psx_mod_read_byte(player+0x22d)!=9 && psx_mod_read_byte(player+0x22d)!=0) ||
            psx_mod_read_word(camera+0xa4)!=player || psx_mod_read_word(player+0x7d4)!=camera ||
            !identity()) return;
        if(reach_steer) {manual_air_control(f);return;}
        if(psx_mod_read_half(player+0x60)==108) {
            if(fall_velocity_pending) {
                auto v=redirect((int32_t)psx_mod_read_word(player+0x1f4),
                                (int32_t)psx_mod_read_word(player+0x1fc),flight_intent);
                const double magnitude=std::hypot(v.x,v.z);
                double speed=fall_speed>0?fall_speed:(fall_walking?std::min(magnitude,2048.0):magnitude);
                // D08L: never above the running jump (10085) or walking's 2048.
                // The stride estimate can spike to its 14000 clamp at a run
                // start; a ledge departure must not outreach the original jump.
                speed=std::min(speed,fall_walking?2048.0:10000.0);
                if(magnitude>0) {v.x*=speed/magnitude;v.z*=speed/magnitude;}
                psx_mod_write_word(player+0x1f4,(int32_t)std::lround(v.x));
                psx_mod_write_word(player+0x1fc,(int32_t)std::lround(v.z));
                fall_velocity_pending=false;
                manual_air_seed();
            }
            manual_air_control(f);
            return;
        }
        if(delayed_takeoff && terrain_epoch==f.epoch) {
            psx_mod_write_word(player+0x1f8,delayed_vertical);delayed_takeoff=false;
        }
        swim_ledge_ballistic(f);
        if(flight_valid && flight_epoch!=f.epoch) flight_valid=false;
        const bool standing=psx_mod_read_half(player+0x60)==98;
        const bool fresh=standing ? takeoff_owned() && takeoff_direction : had_movement && intent_epoch==f.epoch &&
            f.sequence>=movement_frame && f.sequence-movement_frame<=4;
        // Own the flight lease even if Mouse2 is still held. View-aim clears
        // original_aim only while movement_ready; once airborne that clears
        // and the physical button returns, which used to skip flight_valid and
        // block E-owned airborne stow/ladder grabs. Keep aim's original jump
        // arc: only redirect ballistic velocity when aim is not held.
        ledge_reach_arm(f);
        if(!flight_valid && fresh) {
            input_take_jump(); // A committed launch cannot leave a second buffered press.
            flight_intent=standing?takeoff_intent:intent;flight_epoch=f.epoch;flight_valid=true;
            if(!f.held[original_aim]) {
                auto v=redirect((int32_t)psx_mod_read_word(player+0x1f4),
                                (int32_t)psx_mod_read_word(player+0x1fc),flight_intent);
                psx_mod_write_word(player+0x1f4,(int32_t)std::lround(v.x));
                psx_mod_write_word(player+0x1fc,(int32_t)std::lround(v.z));++jumps;
            }
            manual_air_seed();
        } else manual_air_control(f);
    } else if (address==0x8002a7fc) {
        min_boom_restore();
        // Normal camera's final orientation conversion after position collision.
        // Supply the requested view matrix to its original quaternion/smoothing
        // path, so a shortened/sideways boom cannot pin look-at yaw to the wall.
        if (!orbit_valid || !lease_ready(true) || !identity() ||
            cpu->gpr[18]!=camera || sp!=orbit_sp-0xb8-0xa8) return;
        const bool smooth=ra==0x80029220 && cpu->gpr[4]==sp+0x18 && cpu->gpr[5]==sp+0x68;
        const bool direct=ra==0x8002920c && cpu->gpr[4]==camera && cpu->gpr[5]==camera+0x54;
        if (!smooth && !direct) return;
        view_matrix(cpu->gpr[4]);++orientations;
    } else if (address==0x80058120) {
        // Immediately after original heading update; before original aim/model work.
        if (cpu->gpr[4]==player && ra==0x80041c44) {
            god_mode_update(); // after this update's mode-10 fuel drain, before the HUD
            jet_post_update();
            select_weapon(cpu);
            kick_update(cpu);
            if(presentation_ready())face_view();
        }
    } else if (address==0x80097a44) {
        // Existing upper-body aim path only. Replace its direction argument,
        // never persistent auto-aim state, model flags, or body pitch.
        if (ra!=0x80097f28 || cpu->gpr[18]!=player || cpu->gpr[5]!=player+0x124 ||
            cpu->gpr[4]<0x1f800000 || cpu->gpr[4]>0x1f8003e0 || (cpu->gpr[4]&31) ||
            psx_mod_read_byte(player+0x3b8)!=2 || !presentation_ready()) return;
        if (!presentation_vector) presentation_vector=psx_mod_alloc_guest_memory(16,16);
        if (!presentation_vector) return;
        for(int i=0;i<3;++i) psx_mod_write_word(presentation_vector+4*i,
            (int32_t)(int16_t)psx_mod_read_half(camera+12+2*i));
        cpu->gpr[5]=presentation_vector;
        ++arms;
    } else if (address==0x8002a038) {
        min_boom_restore();
        first_person_follow(cpu,ra,sp);
    } else if (address==0x8008ba30) {
        widescreen_hud_begin(ra);
        gadget_hud(cpu);
    } else if (address==0x8001fc44) {
        widescreen_hud_restore();
    } else if (address==0x800b4d9c) {
        widescreen_hud_restore();
        widescreen_view_rect(ra);
        first_person_projection(cpu,ra);
    } else if (address==0x800348d8) {
        // Original actor draw (a0 camera, a1 actor) from the object-list loop.
        actor_drawing=true;
        duke_drawing=cpu->gpr[4]==camera && cpu->gpr[5]==player && ra==0x8003769c;
        weapon_draw_bounds(duke_drawing && fp_blend>0 && identity(),false);
        {
            // D17: the live evaluation is kept for render replay workers;
            // lease_ready() itself must not be called again (state() has effects).
            const bool lease_cam=render_override ? override_lease_cam : lease_ready(true);
            if(!render_override && duke_drawing) last_lease_cam=lease_cam;
            first_person_draw(duke_drawing && identity(),psx_mod_read_word(camera+0xa4)==player && lease_cam);
        }
    } else if (address==0x8001ca4c) {
        // Next object-list step after Duke's draw returns: the head joint flag is cleared.
        duke_drawing=false;actor_drawing=false;
        if(ra==0x8007fe78) pickup_select_end(); // D08A9: after the pickup dispatcher
        if(ra==0x800376b4) {head_restore();weapon_draw_bounds(false,true);}
    } else if (address==0x800292a0) {
        first_person_weapon_joint(cpu,ra);
    } else if (address==0x80033e40 || address==0x800341e4 || address==0x80033f5c) {
        first_person_weapon_attached(cpu,address,ra);
    } else if (address==0x8003ade4) {
        min_boom_restore();
        // After a savestate load the camera starts from the loaded game's
        // view: keeping the previous yaw turned Duke away from his saved
        // facing within a few frames (2026-10-03).
        {
            static uint32_t loads_seen=psx_mod_savestate_loads();
            const uint32_t loads=psx_mod_savestate_loads();
            if(loads!=loads_seen) {
                loads_seen=loads;orbit_valid=false;look.reset();camera_frame=0;walking=false;
                input_state_loaded();
            }
        }
        if (cpu->gpr[4]!=camera || cpu->gpr[5]!=player || ra!=0x80025ee8) return;
        const bool ident=identity();
        if(input_wants_initial_capture() && ident && gameplay_context() &&
           (state() || state(true) || swim_lease_ready())) input_offer_gameplay_capture();
        unsigned animation=psx_mod_read_half(player+0x60);
        if(!takeoff_owned() || (animation!=96 && animation!=97 && animation!=98)) {takeoff_valid=false;takeoff_direction=false;}
        // Continue the camera only from a live lease on the previous camera update.
        if(!jump_animation(animation)) jump_camera=false;
        else if(!jump_camera && orbit_valid && camera_frame && camera_epoch==f.epoch &&
                f.sequence>=camera_frame && f.sequence-camera_frame<=4) jump_camera=true;
        if (!eligible(true) || !ident) {
            lease_refuse_trace(!gameplay_context()?"context":!ident?"identity":!f.active?"released":"state",ident);
            distance_seen=f.distance_total;distance_epoch=f.epoch;camera_frame=0;walking=false;orbit_valid=false;look.reset();
            first_person_release("lease");return;
        }
        if (flight_epoch!=f.epoch || (psx_mod_read_half(player+0x60)!=98 && psx_mod_read_half(player+0x60)!=103 && psx_mod_read_half(player+0x60)!=104 && psx_mod_read_half(player+0x60)!=105 && psx_mod_read_half(player+0x60)!=109 && !(short_fall && psx_mod_read_half(player+0x60)==108))) flight_valid=false;
        if(first_map())apartment_patch_install();
        orbit_begin(sp);
        if(!orbit_valid) first_person_release("orbit");
        else {
            const unsigned ticks=fp_frame && f.sequence>fp_frame ? unsigned(std::min<uint64_t>(f.sequence-fp_frame,4)) : 1;
            first_person_update(f,ticks);fp_frame=f.sequence;
        }
        if(shallow_land_water_early() && !f.held[original_aim])face_view();
        camera_frame=f.sequence; camera_epoch=f.epoch; ++cameras;
        if(swim_lease_ready()) {
            swim_frame=f.sequence;swim_epoch=f.epoch;
            swim_update(cpu,true);
        }
        if(state() && (animation==63 || (animation>=72 && animation<=79))) {
            ground_frame=f.sequence;ground_epoch=f.epoch;
        }
        // A short run-up can enter jump before a root-motion callback. Keep the
        // verified normal-state input intent fresh independently of gait phase.
        if(state() && (f.move_x || f.move_y) && !f.held[original_aim]) {
            intent=movement(view_yaw(),f.move_x,f.move_y);had_movement=true;
            intent_epoch=f.epoch;movement_frame=f.sequence;
        }
        if (psx_mod_read_half(player+0x60)==63 && !f.move_x && !f.move_y) had_movement=false;
    } else if (address==0x8003aa48) {
        if (!orbit_valid || !lease_ready(true) || !identity() || cpu->gpr[4]!=camera || ra!=0x8003aeb0 ||
            sp!=orbit_sp-0xb8 || sp<0x80010000 || sp>0x801fffdc || (sp&3) ||
            cpu->gpr[5]!=sp+0x18) {min_boom_restore();return;}
        first_person_constraint();
        orbit_constraint(cpu->gpr[5]);
    } else if(address==0x80054308) {
        walking=false;
        if(cpu->gpr[4]!=player || (ra!=0x80048a68 && ra!=0x80048ab0) || !movement_ready())return;
        if(f.move_x || f.move_y) {
            intent=movement(view_yaw(),f.move_x,f.move_y);
            auto delta=redirect((int16_t)psx_mod_read_half(player+0xfc),(int16_t)psx_mod_read_half(player+0x100),intent);
            psx_mod_write_half(player+0xfc,(int16_t)std::lround(delta.x));psx_mod_write_half(player+0x100,(int16_t)std::lround(delta.z));
            walking_sp=sp;walking_size=0x20;probe_return=0x8005432c;walking=true;
        }
    } else if (address==0x800539f8) {
        // Mid-depth wade handler (anims 80/81, mode 1), entered from the gait
        // dispatcher's wade cases only. The track advance has already written
        // this update's world-space root; the handler's own 800788e0 collision
        // and position add still follow. Retarget that root to camera-relative
        // WASD at the land pace instead of the clip's forward-only ~22 units.
        // No direction held: keep the original displacement and stop.
        if(cpu->gpr[4]!=player || (ra!=0x8004870c && ra!=0x8004873c && ra!=0x8004876c && ra!=0x8004879c))return;
        if(!mid_wade_input_ready() || (!f.move_x && !f.move_y) || f.held[original_aim])return;
        const auto dir=movement(view_yaw(),f.move_x,f.move_y);
        double length=std::hypot((int16_t)psx_mod_read_half(player+0xfc),
                                 (int16_t)psx_mod_read_half(player+0x100));
        // The wade clip's stride is uneven (6..44 per tick, ~25 average) and,
        // unlike the land handlers, not time-scaled. Keep its rhythm, but at
        // the land run's per-tick band (its 80..120 floor/ceiling).
        const bool run=input_running();
        length=std::clamp(length*(run?mid_wade_run_gain:mid_wade_walk_gain),
                          run?mid_wade_run_floor:mid_wade_walk_floor,
                          run?mid_wade_run_ceiling:mid_wade_walk_ceiling);
        psx_mod_write_half(player+0xfc,(int16_t)std::lround(dir.x*length));
        psx_mod_write_half(player+0x100,(int16_t)std::lround(dir.z*length));
        ++moves;
    } else if (address==0x80053500 || address==0x80053404 || address==0x80054218) {
        walking=false;
        bool run=address==0x80053500, stopping=address==0x80054218;
        if (stopping && ((ra!=0x800487c4 && ra!=0x80048810) || cpu->gpr[5]!=63 || cpu->gpr[6]!=4)) return;
        if (cpu->gpr[4]!=player || (!stopping && (run ? (ra!=0x80048664 && ra!=0x800486a8) : (ra!=0x80048620 && ra!=0x800485dc)))
            || !movement_ready()) return;
        // Original 52298/4bc68 set bit 8 on run entry; 539b4 rejects jump
        // until the stride completes at 4b938. A fresh bounded press may remove
        // that animation delay. The original run handler still owns its floor,
        // obstruction, jetpack and takeoff checks; never clear other flags.
        if(run && input_jump_pending() && independent_camera() &&
           (f.move_x || f.move_y) && !f.held[crouch] && !f.held[original_aim] &&
           !(psx_mod_read_word(player+0x224)&0x202002c1))
            psx_mod_write_word(player+0x228,psx_mod_read_word(player+0x228)&~8u);
        if (intent_epoch!=f.epoch) {had_movement=false;intent_epoch=f.epoch;}
        if (f.move_x || f.move_y) {intent=movement(view_yaw(),f.move_x,f.move_y);had_movement=true;movement_frame=f.sequence;}
        else if (!had_movement) return;
        // Continue the original animation's deceleration in the last requested
        // direction. Never revert its tail displacement to actor facing.
        walking_size=run?0x30:0x20; probe_return=stopping?0x80054240:(run?0x80053548:0x80053424);
        double length=std::hypot((int16_t)psx_mod_read_half(player+0xfc),
                                 (int16_t)psx_mod_read_half(player+0x100));
        if(stopping) {
            // Shorten the animation-driven braking tail; original collision still commits it.
            length*=0.35;gait_frame=0;
        } else if(f.move_x || f.move_y) {
            if(!gait_frame || gait_epoch!=f.epoch || f.sequence-gait_frame>4 || gait_running!=run)
                gait_length=std::max(length,run?80.0:20.0);
            else if(f.sequence!=gait_frame) gait_length=(gait_length+length)*0.5;
            else gait_length=length; // multiple original updates, never replay a cached displacement
            gait_frame=f.sequence;gait_epoch=f.epoch;gait_running=run;length=gait_length;
        }
        const unsigned dt=psx_mod_read_half(player+0xba);
        if(!stopping && (f.move_x || f.move_y) && dt>0 && dt<=64) {
            if(landing_epoch==f.epoch && f.sequence<=landing_until && fall_speed>0) {
                // The original landing starts with a running stride even for
                // walking. Bridge its first few updates at the owned approach
                // speed; no position write and no extension after release.
                length=fall_speed*dt/1024.0;gait_length=length;
            } else if(!step_dispatch && !small_landing && run==input_running()) {
                stride_speed=std::clamp(length*1024.0/dt,0.0,run?14000.0:4096.0);
                stride_epoch=f.epoch;
            }
        }
        if(step_dispatch && !input_running()) {
            length=std::min(length,20.0);gait_length=length;
        }
        Vec2 delta{length*intent.x,length*intent.z};
        psx_mod_write_half(player+0xfc,(int16_t)std::lround(delta.x));
        psx_mod_write_half(player+0x100,(int16_t)std::lround(delta.z));
        walking_sp=sp; walking=true; ++moves;
    } else if (address==0x800780b4) {
        // Standing preparation's original 375-unit clearance query. Rotate
        // vector AND endpoint; original room/headroom/wall tests still decide.
        if(cpu->gpr[4]==player && ra==0x8007985c && cpu->gpr[5]==sp+0x10 &&
           psx_mod_read_word(sp+0x44)==0x800549c4 && directional_takeoff_ready()) {
            auto v=redirect((int32_t)psx_mod_read_word(sp+0x10),
                            (int32_t)psx_mod_read_word(sp+0x18),takeoff_intent);
            int32_t x=std::lround(v.x),z=std::lround(v.z);
            psx_mod_write_word(sp+0x10,x);psx_mod_write_word(sp+0x18,z);
            psx_mod_write_word(sp+0x30,psx_mod_read_word(player+4)+x);
            psx_mod_write_word(sp+0x38,psx_mod_read_word(player+12)+z);
            return;
        }
        // The original run-jump helper looks 1024 units ahead and postpones
        // takeoff until the edge. Modern jump means jump now. Probe the current
        // support instead; original jump selection, gravity and swept collision
        // still run. This caller is not the actual movement collision query.
        // D08Y: except before a real gap. The levels are built around that
        // postponement (Duke leaps from the lip: the slot-5 gap is ~200 units
        // too long for a takeoff where Space was pressed). When the original
        // floor probe at the helper's 1024-ahead endpoint (sp+0x20) finds a
        // large drop (over 768, D08L's run-off threshold), leave the helper
        // alone so the original queues the jump (+0x228 bit 4) and launches
        // it at the edge. Small drops (furniture, steps) still jump now.
        if(cpu->gpr[4]==player && ra==0x80078c78 && cpu->gpr[5]==sp+0x10 &&
           psx_mod_read_word(sp+0x44)==0x8005398c && movement_ready() &&
           !f.held[original_aim] && (f.move_x || f.move_y) && input_running()) {
            // D08Z: the manual jump style leaves on the press before gaps too.
            if(!manual_jump()) {
                uint32_t probe[0x60/4];
                for(unsigned i=0;i<0x60/4;++i)probe[i]=psx_mod_read_word(player+0x174+4*i);
                int32_t ignored=0;
                const bool probed=!psx_mod_read_word(player+0x174) && original_call(cpu,0x8007765c,sp+0x20,&ignored);
                const int32_t ahead=(int32_t)psx_mod_read_word(player+0x1c4);
                for(unsigned i=0;i<0x60/4;++i)psx_mod_write_word(player+0x174+4*i,probe[i]);
                if(probed && ahead>768) {++edge_postpones;return;}
            }
            for(int i=0;i<3;++i) {
                psx_mod_write_word(sp+0x10+4*i,0);
                psx_mod_write_word(sp+0x20+4*i,psx_mod_read_word(player+4+4*i));
            }
            return;
        }
        // 7926c's world probe is fully rotated at this point. Its local endpoint
        // is already calculated, so update BOTH vector and endpoint before the
        // original world query; preserve probe length, vertical component/mode.
        if (cpu->gpr[4]!=player || ra!=0x800794b8 || cpu->gpr[5]!=sp+0x10 ||
            !movement_ready()) return;
        const bool starting=(psx_mod_read_word(sp+0x54)==0x80052228 || psx_mod_read_word(sp+0x54)==0x80052d34) &&
            (psx_mod_read_half(player+0x60)==63 || psx_mod_read_half(player+0x60)==178) && (f.move_x || f.move_y) &&
            !f.held[original_aim];
        // Idle's forward clearance test runs BEFORE locomotion is selected.
        // It must test requested WASD direction too, otherwise a wall in front
        // prevents starting a strafe/backpedal although that path is clear.
        if (starting) intent=movement(view_yaw(),f.move_x,f.move_y);
        else if (!walking || sp!=walking_sp-walking_size-0x58 ||
                 psx_mod_read_word(sp+0x54)!=probe_return) return;
        auto v=redirect((int32_t)psx_mod_read_word(sp+0x10),
                        (int32_t)psx_mod_read_word(sp+0x18),intent);
        int32_t x=std::lround(v.x),z=std::lround(v.z);
        psx_mod_write_word(sp+0x10,x); psx_mod_write_word(sp+0x18,z);
        psx_mod_write_word(sp+0x30,psx_mod_read_word(player+4)+x);
        psx_mod_write_word(sp+0x38,psx_mod_read_word(player+12)+z);
        walking=false; ++probes;
    }
}
// D17 render replay: the host state the draw-time hooks read, captured with each
// published frame and loaded in the worker process that redraws it (a worker
// holds only a fork-time copy of this file's statics).
struct RenderState {
    double fp_blend; bool orbit_valid, eye_offset_valid, live_valid, lease_cam;
    double eye_offset[3], live_q[4], live_rel[3], live_torso_q[4], live_torso[3], kick;
    FirePose fire_poses[32];
    uint32_t weapon_matrix, hidden_matrix;
    bool kick_active; unsigned kick_frame; uint32_t kick_point, leg_matrices;
};
bool replay_gameplay_context() { return gameplay_context(); }
void render_state_prepare() {
    // These matrices are completely populated by their draw hooks before use.
    // Reserve the host pointers before forking so first exposure does not
    // synchronously replace every worker during active camera movement.
    if(!weapon_matrix) weapon_matrix=psx_mod_alloc_guest_memory(0x20,16);
    if(!hidden_matrix) hidden_matrix=psx_mod_alloc_guest_memory(0x20,16);
    if(!kick_point) kick_point=psx_mod_alloc_guest_memory(16,16);
    if(!leg_matrices) leg_matrices=psx_mod_alloc_guest_memory(0x20*leg_joints,16);
    if(!presentation_vector) presentation_vector=psx_mod_alloc_guest_memory(16,16);
    if(!ignored_pickup) ignored_pickup=psx_mod_alloc_guest_memory(0x40,16);
}
bool late_view(double& out_yaw,double& out_pitch,double at_ms) {
    // Only while the modern camera owns the view (the live frame's lease at
    // Duke's draw) and follows Duke: a game-owned camera (new-game opening,
    // scripted views, climbs) must never get the mouse direction.
    if(!input_modernized() || !orbit_valid || recentering || !independent_camera() || !last_lease_cam ||
       psx_mod_read_word(camera+0xa4)!=player) return false;
    uint64_t live_epoch=0; double x=0,y=0;
    if(!input_live_look(live_epoch,x,y,at_ms) || !look.valid || look.epoch!=live_epoch) return false;
    const char* inverted=std::getenv("DNTTK_MOUSE_INVERT_Y");
    out_yaw=wrap_yaw(yaw+(x-look.x)*sensitivity());
    out_pitch=clamp_pitch(pitch+(y-look.y)*sensitivity()*(inverted && !std::strcmp(inverted,"1")?-1:1));
    return true;
}
bool late_pivot(double* xyz) {return pivot(xyz);}
double late_fp_blend() {return fp_blend;}
size_t render_state_size() {return sizeof(RenderState);}
void render_state_save(void* out) {
    RenderState r{};
    r.fp_blend=fp_blend; r.orbit_valid=orbit_valid; r.eye_offset_valid=eye_offset_valid; r.live_valid=live_valid;
    r.lease_cam=last_lease_cam;
    std::memcpy(r.eye_offset,eye_offset,sizeof eye_offset); std::memcpy(r.live_q,live_q,sizeof live_q);
    std::memcpy(r.live_rel,live_rel,sizeof live_rel); std::memcpy(r.live_torso_q,live_torso_q,sizeof live_torso_q);
    std::memcpy(r.live_torso,live_torso,sizeof live_torso); r.kick=kick;
    std::memcpy(r.fire_poses,fire_poses,sizeof fire_poses);
    r.weapon_matrix=weapon_matrix; r.hidden_matrix=hidden_matrix;
    r.kick_active=kick_active; r.kick_frame=kick_active ? kick_frame() : 0u;
    r.kick_point=kick_point; r.leg_matrices=leg_matrices;
    std::memcpy(out,&r,sizeof r);
}
void render_state_load(const void* in) {
    RenderState r;
    std::memcpy(&r,in,sizeof r);
    fp_blend=r.fp_blend; orbit_valid=r.orbit_valid; eye_offset_valid=r.eye_offset_valid; live_valid=r.live_valid;
    std::memcpy(eye_offset,r.eye_offset,sizeof eye_offset); std::memcpy(live_q,r.live_q,sizeof live_q);
    std::memcpy(live_rel,r.live_rel,sizeof live_rel); std::memcpy(live_torso_q,r.live_torso_q,sizeof live_torso_q);
    std::memcpy(live_torso,r.live_torso,sizeof live_torso); kick=r.kick;
    std::memcpy(fire_poses,r.fire_poses,sizeof fire_poses);
    weapon_matrix=r.weapon_matrix; hidden_matrix=r.hidden_matrix;
    kick_active=r.kick_active; override_kick_frame=r.kick_frame;
    kick_point=r.kick_point; leg_matrices=r.leg_matrices;
    // Draw-time transients start clean, as at the start of a live frame.
    head_flag=0; duke_drawing=false; actor_drawing=false; weapon_duke_draw=false; weapon_viewmodel=false;
    render_override=true; override_lease_cam=r.lease_cam;
}
bool first_person_view_live() {return input_modernized() && fp_blend>0 && orbit_valid;}
bool first_person_duke_drawing() {return duke_drawing;}
bool first_person_actor_drawing() {return actor_drawing;}
bool first_person_weapon_drawing() {return weapon_viewmodel && weapon_duke_draw && first_person_view_live();}
const char* controls_debug_json() {
    static char buffer[8192];
    std::snprintf(buffer,sizeof buffer,"{\"edge_jumps\":%u,\"step_drops\":%u,\"run_offs\":%u,\"delayed_jumps\":%u,\"standing_takeoffs\":%u,\"swim_mantles\":%u,\"wade_jumps\":%u,\"swim_dives\":%u,\"swim_fire_masks\":%u,\"swim_fire_restores\":%u,\"swim_aims\":%u,\"swim_aim_restores\":%u,\"swim_arm_aims\":%u,\"swim_redraws\":%u,\"ledge_assists\":%u,\"jet_updates\":%u,\"jet_hovers\":%u,\"jet_descents\":%u,\"jet_hold_captures\":%u,\"jet_root_nudges\":%u,\"jet\":%s,\"jet_scheme\":\"%s\",\"jet_classic\":%s,\"jet_classic_updates\":%u,\"push_masks\":%u,\"push_idle_masks\":%u,\"push_grab\":%s,\"ledge_reach_arms\":%u,\"air_mantles\":%u,\"hang_releases\":%u,\"lift_catches\":%u,\"turn_catches\":%u,\"drop_mantles\":%u,\"last_drop_rel\":%d,\"step_ups\":%u,\"last_step_rel\":%d,\"edge_postpones\":%u,\"jump_style\":\"%s\",\"air_steers\":%u,\"coyote_jumps\":%u,\"quick_takeoffs\":%u,\"air_cap\":%.0f,\"hang_settles\":%u,\"ceiling\":{\"ready\":%s,\"updates\":%u,\"turns\":%u,\"advances\":%u,\"drops\":%u},\"ladder_top\":{\"available\":%s,\"active\":%s,\"starts\":%u,\"mounts\":%u,\"aborts\":%u,\"leaps\":%u,\"leap_nudges\":%u,\"end_probes\":%u,\"end_hits\":%u,\"end_stops\":%u,\"mount_finishes\":%u,\"actor_exits\":%u,\"actor_exit_checks\":%u},\"pole\":{\"mount_starts\":%u,\"mounts\":%u,\"mount_aborts\":%u,\"floor_rejects\":%u,\"stall_drops\":%u,\"mounting\":%s},\"identity_calls\":%llu,\"identity_checks\":%llu,\"identity_us\":%llu,\"selections\":%llu,\"jumps\":%llu,\"speed_changes\":%llu,\"moves\":%llu,\"probes\":%llu,\"cameras\":%llu,\"refusals\":%llu,\"ready\":%s,\"swim\":%s,\"looks\":%llu,\"orbits\":%llu,\"yaw\":%.6f,\"pitch\":%.6f,\"radius\":%.3f,\"preferred_radius\":%.3f,\"shoulder_offset\":%.3f,\"recentering\":%s,\"recenters\":%llu,\"rest_pitch\":%.6f,\"selected_item\":%u,\"last_weapon\":%u,\"crouch_entries\":%u,\"crouch_exits\":%u,\"crouch_blocked\":%u,\"facings\":%llu,\"arms\":%llu,\"orientations\":%llu,\"fp\":{\"requested\":%s,\"supported\":%s,\"blend\":%.3f,\"reason\":\"%s\",\"updates\":%llu,\"fallbacks\":%llu,\"follows\":%llu,\"head_hides\":%llu,\"head_reclaims\":%llu,\"head_flag\":%u,\"min_boom\":%u,\"projection\":%d,\"projections\":%llu,\"near\":%s,\"weapon\":{\"enabled\":%s,\"hands\":%llu,\"draws\":%llu,\"flashes\":%llu,\"items\":%llu,\"readies\":%llu,\"pose\":\"%s\",\"recorded\":%llu,\"arm_hides\":%llu,\"fade_clears\":%llu,\"matrix\":%u},\"kick\":%s},\"aim\":%s,\"draw_distance\":%s,\"steroids\":%s,\"steroid_beat\":%s,\"pickup_select\":%s,\"run_click\":%s}",
        edge_jumps,step_drops,run_offs,delayed_jumps,standing_takeoffs,swim_mantles,wade_jumps,swim_dives,swim_fire_masks,swim_fire_restores,swim_aims,swim_aim_restores,swim_arm_aims,swim_redraws,ledge_assists,jet_updates,jet_hover_engages,jet_descents,jet_hold_captures,jet_root_nudges,jetpack_input_ready()?"true":"false",jetpack_classic()?"classic":"modern",jetpack_classic_input_ready()?"true":"false",jet_classic_updates,push_masks,push_idle_masks,push_grab_ready()?"true":"false",ledge_reach_arms,air_mantles,hang_releases,lift_catches,turn_catches,drop_mantles,last_drop_rel,step_ups,last_step_rel,edge_postpones,manual_jump()?"manual":"assisted",air_steers,coyote_jumps,quick_takeoffs,air_cap,hang_settles,ceiling_hang_ready()?"true":"false",ceiling_updates,ceiling_turns,ceiling_advances,ceiling_drops,ladder_top_available()?"true":"false",mount.active?"true":"false",ladder_mount_starts,ladder_mounts,ladder_mount_aborts,ladder_leaps,ladder_leap_nudges,ladder_end_probes,ladder_end_hits,ladder_end_stops,ladder_mount_finishes,ladder_actor_exits,ladder_actor_checks,pole_mount_starts,pole_mounts,pole_mount_aborts,pole_floor_rejects,pole_stall_drops,pole_mount.active?"true":"false",(unsigned long long)identity_calls,(unsigned long long)identity_checks,(unsigned long long)(identity_ns/1000),(unsigned long long)selections,(unsigned long long)jumps,(unsigned long long)speed_changes,(unsigned long long)moves,(unsigned long long)probes,(unsigned long long)cameras,
        (unsigned long long)refusals,lease_ready()?"true":"false",swim_lease_ready()?"true":"false",(unsigned long long)looks,(unsigned long long)orbits,yaw,pitch,radius,preferred_radius,shoulder_offset,recentering?"true":"false",(unsigned long long)recenters,rest_pitch,selected_item,last_equipped,crouch_entries,crouch_exits,crouch_blocked,(unsigned long long)facings,(unsigned long long)arms,(unsigned long long)orientations,
        input_snapshot(Context::Gameplay).first_person?"true":"false",fp_supported?"true":"false",fp_blend,fp_reason,(unsigned long long)fp_updates,(unsigned long long)fp_fallbacks,(unsigned long long)fp_follows,(unsigned long long)head_hides,(unsigned long long)head_reclaims,head_flag,psx_mod_read_word(min_boom_word),fp_blend>0?last_projection:(int)(int16_t)psx_mod_read_half(camera+0x42),(unsigned long long)projections,near_clip_debug_json(),weapon_view_enabled()?"true":"false",(unsigned long long)weapon_hands,(unsigned long long)weapon_draws,(unsigned long long)weapon_flashes,(unsigned long long)weapon_items,(unsigned long long)weapon_readies,weapon_ready_pose()?"ready":"animated",(unsigned long long)poses_recorded,(unsigned long long)arm_hides,(unsigned long long)fade_clears,weapon_matrix,kick_debug_json(),aim_debug_json(),draw_distance_debug_json(),steroids_debug_json(),steroids_beat_debug_json(),pickup_select_debug_json(),run_click_debug_json());
    return buffer;
}
}
PSX_MOD_CONSTRUCTOR(register_ttk_controls) {
    psx_mod_register_activation_plugin("ttk.widescreen",ttk::widescreen_activate);
    for (uint32_t address: {0x80076330u,0x80081a48u,0x8002b9f4u,0x800493a4u,0x80054308u,0x8005a210u,0x80059db0u,0x80048410u,0x8003ebf4u,0x8002a7fcu,0x80058120u,0x80097a44u,0x80054218u,0x80053404u,0x80053500u,0x800539f8u,0x800780b4u,0x8003ade4u,0x8003aa48u,0x800348d8u,0x8001ca4cu,0x8002a038u,0x800b4d9cu,0x800292a0u,0x80033e40u,0x800341e4u,0x80033f5cu,0x80051cf0u,0x80051890u,0x8008ba30u,0x8001fc44u,0x8006276cu,0x80062b48u,0x8002ffecu,0x800455bcu,0x80055e80u,0x800411b8u})
        psx_mod_register_function_entry_plugin("ttk.modern.controls",address,ttk::hook);
    // D08O2A: a shared matrix routine; its own hook keeps the per-joint cost to a few compares.
    psx_mod_register_function_entry_plugin("ttk.modern.controls",0x800b42ec,[](CPUState* cpu,uint32_t){ttk::swim_aim_arm(cpu);});
    // D08J5: a shared bone lookup; only the pole/chain down probe's call (ra 0x8007d7fc) goes further.
    psx_mod_register_function_entry_plugin("ttk.modern.controls",0x8003964c,[](CPUState* cpu,uint32_t){if(cpu->gpr[31]==0x8007d7fcu)ttk::pole_probe_floor(cpu);});
    // D08A4: the sound routine; only the pickup tail's call (ra 0x800828d8) goes further.
    psx_mod_register_function_entry_plugin("ttk.modern.controls",0x8006b73c,[](CPUState* cpu,uint32_t){ttk::sound_logged(cpu);if(cpu->gpr[31]==0x800828d8u)ttk::steroids_pickup_sound(cpu);});
    // D27A: the player update; only Duke's call goes further.
    psx_mod_register_function_entry_plugin("ttk.modern.controls",0x800412a4,[](CPUState* cpu,uint32_t){if(cpu->gpr[4]==ttk::player)ttk::run_click_presync();});
    // D12C: the damage sphere; only the original kick's call (ra 0x80049098) goes further.
    psx_mod_register_function_entry_plugin("ttk.modern.controls",0x800a979c,[](CPUState* cpu,uint32_t){if(cpu->gpr[31]==0x80049098u)ttk::kick_sphere_entry(cpu);});
    // D08A8: the composition's call after the status bar when 0x8001fc44 is skipped.
    psx_mod_register_function_entry_plugin("ttk.modern.controls",0x8002e850,[](CPUState* cpu,uint32_t){if(cpu->gpr[31]==0x800265d4u)ttk::steroids_hud_restore();});
}
