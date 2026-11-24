// D17AO1: Duke Hill's sky behind painted skylights (sky_borrow.h). See
// documentation/398-d17ao1-borrowed-sky.md.
//
// The composition 0x80026164 calls 0x8001fc44(0) (ra 0x800265c4) and then the
// sky routine 0x800388e4 only when the render requested sky (sky_widescreen
// sky_background forces that request while a painted face was left out); it
// calls 0x8002e850 next (ra 0x800265d4). For that one sky call the routine's
// level words are swapped: the four layer selectors and the fill colour
// (0x800da344..0x800da46c, note 92) take Duke Hill's values and the model
// table pointer 0x800ddb18 points just before a guest copy of Duke Hill's
// four meshes, so its entries for 0x611..0x614 are those meshes. All six
// words are restored before 0x8002e850, so a savestate or any other code never
// sees them; the routine, its timer, matrices and draw calls are unchanged.
// The near-clip host path draws the copy's polygons with host vertices marked
// for the runtime's second texture bank (psx_mod_gpu_host_vertex_foreign):
// Duke Hill's VRAM image from the pack, not the level's own textures. Retail
// packets can never carry that mark (an earlier CLUT bit 15 marker could:
// level 3's own models set it).
//
// The meshes come from build-local/ttk-sky.pack (build_ttk_sky.py, from the
// owned disc). No pack, Vanilla, two players, a level with its own sky or an
// unauthenticated routine: nothing changes. DNTTK_SKY_BORROW=0 (developer)
// turns it off.
#include "sky_borrow.h"
#include "ttk_register.h"
#include "security_monitor.h"
#include "prop_lights.h"
#include "sky_render.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "pc_input.h"
#include "modern_controls.h"
#if defined(PSX_SDL3)
#include <SDL3/SDL.h>
#else
#include <SDL.h>
#endif
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

extern "C" uint8_t* g_psx_ram;
extern "C" uint32_t g_dirty_ram_code_gen;

