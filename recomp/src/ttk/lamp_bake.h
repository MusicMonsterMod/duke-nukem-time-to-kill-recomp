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
//  [12] back chart v0, [13] channel lamps 0-1 (a*256+b, 255 none),
//  [14] channel lamps 2-3, [15] 0
struct BakeOutput {
    int width = 0, height = 0;
    std::vector<uint32_t> steady;     // GL_RGB9_E5: the steady lamps' light
    std::vector<uint32_t> channels;   // RGBA8: up to 4 channel lamps' weights (0-1)
    std::vector<float> charts;        // 16 floats per face
    // Statistics.
    double ms = 0;
    uint64_t texels = 0, rays = 0, lamp_texels = 0;
    unsigned faces_lit = 0, faces_two_sided = 0, channel_overflows = 0;
};

bool lamp_bake(const BakeInput& in, BakeOutput& out, std::string& error);

// Cache file (one per level): the inputs' hash in the name, raw output.
uint64_t lamp_bake_hash(const BakeInput& in);
bool lamp_bake_save(const std::string& path, const BakeOutput& out);
bool lamp_bake_load(const std::string& path, BakeOutput& out);

} // namespace ttk
