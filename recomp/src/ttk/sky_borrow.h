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
#include "mod_plugins.h"
#include <atomic>
#include <algorithm>
#include <utility>
#include <vector>
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
// D17BP: a painted face left out in this composition (the near-clip path
// may run on the redraw workers too, hence atomic).
inline std::atomic<bool> sky_borrow_face_left{false},sky_borrow_face_left_prev{false};
inline bool sky_borrow_skip(uint32_t clut,uint32_t tpage,const uint32_t uv[4],unsigned count) {
    const bool skip=sky_borrow_ready && painted_sky_face(sky_borrow_level,clut,tpage,uv,count);
    if(skip) sky_borrow_face_left.store(true,std::memory_order_relaxed);
    return skip;
}
// D17BJ: openings without a face (the GOLD AND GUNS cabin shaft) show the
// borrowed sky too (note 504). D17BP (note 550): but not every opening is
// a window: the level's cavern roof has a gap (a hole in the original
// geometry, drawn black by the game) that then showed a patch of sky. So
// the sky is drawn while a painted face was left out (this or the last
// composition, as D17AO1) or while a room with an open shaft top is shown:
// a level rim of unshared wall edges with nothing above (sky_borrow.cpp,
// found from the level's mesh). A gap in a roof (sloped edges, sloped
// faces round it) is no shaft and stays black, as the original drew it.
inline std::atomic<bool> sky_borrow_shaft_shown{false};
// D17BP rev 2: roof gaps closed with rock (sky_borrow.cpp find_shafts). The
// caps live in one block of mod memory, allocated on the first live frame
// (before any redraw worker is forked, so every worker knows it) and filled
// by the live game thread only; a redraw worker reads the table from its
// frame's own image. Table: +0 the level it was built for, +4 count, then
// (room, piece) pairs; the pieces follow at +0x100. host_visibility.inc
// draws them with their room, near_clip accepts them.
inline uint32_t roof_cap_block=0;
constexpr uint32_t roof_cap_block_size=0x4000,roof_cap_table_max=30;
inline bool roof_cap_owns(uint32_t address) {return roof_cap_block && address>=roof_cap_block && address<roof_cap_block+roof_cap_block_size;}
inline unsigned roof_cap_count(uint32_t level) {
    if(!roof_cap_block || psx_mod_read_word(roof_cap_block)!=level) return 0;
    return std::min<unsigned>(psx_mod_read_word(roof_cap_block+4),roof_cap_table_max);
}
inline uint32_t roof_cap_room(unsigned i) {return psx_mod_read_word(roof_cap_block+8+8*i);}
inline uint32_t roof_cap_piece(unsigned i) {return psx_mod_read_word(roof_cap_block+12+8*i);}
inline bool sky_borrow_wanted() {
    return sky_borrow_ready && (sky_borrow_face_left.load(std::memory_order_relaxed) || sky_borrow_face_left_prev.load(std::memory_order_relaxed) ||
                                sky_borrow_shaft_shown.load(std::memory_order_relaxed));
}
}
