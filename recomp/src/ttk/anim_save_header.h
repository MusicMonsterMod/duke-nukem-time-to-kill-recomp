#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace ttk::anim_save {
constexpr uint32_t header_bytes=0x220, buffer_bytes=320*1024;
struct Record {uint16_t clip,first,count;};
struct Header {
    uint32_t frm,rot,frames,commands,count;
    std::array<Record,64> records{};
};
// Validate all recovery metadata before allowing a single guest write. Existing
// TTKA v1 saves remain readable; absent/cleared/foreign buffers are never guessed.
inline bool decode(const uint8_t* p,size_t size,uint32_t saved_base,
                   uint32_t live_frm,uint32_t live_rot,Header& out) {
    if(size<header_bytes || saved_base<0x80f00000u ||
       saved_base>0x81000000u-buffer_bytes) return false;
    auto u16=[&](size_t n){return uint16_t(p[n])|uint16_t(p[n+1])<<8;};
    auto u32=[&](size_t n){return uint32_t(u16(n))|uint32_t(u16(n+2))<<16;};
    if(u32(0)!=0x414b5454 || u32(4)!=1) return false;
    Header h{};h.frm=u32(8);h.rot=u32(12);h.frames=u32(16);
    h.commands=u32(20);h.count=u32(24);
    const uint32_t extra=u32(28);
    auto ram_range=[](uint32_t a,uint32_t bytes) {
        return !(a&3) && a>=0x80010000 && a<0x80200000 && bytes<=0x80200000-a;
    };
    if(!h.frames || h.frames>16384 || !h.commands || h.commands>=65536 ||
       extra>65535-h.frames || h.count>64 ||
       !ram_range(h.frm,8*h.frames) || !ram_range(h.rot,8*h.commands) ||
       uint64_t(header_bytes)+8ull*(h.frames+extra+h.commands)>buffer_bytes ||
       live_frm!=saved_base+header_bytes ||
       live_rot!=live_frm+8*(h.frames+extra)) return false;
    std::array<bool,4096> seen{};
    for(uint32_t i=0;i<h.count;++i) {
        const size_t n=0x20+8*i;
        Record r{uint16_t(u16(n)),uint16_t(u16(n+2)),uint16_t(u16(n+4))};
        const unsigned clip=r.clip&0x7fff;
        if(clip>=2048 || !r.count || u16(n+6)) return false;
        // Extra host IDs and original clip indices are separate namespaces.
        const unsigned key=clip+((r.clip&0x8000)?2048:0);
        if(seen[key]) return false;
        seen[key]=true;
        const uint32_t limit=(r.clip&0x8000)?h.frames+extra:h.frames;
        if(uint32_t(r.first)+r.count>limit || ((r.clip&0x8000) && r.first<h.frames)) return false;
        h.records[i]=r;
    }
    out=h;return true;
}
}
