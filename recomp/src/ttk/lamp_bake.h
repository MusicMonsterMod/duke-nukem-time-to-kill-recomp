// D41M: baked light of a level's fixed lamps on its static room mesh
// (documentation/446-d41m-baked-lamps.md). Pure computation, no emulator
// access: the title gives it the shadow mesh (world triangles, as
// light_shadows.inc builds it) and the lamps; it returns a lightmap atlas and
// one chart record per triangle for the renderer.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ttk {

struct BakeLamp {
    float p[3];          // world position
    float rgb[3];        // colour x strength (the live light's rgb at full style)
    float radius;        // reach (Relit reach of the lamp's own room applied)
    bool channel;        // varies live (style, signal, breakable prop): own channel
    float mean;          // its mean scalar, when it overflows a triangle's channels
    // D55A: how far it strays from its mean (0-1), for choosing a face's four
    // channel lamps: a traffic light (on, off) baked at its mean is wrong all
    // the time, a candle (0.78-1.0) hardly at all.
    float swing = 1.0f;
};

struct BakeInput {
    std::vector<float> tris;      // 9 floats per triangle (world)
    // Faces: a triangle, or a quad as two consecutive triangles (the GPU's
    // halves (0,1,2), (1,2,3)). One chart and one surface id per face.
    struct Face {uint32_t first;uint32_t count;};
    std::vector<Face> faces;
    std::vector<BakeLamp> lamps;  // up to 255
    float texel = 32.0f;          // world units per lightmap texel
    int samples = 2;              // per texel axis (visibility supersampling)
    int atlas_width = 4096;
    unsigned threads = 0;         // 0: half the logical CPUs, at most 8
};

// One record per face, 16 floats (four RGBA32F texels for the shader):
//  [0..2]  chart origin P0 (world), [3]  front chart u0 (texels, -1 none)
//  [4..6]  e1 / texel,              [7]  front chart v0
//  [8..10] e2 / texel,              [11] back chart u0 (-1 none)
//  [12] back chart v0, [13] channel lamps 0-2 (a*65536+b*256+c, 255 none),
//  [14] channel lamps 3-5, [15] chart width * 4096 + height.
// D55A: a chart whose channel lamp 4 is not 255 is "wide": channels 4-5 sit
// in the R and G of the channel texels one chart width to the right.
struct BakeOutput {
    int width = 0, height = 0;
    std::vector<uint32_t> steady;     // GL_RGB9_E5: the steady lamps' light
    std::vector<uint32_t> channels;   // RGBA8: up to 4 channel lamps' weights (0-1); 4-5 beside wide charts
    std::vector<float> charts;        // 16 floats per face
    // Statistics.
    double ms = 0;
    uint64_t texels = 0, rays = 0, lamp_texels = 0;
    unsigned faces_lit = 0, faces_two_sided = 0, channel_overflows = 0;
    unsigned faces_vary[4] = {0,0,0,0};   // D55A: faces with 1-4, 5-6, 7-8, 9+ varying lamps
    unsigned strong_overflows = 0;   // D55A: overflowing lamps with a swing of 0.5 or more (on/off)
};

bool lamp_bake(const BakeInput& in, BakeOutput& out, std::string& error);

// Cache file (one per level): the inputs' hash in the name, raw output.
uint64_t lamp_bake_hash(const BakeInput& in);
bool lamp_bake_save(const std::string& path, const BakeOutput& out);
bool lamp_bake_load(const std::string& path, BakeOutput& out);

} // namespace ttk
