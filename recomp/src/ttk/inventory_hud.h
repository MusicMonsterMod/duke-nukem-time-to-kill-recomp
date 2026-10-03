#pragma once
#include <stdint.h>
#ifdef __cplusplus
namespace ttk {
bool inventory_visible();
void inventory_update(unsigned selected,const uint16_t* flags,const int16_t* amount,const uint16_t* capacity,bool announce);
}
extern "C" {
#endif
int ttk_inventory_image(const uint32_t** pixels,int* width,int* height,int available_width);
int ttk_inventory_pixels(const uint32_t* pixels);
#ifdef __cplusplus
}
#endif
