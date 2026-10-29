#pragma once
#include <cmath>
#include <cstdlib>

namespace ttk {
// D14A: horizontal degrees at 4:3. The 512x240 native image has an
// approximately 1.6x horizontal camera-matrix scale: half-width is 160
// in square-pixel camera units, half-height 120. Widescreen adds columns.
constexpr double fp_fov_default=90.0, fp_fov_min=60.0, fp_fov_max=120.0;
inline bool fp_fov_valid(double degrees) {
    return std::isfinite(degrees) && degrees>=fp_fov_min && degrees<=fp_fov_max;
}
inline int fp_fov_projection(double degrees) {
    if(!fp_fov_valid(degrees))degrees=fp_fov_default;
    return int(std::lround(160.0/std::tan(degrees*3.14159265358979323846/360.0)));
}
inline double fp_fov_degrees(int projection,double half_extent=160.0) {
    return 360.0/3.14159265358979323846*std::atan(half_extent/projection);
}
inline bool fp_fov_parse(const char* text,double& degrees) {
    if(!text || !*text)return false;
    char* end=nullptr;const double value=std::strtod(text,&end);
    if(end==text || *end || !fp_fov_valid(value))return false;
    degrees=value;return true;
}
}
