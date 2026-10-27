#pragma once
#include <cstdint>
extern "C" void ttk_music_mix(int16_t* stereo, int frames);
extern "C" void ttk_music_reset();
extern "C" void ttk_music_update();
