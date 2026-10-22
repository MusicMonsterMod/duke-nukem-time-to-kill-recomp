#pragma once
#include <cstdint>

namespace ttk {
// D08K1/D08K4: the merged frame range of an extra animation clip (pack entry
// group 0xffff, host id) in the tables now loaded; false when the override is
// not applied (no pack, Vanilla, before the first gameplay update of a level).
bool anim_extra_clip(unsigned id, uint32_t& first, uint32_t& count);
}
