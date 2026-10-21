#pragma once

namespace ttk {
// D24B: the level recorded beside a savestate slot (its ".ttk" sidecar), if
// the slot was saved in gameplay by a build that writes it.
bool savestate_slot_level(int slot, unsigned& level);
// D19Q: the slot was saved on SAVE GAME (paused): loading it resumes play.
bool savestate_slot_resumes(int slot);
// D19S: delete a slot (file, thumbnail, sidecar); true when it is gone.
bool savestate_slot_delete(int slot);
// D19Q: from ttk_savestate_saved; true for SAVE GAME's own save.
bool game_menu_saved(int slot);
// D24B: the era a level belongs to ("" when none).
const char* level_era(unsigned level);
}
