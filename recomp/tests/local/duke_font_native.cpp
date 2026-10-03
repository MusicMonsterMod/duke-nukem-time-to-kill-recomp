#include "duke_font.h"
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
namespace ttk {bool modern=true;bool input_modernized(){return modern;}}
int main(int argc,char**argv) {
    assert(argc>=2);setenv("DNTTK_FONT_PACK",argv[1],1);
    std::vector<uint32_t> image(1024*512);int w=0,h=0;
    if(argc>2 && std::string(argv[2])=="invalid") {
        assert(!ttk_font_rasterize("God Mode: On",0,image.data(),1024,512,624,&w,&h));return 0;
    }
    const char* texts[]={"God Mode: On","God Mode: Off","Giving Everything!","Got All Inventory","Got All Weapons/Ammo","Got All Keys","Steroids","Monsters: Off","Monsters: On","Ammo: 0123456789 !?.,:;+-/()[]", "Missing: \xc3\xa9 \xe2\x98\x83", "Long message with words that must wrap safely within a small window."};
    for(int width:{304,624,1008})for(int style:{0,1,2})for(const char* text:texts) {
        if(!ttk_font_rasterize(text,style,image.data(),1024,512,width,&w,&h)){std::fprintf(stderr,"font failed style=%d width=%d text=%s\n",style,width,text);return 1;}assert(w<=width && h<=512 && w%2==0 && h%2==0);
        bool transparent=false,colour=false;
        for(int i=0;i<w*h;++i){transparent|=image[i]==0;colour|=(image[i]&0xffffff)!=0;}
        assert(transparent && colour);
    }
    assert(ttk_font_rasterize("God Mode: On",0,image.data(),1024,512,624,&w,&h));auto reference=image;
    assert(ttk_font_rasterize("God Mode: On",0,image.data(),1024,512,624,&w,&h));assert(image==reference);
    ttk::modern=false;assert(!ttk_font_rasterize("God Mode: On",0,image.data(),1024,512,624,&w,&h));ttk::modern=true;
    if(argc>2) {
        std::vector<uint32_t> sheet(640*1100,0xff303030);
        int y=8;
        for(int style:{1,0})for(const char* text:texts) {
            assert(ttk_font_rasterize(text,style,image.data(),1024,512,624,&w,&h));
            if(y+h>=1100)break;
            for(int iy=0;iy<h;++iy)for(int ix=0;ix<w;++ix)if(image[iy*w+ix]>>24)sheet[(y+iy)*640+8+ix]=image[iy*w+ix];
            y+=h+4;
        }
        std::ofstream f(argv[2],std::ios::binary);f<<"P6\n640 1100\n255\n";
        for(auto p:sheet){f.put(p>>16);f.put(p>>8);f.put(p);}
    }
}
