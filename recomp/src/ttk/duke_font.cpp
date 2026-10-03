#include "duke_font.h"
#include "pc_input.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
namespace {
struct Glyph {unsigned w,h,advance,y,offset;};
std::vector<unsigned char> pack;
Glyph glyphs[190];
std::string loaded_path;
bool loaded;
unsigned u32(const unsigned char* p) {return p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;}
bool load() {
    const char* override_path=std::getenv("DNTTK_FONT_PACK");
    std::string path;
    if(override_path)path=override_path;
    else {
        auto base=SDL_GetBasePath();if(!base)return false;
        path=std::string(base)+"ttk-fonts.pack";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    }
    if(path==loaded_path)return loaded;
    loaded_path=path;loaded=false;pack.clear();
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<16+190*8 || file.tellg()>1024*1024)return false;
    pack.resize(static_cast<size_t>(file.tellg()));file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(pack.data()),pack.size()))return false;
    if(std::memcmp(pack.data(),"TTKFONT1",8) || u32(pack.data()+8)!=pack.size()-16)return false;
    unsigned hash=2166136261u;
    for(size_t i=16;i<pack.size();++i)hash=(hash^pack[i])*16777619u;
    if(hash!=u32(pack.data()+12))return false;
    for(unsigned i=0;i<190;++i) {
        auto p=pack.data()+16+8*i;Glyph g{p[0],p[1],p[2],p[3],u32(p+4)};
        if(g.w>32 || g.h>32 || !g.advance || g.advance>33 || g.y+g.h>32 ||
           g.offset<190*8 || g.offset>pack.size()-16 || g.w*g.h*4>pack.size()-16-g.offset)return false;
        glyphs[i]=g;
    }
    loaded=true;return true;
}
std::string ascii(const char* text) {
    std::string result;
    for(const unsigned char* p=reinterpret_cast<const unsigned char*>(text);*p && result.size()<256;) {
        unsigned c=*p++;
        if(c>=32 && c<=126)result.push_back(char(c));
        else if(c=='\n')result.push_back('\n');
        else if(c=='\t')result.push_back(' ');
        else {
            result.push_back('?');
            // One deliberate fallback for a non-ASCII UTF-8 sequence.
            if(c>=0xc0)while((*p&0xc0)==0x80)++p;
        }
    }
    return result;
}
}
extern "C" int ttk_font_rasterize(const char* text,int style,uint32_t* pixels,int cap_w,
                                  int cap_h,int available_w,int* width,int* height) {
    if(!ttk::input_modernized() || !text || !pixels || !width || !height ||
       style<0 || style>3 || cap_w<32 || cap_h<16 || !load())return 0;
    const bool centered=style==2;
    const bool small=style==3;
    const int scale=small?1:2;
    if(centered || small)style=0;
    const int padding=small?2:3;
    int limit=std::min(cap_w,available_w)/scale-2*padding;
    if(limit<8)return 0;
    const int line_height=style?18:10;
    auto g=[&](unsigned char c)->const Glyph& {return glyphs[style*95+c-32];};
    std::vector<std::string> lines;std::string line;int extent=0;
    for(char c:ascii(text)) {
        if(c=='\n'){lines.push_back(line);line.clear();extent=0;continue;}
        while(extent+int(g(c).advance)>limit && !line.empty()) {
            auto space=line.find_last_of(' ');
            if(space!=std::string::npos) {
                lines.push_back(line.substr(0,space));line=line.substr(space+1);extent=0;
                for(char d:line)extent+=g(d).advance;
            } else {lines.push_back(line);line.clear();extent=0;}
        }
        if(line.empty() && c==' ')continue;
        line+=c;extent+=g(c).advance;
    }
    lines.push_back(line);
    int w=1;
    // Small console and centered quotes share EDuke32-style tightened advance.
    const int tighten_width=(centered||small)?1:0;
    for(const auto& l:lines){int n=0;for(char c:l)n+=std::max(1,(int)g(c).advance-tighten_width);w=std::max(w,n);}
    w=(w+2*padding)*scale;int h=(int(lines.size())*line_height+2*padding)*scale;
    if(w>cap_w || w>available_w || h>cap_h)return 0;
    std::fill(pixels,pixels+w*h,0u);
    auto pixel=[&](int x,int y,uint32_t colour) {
        if(x>=0 && y>=0 && x<w && y<h)pixels[y*w+x]=colour;
    };
    // Glyph transparency and palette remain original. A small host shadow keeps
    // the light message font legible on bright scenery without a solid box.
    const int tighten=(centered||small)?1:0;
    for(int shadow=1;shadow>=0;--shadow)for(size_t row=0;row<lines.size();++row) {
        int line_width=0;for(char c:lines[row])line_width+=std::max(1,(int)g(c).advance-tighten);
        int x=centered?(w/scale-line_width)/2:padding;
        for(char c:lines[row]) {
            auto v=g(c);
            for(unsigned iy=0;iy<v.h;++iy)for(unsigned ix=0;ix<v.w;++ix) {
                uint32_t colour=u32(pack.data()+16+v.offset+4*(iy*v.w+ix));
                if(!(colour>>24))continue;
                for(int dy=0;dy<scale;++dy)for(int dx=0;dx<scale;++dx)
                    pixel((x+int(ix)+shadow)*scale+dx,(padding+int(row)*line_height+v.y+iy+shadow)*scale+dy,shadow?0xd0000000u:colour);
            }
            x+=std::max(1,(int)v.advance-tighten);
        }
    }
    *width=w;*height=h;return 1;
}
