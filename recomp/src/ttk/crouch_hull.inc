// D08K2: crouch-height movement collision (documentation/185-d08k2-crouch-hull.md).
// The original move probe 0x8007926c judges every step with Duke's standing
// height: clearance (floor - ceiling, 0x8007765c) must be 0x370 and its head
// ray (0x800774cc) runs 0x370 above his feet. Crouched Duke is 591 units tall
// kneeling (pose 178, D08K1) and at most 654 in the crouch walk (D08K3), so the
// crouch probe asks the same questions of the same original queries with a
// 0x2c0 (704) hull, about the standing margin:
// clearance, the head ray at crouch height, a slope check (0x80077438), the
// object probe (0x800780b4), no step up over 0x100 and no drop over 0x100 (a
// crouched Duke stops at edges rather than falling). Stand-up keeps the existing
// crouch.inc rule: clearance 0x370 at Duke's position, never object-relative.
// The test mover below is debug-only (backtick console "crouchwalk"); D08K4
// turns the probe into the Modernized crouch walk.
static constexpr int32_t crouch_hull=0x2c0, crouch_step_up=0x100, crouch_step_down=0x100;
static unsigned hull_probes,hull_moves,hull_blocks[8];
// Debug only: crouchwalk's optional third number tests the probe at another hull height.
static std::atomic<int> hull_test_height{crouch_hull};
enum HullBlock {hull_clear=0,hull_low=1,hull_wall=2,hull_ray=3,hull_slope=4,hull_edge=5,hull_object=6,hull_failed=7};
// Guest scratch for the probe's points, below the update's stack pointer (free
// memory the called routine never reaches: original_call starts at top-0x100).
struct HullScratch {
    uint32_t base;uint32_t saved[8];
    explicit HullScratch(uint32_t sp):base(sp-0x40) {for(unsigned i=0;i<8;++i)saved[i]=psx_mod_read_word(base+4*i);}
    ~HullScratch() {for(unsigned i=0;i<8;++i)psx_mod_write_word(base+4*i,saved[i]);}
    uint32_t point(unsigned k,int32_t x,int32_t y,int32_t z) {
        const uint32_t a=base+16*k;psx_mod_write_word(a,uint32_t(x));psx_mod_write_word(a+4,uint32_t(y));psx_mod_write_word(a+8,uint32_t(z));return a;
    }
};
// original_call with an explicit first argument (0x80077438 takes the normal).
static bool hull_call(CPUState* source,uint32_t address,uint32_t a0,int32_t* result) {
    const uint32_t top=source->gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail)return false;
    uint32_t saved[1024],scratch[256];
    const uint32_t base=(top-4096)&~3u;
    for(unsigned i=0;i<1024;++i)saved[i]=psx_mod_read_word(base+4*i);
    for(unsigned i=0;i<256;++i)scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=0x800000fc;cpu.gpr[4]=a0;
    guest_busy=true;psx_dispatch_call(&cpu,address,0x800000fc);guest_busy=false;
    for(unsigned i=0;i<1024;++i)if(psx_mod_read_word(base+4*i)!=saved[i])psx_mod_write_word(base+4*i,saved[i]);
    for(unsigned i=0;i<256;++i)if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i])psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    if(result)*result=(int32_t)cpu.gpr[2];
    return cpu.pc==0 && !g_psx_call_bail;
}
// Player fields the original queries write: object contact 0x174/0x178, ray
// contact 0x184..0x1a3, floor normal 0x1a4..0x1ac, wall data 0x1b4..0x1c0, step 0x1c4.
static constexpr uint32_t hull_state_first=0x174, hull_state_words=(0x1c8-0x174)/4;
struct HullState {uint32_t w[hull_state_words];};
static HullState hull_save() {HullState s;for(unsigned i=0;i<hull_state_words;++i)s.w[i]=psx_mod_read_word(player+hull_state_first+4*i);return s;}
static void hull_restore(const HullState& s) {for(unsigned i=0;i<hull_state_words;++i)psx_mod_write_word(player+hull_state_first+4*i,s.w[i]);}
// Judge a step of (dx, dz) from Duke's position with the crouch hull. On
// success step_y is the floor change (positive = down) and the player fields
// describe the destination, as after an original accepted probe.
static HullBlock crouch_probe(CPUState* cpu,int32_t dx,int32_t dz,int32_t& step_y,int32_t hull=crouch_hull) {
    ++hull_probes;
    const int32_t x=int32_t(psx_mod_read_word(player+4)),y=int32_t(psx_mod_read_word(player+8)),z=int32_t(psx_mod_read_word(player+0xc));
    const int32_t feet=int32_t(psx_mod_read_word(player+0x1c8));
    const HullState before=hull_save();
    HullScratch scratch(cpu->gpr[29]);
    auto fail=[&](HullBlock b){hull_restore(before);++hull_blocks[b];return b;};
    // Objects first, as 0x8007926c does (it clears and sets the contact 0x174).
    const uint32_t delta=scratch.point(0,dx,0,dz);
    if(!original_call(cpu,0x800780b4,delta)) return fail(hull_failed);
    if(psx_mod_read_word(player+0x174)) return fail(hull_object);
    int32_t clearance=0;
    if(!original_call(cpu,0x8007765c,scratch.point(1,x+dx,y,z+dz),&clearance)) return fail(hull_failed);
    const int32_t step=int32_t(psx_mod_read_word(player+0x1c4));
    if(step<=-0x10000 || clearance==0xffff) return fail(hull_wall);    // no floor there
    if(clearance<hull) return fail(hull_low);
    if(step<-crouch_step_up) return fail(hull_wall);
    if(step>crouch_step_down) return fail(hull_edge);
    // Head ray at crouch height, from here to there (0x8007926c uses 0x370).
    int32_t hit=0;
    const uint32_t from=scratch.point(0,x,feet-hull,z),to=scratch.point(1,x+dx,feet+step-hull,z+dz);
    if(!original_call(cpu,0x800774cc,from,&hit,to)) return fail(hull_failed);
    if(hit) return fail(hull_ray);
    if(step<0) {
        int32_t steep=0;
        if(!hull_call(cpu,0x80077438,player+0x1a4,&steep)) return fail(hull_failed);
        if(steep&0xffff) return fail(hull_slope);
    }
    step_y=step;
    return hull_clear;
}
// Debug test mover: while Duke holds the stationary crouch (lower animation
// 178) on ordinary ground, move him (fwd, side) units per update relative to
// his facing, sliding along whichever axis is clear. Feet keep the crouch idle.
static int hull_test_fwd,hull_test_side;
static std::atomic<bool> hull_test_world{false};  // crouchgo: (dx, dz) in world units
static std::atomic<bool> hull_test_on{false};
static std::atomic<int> hull_scan_request{0},hull_scan_step{64};
static HullBlock hull_last=hull_clear;
static void crouch_test_move(CPUState* cpu) {
    if(!hull_test_on.load() || psx_mod_read_half(player+0x60)!=178 || !player_identity_ready() ||
       (psx_mod_read_word(player)&0x20002) || psx_mod_read_byte(player+0x22c)!=0 || psx_mod_read_byte(player+0x22d)!=0 ||
       in_water_early()) return;
    const double yaw=psx_mod_read_half(player+0x1c)*(6.283185307179586/4096.0);
    const double s=std::sin(yaw),c=std::cos(yaw);
    const bool world=hull_test_world.load();
    const int32_t dx=world?hull_test_fwd:int32_t(std::lround(hull_test_fwd*s+hull_test_side*c));
    const int32_t dz=world?hull_test_side:int32_t(std::lround(hull_test_fwd*c-hull_test_side*s));
    int32_t step=0;
    const int32_t hull=hull_test_height.load();
    HullBlock b=crouch_probe(cpu,dx,dz,step,hull);
    int32_t mx=dx,mz=dz;
    if(b!=hull_clear && dx && dz) {                       // slide: try each axis alone
        mx=dx;mz=0;b=crouch_probe(cpu,mx,mz,step,hull);
        if(b!=hull_clear) {mx=0;mz=dz;b=crouch_probe(cpu,mx,mz,step,hull);}
    }
    hull_last=b;
    if(b!=hull_clear) return;
    psx_mod_write_word(player+4,psx_mod_read_word(player+4)+uint32_t(mx));
    psx_mod_write_word(player+0xc,psx_mod_read_word(player+0xc)+uint32_t(mz));
    psx_mod_write_word(player+8,psx_mod_read_word(player+8)+uint32_t(step));
    psx_mod_write_word(player+0x1c8,psx_mod_read_word(player+0x1c8)+uint32_t(step));
    ++hull_moves;
}
// Debug clearance scan around Duke (Modernized, console "crouchscan [radius] [grid]"):
// grid points whose clearance fits a crouched Duke but not a standing one, on a
// floor within a step of his feet. Printed nearest first.
static void crouch_scan(CPUState* cpu,int radius,int grid) {
    const int32_t x=int32_t(psx_mod_read_word(player+4)),y=int32_t(psx_mod_read_word(player+8)),z=int32_t(psx_mod_read_word(player+0xc));
    const HullState before=hull_save();
    HullScratch scratch(cpu->gpr[29]);
    struct Hit {int32_t x,z,clear,step;int64_t d;};
    std::vector<Hit> hits,edges;
    unsigned bands[6]{},rays=0,stand_hits=0,ray_tests=0;const int32_t feet=int32_t(psx_mod_read_word(player+0x1c8));  // clearance histogram: <hull, hull..0x370, 0x370..0x500, 0x500..0x800, >=0x800, no floor
    const size_t cols=size_t(2*radius/grid+1);
    std::vector<int32_t> row_prev(cols,INT32_MIN),row_cur(cols,INT32_MIN);
    for(int32_t oz=-radius;oz<=radius;oz+=grid,row_prev.swap(row_cur),std::fill(row_cur.begin(),row_cur.end(),INT32_MIN))
        for(int32_t ox=-radius;ox<=radius;ox+=grid) {
            int32_t clearance=0;
            if(!original_call(cpu,0x8007765c,scratch.point(1,x+ox,y,z+oz),&clearance)) continue;
            const int32_t step=int32_t(psx_mod_read_word(player+0x1c4));
            const bool has_floor=!(step<=-0x10000 || clearance==0xffff);
            const int ix=(ox+radius)/grid;
            if(has_floor && ix>0 && row_prev.size()>size_t(ix) && row_cur.size()>size_t(ix-1)) {
                for(int32_t other:{row_cur[ix-1],row_prev[ix]})
                    if(other!=INT32_MIN && step-other>crouch_step_down && step-other<0x1000 && edges.size()<4096)
                        edges.push_back({x+ox,z+oz,clearance,step-other,int64_t(ox)*ox+int64_t(oz)*oz});
            }
            row_cur[ix]=has_floor?step:INT32_MIN;
            if(step<=-0x10000 || clearance==0xffff) ++bands[5];
            else if(step>=-crouch_step_up && step<=crouch_step_down)
                ++bands[clearance<crouch_hull?0:clearance<0x370?1:clearance<0x500?2:clearance<0x800?3:4];
            if(clearance>=crouch_hull && clearance<0x370 && step>=-crouch_step_up && step<=crouch_step_down)
                hits.push_back({x+ox,z+oz,clearance,step,int64_t(ox)*ox+int64_t(oz)*oz});
            // Overhangs: a step that the standing head ray (0x370) hits but the
            // crouch head ray (crouch_hull) clears, on a floor within a step.
            else if(clearance>=crouch_hull && clearance!=0xffff && step>=-crouch_step_up && step<=crouch_step_down) {
                const int32_t floor=feet+step;
                for(int dir=0;dir<2;++dir) {
                    const int32_t ex=dir?0:grid/2,ez=dir?grid/2:0;
                    int32_t stand=0,low=0;
                    if(!original_call(cpu,0x800774cc,scratch.point(0,x+ox,floor-0x370,z+oz),&stand,scratch.point(1,x+ox+ex,floor-0x370,z+oz+ez)))continue;
                    ++ray_tests;if(!stand)continue;
                    ++stand_hits;
                    if(!original_call(cpu,0x800774cc,scratch.point(0,x+ox,floor-crouch_hull,z+oz),&low,scratch.point(1,x+ox+ex,floor-crouch_hull,z+oz+ez)))continue;
                    int32_t knee=0;
                    original_call(cpu,0x800774cc,scratch.point(0,x+ox,floor-0x100,z+oz),&knee,scratch.point(1,x+ox+ex,floor-0x100,z+oz+ez));
                    if(!low && !knee) {hits.push_back({x+ox,z+oz,-1-dir,step,int64_t(ox)*ox+int64_t(oz)*oz});++rays;}
                }
            }
        }
    hull_restore(before);
    std::sort(hits.begin(),hits.end(),[](const Hit& a,const Hit& b){return a.d<b.d;});
    std::fprintf(stderr,"[TTK crouch] scan r=%d at %d,%d,%d: %zu crouch-only points (%u overhang steps; standing ray hits %u of %u); same-floor clearance <hull %u, crouch-only %u, <0x500 %u, <0x800 %u, more %u; no floor %u\n",
                 radius,x,y,z,hits.size(),rays,stand_hits,ray_tests,bands[0],bands[1],bands[2],bands[3],bands[4],bands[5]);
    std::sort(edges.begin(),edges.end(),[](const Hit& a,const Hit& b){return a.d<b.d;});
    for(size_t i=0;i<edges.size() && i<6;++i)
        std::fprintf(stderr,"[TTK crouch]   edge %d,%d (a %d drop beside it)\n",edges[i].x,edges[i].z,edges[i].step);
    for(size_t i=0;i<hits.size() && i<24;++i)
        std::fprintf(stderr,"[TTK crouch]   %d,%d %s %d step %d\n",hits[i].x,hits[i].z,hits[i].clear<0?"overhang dir":"clearance",
                     hits[i].clear<0?-1-hits[i].clear:hits[i].clear,hits[i].step);
}
static void crouch_hull_update(CPUState* cpu) {
    if(const int r=hull_scan_request.exchange(0)) crouch_scan(cpu,r,hull_scan_step.load());
    crouch_test_move(cpu);
}
// Backtick console (event-pump thread): queue only; the update does the work.
bool crouch_console_command(const char* line,void (*say)(const char*),bool& close) {
    close=false;
    const bool go=!std::strncmp(line,"crouchgo",8);
    if(std::strncmp(line,"crouchwalk",10) && std::strncmp(line,"crouchscan",10) && !go)return false;
    if(go) line+=8; else if(!std::strncmp(line,"crouchwalk",10)) line+=10;
    if(!input_modernized()) {say("Crouch tests: Modernized profile only");return true;}
    char text[160];
    if(!go && !std::strncmp(line,"crouchscan",10)) {
        int r=1536,g=64;std::sscanf(line+10,"%d %d",&r,&g);if(r<64)r=64;if(r>8192)r=8192;if(g<16)g=16;if(g>512)g=512;
        hull_scan_step.store(g);
        close=true;hull_scan_request.store(r);std::snprintf(text,sizeof text,"Crouch scan queued (radius %d); results on stderr",r);say(text);return true;
    }
    int fwd=0,side=0,height=crouch_hull;
    if(std::strstr(line,"off") || std::sscanf(line,"%d %d %d",&fwd,&side,&height)<1) {hull_test_on.store(false);hull_test_fwd=hull_test_side=0;}
    else {hull_test_world.store(go);hull_test_fwd=fwd;hull_test_side=side;hull_test_height.store(height<0x100?0x100:height>0x800?0x800:height);hull_test_on.store(true);}
    std::snprintf(text,sizeof text,"Crouch walk test %s (fwd %d side %d hull %d); probes %u moves %u blocked low %u wall %u ray %u slope %u edge %u object %u",
                  hull_test_on.load()?"on":"off",hull_test_fwd,hull_test_side,hull_test_height.load(),hull_probes,hull_moves,hull_blocks[hull_low],hull_blocks[hull_wall],
                  hull_blocks[hull_ray],hull_blocks[hull_slope],hull_blocks[hull_edge],hull_blocks[hull_object]);
    say(text);close=hull_test_on.load();return true;
}
