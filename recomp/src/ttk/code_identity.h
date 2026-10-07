#pragma once
#include "mod_plugins.h"
#include "psx_sha256.h"
#include <array>
#include <vector>
#include <cstdio>
#include <cstring>

namespace ttk {
// Authenticate each expected byte string once (SHA-256 against the guard
// digest), then compare the live bytes on every verification. This caches
// reference bytes, never permission.
//
// Two live-compare paths:
//  * fast: one memcmp per range straight against host RAM (`ram`, the
//    runtime's 2 MB image; guest KSEG0/KUSEG addresses fold with & 0x1fffff).
//    ~10 µs for the 81 KB controls guard set.
//  * exact: the per-word `read_word` walk. `read_word` may substitute words
//    (the Modernized apartment patch), so a memcmp mismatch falls back to it
//    before a range is judged foreign. This was the only path until D23A;
//    at 108 µs a call and ~46 calls per frame it cost 5 ms of every 16.7 ms
//    frame (documentation/58-modernized-frame-budget.md).
template<class Guard, size_t N>
bool code_identity(const Guard (&ranges)[N], std::array<std::vector<uint32_t>,N>& expected,
                   uint32_t (*read_word)(uint32_t)=psx_mod_read_word, const uint8_t* ram=nullptr) {
    for(size_t j=0;j<N;++j) {
        const auto& g=ranges[j];
        if ((g.address&3) || (g.size&3)) return false;
        auto& words=expected[j];
        if(words.empty()) {
            std::vector<uint8_t> bytes(g.size);
            for(uint32_t i=0;i<g.size;i+=4) {
                const auto word=read_word(g.address+i);
                for(unsigned b=0;b<4;++b)bytes[i+b]=uint8_t(word>>(8*b));
            }
            uint8_t digest[32];char hex[65];
            psx_sha256_compute(bytes.data(),bytes.size(),digest);
            for(int i=0;i<32;++i)std::snprintf(hex+2*i,3,"%02x",digest[i]);
            if(std::strcmp(hex,g.digest))return false;
            words.resize(g.size/4);
            for(size_t i=0;i<words.size();++i)
                words[i]=uint32_t(bytes[4*i]) | uint32_t(bytes[4*i+1])<<8 |
                         uint32_t(bytes[4*i+2])<<16 | uint32_t(bytes[4*i+3])<<24;
        }
        // Little-endian host: the packed words are the guest byte image.
        if(ram && (g.address&0x1fffffu)+g.size<=0x200000u &&
           std::memcmp(ram+(g.address&0x1fffffu),words.data(),g.size)==0)continue;
        for(size_t i=0;i<words.size();++i)
            if(read_word(g.address+4*i)!=words[i])return false;
    }
    return true;
}

// Diagnostic only: the first guard whose live bytes differ from its cached
// reference (call after a failed code_identity). Returns -1 if none differ.
template<class Guard, size_t N>
int code_identity_mismatch(const Guard (&ranges)[N], const std::array<std::vector<uint32_t>,N>& expected,
                           uint32_t& address,uint32_t& live,uint32_t& want,
                           uint32_t (*read_word)(uint32_t)=psx_mod_read_word) {
    for(size_t j=0;j<N;++j) {
        if(expected[j].empty())return int(j);
        for(size_t i=0;i<expected[j].size();++i) {
            const uint32_t a=ranges[j].address+4*uint32_t(i),w=read_word(a);
            if(w!=expected[j][i]) {address=a;live=w;want=expected[j][i];return int(j);}
        }
    }
    return -1;
}

// D08V1: original data tables that the game itself rewrites in place. A
// MaskedGuard {address, size, mask, digest} authenticates and compares each
// word & mask, so the bits the original writes at run time are state, never
// a code change. The digest covers the masked words (little-endian).
template<class Guard, size_t N>
bool masked_identity(const Guard (&ranges)[N], std::array<std::vector<uint32_t>,N>& expected,
                     uint32_t (*read_word)(uint32_t)=psx_mod_read_word) {
    for(size_t j=0;j<N;++j) {
        const auto& g=ranges[j];
        if ((g.address&3) || (g.size&3)) return false;
        auto& words=expected[j];
        if(words.empty()) {
            std::vector<uint8_t> bytes(g.size);
            for(uint32_t i=0;i<g.size;i+=4) {
                const auto word=read_word(g.address+i)&g.mask;
                for(unsigned b=0;b<4;++b)bytes[i+b]=uint8_t(word>>(8*b));
            }
            uint8_t digest[32];char hex[65];
            psx_sha256_compute(bytes.data(),bytes.size(),digest);
            for(int i=0;i<32;++i)std::snprintf(hex+2*i,3,"%02x",digest[i]);
            if(std::strcmp(hex,g.digest))return false;
            words.resize(g.size/4);
            for(size_t i=0;i<words.size();++i)
                words[i]=uint32_t(bytes[4*i]) | uint32_t(bytes[4*i+1])<<8 |
                         uint32_t(bytes[4*i+2])<<16 | uint32_t(bytes[4*i+3])<<24;
        }
        for(size_t i=0;i<words.size();++i)
            if((read_word(g.address+4*i)&g.mask)!=words[i])return false;
    }
    return true;
}
template<class Guard, size_t N>
int masked_identity_mismatch(const Guard (&ranges)[N], const std::array<std::vector<uint32_t>,N>& expected,
                             uint32_t& address,uint32_t& live,uint32_t& want,
                             uint32_t (*read_word)(uint32_t)=psx_mod_read_word) {
    for(size_t j=0;j<N;++j) {
        if(expected[j].empty())return int(j);
        for(size_t i=0;i<expected[j].size();++i) {
            const uint32_t a=ranges[j].address+4*uint32_t(i),w=read_word(a);
            if((w&ranges[j].mask)!=expected[j][i]) {address=a;live=w;want=expected[j][i];return int(j);}
        }
    }
    return -1;
}

// Per-frame verdict memo. A verdict is reused only while (a) the host frame
// is the same one it was computed in and (b) the runtime's RAM-code
// generation (memory.c g_dirty_ram_code_gen: CD/EXE loads marking executable
// ranges, clean->dirty page transitions, save-state restores, boot) has not
// moved. So an overlay load between two hooks in the same frame still
// invalidates immediately; only a rewrite inside an already-dirty page is
// deferred to the next frame's full compare.
struct IdentityMemo {
    uint64_t frame=~0ull;uint32_t generation=0;bool ok=false;
    bool valid(uint64_t now_frame,uint32_t now_generation) const {
        return frame==now_frame && generation==now_generation;
    }
    void set(uint64_t now_frame,uint32_t now_generation,bool verdict) {
        frame=now_frame;generation=now_generation;ok=verdict;
    }
};
}
