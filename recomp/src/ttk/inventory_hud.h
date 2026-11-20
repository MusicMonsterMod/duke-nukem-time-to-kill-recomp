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
// D08A46: Duke 3D key card colours are on (red and yellow card icons); off,
// every key card shows the blue icon as the cards in the world do.
void mission_keycard_colours(bool on);
void mission_jewel_colours(bool on);   // D08A45: jewel icons match the world
void mission_browse_press(int direction);
bool mission_visible();
void mission_close();
// D08A18: inventory item (6..16) of the browsed slot while the mission
// inventory shows, else -1.
int mission_browsed_item();
// D20D: the level's mission items, and how many were collected (found or
// used) from the same flags as mission_update; total 0 where it has none.
void mission_tally(unsigned level,const uint16_t* flags,int& got,int& total);
// D08A48: the mission item (6..16) an object of this type gives, or 0. Pickup
// types are the cases of the pickup dispatcher 0x80081a48 that set bit 0 of a
// mission item's flag (the only code that sets one at a fixed offset); crystal
// holders 176-178 give 11-13 through 0x80091cec. Note 517.
constexpr unsigned mission_item_for_type(unsigned type) {
    switch(type) {
    case 153: case 542: case 744: case 862: return 6;
    case 154: case 745: case 863: return 7;
    case 155: case 746: return 8;
    case 157: return 10;
    case 176: return 11; case 177: return 12; case 178: return 13;
    case 547: case 761: case 854: return 14;
    case 548: case 855: return 15;
    case 549: case 856: return 16;
    default: return 0;
    }
}
// D08A48: the items (bit i) the level can give, from a scan of its placed
// objects, their contents and its enemy records (emulation thread). Until a
// scan of that level is reported, every named item is listed (as D08A5).
// mission_update and mission_tally list a named item only when the level can
// give it or Duke has it or used it; a level left with none shows the empty
// panel ("NO MISSION ITEMS") on Comma/Period.
void mission_givable(unsigned level,uint32_t items);
// D08A19: host marker in a mission item's flag halfword for "used". The
// original only ever tests, sets and clears bit 0 of items 6..16 (pickup |= 1,
// use at animation 47 &= ~1, level reset = 0), so the marker rides along in
// savestates and card saves and a level reset clears it. Used = marker set
// and bit 0 clear.
constexpr uint16_t mission_used_bit=0x4000;
}
extern "C" {
#endif
int ttk_inventory_image(const uint32_t** pixels,int* width,int* height,int available_width);
int ttk_inventory_pixels(const uint32_t* pixels);
int ttk_mission_card_image(const uint32_t** pixels,int* width,int* height,int available_width);
#ifdef __cplusplus
}
#endif
