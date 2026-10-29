#pragma once
#include <cstdint>
namespace ttk {
// Emulation-thread only: accepted native piano use, never raw key input.
void piano_sound(int32_t x, int32_t y, int32_t z);
void piano_audio_mix(int16_t* stereo, int frames);
void piano_audio_reset();
}
