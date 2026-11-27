#include <cstring>
#include "ttk_font.h"
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
namespace ttk {bool modern=true;bool input_modernized(){return modern;}}
// D24A: the TTK disc-font pack (build_ttk_fonts.py). argv[1] pack, optional
// argv[2] "invalid" (expects refusal) or a PPM path for a review sheet.
int main(int argc,char**argv) {
    assert(argc>=2);setenv("DNTTK_FONT_PACK",argv[1],1);
    std::vector<uint32_t> image(1024*512);int w=0,h=0;
    if(argc>2 && std::string(argv[2])=="invalid") {
        assert(!ttk_font_rasterize("God Mode: On",0,image.data(),1024,512,624,&w,&h));return 0;
    }
    const char* texts[]={"God Mode: On","God Mode: Off","Giving Everything!","Got All Inventory","Got All Weapons/Ammo","Got All Keys","Steroids","Monsters: Off","Monsters: On","Play Zaxtor's Oblivion!","Ammo: 0123456789 !?.,:;+-/()[]", "Missing: \xc3\xa9 \xe2\x98\x83", "Long message with words that must wrap safely within a small window.","> spawn skeleton key","D26L: #$&*+[\\]^_{|}~` \"quoted\" 'single'"};
    for(int width:{304,624,1008})for(int style:{0,1,2,3})for(const char* text:texts) {
        if(!ttk_font_rasterize(text,style,image.data(),1024,512,width,&w,&h)){std::fprintf(stderr,"font failed style=%d width=%d text=%s\n",style,width,text);return 1;}
        assert(w<=width && h<=512);
        if(style==1)assert(w%2==0 && h%2==0); // headings are drawn at 2x
        bool transparent=false,colour=false,shadow=false;
        for(int i=0;i<w*h;++i){transparent|=image[i]==0;colour|=(image[i]&0xffffff)!=0;shadow|=image[i]==0xff0c0c18u;}
        assert(transparent && colour);
        if(style!=1)assert(shadow); // messages and console carry the navy drop shadow
    }
    // D26L1: the layout matches what rasterize draws, byte for byte: drawn bytes
    // advance left to right inside the image, a space dropped at a wrap has no
    // width and sits at its row's end, and the image is exactly the rows' size.
    for(int width:{304,624,1008})for(const char* text:texts) {
        assert(ttk_font_rasterize(text,3,image.data(),1024,512,width,&w,&h));
        const int n=int(std::string(text).size());
        std::vector<int> xs(n),ws(n),rs(n);int lh=0,top=0;
        const int rows=ttk_font_layout(text,3,1024,512,width,xs.data(),ws.data(),rs.data(),n,&lh,&top);
        assert(rows>=1 && lh>0 && top>0);
        assert(h==rows*lh+2*top+top/2);
        int right=0;
        for(int i=0;i<n;++i) {
            assert(rs[i]>=0 && rs[i]<rows && xs[i]>=top && xs[i]+ws[i]<=w);
            if(i && rs[i]==rs[i-1])assert(xs[i]==xs[i-1]+ws[i-1]);
            if(i)assert(rs[i]>=rs[i-1]);
            right=std::max(right,xs[i]+ws[i]);
        }
        assert(right+top+top/2==w);
    }
    {
        const char* wrapped="Long message with words that must wrap safely within a small window.";
        const int n=int(std::string(wrapped).size());
        std::vector<int> xs(n),ws(n),rs(n);int lh=0,top=0;
        const int rows=ttk_font_layout(wrapped,3,1024,512,304,xs.data(),ws.data(),rs.data(),n,&lh,&top);
        assert(rows>=2);
        for(int i=1;i<n;++i)if(rs[i]!=rs[i-1]){assert(wrapped[i-1]==' ' && ws[i-1]==0);}
        ttk::modern=false;assert(!ttk_font_layout(wrapped,3,1024,512,304,xs.data(),ws.data(),rs.data(),n,&lh,&top));ttk::modern=true;
    }
    // D26L: '`' is a backquote, not D19T's opening quote (glyph 127).
    {
        std::vector<uint32_t> a(1024*512),b(1024*512);int aw=0,ah=0,bw=0,bh=0;
        assert(ttk_font_rasterize("`",3,a.data(),1024,512,624,&aw,&ah));
        assert(ttk_font_rasterize("\"x",3,b.data(),1024,512,624,&bw,&bh));
        assert(aw!=bw || !std::equal(a.begin(),a.begin()+aw*ah,b.begin()));
    }
    // Lower case is drawn as upper case (all caps).
    std::vector<uint32_t> lower(1024*512),upper(1024*512);int lw=0,lh=0,uw=0,uh=0;
    assert(ttk_font_rasterize("spawn steroids",3,lower.data(),1024,512,624,&lw,&lh));
    assert(ttk_font_rasterize("SPAWN STEROIDS",3,upper.data(),1024,512,624,&uw,&uh));
    assert(lw==uw && lh==uh && std::equal(lower.begin(),lower.begin()+lw*lh,upper.begin()));
    assert(ttk_font_rasterize("God Mode: On",0,image.data(),1024,512,624,&w,&h));auto reference=image;
    assert(ttk_font_rasterize("God Mode: On",0,image.data(),1024,512,624,&w,&h));assert(image==reference);
    ttk::modern=false;assert(!ttk_font_rasterize("God Mode: On",0,image.data(),1024,512,624,&w,&h));ttk::modern=true;
    {   // D26L2: the console's entry points, in both profiles, tinted per byte.
        const char* line="goto room 12";const int n=int(std::strlen(line));
        std::vector<uint32_t> plain(1024*64),tinted(1024*64),vanilla(1024*64);int pw=0,ph=0,tw=0,th=0,vw=0,vh=0;
        assert(ttk_font_rasterize("goto room 12",3,image.data(),1024,512,1264,&w,&h));
        assert(ttk_font_rasterize_console(line,3,plain.data(),1024,64,1264,&pw,&ph,nullptr,0) && pw==w && ph==h);
        assert(std::equal(plain.begin(),plain.begin()+pw*ph,image.begin()));   // untinted: the same picture
        std::vector<uint32_t> tints(size_t(n),0u);for(int i=0;i<4;++i)tints[size_t(i)]=0xfff4bc48u;
        assert(ttk_font_rasterize_console(line,3,tinted.data(),1024,64,1264,&tw,&th,tints.data(),n) && tw==pw && th==ph);
        int changed=0,alpha_kept=1;
        for(int i=0;i<pw*ph;++i)if(plain[size_t(i)]!=tinted[size_t(i)]) {
            ++changed;
            if((plain[size_t(i)]>>24)!=(tinted[size_t(i)]>>24))alpha_kept=0;
        }
        assert(changed>0 && alpha_kept);
        // Only "goto" changes: nothing right of its last glyph.
        int xs[16],ws[16],rs[16],lh=0,top=0;
        assert(ttk_font_layout_console(line,3,1024,64,1264,xs,ws,rs,n,&lh,&top)==1);
        for(int y=0;y<ph;++y)for(int x=xs[5];x<pw;++x)assert(plain[size_t(y*pw+x)]==tinted[size_t(y*pw+x)]);
        ttk::modern=false;
        assert(ttk_font_rasterize_console(line,3,vanilla.data(),1024,64,1264,&vw,&vh,nullptr,0) && vw==pw);
        assert(ttk_font_layout_console(line,3,1024,64,1264,xs,ws,rs,n,&lh,&top)==1);
        ttk::modern=true;
    }
    if(argc>2) {
        std::vector<uint32_t> sheet(640*1100,0xff303030);
        int y=8;
        for(int style:{1,0,3})for(const char* text:texts) {
            assert(ttk_font_rasterize(text,style,image.data(),1024,512,624,&w,&h));
            if(y+h>=1100)break;
            for(int iy=0;iy<h;++iy)for(int ix=0;ix<w;++ix)if(image[iy*w+ix]>>24)sheet[(y+iy)*640+8+ix]=image[iy*w+ix];
            y+=h+4;
        }
        std::ofstream f(argv[2],std::ios::binary);f<<"P6\n640 1100\n255\n";
        for(auto p:sheet){f.put(p>>16);f.put(p>>8);f.put(p);}
    }
}
