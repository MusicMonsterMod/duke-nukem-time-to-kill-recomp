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
// - Duke's muzzle flash: player word 0 bit 3 (the flash drawn at his hand,
//   note 63) lights a short warm pulse in front of him.
// - console "light": a test light at Duke ("light", "light <r> <g> <b>
//   [radius]", "light off").
// The per-image list (LightView) travels to redraw workers in RenderState and
// is sent with each image (psx_mod_gpu_set_lights) where the host walk sets
// the frame's camera (host_visibility.inc). Modernized Polymorph only:
// Vanilla and the Classic renderer draw nothing different. Developer
// DNTTK_LIGHTS=off disables; DNTTK_LIGHT_AMBIENT=<percent> scales the
// original colours of lit surfaces (100 default).
struct LightSrc {double p[3];float rgb[3];double radius;int age,life,kind;};
enum {light_kind_blast=1,light_kind_muzzle=2,light_kind_test=3};
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
static uint64_t light_blasts,light_muzzles,light_uploads;
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
    if(s.kind==light_kind_muzzle) return s.age==0?1:0.45*(1-t);
    if(s.age==0) return 0.8;
    const double f=1-t;return f*f;
}
// D41B (first part): rounds in flight carry light. The projectile pool
// 0x800da978 holds 100 records of 0x60 bytes (+0 flags, 3 while a round
// flies; +4 position; +0x30 type: 8 RPG rocket, 15 energy beam round; note
// 413 D41B probe). Read each tick, never stored.
struct FlightLight {uint8_t type;float rgb[3];double radius;};
static constexpr FlightLight flight_lights[]={
    {8,{0.95f,0.55f,0.22f},2800},   // RPG rocket: Quake's rocket light, warm
    {15,{0.25f,0.75f,1.10f},2200},  // energy beam round: cyan
};
static unsigned lights_flight(PSXModLight* out,unsigned room) {
    unsigned n=0;
    for(uint32_t i=0;i<100 && n<room;++i) {
        const uint32_t a=0x800da978u+i*0x60u;
        if(psx_mod_read_half(a)!=3) continue;
        const uint8_t type=psx_mod_read_byte(a+0x30);
        for(const FlightLight& f:flight_lights) if(f.type==type) {
            PSXModLight& l=out[n++];
            l.x=(float)(int32_t)psx_mod_read_word(a+4);l.y=(float)(int32_t)psx_mod_read_word(a+8);l.z=(float)(int32_t)psx_mod_read_word(a+12);
            l.radius=(float)f.radius;l.r=f.rgb[0];l.g=f.rgb[1];l.b=f.rgb[2];l.flags=0;
            break;
        }
    }
    return n;
}
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
    // Duke's muzzle flash: the frame his hand draws the flash (word 0 bit 3).
    if(psx_mod_read_word(player)&8u) {
        // Ahead of Duke along the camera's forward row, at shoulder height.
        double f[3];for(int i=0;i<3;++i) f[i]=(int16_t)psx_mod_read_half(camera+0x20+2*(6+i))/4096.0;
        const double fl=std::sqrt(f[0]*f[0]+f[2]*f[2]);
        double p[3]={duke[0]+(fl>1e-3?f[0]/fl*260:0),duke[1]-620,duke[2]+(fl>1e-3?f[2]/fl*260:0)};
        // D41B: the gun's muzzle point (player +0xbc, where rounds launch;
        // about 400 units from Duke's feet) when it is near him.
        const double m[3]={(double)(int32_t)psx_mod_read_word(player+0xbc),(double)(int32_t)psx_mod_read_word(player+0xc0),(double)(int32_t)psx_mod_read_word(player+0xc4)};
        const double dm=std::sqrt((m[0]-duke[0])*(m[0]-duke[0])+(m[1]-duke[1])*(m[1]-duke[1])+(m[2]-duke[2])*(m[2]-duke[2]));
        if(dm>50 && dm<1500) for(int i=0;i<3;++i) p[i]=m[i];
        light_add(p,0.80f,0.56f,0.28f,3400,3,light_kind_muzzle);++light_muzzles;
        if(std::getenv("DNTTK_LIGHT_TRACE")) std::fprintf(stderr,"[TTK light] muzzle at %.0f %.0f %.0f\n",p[0],p[1],p[2]);
    }
    lights_publish();
}
static void light_blast_at(uint32_t pos,const char* why) {
    if(!lights_enabled() || !gameplay_context() || pos<0x80000000u || pos>0x801ffff0u) return;
    const double p[3]={(double)(int32_t)psx_mod_read_word(pos),(double)(int32_t)psx_mod_read_word(pos+4)-400,(double)(int32_t)psx_mod_read_word(pos+8)};
    light_add(p,1.30f,0.78f,0.36f,7000,16,light_kind_blast);++light_blasts;
    lights_publish();
    if(std::getenv("DNTTK_LIGHT_TRACE")) std::fprintf(stderr,"[TTK light] blast %s at %.0f %.0f %.0f\n",why,p[0],p[1],p[2]);
}
// Entry of the sound routine 0x8006b73c: the blast calls (return addresses
// from the D39F map) carry the position in a1.
static void lights_sound(CPUState* cpu) {
    switch(cpu->gpr[31]) {
    case 0x80070418u: light_blast_at(cpu->gpr[5],"projectile");break;
    case 0x800708e4u: light_blast_at(cpu->gpr[5],"pipe bomb");break;
    case 0x80071044u: light_blast_at(cpu->gpr[5],"dynamite");break;
    case 0x8006413cu: light_blast_at(cpu->gpr[5],"gib");break;
    }
}
// Under water the same blasts use 0x8006b270.
static void lights_sound_water(CPUState* cpu) {
    switch(cpu->gpr[31]) {
    case 0x80070858u: light_blast_at(cpu->gpr[5],"pipe bomb (water)");break;
    case 0x80070ff8u: light_blast_at(cpu->gpr[5],"dynamite (water)");break;
    }
}
// The barrel blast has no sound call of its own (D39F): its shake request.
static void lights_shake(CPUState* cpu) {
    if(cpu->gpr[31]==0x800973e0u) light_blast_at(cpu->gpr[5],"barrel");
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
