// D41M: lamp bake (see lamp_bake.h and documentation/446-d41m-baked-lamps.md).
//
// Per world triangle, per side the eye can see it from, a chart of texels in
// the triangle's plane; per texel the light the live shader gives a lamp
// (gpu_gl_renderer.c lit_col, flags 0): (1 - d/r)^2, facing 0.85|n.l| + 0.15,
// the side test smoothstep(-0.02, 0.05, side), and the walls (rays against
// the same mesh the shadow maps draw), supersampled for the edges.
#include "lamp_bake.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <thread>
#if defined(__linux__)
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace ttk {
namespace {

struct V3 {float x,y,z;};
inline V3 sub(V3 a,V3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline V3 add(V3 a,V3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
inline V3 mul(V3 a,float s){return {a.x*s,a.y*s,a.z*s};}
inline float dot(V3 a,V3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline V3 cross(V3 a,V3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
inline float len(V3 a){return std::sqrt(dot(a,a));}
inline V3 tri_v(const std::vector<float>& t,size_t i,int k){return {t[i*9+k*3],t[i*9+k*3+1],t[i*9+k*3+2]};}

// GL_RGB9_E5 (OpenGL 4.6 section 8.25: shared exponent, B 15, N 9).
uint32_t rgb9e5_of(float r,float g,float b) {
    const float mx_v=65408.0f;
    r=std::clamp(r,0.0f,mx_v);g=std::clamp(g,0.0f,mx_v);b=std::clamp(b,0.0f,mx_v);
    const float m=std::max({r,g,b});
    if(m<=0) return 0;
    int e=std::max(-16,(int)std::floor(std::log2(m)))+1+15;
    float denom=std::ldexp(1.0f,e-15-9);
    if((int)std::floor(m/denom+0.5f)==512) {denom*=2;++e;}
    const uint32_t rm=(uint32_t)std::floor(r/denom+0.5f),gm=(uint32_t)std::floor(g/denom+0.5f),bm=(uint32_t)std::floor(b/denom+0.5f);
    return std::min(rm,511u)|std::min(gm,511u)<<9|std::min(bm,511u)<<18|(uint32_t)e<<27;
}
inline uint32_t rgba8_of(const float* w) {
    uint32_t v=0;for(int k=0;k<4;++k) v|=(uint32_t)std::lround(std::clamp(w[k],0.0f,1.0f)*255.0f)<<(8*k);
    return v;
}
[[maybe_unused]] uint16_t half_of(float f) {
    uint32_t x;std::memcpy(&x,&f,4);
    const uint32_t sign=(x>>16)&0x8000u;
    int32_t e=(int32_t)((x>>23)&0xff)-127+15;
    uint32_t m=x&0x7fffffu;
    if(e<=0) return (uint16_t)sign;                 // tiny: zero
    if(e>=31) return (uint16_t)(sign|0x7bffu);      // clamp to the largest finite
    const uint32_t h=sign|((uint32_t)e<<10)|(m>>13);
    return (uint16_t)(h+((m>>12)&1u));              // round half up
}

// Bounding volume hierarchy for any-hit rays.
struct Bvh {
    struct Node {float lo[3],hi[3];uint32_t a,b;};   // leaf: b = count | 1<<31, a = first
    std::vector<Node> nodes;std::vector<uint32_t> idx;const std::vector<float>* tris=nullptr;
    void build(const std::vector<float>& t) {
        tris=&t;const size_t n=t.size()/9;idx.resize(n);for(size_t i=0;i<n;++i) idx[i]=(uint32_t)i;
        std::vector<V3> c(n);for(size_t i=0;i<n;++i) c[i]=mul(add(add(tri_v(t,i,0),tri_v(t,i,1)),tri_v(t,i,2)),1.0f/3);
        nodes.clear();nodes.reserve(2*n+1);
        struct Job {uint32_t node,first,count;};std::vector<Job> stack;
        nodes.push_back({});stack.push_back({0,0,(uint32_t)n});
        while(!stack.empty()) {
            const Job j=stack.back();stack.pop_back();
            Node nd{};for(int k=0;k<3;++k) {nd.lo[k]=1e30f;nd.hi[k]=-1e30f;}
            for(uint32_t q=j.first;q<j.first+j.count;++q) for(int v=0;v<3;++v) for(int k=0;k<3;++k) {
                const float x=t[idx[q]*9+v*3+k];nd.lo[k]=std::min(nd.lo[k],x);nd.hi[k]=std::max(nd.hi[k],x);
            }
            if(j.count<=4) {nd.a=j.first;nd.b=j.count|0x80000000u;nodes[j.node]=nd;continue;}
            int ax=0;float best=-1;
            for(int k=0;k<3;++k) if(nd.hi[k]-nd.lo[k]>best) {best=nd.hi[k]-nd.lo[k];ax=k;}
            const uint32_t mid=j.first+j.count/2;
            std::nth_element(idx.begin()+j.first,idx.begin()+mid,idx.begin()+j.first+j.count,[&](uint32_t p,uint32_t q){
                const float a=ax==0?c[p].x:ax==1?c[p].y:c[p].z,b=ax==0?c[q].x:ax==1?c[q].y:c[q].z;return a<b;});
            const uint32_t l=(uint32_t)nodes.size();nodes.push_back({});nodes.push_back({});
            nd.a=l;nd.b=l+1;nodes[j.node]=nd;
            stack.push_back({l,j.first,mid-j.first});stack.push_back({l+1,mid,j.first+j.count-mid});
        }
    }
    // Any triangle hit on the segment o + t d, t in (0, tmax), other than skip.
    bool occluded(V3 o,V3 d,float tmax,uint32_t skip,uint32_t skip2) const {
        const float inv[3]={1.0f/(d.x==0?1e-30f:d.x),1.0f/(d.y==0?1e-30f:d.y),1.0f/(d.z==0?1e-30f:d.z)};
        const float oo[3]={o.x,o.y,o.z};
        uint32_t st[64];int sp=0;st[sp++]=0;
        const std::vector<float>& t=*tris;
        while(sp) {
            const Node& nd=nodes[st[--sp]];
            float t0=0,t1=tmax;
            for(int k=0;k<3;++k) {
                float a=(nd.lo[k]-oo[k])*inv[k],b=(nd.hi[k]-oo[k])*inv[k];
                if(a>b) std::swap(a,b);
                t0=std::max(t0,a);t1=std::min(t1,b);
                if(t0>t1) break;
            }
            if(t0>t1) continue;
            if(nd.b&0x80000000u) {
                const uint32_t n=nd.b&0x7fffffffu;
                for(uint32_t q=nd.a;q<nd.a+n;++q) {
                    const uint32_t i=idx[q];if(i==skip || i==skip2) continue;
                    const V3 a=tri_v(t,i,0),e1=sub(tri_v(t,i,1),a),e2=sub(tri_v(t,i,2),a);
                    const V3 p=cross(d,e2);const float det=dot(e1,p);
                    if(std::fabs(det)<1e-9f) continue;
                    const float id=1.0f/det;const V3 s=sub(o,a);
                    const float u=dot(s,p)*id;if(u<0 || u>1) continue;
                    const V3 qv=cross(s,e1);const float v=dot(d,qv)*id;if(v<0 || u+v>1) continue;
                    const float th=dot(e2,qv)*id;
                    if(th>1e-3f && th<tmax) return true;
                }
            } else if(sp<62) {st[sp++]=nd.a;st[sp++]=nd.b;}
        }
        return false;
    }
};

// Closest point of triangle (a, b, c) to p (Ericson, Real-Time Collision Detection 5.1.5).
V3 closest_on_tri(V3 p,V3 a,V3 b,V3 c) {
    const V3 ab=sub(b,a),ac=sub(c,a),ap=sub(p,a);
    const float d1=dot(ab,ap),d2=dot(ac,ap);if(d1<=0 && d2<=0) return a;
    const V3 bp=sub(p,b);const float d3=dot(ab,bp),d4=dot(ac,bp);if(d3>=0 && d4<=d3) return b;
    const float vc=d1*d4-d3*d2;if(vc<=0 && d1>=0 && d3<=0) return add(a,mul(ab,d1/(d1-d3)));
    const V3 cp=sub(p,c);const float d5=dot(ab,cp),d6=dot(ac,cp);if(d6>=0 && d5<=d6) return c;
    const float vb=d5*d2-d1*d6;if(vb<=0 && d2>=0 && d6<=0) return add(a,mul(ac,d2/(d2-d6)));
    const float va=d3*d6-d5*d4;if(va<=0 && (d4-d3)>=0 && (d5-d6)>=0) return add(b,mul(sub(c,b),(d4-d3)/((d4-d3)+(d5-d6))));
    const float den=1.0f/(va+vb+vc);return add(a,add(mul(ab,vb*den),mul(ac,vc*den)));
}

struct Chart {V3 p0,e1,e2;int w=0,h=0;int u[2]={-1,-1},v[2]={-1,-1};bool side[2]={false,false};std::vector<uint8_t> lamps;};

inline float smooth(float a,float b,float x){const float t=std::clamp((x-a)/(b-a),0.0f,1.0f);return t*t*(3-2*t);}

} // namespace

bool lamp_bake(const BakeInput& in,BakeOutput& out,std::string& error) {
    const auto t0=std::chrono::steady_clock::now();
    out=BakeOutput{};
    const size_t ntri=in.tris.size()/9;
    if(!ntri) {error="no triangles";return false;}
    std::vector<BakeInput::Face> faces=in.faces;
    if(faces.empty()) for(size_t i=0;i<ntri;++i) faces.push_back({(uint32_t)i,1});
    for(const auto& f:faces) if(f.count<1 || f.count>2 || f.first+f.count>ntri) {error="bad face list";return false;}
    const size_t nt=faces.size();
    if(in.lamps.size()>255) {error="more than 255 lamps";return false;}
    const float tx=std::max(2.0f,in.texel);const int ss=std::clamp(in.samples,1,4);
    Bvh bvh;bvh.build(in.tris);
    // Charts and the lamps that can reach each triangle (both sides).
    std::vector<Chart> ch(nt);
    for(size_t i=0;i<nt;++i) {
        Chart& c=ch[i];const BakeInput::Face& fc=faces[i];
        const V3 a=tri_v(in.tris,fc.first,0),b=tri_v(in.tris,fc.first,1),cc=tri_v(in.tris,fc.first,2);
        const V3 d4=fc.count==2?tri_v(in.tris,fc.first+1,2):cc;
        V3 n=cross(sub(b,a),sub(cc,a));const float nl=len(n);
        if(nl<1e-3f) continue;
        n=mul(n,1/nl);
        c.e1=mul(sub(b,a),1/len(sub(b,a)));c.e2=cross(n,c.e1);
        float s0=1e30f,s1=-1e30f,q0=1e30f,q1=-1e30f;
        for(V3 p:{a,b,cc,d4}) {const float s=dot(sub(p,a),c.e1),q=dot(sub(p,a),c.e2);s0=std::min(s0,s);s1=std::max(s1,s);q0=std::min(q0,q);q1=std::max(q1,q);}
        float lo[3],hi[3];for(int k=0;k<3;++k) {lo[k]=1e30f;hi[k]=-1e30f;}
        for(uint32_t t=fc.first;t<fc.first+fc.count;++t) for(int v=0;v<3;++v) for(int k=0;k<3;++k) {
            const float x=in.tris[t*9+v*3+k];lo[k]=std::min(lo[k],x);hi[k]=std::max(hi[k],x);
        }
        for(size_t L=0;L<in.lamps.size();++L) {
            const BakeLamp& lp=in.lamps[L];float d2=0;
            for(int k=0;k<3;++k) {const float e=lp.p[k]<lo[k]?lo[k]-lp.p[k]:lp.p[k]>hi[k]?lp.p[k]-hi[k]:0;d2+=e*e;}
            if(d2>=lp.radius*lp.radius) continue;
            const float side=dot(sub(V3{lp.p[0],lp.p[1],lp.p[2]},a),n);
            if(side>-0.02f*lp.radius) c.side[0]=true;     // generous: the texels decide
            if(side<0.02f*lp.radius) c.side[1]=true;
            c.lamps.push_back((uint8_t)L);
        }
        if(c.lamps.empty()) continue;
        // The chart covers only the part of the face some lamp reaches: the
        // union of the lamps' circles in the face's plane, within the face.
        float u0=1e30f,u1=-1e30f,v0=1e30f,v1=-1e30f;
        for(uint8_t L:c.lamps) {
            const BakeLamp& lp=in.lamps[L];const V3 d=sub(V3{lp.p[0],lp.p[1],lp.p[2]},a);
            const float h=dot(d,n),rr2=lp.radius*lp.radius-h*h;if(rr2<=0) continue;
            const float rr=std::sqrt(rr2),sc=dot(d,c.e1),qc=dot(d,c.e2);
            u0=std::min(u0,sc-rr);u1=std::max(u1,sc+rr);v0=std::min(v0,qc-rr);v1=std::max(v1,qc+rr);
        }
        s0=std::max(s0,u0);s1=std::min(s1,u1);q0=std::max(q0,v0);q1=std::min(q1,v1);
        if(s1<=s0 || q1<=q0) {c.lamps.clear();continue;}
        // One texel of border all round: the origin is a texel before the area.
        c.p0=add(a,add(mul(c.e1,s0-tx),mul(c.e2,q0-tx)));
        c.w=(int)std::ceil((s1-s0)/tx)+2;c.h=(int)std::ceil((q1-q0)/tx)+2;
        if(c.w>in.atlas_width) {c.w=0;c.lamps.clear();continue;}   // wider than the atlas: left to the live path
    }
    // Shelf packing, tallest first.
    std::vector<std::pair<int,int>> order;   // (height, chart side as 2*i+s)
    for(size_t i=0;i<nt;++i) for(int s=0;s<2;++s) if(ch[i].w && ch[i].side[s] && !ch[i].lamps.empty()) order.push_back({ch[i].h,(int)(2*i+s)});
    std::sort(order.begin(),order.end(),[](auto& x,auto& y){return x.first>y.first;});
    int W=in.atlas_width,x=0,y=0,shelf=0;
    for(auto& o:order) {
        Chart& c=ch[o.second/2];const int s=o.second&1;
        if(x+c.w>W) {x=0;y+=shelf;shelf=0;}
        c.u[s]=x;c.v[s]=y;x+=c.w;shelf=std::max(shelf,c.h);
    }
    const int H=std::max(1,y+shelf);
    if(H>16384) {error="atlas taller than 16384";return false;}
    out.width=W;out.height=H;
    out.steady.assign((size_t)W*H,0);out.channels.assign((size_t)W*H,0);
    out.charts.assign(nt*16,0.0f);
    std::atomic<size_t> next{0};std::atomic<uint64_t> rays{0},texels{0},lamp_texels{0};std::atomic<unsigned> lit{0},two{0},over{0};
    auto work=[&]{
        std::vector<float> wsum;std::vector<float> per;   // per texel x lamp
        for(;;) {
            const size_t i=next.fetch_add(1);if(i>=nt) break;
            Chart& c=ch[i];float* rec=&out.charts[i*16];
            rec[3]=rec[11]=-1;rec[13]=rec[14]=255*256+255;
            if(!c.w || c.lamps.empty() || (!c.side[0] && !c.side[1])) continue;
            const BakeInput::Face& fc=faces[i];
            const V3 a=tri_v(in.tris,fc.first,0),b=tri_v(in.tris,fc.first,1),cc=tri_v(in.tris,fc.first,2);
            const bool quad=fc.count==2;
            const V3 a2=quad?tri_v(in.tris,fc.first+1,0):a,b2=quad?tri_v(in.tris,fc.first+1,1):b,c2=quad?tri_v(in.tris,fc.first+1,2):cc;
            const uint32_t skip1=fc.first,skip2=quad?fc.first+1:fc.first;
            auto on_face=[&](V3 p){
                const V3 q1=closest_on_tri(p,a,b,cc);if(!quad) return q1;
                const V3 q2=closest_on_tri(p,a2,b2,c2);return dot(sub(q1,p),sub(q1,p))<=dot(sub(q2,p),sub(q2,p))?q1:q2;
            };
            const V3 n=cross(c.e1,c.e2);
            const size_t nl=c.lamps.size();
            per.assign((size_t)c.w*c.h*2*nl,0.0f);
            uint64_t r=0,lt=0;
            for(int s=0;s<2;++s) {
                if(c.u[s]<0) continue;
                const V3 ns=s?mul(n,-1):n;
                for(int j=0;j<c.h;++j) for(int k=0;k<c.w;++k) {
                    const V3 centre=on_face(add(c.p0,add(mul(c.e1,(k+0.5f)*tx),mul(c.e2,(j+0.5f)*tx))));
                    for(size_t L=0;L<nl;++L) {
                        const BakeLamp& lp=in.lamps[c.lamps[L]];const V3 lpos{lp.p[0],lp.p[1],lp.p[2]};
                        const V3 dv=sub(lpos,centre);const float dist=len(dv);
                        if(dist>=lp.radius) continue;
                        const V3 l=dist>1e-3f?mul(dv,1/dist):V3{0,-1,0};
                        const float side=dot(ns,l);
                        if(side<-0.02f) continue;
                        float at=1-dist/lp.radius;at*=at;
                        const float wrap=(std::fabs(dot(n,l))*0.85f+0.15f)*smooth(-0.02f,0.05f,side);
                        if(at*wrap<=0) continue;
                        // Visibility over ss x ss points of the texel (each kept on the triangle).
                        int seen=0;
                        for(int sj=0;sj<ss;++sj) for(int sk=0;sk<ss;++sk) {
                            const V3 p=ss==1?centre:on_face(add(c.p0,add(mul(c.e1,(k+(sk+0.5f)/ss)*tx),mul(c.e2,(j+(sj+0.5f)/ss)*tx))));
                            const V3 o=add(p,mul(ns,2.0f));const V3 d=sub(lpos,o);const float dl=len(d);
                            ++r;
                            if(dl<=2.0f || !bvh.occluded(o,mul(d,1/dl),dl-1.0f,skip1,skip2)) ++seen;
                        }
                        if(!seen) continue;
                        per[(((size_t)s*c.h+j)*c.w+k)*nl+L]=at*wrap*(float)seen/(ss*ss);++lt;
                    }
                }
            }
            // The triangle's channel lamps: the strongest four that vary.
            wsum.assign(nl,0.0f);
            for(size_t q=0;q<per.size();++q) wsum[q%nl]+=per[q];
            std::vector<size_t> vary;for(size_t L=0;L<nl;++L) if(in.lamps[c.lamps[L]].channel && wsum[L]>0) vary.push_back(L);
            std::sort(vary.begin(),vary.end(),[&](size_t p,size_t q){return wsum[p]>wsum[q];});
            if(vary.size()>4) {over+=(unsigned)(vary.size()-4);vary.resize(4);}
            int slot_of[256];std::fill(slot_of,slot_of+256,-1);
            for(size_t q=0;q<vary.size();++q) slot_of[vary[q]]=(int)q;
            int ids[4]={255,255,255,255};for(size_t q=0;q<vary.size();++q) ids[q]=c.lamps[vary[q]];
            bool any=false;
            for(int s=0;s<2;++s) {
                if(c.u[s]<0) continue;
                for(int j=0;j<c.h;++j) for(int k=0;k<c.w;++k) {
                    float rgb[3]={0,0,0},chw[4]={0,0,0,0};
                    for(size_t L=0;L<nl;++L) {
                        const float wv=per[(((size_t)s*c.h+j)*c.w+k)*nl+L];if(wv<=0) continue;
                        const BakeLamp& lp=in.lamps[c.lamps[L]];
                        if(slot_of[L]>=0) {chw[slot_of[L]]=wv;continue;}
                        const float m=lp.channel?lp.mean:1.0f;
                        for(int q=0;q<3;++q) rgb[q]+=lp.rgb[q]*wv*m;
                    }
                    const size_t at=((size_t)(c.v[s]+j))*W+(c.u[s]+k);
                    out.steady[at]=rgb9e5_of(rgb[0],rgb[1],rgb[2]);out.channels[at]=rgba8_of(chw);
                    for(int q=0;q<3;++q) any|=rgb[q]>0;
                    for(int q=0;q<4;++q) any|=chw[q]>0;
                }
            }
            texels+=(uint64_t)c.w*c.h*((c.u[0]>=0)+(c.u[1]>=0));rays+=r;lamp_texels+=lt;
            if(any) ++lit;
            if(c.u[0]>=0 && c.u[1]>=0) ++two;
            rec[0]=c.p0.x;rec[1]=c.p0.y;rec[2]=c.p0.z;rec[3]=(float)c.u[0];
            rec[4]=c.e1.x/tx;rec[5]=c.e1.y/tx;rec[6]=c.e1.z/tx;rec[7]=(float)c.v[0];
            rec[8]=c.e2.x/tx;rec[9]=c.e2.y/tx;rec[10]=c.e2.z/tx;rec[11]=(float)c.u[1];
            rec[12]=(float)c.v[1];rec[13]=(float)(ids[0]*256+ids[1]);rec[14]=(float)(ids[2]*256+ids[3]);rec[15]=0;
        }
    };
    // A few threads at the lowest priority: the game keeps its cores while a
    // level bakes (the cache makes later loads instant).
    const unsigned hw=std::max(1u,std::thread::hardware_concurrency());
    const unsigned nthreads=in.threads?in.threads:std::clamp(hw/4,1u,4u);
    auto low=[&]{
#if defined(__linux__)
        setpriority(PRIO_PROCESS,(id_t)syscall(SYS_gettid),19);
#endif
        work();
    };
    std::vector<std::thread> pool;for(unsigned k=0;k<nthreads;++k) pool.emplace_back(low);
    for(auto& t:pool) t.join();
    out.rays=rays;out.texels=texels;out.lamp_texels=lamp_texels;out.faces_lit=lit;out.faces_two_sided=two;out.channel_overflows=over;
    out.ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count();
    return true;
}

uint64_t lamp_bake_hash(const BakeInput& in) {
    uint64_t h=1469598103934665603ull;
    auto mix=[&](const void* p,size_t n){const uint8_t* b=(const uint8_t*)p;for(size_t i=0;i<n;++i) h=(h^b[i])*1099511628211ull;};
    const uint32_t version=3;mix(&version,4);
    for(const auto& f:in.faces) {mix(&f.first,4);mix(&f.count,4);}
    mix(in.tris.data(),in.tris.size()*4);
    for(const BakeLamp& l:in.lamps) {mix(l.p,12);mix(l.rgb,12);mix(&l.radius,4);mix(&l.channel,1);mix(&l.mean,4);}
    mix(&in.texel,4);mix(&in.samples,4);mix(&in.atlas_width,4);
    return h;
}

bool lamp_bake_save(const std::string& path,const BakeOutput& o) {
    const std::string tmp=path+".tmp";
    {
        std::ofstream f(tmp,std::ios::binary);if(!f) return false;
        const uint32_t hdr[4]={0x4b414254u /* TBAK */,2u,(uint32_t)o.width,(uint32_t)o.height};
        const uint64_t nc=o.charts.size();
        f.write((const char*)hdr,sizeof hdr);f.write((const char*)&nc,8);
        f.write((const char*)o.steady.data(),o.steady.size()*4);f.write((const char*)o.channels.data(),o.channels.size()*4);
        f.write((const char*)o.charts.data(),o.charts.size()*4);
        if(!f) return false;
    }
    return std::rename(tmp.c_str(),path.c_str())==0;
}

bool lamp_bake_load(const std::string& path,BakeOutput& o) {
    std::ifstream f(path,std::ios::binary);if(!f) return false;
    uint32_t hdr[4];uint64_t nc=0;
    if(!f.read((char*)hdr,sizeof hdr) || hdr[0]!=0x4b414254u || hdr[1]!=2u || !hdr[2] || !hdr[3] || hdr[2]>16384 || hdr[3]>16384) return false;
    if(!f.read((char*)&nc,8) || nc>(1u<<26)) return false;
    o=BakeOutput{};o.width=(int)hdr[2];o.height=(int)hdr[3];
    const size_t px=(size_t)o.width*o.height;
    o.steady.resize(px);o.channels.resize(px);o.charts.resize(nc);
    if(!f.read((char*)o.steady.data(),px*4) || !f.read((char*)o.channels.data(),px*4) || !f.read((char*)o.charts.data(),nc*4)) return false;
    return true;
}

} // namespace ttk
