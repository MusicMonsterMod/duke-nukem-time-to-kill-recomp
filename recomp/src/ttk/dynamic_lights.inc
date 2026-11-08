// D41A: dynamic lights (documentation/413-d41-lighting-case-study.md).
//
// The original game paints its light into vertex colours (Gouraud) and never
// computes any: there are no light entities, its GTE lighting routines are
// never called, and a blast lights nothing. Polymorph emits every world, prop
// and actor polygon itself with view-space corners (near_clip.cpp); with this
// job each lit corner also carries its world position (relative to the eye),
// and the OpenGL renderer adds the frame's lights per pixel to the original
// vertex colour before the PS1 texture multiply: the baked light stays, real
// light lands on top of it, falls off with distance and faces the surfaces it
// reaches (the face normal comes from screen-space derivatives).
//
// Light sources here (game ticks, main process):
// - blasts: the blast sound 0x8006b73c (position in a1) from the projectile,
//   pipe bomb, dynamite and gib blasts (note 370 D39F map), and the barrel
//   0x80097090 (shake request, a1 position): a warm flash that swells over a
//   tic and fades over half a second.
// - D41B shots: every round's launch (0x800718ac, Duke's and the enemies')
//   lights a muzzle flash at its launch point in its type's colour, rounds in
//   flight carry light, and a round that meets a wall (0x8006f1b8) lights a
//   short spark; blasts are sized per source. Weapons without rounds (energy
//   beam, flamethrower) light Duke's hand flash (player word 0 bit 3).
// - console "light": a test light at Duke ("light", "light <r> <g> <b>
//   [radius]", "light off").
// The per-image list (LightView) travels to redraw workers in RenderState and
// is sent with each image (psx_mod_gpu_set_lights) where the host walk sets
// the frame's camera (host_visibility.inc). Modernized Polymorph only:
// Vanilla and the Classic renderer draw nothing different. Developer
// DNTTK_LIGHTS=off disables; DNTTK_LIGHT_AMBIENT=<percent> scales the
// original colours of lit surfaces (100 default).
struct LightSrc {double p[3];float rgb[3];double radius;int age,life,kind;};
enum {light_kind_blast=1,light_kind_muzzle=2,light_kind_test=3,light_kind_spark=4};
static LightSrc light_srcs[24];static unsigned light_src_count;
// D41H: smooth tells near_clip.cpp to give model corners smoothed normals
// (console "light smooth on|off", DNTTK_SMOOTH_LIGHT=0 developer default off).
struct LightView {uint32_t n=0;float amb=1;bool smooth=true;PSXModLight l[PSX_MOD_MAX_LIGHTS];};
static LightView light_view;                 // this image's lights (RenderState)
static std::atomic<int> light_console{0};    // 1 add test light, 2 clear
static std::atomic<int> light_smooth{-1};     // D41H session override (-1 default)
static bool light_smooth_on() {
    const int o=light_smooth.load(std::memory_order_relaxed);
    if(o>=0) return o!=0;
    static const bool v=[]{const char* t=std::getenv("DNTTK_SMOOTH_LIGHT");return !(t && t[0]=='0');}();return v;
}
static float light_console_rgb[3]={1.0f,0.85f,0.6f};static double light_console_radius=4000;
static uint64_t light_blasts,light_muzzles,light_uploads,light_enemy_muzzles,light_sparks;
static uint32_t light_ticks,light_duke_launch_tick=~0u;  // D41B: game tics; Duke's last launch
static bool light_duke_flash_pending;
static uint32_t light_loads;
static bool lights_enabled() {
    const char* t=ttk_live_env("DNTTK_LIGHTS");
    return input_modernized() && !ttk_world_classic() && !(t && !std::strcmp(t,"off"));
}
static float light_ambient() {
    static const float v=[]{const char* t=std::getenv("DNTTK_LIGHT_AMBIENT");
        const long n=t?std::atol(t):100;return (n>=0 && n<=200 ? n : 100)/100.0f;}();
    return v;
}
static void light_add(const double p[3],float r,float g,float b,double radius,int life,int kind) {
    unsigned at=light_src_count;
    if(at>=std::size(light_srcs)) {
        // Replace the oldest blast or muzzle light (never a test light).
        at=0;int oldest=-1;
        for(unsigned i=0;i<light_src_count;++i)
            if(light_srcs[i].kind!=light_kind_test && light_srcs[i].age>oldest) {oldest=light_srcs[i].age;at=i;}
    } else ++light_src_count;
    LightSrc& s=light_srcs[at];
    for(int i=0;i<3;++i) s.p[i]=p[i];
    s.rgb[0]=r;s.rgb[1]=g;s.rgb[2]=b;s.radius=radius;s.age=0;s.life=life;s.kind=kind;
}
// Brightness over a light's life: blasts swell for a tic then fall away,
// the muzzle flash is on for its tic and gone, a test light stays.
static double light_envelope(const LightSrc& s) {
    if(s.kind==light_kind_test || s.life<=0) return 1;
    const double t=(double)s.age/s.life;
    if(t>=1) return 0;
    if(s.kind==light_kind_muzzle || s.kind==light_kind_spark) return s.age==0?1:0.45*(1-t);
    if(s.age==0) return 0.8;
    const double f=1-t;return f*f;
}
// D41B: shot lights by projectile type (note 413 sections 7 and 10). The
// launcher 0x800718ac (a0 shooter, a1 origin, a3 type) starts every round,
// Duke's and the enemies', at its real muzzle point; 0x8006f1b8 (a0 the
// round, a1 the point) is where a round meets a wall or an actor. Rounds in
// flight: the projectile pool 0x800da978 holds 100 records of 0x60 bytes (+0
// flags, 3 while a round flies; +4 position; +0x30 type), read each tick,
// never stored. Values are TTK units (Duke's shoulder is about 620 up) and
// game tics; Quake's muzzle flash (radius 200 for 0.1 s) and EDuke32
// Polymer's orange muzzle light are the reference for the warm guns.
struct ShotLight {
    uint8_t type;
    float mz[3];double mr;int ml;   // muzzle flash at the launch point (life 0: none)
    float im[3];double ir;int il;   // impact where the round stops (life 0: none)
    float fl[3];double fr;          // in flight (radius 0: none)
};
static constexpr ShotLight shot_lights[]={
    {0,{0.85f,0.58f,0.28f},3400,3,{1.00f,0.78f,0.42f},1500,2,{0,0,0},0},          // Desert Eagle / pistol rounds
    {1,{0.95f,0.62f,0.30f},4400,3,{1.00f,0.78f,0.42f},1100,2,{0,0,0},0},          // shotgun pellets
    {2,{0.85f,0.58f,0.28f},2800,3,{1.00f,0.78f,0.42f},1200,2,{0,0,0},0},          // enemy rounds (pig cops; level 0 probe)
    {6,{1.00f,0.64f,0.30f},4600,3,{1.00f,0.78f,0.42f},1600,2,{0,0,0},0},          // Buffalo Rifle
    {7,{0.85f,0.60f,0.30f},3000,2,{1.00f,0.78f,0.42f},1300,2,{0,0,0},0},          // Gatling rounds
    {8,{1.00f,0.62f,0.30f},4800,4,{0,0,0},0,0,{0.95f,0.55f,0.22f},2800},          // RPG rocket (its blast lights the hit)
    {13,{0.55f,0.50f,1.25f},3000,3,{0.60f,0.60f,1.30f},1900,4,{0.50f,0.45f,1.15f},1800}, // freezer: violet-blue, as its shots
    {15,{0.45f,1.15f,0.40f},3200,3,{0.50f,1.25f,0.45f},2300,4,{0.40f,1.05f,0.35f},2200}, // energy round: green, as the weapon's own glow
    // Throws and bolts (D41B probe): no flash at the hand and no spark; the
    // explosives light through the blast sources. Pipe bomb 4, Holy Hand
    // Grenade 11, dynamite 12, throwing knife 16, throwing axe 17, crossbow
    // bolt 18.
    {4,{0,0,0},0,0,{0,0,0},0,0,{0,0,0},0},
    {11,{0,0,0},0,0,{0,0,0},0,0,{0,0,0},0},
    {12,{0,0,0},0,0,{0,0,0},0,0,{0,0,0},0},
    {16,{0,0,0},0,0,{0,0,0},0,0,{0,0,0},0},
    {17,{0,0,0},0,0,{0,0,0},0,0,{0,0,0},0},
    {18,{0,0,0},0,0,{0,0,0},0,0,{0,0,0},0},
};
// Other types (enemy guns not yet probed: turrets, flyers, bosses) take a
// warm flash and spark.
static constexpr ShotLight shot_light_default={255,{0.80f,0.56f,0.28f},3000,3,{1.00f,0.78f,0.42f},1200,2,{0,0,0},0};
static const ShotLight& shot_light(unsigned type) {
    for(const ShotLight& s:shot_lights) if(s.type==type) return s;
    return shot_light_default;
}
static unsigned lights_flight(PSXModLight* out,unsigned room) {
    unsigned n=0;
    for(uint32_t i=0;i<100 && n<room;++i) {
        const uint32_t a=0x800da978u+i*0x60u;
        if(psx_mod_read_half(a)!=3) continue;
        const ShotLight& f=shot_light(psx_mod_read_byte(a+0x30));
        if(f.fr<=0) continue;
        PSXModLight& l=out[n++];
        l.x=(float)(int32_t)psx_mod_read_word(a+4);l.y=(float)(int32_t)psx_mod_read_word(a+8);l.z=(float)(int32_t)psx_mod_read_word(a+12);
        l.radius=(float)f.fr;l.r=f.fl[0];l.g=f.fl[1];l.b=f.fl[2];l.flags=0;
    }
    return n;
}
static void light_spark(const double p[3],const ShotLight& s,unsigned type,uint32_t owner);
static void lights_publish() {
    LightView v;v.amb=light_ambient();v.smooth=light_smooth_on();
    v.n=lights_flight(v.l,8);
    for(unsigned i=0;i<light_src_count && v.n<PSX_MOD_MAX_LIGHTS;++i) {
        const LightSrc& s=light_srcs[i];
        const double e=light_envelope(s);
        if(e<=0.01) continue;
        PSXModLight& l=v.l[v.n++];
        l.x=(float)s.p[0];l.y=(float)s.p[1];l.z=(float)s.p[2];
        l.radius=(float)(s.radius*(s.kind==light_kind_blast?0.75+0.25*e:1.0));
        l.r=(float)(s.rgb[0]*e);l.g=(float)(s.rgb[1]*e);l.b=(float)(s.rgb[2]*e);
        l.flags=0;
    }
    light_view=v;
}
// Once per Duke update (game tick).
static void lights_tick() {
    if(!lights_enabled() || !gameplay_context()) {
        light_src_count=0;light_view=LightView{};return;
    }
    // A loaded savestate starts with no lights in flight.
    if(light_loads!=psx_mod_savestate_loads()) {
        light_loads=psx_mod_savestate_loads();
        unsigned k=0;for(unsigned i=0;i<light_src_count;++i) if(light_srcs[i].kind==light_kind_test) light_srcs[k++]=light_srcs[i];
        light_src_count=k;
    }
    unsigned k=0;
    for(unsigned i=0;i<light_src_count;++i) {
        LightSrc& s=light_srcs[i];
        if(s.kind!=light_kind_test && ++s.age>=s.life) continue;
        light_srcs[k++]=s;
    }
    light_src_count=k;
    const double duke[3]={(double)(int32_t)psx_mod_read_word(player+4),(double)(int32_t)psx_mod_read_word(player+8),(double)(int32_t)psx_mod_read_word(player+12)};
    switch(light_console.exchange(0)) {
    case 1: {
        const double p[3]={duke[0],duke[1]-900,duke[2]};
        light_add(p,light_console_rgb[0],light_console_rgb[1],light_console_rgb[2],light_console_radius,0,light_kind_test);
        break;
    }
    case 2: {
        unsigned j=0;for(unsigned i=0;i<light_src_count;++i) if(light_srcs[i].kind!=light_kind_test) light_srcs[j++]=light_srcs[i];
        light_src_count=j;break;
    }
    }
    ++light_ticks;
    // Duke's muzzle flash without a round (word 0 bit 3, the flash his hand
    // draws): weapons that launch light their muzzle in lights_launch, so a
    // flash is only lit here when no launch of Duke's came within a tic of it.
    if(light_duke_flash_pending && light_ticks-light_duke_launch_tick>2) {
        double f[3];for(int i=0;i<3;++i) f[i]=(int16_t)psx_mod_read_half(camera+0x20+2*(6+i))/4096.0;
        const double fl=std::sqrt(f[0]*f[0]+f[2]*f[2]);
        double p[3]={duke[0]+(fl>1e-3?f[0]/fl*260:0),duke[1]-620,duke[2]+(fl>1e-3?f[2]/fl*260:0)};
        // The gun's muzzle point (player +0xbc, where rounds launch) when near him.
        const double m[3]={(double)(int32_t)psx_mod_read_word(player+0xbc),(double)(int32_t)psx_mod_read_word(player+0xc0),(double)(int32_t)psx_mod_read_word(player+0xc4)};
        const double dm=std::sqrt((m[0]-duke[0])*(m[0]-duke[0])+(m[1]-duke[1])*(m[1]-duke[1])+(m[2]-duke[2])*(m[2]-duke[2]));
        if(dm>50 && dm<1500) for(int i=0;i<3;++i) p[i]=m[i];
        // Weapons without rounds (D41B probe): the energy weapon's beam is
        // green like its own glow; the flamethrower's jet is a flickering fire light carried
        // ahead of the nozzle, toward the middle of the flame.
        const unsigned w=base_weapon(psx_mod_read_byte(player+0x3b9));
        if(w==10) light_add(p,0.45f,1.15f,0.40f,3200,3,light_kind_muzzle);
        else if(w==9) {
            const float k=0.85f+0.3f*(float)((light_ticks*2654435761u>>24)&255)/255.0f;
            if(fl>1e-3) {p[0]+=f[0]/fl*700;p[2]+=f[2]/fl*700;}
            light_add(p,1.15f*k,0.62f*k,0.24f*k,5200,2,light_kind_muzzle);
        } else light_add(p,0.80f,0.56f,0.28f,3400,3,light_kind_muzzle);
        ++light_muzzles;
        if(std::getenv("DNTTK_LIGHT_TRACE")) std::fprintf(stderr,"[TTK light] flash (no round) weapon %u at %.0f %.0f %.0f\n",psx_mod_read_byte(player+0x3b9),p[0],p[1],p[2]);
    }
    light_duke_flash_pending=(psx_mod_read_word(player)&8u) && light_ticks-light_duke_launch_tick>1;
    lights_publish();
}
// D41B: blast sizes per source (radius, life in tics, brightness): the TNT
// barrel and dynamite are the biggest bangs, a rocket or grenade the D41A
// first value, a gib burst a short, smaller flash.
struct BlastLight {double radius;int life;float k;};
static constexpr BlastLight blast_barrel{9000,20,1.10f},blast_dynamite{8500,18,1.05f},blast_pipe{7600,16,1.0f},blast_round{7000,16,1.0f},blast_gib{3800,8,0.65f};
static void light_blast_at(uint32_t pos,const char* why,const BlastLight& b=blast_round) {
    if(!lights_enabled() || !gameplay_context() || pos<0x80000000u || pos>0x801ffff0u) return;
    const double p[3]={(double)(int32_t)psx_mod_read_word(pos),(double)(int32_t)psx_mod_read_word(pos+4)-400,(double)(int32_t)psx_mod_read_word(pos+8)};
    light_add(p,1.30f*b.k,0.78f*b.k,0.36f*b.k,b.radius,b.life,light_kind_blast);++light_blasts;
    lights_publish();
    if(std::getenv("DNTTK_LIGHT_TRACE")) std::fprintf(stderr,"[TTK light] blast %s at %.0f %.0f %.0f\n",why,p[0],p[1],p[2]);
}
// D41B: every round's launch (0x800718ac entry: a0 shooter, a1 origin, a3
// type). One muzzle light per shooter and tic (a shotgun's pellets share it).
static void lights_launch(CPUState* cpu) {
    if(!lights_enabled() || !gameplay_context()) return;
    const uint32_t shooter=cpu->gpr[4],origin=cpu->gpr[5],type=cpu->gpr[7];
    if(origin<0x80000000u || origin>0x801ffff0u) return;
    const bool duke=shooter==player;
    if(duke) {if(light_duke_launch_tick==light_ticks) return;light_duke_launch_tick=light_ticks;light_duke_flash_pending=false;}
    else {
        static uint32_t recent[8],recent_tick[8];static unsigned next;
        for(unsigned i=0;i<8;++i) if(recent[i]==shooter && recent_tick[i]==light_ticks) return;
        recent[next]=shooter;recent_tick[next]=light_ticks;next=(next+1)&7;
    }
    const ShotLight& s=shot_light(type);
    if(s.ml<=0) return;
    const double p[3]={(double)(int32_t)psx_mod_read_word(origin),(double)(int32_t)psx_mod_read_word(origin+4),(double)(int32_t)psx_mod_read_word(origin+8)};
    // Enemies' flashes are a little smaller than Duke's own.
    const double k=duke?1.0:0.8;
    light_add(p,s.mz[0],s.mz[1],s.mz[2],s.mr*k,s.ml,light_kind_muzzle);
    if(duke) ++light_muzzles;else ++light_enemy_muzzles;
    lights_publish();
    if(std::getenv("DNTTK_LIGHT_TRACE")) std::fprintf(stderr,"[TTK light] launch %s %08x type %u weapon %u at %.0f %.0f %.0f\n",duke?"duke":"enemy",shooter,type,duke?psx_mod_read_byte(player+0x3b9):255u,p[0],p[1],p[2]);
}
// D41B: a round meets a wall (0x8006f1b8 entry: a0 the round, a1 the point;
// rounds that hit a character do not call it). Sparks closer than 600 units
// to one already lit this tic are one spark (shotgun pellets).
static void lights_impact(CPUState* cpu) {
    if(!lights_enabled() || !gameplay_context()) return;
    const uint32_t a=cpu->gpr[4],at=cpu->gpr[5];
    if(a<0x800da978u || a>=0x800dcd78u || (a-0x800da978u)%0x60u || at<0x80000000u || at>0x801ffff0u) return;
    const unsigned type=psx_mod_read_byte(a+0x30);
    const ShotLight& s=shot_light(type);
    if(s.il<=0) return;
    const double p[3]={(double)(int32_t)psx_mod_read_word(at),(double)(int32_t)psx_mod_read_word(at+4),(double)(int32_t)psx_mod_read_word(at+8)};
    light_spark(p,s,type,psx_mod_read_word(a+0x10));
}
static void light_spark(const double p[3],const ShotLight& s,unsigned type,uint32_t owner) {
    for(unsigned i=0;i<light_src_count;++i) {
        const LightSrc& o=light_srcs[i];
        if(o.kind!=light_kind_spark || o.age) continue;
        const double dx=o.p[0]-p[0],dy=o.p[1]-p[1],dz=o.p[2]-p[2];
        if(dx*dx+dy*dy+dz*dz<600.0*600.0) return;
    }
    light_add(p,s.im[0],s.im[1],s.im[2],s.ir,s.il,light_kind_spark);++light_sparks;
    lights_publish();
    if(std::getenv("DNTTK_LIGHT_TRACE")) std::fprintf(stderr,"[TTK light] impact type %u owner %08x at %.0f %.0f %.0f\n",type,owner,p[0],p[1],p[2]);
}
// Entry of the sound routine 0x8006b73c: the blast calls (return addresses
// from the D39F map) carry the position in a1.
static void lights_sound(CPUState* cpu) {
    switch(cpu->gpr[31]) {
    case 0x80070418u: light_blast_at(cpu->gpr[5],"projectile");break;
    case 0x800708e4u: light_blast_at(cpu->gpr[5],"pipe bomb or grenade",blast_pipe);break;
    case 0x80071044u: light_blast_at(cpu->gpr[5],"dynamite",blast_dynamite);break;
    case 0x8006413cu: light_blast_at(cpu->gpr[5],"gib",blast_gib);break;
    }
}
// Under water the same blasts use 0x8006b270.
static void lights_sound_water(CPUState* cpu) {
    switch(cpu->gpr[31]) {
    case 0x80070858u: light_blast_at(cpu->gpr[5],"pipe bomb or grenade (water)",blast_pipe);break;
    case 0x80070ff8u: light_blast_at(cpu->gpr[5],"dynamite (water)",blast_dynamite);break;
    }
}
// The barrel blast has no sound call of its own (D39F): its shake request.
static void lights_shake(CPUState* cpu) {
    if(cpu->gpr[31]==0x800973e0u) light_blast_at(cpu->gpr[5],"barrel",blast_barrel);
}
// Host walk (draw time, live process and redraw workers): this image's
// lights relative to the eye that the corners are given in.
static void lights_upload(bool valid,const double m[9],const double eye[3]) {
    // D41H: normals only matter where the shader lights (a light or a dimmed
    // ambient); otherwise lit triangles keep their original colour.
    near_clip_light_frame(valid && lights_enabled(),m,light_view.smooth && (light_view.n>0 || light_view.amb!=1.0f));
    if(!valid || !lights_enabled()) {psx_mod_gpu_set_lights(nullptr,0,1.0f);return;}
    PSXModLight l[PSX_MOD_MAX_LIGHTS];
    const unsigned n=std::min<unsigned>(light_view.n,PSX_MOD_MAX_LIGHTS);
    for(unsigned i=0;i<n;++i) {
        l[i]=light_view.l[i];
        l[i].x=(float)(light_view.l[i].x-eye[0]);l[i].y=(float)(light_view.l[i].y-eye[1]);l[i].z=(float)(light_view.l[i].z-eye[2]);
    }
    psx_mod_gpu_set_lights(l,(int)n,light_view.amb);++light_uploads;
}
// Backtick console: "light" (a test light at Duke), "light <r> <g> <b>
// [radius]" (colour 0..4 each, radius 500..30000), "light off" (test lights
// cleared).
bool lights_console_command(const char* line,void (*say)(const char*),bool& close) {
    close=false;
    if(std::strncmp(line,"light",5) || (line[5] && line[5]!=' '))return false;
    if(!input_modernized()) {say("Light: Modernized profile only (Vanilla is unchanged)");return true;}
    if(ttk_world_classic()) {say("Light: dynamic lights need the Polymorph renderer");return true;}
    const char* arg=line+5;while(*arg==' ')++arg;
    if(!std::strcmp(arg,"off")) {light_console.store(2);say("Light: test lights removed");return true;}
    // D41H: "light smooth [on|off|default]" - smooth or faceted light on
    // models for this session (matched comparisons).
    if(!std::strncmp(arg,"smooth",6) && (!arg[6] || arg[6]==' ')) {
        const char* v=arg+6;while(*v==' ')++v;
        if(!std::strcmp(v,"on")) light_smooth.store(1);
        else if(!std::strcmp(v,"off")) light_smooth.store(0);
        else if(!std::strcmp(v,"default")) light_smooth.store(-1);
        else if(*v) {say("Light: light smooth [on|off|default]");return true;}
        say(light_smooth_on()?"Light: smooth light on models":"Light: faceted light on models (session only)");
        return true;
    }
    if(*arg) {
        float r,g,b;double radius=light_console_radius;
        const int got=std::sscanf(arg,"%f %f %f %lf",&r,&g,&b,&radius);
        if(got<3 || r<0 || g<0 || b<0 || r>4 || g>4 || b>4 || radius<500 || radius>30000) {say("Light: light [<r> <g> <b> [radius]] or light off (colour 0-4, radius 500-30000)");return true;}
        light_console_rgb[0]=r;light_console_rgb[1]=g;light_console_rgb[2]=b;light_console_radius=radius;
    }
    light_console.store(1);
    char reply[96];std::snprintf(reply,sizeof reply,"Light: %.2f %.2f %.2f radius %.0f at Duke",light_console_rgb[0],light_console_rgb[1],light_console_rgb[2],light_console_radius);
    say(reply);close=true;
    return true;
}
