#pragma once
#include <algorithm>
#include <cmath>
namespace ttk {
constexpr double tau = 6.2831853071795864769;
struct Vec2 { double x, z; };
inline Vec2 movement(double yaw, double right, double forward) {
    double magnitude = std::max(1.0, std::hypot(right, forward));
    return {(std::sin(yaw)*forward + std::cos(yaw)*right)/magnitude,
            (std::cos(yaw)*forward - std::sin(yaw)*right)/magnitude};
}
inline Vec2 redirect(double x, double z, Vec2 direction) {
    double length = std::hypot(x,z);
    return {length*direction.x, length*direction.z};
}
struct LookAccumulator {
    bool valid=false;
    unsigned long long epoch=0;
    double x=0,y=0;
    Vec2 consume(unsigned long long next_epoch,double total_x,double total_y) {
        Vec2 delta{};
        if (valid && epoch==next_epoch) delta={total_x-x,total_y-y};
        valid=true;epoch=next_epoch;x=total_x;y=total_y;
        return delta;
    }
    void reset() {valid=false;}
};
inline double wrap_yaw(double yaw) {return std::remainder(yaw,tau);}
inline double clamp_pitch(double pitch) {return std::clamp(pitch,-tau/6,tau/6);}
// D08Z air steering step: move the horizontal velocity at most accel toward
// the requested direction at the flight's speed cap. The path stays inside
// the cap, so steering never gains speed, and a turn finishes in finite time.
inline Vec2 air_steer(Vec2 v,Vec2 wish,double cap,double accel) {
    const double dx=wish.x*cap-v.x,dz=wish.z*cap-v.z,distance=std::hypot(dx,dz);
    if(distance<=accel)return {wish.x*cap,wish.z*cap};
    return {v.x+dx*accel/distance,v.z+dz*accel/distance};
}
}
