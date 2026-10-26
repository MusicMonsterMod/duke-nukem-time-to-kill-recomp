#include "anim_save_header.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <utility>
using namespace ttk::anim_save;
int main() {
    std::array<uint8_t,header_bytes> p{};
    auto put=[&](unsigned o,uint32_t v){for(unsigned i=0;i<4;++i)p[o+i]=v>>(8*i);};
    const uint32_t b=0x80f80010,frm=b+header_bytes,rot=frm+8*110;
    put(0,0x414b5454);put(4,1);put(8,0x80100000);put(12,0x80110000);
    put(16,100);put(20,200);put(24,2);put(28,10);
    put(32,63|(10u<<16));put(36,5);
    put(40,0x8001|(100u<<16));put(44,10);
    Header h{};auto ok=[&]{return decode(p.data(),p.size(),b,frm,rot,h);};
    assert(ok() && h.frm==0x80100000 && h.records[0].first==10);
    const auto valid=p;
    // Each malformed field must fail without publishing a partially decoded result.
    for(auto pair: {std::pair<unsigned,uint32_t>{0,0x00ffffff},{4,2},{8,0x80f00000},
                  {12,0x801ffffc},{16,0xffffffff},{20,65536},{24,65},{28,0xffffffff},
                  {32,2048},{36,0},{36,0x10005},{40,63|(100u<<16)},{44,11}}) {
        p=valid;put(pair.first,pair.second);h.frm=123;
        assert(!ok() && h.frm==123);
    }
    p=valid;put(40,0x803f|(100u<<16));assert(ok());
    p=valid;
    assert(!decode(p.data(),20,b,frm,rot,h));
    assert(!decode(p.data(),p.size(),b,frm,rot+8,h));
    assert(!decode(p.data(),p.size(),0x80ffff00,frm,rot,h));
    puts("animation save metadata validation PASS");
}
