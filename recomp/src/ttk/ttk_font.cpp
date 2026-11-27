// D24A Modernized text renderer: Time to Kill's own fonts from the player's disc
// (tools/local/build_ttk_fonts.py writes ttk-fonts.pack, format TTKFONT3). It
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
// D26L: 96 glyphs, ASCII 32..126 and 127 for the opening double quote (TTKFONT3).
constexpr unsigned max_sets=16,glyph_count=96,set_bytes=8+glyph_count*8;
constexpr char open_quote=char(127);
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
    if(std::memcmp(pack.data(),"TTKFONT3",8) || u32(pack.data()+8)!=pack.size()-16)return false;
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
// One character to draw and the byte of the source text it came from (D26L1:
// the console's caret, selection and mouse need to know where each byte lands).
struct Placed {char c;int src;};
// D19T: '"' opens and closes in turn: the pack's glyph 127 is the opening
// quote (D26L; it was '`', which is now a backquote of its own).
std::vector<Placed> ascii_placed(const char* text) {
    std::vector<Placed> result;bool open=true;
    const unsigned char* start=reinterpret_cast<const unsigned char*>(text);
    for(const unsigned char* p=start;*p && result.size()<256;) {
        const int src=int(p-start);
        unsigned c=*p++;
        if(c=='"'){result.push_back({open?open_quote:'"',src});open=!open;}
        else if(c>=32 && c<=126)result.push_back({char(c),src});
        else if(c=='\n')result.push_back({'\n',src});
        else if(c=='\t')result.push_back({' ',src});
        else {
            result.push_back({'?',src});
            // One deliberate fallback for a non-ASCII UTF-8 sequence.
            if(c>=0xc0)while((*p&0xc0)==0x80)++p;
        }
    }
    return result;
}
std::string ascii(const char* text) {
    std::string result;
    for(const Placed& p:ascii_placed(text))result.push_back(p.c);
    return result;
}
// Word wrap shared by drawing and layout: rows of placed characters, breaking at
// the last space that fits (the space is dropped), spaces never start a row.
std::vector<std::vector<Placed>> wrap(const Set& set,const std::vector<Placed>& text,int limit) {
    auto adv=[&](char c){return int(set.glyphs[(unsigned char)c-32].advance);};
    std::vector<std::vector<Placed>> lines;std::vector<Placed> line;int extent=0;
    for(const Placed& p:text) {
        if(p.c=='\n'){lines.push_back(line);line.clear();extent=0;continue;}
        while(extent+adv(p.c)>limit && !line.empty()) {
            size_t space=line.size();
            for(size_t i=line.size();i-->0;)if(line[i].c==' '){space=i;break;}
            if(space!=line.size()) {
                lines.emplace_back(line.begin(),line.begin()+long(space));
                line.erase(line.begin(),line.begin()+long(space)+1);
                extent=0;for(const Placed& d:line)extent+=adv(d.c);
            } else {lines.push_back(line);line.clear();extent=0;}
        }
        if(line.empty() && p.c==' ')continue;
        line.push_back(p);extent+=adv(p.c);
    }
    lines.push_back(line);
    return lines;
}
struct Laid {const Set* set;int scale,padding,w,h;bool centered;std::vector<std::vector<Placed>> rows;};
// Lays text out exactly as ttk_font_rasterize draws it; false where it would fail.
// any_profile (D26L2): the console, a developer tool, uses the fonts in Vanilla too.
bool lay_out(const char* text,int style,int cap_w,int cap_h,int available_w,Laid& out,bool any_profile=false) {
    if((!any_profile && !ttk::input_modernized()) || !text || style<0 || style>3 || cap_w<32 || cap_h<16 || !load())return false;
    out.centered=style==2;
    out.set=&sets[style==1?1:style==3?2:0];
    out.scale=int(out.set->scale);
    out.padding=style==3?2:3;
    const int limit=std::min(cap_w,available_w)/out.scale-2*out.padding-1;
    if(limit<8)return false;
    out.rows=wrap(*out.set,ascii_placed(text),limit);
    int widest=1;
    for(const auto& l:out.rows) {int n=0;for(const Placed& p:l)n+=int(out.set->glyphs[(unsigned char)p.c-32].advance);widest=std::max(widest,n);}
    // One extra pixel each way holds the drop shadow.
    out.w=(widest+2*out.padding+1)*out.scale;
    out.h=(int(out.rows.size())*int(out.set->line_height)+2*out.padding+1)*out.scale;
    return !(out.w>cap_w || out.w>available_w || out.h>cap_h);
}
int row_width(const Laid& laid,const std::vector<Placed>& row) {
    int n=0;for(const Placed& p:row)n+=int(laid.set->glyphs[(unsigned char)p.c-32].advance);return n;
}
// D26L2: a glyph pixel recoloured by a tint, keeping the font's own shading:
// the steel ramp's luminance scales the tint up to the letters' body (about
// 200 of 255), and the brighter highlights above it lean towards white.
uint32_t tinted(uint32_t colour,uint32_t tint) {
    const int r=int(colour>>16&255),g=int(colour>>8&255),b=int(colour&255);
    const int lum=(r*77+g*150+b*29)>>8,k=lum*256/200;
    auto ch=[&](int shift) {
        const int t=int(tint>>shift&255);
        const int v=k<=256 ? t*k>>8 : t+(255-t)*std::min(k-256,64)*3/(64*10);
        return uint32_t(std::min(v,255));
    };
    return (colour&0xff000000u)|ch(16)<<16|ch(8)<<8|ch(0);
}
int rasterize(const char* text,int style,uint32_t* pixels,int cap_w,int cap_h,int available_w,int* width,int* height,
              const uint32_t* tints,int tint_count,bool any_profile) {
    Laid laid;
    if(!pixels || !width || !height || !lay_out(text,style,cap_w,cap_h,available_w,laid,any_profile))return 0;
    const Set& set=*laid.set;
    const int scale=laid.scale,padding=laid.padding,w=laid.w,h=laid.h;
    std::fill(pixels,pixels+w*h,0u);
    auto pixel=[&](int x,int y,uint32_t colour) {
        if(x>=0 && y>=0 && x<w && y<h)pixels[y*w+x]=colour;
    };
    for(int shadow=set.shadow?1:0;shadow>=0;--shadow)for(size_t row=0;row<laid.rows.size();++row) {
        int x=laid.centered?(w/scale-1-row_width(laid,laid.rows[row]))/2:padding;
        for(const Placed& p:laid.rows[row]) {
            const Glyph& v=set.glyphs[(unsigned char)p.c-32];
            const uint32_t tint=tints && p.src<tint_count ? tints[p.src] : 0u;
            for(unsigned iy=0;iy<v.h;++iy)for(unsigned ix=0;ix<v.w;++ix) {
                uint32_t colour=u32(pack.data()+16+v.offset+4*(iy*v.w+ix));
                if(!(colour>>24))continue;
                if(shadow)colour=set.shadow_colour;
                else if(tint)colour=tinted(colour,tint);
                for(int dy=0;dy<scale;++dy)for(int dx=0;dx<scale;++dx)
                    pixel((x+int(ix)+shadow)*scale+dx,(padding+int(row)*int(set.line_height)+int(v.y+iy)+shadow)*scale+dy,colour);
            }
            x+=int(v.advance);
        }
    }
    *width=w;*height=h;return 1;
}
int layout(const char* text,int style,int cap_w,int cap_h,int available_w,
           int* xs,int* ws,int* rows,int max_bytes,int* line_height,int* top,bool any_profile) {
    Laid laid;
    if(!xs || !ws || !rows || max_bytes<0 || !lay_out(text,style,cap_w,cap_h,available_w,laid,any_profile))return 0;
    const int n=std::min(max_bytes,int(std::strlen(text)));
    for(int i=0;i<n;++i)ws[i]=-1;
    for(size_t row=0;row<laid.rows.size();++row) {
        int x=laid.centered?(laid.w/laid.scale-1-row_width(laid,laid.rows[row]))/2:laid.padding;
        for(const Placed& p:laid.rows[row]) {
            const int adv=int(laid.set->glyphs[(unsigned char)p.c-32].advance);
            if(p.src<n){xs[p.src]=x*laid.scale;ws[p.src]=adv*laid.scale;rows[p.src]=int(row);}
            x+=adv;
        }
    }
    for(int i=0;i<n;++i)if(ws[i]<0) {
        if(i){xs[i]=xs[i-1]+ws[i-1];rows[i]=rows[i-1];}
        else {xs[i]=laid.padding*laid.scale;rows[i]=0;}
        ws[i]=0;
    }
    if(line_height)*line_height=int(laid.set->line_height)*laid.scale;
    if(top)*top=laid.padding*laid.scale;
    return int(laid.rows.size());
}
}
extern "C" int ttk_font_rasterize(const char* text,int style,uint32_t* pixels,int cap_w,
                                  int cap_h,int available_w,int* width,int* height) {
    return rasterize(text,style,pixels,cap_w,cap_h,available_w,width,height,nullptr,0,false);
}
// D26L2: the console's text, in either profile, each byte i drawn in tints[i]
// (0 keeps the font's colour; the drop shadow is never tinted).
extern "C" int ttk_font_rasterize_console(const char* text,int style,uint32_t* pixels,int cap_w,int cap_h,
                                          int available_w,int* width,int* height,const uint32_t* tints,int tint_count) {
    return rasterize(text,style,pixels,cap_w,cap_h,available_w,width,height,tints,tint_count,true);
}
// D26L1: where ttk_font_rasterize puts each byte of `text`, in pixels of its
// image: left edge xs[i], advance ws[i] (0 for a byte not drawn: a space dropped
// at a wrap or a row start, a UTF-8 continuation) and row rows[i]. Row r's top
// is *top + r * *line_height. Returns the row count, or 0 where rasterize fails.
extern "C" int ttk_font_layout(const char* text,int style,int cap_w,int cap_h,int available_w,
                               int* xs,int* ws,int* rows,int max_bytes,int* line_height,int* top) {
    return layout(text,style,cap_w,cap_h,available_w,xs,ws,rows,max_bytes,line_height,top,false);
}
// D26L2: ttk_font_layout for ttk_font_rasterize_console (either profile).
extern "C" int ttk_font_layout_console(const char* text,int style,int cap_w,int cap_h,int available_w,
                                       int* xs,int* ws,int* rows,int max_bytes,int* line_height,int* top) {
    return layout(text,style,cap_w,cap_h,available_w,xs,ws,rows,max_bytes,line_height,top,true);
}
// D24B: one line of text from glyph set `set` straight into an ARGB surface
// (the savestate panel). No Modernized gate: callers decide. Returns the width
// drawn, or -1 without a valid pack or set.
extern "C" int ttk_font_draw(int set,const char* text,uint32_t* dst,int dst_w,int dst_h,int x,int y) {
    return ttk_font_draw_spaced(set,text,dst,dst_w,dst_h,x,y,0);
}
extern "C" int ttk_font_draw_spaced(int set,const char* text,uint32_t* dst,int dst_w,int dst_h,int x,int y,int spacing) {
    if(!text || !dst || !load() || set<0 || unsigned(set)>=set_count)return -1;
    const Set& font=sets[set];
    const int scale=int(font.scale);
    const std::string line=ascii(text);
    int width=0;
    for(char c:line){if(c=='\n')break;width+=int(font.glyphs[(unsigned char)c-32].advance)+spacing;}
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
            cx+=int(v.advance)+spacing;
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

