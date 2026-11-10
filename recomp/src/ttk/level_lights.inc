// D41D: level light sources (documentation/428-d41d-level-lights.md).
//
// Time to Kill paints its lamps, signs and neon into vertex colours and has no
// light entities. Here each level can have a list of real lights ("lamps"),
// kept in a text file we author (recomp/assets/lights/levelNN.lights, like
// Duke-RT's LIGHTOVR or DarkPlaces' .rtlights) and drawn by the D41A per-pixel
// path on top of the painted light, so walls, props, enemies and Duke pick up
// the colour of the neon they pass. Included by modern_controls.cpp after
// dynamic_lights.inc; Modernized Polymorph with DYNAMIC LIGHTS only.
//
// - Lamps: position, colour, radius, strength and a style (steady, flicker,
//   pulse, candle, strobe, broken), animated at the game tick.
// - Per image: the nearest lamps to Duke (up to level_lamp_max) whose reach
//   comes within level_lamp_margin of him, faded in over the last
//   level_lamp_fade so a lamp never pops on.
// - Scan: the painted-light peaks of the level's rooms (room table
//   0x800da454, 88-byte rooms, origin +0, pieces +0x40 / count +0x32; piece =
//   world mesh: +0x14 vertex count, +0x1c 8-byte vertices, x/z grid byte <<
//   10, y byte << 7, RGB word) clustered into coloured lamp candidates.
// - Editor: backtick console "lamp ..." (lamp_console_command). Edits apply at
//   once; "lamp save" writes the level's file.
// - D41D1 wave style: the game's own animated light. A world vertex whose
//   colour-index byte has bit 4 is darkened by the light table entry its
//   colour word's top byte (0-31, a phase) selects from the pointer at
//   ctx+0x68 (0x800d67a8), which steps one entry a game tick through a
//   64-entry wave (level 0: 3072 * |sin(pi k / 32)|, 0x800da350). A wave lamp
//   follows that same entry live: "wave:P:D" is phase P, depth D (1 = as
//   deep as the original vertex light), normalised so its average stays its
//   strength. The scan gives it to clusters whose phases agree.
// - D41D1 sign scan: static props (object kind 1, mesh +0x44 in the object
//   renderer layout) whose textures are bright and strongly coloured (EXIT
//   signs, neon, lit panels; texels read from VRAM through their CLUT) get a
//   lamp just in front of the glowing faces, in the glow's colour.
//
// File format, one lamp per line ('#' starts a comment):
//   x y z  r g b  radius  strength  style  [# note]
// style: steady flicker pulse candle strobe broken, or wave:P:D
// (Standard headers are included at the top of modern_controls.cpp.)
#ifndef TTK_LIGHTS_DIR
#define TTK_LIGHTS_DIR ""
#endif
// room: the room the lamp belongs to (-2 not yet known, -1 none): from its
// note ("... room N", written by the scans) or else the smallest room whose
// piece bounds hold it.
struct LevelLamp {double p[3];float rgb[3];double radius;float k;int style;std::string note;int phase=0;float depth=1;int room=-2;};
enum {lamp_steady,lamp_flicker,lamp_pulse,lamp_candle,lamp_strobe,lamp_broken,lamp_wave,lamp_styles};
static const char* const lamp_style_names[lamp_styles]={"steady","flicker","pulse","candle","strobe","broken","wave"};
static constexpr unsigned level_lamp_max=12;          // of PSX_MOD_MAX_LIGHTS per image
static constexpr double level_lamp_margin=6000,level_lamp_fade=2000;
static std::vector<LevelLamp> level_lamps;           // game thread
static unsigned level_lamps_level=~0u;
static bool level_lamps_dirty;                        // edited since load or save
static int level_lamp_selected=-1;
static uint32_t level_lamp_flash_until;               // light_ticks: the selected lamp blinks
static std::atomic<int> level_lamps_shown{1};         // console "lamp off|on" (session)
// D41D1 room gate: a lamp lights only while Polymorph shows its room (the
// host visibility walk, host_visibility.inc), fading over level_lamp_gate_fade
// ticks, so a sign behind a wall does not light the room in front of it.
static std::atomic<int> level_lamps_gate{1};          // console "lamp gate off|on" (session)
static constexpr float level_lamp_gate_fade=8;
static std::vector<float> level_lamp_vis;             // per lamp, 0-1
static uint32_t level_lamp_vis_tick;
static std::string level_lamps_path_used;
// Console thread -> game thread.
static std::mutex level_lamp_mutex;
static std::deque<std::string> level_lamp_queue;
static std::vector<std::string> level_lamp_listing;   // last "lamp list" snapshot (game thread writes)
static unsigned level_lamp_listing_level=~0u;

