"""Compile the real GL flat-batch functions against an observing GL shim.

Checks depth/painter batch boundaries and canonical/wide draws without a GPU.
Actual shader/raster output is additionally checked in the offscreen game.
"""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'psxrecomp/runtime/src/gpu_gl_renderer.c'

SHIM = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned GLenum;
enum { GL_TRIANGLES=1, GL_LINES, GL_BLEND, GL_DEPTH_TEST, GL_LEQUAL,
       GL_TRUE, PSXGL_ARRAY_BUFFER, PSXGL_STREAM_DRAW };
static int s_pc_valid, s_hz_valid, s_mask_set, s_scale=4;
static float s_pc_x[3]={1.25f,2.5f,3.75f}, s_pc_y[3]={4,5,6};
static float s_hz[3]={.2f,.4f,.6f};
static uint64_t s_hz_tris;
static int s_wide_fast, g_wide_cur, s_wide_suppress, s_ws_ablate;
static int g_ws_bd_stretch_on, s_bd_gate;
static int s_geo_prog, s_geo_vao, s_geo_vbo, s_geo_uXoff, s_geo_uXhalf;
static int s_geo_uXscale, s_geo_uXcenter;
static int depth_enabled, draws, uploaded_n, expected_depth, wide_active;
static float uploaded[8190*7];
static void gs_enable(int cap){if(cap==GL_DEPTH_TEST) depth_enabled=1;}
static void gs_disable(int cap){if(cap==GL_DEPTH_TEST) depth_enabled=0;}
static void p_glBufferData(int a,ptrdiff_t bytes,const void *p,int b){
    (void)a;(void)b;assert(bytes% (7*sizeof(float))==0);
    uploaded_n=bytes/(7*sizeof(float));memcpy(uploaded,p,bytes);
}
static void glDrawArrays(int mode,int first,int n){
    assert(first==0 && n==uploaded_n);assert(depth_enabled==expected_depth);
    for(int i=0;i<n;i++) {
        float z=uploaded[i*7+6];
        if(expected_depth) assert(z==s_hz[i%3]); else assert(z<0);
    }
    draws++;
}
#define hr_begin(...) ((void)0)
#define hr_end(...) ((void)0)
#define apply_psx_blend(...) ((void)0)
#define mask_stencil(...) ((void)0)
#define gs_use_prog(...) ((void)0)
#define gs_bind_vao(...) ((void)0)
#define p_glBindBuffer(...) ((void)0)
#define glDepthFunc(...) ((void)0)
#define glDepthMask(...) ((void)0)
#define glLineWidth(...) ((void)0)
#define flush_tex_batch(...) ((void)0)
#define flush_cpu_upload(...) ((void)0)
#define mark_prim_dirty(...) ((void)0)
#define mirror_x_center_only(...) 0
#define mirror_geo_center_only(...) 0
#define wide_dx(...) 0
#define bd_prim_gate(...) 0
#define gl_perf_mirror_begin(...) ((void)0)
#define gl_perf_mirror_end(...) ((void)0)
#define wide_target_begin(...) (wide_active=1)
#define wide_target_end(...) (wide_active=0)
#define wide_set_bd_scale(...) ((void)0)
#define wide_clear_bd_scale(...) ((void)0)
'''
MAIN = r'''
int main(void){
    const int x[3]={1,2,3},y[3]={4,5,6};const uint16_t c[3]={31,992,31744};
    int enabled=!(getenv("PSX_HOST_FLAT_DEPTH") && getenv("PSX_HOST_FLAT_DEPTH")[0]=='0');
    for(int wide=0;wide<2;wide++){
        g_wide_cur=wide;
        s_pc_valid=s_hz_valid=1;
        expected_depth=enabled;
        gpu_geometry(GL_TRIANGLES,x,y,c,3,-1);
        gpu_geometry(GL_TRIANGLES,x,y,c,3,-1);
        assert(s_fb_n==6); // compatible host tris batch
        // A native triangle must flush the depth run before entering painter order.
        s_pc_valid=s_hz_valid=0;
        gpu_geometry(GL_TRIANGLES,x,y,c,3,-1);
        if(enabled){assert(s_fb_n==3);assert(!depth_enabled);}
        expected_depth=0;flush_flat_batch();assert(!depth_enabled);
        // Negative weapon depth is passed as s_hz_valid=0, even with precise XY.
        s_pc_valid=1;s_hz_valid=0;
        gpu_geometry(GL_TRIANGLES,x,y,c,3,-1);flush_flat_batch();
        // Semi-transparent host effects must keep their original ordering.
        s_hz_valid=1;
        gpu_geometry(GL_TRIANGLES,x,y,c,3,0);flush_flat_batch();
        gpu_geometry(GL_LINES,x,y,c,2,-1);assert(!depth_enabled);
        // Host state must not change the colors or subpixel coordinates.
        expected_depth=enabled;
        gpu_geometry(GL_TRIANGLES,x,y,c,3,-1);flush_flat_batch();
        assert(uploaded[0]==1.25f && uploaded[1]==4);
        assert(uploaded[2]==248/255.0f && uploaded[3]==0);
        assert(!depth_enabled && !wide_active);
        expected_depth=0;
    }
    assert(draws>0);assert(s_hz_tris==(enabled?6:0));
    // Capacity flush preserves the depth mode for the following batch.
    expected_depth=enabled;s_pc_valid=s_hz_valid=1;
    for(int i=0;i<FLATBATCH_MAXV/3+1;i++) gpu_geometry(GL_TRIANGLES,x,y,c,3,-1);
    assert(s_fb_n==3);flush_flat_batch();assert(!depth_enabled);
    return 0;
}
'''

class FlatDepthTest(unittest.TestCase):
    def test_batch_depth_and_painter_boundaries(self):
        source = SOURCE.read_text()
        body = source[source.index('#define GEOV'):source.index('static void gpu_triangle(')]
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            (path/'test.c').write_text(SHIM+body+MAIN)
            subprocess.run(['cc','-std=c11',str(path/'test.c'),'-o',str(path/'test')],check=True)
            for enabled in ('0','1'):
                subprocess.run([str(path/'test')],env={**os.environ,'PSX_HOST_FLAT_DEPTH':enabled},check=True)

if __name__ == '__main__':
    unittest.main()
