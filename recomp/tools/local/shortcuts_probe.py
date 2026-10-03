"""D08A private SDL route; explicit inventory fixtures are never saved to a card."""
import struct
P=0x800d7198

def exercise(call,xdo,eventually,directory,report,flush):
    rows=report['shortcuts']=[];report['fixture_writes']=[]
    def frames(n):
        end=call('frame')['frame']+n
        eventually(lambda:call('frame')['frame']>=end,30)
    def read():
        b=bytes.fromhex(call('read_ram',addr=hex(P),len=0x8a4)['hex'])
        return {'anim':struct.unpack_from('<H',b,0x60)[0],'upper':struct.unpack_from('<H',b,0x74)[0],
          'equipment':b[0x3b8],'weapon':b[0x3b9],'requested':b[0x3ba],'flags':struct.unpack_from('<I',b,0x224)[0],
          'health':struct.unpack_from('<H',b,0x32)[0],'ammo':[struct.unpack_from('<h',b,0x2c6+i*4)[0] for i in range(30)],
          'items':[struct.unpack_from('<HH',b,0x354+i*4) for i in range(6)],'frame':call('frame')['frame'],
          'input':call('ttk_input')['input']}
    def sample(name):
        r=read();r['name']=name;rows.append(r);flush();return r
    def key(k):xdo('keydown',k);frames(3);xdo('keyup',k)
    def write(a,v,n=2):
        report['fixture_writes'].append({'address':hex(a),'value':v,'bytes':n});flush()
        for i in range(n):call('write_ram',addr=hex(a+i),val=hex((v>>(i*8))&255))
    def selected(k,slot):
        write(P+0x32,10000) # private fixture: survive stationary enemy fire
        key(k);frames(65);r=sample('select-'+k+'-'+str(slot))
        if slot==13 and r['weapon']==14 and r['requested']==13 and r['flags']&4:
            # Original lit dynamite cannot be stowed. The original request waits
            # for the throw; the production shortcut never synthesizes fire.
            xdo('mousedown','1');frames(4);xdo('mouseup','1');frames(65)
            r=sample('original-dynamite-throw-then-holy')
        for retry in range(12):
            if (r['equipment']==0 and r['requested']==0 if slot==0 else r['weapon']==slot and r['equipment']==2) and not r['flags']&4:break
            write(P+0x32,10000);frames(20);r=sample('equipment-settle-'+k+'-'+str(retry))
        assert (r['equipment']==0 and r['requested']==0 if slot==0 else r['weapon']==slot and r['equipment']==2) and not r['flags']&4,(k,slot,r)
    call('turbo',enabled=0)
    if not read()['input']['captured']:key('F10');frames(8)
    call('screenshot_file',path=str(directory/'gameplay-entry.png'))
    before=sample('natural-entry');key('m');frames(35);r=sample('absent-medkit');assert r['health']<=before['health']
    selected('1',0);key('q');frames(2);kick=sample('original-boot-kick');assert kick['anim'] in range(112,116),kick
    frames(80);selected('2',4);selected('x',0);selected('x',4)
    # Natural Ctrl press/release timings; no stance, flags or position fixtures.
    for turn in range(2):
        write(P+0x32,10000)
        start=call('frame')['frame'];xdo('keydown','Control_L')
        for i in range(12):frames(2);sample('crouch-held-'+str(turn)+'-'+str(i))
        held=rows[-1];assert held['anim'] in (178,179),held
        if turn==1:call('screenshot_file',path=str(directory/'crouch-held.png'))
        xdo('keyup','Control_L')
        for i in range(12):frames(2);sample('crouch-release-'+str(turn)+'-'+str(i))
        assert rows[-1]['anim']==63,rows[-1]
    write(P+0x32,10000)
    game_window=xdo('search','--name','Duke').splitlines()[0]
    other_window=xdo('search','--name','D04 focus sink').splitlines()[0]
    xdo('keydown','Control_L');frames(15)
    xdo('windowfocus','--sync',other_window);xdo('keyup','Control_L');frames(20)
    lost=sample('crouch-focus-release');assert lost['anim']==63 and not lost['input']['captured'],lost
    xdo('windowfocus','--sync',game_window)
    eventually(lambda:read()['input']['captured']);frames(10)
    write(P+0x32,10000)
    call('screenshot_file',path=str(directory/'crouch-released.png'))
    before=sample('distance-before');xdo('keydown','Alt_L');xdo('click','--repeat','4','--delay','25','4');xdo('keyup','Alt_L');frames(35)
    close=sample('distance-close');assert close['input']['controls']['preferred_radius']<before['input']['controls']['preferred_radius']
    assert close['weapon']==before['weapon']
    xdo('keydown','Alt_L');xdo('click','--repeat','4','--delay','25','5');xdo('keyup','Alt_L');frames(35)
    far=sample('distance-far');assert far['input']['controls']['preferred_radius']>close['input']['controls']['preferred_radius']
    # Explicit isolated ownership/ammo fixtures, original key selection and draw paths.
    for slot in range(15):write(P+0x2c4+slot*4,1);write(P+0x2c6+slot*4,20 if slot else 1)
    for k,slot in [('3',5),('4',7),('5',8),('6',12),('7',10),('8',9),('9',6),('9',3),('0',11),('1',0),('1',1),('1',2)]:
        selected(k,slot)
    selected('2',4)
    write(P+0x32,10000);xdo('click','5');frames(65);wheel_next=sample('wheel-next');assert wheel_next['weapon']==5,wheel_next
    xdo('click','4');frames(65);wheel_previous=sample('wheel-previous');assert wheel_previous['weapon']==4,wheel_previous
    for item in (1,2,3,5):write(P+0x354+item*4,1);write(P+0x356+item*4,1000)
    # Direct toggles use the original item handler. Probe swaps jetpack to Space.
    def gadget_settled(name,item):
        r=sample(name+'-initial')
        for retry in range(20):
            if not r['items'][item][0]&0x8000 and not r['flags']&12 and r['equipment']!=1 and r['upper'] in (5,20,29,35,39,63):break
            write(P+0x32,10000);frames(10);r=sample(name+'-settle-'+str(retry))
        assert not r['items'][item][0]&0x8000 and not r['flags']&12 and r['equipment']!=1 and r['upper'] in (5,20,29,35,39,63),r
        return r
    for k,item in [('n',3),('b',2),('space',1)]:
        write(P+0x32,10000)
        write(P+0x356+item*4,9000)
        key(k);frames(65);on=gadget_settled('item-on-'+k,item);assert on['items'][item][0]&2,on
        write(P+0x32,10000);key(k);frames(65);off=gadget_settled('item-off-'+k,item);assert not off['items'][item][0]&2,off
    write(P+0x32,10000);key('m');frames(30)
    full=sample('full-health-medkit-attempt') # enemies may damage before original use
    assert full['items'][5][1]<=1000,full
    write(P+0x36a,1000)
    write(P+0x32,8000)
    key('m');frames(3);med=sample('owned-medkit');assert med['items'][5][1]<1000,med # enemy damage can hide the health gain
    write(P+0x32,10000)
    key('bracketright');frames(5);chosen=sample('item-cycle-next');assert chosen['input']['controls']['selected_item']==1,chosen
    key('u');frames(12);used=sample('selected-item-use');assert used['items'][1][0]&2,used
    key('space');frames(12)
    call('screenshot_file',path=str(directory/'gadgets-complete.png'))
    # Slot 4 steroids are original pickup-activated; R and Q must never fire.
    before=sample('no-macro-before');key('r');key('q');frames(20);after=sample('no-macro-after');assert before['ammo']==after['ammo']
    # Explosive fuse/throw checks run last: the original blast can kill Duke.
    selected('6',12);selected('6',14);selected('6',13)
    report['assertions']='Original boot/pistol/history/kick, held/released crouch and exclusive distance wheel; logged private health top-ups and synthetic inventory ownership with original equipment/use transitions. Enemy fire prevents a stable full-health assertion.'
    flush()
