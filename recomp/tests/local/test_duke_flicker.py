"""D41AB: Duke Nukem 3D's random light (SE 4) as reimplemented in duke_flicker.h."""
from pathlib import Path
import subprocess
import tempfile
import unittest

class DukeFlicker(unittest.TestCase):
    def test_se4_rule_rate_and_frame_rate_independence(self):
        root=Path(__file__).resolve().parents[2]
        source=r'''
#include "duke_flicker.h"
#include <cassert>
#include <cmath>
#include <initializer_list>
int main() {
    using namespace ttk;
    // The EDuke32 rule, tick by tick.
    for (int hitag: {0,1,9,11}) {
        unsigned lit=0,runs=0,longest=0,run=0;bool was=false;
        for (uint32_t t=0;t<300000;++t) {
            const uint32_t r=duke_global_random(t);
            assert(r<=0xffff);
            const DukeLight l=duke_random_light(t,hitag,0,20);
            const bool expect=((r/(uint32_t)(hitag+1))&31)<4;
            assert(l.lit==expect);
            assert(l.lit ? l.shade==(int)(r&15) : l.shade==20);
            if(l.lit) {++lit;run=was?run+1:1;if(!was) ++runs;if(run>longest) longest=run;}
            was=l.lit;
        }
        const double p=lit/300000.0;
        assert(std::fabs(p-0.125)<0.006);         // one tick in eight
        assert(lit/(double)runs<1.25);             // flashes mostly one tick long
        assert(longest<=8);
    }
    // Different hitags flash apart.
    unsigned both=0,a=0;
    for (uint32_t t=0;t<100000;++t) {bool x=duke_random_light(t,0,0,20).lit,y=duke_random_light(t,11,0,20).lit;a+=x;both+=x&&y;}
    assert(both<a/3);
    // Build's shade factor, the tutorial's levels.
    assert(duke_shade_factor(0)==1.0f && duke_shade_factor(-5)==1.0f && duke_shade_factor(20)==12.0f/32);
    assert(duke_shade_factor(40)==0.0f);
    // The level holds for a whole 1/30 s tick whatever the image rate.
    for (uint32_t ms=0;ms<60000;++ms) {
        const uint32_t t=duke_tick(ms);
        assert(t==(uint32_t)(ms*30ull/1000));
        assert(duke_random_light_level(ms,0)==duke_random_light_level_at(t,0));
    }
    const float m=duke_random_light_mean(0,20);
    assert(m>0.375f && m<0.45f);
    assert(std::fabs(duke_random_light_swing(0,20)-(1.0f-m))<1e-6);
}
'''
        with tempfile.TemporaryDirectory() as d:
            p=Path(d);(p/'test.cpp').write_text(source)
            subprocess.run(['c++','-std=c++17','-I',str(root/'src/ttk'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)
