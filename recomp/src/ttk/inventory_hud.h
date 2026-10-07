#pragma once
#include <stdint.h>
#ifdef __cplusplus
namespace ttk {
bool inventory_visible();
void inventory_update(unsigned selected,const uint16_t* flags,const int16_t* amount,const uint16_t* capacity,bool announce);
// D08A5 mission inventory: current level and the player's inventory flags
// player+0x354+4*i for i = 0..16 (emulation thread, after inventory_update);
// Comma (-1) / Period (+1) presses from any thread; open state and closing
// (Enter / U close it instead of using a gadget).
void mission_update(unsigned level,const uint16_t* flags);
void mission_browse_press(int direction);
bool mission_visible();
void mission_close();
}
extern "C" {
#endif
int ttk_inventory_image(const uint32_t** pixels,int* width,int* height,int available_width);
int ttk_inventory_pixels(const uint32_t* pixels);
int ttk_mission_card_image(const uint32_t** pixels,int* width,int* height,int available_width);
#ifdef __cplusplus
}
#endif
