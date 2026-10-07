// D24A Modernized text renderer: Time to Kill's own fonts from the player's disc
// (tools/local/build_ttk_fonts.py writes ttk-fonts.pack, format TTKFONT2). It
// replaced the Duke Nukem 3D message/Atomic glyph pack. Glyph colours, spacing,
// scale and the drop shadow come from the pack; this file only lays text out.
#include "ttk_font.h"
#include "pc_input.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
namespace {
constexpr unsigned max_sets=12,glyph_count=95,set_bytes=8+glyph_count*8;
struct Glyph {unsigned w,h,advance,y,offset;};
struct Set {unsigned line_height,scale;bool shadow;uint32_t shadow_colour;Glyph glyphs[glyph_count];};
std::vector<unsigned char> pack;
Set sets[max_sets];
unsigned set_count;
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
    if(!file || file.tellg()<16+4+3*set_bytes || file.tellg()>1024*1024)return false;
    pack.resize(static_cast<size_t>(file.tellg()));file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(pack.data()),pack.size()))return false;
    if(std::memcmp(pack.data(),"TTKFONT2",8) || u32(pack.data()+8)!=pack.size()-16)return false;
    unsigned hash=2166136261u;
    for(size_t i=16;i<pack.size();++i)hash=(hash^pack[i])*16777619u;
    if(hash!=u32(pack.data()+12))return false;
    const unsigned char* payload=pack.data()+16;
    const size_t payload_size=pack.size()-16;
    set_count=u32(payload);
    if(set_count<3 || set_count>max_sets || 4+set_count*set_bytes>payload_size)return false;
    for(unsigned s=0;s<set_count;++s) {
        const unsigned char* p=payload+4+s*set_bytes;
        Set& set=sets[s];
        set.line_height=p[0];set.scale=p[1];set.shadow=p[2]!=0;set.shadow_colour=u32(p+4);
        if(set.line_height<4 || set.line_height>40 || set.scale<1 || set.scale>2)return false;
        for(unsigned i=0;i<glyph_count;++i) {
            const unsigned char* q=p+8+8*i;
            Glyph g{q[0],q[1],q[2],q[3],u32(q+4)};
            if(!g.w || !g.h || g.w>32 || g.h>32 || !g.advance || g.advance>40 || g.y+g.h>set.line_height ||
               g.offset<4+set_count*set_bytes || g.offset>payload_size || size_t(g.w)*g.h*4>payload_size-g.offset)return false;
            set.glyphs[i]=g;
        }
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
    const Set& set=sets[style==1?1:style==3?2:0];
    const int scale=int(set.scale);
    const int padding=style==3?2:3;
    const int limit=std::min(cap_w,available_w)/scale-2*padding-1;
    if(limit<8)return 0;
    auto g=[&](char c)->const Glyph& {return set.glyphs[(unsigned char)c-32];};
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
    auto line_width=[&](const std::string& l){int n=0;for(char c:l)n+=g(c).advance;return n;};
    int widest=1;
    for(const auto& l:lines)widest=std::max(widest,line_width(l));
    // One extra pixel each way holds the drop shadow.
    const int w=(widest+2*padding+1)*scale,h=(int(lines.size())*int(set.line_height)+2*padding+1)*scale;
    if(w>cap_w || w>available_w || h>cap_h)return 0;
    std::fill(pixels,pixels+w*h,0u);
    auto pixel=[&](int x,int y,uint32_t colour) {
        if(x>=0 && y>=0 && x<w && y<h)pixels[y*w+x]=colour;
    };
    for(int shadow=set.shadow?1:0;shadow>=0;--shadow)for(size_t row=0;row<lines.size();++row) {
        int x=centered?(w/scale-1-line_width(lines[row]))/2:padding;
        for(char c:lines[row]) {
            const Glyph& v=g(c);
            for(unsigned iy=0;iy<v.h;++iy)for(unsigned ix=0;ix<v.w;++ix) {
                const uint32_t colour=u32(pack.data()+16+v.offset+4*(iy*v.w+ix));
                if(!(colour>>24))continue;
                for(int dy=0;dy<scale;++dy)for(int dx=0;dx<scale;++dx)
                    pixel((x+int(ix)+shadow)*scale+dx,(padding+int(row)*int(set.line_height)+int(v.y+iy)+shadow)*scale+dy,
                          shadow?set.shadow_colour:colour);
            }
            x+=int(v.advance);
        }
    }
    *width=w;*height=h;return 1;
}
// D24B: one line of text from glyph set `set` straight into an ARGB surface
// (the savestate panel). No Modernized gate: callers decide. Returns the width
// drawn, or -1 without a valid pack or set.
extern "C" int ttk_font_draw(int set,const char* text,uint32_t* dst,int dst_w,int dst_h,int x,int y) {
    if(!text || !dst || !load() || set<0 || unsigned(set)>=set_count)return -1;
    const Set& font=sets[set];
    const int scale=int(font.scale);
    const std::string line=ascii(text);
    int width=0;
    for(char c:line){if(c=='\n')break;width+=int(font.glyphs[(unsigned char)c-32].advance);}
    auto pixel=[&](int px,int py,uint32_t colour){if(px>=0 && py>=0 && px<dst_w && py<dst_h)dst[py*dst_w+px]=colour;};
    for(int shadow=font.shadow?1:0;shadow>=0;--shadow) {
        int cx=0;
        for(char c:line) {
            if(c=='\n')break;
            const Glyph& v=font.glyphs[(unsigned char)c-32];
            for(unsigned iy=0;iy<v.h;++iy)for(unsigned ix=0;ix<v.w;++ix) {
                const uint32_t colour=u32(pack.data()+16+v.offset+4*(iy*v.w+ix));
                if(!(colour>>24))continue;
                for(int dy=0;dy<scale;++dy)for(int dx=0;dx<scale;++dx)
                    pixel(x+(cx+int(ix)+shadow)*scale+dx,y+(int(v.y+iy)+shadow)*scale+dy,shadow?font.shadow_colour:colour);
            }
            cx+=int(v.advance);
        }
    }
    return width*scale;
}
extern "C" int ttk_font_text_width(int set,const char* text) {
    if(!text || !load() || set<0 || unsigned(set)>=set_count)return -1;
    int width=0;
    for(char c:ascii(text)){if(c=='\n')break;width+=int(sets[set].glyphs[(unsigned char)c-32].advance);}
    return width*int(sets[set].scale);
}
extern "C" int ttk_font_line_height(int set) {
    if(!load() || set<0 || unsigned(set)>=set_count)return 0;
    return int(sets[set].line_height*sets[set].scale);
}

