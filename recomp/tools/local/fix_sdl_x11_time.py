"""Apply the reviewed raw X11 timestamp repair to the pinned fetched SDL3.

System SDL is not edited. Fail closed if the two expected source fragments
change, so upgrading SDL requires reviewing whether this repair is still needed.
"""
from pathlib import Path
import sys
OLD = '''Uint64 X11_GetEventTimestamp(unsigned long time)
{
    // FIXME: Get the event time in the SDL tick time base
    return SDL_GetTicksNS();
}'''
NEW = '''Uint64 X11_GetEventTimestamp(unsigned long time)
{
    /* TTK: X11 time is a wrapping 32-bit millisecond counter, while SDL3
     * events carry nanoseconds relative to SDL's clock. Preserve event spacing
     * when several input reports are pumped together. The smallest observed
     * arrival offset removes queue delay without predicting future input. */
    static bool initialized;
    static Uint32 last;
    static Sint64 unwrapped, offset;
    const Uint32 stamp = (Uint32)time;
    const Sint64 now = (Sint64)SDL_GetTicksNS();
    if (!initialized) {
        initialized = true;
        last = stamp;
        unwrapped = (Sint64)stamp * SDL_NS_PER_MS;
        offset = now - unwrapped;
    }
    const Sint64 delta = (Sint32)(stamp - last);
    const Sint64 event_ns = unwrapped + delta * SDL_NS_PER_MS;
    if (delta >= 0) {
        last = stamp;
        unwrapped = event_ns;
    }
    if (now - event_ns < offset) offset = now - event_ns;
    const Sint64 mapped = event_ns + offset;
    return mapped > 0 ? (Uint64)mapped : 1;
}'''
CALL_OLD = 'SDL_SendMouseMotion(rawev->time, mouse->focus,'
CALL_NEW = 'SDL_SendMouseMotion(X11_GetEventTimestamp(rawev->time), mouse->focus,'
def apply(root):
    edits = [(root/'src/video/x11/SDL_x11events.c', OLD, NEW),
             (root/'src/video/x11/SDL_x11xinput2.c', CALL_OLD, CALL_NEW)]
    pending=[]
    for p,old,new in edits:
        s=p.read_text()
        if new in s: continue
        if s.count(old)!=1: raise RuntimeError(f'Unrecognized SDL source: {p}; review X11 timestamp repair')
        pending.append((p,s.replace(old,new)))
    for p,s in pending:p.write_text(s)
    print('SDL3 X11 event timestamp repair: ready')
if __name__=='__main__':apply(Path(sys.argv[1]))