static std::string level_lamps_dir() {
    if(const char* e=std::getenv("DNTTK_LIGHTS_DIR"); e && *e) return e;
    std::error_code ec;
    if(*TTK_LIGHTS_DIR && std::filesystem::is_directory(TTK_LIGHTS_DIR,ec)) return TTK_LIGHTS_DIR;
    std::string dir="lights";
    if(auto base=SDL_GetBasePath()) {
        dir=std::string(base)+"lights";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    }
    return dir;
}
static std::string level_lamps_path(unsigned level) {
    char name[32];std::snprintf(name,sizeof name,"/level%02u.lights",level);
    return level_lamps_dir()+name;
}
static int lamp_style_from(const char* s) {
    for(int i=0;i<lamp_styles;++i) if(!std::strcmp(s,lamp_style_names[i])) return i;
    return -1;
}
// A style token: a name, or "wave[:P[:D]]" / "wave [P [D]]" (phase 0-31, depth 0-1).
static bool lamp_style_parse(const char* s,LevelLamp& l) {
    char name[16]={0};int used=0;
    if(std::sscanf(s,"%15[a-z]%n",name,&used)!=1) return false;
    const int st=lamp_style_from(name);
    if(st<0) return false;
    l.style=st;
    if(st==lamp_wave) {
        int ph=0;float dp=1;const char* r=s+used;
        while(*r==':' || *r==' ') ++r;
        if(std::sscanf(r,"%d",&ph)==1) {
            while(*r && *r!=':' && *r!=' ') ++r;
            while(*r==':' || *r==' ') ++r;
            std::sscanf(r,"%f",&dp);
        }
        l.phase=((ph%32)+32)%32;l.depth=std::clamp(dp,0.0f,1.0f);
    }
    return true;
}
static std::string lamp_style_text(const LevelLamp& l) {
    if(l.style!=lamp_wave) return lamp_style_names[l.style];
    char b[32];std::snprintf(b,sizeof b,"wave:%d:%.2f",l.phase,l.depth);return b;
}
static void level_lamps_load(unsigned level) {
    level_lamps.clear();level_lamp_selected=-1;level_lamps_dirty=false;level_lamps_level=level;
    level_lamps_path_used=level_lamps_path(level);
    std::ifstream f(level_lamps_path_used);
    if(!f) return;
    std::string line;unsigned bad=0;
    while(std::getline(f,line) && level_lamps.size()<512) {
        std::string note;
        if(auto h=line.find('#');h!=std::string::npos) {note=line.substr(h+1);line.resize(h);}
        while(!note.empty() && note[0]==' ') note.erase(0,1);
        LevelLamp l{};char style[24]={0};
        const int n=std::sscanf(line.c_str(),"%lf %lf %lf %f %f %f %lf %f %23s",&l.p[0],&l.p[1],&l.p[2],&l.rgb[0],&l.rgb[1],&l.rgb[2],&l.radius,&l.k,style);
        if(n<=0) continue;
        if(n<7 || l.radius<300 || l.radius>30000) {++bad;continue;}
        if(n<8) l.k=1;
        l.style=lamp_steady;
        if(n>=9 && !lamp_style_parse(style,l)) {++bad;continue;}
        for(float& c:l.rgb) c=std::clamp(c,0.0f,4.0f);
        l.k=std::clamp(l.k,0.0f,4.0f);l.note=note;
        level_lamps.push_back(l);
    }
    std::fprintf(stderr,"[TTK lamp] level %u: %zu lamps from %s%s\n",level,level_lamps.size(),level_lamps_path_used.c_str(),bad?" (some lines skipped)":"");
}
static bool level_lamps_save(std::string& error) {
    std::error_code ec;
    const std::filesystem::path path(level_lamps_path(level_lamps_level));
    std::filesystem::create_directories(path.parent_path(),ec);
    const std::filesystem::path tmp=path.string()+".tmp";
    {
        std::ofstream f(tmp);
        if(!f) {error="cannot write "+path.string();return false;}
        f<<"# D41D level "<<level_lamps_level<<" lamps (documentation/428-d41d-level-lights.md, 429-d41d1-signs-waves-levels.md)\n";
        f<<"# x y z  r g b  radius  strength  style  # note\n";
        char row[200];
        for(const LevelLamp& l:level_lamps) {
            std::snprintf(row,sizeof row,"%.0f %.0f %.0f  %.2f %.2f %.2f  %.0f  %.2f  %s",
                          l.p[0],l.p[1],l.p[2],l.rgb[0],l.rgb[1],l.rgb[2],l.radius,l.k,lamp_style_text(l).c_str());
            f<<row;if(!l.note.empty()) f<<"  # "<<l.note;f<<'\n';
        }
        if(!f) {error="write failed";return false;}
    }
    std::filesystem::rename(tmp,path,ec);
    if(ec) {error=ec.message();return false;}
    level_lamps_path_used=path.string();level_lamps_dirty=false;
    return true;
}
// Brightness of a style at game tic t (30 Hz); seed spreads lamps' phases.
static float lamp_style_value(int style,uint32_t t,uint32_t seed) {
    const uint32_t h=(t+seed*7919u)*2654435761u;
    const float r=(float)((h>>8)&0xffff)/65535.0f;
    switch(style) {
    case lamp_flicker: {  // neon buzz: steady, now and then a quick dip
        const uint32_t slot=(t/3+seed)*2246822519u;
        return ((slot>>27)==0) ? 0.35f+0.3f*r : 0.92f+0.08f*r;
    }
    case lamp_pulse: return 0.7f+0.3f*std::sin((t+seed*13u)*(2*3.14159265f/60.0f));
    case lamp_candle: return 0.78f+0.22f*r;
    case lamp_strobe: return ((t+seed)%15u)<2 ? 1.0f : 0.0f;
    case lamp_broken: {   // mostly dark, bursts of flicker
        const uint32_t phase=(t+seed*37u)%90u;
        return phase<14 ? (r>0.45f?1.0f:0.1f) : 0.05f;
    }
    default: return 1.0f;
    }
}
// D41D1: the lamp's room (see LevelLamp::room).
static int level_lamp_room(const LevelLamp& l) {
    if(auto at=l.note.find("room ");at!=std::string::npos) {
        const int r=std::atoi(l.note.c_str()+at+5);
        if(r>=0 && r<256) return r;
    }
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return -1;
    int best=-1;double best_volume=1e300;
    for(uint32_t r=0;r<rooms;++r) {
        const uint32_t a=table+88*r;
        const unsigned n=psx_mod_read_byte(a+0x32);const uint32_t list=psx_mod_read_word(a+0x40);
        if(!n || list<0x80010000u || list>0x80200000u-n*4 || (list&3)) continue;
        double lo[3]={1e300,1e300,1e300},hi[3]={-1e300,-1e300,-1e300};
        for(unsigned i=0;i<n;++i) {
            const uint32_t piece=psx_mod_read_word(list+4*i);
            if(piece<0x80010000u || piece>0x801f0000u || (piece&3)) continue;
            for(int k=0;k<3;++k) {
                const double o=(int32_t)psx_mod_read_word(a+4*k);
                lo[k]=std::min(lo[k],o+(int16_t)psx_mod_read_half(piece+4+2*k));
                hi[k]=std::max(hi[k],o+(int16_t)psx_mod_read_half(piece+12+2*k));
            }
        }
        bool in=true;double volume=1;
        for(int k=0;k<3;++k) {in&=l.p[k]>=lo[k]-64 && l.p[k]<=hi[k]+64;volume*=std::max(1.0,hi[k]-lo[k]);}
        if(in && volume<best_volume) {best_volume=volume;best=(int)r;}
    }
    return best;
}
// D41D1: the original animated light table, read live. Per image: the
// pointer (ctx+0x68, which the game steps a tick at a time) and the table's
// mean over one period, so a wave lamp keeps its average brightness.
struct LampWave {uint32_t ptr=0;float mean=0;bool ok=false;};
static LampWave lamp_wave_now() {
    LampWave w;w.ptr=psx_mod_read_word(0x800d67a8u+0x68);
    if(w.ptr<0x80010000u || w.ptr>0x801fff00u || (w.ptr&3)) return w;
    double sum=0;
    for(unsigned i=0;i<32;++i) {
        const int v=(int16_t)psx_mod_read_word(w.ptr+4*i);
        if(v<-4096 || v>4096) return w;
        sum+=v;
    }
    w.mean=(float)(sum/32);w.ok=true;
    return w;
}
static float lamp_wave_value(const LampWave& w,int phase,float depth) {
    if(!w.ok) return 1.0f;
    const float v=(float)(int16_t)psx_mod_read_word(w.ptr+4*(unsigned)(phase&31));
    const float base=1.0f-depth*w.mean/4096.0f;
    return base>0.05f ? std::clamp((1.0f-depth*v/4096.0f)/base,0.0f,2.0f) : 1.0f;
}
// Called from lights_publish (game tick): the nearest lamps for this image.
static unsigned level_lamps_publish(PSXModLight* out,unsigned room) {
    if(!level_lamps_shown.load(std::memory_order_relaxed) || level_lamps.empty() || !room) return 0;
    const double duke[3]={(double)(int32_t)psx_mod_read_word(player+4),(double)(int32_t)psx_mod_read_word(player+8),(double)(int32_t)psx_mod_read_word(player+12)};
    struct Near {double d;unsigned i;};
    Near near[64];unsigned n=0;
    for(unsigned i=0;i<level_lamps.size();++i) {
        const LevelLamp& l=level_lamps[i];
        const double dx=l.p[0]-duke[0],dy=l.p[1]-duke[1],dz=l.p[2]-duke[2];
        const double d=std::sqrt(dx*dx+dy*dy+dz*dz)-l.radius;
        if(d>level_lamp_margin && (int)i!=level_lamp_selected) continue;
        if(n<std::size(near)) near[n++]={d,i};
        else {unsigned w=0;for(unsigned j=1;j<n;++j) if(near[j].d>near[w].d) w=j;if(d<near[w].d) near[w]={d,i};}
    }
    std::sort(near,near+n,[](const Near& a,const Near& b){return a.d<b.d;});
    const LampWave wave=lamp_wave_now();
    if(static const bool trace=std::getenv("DNTTK_LAMP_TRACE")!=nullptr;trace && light_ticks%10==0)
        for(unsigned j=0;j<n;++j) if(const LevelLamp& l=level_lamps[near[j].i];l.style==lamp_wave) {
            std::fprintf(stderr,"[TTK lamp] wave tick %u ptr %08x lamp %u phase %d depth %.2f entry %d value %.3f\n",light_ticks,wave.ptr,near[j].i,l.phase,l.depth,
                         wave.ok?(int)(int16_t)psx_mod_read_word(wave.ptr+4*(unsigned)l.phase):0,lamp_wave_value(wave,l.phase,l.depth));
            break;
        }
    // Room gate: fade each candidate toward whether its room is shown.
    if(level_lamp_vis.size()!=level_lamps.size()) level_lamp_vis.assign(level_lamps.size(),1.0f);
    const float step=std::min(1.0f,(float)(light_ticks-level_lamp_vis_tick)/level_lamp_gate_fade);
    level_lamp_vis_tick=light_ticks;
    const bool gate=level_lamps_gate.load(std::memory_order_relaxed)!=0;
    for(unsigned j=0;j<n;++j) {
        LevelLamp& l=level_lamps[near[j].i];
        if(l.room==-2) l.room=level_lamp_room(l);
        float target=1;
        if(gate && host_vis_shown_valid && l.room>=0)
            target=(size_t)l.room<host_vis_shown.size() && host_vis_shown[l.room] ? 1.0f : 0.0f;
        else if(gate && !host_vis_shown_valid) target=level_lamp_vis[near[j].i];  // mid-walk: hold
        float& v=level_lamp_vis[near[j].i];
        v=v<target?std::min(target,v+step):std::max(target,v-step);
    }
    unsigned k=0;
    for(unsigned j=0;j<n && k<std::min(room,level_lamp_max);++j) {
        const LevelLamp& l=level_lamps[near[j].i];
        float e=l.k*(l.style==lamp_wave ? lamp_wave_value(wave,l.phase,l.depth) : lamp_style_value(l.style,light_ticks,near[j].i));
        e*=(float)std::clamp((level_lamp_margin-near[j].d)/level_lamp_fade,0.0,1.0);
        if((int)near[j].i!=level_lamp_selected) e*=level_lamp_vis[near[j].i];
        if((int)near[j].i==level_lamp_selected && light_ticks<level_lamp_flash_until)
            e=(light_ticks/6)&1 ? std::max(e,1.2f) : 0.0f;  // editor: the selected lamp blinks
        if(e<=0.01f) continue;
        PSXModLight& o=out[k++];
        o.x=(float)l.p[0];o.y=(float)l.p[1];o.z=(float)l.p[2];o.radius=(float)l.radius;
        o.r=l.rgb[0]*e;o.g=l.rgb[1]*e;o.b=l.rgb[2]*e;o.flags=0;
    }
    return k;
}

