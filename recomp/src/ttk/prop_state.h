// D54: is a static prop still standing? (documentation/437-d54-broken-lights.md)
//
// Our lights that belong to a prop (D41D1 sign lamps, D41L woodburners and
// chandeliers, D53 screens and the power sign) go out when the prop breaks.
// How TTK breaks a prop (CONFIRMED, level 5 traffic lights type 217 and fire
// barrel type 220, UI slot 6): at the hit that takes its health (+0x32, from
// the type table's fourth word) to 0, the object's word +0x00 gains bit 2;
// a little later the object is unlinked from its room's object lists (+0x48,
// +0x4c). Its memory (type, place, mesh) is left as it was.
//
// A prop stands while it is linked in a room list, is a static prop (kind
// +0x14 = 1), has bit 2 of +0x00 clear, and still has the type and place it
// was found with (the memory of an unlinked object can be reused). Lights are
// bound by type and place, never by address alone, so a savestate taken
// after the break keeps the light out and one taken before brings it back.
//
// Header-only (inline), so test binaries that link modern_controls.cpp or
// near_clip.cpp alone need nothing more. Game thread (the snapshot is per
// thread in any case).
#pragma once
#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "mod_plugins.h"

namespace ttk {
struct PropRef {uint32_t o=0;uint16_t type=0;int32_t p[3]={0,0,0};};
struct PropSnapshot {
    std::chrono::steady_clock::time_point taken{};
    bool ever=false;
    std::unordered_map<uint32_t,bool> linked;      // object -> standing (linked, kind 1, not broken)
    std::vector<PropRef> standing;                 // linked static props that stand
    std::unordered_set<uint64_t> standing_keys;    // prop_key of each
    bool valid=false;                              // the room table looked sane and held props
    uint64_t level_sig=0;                          // the room table's fingerprint (rooms and their origins)
};
inline thread_local PropSnapshot prop_snapshot;

inline PropRef prop_ref(uint32_t o) {
    PropRef r;r.o=o;r.type=(uint16_t)psx_mod_read_half(o+0x2c);
    for(int k=0;k<3;++k) r.p[k]=(int32_t)psx_mod_read_word(o+4+4*k);
    return r;
}
inline uint64_t prop_key(uint16_t type,const int32_t p[3]) {
    uint64_t k=type;
    for(int i=0;i<3;++i) {k^=(uint64_t)(uint32_t)p[i]+0x9e3779b97f4a7c15ull+(k<<6)+(k>>2);k*=0xff51afd7ed558ccdull;}
    return k;
}
inline bool prop_broken_flag(uint32_t o) {return (psx_mod_read_word(o)&4u)!=0;}

// The snapshot, rebuilt when older than 5 ms (callers in one game tick
// share it) or when fresh is asked for (editor commands).
inline const PropSnapshot& prop_snapshot_take(bool fresh=false) {
    PropSnapshot& s=prop_snapshot;
    const auto now=std::chrono::steady_clock::now();
    if(s.ever && !fresh && now-s.taken<std::chrono::milliseconds(5)) return s;
    s.taken=now;s.ever=true;s.linked.clear();s.standing.clear();s.standing_keys.clear();s.valid=false;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return s;
    s.level_sig=rooms;
    for(uint32_t r=0;r<rooms;++r) for(int k=0;k<3;++k) s.level_sig=(s.level_sig^psx_mod_read_word(table+88*r+4*k))*0x100000001b3ull;
    for(uint32_t r=0;r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
        uint32_t node=psx_mod_read_word(table+88*r+head_at);
        for(unsigned guard=0;node && guard<1024;++guard) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            if(o<0x80010000u || o>0x801fff00u || (o&3) || s.linked.count(o)) continue;
            const bool stands=psx_mod_read_byte(o+0x14)==1 && !prop_broken_flag(o);
            s.linked[o]=stands;
            if(stands) {s.standing.push_back(prop_ref(o));s.standing_keys.insert(prop_key(s.standing.back().type,s.standing.back().p));}
        }
    }
    s.valid=!s.standing.empty();
    return s;
}
// Lights found on one level compare against snapshots of that level only:
// at a level change the next level's props are in place a moment before the
// lights are reloaded, and none of the old props would be found. sig is the
// fingerprint the lights were found with (0: take this snapshot's).
inline bool prop_snapshot_usable(const PropSnapshot& s,uint64_t& sig) {
    if(!s.valid) return false;
    if(!sig) sig=s.level_sig;
    return s.level_sig==sig;
}
// The prop found as ref still stands.
inline bool prop_standing(const PropSnapshot& s,const PropRef& ref) {
    auto it=s.linked.find(ref.o);
    if(it==s.linked.end() || !it->second) return false;
    const PropRef now=prop_ref(ref.o);
    return now.type==ref.type && now.p[0]==ref.p[0] && now.p[1]==ref.p[1] && now.p[2]==ref.p[2];
}
// A standing prop of this type at exactly this place (address may differ).
inline bool prop_standing_at(const PropSnapshot& s,uint16_t type,const int32_t p[3]) {
    return s.standing_keys.count(prop_key(type,p))!=0;
}

// A light dying with its prop: a few quick sputters fading out over
// prop_dying_ticks game ticks (30 Hz), then dark. t counts from the break.
constexpr uint32_t prop_dying_ticks=12;
inline float prop_dying_level(uint32_t t,uint32_t seed) {
    if(t>=prop_dying_ticks) return 0.0f;
    const float fade=1.0f-(float)t/prop_dying_ticks;
    uint32_t h=(t+seed*0x9e3779b9u)*2654435761u;h^=h>>15;
    const bool on=t<2 || ((h>>8)&3)!=0;            // first two ticks lit, then three in four
    return on ? fade*fade : 0.08f*fade;
}
}  // namespace ttk
