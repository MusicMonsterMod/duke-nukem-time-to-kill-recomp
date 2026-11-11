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
}