// Painted-light scan: vertices at or above the brightness threshold are
// clustered (2560 on the floor plane, 4096 in height), clusters of the same
// room and a similar colour within 6000 merge, and each room keeps its three
// largest. A candidate sits at the cluster's weighted centre on the floor
// plane, 65% of the way up its room column, coloured by the cluster's mean.
struct ScanVert {double p[3];int c[3];int lum;unsigned room;bool wave;int phase;};
// wx/wy: the summed unit phasors of the cluster's wave vertices (nw of them).
struct ScanCand {double p[3];double rgb[3];double radius;unsigned room,n;double wx=0,wy=0;unsigned nw=0;};
static bool level_lamps_scan_verts(std::vector<ScanVert>& v) {
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return false;
    for(uint32_t r=0;r<rooms;++r) {
        const uint32_t a=table+88*r;
        const double org[3]={(double)(int32_t)psx_mod_read_word(a),(double)(int32_t)psx_mod_read_word(a+4),(double)(int32_t)psx_mod_read_word(a+8)};
        const unsigned n=psx_mod_read_byte(a+0x32);const uint32_t list=psx_mod_read_word(a+0x40);
        if(!n || list<0x80010000u || list>0x80200000u-n*4 || (list&3)) continue;
        for(unsigned i=0;i<n;++i) {
            const uint32_t piece=psx_mod_read_word(list+4*i);
            if(piece<0x80010000u || piece>0x801f0000u || (piece&3)) continue;
            const unsigned count=psx_mod_read_half(piece+0x14);
            if(!count || count>2048 || piece+0x1c+8*count>0x80200000u) continue;
            for(unsigned j=0;j<count;++j) {
                const uint32_t w0=psx_mod_read_word(piece+0x1c+8*j),w1=psx_mod_read_word(piece+0x20+8*j);
                ScanVert s;
                s.p[0]=org[0]+(int16_t)(((w0&0xff)<<10)&0xffff);
                s.p[1]=org[1]+(int16_t)((((w0>>8)&0xff)<<7)&0xffff);
                s.p[2]=org[2]+(int16_t)((((w0>>16)&0xff)<<10)&0xffff);
                s.c[0]=w1&0xff;s.c[1]=(w1>>8)&0xff;s.c[2]=(w1>>16)&0xff;
                s.lum=std::max({s.c[0],s.c[1],s.c[2]});s.room=r;
                s.wave=(w0>>24)&0x10;s.phase=(w1>>24)&31;
                v.push_back(s);
            }
        }
    }
    return true;
}
static std::vector<ScanCand> level_lamps_scan(int threshold) {
    std::vector<ScanVert> all;std::vector<ScanCand> c;
    if(!level_lamps_scan_verts(all)) return c;
    std::vector<const ScanVert*> hot;
    for(const ScanVert& s:all) if(s.lum>=threshold) hot.push_back(&s);
    std::stable_sort(hot.begin(),hot.end(),[](const ScanVert* a,const ScanVert* b){return a->lum>b->lum;});
    std::vector<bool> used(hot.size());
    for(size_t i=0;i<hot.size();++i) {
        if(used[i]) continue;
        const ScanVert& v=*hot[i];
        double wsum=0,cx=0,cz=0,col[3]={0,0,0};unsigned n=0;std::vector<const ScanVert*> pts;
        for(size_t j=0;j<hot.size();++j) {
            if(used[j]) continue;
            const ScanVert& u=*hot[j];
            if(std::fabs(u.p[0]-v.p[0])>2560 || std::fabs(u.p[2]-v.p[2])>2560 || std::fabs(u.p[1]-v.p[1])>4096) continue;
            used[j]=true;pts.push_back(&u);
        }
        if(pts.size()<3) continue;
        for(const ScanVert* p:pts) {
            const double wt=p->lum-threshold+10;wsum+=wt;cx+=p->p[0]*wt;cz+=p->p[2]*wt;
            for(int k=0;k<3;++k) col[k]+=p->c[k];++n;
        }
        cx/=wsum;cz/=wsum;
        const double m=std::max({col[0],col[1],col[2],1.0});
        double lo=1e18,hi=-1e18;
        for(const ScanVert& u:all)
            if(u.room==v.room && std::fabs(u.p[0]-v.p[0])<=1024 && std::fabs(u.p[2]-v.p[2])<=1024) {lo=std::min(lo,u.p[1]);hi=std::max(hi,u.p[1]);}
        double ext=0;for(const ScanVert* p:pts) ext=std::max(ext,std::hypot(p->p[0]-cx,p->p[2]-cz));
        ScanCand s{{cx,hi-0.65*(hi-lo),cz},{col[0]/m,col[1]/m,col[2]/m},std::min(6000.0,std::max(2500.0,ext*1.3+1500)),v.room,n};
        for(const ScanVert* p:pts) if(p->wave) {
            const double a=p->phase*(2*3.14159265358979/32);s.wx+=std::cos(a);s.wy+=std::sin(a);++s.nw;
        }
        c.push_back(s);
    }
    for(bool merged=true;merged;) {
        merged=false;
        for(size_t a=0;a<c.size() && !merged;++a) for(size_t b=0;b<c.size() && !merged;++b) {
            if(a==b || c[a].room!=c[b].room) continue;
            if(std::fabs(c[a].rgb[0]-c[b].rgb[0])+std::fabs(c[a].rgb[1]-c[b].rgb[1])+std::fabs(c[a].rgb[2]-c[b].rgb[2])>0.35) continue;
            const double sep=std::hypot(c[a].p[0]-c[b].p[0],c[a].p[2]-c[b].p[2]);
            if(sep>6000) continue;
            const double na=c[a].n,nb=c[b].n,t=na+nb;
            for(int k=0;k<3;++k) {c[a].p[k]=(c[a].p[k]*na+c[b].p[k]*nb)/t;c[a].rgb[k]=(c[a].rgb[k]*na+c[b].rgb[k]*nb)/t;}
            c[a].radius=std::min(6000.0,std::max(c[a].radius,c[b].radius)+sep/2);c[a].n+=c[b].n;
            c[a].wx+=c[b].wx;c[a].wy+=c[b].wy;c[a].nw+=c[b].nw;
            c.erase(c.begin()+b);merged=true;
        }
    }
    std::stable_sort(c.begin(),c.end(),[](const ScanCand& a,const ScanCand& b){return a.n>b.n;});
    std::vector<ScanCand> keep;std::map<unsigned,unsigned> per_room;
    for(const ScanCand& s:c) if(s.n>=6 && per_room[s.room]++<3) keep.push_back(s);
    return keep;
}

