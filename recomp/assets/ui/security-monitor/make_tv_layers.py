# D53 security TV, revision 2: original pixel art painted in the style and
# colours of Duke Nukem 3D's security screen (VIEWSCREEN) and monitor bezel.
# No retail pixels are copied; colours are matched by value.
import random
from PIL import Image
random.seed(53)
# Duke 3D palette values used by its security screen and bezel
BLACK=(0,0,0);DEEP=(12,12,8);D1=(24,24,24);D2=(36,32,32);BASE=(44,40,44);M1=(56,52,52);M2=(64,60,60);L1=(76,72,72);L2=(88,80,80);L3=(96,88,92);GL=(116,108,112);GL2=(128,116,120);GL3=(150,140,146)
GLASS=(16,12,16);GLASS2=(20,16,20)
OL0=(48,48,28);OL1=(68,64,40);OL2=(84,84,52);OL3=(104,100,64);OL4=(112,108,68);OL5=(132,128,80);OL6=(148,148,84)
F5={'A':["###","#.#","###","#.#","#.#"],'B':["##.","#.#","##.","#.#","##."],'C':["###","#..","#..","#..","###"],'D':["##.","#.#","#.#","#.#","##."],
'E':["###","#..","##.","#..","###"],'F':["###","#..","##.","#..","#.."],'G':["###","#..","#.#","#.#","###"],'H':["#.#","#.#","###","#.#","#.#"],
'I':["###",".#.",".#.",".#.","###"],'K':["#.#","#.#","##.","#.#","#.#"],'L':["#..","#..","#..","#..","###"],'M':["#.#","###","###","#.#","#.#"],
'N':["##.","#.#","#.#","#.#","#.#"],'O':["###","#.#","#.#","#.#","###"],'P':["###","#.#","###","#..","#.."],'R':["##.","#.#","##.","#.#","#.#"],
'S':["###","#..","###","..#","###"],'T':["###",".#.",".#.",".#.",".#."],'U':["#.#","#.#","#.#","#.#","###"],'V':["#.#","#.#","#.#","#.#",".#."],
'W':["#.#","#.#","###","###","#.#"],'Y':["#.#","#.#",".#.",".#.",".#."],'X':["#.#","#.#",".#.","#.#","#.#"],
'0':["###","#.#","#.#","#.#","###"],'1':[".#.","##.",".#.",".#.","###"],'2':["##.","..#",".#.","#..","###"],'3':["##.","..#",".#.","..#","##."],
'4':["#.#","#.#","###","..#","..#"],'5':["###","#..","##.","..#","##."],'6':["###","#..","###","#.#","###"],'7':["###","..#",".#.",".#.",".#."],
'8':["###","#.#","###","#.#","###"],'9':["###","#.#","###","..#","##."],':':["...",".#.","...",".#.","..."],'>':["#..",".#.","..#",".#.","#.."],
'-':["...","...","###","...","..."],'.':["...","...","...","...",".#."],' ':["...","...","...","...","..."],'/':["..#","..#",".#.","#..","#.."]}
# Wide, heavy capitals (Duke's SECURITY is set extended): 9 wide, 7 tall.
BIG={
'S':[".#######.","##.....##","##.......",".#######.",".......##","##.....##",".#######."],
'E':["#########","##.......","##.......","########.","##.......","##.......","#########"],
'C':[".#######.","##.....##","##.......","##.......","##.......","##.....##",".#######."],
'U':["##.....##","##.....##","##.....##","##.....##","##.....##","##.....##",".#######."],
'R':["########.","##.....##","##.....##","########.","##...##..","##....##.","##.....##"],
'I':["..#####..","...##....","...##....","...##....","...##....","...##....","..#####.."],
'T':["#########","....##...","....##...","....##...","....##...","....##...","....##..."],
'Y':["##.....##",".##...##.","..##.##..","...###...","....##...","....##...","....##..."]}
def put(px,x,y,c):
    px[x,y]=c+(255,)
def small(px,x,y,s,c,c2=None):
    for ch in s:
        for r,row in enumerate(F5[ch]):
            for k,v in enumerate(row):
                if v=='#':put(px,x+k,y+r,c if (c2 is None or r<3) else c2)
        x+=4
    return x
