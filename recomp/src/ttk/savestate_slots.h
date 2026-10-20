#pragma once

namespace ttk {
// D24B: the level recorded beside a savestate slot (its ".ttk" sidecar), if
// the slot was saved in gameplay by a build that writes it.
bool savestate_slot_level(int slot, unsigned& level);
// D24B: the era a level belongs to ("" when none).
const char* level_era(unsigned level);
}
