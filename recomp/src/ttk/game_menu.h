#pragma once
#include <cstdint>

struct CPUState;

namespace ttk {
// D19: Modernized title OPTIONS, extended with PC CONTROLS, DISPLAY and
// MODERNIZED screens. Contract: documentation/145-d19-game-menu.md.
struct MenuKeys {bool up=false,down=false,left=false,right=false,confirm=false,back=false,square=false;int wheel=0;
    // D19H: key-down events since the last read, so a tap shorter than the
    // gap between two pad reads still counts (the death screen reads seldom).
    int up_taps=0,down_taps=0,left_taps=0,right_taps=0,confirm_taps=0,back_taps=0,square_taps=0;
    int delete_taps=0;}; // D19S: Delete (LOAD GAME and SAVE GAME delete a save)
// From input_pad(): true while one of our screens owns the keys (the pad
// then reads as nothing pressed). wheel: notches (+ up) taken by the menu.
bool game_menu_input(const MenuKeys& keys);
// From the frontend's composition (0x80026164) and logo (0x80025554) entry
// hooks on live frames, before the D17 capture: draw at most once per image.
void game_menu_frame(CPUState* cpu);
}