def screen(cursor):
    W=H=96;im=Image.new('RGBA',(W,H),GLASS+(255,));px=im.load()
    # faint glass sheen: a wide soft diagonal band from the top right
    for y in range(H):
        for x in range(W):
            d=(x-y)-40
            if 0<=d<26 and (x+y)%2==0 and y<60:put(px,x,y,GLASS2)
    # rounded bevelled glass rim (radius 6)
    R=6
    def inside(x,y,inset):
        lo=inset;hi=W-1-inset
        if x<lo or y<lo or x>hi or y>hi:return False
        cx=min(max(x,lo+R),hi-R);cy=min(max(y,lo+R),hi-R)
        return (x-cx)**2+(y-cy)**2<=R*R
    for y in range(H):
        for x in range(W):
            if not inside(x,y,0):put(px,x,y,BLACK)
            elif not inside(x,y,1):put(px,x,y,D2 if (x<W/2)==(y<H/2) else BASE)
            elif not inside(x,y,2):
                # lit rim: brighter toward the top right and bottom right
                t=(x/W)*0.6+(1-y/H)*0.4
                put(px,x,y,L2 if t>0.75 else M2 if t>0.5 else M1 if t>0.3 else D2)
            elif not inside(x,y,3):put(px,x,y,(10,8,10))
    # corner glints (top right brightest), as on the glass of the original
    for (x,y,c) in [(W-4,3,GL3),(W-5,3,GL2),(W-4,4,GL2),(W-6,3,GL),(W-4,5,GL),(W-7,4,L3),(W-5,6,L3),
                    (3,3,L2),(4,3,M2),(3,4,M2),(W-4,H-4,L3),(W-5,H-4,L1),(W-4,H-5,L1),(3,H-4,M1)]:put(px,x,y,c)
    for i in range(7):put(px,W-14-i,8+i,(30,26,30))   # a faint diagonal streak
    # SECURITY: heavy, extended, olive with a light top and dark foot
    x=7;ramp=[OL5,OL5,OL4,OL4,OL3,OL2,OL1]
    for ch in 'SECURITY':
        for r,row in enumerate(BIG[ch]):
            for k,v in enumerate(row):
                if v=='#':
                    put(px,x+k+1,8+r+1,OL0) if (x+k+1<W-4 and px[x+k+1,8+r+1][:3]==GLASS) else None
        for r,row in enumerate(BIG[ch]):
            for k,v in enumerate(row):
                if v=='#':put(px,x+k,8+r,ramp[r])
        x+=10
    small(px,7,19,'SURVEILLANCE NET',OL1,OL0)
    small(px,7,25,'SYSTEM READY',OL1,OL0)
    # Generic: the one feed behind this screen, whatever the level's camera count.
    rows=[('FEED','LIVE'),('LINK','SECURE'),('TAPE','REC')]
    y=37
    for a,c in rows:
        small(px,10,y,a,OL2,OL1);small(px,52,y,c,OL2,OL1);y+=7
    xx=small(px,10,61,'>',OL2,OL1)
    if cursor:
        for yy in range(61,66):
            for k in range(3):put(px,xx+k,yy,OL3)
    return im
