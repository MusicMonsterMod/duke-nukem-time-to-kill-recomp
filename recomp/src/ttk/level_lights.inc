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
//
// File format, one lamp per line ('#' starts a comment):
//   x y z  r g b  radius  strength  style  [# note]
// (Standard headers are included at the top of modern_controls.cpp.)
#ifndef TTK_LIGHTS_DIR
#define TTK_LIGHTS_DIR ""
#endif
struct LevelLamp {double p[3];float rgb[3];double radius;float k;int style;std::string note;};
enum {lamp_steady,lamp_flicker,lamp_pulse,lamp_candle,lamp_strobe,lamp_broken,lamp_styles};
static const char* const lamp_style_names[lamp_styles]={"steady","flicker","pulse","candle","strobe","broken"};
static constexpr unsigned level_lamp_max=12;          // of PSX_MOD_MAX_LIGHTS per image
static constexpr double level_lamp_margin=6000,level_lamp_fade=2000;
static std::vector<LevelLamp> level_lamps;           // game thread
static unsigned level_lamps_level=~0u;
static bool level_lamps_dirty;                        // edited since load or save
static int level_lamp_selected=-1;
static uint32_t level_lamp_flash_until;               // light_ticks: the selected lamp blinks
static std::atomic<int> level_lamps_shown{1};         // console "lamp off|on" (session)
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
        l.style=n>=9?lamp_style_from(style):lamp_steady;
        if(l.style<0) {++bad;continue;}
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
        f<<"# D41D level "<<level_lamps_level<<" lamps (documentation/428-d41d-level-lights.md)\n";
        f<<"# x y z  r g b  radius  strength  style  # note\n";
        char row[200];
        for(const LevelLamp& l:level_lamps) {
            std::snprintf(row,sizeof row,"%.0f %.0f %.0f  %.2f %.2f %.2f  %.0f  %.2f  %s",
                          l.p[0],l.p[1],l.p[2],l.rgb[0],l.rgb[1],l.rgb[2],l.radius,l.k,lamp_style_names[l.style]);
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
    unsigned k=0;
    for(unsigned j=0;j<n && k<std::min(room,level_lamp_max);++j) {
        const LevelLamp& l=level_lamps[near[j].i];
        float e=l.k*lamp_style_value(l.style,light_ticks,near[j].i);
        e*=(float)std::clamp((level_lamp_margin-near[j].d)/level_lamp_fade,0.0,1.0);
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
struct ScanVert {double p[3];int c[3];int lum;unsigned room;};
struct ScanCand {double p[3];double rgb[3];double radius;unsigned room,n;};
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
            c.erase(c.begin()+b);merged=true;
        }
    }
    std::stable_sort(c.begin(),c.end(),[](const ScanCand& a,const ScanCand& b){return a.n>b.n;});
    std::vector<ScanCand> keep;std::map<unsigned,unsigned> per_room;
    for(const ScanCand& s:c) if(s.n>=6 && per_room[s.room]++<3) keep.push_back(s);
    return keep;
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
                  lamp_style_names[l.style],l.p[0],l.p[1],l.p[2],l.note.empty()?"":" - ",l.note.c_str());
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
        level_lamps.push_back(l);level_lamp_select((int)level_lamps.size()-1);level_lamps_dirty=true;
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
        level_lamp_aim(cpu,s->p);level_lamps_dirty=true;level_lamp_select(level_lamp_selected);
        level_lamp_describe(b,sizeof b,level_lamp_selected);level_lamp_notice("Lamp moved: %s",b);
    } else if(!std::strcmp(verb,"nudge")) {
        if(!need_sel()) return;
        double d[3]={0,0,0};std::sscanf(rest,"%lf %lf %lf",&d[0],&d[1],&d[2]);
        for(int i=0;i<3;++i) s->p[i]+=std::clamp(d[i],-8000.0,8000.0);
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
        const int st=lamp_style_from(rest);
        if(st<0) {level_lamp_notice("Lamp: styles steady flicker pulse candle strobe broken");return;}
        s->style=st;level_lamps_dirty=true;level_lamp_flash_until=0;
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
    } else if(!std::strcmp(verb,"scan")) {
        int threshold=170;if(*rest) threshold=std::clamp(std::atoi(rest),96,250);
        const std::vector<ScanCand> c=level_lamps_scan(threshold);
        level_lamps.clear();level_lamp_selected=-1;
        for(const ScanCand& k:c) {
            LevelLamp l{};for(int i=0;i<3;++i) {l.p[i]=std::round(k.p[i]);l.rgb[i]=(float)(std::round(k.rgb[i]*100)/100);}
            l.radius=std::round(k.radius);l.k=0.6f;l.style=lamp_steady;
            char note[32];std::snprintf(note,sizeof note,"scan room %u",k.room);l.note=note;
            level_lamps.push_back(l);
        }
        level_lamps_dirty=true;
        level_lamp_notice("Lamp scan: %zu lamps from the painted light (lamp save keeps them)",level_lamps.size());
        std::fprintf(stderr,"[TTK lamp] scan level %u threshold %d: %zu candidates\n",level_lamps_level,threshold,level_lamps.size());
    }
    level_lamp_snapshot();
}
// Game thread, beside spawn_update (same guards for the aim query).
static void level_lamps_update(CPUState* cpu) {
    if(!lights_enabled() || !gameplay_context()) return;
    const unsigned level=psx_mod_read_word(0x800be570);
    if(level!=level_lamps_level) {level_lamps_load(level);level_lamp_snapshot();}
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
        say("Lamp (level lights): lamp list | scan [threshold] | add [r g b [radius]] | here | pick [n]");
        say("  move | nudge dx dy dz | color r g b | radius R | power K | style S | note text | del");
        say("  save | reload | clear | off | on.  Styles: steady flicker pulse candle strobe broken");
        say("  add/move/pick use the point the crosshair meets. Edits show at once; save writes the level file.");
        return true;
    }
    if(!std::strcmp(arg,"off")) {level_lamps_shown.store(0);say("Lamp: level lights hidden (session)");return true;}
    if(!std::strcmp(arg,"on")) {level_lamps_shown.store(1);say("Lamp: level lights shown");return true;}
    if(!std::strcmp(arg,"list")) {
        std::lock_guard<std::mutex> g(level_lamp_mutex);
        char head[96];std::snprintf(head,sizeof head,"Level %u: %zu lamps",level_lamp_listing_level,level_lamp_listing.size());say(head);
        for(const std::string& r:level_lamp_listing) say(r.c_str());
        return true;
    }
    static const char* const verbs[]={"add","here","pick","move","nudge","color","colour","radius","power","style","note","del","save","reload","clear","scan"};
    bool known=false;for(const char* v:verbs) {const size_t n=std::strlen(v);if(!std::strncmp(arg,v,n) && (!arg[n] || arg[n]==' ')) known=true;}
    if(!known) {say("Lamp: unknown command (type lamp for help)");return true;}
    {std::lock_guard<std::mutex> g(level_lamp_mutex);if(level_lamp_queue.size()<64) level_lamp_queue.emplace_back(arg);}
    // Placement commands close the console so the result is visible.
    close=!std::strncmp(arg,"add",3) || !std::strncmp(arg,"here",4) || !std::strncmp(arg,"move",4) || !std::strncmp(arg,"pick",4);
    if(!close) say("Lamp: done (see the notice)");
    return true;
}
