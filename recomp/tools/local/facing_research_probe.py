"""Read-only D07A draw/holster field correlation through original SDL input."""
from ttk_state_probe import sample
import json


def exercise(call,xdo,eventually,directory,report,flush):
    def frames(n):
        end=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=end,30)
    rows=report['facing_research']=[]
    def capture(name):
        s=sample(call);(directory/(name+'.json')).write_text(json.dumps(s,indent=2))
        rows.append({'name':name,'state':{k:v for k,v in s.items() if k!='raw_hex'},'input':call('ttk_input'),'pad':call('pad_status')});flush()
        call('screenshot_file',path=str(directory/(name+'.png')))
    call('turbo',enabled=0);xdo('key','F10');frames(6);capture('initial')
    for key in ['h','h','Shift_L']:
        prefix=str(len(rows))+'-'+key
        xdo('keydown',key);frames(20);capture(prefix+'-held');xdo('keyup',key);frames(30);capture(prefix+'-released')
    xdo('mousedown',1);frames(24);capture('fire-held');xdo('mouseup',1);frames(12);capture('fire-released')
