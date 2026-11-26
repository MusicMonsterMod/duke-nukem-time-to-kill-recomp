// D61B: the hidden levels' titles (user 2026-11-26: "line up all the real
// level titles so they are actually presented to the user when warping to one
// of the lost levels. currently it says XXX"). On the disc True Grit (4) and
// Modern 4 (13) are named by strings 93 and 102, both "XXX", and index 30, the
// empty room, names nothing; its slot's neighbour, string 235, is the game's
// own "BOSS 4". Modernized draws its own level card, pause title, save names
// and console replies (game_menu.cpp, level_select.cpp): there these levels
// take "TRUE GRIT" (its prototype title in Mortimer's prototype notes, "Level
// 04? - True Grit"), "MODERN 4" (the only name it has had: the fourth modern
// loop) and "BOSS 4". Only while the disc's own name id is in place; the
// animation test and the sound test keep their titles. Guest data is not
// changed, so Vanilla shows the game's own.
#pragma once
#include "mod_plugins.h"
#include <cstdint>

namespace ttk {
inline const char* hidden_level_title(unsigned level) {
    if(level>=32) return nullptr;
    const unsigned id=psx_mod_read_half(0x800c3d2cu+2*level);
    if(level==4 && id==93) return "TRUE GRIT";
    if(level==13 && id==102) return "MODERN 4";
    if(level==30 && id==0) return "BOSS 4";
    return nullptr;
}
}