// D41D1 sign scan. Static props (kind 1) and their meshes (+0x44; object
// renderer layout: +6 vertex count, +0x10 8-byte vertices, +0x14 polygon
// groups) are walked from the room object lists (+0x48, +0x4c). Each
// textured face's texels (UV box, through its CLUT) are scored as emitters:
// bright (max 0.75-0.95) and strongly coloured (saturation 0.6-0.9), stricter
// than the shader rule that keeps them crisp, so sand, wood and first-aid
// boxes are left out. Line art on a dark ground (neon outlines: at least 40%
// of the face's texels darker than 0.2) counts three times its emitter
// fraction, as the tubes light the whole panel. The lamp's colour is the
// sign's dominant hue: lit texels (max 0.6-0.9, saturation 0.3-0.6, looser
// than the test) in 12 hue bins weighted by face area, the strongest bin and
// its two neighbours averaged (the club's pink neon over its yellow bulbs).
// A prop whose glowing area (face area x emitter
// fraction) reaches sign_min_glow gets one lamp sign_offset in front of its
// glowing faces (outward = minus the corner cross product: TTK's faces wind
// that way, 3104 of 3462 closed-model faces on level 0), in their colour.
// Neon signs are never lit: a face of neon on a dark ground (40% of texels
// darker than 0.2, 10% at max 0.7 or more and saturation 0.5 or more, flame
// hues 12-70 degrees left out) is left out of dynamic light (near_clip
// lit_frame, near_clip_set_glow_faces). D41K: only that face; the rest of
// the prop (a control panel beside its screen, the bar's shelves)
// takes light like any prop. Its lamp still lights the room around it; no
// light falls on its letters, which the D41D per-texel rule cannot hold when
// the neon is stored dim and brightened by the corner colour (the club's
// inner Bootylicious sign, type 51; user report, UI slot 7). Lanterns,
// torches and lit panels still take light (animated faces never count).
// Pickups (type flag 0x20000000) are skipped. Animated faces (neon that
// changes frame) are scored on their current frame from the object
// animation table (near_clip_object_animation; the same step as the object
// renderer's 0x800105d0), and a prop that glows mostly through them gets the
// candle style when warm (lantern and torch flames) or flicker (neon).
// gpu_vram_peek: gpu.h (through the backend, which syncs its copy first).
static constexpr double sign_min_glow=3000,sign_offset=300;
struct SignCand {double p[3];double rgb[3];double radius,glow;unsigned type,room;bool animated;};
static inline double lamp_ss(double a,double b,double x) {const double t=std::clamp((x-a)/(b-a),0.0,1.0);return t*t*(3-2*t);}
static std::vector<SignCand> level_lamps_signs(std::vector<uint32_t>* neon=nullptr) {
    std::vector<SignCand> out;
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c),types=psx_mod_read_word(0x800d2660);
    if(!rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return out;
    const bool types_ok=types>=0x80010000u && types<0x80200000u-0x7428u && !(types&3);
    std::vector<uint16_t> vram(1024*512);
    for(int y=0;y<512;++y) for(int x=0;x<1024;++x) vram[y*1024+x]=gpu_vram_peek(x,y);
    auto texel=[&](int x,int y){return vram[(y&511)*1024+(x&1023)];};
    struct Tex {double w=0,n=0,dark=0,neon=0,col[3]={0,0,0},hw[12]={},hrgb[12][3]={};};
    std::map<std::array<uint32_t,3>,Tex> texes;
    auto tex_score=[&](uint32_t t1,uint32_t t2,uint32_t t3,unsigned nc)->const Tex& {
        const std::array<uint32_t,3> key{t1,t2&0x1ffffffu,nc==4?t3:t3&0xffffu};
        if(auto it=texes.find(key);it!=texes.end()) return it->second;
        Tex& t=texes[key];
        const uint32_t clut=t1>>16,tp=(t2>>16)&0x1ff;
        const uint32_t uv[4]={t1&0xffff,t2&0xffff,t3&0xffff,t3>>16};
        int u0=255,u1=0,v0=255,v1=0;
        for(unsigned i=0;i<nc;++i) {u0=std::min<int>(u0,uv[i]&0xff);u1=std::max<int>(u1,uv[i]&0xff);v0=std::min<int>(v0,uv[i]>>8);v1=std::max<int>(v1,uv[i]>>8);}
        const int tx=(tp&0xf)*64,ty=((tp>>4)&1)*256,depth=(tp>>7)&3,cx=(clut&0x3f)*16,cy=(clut>>6)&0x1ff;
        for(int v=v0;v<=v1;++v) for(int u=u0;u<=u1;++u) {
            uint16_t c;
            if(depth==0) c=texel(cx+((texel(tx+u/4,ty+v)>>(4*(u&3)))&0xf),cy);
            else if(depth==1) c=texel(cx+((texel(tx+u/2,ty+v)>>(8*(u&1)))&0xff),cy);
            else c=texel(tx+u,ty+v);
            t.n+=1;
            const double r=(c&31)/31.0,g=((c>>5)&31)/31.0,b=((c>>10)&31)/31.0,mx=std::max({r,g,b}),mn=std::min({r,g,b});
            if(mx<0.2) t.dark+=1;
            if(!c) continue;
            if(mx>=0.7 && (mx-mn)/mx>=0.5) {
                // Flame colours (orange to yellow, hue 12-70 degrees) are lanterns
                // and torches, not neon.
                const double d=mx-mn,hue=mx==r?std::fmod(60*(g-b)/d+360,360.0):mx==g?60*(b-r)/d+120:60*(r-g)/d+240;
                if(hue<12 || hue>70) t.neon+=1;
            }
            const double sat=mx>0?(mx-mn)/mx:0;
            const double k=lamp_ss(0.75,0.95,mx)*lamp_ss(0.6,0.9,sat);
            t.w+=k;t.col[0]+=k*r;t.col[1]+=k*g;t.col[2]+=k*b;
            if(const double h=lamp_ss(0.6,0.9,mx)*lamp_ss(0.3,0.6,sat);h>0) {
                const double d=mx-mn;
                double hue=mx==r?std::fmod((g-b)/d+6,6.0):mx==g?(b-r)/d+2:(r-g)/d+4;  // 0-6
                const int bin=std::clamp((int)(hue*2),0,11);
                t.hw[bin]+=h;t.hrgb[bin][0]+=h*r;t.hrgb[bin][1]+=h*g;t.hrgb[bin][2]+=h*b;
            }
        }
        return t;
    };
    // Per mesh, in model space: glowing area, its weighted centre, outward
    // direction and colour.
    struct Glow {std::vector<uint32_t> neon;double e=0,animated=0,c[3]={0,0,0},out[3]={0,0,0},col[3]={0,0,0},hw[12]={},hrgb[12][3]={};};
    std::map<uint32_t,Glow> meshes;
    auto mesh_glow=[&](uint32_t mesh)->const Glow& {
        if(auto it=meshes.find(mesh);it!=meshes.end()) return it->second;
        Glow& g=meshes[mesh];
        const unsigned nv=psx_mod_read_byte(mesh+6);
        const uint32_t vp=psx_mod_read_word(mesh+0x10);uint32_t cur=psx_mod_read_word(mesh+0x14);
        if(!nv || vp<0x80010000u || vp>0x80200000u-8*nv || (vp&3)) return g;
        for(unsigned guard=0;guard<256;++guard) {
            if(cur<0x80010000u || cur>0x801ffffcu || (cur&3)) break;
            const uint32_t header=psx_mod_read_word(cur);cur+=4;
            const uint32_t type=header&0xffff,n=header>>16;
            if(type==0xff) break;
            unsigned size=0;
            switch(type) {case 0x20: case 0x28: size=8;break;case 0x30: case 0x38: size=12;break;case 0x24: case 0x2c: size=16;break;case 0x34: case 0x3c: size=20;break;}
            if(!size || !n || n>4096 || cur>0x80200000u-n*size) break;
            for(uint32_t k=0;k<n;++k,cur+=size) {
                if(type!=0x24 && type!=0x2c && type!=0x34 && type!=0x3c) continue;
                const uint32_t word=psx_mod_read_word(cur);
                uint32_t t1=psx_mod_read_word(cur+4),t2=psx_mod_read_word(cur+8),t3=psx_mod_read_word(cur+12);
                const bool animated=(t2>>25)&0x20;
                if(animated) {
                    const uint32_t anim=near_clip_object_animation();
                    if(!anim) continue;
                    const uint32_t frame=psx_mod_read_word(anim+(t1>>16)*16+8);
                    if(frame<0x80010000u || frame>0x801ffff8u || (frame&3)) continue;
                    const uint32_t s1=psx_mod_read_word(frame),s2=psx_mod_read_word(frame+4),s4=s1&0xffff;
                    t2=(t2&0xffff)+((s2&0xffff0000u)|s4);t1=(t1&0xffff)+s1;t3=t3+((s4<<16)|s4);
                }
                const unsigned nc=(type&8)?4:3;
                double P[4][3];bool bad=false;
                for(unsigned j=0;j<nc;++j) {
                    const unsigned i=(word>>(8*j))&0xff;if(i>=nv) {bad=true;break;}
                    for(int a=0;a<3;++a) P[j][a]=(int16_t)psx_mod_read_half(vp+8*i+2*a);
                }
                if(bad) continue;
                auto cross=[&](const double* a,const double* b,const double* c,double* o) {
                    const double u[3]={b[0]-a[0],b[1]-a[1],b[2]-a[2]},v[3]={c[0]-a[0],c[1]-a[1],c[2]-a[2]};
                    o[0]+=u[1]*v[2]-u[2]*v[1];o[1]+=u[2]*v[0]-u[0]*v[2];o[2]+=u[0]*v[1]-u[1]*v[0];
                };
                double nn[3]={0,0,0};cross(P[0],P[1],P[2],nn);if(nc==4) cross(P[1],P[3],P[2],nn);
                const double len=std::sqrt(nn[0]*nn[0]+nn[1]*nn[1]+nn[2]*nn[2]);
                if(len<1) continue;
                const Tex& t=tex_score(t1,t2,t3,nc);
                // Hue votes come from every textured face of the prop.
                const double per=t.n>0?len/2/t.n:0;
                for(int b=0;b<12;++b) {g.hw[b]+=per*t.hw[b];for(int a=0;a<3;++a) g.hrgb[b][a]+=per*t.hrgb[b][a];}
                // Neon on a dark ground (40% of texels below 0.2, 10% at
                // max 0.7 and saturation 0.5): a neon sign (see below).
                if(!animated && t.n>=64 && t.dark>=0.4*t.n && t.neon>=0.1*t.n) g.neon.push_back(cur); // flames (animated) are not signs
                double frac=t.n>0?t.w/t.n:0;
                if(frac>=0.01 && t.dark>=0.4*t.n) frac=std::min(1.0,frac*3);
                if(frac<0.05) continue;
                const double e=len/2*frac;
                for(int a=0;a<3;++a) {
                    double c=0;for(unsigned j=0;j<nc;++j) c+=P[j][a];
                    g.c[a]+=e*c/nc;g.out[a]-=e*nn[a]/len;g.col[a]+=e*t.col[a]/t.w;
                }
                g.e+=e;if(animated) g.animated+=e;
            }
        }
        if(g.e>0) for(int a=0;a<3;++a) {g.c[a]/=g.e;g.out[a]/=g.e;g.col[a]/=g.e;}
        int top=0;for(int b=1;b<12;++b) if(g.hw[b]>g.hw[top]) top=b;
        if(g.hw[top]>0) {
            double w=0,c[3]={0,0,0};
            for(int b:{(top+11)%12,top,(top+1)%12}) {w+=g.hw[b];for(int a=0;a<3;++a) c[a]+=g.hrgb[b][a];}
            for(int a=0;a<3;++a) g.col[a]=c[a]/w;
        }
        return g;
    };
    std::set<uint32_t> seen;
    for(uint32_t r=0;r<rooms;++r) for(uint32_t head_at:{0x48u,0x4cu}) {
        uint32_t node=psx_mod_read_word(table+88*r+head_at);
        for(unsigned guard=0;node && guard<1024;++guard) {
            if(node<0x80010000u || node>0x801ffff0u || (node&3)) break;
            const uint32_t o=psx_mod_read_word(node+8);node=psx_mod_read_word(node+4);
            if(o<0x80010000u || o>0x801fff00u || (o&3) || !seen.insert(o).second) continue;
            if(psx_mod_read_byte(o+0x14)!=1) continue;               // static props only
            const unsigned type=psx_mod_read_half(o+0x2c);
            if(types_ok && type<0x7428u/28 && (psx_mod_read_word(types+28*type)&0x20000000u)) continue; // pickups
            const uint32_t mesh=psx_mod_read_word(o+0x44);
            if(mesh<0x80010000u || mesh>0x801ffff0u || (mesh&3)) continue;
            const Glow& g=mesh_glow(mesh);
            if(neon) neon->insert(neon->end(),g.neon.begin(),g.neon.end());
            if(std::getenv("DNTTK_LAMP_TRACE") && !g.neon.empty())
                std::fprintf(stderr,"[TTK lamp]   prop %08x type %u mesh %08x: %zu neon faces (first %08x)\n",o,type,mesh,g.neon.size(),g.neon[0]);
            if(std::getenv("DNTTK_LAMP_TRACE") && g.e>0)
                std::fprintf(stderr,"[TTK lamp]   prop %08x type %u room %u glow %.0f animated %.0f (table %08x)\n",o,type,r,g.e,g.animated,near_clip_object_animation());
            if(g.e<sign_min_glow) continue;
            // Model to world: yaw +0x1c (4096 a turn, 0 = +Z, 1024 = +X).
            const double a=(int16_t)psx_mod_read_half(o+0x1c)*(2*3.14159265358979/4096),ca=std::cos(a),sa=std::sin(a);
            auto rot=[&](const double* v,double* w){w[0]=v[0]*ca+v[2]*sa;w[1]=v[1];w[2]=-v[0]*sa+v[2]*ca;};
            double c[3],dir[3];rot(g.c,c);rot(g.out,dir);
            const double dl=std::sqrt(dir[0]*dir[0]+dir[1]*dir[1]+dir[2]*dir[2]);
            SignCand s{};s.type=type;s.room=r;s.glow=g.e;s.animated=g.animated*2>g.e;
            for(int k=0;k<3;++k) s.p[k]=(int32_t)psx_mod_read_word(o+4+4*k)+c[k]+(dl>0.3?dir[k]/dl*sign_offset:0);
            const double m=std::max({g.col[0],g.col[1],g.col[2],1e-6});
            for(int k=0;k<3;++k) s.rgb[k]=g.col[k]/m;
            s.radius=std::clamp(1200+6*std::sqrt(g.e),1500.0,5000.0);
            out.push_back(s);
        }
    }
    // Props stacked in one place (a tube light drawn twice) give one lamp.
    for(size_t i=0;i<out.size();++i) for(size_t j=i+1;j<out.size();) {
        double d=0,dc=0;for(int k=0;k<3;++k) {d+=(out[i].p[k]-out[j].p[k])*(out[i].p[k]-out[j].p[k]);dc+=std::fabs(out[i].rgb[k]-out[j].rgb[k]);}
        if(d<600.0*600.0 && dc<0.35) {
            const double t=out[i].glow+out[j].glow;
            for(int k=0;k<3;++k) {out[i].p[k]=(out[i].p[k]*out[i].glow+out[j].p[k]*out[j].glow)/t;out[i].rgb[k]=(out[i].rgb[k]*out[i].glow+out[j].rgb[k]*out[j].glow)/t;}
            out[i].radius=std::max(out[i].radius,out[j].radius);out[i].glow=t;out.erase(out.begin()+j);
        } else ++j;
    }
    return out;
}
// Removes lamps whose note starts with prefix; returns how many are left.
static size_t level_lamps_drop_notes(const char* prefix) {
    const size_t n=std::strlen(prefix);
    level_lamps.erase(std::remove_if(level_lamps.begin(),level_lamps.end(),[&](const LevelLamp& l){return !l.note.compare(0,n,prefix);}),level_lamps.end());
    level_lamp_selected=-1;
    return level_lamps.size();
}

