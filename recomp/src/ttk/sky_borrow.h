// D17AO1: a real sky behind skylights painted with a sky picture. See
// documentation/398-d17ao1-borrowed-sky.md.
//
// Some levels load no sky (all sky selectors zero, note 92) and close an
// opening to the outside with one world face textured with a small sky
// picture. In Modernized that face is left out and the native sky routine
// draws another level's sky behind it (sky_borrow.cpp), as EDuke32 draws a
// parallaxed ceiling: an eye-centred ring that turns with the view and never
// moves with position. Shared with near_clip.cpp, which is also linked into
// the native tests, so the state lives here.
#pragma once
#include <cstdint>

namespace ttk {
// A painted sky: the level, its texture (CLUT, page) and the UV rectangle
// of the picture, inclusive. GOLD AND GUNS (level 3): the shaft top in
// DB03A.RAW at VRAM (912,232), the only level using that picture.
struct PaintedSky {uint32_t level;uint16_t clut,tpage;uint8_t u0,v0,u1,v1;};
inline constexpr PaintedSky painted_skies[]={{3,0x1438,0x000e,64,232,86,255}};

// Set by sky_borrow.cpp each composition: the borrowed sky is loaded,
// Modernized, one player, the sky routine authenticated and this level has
// a painted sky and no sky of its own.
inline bool sky_borrow_ready=false;
inline uint32_t sky_borrow_level=~0u;
// Composition counter, and the last one that left a painted face out.
inline uint32_t sky_borrow_composition=0;
inline uint32_t sky_borrow_seen=0;
inline uint64_t sky_borrow_faces=0;
// The guest copy of the borrowed meshes (mod memory, above main RAM), which
// the near-clip host path accepts for the sky routine's draws.
inline uint32_t sky_borrow_copy=0,sky_borrow_copy_end=0;
inline bool sky_borrow_owns(uint32_t address) {return sky_borrow_copy && address>=sky_borrow_copy && address<sky_borrow_copy_end;}

inline bool painted_sky_face(uint32_t level,uint32_t clut,uint32_t tpage,const uint32_t uv[4],unsigned count) {
    for(const auto& s:painted_skies) {
        if(s.level!=level || (clut&0x7fff)!=s.clut || (tpage&0x1ff)!=s.tpage) continue;
        bool inside=true;
        for(unsigned i=0;i<count;++i) {
            const unsigned u=uv[i]&0xff,v=(uv[i]>>8)&0xff;
            inside&=u>=s.u0 && u<=s.u1 && v>=s.v0 && v<=s.v1;
        }
        if(inside) return true;
    }
    return false;
}
// World face check (near_clip host path): true when the face is a painted
// sky to leave out this composition.
inline bool sky_borrow_skip(uint32_t clut,uint32_t tpage,const uint32_t uv[4],unsigned count) {
    if(!sky_borrow_ready || !painted_sky_face(sky_borrow_level,clut,tpage,uv,count)) return false;
    sky_borrow_seen=sky_borrow_composition;++sky_borrow_faces;
    return true;
}
// The sky is wanted while a painted face was left out this composition or
// the previous one (the sky routine runs after the world).
inline bool sky_borrow_wanted() {
    return sky_borrow_ready && sky_borrow_seen && sky_borrow_composition-sky_borrow_seen<=1;
}
}