def front():
    # Laid out as Duke 3D's monitor bezel on a 64-grid, painted at 128 (2x):
    # 8-cell borders top and sides, the screen opening (16..111 here), and the
    # controls built into the deeper bottom border under a raised bevel.
    W=H=128;im=Image.new('RGBA',(W,H));px=im.load()
    def grain(x,y):
        r=random.random();return D2 if r<0.22 else BASE if r<0.9 else M1
    for y in range(H):
        for x in range(W):put(px,x,y,grain(x,y))
    def hline(y0,y1,x0,x1,c):
        for y in range(y0,y1):
            for x in range(x0,x1):put(px,x,y,c(x,y) if callable(c) else c)
    def vline(x0,x1,y0,y1,c):hline(y0,y1,x0,x1,c)
    # outer edge: dark rim, the left edge catching light
    hline(0,2,0,W,D2);vline(0,2,0,H,D2);vline(W-2,W,0,H,D2);hline(H-2,H,0,W,D1)
    vline(2,3,2,H-2,M1)
    # inner frame around the screen (cells 4..7 from the edge): dark groove,
    # lit bevel (left and top brighter), dark groove, then the lip
    hline(8,10,8,W-8,D2);vline(8,10,8,110,D2);vline(W-10,W-8,8,110,D2)
    hline(10,12,10,W-10,M1);vline(10,12,10,110,lambda x,y:L1 if y%5 else L2);vline(W-12,W-10,10,110,M1)
    hline(12,14,12,W-12,D2);vline(12,14,12,110,D2);vline(W-14,W-12,12,110,D2)
    hline(14,16,14,W-14,M2);vline(14,16,14,112,M2);vline(W-16,W-14,14,112,D2)
    for y in range(16,112):
        for x in range(16,112):px[x,y]=(0,0,0,0)        # the screen shows through
    # under the screen: dark lip, then the raised bevel line with a small
    # emblem in the middle, then a dark seam
    hline(110,112,8,W-8,D2);hline(112,114,8,W-8,D2)
    hline(114,116,10,W-10,lambda x,y:M2 if (x//2)%4 else L1)
    for x,c in ((58,GL),(60,GL2),(62,GL3),(64,GL3),(66,GL2),(68,GL)):hline(114,116,x,x+2,c)
    hline(116,118,4,W-4,D2)
    # the tray: dotted speaker grilles at both ends, three keys and a dark
    # readout in the middle
    for y in range(118,124):
        for x in range(2,30):put(px,x,y,D1 if ((x//2)+(y//2))%2 else D2)
        for x in range(W-30,W-2):put(px,x,y,D1 if ((x//2)+(y//2))%2 else D2)
    hline(118,122,30,W-30,BASE)
    for kx in (32,44,56):
        hline(118,120,kx,kx+10,BASE);hline(118,119,kx,kx+10,M1);hline(120,122,kx,kx+10,D2)
        vline(kx-1,kx,118,122,M1);vline(kx+10,kx+11,118,122,D2)
    hline(118,122,72,96,(24,20,12))
    hline(118,119,72,96,(32,32,16));hline(121,122,72,96,(40,40,24))
    for x,c in ((76,(68,64,40)),(78,(68,64,40)),(82,(32,32,16)),(86,(48,48,28)),(90,(68,64,40))):hline(119,121,x,x+2,c)
    put(px,93,119,(128,100,4));put(px,94,119,(168,136,24));put(px,93,120,(96,72,4));put(px,94,120,(128,100,4))   # amber power light
    hline(122,124,30,W-30,D2)
    # the tray's lower bevel and the bottom edge
    hline(124,125,32,W-32,M2);hline(125,126,4,W-4,L1);hline(126,128,0,W,D1)
    for x in range(4,W-4,2):put(px,x,125,M2 if x%8 else L2)
    return im
def side_h():
    # Top and bottom of the box, 128x32 at the bezel's density (620 x 140
    # units): the bezel's grained plastic, its front edge lit (row 0), a
    # moulding seam, and shade toward the wall (row 31).
    W,H=128,32;im=Image.new('RGBA',(W,H));px=im.load()
    for y in range(H):
        for x in range(W):
            r=random.random();c=D2 if r<0.24 else BASE if r<0.9 else M1
            if y>=26:c=D2 if r<0.6 else BASE
            if y>=30:c=D1
            put(px,x,y,c)
    for x in range(W):
        put(px,x,0,L2 if x%7 else L3);put(px,x,1,L1);put(px,x,2,M2)
        put(px,x,13,D2);put(px,x,14,M1)
    return im
def side_v():
    # Left and right sides: the same strip turned, front edge at column 0.
    return side_h().transpose(Image.Transpose.TRANSPOSE)
def inner():
    return Image.new('RGBA',(16,16),DEEP+(255,))
def haze():
    # The glow ring's gradient (D53 revision 7): 32 columns alike, row 0 at
    # the screen's edge, fading out to row 15 at the bezel's edge. Olive
    # white like the screen's text; drawn additively, so it only brightens.
    # Alpha 160 marks the texels semi-transparent (the PS1 STP bit).
    im=Image.new('RGBA',(32,16),(0,0,0,0));px=im.load()
    for r in range(16):
        k=(1-r/16)**1.8
        c=(round(150*k),round(146*k),round(100*k))
        for x in range(32):px[x,r]=(c+(160,)) if max(c)>=8 else (0,0,0,0)
    return im
if __name__=='__main__':
    import sys;out=sys.argv[1] if len(sys.argv)>1 else 'tv2'
    import os;os.makedirs(out,exist_ok=True)
    front().save(f'{out}/front.png');screen(False).save(f'{out}/screen-a.png');screen(True).save(f'{out}/screen-b.png');side_h().save(f'{out}/side-h.png');side_v().save(f'{out}/side-v.png');inner().save(f'{out}/inner.png');haze().save(f'{out}/haze.png')
