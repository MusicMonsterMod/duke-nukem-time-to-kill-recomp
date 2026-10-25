#pragma once
#include <cstdint>
namespace ttk {
// Emulation-thread only. Called after a successful Modern on/off transition.
void night_vision_sound();
}
// Host-only PCM, mixed before runtime fade/master volume, never into SPU state.
extern "C" void ttk_audio_mix(int16_t* stereo, int frames);
extern "C" void ttk_audio_reset();
