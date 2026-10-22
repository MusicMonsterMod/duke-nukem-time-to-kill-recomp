"""Compile the production rate resolver and renderer setters; no window required."""
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]

class AutoFrameRateTest(unittest.TestCase):
    def test_rates_and_live_display_changes(self):
        source = (ROOT / 'psxrecomp/runtime/src/gpu_gl_renderer.c').read_text()
        def function(name):
            start = source.index('void ' + name + '(')
            return source[start:source.index('\n}', start) + 2]
        code = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "replay_rate.h"
static double s_rp_host_hz, s_rp_target_hz;
static uint64_t s_rp_next_deadline, s_rp_last_prefetch;
static int s_rp_schedule;
static void frame_interpolation_schedule_reset(int *p) {*p=0;}
'''
        code += function('gl_renderer_set_replay_host_hz') + '\n'
        code += function('gl_renderer_set_replay_target_hz') + '\n'
        code += r'''
int main(void) {
    double modes[][2]={{24,24},{50,50},{59.94,59.94},{60,60},{75,75},
        {100,100},{119.88,119.88},{120,120},{144,72},{165,82.5},
        {180,90},{239.76,119.88},{240,120},{360,120},{500,100}};
    for (unsigned i=0;i<sizeof(modes)/sizeof(modes[0]);++i) {
        double hz=replay_resolved_hz(-2,modes[i][0]);
        assert(fabs(hz-modes[i][1])<0.00001 && hz<=120);
        assert(fabs(modes[i][0]/hz-round(modes[i][0]/hz))<0.00001);
        s_rp_target_hz=-2;s_rp_next_deadline=100;s_rp_last_prefetch=50;s_rp_schedule=1;
        gl_renderer_set_replay_host_hz(modes[i][0]);
        assert(s_rp_next_deadline==0 && s_rp_last_prefetch==0 && s_rp_schedule==0);
        assert(replay_resolved_hz(s_rp_target_hz,s_rp_host_hz)==hz);
    }
    assert(replay_resolved_hz(-2,0)==60);
    assert(replay_resolved_hz(-2,-1)==60);
    assert(replay_resolved_hz(-2,NAN)==60);
    assert(replay_resolved_hz(-2,INFINITY)==60);
    gl_renderer_set_replay_target_hz(144);
    s_rp_next_deadline=100;gl_renderer_set_replay_host_hz(60);
    assert(s_rp_next_deadline==100 && replay_resolved_hz(s_rp_target_hz,s_rp_host_hz)==144);
    gl_renderer_set_replay_target_hz(0);
    assert(s_rp_next_deadline==0 && replay_resolved_hz(0,165)==165);
    gl_renderer_set_replay_target_hz(-1);
    assert(replay_resolved_hz(-1,60)==1000);
    gl_renderer_set_replay_target_hz(-2);
    assert(replay_resolved_hz(s_rp_target_hz,240)==120);
}
'''
        with tempfile.TemporaryDirectory() as temp:
            path = pathlib.Path(temp)
            (path/'test.c').write_text(code)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-I',str(ROOT/'psxrecomp/runtime/include'),str(path/'test.c'),'-lm','-o',str(path/'test')],check=True)
            subprocess.run([str(path/'test')],check=True,capture_output=True)

if __name__ == '__main__':
    unittest.main()
