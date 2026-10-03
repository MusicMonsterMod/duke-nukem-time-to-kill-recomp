"""Geometry invariants independent of guest code or an owned disc."""
from pathlib import Path
import subprocess
import tempfile
import unittest

class ControlMath(unittest.TestCase):
    def test_camera_relative_norm_and_animation_speed(self):
        root=Path(__file__).resolve().parents[2]
        source=r'''
#include "control_math.h"
#include <cassert>
int main() {
    using namespace ttk;
    for (int degrees=-720;degrees<=720;++degrees) {
        double yaw=degrees*tau/360;
        auto f=movement(yaw,0,1),r=movement(yaw,1,0);
        assert(std::abs(f.x*r.x+f.z*r.z)<1e-12);
        for (int x=-1;x<=1;++x) for(int z=-1;z<=1;++z) {
            auto d=movement(yaw,x,z);
            assert(std::hypot(d.x,d.z)<=1.00000001);
            auto v=redirect(30,40,d);
            assert(std::abs(std::hypot(v.x,v.z)-(x||z?50:0))<1e-10);
        }
    }
    for (int batch: {1,2,5,10,20}) {
        LookAccumulator look; look.consume(1,0,0);
        double x=0,y=0;
        for (int i=batch;i<=100;i+=batch) {
            auto d=look.consume(1,i,-i/2.0);x+=d.x;y+=d.z;
            auto duplicate=look.consume(1,i,-i/2.0);assert(duplicate.x==0 && duplicate.z==0);
        }
        assert(x==100 && y==-50);
        auto transition=look.consume(2,400,600);assert(transition.x==0 && transition.z==0);
        look.reset();auto reset=look.consume(2,800,900);assert(reset.x==0 && reset.z==0);
    }
    assert(clamp_pitch(100)==tau/6 && clamp_pitch(-100)==-tau/6);
    assert(std::abs(wrap_yaw(100*tau+0.5)-0.5)<1e-12);
    auto f=movement(tau/4,0,1); assert(std::abs(f.x-1)<1e-12);
    auto b=movement(tau/4,0,-1); assert(std::abs(b.x+1)<1e-12);
}
'''
        with tempfile.TemporaryDirectory() as d:
            p=Path(d);(p/'test.cpp').write_text(source)
            subprocess.run(['c++','-std=c++17','-I',str(root/'src/ttk'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)
