// D41F: shadows for the dynamic lights (documentation/439-d41f-shadows.md).
//
// Without shadows a light reaches every lit surface within its radius, so a
// lamp or a blast in one room lights the next room through the wall between
// them (the D41D1 room gate only hides the worst cases). Here the level's room
// meshes become one world-space shadow mesh (the framework's
// psx_mod_gpu_set_shadow_mesh, shared by the renderer instances), and the
// strongest lights near Duke get a shadow slot: every renderer keeps a cube
// depth map per slot, rendered from the light over the mesh, and the textured
// shader stops the light where a wall stands between it and the surface.
// Included by modern_controls.cpp after dynamic_lights.inc; Modernized
// Polymorph with DYNAMIC LIGHTS only (Vanilla and Classic never reach it).
//
// - Mesh: the room table (0x800da454, count 0x800da47c, 88-byte rooms,
//   origin +0, piece count +0x32, piece pointers +0x40), each piece a world
//   mesh (+0x14 vertex count, +0x18 polygon list offset, +0x1c 8-byte
//   vertices as in level_lights.inc and near_clip.cpp's world(); the list's
//   groups 0x60 triangles and 0x61 quads, 8-byte records whose first word
//   holds the corner indices, 0xff ends). One chunk per piece with its bounds.
//   Rebuilt when the table, its pieces or their sizes change (level loads)
//   and after a savestate load. Props, actors and Duke do not cast shadows.
// - Slots: lights that may cast shadows (lights_publish's cand: rounds, lamps,
//   screens, prop lamps, fire, blasts, test lights; not muzzle flashes or
//   sparks), the strongest near Duke first, up to PSX_MOD_SHADOW_SLOTS. A
//   light keeps its slot while it stays where it is and within the reach its
//   map was drawn for (a fading blast, a flickering fire), so placed lamps
//   are drawn once; at most shadow_new_per_tick new maps a tick (the others
//   stay unshadowed for that tick).
// - Console: "light shadows" (state and counters), "light shadows
//   on|off|default" (session). Developer DNTTK_SHADOWS=off.
extern "C" void gl_shadow_counters(uint64_t out[4]);
static std::atomic<int> light_shadow_mode{-1};       // session override (-1 default)
static bool shadows_on() {
    const int o=light_shadow_mode.load(std::memory_order_relaxed);
    if(o>=0) return o!=0;
    const char* t=ttk_live_env("DNTTK_SHADOWS");
    return !(t && !std::strcmp(t,"off"));
}
// D41F1: SHADOW EDGES (profile shadow_edges, DNTTK_SHADOW_EDGES, live): smooth
// (default) = the exact shape of the occluding triangles, anti-aliased;
// simple = one shadow-map texel, stepped but cheaper.
static bool shadow_edges_smooth() {
    const char* t=ttk_live_env("DNTTK_SHADOW_EDGES");
    return !(t && !std::strcmp(t,"simple"));
}
static uint64_t shadow_mesh_sig;static uint32_t shadow_mesh_loads=~0u;
static uint64_t shadow_dropped;   // D41E: lights dimmed or dropped for want of a map
static uint32_t shadow_mesh_tris,shadow_mesh_chunks,shadow_mesh_builds;static double shadow_mesh_ms;
static std::vector<float> shadow_mesh_copy;static std::vector<uint32_t> shadow_mesh_owner;   // ray test (console)
// D41M: each face of the mesh (its record address in the piece's polygon
// list and its triangles in shadow_mesh_copy), for the lamp bake.
struct ShadowFace {uint32_t record,first,count;};
static std::vector<ShadowFace> shadow_mesh_faces;static uint32_t shadow_mesh_builds_seen;
static constexpr unsigned shadow_new_per_tick=8;
struct ShadowSlot {float p[3];float far;uint32_t stamp,tick;bool used;};
static ShadowSlot shadow_slots[PSX_MOD_SHADOW_SLOTS];static uint32_t shadow_stamp_next=1;
static uint32_t shadow_new_tick=~0u,shadow_new_count;
static uint64_t shadow_assigned,shadow_new,shadow_deferred;
static bool shadow_ram(uint32_t a,uint32_t size) {return a>=0x80010000u && a<=0x80200000u-size && !(a&3);}
// Rooms, their pieces and the pieces' sizes: changes when a level loads.
static uint64_t shadow_mesh_signature() {
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    uint64_t h=0xcbf29ce484222325ull;
    auto mix=[&](uint64_t v){h=(h^v)*0x100000001b3ull;};
    mix(table);mix(rooms);
    if(!rooms || rooms>256 || !shadow_ram(table,rooms*88)) return h;
    for(uint32_t r=0;r<rooms;++r) {
        const uint32_t a=table+88*r,n=psx_mod_read_byte(a+0x32),list=psx_mod_read_word(a+0x40);
        mix(psx_mod_read_word(a));mix(psx_mod_read_word(a+4));mix(psx_mod_read_word(a+8));mix(n);mix(list);
        if(!n || !shadow_ram(list,n*4)) continue;
        for(unsigned i=0;i<n;++i) {const uint32_t piece=psx_mod_read_word(list+4*i);mix(piece);if(shadow_ram(piece,0x1c)) mix(psx_mod_read_half(piece+0x14));}
    }
    return h;
}
static void shadow_mesh_build() {
    const auto t0=std::chrono::steady_clock::now();
    std::vector<float> xyz;std::vector<PSXModShadowChunk> chunks;std::vector<ShadowFace> faces;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(rooms && rooms<=256 && shadow_ram(table,rooms*88)) for(uint32_t r=0;r<rooms;++r) {
        const uint32_t a=table+88*r;
        const double org[3]={(double)(int32_t)psx_mod_read_word(a),(double)(int32_t)psx_mod_read_word(a+4),(double)(int32_t)psx_mod_read_word(a+8)};
        const unsigned n=psx_mod_read_byte(a+0x32);const uint32_t list=psx_mod_read_word(a+0x40);
        if(!n || !shadow_ram(list,n*4)) continue;
        for(unsigned i=0;i<n;++i) {
            const uint32_t piece=psx_mod_read_word(list+4*i);
            if(!shadow_ram(piece,0x1c) || piece>0x801f0000u) continue;
            const unsigned count=psx_mod_read_half(piece+0x14);
            if(!count || count>256 || piece+0x1c+8*count>0x80200000u) continue;
            float v[256][3];
            for(unsigned j=0;j<count;++j) {
                const uint32_t w0=psx_mod_read_word(piece+0x1c+8*j);
                v[j][0]=(float)(org[0]+(int16_t)(((w0&0xff)<<10)&0xffff));
                v[j][1]=(float)(org[1]+(int16_t)((((w0>>8)&0xff)<<7)&0xffff));
                v[j][2]=(float)(org[2]+(int16_t)((((w0>>16)&0xff)<<10)&0xffff));
            }
            PSXModShadowChunk c{};c.first=(uint32_t)(xyz.size()/9);
            for(int k=0;k<3;++k) {c.lo[k]=1e30f;c.hi[k]=-1e30f;}
            auto tri=[&](unsigned x,unsigned y,unsigned z){
                if(x==y || y==z || x==z) return;
                for(unsigned q:{x,y,z}) for(int k=0;k<3;++k) {
                    xyz.push_back(v[q][k]);c.lo[k]=std::min(c.lo[k],v[q][k]);c.hi[k]=std::max(c.hi[k],v[q][k]);
                }
            };
            uint32_t cursor=piece+psx_mod_read_word(piece+0x18);
            bool ok=shadow_ram(cursor,4);
            for(unsigned guard=0;ok && guard<=256;++guard) {
                const uint32_t header=psx_mod_read_word(cursor);cursor+=4;
                const uint32_t type=header&0xffff,m=header>>16;
                if(type==0xff) break;
                if((type!=0x60 && type!=0x61) || !m || m>4096 || !shadow_ram(cursor,m*8)) {ok=false;break;}
                for(uint32_t k=0;k<m;++k,cursor+=8) {
                    const uint32_t word=psx_mod_read_word(cursor);
                    unsigned idx[4];bool in=true;
                    for(unsigned j=0;j<(type==0x60?3u:4u);++j) {idx[j]=(word>>(8*j))&0xff;in&=idx[j]<count;}
                    if(!in) continue;
                    const uint32_t before=(uint32_t)(xyz.size()/9);
                    tri(idx[0],idx[1],idx[2]);
                    if(type==0x61) tri(idx[1],idx[2],idx[3]);   // the GPU's second half
                    const uint32_t made=(uint32_t)(xyz.size()/9)-before;
                    if(made) faces.push_back({cursor,before,made});
                }
            }
            c.count=(uint32_t)(xyz.size()/9)-c.first;
            if(c.count) chunks.push_back(c);
        }
    }
    shadow_mesh_tris=(uint32_t)(xyz.size()/9);shadow_mesh_chunks=(uint32_t)chunks.size();
    shadow_mesh_copy=xyz;shadow_mesh_owner.clear();shadow_mesh_faces=std::move(faces);
    for(const auto& c:chunks) for(uint32_t t=0;t<c.count;++t) shadow_mesh_owner.push_back((uint32_t)(&c-chunks.data()));
    psx_mod_gpu_set_shadow_mesh(xyz.data(),shadow_mesh_tris,chunks.data(),shadow_mesh_chunks);
    ++shadow_mesh_builds;
    shadow_mesh_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count();
    for(auto& s:shadow_slots) s.used=false;   // maps drawn for the old mesh are redrawn
    if(std::getenv("DNTTK_LIGHT_TRACE")) std::fprintf(stderr,"[TTK light] shadow mesh %u triangles, %u chunks, %.2f ms\n",shadow_mesh_tris,shadow_mesh_chunks,shadow_mesh_ms);
}
// DNTTK_SHADOW_PROBE=1 (developer): every 60 ticks, the emitted world corners
// (plus the eye) against the nearest shadow-mesh corner.
static void shadow_probe_report() {
    static uint32_t t;if(++t%60) return;
    auto s=near_clip_shadow_probe_take();
    if(s.empty() || shadow_mesh_copy.empty()) return;
    std::vector<double> d;double ex=shadow_probe_eye[0],ey=shadow_probe_eye[1],ez=shadow_probe_eye[2];
    double sum[3]={0,0,0};unsigned ns=0;
    for(size_t k=0;k<s.size();k+=std::max<size_t>(1,s.size()/256)) {
        const double p[3]={s[k][0]+ex,s[k][1]+ey,s[k][2]+ez};double best=1e30;double bd[3]={0,0,0};
        for(size_t j=0;j+3<=shadow_mesh_copy.size();j+=3) {
            const double dx=shadow_mesh_copy[j]-p[0],dy=shadow_mesh_copy[j+1]-p[1],dz=shadow_mesh_copy[j+2]-p[2],q=dx*dx+dy*dy+dz*dz;
            if(q<best) {best=q;bd[0]=dx;bd[1]=dy;bd[2]=dz;}
        }
        d.push_back(std::sqrt(best));
        if(best<200*200) {for(int i=0;i<3;++i) sum[i]+=bd[i];++ns;}
    }
    std::sort(d.begin(),d.end());
    std::fprintf(stderr,"[TTK light] shadow probe %zu corners: nearest mesh corner median %.1f p90 %.1f max %.1f; mean offset (mesh - corner) %.1f %.1f %.1f over %u\n",
        d.size(),d[d.size()/2],d[d.size()*9/10],d.back(),ns?sum[0]/ns:0,ns?sum[1]/ns:0,ns?sum[2]/ns:0,ns);
}
// Once per game tick, before the lights are published.
static bool lamp_bake_wanted();   // D41M (lamp_bake.inc)
static void shadow_mesh_tick() {
    if(std::getenv("DNTTK_SHADOW_PROBE")) shadow_probe_report();
    // D41M: the baked lamps need the room mesh also with SHADOWS off (walls
    // always stop them); no shadow slot is assigned then (shadows_assign).
    if(!shadows_on() && !(lamp_bake_wanted() && lights_enabled())) {
        if(shadow_mesh_tris) {psx_mod_gpu_set_shadow_mesh(nullptr,0,nullptr,0);shadow_mesh_tris=shadow_mesh_chunks=0;shadow_mesh_sig=0;}
        return;
    }
    const uint64_t sig=shadow_mesh_signature();
    const uint32_t loads=psx_mod_savestate_loads();
    if(sig==shadow_mesh_sig && loads==shadow_mesh_loads) return;
    shadow_mesh_sig=sig;shadow_mesh_loads=loads;
    shadow_mesh_build();
}
static void shadows_assign(LightView& v,const uint8_t* cand) {
    v.nsh=0;
    if(!shadows_on() || !shadow_mesh_tris) return;
    const double duke[3]={(double)(int32_t)psx_mod_read_word(player+4),(double)(int32_t)psx_mod_read_word(player+8),(double)(int32_t)psx_mod_read_word(player+12)};
    struct P {unsigned i;double score;};P picks[PSX_MOD_MAX_LIGHTS];unsigned np=0;
    for(unsigned i=0;i<v.n;++i) {
        const PSXModLight& l=v.l[i];
        if(!cand[i] || !(l.radius>256.0f)) continue;
        const double dx=l.x-duke[0],dy=l.y-duke[1],dz=l.z-duke[2],r2=(double)l.radius*l.radius;
        const double k=std::max({l.r,l.g,l.b});
        picks[np++]={i,k*r2/(dx*dx+dy*dy+dz*dz+r2)};
    }
    std::stable_sort(picks,picks+np,[](const P& a,const P& b){return a.score>b.score;});
    // D41E: more shadow casters than maps. One without a map would light
    // through the walls, so the weakest go dark instead, the last ones in
    // fading toward the first left out (a score within 1.5x of it), so a
    // swap never pops.
    if(np>PSX_MOD_SHADOW_SLOTS) {
        const double cut=picks[PSX_MOD_SHADOW_SLOTS].score;
        for(unsigned j=0;j<np;++j) {
            const float f=j>=PSX_MOD_SHADOW_SLOTS || !(cut>0) ? 0.0f : (float)std::clamp((picks[j].score/cut-1.0)/0.5,0.0,1.0);
            if(f<1.0f) {PSXModLight& l=v.l[picks[j].i];l.r*=f;l.g*=f;l.b*=f;++shadow_dropped;}
        }
    }
    np=std::min<unsigned>(np,PSX_MOD_SHADOW_SLOTS);
    if(shadow_new_tick!=light_ticks) {shadow_new_tick=light_ticks;shadow_new_count=0;}
    bool claimed[PSX_MOD_SHADOW_SLOTS]={};int slot_of[PSX_MOD_MAX_LIGHTS],stale_of[PSX_MOD_MAX_LIGHTS];
    for(unsigned j=0;j<np;++j) {
        const PSXModLight& l=v.l[picks[j].i];slot_of[j]=stale_of[j]=-1;
        for(unsigned s=0;s<PSX_MOD_SHADOW_SLOTS;++s) {
            const ShadowSlot& o=shadow_slots[s];
            if(!o.used || claimed[s] || o.p[0]!=l.x || o.p[1]!=l.y || o.p[2]!=l.z) continue;
            if(l.radius>o.far) {if(stale_of[j]<0) stale_of[j]=(int)s;continue;}
            slot_of[j]=(int)s;claimed[s]=true;break;
        }
    }
    for(unsigned j=0;j<np;++j) {
        if(slot_of[j]>=0) continue;
        // D41E: out of new maps this tick, a light whose reach grew (Relit
        // easing in) keeps its shorter map for now; one with no map at all
        // (after a load) stays dark for the tick rather than light through
        // the walls.
        if(shadow_new_count>=shadow_new_per_tick) {
            ++shadow_deferred;
            if(stale_of[j]>=0 && !claimed[stale_of[j]]) {slot_of[j]=stale_of[j];claimed[stale_of[j]]=true;}
            else {PSXModLight& l=v.l[picks[j].i];l.r=l.g=l.b=0;}   // dark until its map is drawn (next tick)
            continue;
        }
        int best=-1;
        for(unsigned s=0;s<PSX_MOD_SHADOW_SLOTS;++s) {
            if(claimed[s]) continue;
            if(best<0 || !shadow_slots[s].used || (shadow_slots[best].used && shadow_slots[s].tick<shadow_slots[best].tick)) best=(int)s;
            if(!shadow_slots[s].used) break;
        }
        if(best<0) break;
        const PSXModLight& l=v.l[picks[j].i];ShadowSlot& o=shadow_slots[best];
        o.p[0]=l.x;o.p[1]=l.y;o.p[2]=l.z;o.far=l.radius*1.15f;o.stamp=shadow_stamp_next++;o.used=true;
        if(!shadow_stamp_next) shadow_stamp_next=1;
        claimed[best]=true;slot_of[j]=best;++shadow_new_count;++shadow_new;
    }
    for(unsigned j=0;j<np;++j) {
        if(slot_of[j]<0) continue;
        ShadowSlot& o=shadow_slots[slot_of[j]];o.tick=light_ticks;
        v.l[picks[j].i].flags=(float)(2*(slot_of[j]+1));
        v.sh[v.nsh++]={o.p[0],o.p[1],o.p[2],o.far,(uint32_t)slot_of[j],o.stamp};
        ++shadow_assigned;
    }
}
// The nearest room-mesh triangle on the segment a0-a1 (t its fraction), or
// ~0 (game thread; D41E lamp wall check and the console's ray).
static size_t shadow_ray(const double a0[3],const double a1[3],double& best) {
        const double d[3]={a1[0]-a0[0],a1[1]-a0[1],a1[2]-a0[2]};
        best=2;size_t bt=~size_t(0);
        for(size_t t=0;t+9<=shadow_mesh_copy.size();t+=9) {
            const float* v=&shadow_mesh_copy[t];
            const double e1[3]={v[3]-v[0],v[4]-v[1],v[5]-v[2]},e2[3]={v[6]-v[0],v[7]-v[1],v[8]-v[2]};
            const double p[3]={d[1]*e2[2]-d[2]*e2[1],d[2]*e2[0]-d[0]*e2[2],d[0]*e2[1]-d[1]*e2[0]};
            const double det=e1[0]*p[0]+e1[1]*p[1]+e1[2]*p[2];
            if(std::fabs(det)<1e-9) continue;
            const double s0[3]={a0[0]-v[0],a0[1]-v[1],a0[2]-v[2]};
            const double uu=(s0[0]*p[0]+s0[1]*p[1]+s0[2]*p[2])/det;if(uu<0 || uu>1) continue;
            const double q[3]={s0[1]*e1[2]-s0[2]*e1[1],s0[2]*e1[0]-s0[0]*e1[2],s0[0]*e1[1]-s0[1]*e1[0]};
            const double vv=(d[0]*q[0]+d[1]*q[1]+d[2]*q[2])/det;if(vv<0 || uu+vv>1) continue;
            const double tt=(e2[0]*q[0]+e2[1]*q[1]+e2[2]*q[2])/det;
            if(tt>0 && tt<best) {best=tt;bt=t/9;}
        }
        return bt;
}
static bool shadows_console(const char* arg,void (*say)(const char*)) {
    while(*arg==' ') ++arg;
    // "light shadows ray x y z x2 y2 z2": the nearest shadow triangle on the segment.
    if(!std::strncmp(arg,"ray",3)) {
        double a0[3],a1[3];
        if(std::sscanf(arg+3,"%lf %lf %lf %lf %lf %lf",a0,a0+1,a0+2,a1,a1+1,a1+2)!=6) {say("Light: light shadows ray x y z x2 y2 z2");return true;}
        const double d[3]={a1[0]-a0[0],a1[1]-a0[1],a1[2]-a0[2]};
        double best;const size_t bt=shadow_ray(a0,a1,best);
        char b[320];
        if(bt==~size_t(0)) std::snprintf(b,sizeof b,"Light: ray clear (%zu triangles)",shadow_mesh_copy.size()/9);
        else {const float* v=&shadow_mesh_copy[bt*9];
            std::snprintf(b,sizeof b,"Light: ray hits at %.3f (%.0f units), triangle %zu piece %u: %.0f %.0f %.0f / %.0f %.0f %.0f / %.0f %.0f %.0f",best,best*std::sqrt(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]),bt,shadow_mesh_owner[bt],v[0],v[1],v[2],v[3],v[4],v[5],v[6],v[7],v[8]);}
        say(b);return true;
    }
    if(!std::strcmp(arg,"on")) light_shadow_mode.store(1);
    else if(!std::strcmp(arg,"off")) light_shadow_mode.store(0);
    else if(!std::strcmp(arg,"default")) light_shadow_mode.store(-1);
    // D41F1: "light shadows smooth|simple" - SHADOW EDGES for this session.
    else if(!std::strcmp(arg,"smooth") || !std::strcmp(arg,"simple") || !std::strcmp(arg,"reference")) ttk_live_env_set("DNTTK_SHADOW_EDGES",arg);   // reference: developer, the exact test everywhere
    else if(*arg) {say("Light: light shadows [on|off|default|smooth|simple]");return true;}
    uint64_t g[4];gl_shadow_counters(g);
    char b[256];std::snprintf(b,sizeof b,"Light: shadows %s, %s edges; mesh %u triangles in %u pieces (%.1f ms); %u shadowed now; %llu maps drawn, %.1f ms",
        shadows_on()?"on":"off",shadow_edges_smooth()?"smooth":"simple",shadow_mesh_tris,shadow_mesh_chunks,shadow_mesh_ms,light_view.nsh,(unsigned long long)g[0],g[2]/1000.0);
    say(b);
    return true;
}
