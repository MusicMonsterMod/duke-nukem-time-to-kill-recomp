#pragma once

#include <algorithm>
#include <cmath>

namespace ttk::contact_arm {
struct Vec {
    double x{}, y{}, z{};
    Vec operator+(Vec b) const { return {x+b.x,y+b.y,z+b.z}; }
    Vec operator-(Vec b) const { return {x-b.x,y-b.y,z-b.z}; }
    Vec operator*(double s) const { return {x*s,y*s,z*s}; }
    double dot(Vec b) const { return x*b.x+y*b.y+z*b.z; }
    double length() const { return std::sqrt(dot(*this)); }
};
struct Pose { Vec shoulder, elbow, hand; double upper_scale, lower_scale; };

// Contact is authoritative. Solve a display-only chain from a virtual shoulder,
// retaining native bone lengths whenever reachable. Unreachable targets extend
// both segments proportionally rather than pulling a hand off its contact.
// An authored pole controls the bend plane. The native elbow is only a
// singular-pole fallback; no temporal state is needed for replay/save load.
inline bool solve(Vec shoulder, Vec hand, Vec native_elbow, Vec pole,
                  double upper, double lower, Pose& out) {
    if(!std::isfinite(upper) || !std::isfinite(lower) ||
       upper<1 || lower<1 || upper>1024 || lower>1024)return false;
    const Vec reach=hand-shoulder;
    const double distance=reach.length();
    if(!std::isfinite(distance) || distance<1)return false;
    const Vec axis=reach*(1/distance);
    // Keep a small elbow bend even at maximum reach, avoiding the singular
    // straight-arm flip as the target passes between reachable/unreachable.
    const double scale=std::max(1.0,distance/((upper+lower)*.98));
    const double a=upper*scale,b=lower*scale;
    const double along=std::clamp((distance*distance+a*a-b*b)/(2*distance),-a,a);
    const double height=std::sqrt(std::max(0.0,a*a-along*along));
    Vec bend=pole-axis*pole.dot(axis);
    if(bend.length()<.01) {
        bend=native_elbow-shoulder;
        bend=bend-axis*bend.dot(axis);
    }
    if(bend.length()<.01) {
        const Vec fallback=std::abs(axis.y)<.9?Vec{0,1,0}:Vec{1,0,0};
        bend=fallback-axis*fallback.dot(axis);
    }
    bend=bend*(1/bend.length());
    const Vec elbow=shoulder+axis*along+bend*height;
    out={shoulder,elbow,hand,(elbow-shoulder).length()/upper,(hand-elbow).length()/lower};
    return true;
}
} // namespace ttk::contact_arm