namespace ttk {
bool host_visibility_room_shown(uint32_t room);   // D17AT6 (host_visibility.inc)
namespace {
// D17BP: the rooms with an open shaft top, found from the level's mesh once a
// level (and after a load). A world face's corners: room origin + grid bytes
// (x and z << 10, y << 7), as level_lights.inc decodes them. An edge used by
// one face only is an opening's edge; it is a shaft rim when it is level (both
// ends at one height) and its face is a wall (every corner in the edge's
// vertical plane) going down from it. A room with shaft_rim_min such edges at
// one height, at the room's top, has an open shaft top: the borrowed sky is
// drawn while it is shown. Developer: DNTTK_SKY_SHAFT_TRACE=1 lists them.
constexpr unsigned shaft_rim_min=3;
std::vector<uint8_t> shaft_rooms;   // 1: the room has an open shaft top
uint32_t shaft_level=~0u,shaft_loads=~0u;
// D17BP rev 2: roof gaps closed with rock. A gap in a roof (a loop of
// unshared edges, near level, in the upper quarter of its room, every corner
// on the room's grid, round one missing grid point: GOLD AND GUNS room 16, a
// missing cell of the cavern roof) is filled by a cap piece in mod memory,
// in the game's own world-piece format: the gap's corners plus that grid
// point (at their mean height), a fan of triangles each within one 1024
// tile, each textured like the roof face across its edge (an entry of the
// level's texture table with the same page and palette whose corners fit the
// tile) and painted with the roof's own vertex colours. host_visibility.inc
// draws a room's caps with its pieces (Polymorph); Classic and Vanilla are
// unchanged. Developer: DNTTK_ROOF_CAPS=0 leaves the gaps open.
void find_shafts(uint32_t level) {
    const auto t0=std::chrono::steady_clock::now();
    shaft_rooms.clear();
    std::vector<std::pair<uint32_t,uint32_t>> caps;   // room, piece
    const bool build=roof_cap_block && !psx_mod_replay_active();   // the live game thread only
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    const uint32_t textures=psx_mod_read_word(0x800d67a8u+0x58);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) {if(build) psx_mod_write_word(roof_cap_block,~0u);return;}
    shaft_rooms.assign(rooms,0);
    struct P {int32_t x,y,z;bool operator<(const P& o) const {return x!=o.x?x<o.x:y!=o.y?y<o.y:z<o.z;}bool operator==(const P& o) const {return x==o.x && y==o.y && z==o.z;}};
    struct Edge {P a,b;unsigned face;};
    struct FaceInfo {int16_t tex;uint16_t flags;uint32_t piece;};
    std::vector<int16_t> level_tex;   // every texture index the level's faces use
    std::vector<bool> tex_seen(65536,false);
    struct Pending {uint32_t room;int32_t org[3];std::vector<P> ring;std::vector<int16_t> tex;std::vector<uint16_t> flags;std::vector<uint32_t> rgb;uint32_t header_from;};
    std::vector<Pending> pending;
    const bool caps_on=build && !(std::getenv("DNTTK_ROOF_CAPS") && !std::strcmp(std::getenv("DNTTK_ROOF_CAPS"),"0"));
    const bool trace=std::getenv("DNTTK_SKY_SHAFT_TRACE")!=nullptr;
    for(uint32_t r=0;r<rooms;++r) {
        const uint32_t a=table+88*r;
        const int32_t org[3]={(int32_t)psx_mod_read_word(a),(int32_t)psx_mod_read_word(a+4),(int32_t)psx_mod_read_word(a+8)};
        const unsigned n=psx_mod_read_byte(a+0x32);const uint32_t list=psx_mod_read_word(a+0x40);
        if(!n || list<0x80010000u || list>0x80200000u-n*4 || (list&3)) continue;
        std::vector<Edge> edges;std::vector<std::array<P,4>> faces;std::vector<unsigned> corners;std::vector<FaceInfo> info;
        std::vector<std::pair<P,uint32_t>> colour;   // a corner's painted colour (w1)
        uint32_t first_piece=0;
        for(unsigned i=0;i<n;++i) {
            const uint32_t piece=psx_mod_read_word(list+4*i);
            if(piece<0x80010000u || piece>0x801f0000u || (piece&3)) continue;
            const unsigned count=psx_mod_read_half(piece+0x14);
            if(!count || count>2048 || piece+0x1c+8*count>0x80200000u) continue;
            if(!first_piece) first_piece=piece;
            uint32_t cursor=piece+psx_mod_read_word(piece+0x18);
            if(cursor<0x80010000u || cursor>0x801ffff0u) continue;
            for(unsigned guard=0;guard<=256;++guard) {
                const uint32_t header=psx_mod_read_word(cursor);cursor+=4;
                const uint32_t type=header&0xffff,m=header>>16;
                if(type==0xff || (type!=0x60 && type!=0x61) || !m || m>4096 || cursor+m*8>0x80200000u) break;
                for(uint32_t k=0;k<m;++k,cursor+=8) {
                    const uint32_t word=psx_mod_read_word(cursor);const unsigned nc=type==0x60?3u:4u;
                    std::array<P,4> f{};bool ok=true;
                    for(unsigned j=0;j<nc;++j) {
                        const unsigned idx=(word>>(8*j))&0xff;
                        if(idx>=count) {ok=false;break;}
                        const uint32_t w0=psx_mod_read_word(piece+0x1c+8*idx);
                        f[j]={org[0]+(int16_t)(((w0&0xff)<<10)&0xffff),org[1]+(int16_t)((((w0>>8)&0xff)<<7)&0xffff),org[2]+(int16_t)((((w0>>16)&0xff)<<10)&0xffff)};
                        colour.push_back({f[j],psx_mod_read_word(piece+0x20+8*idx)&0xffffffu});
                    }
                    if(!ok) continue;
                    const unsigned id=(unsigned)faces.size();faces.push_back(f);corners.push_back(nc);
                    const int16_t tex=(int16_t)psx_mod_read_half(cursor+4);
                    info.push_back({tex,psx_mod_read_half(cursor+6),piece});
                    if(!tex_seen[(uint16_t)tex]) {tex_seen[(uint16_t)tex]=true;level_tex.push_back(tex);}
                    // Around the face: a GT4's corners run 0 1 3 2.
                    const unsigned ring[4]={0,1,nc==4?3u:2u,2};
                    for(unsigned e=0;e<nc;++e) {
                        P p=f[ring[e]],q=f[ring[(e+1)%nc]];
                        if(p==q) continue;
                        if(q<p) std::swap(p,q);
                        edges.push_back({p,q,id});
                    }
                }
            }
        }
        std::sort(edges.begin(),edges.end(),[](const Edge& x,const Edge& y){return x.a<y.a || (x.a==y.a && x.b<y.b);});
        std::vector<std::pair<int32_t,unsigned>> rims;   // height, edges
        std::vector<Edge> open;   // unshared edges that are not shaft rims
        for(size_t i=0;i<edges.size();) {
            size_t j=i+1;while(j<edges.size() && edges[j].a==edges[i].a && edges[j].b==edges[i].b) ++j;
            const Edge& e=edges[i];
            bool rim=false;
            if(j==i+1 && e.a.y==e.b.y) {
                // Its face a wall going down from the edge?
                const auto& f=faces[e.face];bool wall=true,below=false;
                const double dx=e.b.x-e.a.x,dz=e.b.z-e.a.z,len=std::sqrt(dx*dx+dz*dz);
                for(unsigned c=0;c<corners[e.face] && wall;++c) {
                    const double cross=(f[c].x-e.a.x)*dz-(f[c].z-e.a.z)*dx;
                    wall=len>0 && std::fabs(cross)/len<=64;
                    if(f[c].y<e.a.y) wall=false;   // nothing of the face above the rim (up is -y)
                    below|=f[c].y>e.a.y;
                }
                if(wall && below) {
                    rim=true;bool found=false;
                    for(auto& h:rims) if(h.first==e.a.y) {++h.second;found=true;}
                    if(!found) rims.push_back({e.a.y,1});
                }
            }
            if(j==i+1 && !rim) open.push_back(e);
            i=j;
        }
        // A shaft top is the room's highest point (a rim at the floor is a pit going down).
        int32_t top=INT32_MAX,bottom=INT32_MIN;
        for(size_t i=0;i<faces.size();++i) for(unsigned c=0;c<corners[i];++c) {top=std::min(top,faces[i][c].y);bottom=std::max(bottom,faces[i][c].y);}
        for(const auto& h:rims) if(h.second>=shaft_rim_min && h.first<=top+256) {
            shaft_rooms[r]=1;
            if(trace) std::fprintf(stderr,"[TTK sky] level %u room %u: open shaft top at y %d (%u rim edges)\n",level,r,h.first,h.second);
        }
        if(!caps_on) continue;
        // Roof gaps: closed loops of the open edges (every corner on two of them).
        std::vector<bool> used(open.size(),false);
        for(size_t s0=0;s0<open.size();++s0) {
            if(used[s0]) continue;
            std::vector<P> ring{open[s0].a};std::vector<size_t> loop{s0};used[s0]=true;
            P at=open[s0].b;bool closed=false;
            for(unsigned guard=0;guard<16;++guard) {
                if(at==ring[0]) {closed=true;break;}
                ring.push_back(at);
                size_t next=open.size();unsigned degree=0;
                for(size_t k=0;k<open.size();++k) if(!used[k] && (open[k].a==at || open[k].b==at)) {++degree;next=k;}
                if(degree!=1) break;
                used[next]=true;loop.push_back(next);at=open[next].a==at?open[next].b:open[next].a;
            }
            if(!closed || ring.size()<3) continue;
            int32_t lo[3]={INT32_MAX,INT32_MAX,INT32_MAX},hi[3]={INT32_MIN,INT32_MIN,INT32_MIN};double my=0;bool grid=true;
            for(const P& q:ring) {
                const int32_t v[3]={q.x,q.y,q.z};
                for(int k=0;k<3;++k) {lo[k]=std::min(lo[k],v[k]);hi[k]=std::max(hi[k],v[k]);}
                grid&=(q.x-org[0])%1024==0 && (q.z-org[2])%1024==0 && (q.y-org[1])%128==0;my+=q.y;
            }
            my/=ring.size();
            const bool roof=grid && hi[1]-lo[1]<=1024 && hi[0]-lo[0]>=1024 && hi[2]-lo[2]>=1024 && my<=top+0.25*(bottom-top);
            // The missing grid point: the ring's middle, snapped; every corner within one tile of it.
            const int32_t cx=org[0]+(int32_t)std::lround(((hi[0]+lo[0])/2.0-org[0])/1024.0)*1024,cz=org[2]+(int32_t)std::lround(((hi[2]+lo[2])/2.0-org[2])/1024.0)*1024;
            bool ring_ok=roof;
            for(const P& q:ring) ring_ok&=std::abs(q.x-cx)<=1024 && std::abs(q.z-cz)<=1024 && !(q.x==cx && q.z==cz);
            if(trace) std::fprintf(stderr,"[TTK sky] level %u room %u: open loop of %zu edges, x %d..%d y %d..%d z %d..%d: %s\n",level,r,ring.size(),lo[0],hi[0],lo[1],hi[1],lo[2],hi[2],
                                   ring_ok?"a roof gap, capped":roof?"a roof gap, not round one grid point (left open)":"not a roof gap");
            if(!ring_ok) continue;
            Pending pd{};pd.room=r;std::copy(org,org+3,pd.org);pd.ring=ring;pd.header_from=first_piece;
            for(size_t e=0;e<ring.size();++e) {
                const Edge& oe=open[loop[e]];   // edge ring[e] -> ring[e+1]
                pd.tex.push_back(info[oe.face].tex);pd.flags.push_back(info[oe.face].flags);
            }
            for(const P& q:ring) {uint32_t c=0x808080;for(const auto& pc:colour) if(pc.first==q) {c=pc.second;break;}pd.rgb.push_back(c);}
            pending.push_back(pd);
        }
    }
    // Build the caps (mod memory, kept for the level).
    for(const Pending& pd:pending) {
        const size_t nr=pd.ring.size();
        int64_t sy=0;for(const P& q:pd.ring) sy+=q.y;
        const int32_t cy=pd.org[1]+(int32_t)std::lround(((double)sy/nr-pd.org[1])/128.0)*128;
        int32_t cxz[2]={0,0};
        {   int32_t lo[2]={INT32_MAX,INT32_MAX},hi[2]={INT32_MIN,INT32_MIN};
            for(const P& q:pd.ring) {lo[0]=std::min(lo[0],q.x);hi[0]=std::max(hi[0],q.x);lo[1]=std::min(lo[1],q.z);hi[1]=std::max(hi[1],q.z);}
            cxz[0]=pd.org[0]+(int32_t)std::lround(((hi[0]+lo[0])/2.0-pd.org[0])/1024.0)*1024;
            cxz[1]=pd.org[2]+(int32_t)std::lround(((hi[1]+lo[1])/2.0-pd.org[2])/1024.0)*1024;}
        std::vector<P> verts=pd.ring;verts.push_back({cxz[0],cy,cxz[1]});
        std::vector<uint32_t> rgb=pd.rgb;
        {uint32_t s[3]={0,0,0};for(uint32_t c:pd.rgb) for(int k=0;k<3;++k) s[k]+=(c>>(8*k))&0xff;rgb.push_back((s[0]/nr)|(s[1]/nr)<<8|(s[2]/nr)<<16);}
        // Encode a corner (room-relative grid bytes); false when it does not decode back.
        auto encode=[&](const P& q,uint32_t& w0)->bool{
            const int32_t rx=q.x-pd.org[0],ry=q.y-pd.org[1],rz=q.z-pd.org[2];
            const uint32_t xb=((uint32_t)rx>>10)&0xff,yb=((uint32_t)ry>>7)&0xff,zb=((uint32_t)rz>>10)&0xff;
            w0=xb|yb<<8|zb<<16;
            return (int16_t)((xb<<10)&0xffff)==rx && (int16_t)((yb<<7)&0xffff)==ry && (int16_t)((zb<<10)&0xffff)==rz;
        };
        bool ok=true;std::vector<uint32_t> w0s;
        for(const P& q:verts) {uint32_t w;ok&=encode(q,w);w0s.push_back(w);}
        // Each triangle (centre, ring[e], ring[e+1]) is three corners of one tile:
        // find a texture entry with the neighbour's page and palette whose first
        // three corners are the tile corners these take (any mirror of the tile).
        struct Tri {uint8_t i[3];int16_t tex;uint16_t flags;};
        std::vector<Tri> tris;
        const unsigned c=(unsigned)nr;
        for(size_t e=0;e<nr && ok;++e) {
            const unsigned vi[3]={c,(unsigned)e,(unsigned)((e+1)%nr)};
            const int16_t nt=pd.tex[e];
            const uint32_t ne=textures+(uint32_t)(nt*12);
            const uint32_t n1=psx_mod_read_word(ne),n2=psx_mod_read_word(ne+4),n3=psx_mod_read_word(ne+8);
            const uint32_t clut=n1>>16,page=n2>>16,nuv[4]={n1&0xffff,n2&0xffff,n3&0xffff,n3>>16};
            int umin=255,umax=0,vmin=255,vmax=0;
            for(int q3=0;q3<3;++q3) {const uint32_t q=nuv[q3];   // a triangle's entry has no fourth corner (0)
                umin=std::min<int>(umin,q&0xff);umax=std::max<int>(umax,q&0xff);vmin=std::min<int>(vmin,q>>8);vmax=std::max<int>(vmax,q>>8);}
            int32_t tx0=INT32_MAX,tz0=INT32_MAX;for(unsigned k:vi) {tx0=std::min(tx0,verts[k].x);tz0=std::min(tz0,verts[k].z);}
            bool found=false;Tri t{};
            for(int variant=0;variant<8 && !found;++variant) {
                uint32_t want[3];
                for(int k=0;k<3;++k) {
                    int ax=(verts[vi[k]].x-tx0)/1024,az=(verts[vi[k]].z-tz0)/1024;   // 0 or 1
                    if(variant&4) std::swap(ax,az);
                    if(variant&1) ax^=1;
                    if(variant&2) az^=1;
                    want[k]=(uint32_t)(ax?umax:umin)|(uint32_t)(az?vmax:vmin)<<8;
                }
                static const int perm[6][3]={{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
                for(int16_t cand:level_tex) {
                    const uint32_t ce=textures+(uint32_t)(cand*12);
                    const uint32_t c1=psx_mod_read_word(ce),c2=psx_mod_read_word(ce+4),c3=psx_mod_read_word(ce+8);
                    if((c1>>16)!=clut || (c2>>16)!=page) continue;
                    const uint32_t cuv[3]={c1&0xffff,c2&0xffff,c3&0xffff};
                    // Within 2 texels: tiles are painted 0..62 or 0..63 wide.
                    auto near=[](uint32_t x,uint32_t y){return std::abs((int)(x&0xff)-(int)(y&0xff))<=2 && std::abs((int)(x>>8)-(int)(y>>8))<=2;};
                    for(const auto& pm:perm) if(near(cuv[0],want[pm[0]]) && near(cuv[1],want[pm[1]]) && near(cuv[2],want[pm[2]])) {
                        for(int k=0;k<3;++k) t.i[k]=(uint8_t)vi[pm[k]];
                        t.tex=cand;t.flags=(uint16_t)(pd.flags[e]|0x200);found=true;break;
                    }
                    if(found) break;
                }
            }
            if(!found) {ok=false;if(trace) std::fprintf(stderr,"[TTK sky] room %u cap: no texture entry fits the tile of edge %zu (texture %d: page %x clut %x uv %04x %04x %04x %04x; corners %d,%d %d,%d %d,%d)\n",
                pd.room,e,nt,page,clut,nuv[0],nuv[1],nuv[2],nuv[3],verts[vi[0]].x,verts[vi[0]].z,verts[vi[1]].x,verts[vi[1]].z,verts[vi[2]].x,verts[vi[2]].z);break;}
            tris.push_back(t);
        }
        if(!ok) continue;
        // The piece: header from one of the room's own pieces, bounds, corners
        // (two spare records: the vertex pass reads in threes), the list.
        const unsigned nv=(unsigned)verts.size(),read_v=(nv+2)/3*3;
        const uint32_t list_off=(0x1c+8*read_v+15)&~15u,bytes=list_off+4+8*(uint32_t)tris.size()+4;
        static uint32_t cap_used=0;
        if(caps.empty()) cap_used=0x100;
        if(caps.size()>=roof_cap_table_max || cap_used+bytes>roof_cap_block_size) break;
        const uint32_t piece=roof_cap_block+cap_used;cap_used+=(bytes+15)&~15u;
        for(uint32_t k=0;k<0x1c;k+=4) psx_mod_write_word(piece+k,pd.header_from?psx_mod_read_word(pd.header_from+k):0);
        int32_t lo[3]={INT32_MAX,INT32_MAX,INT32_MAX},hi[3]={INT32_MIN,INT32_MIN,INT32_MIN};
        for(const P& q:verts) {const int32_t v[3]={q.x-pd.org[0],q.y-pd.org[1],q.z-pd.org[2]};for(int k=0;k<3;++k) {lo[k]=std::min(lo[k],v[k]);hi[k]=std::max(hi[k],v[k]);}}
        for(int k=0;k<3;++k) {psx_mod_write_half(piece+4+2*k,(uint16_t)(int16_t)lo[k]);psx_mod_write_half(piece+12+2*k,(uint16_t)(int16_t)hi[k]);}
        psx_mod_write_half(piece+0x14,(uint16_t)nv);psx_mod_write_word(piece+0x18,list_off);
        for(unsigned k=0;k<read_v;++k) {
            const unsigned src=k<nv?k:nv-1;
            psx_mod_write_word(piece+0x1c+8*k,w0s[src]);psx_mod_write_word(piece+0x20+8*k,rgb[src]);
        }
        uint32_t cur=piece+list_off;
        psx_mod_write_word(cur,0x60u|(uint32_t)tris.size()<<16);cur+=4;
        for(const Tri& t:tris) {
            psx_mod_write_word(cur,t.i[0]|t.i[1]<<8|t.i[2]<<16);psx_mod_write_half(cur+4,(uint16_t)t.tex);psx_mod_write_half(cur+6,t.flags);cur+=8;
        }
        psx_mod_write_word(cur,0xffu);
        caps.push_back({pd.room,piece});
        if(trace) std::fprintf(stderr,"[TTK sky] level %u room %u: roof gap capped (%u corners, %zu triangles, piece %08x)\n",level,pd.room,nv,tris.size(),piece);
    }
    if(build) {
        psx_mod_write_word(roof_cap_block+4,(uint32_t)caps.size());
        for(size_t i=0;i<caps.size();++i) {psx_mod_write_word(roof_cap_block+8+8*(uint32_t)i,caps[i].first);psx_mod_write_word(roof_cap_block+12+8*(uint32_t)i,caps[i].second);}
        psx_mod_write_word(roof_cap_block,level);
    }
    if(trace) {
        unsigned k=0;for(uint8_t v:shaft_rooms) k+=v;
        std::fprintf(stderr,"[TTK sky] level %u: %u rooms with an open shaft top, %zu roof gaps capped (%.2f ms)\n",level,k,caps.size(),
                     std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count());
    }
}
// Is a room with a shaft top shown (by the host's walk, or the game's own
// portal walk bitset 0x800e1f10 when the host did not walk: Classic)?
bool shaft_shown() {
    static const bool trace=std::getenv("DNTTK_SKY_SHAFT_TRACE")!=nullptr;
    static unsigned last=~0u;
    for(uint32_t r=0;r<shaft_rooms.size();++r) {
        if(!shaft_rooms[r]) continue;
        const unsigned how=host_visibility_room_shown(r)?1u:r<128 && ((psx_mod_read_word(0x800e1f10u+4u*(r>>5))>>(r&31))&1u)?2u:0u;
        if(how) {
            if(trace && last!=(r<<2|how)) {last=r<<2|how;std::fprintf(stderr,"[TTK sky] shaft room %u shown (%s)\n",r,how==1?"host walk":"game walk");}
            return true;
        }
    }
    if(trace && last!=0) {last=0;std::fprintf(stderr,"[TTK sky] no shaft room shown\n");}
    return false;
}
constexpr uint32_t level_index=0x800be570u,table_pointer=0x800ddb18u,players=0x800c27bcu;
// The routine's level words (note 92): fixed ring, slow band, backdrop, fill
// colour, fast band. Duke Hill's values, read from LEVEL01 on a private run.
constexpr uint32_t sky_words[5]={0x800da344u,0x800da45cu,0x800da464u,0x800da468u,0x800da46cu};
constexpr uint32_t duke_hill_words[5]={0x612u,0x613u,0x611u,0x00661f29u,0x614u};
constexpr uint32_t first_id=0x611u,mesh_count=4;

struct Pack {
    bool ok=false;
    uint32_t ids[mesh_count]={};
    std::vector<uint8_t> meshes[mesh_count];
    std::vector<uint16_t> vram;   // 1024x512, Duke Hill's x 512..1023
};
Pack pack;
std::once_flag pack_once;

uint32_t rd32(const std::vector<uint8_t>& b,size_t o) {return b[o]|b[o+1]<<8|b[o+2]<<16|(uint32_t)b[o+3]<<24;}

// Object mesh records (near_clip.cpp object_record_size).
uint32_t record_size(uint32_t type) {
    switch(type) {
        case 0x20: case 0x28: return 8;
        case 0x30: case 0x38: return 12;
        case 0x24: case 0x2c: return 16;
        case 0x34: case 0x3c: return 20;
        default: return 0;
    }
}
void load_pack() {
    std::string path;
    if(const char* p=std::getenv("DNTTK_SKY_PACK")) path=p;
    else if(auto base=SDL_GetBasePath()) {
        path=std::string(base)+"ttk-sky.pack";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    } else return;
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file || file.tellg()<16 || file.tellg()>1024*1024) return;
    std::vector<uint8_t> b(static_cast<size_t>(file.tellg()));
    file.seekg(0);
    if(!file.read(reinterpret_cast<char*>(b.data()),b.size())) return;
    if(std::memcmp(b.data(),"TTKSKY1\0",8) || rd32(b,8)!=1 || rd32(b,12)!=mesh_count) return;
    size_t o=16;
    for(uint32_t i=0;i<mesh_count;++i) {
        if(o+8>b.size()) return;
        const uint32_t id=rd32(b,o),n=rd32(b,o+4);o+=8;
        if(id!=first_id+i || n<0x18 || n>0x4000 || o+n>b.size()) return;
        std::vector<uint8_t> m(b.begin()+o,b.begin()+o+n);o+=n;
        // Validate the polygon list.
        const uint32_t list=rd32(m,0x14);
        for(uint32_t at=list;;) {
            if(at+4>n) return;
            const uint32_t header=rd32(m,at),type=header&0xffff,k=header>>16;at+=4;
            if(type==0xff) break;
            const uint32_t size=record_size(type);
            if(!size || !k || at+k*size>n) return;
            at+=k*size;
        }
        pack.ids[i]=id;pack.meshes[i]=std::move(m);
    }
    constexpr size_t a_bytes=448*256*2,b_bytes=512*256*2;
    if(o+a_bytes+b_bytes!=b.size()) return;
    pack.vram.assign(1024*512,0);
    for(int y=0;y<256;++y) {
        std::memcpy(&pack.vram[y*1024+512],&b[o+y*448*2],448*2);
        std::memcpy(&pack.vram[(256+y)*1024+512],&b[o+a_bytes+y*512*2],512*2);
    }
    pack.ok=true;
    std::fprintf(stderr,"[TTK sky] borrowed sky pack %s\n",path.c_str());
}
bool pack_ok() {std::call_once(pack_once,load_pack);return pack.ok;}

uint32_t copy=0;         // guest copy of the meshes (allocated live, never in a worker)
uint32_t mesh_at[mesh_count]={};
bool injected=false;
uint32_t saved[6];
uint64_t injections=0;

bool enabled() {
    static const bool on=[]{const char* t=std::getenv("DNTTK_SKY_BORROW");return !(t && !std::strcmp(t,"0"));}();
    return on;
}
bool own_sky() {
    for(uint32_t a:sky_words) if(psx_mod_read_word(a)&(a==0x800da468u?0xffffffu:0xffffffffu)) return true;
    return false;
}
bool allocate() {
    if(copy) return true;
    if(psx_mod_replay_active()) return false;
    uint32_t bytes=0;
    for(uint32_t i=0;i<mesh_count;++i) {mesh_at[i]=bytes;bytes+=(uint32_t)(pack.meshes[i].size()+3)&~3u;}
    // Table words for ids 0x611..0x614 first, then the meshes.
    copy=psx_mod_alloc_guest_memory(16+bytes,16);
    if(!copy) return false;
    for(auto& m:mesh_at) m+=copy+16;
    sky_borrow_copy=copy;sky_borrow_copy_end=copy+16+bytes;
    return true;
}
// Rewritten on every use: a savestate load restores mod memory to the saved
// image, which may predate the copy.
void write_copy() {
    for(uint32_t i=0;i<mesh_count;++i) {
        const auto& m=pack.meshes[i];const uint32_t at=mesh_at[i];
        psx_mod_write_word(copy+4*i,at);
        for(size_t o=0;o+4<=m.size();o+=4) psx_mod_write_word(at+(uint32_t)o,rd32(m,o));
        for(size_t o=m.size()&~size_t(3);o<m.size();++o) psx_mod_write_byte(at+(uint32_t)o,m[o]);
        psx_mod_write_half(at,(uint16_t)pack.ids[i]);
        psx_mod_write_word(at+0xc,0);
        for(uint32_t f:{0x8u,0x10u,0x14u}) psx_mod_write_word(at+f,at+rd32(m,f));
    }
}
void restore() {
    if(!injected) return;
    for(int i=0;i<5;++i) psx_mod_write_word(sky_words[i],saved[i]);
    psx_mod_write_word(table_pointer,saved[5]);
    injected=false;
}

void hook(CPUState* cpu,uint32_t address) {
    if(address==0x80026164u) {
        restore();
        // D17BP: the roof caps' block, on the first live frame (no worker forked yet).
        if(!roof_cap_block && !psx_mod_replay_active() && input_modernized()) {
            roof_cap_block=psx_mod_alloc_guest_memory(roof_cap_block_size,16);
            if(roof_cap_block) {psx_mod_write_word(roof_cap_block,~0u);psx_mod_write_word(roof_cap_block+4,0);}
        }
        const uint32_t level=psx_mod_read_word(level_index);
        bool painted=false;
        for(const auto& s:painted_skies) painted|=s.level==level;
        static SkyRenderIdentity identity;
        sky_borrow_ready=painted && enabled() && input_modernized() && psx_mod_read_word(players)==1 &&
            pack_ok() && !own_sky() && allocate() &&
            identity.valid(input_host_frame(),g_dirty_ram_code_gen,g_psx_ram);
        sky_borrow_level=level;
        // D17BP: what this composition may open onto (see sky_borrow_wanted).
        sky_borrow_face_left_prev.store(sky_borrow_face_left.exchange(false,std::memory_order_relaxed),std::memory_order_relaxed);
        if(sky_borrow_ready) {
            const uint32_t loads=psx_mod_savestate_loads();
            if(level!=shaft_level || loads!=shaft_loads) {shaft_level=level;shaft_loads=loads;find_shafts(level);}
            sky_borrow_shaft_shown.store(shaft_shown(),std::memory_order_relaxed);
        } else {
            sky_borrow_shaft_shown.store(false,std::memory_order_relaxed);
            shaft_level=~0u;   // D17BP: a level without a painted sky (its caps table names another level)
        }
        return;
    }
    if(address==0x8001fc44u) {
        if(cpu->gpr[31]!=0x800265c4u || cpu->gpr[4]!=0 || injected || !sky_borrow_wanted() || !copy) return;
        for(int i=0;i<5;++i) saved[i]=psx_mod_read_word(sky_words[i]);
        saved[5]=psx_mod_read_word(table_pointer);
        write_copy();
        for(int i=0;i<5;++i) psx_mod_write_word(sky_words[i],duke_hill_words[i]);
        psx_mod_write_word(table_pointer,copy-4*first_id);
        injected=true;++injections;
        return;
    }
    if(address==0x8002e850u && cpu->gpr[31]==0x800265d4u) restore();
}
}

// Second texture bank for the OpenGL renderer (near_clip marks the borrowed
// meshes' polygons with psx_mod_gpu_host_vertex_foreign).
// The second bank: Duke Hill at x 512..1023 when the sky pack is there, and
// D53's security TV atlas at x 0..255 (security_monitor.cpp), D41L's prop
// lights at x 256 (rows 0-31), D08A46's key card palettes at x 256-287,
// y 480 and D08A45's jewel palettes at x 256-447, y 481. Built once, on
// the renderer's first upload.
extern "C" const uint16_t* ttk_foreign_vram(void) {
    static std::vector<uint16_t> bank;
    static std::once_flag once;
    std::call_once(once,[]{
        const bool sky=pack_ok();
        bank=sky?pack.vram:std::vector<uint16_t>(1024*512,0);
        if(security_tv_paint_bank) security_tv_paint_bank(bank.data());
        if(prop_light_paint_bank) prop_light_paint_bank(bank.data());   // D41L: flames, halos, embers at x 256
        keycard_paint_bank(bank.data());   // D08A46: red and yellow key card palettes at x 256-287, y 480
        jewel_paint_bank(bank.data());   // D08A45: blue, green and red jewel palettes at x 256-447, y 481
        bool any=sky;for(size_t i=0;!any && i<bank.size();++i) any=bank[i]!=0;
        if(!any) bank.clear();
    });
    return bank.empty()?nullptr:bank.data();
}
}

TTK_REGISTER(register_ttk_sky_borrow, TTK_RANK_SKY_BORROW) {
    for(uint32_t address:{0x80026164u,0x8001fc44u,0x8002e850u})
        psx_mod_register_function_entry_plugin("ttk.sky.borrow",address,ttk::hook);
}
