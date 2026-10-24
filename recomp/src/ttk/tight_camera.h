#pragma once
#include <algorithm>
#include <cmath>
namespace ttk {
// Presentation-only curves. Distances are original world units.
inline double camera_smoothstep(double lo,double hi,double x) {
    const double t=std::clamp((x-lo)/(hi-lo),0.0,1.0);
    return t*t*(3.0-2.0*t);
}
inline double camera_body_coverage(double distance,double lateral) {
    // Only dissolve a close body that actually overlaps the camera's sightline.
    const double proximity=camera_smoothstep(650,1700,distance);
    const double off_center=camera_smoothstep(260,660,std::abs(lateral));
    return proximity+(1-proximity)*off_center;
}
}