// The aim point: where the view meets the world within 12000, backed off 250
// toward the eye; 1500 ahead when nothing is hit.
static void level_lamp_aim(CPUState* cpu,double out[3]) {
    double eye[3],fwd[3],end[3],hit[3];
    for(int i=0;i<3;++i) {
        eye[i]=(int32_t)psx_mod_read_word(camera+0x14+4*i);
        fwd[i]=(int16_t)psx_mod_read_half(camera+0x20+2*(6+i))/4096.0;
        end[i]=eye[i]+fwd[i]*12000;
    }
    int kind=0;
    if(view_segment_query(cpu,eye,end,hit,kind) && kind) {for(int i=0;i<3;++i) out[i]=hit[i]-fwd[i]*250;}
    else for(int i=0;i<3;++i) out[i]=eye[i]+fwd[i]*1500;
}
static void level_lamp_select(int i) {level_lamp_selected=i;level_lamp_flash_until=light_ticks+75;}
static void level_lamp_notice(const char* fmt,...) {
    char b[160];va_list ap;va_start(ap,fmt);std::vsnprintf(b,sizeof b,fmt,ap);va_end(ap);
    input_notice(b);
}
static void level_lamp_describe(char* b,size_t n,unsigned i) {
    const LevelLamp& l=level_lamps[i];
    std::snprintf(b,n,"lamp %u: %.2f %.2f %.2f r %.0f x%.2f %s at %.0f %.0f %.0f%s%s",i,l.rgb[0],l.rgb[1],l.rgb[2],l.radius,l.k,
                  lamp_style_text(l).c_str(),l.p[0],l.p[1],l.p[2],l.note.empty()?"":" - ",l.note.c_str());
}
static void level_lamp_snapshot() {
    std::vector<std::string> rows;char b[200];
    for(unsigned i=0;i<level_lamps.size();++i) {level_lamp_describe(b,sizeof b,i);rows.push_back(b);}
    std::lock_guard<std::mutex> g(level_lamp_mutex);level_lamp_listing.swap(rows);level_lamp_listing_level=level_lamps_level;
}
// One queued editor command on the game thread.
static void level_lamp_run(CPUState* cpu,const std::string& cmd) {
    char verb[16]={0};const char* rest=cmd.c_str();
    int used=0;std::sscanf(rest,"%15s%n",verb,&used);rest+=used;while(*rest==' ')++rest;
    const bool sel=level_lamp_selected>=0 && level_lamp_selected<(int)level_lamps.size();
    LevelLamp* s=sel?&level_lamps[level_lamp_selected]:nullptr;
    char b[200];
    auto need_sel=[&]{if(!s) level_lamp_notice("Lamp: none selected (lamp pick / lamp add)");return s!=nullptr;};
    if(!std::strcmp(verb,"add") || !std::strcmp(verb,"here")) {
        LevelLamp l{};l.rgb[0]=1.0f;l.rgb[1]=0.85f;l.rgb[2]=0.6f;l.radius=4000;l.k=1;l.style=lamp_steady;
        if(s) {for(int i=0;i<3;++i) l.rgb[i]=s->rgb[i];l.radius=s->radius;l.k=s->k;l.style=s->style;}
        float r,g,bl;double rad;const int n=std::sscanf(rest,"%f %f %f %lf",&r,&g,&bl,&rad);
        if(n>=3) {l.rgb[0]=r;l.rgb[1]=g;l.rgb[2]=bl;}
        if(n>=4) l.radius=std::clamp(rad,300.0,30000.0);
        if(verb[0]=='h') {for(int i=0;i<3;++i) l.p[i]=(int32_t)psx_mod_read_word(player+4+4*i);l.p[1]-=900;}
        else level_lamp_aim(cpu,l.p);
        l.room=-2;level_lamps.push_back(l);level_lamp_select((int)level_lamps.size()-1);level_lamps_dirty=true;
        level_lamp_describe(b,sizeof b,level_lamp_selected);level_lamp_notice("Lamp added: %s",b);
    } else if(!std::strcmp(verb,"pick")) {
        int want=-1;
        if(*rest) want=std::atoi(rest);
        else {
            double aim[3];level_lamp_aim(cpu,aim);double best=1e18;
            for(unsigned i=0;i<level_lamps.size();++i) {
                double d=0;for(int k=0;k<3;++k) d+=(level_lamps[i].p[k]-aim[k])*(level_lamps[i].p[k]-aim[k]);
                if(d<best) {best=d;want=(int)i;}
            }
        }
        if(want<0 || want>=(int)level_lamps.size()) {level_lamp_notice("Lamp: no lamp to pick");return;}
        level_lamp_select(want);level_lamp_describe(b,sizeof b,want);level_lamp_notice("Lamp picked: %s",b);
    } else if(!std::strcmp(verb,"move")) {
        if(!need_sel()) return;
        level_lamp_aim(cpu,s->p);s->room=-2;level_lamps_dirty=true;level_lamp_select(level_lamp_selected);
        level_lamp_describe(b,sizeof b,level_lamp_selected);level_lamp_notice("Lamp moved: %s",b);
    } else if(!std::strcmp(verb,"nudge")) {
        if(!need_sel()) return;
        double d[3]={0,0,0};std::sscanf(rest,"%lf %lf %lf",&d[0],&d[1],&d[2]);
        for(int i=0;i<3;++i) s->p[i]+=std::clamp(d[i],-8000.0,8000.0);
        s->room=-2;
        level_lamps_dirty=true;level_lamp_select(level_lamp_selected);
        level_lamp_describe(b,sizeof b,level_lamp_selected);level_lamp_notice("Lamp: %s",b);
    } else if(!std::strcmp(verb,"color") || !std::strcmp(verb,"colour")) {
        if(!need_sel()) return;
        float r,g,bl;if(std::sscanf(rest,"%f %f %f",&r,&g,&bl)!=3) {level_lamp_notice("Lamp: lamp color <r> <g> <b> (0-4)");return;}
        s->rgb[0]=std::clamp(r,0.0f,4.0f);s->rgb[1]=std::clamp(g,0.0f,4.0f);s->rgb[2]=std::clamp(bl,0.0f,4.0f);level_lamps_dirty=true;
        level_lamp_describe(b,sizeof b,level_lamp_selected);level_lamp_notice("Lamp: %s",b);
    } else if(!std::strcmp(verb,"radius")) {
        if(!need_sel()) return;
        s->radius=std::clamp(std::atof(rest),300.0,30000.0);level_lamps_dirty=true;level_lamp_select(level_lamp_selected);
        level_lamp_describe(b,sizeof b,level_lamp_selected);level_lamp_notice("Lamp: %s",b);
    } else if(!std::strcmp(verb,"power")) {
        if(!need_sel()) return;
        s->k=(float)std::clamp(std::atof(rest),0.0,4.0);level_lamps_dirty=true;
        level_lamp_describe(b,sizeof b,level_lamp_selected);level_lamp_notice("Lamp: %s",b);
    } else if(!std::strcmp(verb,"style")) {
        if(!need_sel()) return;
        LevelLamp t=*s;
        if(!lamp_style_parse(rest,t)) {level_lamp_notice("Lamp: styles steady flicker pulse candle strobe broken wave [phase 0-31] [depth 0-1]");return;}
        s->style=t.style;s->phase=t.phase;s->depth=t.depth;level_lamps_dirty=true;level_lamp_flash_until=0;
        level_lamp_describe(b,sizeof b,level_lamp_selected);level_lamp_notice("Lamp: %s",b);
    } else if(!std::strcmp(verb,"note")) {
        if(!need_sel()) return;
        s->note=rest;level_lamps_dirty=true;level_lamp_notice("Lamp %d note: %s",level_lamp_selected,rest);
    } else if(!std::strcmp(verb,"del")) {
        if(!need_sel()) return;
        level_lamps.erase(level_lamps.begin()+level_lamp_selected);level_lamps_dirty=true;
        level_lamp_notice("Lamp %d deleted (%zu left)",level_lamp_selected,level_lamps.size());level_lamp_selected=-1;
    } else if(!std::strcmp(verb,"save")) {
        std::string error;
        if(level_lamps_save(error)) level_lamp_notice("Lamps saved: %zu to level%02u.lights",level_lamps.size(),level_lamps_level);
        else level_lamp_notice("Lamp save failed: %s",error.c_str());
        std::fprintf(stderr,"[TTK lamp] save level %u: %zu lamps to %s %s\n",level_lamps_level,level_lamps.size(),level_lamps_path_used.c_str(),error.c_str());
    } else if(!std::strcmp(verb,"reload")) {
        level_lamps_load(level_lamps_level);level_lamp_notice("Lamps reloaded: %zu",level_lamps.size());
    } else if(!std::strcmp(verb,"clear")) {
        level_lamps.clear();level_lamp_selected=-1;level_lamps_dirty=true;level_lamp_notice("Lamps cleared (lamp reload undoes, lamp save keeps)");
    } else if(!std::strcmp(verb,"signs")) {
        const std::vector<SignCand> c=level_lamps_signs();
        const size_t kept=level_lamps_drop_notes("sign ");
        // A placed lamp (not from a scan) of a similar colour within 2500 of
        // a sign already lights it: placed lamps win.
        const std::vector<LevelLamp> placed=[&]{std::vector<LevelLamp> v;for(const LevelLamp& l:level_lamps) if(l.note.compare(0,9,"scan room")) v.push_back(l);return v;}();
        size_t skipped=0;
        for(const SignCand& k:c) {
            bool covered=false;
            for(const LevelLamp& q:placed) {
                double d=0,dc=0;for(int i=0;i<3;++i) {d+=(q.p[i]-k.p[i])*(q.p[i]-k.p[i]);dc+=std::fabs(q.rgb[i]/std::max({q.rgb[0],q.rgb[1],q.rgb[2],0.01f})-k.rgb[i]);}
                covered|=d<2500.0*2500.0 && dc<0.6;
            }
            if(covered) {++skipped;std::fprintf(stderr,"[TTK lamp]   sign type %u room %u: a placed lamp already lights it\n",k.type,k.room);continue;}
            LevelLamp l{};for(int i=0;i<3;++i) {l.p[i]=std::round(k.p[i]);l.rgb[i]=(float)(std::round(k.rgb[i]*100)/100);}
            // Animated glow: a warm one is a flame (lanterns, torches), the rest neon.
            const bool warm=k.rgb[0]>=0.99 && k.rgb[1]>=0.4 && k.rgb[2]<0.5;
            l.radius=std::round(k.radius);l.k=1.0f;l.style=k.animated?(warm?lamp_candle:lamp_flicker):lamp_steady;
            char note[48];std::snprintf(note,sizeof note,"sign type %u room %u%s",k.type,k.room,k.animated?" animated":"");l.note=note;
            level_lamps.push_back(l);
        }
        level_lamps_dirty=true;
        level_lamp_notice("Lamp signs: %zu glowing props, %zu lamps (%zu lit by placed lamps; %zu other lamps kept; lamp save keeps them)",c.size(),c.size()-skipped,skipped,kept);
        std::fprintf(stderr,"[TTK lamp] signs level %u: %zu lamps, %zu kept\n",level_lamps_level,c.size(),kept);
        for(const SignCand& k:c) std::fprintf(stderr,"[TTK lamp]   sign type %u room %u at %.0f %.0f %.0f rgb %.2f %.2f %.2f glow %.0f r %.0f\n",
                                               k.type,k.room,k.p[0],k.p[1],k.p[2],k.rgb[0],k.rgb[1],k.rgb[2],k.glow,k.radius);
    } else if(!std::strcmp(verb,"scan")) {
        int threshold=170;if(*rest) threshold=std::clamp(std::atoi(rest),96,250);
        const std::vector<ScanCand> c=level_lamps_scan(threshold);
        // D41D1: earlier scan lamps are replaced; placed lamps and sign lamps stay.
        const size_t kept=level_lamps_drop_notes("scan room");
        unsigned waves=0;
        for(const ScanCand& k:c) {
            LevelLamp l{};for(int i=0;i<3;++i) {l.p[i]=std::round(k.p[i]);l.rgb[i]=(float)(std::round(k.rgb[i]*100)/100);}
            l.radius=std::round(k.radius);l.k=0.6f;l.style=lamp_steady;
            // The original light ripples here: follow it where the phases agree.
            const double coherence=k.nw?std::hypot(k.wx,k.wy)/k.nw:0;
            if(k.nw>=6 && k.nw*2>=k.n && coherence>=0.3) {
                l.style=lamp_wave;l.depth=(float)(std::round(std::min(1.0,coherence)*20)/20);
                l.phase=((int)std::lround(std::atan2(k.wy,k.wx)/(2*3.14159265358979)*32)%32+32)%32;++waves;
            }
            char note[32];std::snprintf(note,sizeof note,"scan room %u",k.room);l.note=note;
            level_lamps.push_back(l);
        }
        level_lamps_dirty=true;
        level_lamp_notice("Lamp scan: %zu lamps from the painted light, %u follow its wave (%zu others kept; lamp save keeps them)",c.size(),waves,kept);
        std::fprintf(stderr,"[TTK lamp] scan level %u threshold %d: %zu candidates, %u wave, %zu kept\n",level_lamps_level,threshold,c.size(),waves,kept);
    }
    level_lamp_snapshot();
}
// Game thread, beside spawn_update (same guards for the aim query).
static void level_lamps_update(CPUState* cpu) {
    if(!lights_enabled() || !gameplay_context()) return;
    const unsigned level=psx_mod_read_word(0x800be570);
    if(level!=level_lamps_level) {level_lamps_load(level);level_lamp_snapshot();}
    // D41D1: the neon sign set, 45 ticks after a level or savestate load
    // (the level's textures in place), and once more when the object
    // animation table first shows up (animated signs).
    static unsigned neon_level=~0u,neon_loads=~0u;static uint32_t neon_due;static bool neon_had_table;
    if(const unsigned loads=psx_mod_savestate_loads();level!=neon_level || loads!=neon_loads) {
        neon_level=level;neon_loads=loads;neon_due=light_ticks+45;neon_had_table=false;near_clip_set_glow_faces({});
    }
    if((neon_due && light_ticks>=neon_due) || (!neon_due && !neon_had_table && near_clip_object_animation())) {
        neon_due=0;neon_had_table=near_clip_object_animation()!=0;
        const auto t0=std::chrono::steady_clock::now();
        std::vector<uint32_t> neon;level_lamps_signs(&neon);
        const double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count();
        std::fprintf(stderr,"[TTK lamp] level %u: %zu neon sign faces take no dynamic light (%.1f ms%s)\n",level,neon.size(),ms,neon_had_table?"":", animated faces not yet known");
        near_clip_set_glow_faces(std::move(neon));
    }
    std::deque<std::string> todo;
    {std::lock_guard<std::mutex> g(level_lamp_mutex);todo.swap(level_lamp_queue);}
    if(todo.empty()) return;
    if(!player_identity_ready() || psx_mod_read_word(0x800c27bc)!=1 || guest_busy ||
       cpu->gpr[29]<0x801f4000 || g_precise_mode || g_ls_mode || g_psx_call_bail) {
        std::lock_guard<std::mutex> g(level_lamp_mutex);
        for(auto it=todo.rbegin();it!=todo.rend();++it) level_lamp_queue.push_front(*it);
        return;
    }
    for(const std::string& c:todo) level_lamp_run(cpu,c);
}
// Backtick console: "lamp ..." (help with "lamp").
bool lamp_console_command(const char* line,void (*say)(const char*),bool& close) {
    close=false;
    if(std::strncmp(line,"lamp",4) || (line[4] && line[4]!=' ')) return false;
    if(!input_modernized()) {say("Lamp: Modernized profile only (Vanilla is unchanged)");return true;}
    if(ttk_world_classic()) {say("Lamp: level lights need the Polymorph renderer");return true;}
    const char* arg=line+4;while(*arg==' ')++arg;
    if(!*arg || !std::strcmp(arg,"help")) {
        say("Lamp (level lights): lamp list | scan [threshold] | signs | add [r g b [radius]] | here | pick [n]");
        say("  move | nudge dx dy dz | color r g b | radius R | power K | style S | note text | del");
        say("  save | reload | clear | off | on | gate on|off | props on|off.  Styles: steady flicker pulse candle strobe broken wave [phase] [depth]");
        say("  scan: lamps from the painted light; signs: lamps at glowing props (EXIT, neon). Each replaces its own.");
        say("  add/move/pick use the point the crosshair meets. Edits show at once; save writes the level file.");
        return true;
    }
    if(!std::strcmp(arg,"off")) {level_lamps_shown.store(0);say("Lamp: level lights hidden (session)");return true;}
    if(!std::strcmp(arg,"on")) {level_lamps_shown.store(1);say("Lamp: level lights shown");return true;}
    if(!std::strcmp(arg,"props off")) {prop_lights_enabled.store(false);say("Lamp: woodburners and chandeliers dark (session)");return true;}
    if(!std::strcmp(arg,"props on") || !std::strcmp(arg,"props")) {prop_lights_enabled.store(true);say("Lamp: woodburners and chandeliers lit");return true;}
    if(!std::strcmp(arg,"gate off")) {level_lamps_gate.store(0);say("Lamp: lamps light through walls from unseen rooms (session)");return true;}
    if(!std::strcmp(arg,"gate on") || !std::strcmp(arg,"gate")) {level_lamps_gate.store(1);say("Lamp: a lamp lights only while its room is shown");return true;}
    if(!std::strcmp(arg,"list")) {
        std::lock_guard<std::mutex> g(level_lamp_mutex);
        char head[96];std::snprintf(head,sizeof head,"Level %u: %zu lamps",level_lamp_listing_level,level_lamp_listing.size());say(head);
        for(const std::string& r:level_lamp_listing) say(r.c_str());
        return true;
    }
    static const char* const verbs[]={"add","here","pick","move","nudge","color","colour","radius","power","style","note","del","save","reload","clear","scan","signs"};
    bool known=false;for(const char* v:verbs) {const size_t n=std::strlen(v);if(!std::strncmp(arg,v,n) && (!arg[n] || arg[n]==' ')) known=true;}
    if(!known) {say("Lamp: unknown command (type lamp for help)");return true;}
    {std::lock_guard<std::mutex> g(level_lamp_mutex);if(level_lamp_queue.size()<64) level_lamp_queue.emplace_back(arg);}
    // Placement commands close the console so the result is visible.
    close=!std::strncmp(arg,"add",3) || !std::strncmp(arg,"here",4) || !std::strncmp(arg,"move",4) || !std::strncmp(arg,"pick",4);
    if(!close) say("Lamp: done (see the notice)");
    return true;
}

// D41L (prop_lights.cpp): woodburners and chandeliers, with the room gate.
static unsigned prop_lamps_publish(PSXModLight* out,unsigned room) {
    if(!prop_light_publish || !level_lamps_shown.load(std::memory_order_relaxed)) return 0;
    return prop_light_publish(out,room,[](uint32_t r) {
        return !level_lamps_gate.load(std::memory_order_relaxed) || !host_vis_shown_valid || (r<host_vis_shown.size() && host_vis_shown[r]);
    });
}
