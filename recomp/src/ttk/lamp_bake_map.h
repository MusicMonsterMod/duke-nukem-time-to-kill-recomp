// D41M: face record -> baked surface (1-based), shared with the redraw
// workers (lamp_bake_map.cpp).
#pragma once
#include <cstdint>
#include <utility>
#include <vector>

namespace ttk {
// Game thread: the current level's faces (record address, surface).
void lamp_bake_map_publish(const std::vector<std::pair<uint32_t,uint32_t>>& records);
void lamp_bake_map_clear();
// Any thread or worker: the surface of a world face record (0: none).
uint32_t lamp_bake_surface(uint32_t record);
// D23N1: hold the table current at a redraw job's start until its end.
void lamp_bake_pin(bool on);
// D41P: the level's floors as a grid (Quake's R_LightPoint: a model takes the
// bake of the floor it stands on). Built from the bake's triangles and faces
// (face i is surface i+1); kept with the face map, published by the next
// lamp_bake_map_publish (call this first).
struct LampFloorGrid {float x0=0,z0=0,cell=128;uint32_t w=0,h=0;std::vector<uint32_t> start;std::vector<float> y;std::vector<uint32_t> surf;};
LampFloorGrid lamp_bake_floor_build(const std::vector<float>& tris,const std::vector<std::pair<uint32_t,uint32_t>>& faces);
void lamp_bake_floor_stage(const LampFloorGrid& g);
// Any thread or worker: the surface of the nearest floor at or below (x,y,z)
// (TTK's y grows downward); 0 when none.
uint32_t lamp_bake_floor(float x,float y,float z);
}
