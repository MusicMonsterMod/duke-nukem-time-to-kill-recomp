// D41M: face record -> baked surface, readable by the redraw workers.
//
// The workers are forked copies of this process (render_worker.c); a table
// built after the fork would not reach them. The map lives in a shared
// anonymous mapping made at startup, before any fork, in two halves: the
// game thread writes the idle half and then flips the index, so a worker
// reading mid-update sees one whole table. Elsewhere (no fork) it is plain
// memory.
#include "lamp_bake_map.h"

#include <algorithm>
#include <cmath>
#include <atomic>
#include <cstring>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#define LBM_SHARED 1
#endif

namespace ttk {
namespace {

constexpr uint32_t lbm_cap = 1u << 17;   // faces (level 0: about 25,000 triangles)
// D41P: floor grid capacity (cells and floor entries; pages are touched only as used).
constexpr uint32_t lbf_cells = 1u << 20, lbf_entries = 1u << 21;
struct Floor {float x0,z0,cell;uint32_t w,h,n;uint32_t start[lbf_cells+1];float y[lbf_entries];uint32_t surf[lbf_entries];float gx[lbf_entries],gz[lbf_entries];};   // D41S: the face's slope
struct Half {uint32_t n;uint32_t pad;uint64_t key[lbm_cap];Floor floor;};   // record << 32 | surface
struct Shared {std::atomic<uint32_t> active;uint32_t pad;Half half[2];};
Shared* lbm=nullptr;

Shared* lbm_get() {
    if(lbm) return lbm;
#ifdef LBM_SHARED
    void* m=mmap(nullptr,sizeof(Shared),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE,-1,0);
    if(m!=MAP_FAILED) lbm=new(m) Shared;   // D41P: no value-initialisation (the mapping is zero; 40 MB untouched)
#endif
    if(!lbm) lbm=new Shared;   // counts set below; arrays read only up to them
    lbm->active.store(0);lbm->half[0].n=lbm->half[1].n=0;lbm->half[0].floor.n=lbm->half[1].floor.n=0;
    return lbm;
}

// Before the workers fork (static initialisation).
struct Init {Init(){lbm_get();}} lbm_init;

} // namespace

void lamp_bake_map_publish(const std::vector<std::pair<uint32_t,uint32_t>>& records) {
    Shared* s=lbm_get();
    const uint32_t idle=s->active.load(std::memory_order_acquire)^1u;
    Half& h=s->half[idle];
    const size_t n=std::min<size_t>(records.size(),lbm_cap);
    for(size_t i=0;i<n;++i) h.key[i]=(uint64_t)records[i].first<<32|records[i].second;
    std::sort(h.key,h.key+n);
    h.n=(uint32_t)n;
    s->active.store(idle,std::memory_order_release);
}

void lamp_bake_map_clear() {
    Shared* s=lbm_get();
    const uint32_t idle=s->active.load(std::memory_order_acquire)^1u;
    s->half[idle].n=0;s->half[idle].floor.n=0;
    s->active.store(idle,std::memory_order_release);
}

// D41P: faces facing up or down (|normal y| at least 0.7), each cell centre
// they cover with the face's height there.
LampFloorGrid lamp_bake_floor_build(const std::vector<float>& tris,const std::vector<std::pair<uint32_t,uint32_t>>& faces) {
    LampFloorGrid g;
    struct T {const float* v;uint32_t surf;};
    std::vector<T> flat;
    float lo[2]={1e30f,1e30f},hi[2]={-1e30f,-1e30f};
    for(size_t f=0;f<faces.size();++f) for(uint32_t t=faces[f].first;t<faces[f].first+faces[f].second && 9u*(t+1)<=tris.size();++t) {
        const float* v=&tris[9u*t];
        const float e1[3]={v[3]-v[0],v[4]-v[1],v[5]-v[2]},e2[3]={v[6]-v[0],v[7]-v[1],v[8]-v[2]};
        const float n[3]={e1[1]*e2[2]-e1[2]*e2[1],e1[2]*e2[0]-e1[0]*e2[2],e1[0]*e2[1]-e1[1]*e2[0]};
        const float len=std::sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
        if(!(len>0) || std::fabs(n[1])<0.7f*len) continue;
        flat.push_back({v,(uint32_t)(f+1)});
        for(int k=0;k<3;++k) {lo[0]=std::min(lo[0],v[3*k]);hi[0]=std::max(hi[0],v[3*k]);lo[1]=std::min(lo[1],v[3*k+2]);hi[1]=std::max(hi[1],v[3*k+2]);}
    }
    if(flat.empty()) return g;
    g.x0=lo[0];g.z0=lo[1];
    while(true) {
        g.w=(uint32_t)((hi[0]-lo[0])/g.cell)+1;g.h=(uint32_t)((hi[1]-lo[1])/g.cell)+1;
        if((uint64_t)g.w*g.h<=lbf_cells) break;
        g.cell*=2;
    }
    struct E {float y;uint32_t surf;float gx,gz;};
    std::vector<std::vector<E>> cells((size_t)g.w*g.h);
    for(const T& t:flat) {
        const float* v=t.v;
        const float x0=std::min({v[0],v[3],v[6]}),x1=std::max({v[0],v[3],v[6]}),z0=std::min({v[2],v[5],v[8]}),z1=std::max({v[2],v[5],v[8]});
        const int cx0=std::max(0,(int)std::ceil((x0-g.x0)/g.cell-0.5f)),cx1=std::min((int)g.w-1,(int)std::floor((x1-g.x0)/g.cell-0.5f));
        const int cz0=std::max(0,(int)std::ceil((z0-g.z0)/g.cell-0.5f)),cz1=std::min((int)g.h-1,(int)std::floor((z1-g.z0)/g.cell-0.5f));
        const float ax=v[0],az=v[2],bx=v[3]-ax,bz=v[5]-az,cx=v[6]-ax,cz=v[8]-az;
        const float den=bx*cz-bz*cx;
        if(std::fabs(den)<1e-6f) continue;
        // D41S: the plane's slope, y = y0 + gx (x - x0) + gz (z - z0).
        const float by=v[4]-v[1],cy=v[7]-v[1];
        const float gx=(by*cz-cy*bz)/den,gz=(bx*cy-cx*by)/den;
        for(int i=cx0;i<=cx1;++i) for(int j=cz0;j<=cz1;++j) {
            const float px=g.x0+(i+0.5f)*g.cell-ax,pz=g.z0+(j+0.5f)*g.cell-az;
            const float s=(px*cz-pz*cx)/den,u=(bx*pz-bz*px)/den;
            if(s<-1e-4f || u<-1e-4f || s+u>1.0001f) continue;
            const float y=v[1]+s*(v[4]-v[1])+u*(v[7]-v[1]);
            cells[(size_t)j*g.w+i].push_back({y,t.surf,gx,gz});
        }
    }
    g.start.assign((size_t)g.w*g.h+1,0);
    for(size_t c=0;c<cells.size();++c) {
        g.start[c]=(uint32_t)g.y.size();
        for(auto& e:cells[c]) {if(g.y.size()>=lbf_entries) break;g.y.push_back(e.y);g.surf.push_back(e.surf);g.gx.push_back(e.gx);g.gz.push_back(e.gz);}
    }
    g.start[cells.size()]=(uint32_t)g.y.size();
    return g;
}

void lamp_bake_floor_stage(const LampFloorGrid& g) {
    Shared* s=lbm_get();
    Floor& f=s->half[s->active.load(std::memory_order_acquire)^1u].floor;
    f.n=0;
    if(g.start.empty() || g.start.size()>lbf_cells+1) return;
    f.x0=g.x0;f.z0=g.z0;f.cell=g.cell;f.w=g.w;f.h=g.h;
    std::memcpy(f.start,g.start.data(),g.start.size()*4);
    std::memcpy(f.y,g.y.data(),g.y.size()*4);std::memcpy(f.surf,g.surf.data(),g.surf.size()*4);
    if(g.gx.size()==g.y.size()) {std::memcpy(f.gx,g.gx.data(),g.gx.size()*4);std::memcpy(f.gz,g.gz.data(),g.gz.size()*4);}
    else {std::memset(f.gx,0,g.y.size()*4);std::memset(f.gz,0,g.y.size()*4);}
    f.n=(uint32_t)g.y.size();
}

// D23N1: a redraw job reads one table from start to end. A publish during a
// job (the level's first bake appearing) takes effect at the next job, never
// half way through one, and a rebuilt job sees what the job it rebuilds saw.
// The writer only ever rewrites the idle half, so the pinned half stays whole
// until the next publish but one (seconds apart: a bake takes seconds).
int lbm_pinned=-1;
void lamp_bake_pin(bool on) {lbm_pinned=on && lbm ? (int)(lbm->active.load(std::memory_order_acquire)&1u) : -1;}

uint32_t lamp_bake_floor(float x,float y,float z) {
    const Shared* s=lbm;
    if(!s) return 0;
    const Half& h=s->half[lbm_pinned>=0 ? (unsigned)lbm_pinned : s->active.load(std::memory_order_acquire)&1u];
    const Floor& f=h.floor;
    if(!f.n || !h.n) return 0;
    const float cx=(x-f.x0)/f.cell,cz=(z-f.z0)/f.cell;
    if(!(cx>=0) || !(cz>=0) || cx>=(float)f.w || cz>=(float)f.h) return 0;
    uint32_t best=0;float by=1e30f;
    // The nearest floor at or below the point (a little above counts: a
    // joint at the feet), TTK's y growing downward.
    auto scan=[&](int i,int j){
        if(i<0 || j<0 || i>=(int)f.w || j>=(int)f.h) return;
        const size_t c=(size_t)j*f.w+(size_t)i;
        for(uint32_t k=f.start[c];k<f.start[c+1];++k) if(f.y[k]>=y-64.0f && f.y[k]<by) {by=f.y[k];best=f.surf[k];}
    };
    const int ix=(int)cx,iz=(int)cz;
    scan(ix,iz);
    // D41P2: a floor smaller than a cell covers no cell centre (a beam, a
    // step, a ledge): the eight cells around.
    if(!best) for(int dj=-1;dj<=1;++dj) for(int di=-1;di<=1;++di) if(di || dj) scan(ix+di,iz+dj);
    return best;
}

uint32_t lamp_bake_floor_height(float x,float y,float z,float* floor_y,float* slope,bool ring) {
    const Shared* s=lbm;
    if(!s) return 0;
    const Half& h=s->half[lbm_pinned>=0 ? (unsigned)lbm_pinned : s->active.load(std::memory_order_acquire)&1u];
    const Floor& f=h.floor;
    if(!f.n || !h.n) return 0;
    const float cx=(x-f.x0)/f.cell,cz=(z-f.z0)/f.cell;
    if(!(cx>=0) || !(cz>=0) || cx>=(float)f.w || cz>=(float)f.h) return 0;
    // The exact height of each floor of the cell under (x,z) (its plane from
    // the cell centre and its slope); the nearest at or below y. With ring,
    // a cell without one takes the eight around (their planes carried over).
    uint32_t best=0;float by=1e30f,bgx=0,bgz=0;
    auto scan=[&](int i,int j){
        if(i<0 || j<0 || i>=(int)f.w || j>=(int)f.h) return;
        const size_t c=(size_t)j*f.w+(size_t)i;
        const float dx=x-(f.x0+((float)i+0.5f)*f.cell),dz=z-(f.z0+((float)j+0.5f)*f.cell);
        for(uint32_t k=f.start[c];k<f.start[c+1];++k) {
            const float h=f.y[k]+f.gx[k]*dx+f.gz[k]*dz;
            if(h>=y && h<by) {by=h;best=f.surf[k];bgx=f.gx[k];bgz=f.gz[k];}
        }
    };
    scan((int)cx,(int)cz);
    if(!best && ring) for(int dj=-1;dj<=1;++dj) for(int di=-1;di<=1;++di) if(di || dj) scan((int)cx+di,(int)cz+dj);
    if(best && floor_y) *floor_y=by;
    if(best && slope) {slope[0]=bgx;slope[1]=bgz;}
    return best;
}

uint32_t lamp_bake_surface(uint32_t record) {
    const Shared* s=lbm;
    if(!s) return 0;
    const Half& h=s->half[lbm_pinned>=0 ? (unsigned)lbm_pinned : s->active.load(std::memory_order_acquire)&1u];
    if(!h.n) return 0;
    const uint64_t lo=(uint64_t)record<<32;
    const uint64_t* p=std::lower_bound(h.key,h.key+h.n,lo);
    return p!=h.key+h.n && (uint32_t)(*p>>32)==record ? (uint32_t)*p : 0u;
}

} // namespace ttk
