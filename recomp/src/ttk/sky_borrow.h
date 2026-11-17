// D17AO1: a real sky behind skylights painted with a sky picture. See
// documentation/398-d17ao1-borrowed-sky.md.
//
// Some levels load no sky (all sky selectors zero, note 92) and close an
// opening to the outside with one world face textured with a small sky
// picture. In Modernized that face is left out and the native sky routine
// draws another level's sky behind it (sky_borrow.cpp), as EDuke32 draws a
// parallaxed ceiling: an eye-centred ring that turns with the view and never
// moves with position. D17BJ: such a level draws the borrowed sky behind
// every frame, so an opening left without any face (the GOLD AND GUNS cabin
// shaft) shows the same sky instead of black. Shared with near_clip.cpp, which is also linked into
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
    return sky_borrow_ready && painted_sky_face(sky_borrow_level,clut,tpage,uv,count);
}
// D17BJ: the borrowed sky is drawn behind every composition of a ready
// level, not only while a painted face is in view, so openings without a
// face show it too. Only background pixels change: a same-camera survey of
// 24 views (cabin, lava cavern, level start) changed nothing but the cabin
// opening (note 504).
inline bool sky_borrow_wanted() {return sky_borrow_ready;}
}
