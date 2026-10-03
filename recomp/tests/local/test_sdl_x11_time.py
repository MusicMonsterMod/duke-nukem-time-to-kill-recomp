"""Compile the actual pinned-SDL repair and check queued reports and wrap."""
import importlib.util
from pathlib import Path
import shutil,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('sdl_time_fix',ROOT/'tools/local/fix_sdl_x11_time.py')
fix=importlib.util.module_from_spec(spec);spec.loader.exec_module(fix)
class X11TimeTest(unittest.TestCase):
 def test_queued_reports_wrap_and_old_event(self):
  if not shutil.which('cc'):self.skipTest('C compiler unavailable')
  with tempfile.TemporaryDirectory() as d:
   p=Path(d);(p/'test.c').write_text('''#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
typedef uint64_t Uint64;typedef uint32_t Uint32;typedef int64_t Sint64;typedef int32_t Sint32;
#define SDL_NS_PER_MS 1000000LL
static Uint64 now;static Uint64 SDL_GetTicksNS(void){return now;}
'''+fix.NEW+'''
int main(void){
 now=1000000000ULL;
 assert(X11_GetEventTimestamp(0xfffffff0UL)==now);
 now+=8000000;
 assert(X11_GetEventTimestamp(0xfffffff8UL)==now);
 now+=24000000;
 assert(X11_GetEventTimestamp(0x00000000UL)==now-16000000);
 assert(X11_GetEventTimestamp(0x00000008UL)==now-8000000);
 assert(X11_GetEventTimestamp(0x00000010UL)==now);
 assert(X11_GetEventTimestamp(0x00000008UL)==now-8000000);
 now+=8000000;
 assert(X11_GetEventTimestamp(0x00000018UL)==now);
 return 0;
}
''')
   subprocess.run(['cc','-Wall','-Wextra','-Werror',str(p/'test.c'),'-o',str(p/'test')],check=True)
   subprocess.run([str(p/'test')],check=True)
 def test_idempotent_and_fail_closed(self):
  with tempfile.TemporaryDirectory() as d:
   p=Path(d);q=p/'src/video/x11';q.mkdir(parents=True)
   a=q/'SDL_x11events.c';b=q/'SDL_x11xinput2.c';a.write_text(fix.OLD);b.write_text(fix.CALL_OLD)
   fix.apply(p);fix.apply(p);self.assertEqual(a.read_text(),fix.NEW)
   a.write_text('changed dependency');before=b.read_text()
   with self.assertRaises(RuntimeError):fix.apply(p)
   self.assertEqual(b.read_text(),before)
if __name__=='__main__':unittest.main()
