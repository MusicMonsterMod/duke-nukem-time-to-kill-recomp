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
// - D41I fire: flames in flight from the flamethrowers and burning objects
//   (enemies, Duke; flag word bit 0x800) as flickering fire lights.
// - D41J upgrades: the HiTemp Flamethrower's flames and hand flash light
//   blue and the Laser Gatling's bolts red (D41J1).
// - D41D level lamps (level_lights.inc): placed lights per level.
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
// D41F: nsh shadowed lights (light_shadows.inc); their lights' flags name the slot.
struct LightView {uint32_t n=0;float amb=1;bool smooth=true;PSXModLight l[PSX_MOD_MAX_LIGHTS];uint32_t nsh=0;PSXModShadow sh[PSX_MOD_SHADOW_SLOTS];};
static LightView light_view;                 // this image's lights (RenderState)
static std::atomic<int> light_console{0};    // 1 add test light, 2 clear
static std::atomic<int> light_smooth{-1};     // D41H session override (-1 default)
static bool light_smooth_on() {
    const int o=light_smooth.load(std::memory_order_relaxed);
    if(o>=0) return o!=0;
    static const bool v=[]{const char* t=std::getenv("DNTTK_SMOOTH_LIGHT");return !(t && t[0]=='0');}();return v;
}
static float light_console_rgb[3]={1.0f,0.85f,0.6f};static double light_console_radius=4000;
static bool light_console_at;static double light_console_pos[3];   // D41K: "light at x y z ..."
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
// flight: the projectile pool 0x800da978 holds 96 records of 0x60 bytes (+0
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
    // D41J: the Laser Gatling (record 28) launches type 27, red laser bolts:
    // the Gatling's flash and spark sizes, and each bolt glows. D41J1: the
    // sampled red-pink (1.00 0.18 0.41) read pale pink-white in play. Its
    // fast fire stacks several flashes and bolt glows near the muzzle; red
    // tops out in the shader's soft shoulder while green and blue keep
    // climbing, and the blue-grey street adds blue. Near-pure red keeps the
    // stack red (a single light was already right), as the freezer is blue.
    {27,{1.25f,0.05f,0.02f},3000,2,{1.30f,0.07f,0.03f},1300,2,{1.05f,0.04f,0.02f},1500},
    // D41I enemy guns (lab census, note 413 section 11): the type each enemy
    // launches. Rounds without a coloured effect of their own take the warm
    // gun flash, sized by the gun; the gunship's bolts (31, the freezer's
    // model 0x799 in the type table) glow violet. Enemy flashes are scaled by
    // 0.8 at launch.
    {3,{0.85f,0.58f,0.28f},3400,3,{1.00f,0.78f,0.42f},1300,2,{0,0,0},0},          // type 26 enemy rounds (levels 0-11)
    {5,{0.85f,0.60f,0.30f},3200,2,{1.00f,0.78f,0.42f},1300,2,{0,0,0},0},          // gatling turrets 204 and 540
    {29,{0.85f,0.60f,0.30f},3200,2,{1.00f,0.78f,0.42f},1300,2,{0,0,0},0},         // robotic turret 622
    {31,{0.95f,0.55f,1.20f},3800,3,{1.00f,0.60f,1.25f},1600,3,{0,0,0},0},         // gunship 605's bolts (level 3): violet, as their own glow
    {32,{1.00f,0.62f,0.30f},4400,3,{1.00f,0.78f,0.42f},1600,2,{0,0,0},0},         // boss 61's guns (level 27)
    {34,{0.85f,0.58f,0.28f},3400,3,{1.00f,0.78f,0.42f},1300,2,{0,0,0},0},         // type 95 rounds (levels 7-11)
    {35,{1.00f,0.62f,0.30f},4400,2,{1.00f,0.78f,0.42f},1600,2,{0,0,0},0},         // Moloch's twin gatlings (level 29)
    // Rockets: the RPG's model (0x34b in the type table 0x800c4bc0): robotic
    // turret 623 (33), type 24, and the incendiary RPG (28, a hotter orange).
    {24,{1.00f,0.62f,0.30f},4800,4,{0,0,0},0,0,{0.95f,0.55f,0.22f},2800},
    {33,{1.00f,0.62f,0.30f},4800,4,{0,0,0},0,0,{0.95f,0.55f,0.22f},2800},
    {28,{1.05f,0.55f,0.22f},4800,4,{0,0,0},0,0,{1.05f,0.50f,0.18f},3200},
    // Flames (the flamethrower 14, the hi-temp flamethrower 26): no launch
    // flash (the hand flash lights the nozzle); each flame in flight is a fire
    // light (lights_fire).
    {14,{0,0,0},0,0,{0,0,0},0,0,{0,0,0},0},
    {26,{0,0,0},0,0,{0,0,0},0,0,{0,0,0},0},
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
// Other types (not seen launched in the D41I census: flyers 44 and 847/848/901, which did not fire there) take a warm flash and
// spark.
static constexpr ShotLight shot_light_default={255,{0.80f,0.56f,0.28f},3000,3,{1.00f,0.78f,0.42f},1200,2,{0,0,0},0};
static const ShotLight& shot_light(unsigned type) {
    for(const ShotLight& s:shot_lights) if(s.type==type) return s;
    return shot_light_default;
}
static unsigned lights_flight(PSXModLight* out,unsigned room) {
    unsigned n=0;
    for(uint32_t i=0;i<96 && n<room;++i) {
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
// D41I fire. Flames in flight: the flamethrowers' flame records in the pool
// (types 14 and 26, flag bit 0; drawn by 0x8006cfd8 as flame sprites), up to
// 8 at once; three of them, spread along the stream, each carry a flickering
// fire light. Burning objects: an object whose flag word (+0) has bit 0x800
// set (and 0x4 clear, object_burning) is drawn with flame sprites on its joints (actor draw 0x80033868,
// Duke's 0x80032304); enemies, props and Duke each light as a fire while it
// burns. Objects are the level's placement rows (*0x800de720, 48 bytes, +4
// the live object; spawner and object rows up to the count 0x800de724).
// Collected once per game tick into light_fire; the flicker changes per tick.
struct FireLight {float p[3];float k;double radius;bool blue;};
static FireLight light_fire[8];static unsigned light_fire_count;
static uint64_t light_burning_ticks,light_flame_ticks;
static float light_flicker(uint32_t seed) {
    return 0.80f+0.40f*(float)(((light_ticks*2654435761u)^(seed*40503u))>>24&255)/255.0f;
}
// Burning: flag word bit 0x800 while bit 0x4 is clear. Measured: a burning
// pig cop reads 0x803 and a burning tree 0x802; both turn 0x804 when the
// burn is over (the tree has burned away, the body is charred), and some
// props keep 0x804 from the level's start. The flame sprites stop then too.
static bool object_burning(uint32_t flags) {return (flags&0x804u)==0x800u;}
static std::atomic<int> light_fire_on{1};     // D41I session toggle (console "light fire on|off")
static void lights_fire_collect() {
    light_fire_count=0;
    if(!light_fire_on.load(std::memory_order_relaxed)) return;
    const double duke[3]={(double)(int32_t)psx_mod_read_word(player+4),(double)(int32_t)psx_mod_read_word(player+8),(double)(int32_t)psx_mod_read_word(player+12)};
    // Burning objects, nearest first (at most 5).
    struct B {uint32_t a;double d;} burn[16];unsigned nb=0;
    const uint32_t table=psx_mod_read_word(0x800de720),count=psx_mod_read_word(0x800de724);
    auto consider=[&](uint32_t a){
        if(a<0x80000000u || a>0x801fff00u || !object_burning(psx_mod_read_word(a))) return;
        for(unsigned j=0;j<nb;++j) if(burn[j].a==a) return;
        const double dx=(int32_t)psx_mod_read_word(a+4)-duke[0],dz=(int32_t)psx_mod_read_word(a+12)-duke[2];
        if(nb<std::size(burn)) burn[nb++]={a,dx*dx+dz*dz};
    };
    consider(player);
    for(uint32_t i=0;table>=0x80000000u && table<0x80200000u && i<count && i<1024;++i) consider(psx_mod_read_word(table+i*0x30+4));
    std::sort(burn,burn+nb,[](const B& x,const B& y){return x.d<y.d;});
    for(unsigned j=0;j<nb && j<5;++j) {
        const uint32_t a=burn[j].a;FireLight& f=light_fire[light_fire_count++];
        // The flames rise from the body: the light sits 450 above its feet.
        f.p[0]=(float)(int32_t)psx_mod_read_word(a+4);f.p[1]=(float)(int32_t)psx_mod_read_word(a+8)-450;f.p[2]=(float)(int32_t)psx_mod_read_word(a+12);
        f.k=light_flicker(a);f.radius=3600*(0.92+0.08*f.k);f.blue=false;
    }
    if(nb) ++light_burning_ticks;
    // Flames in flight: the pool's live flame records, oldest to newest.
    uint32_t fl[8];unsigned nf=0;
    for(uint32_t i=0;i<96 && nf<std::size(fl);++i) {
        const uint32_t a=0x800da978u+i*0x60u;
        const unsigned t=psx_mod_read_byte(a+0x30);
        if((psx_mod_read_half(a)&1u) && (t==14 || t==26)) fl[nf++]=a;
    }
    if(nf) ++light_flame_ticks;
    for(unsigned j=0;j<3 && j<nf && light_fire_count<std::size(light_fire);++j) {
        const uint32_t a=fl[nf<=3?j:(j*(nf-1))/2];FireLight& f=light_fire[light_fire_count++];
        f.p[0]=(float)(int32_t)psx_mod_read_word(a+4);f.p[1]=(float)(int32_t)psx_mod_read_word(a+8);f.p[2]=(float)(int32_t)psx_mod_read_word(a+12);
        f.k=0.75f*light_flicker(a);f.radius=2400*(0.9+0.1*f.k);
        // D41J: the HiTemp Flamethrower's flames (type 26) are drawn blue
        // (0x8006cfd8 picks their own sprites); burns stay orange (same flag
        // word and flame sprites whichever flamethrower lit them).
        f.blue=psx_mod_read_byte(a+0x30)==26;
    }
}
static void light_spark(const double p[3],const ShotLight& s,unsigned type,uint32_t owner);
static unsigned level_lamps_publish(PSXModLight* out,unsigned room); // D41D (level_lights.inc)
static unsigned prop_lamps_publish(PSXModLight* out,unsigned room);  // D41L (level_lights.inc)
// D41F (light_shadows.inc): which of the image's lights may cast shadows
// (cand, one per light), then slots for the strongest of them near Duke.
static void shadows_assign(LightView& v,const uint8_t* cand);
static void shadow_mesh_tick();
static double shadow_probe_eye[3];
static bool shadows_console(const char* arg,void (*say)(const char*));
static void lights_publish() {
    LightView v;v.amb=light_ambient();v.smooth=light_smooth_on();
    uint8_t cand[PSX_MOD_MAX_LIGHTS]={};
    // Rounds in flight, placed lamps, TV screens, prop lamps and fire cast
    // shadows; of the sources below, blasts and test lights do (not muzzle
    // flashes at a gun next to a wall, or sparks on the wall they hit).
    v.n=lights_flight(v.l,8);
    v.n+=level_lamps_publish(v.l+v.n,PSX_MOD_MAX_LIGHTS-v.n-8); // D41D: placed lamps, 8 slots kept for blasts and flashes
    if(security_tv_lights) v.n+=security_tv_lights(v.l+v.n,PSX_MOD_MAX_LIGHTS-v.n-8); // D53: TV screens
    v.n+=prop_lamps_publish(v.l+v.n,PSX_MOD_MAX_LIGHTS-v.n-8); // D41L: woodburners, chandeliers
    for(unsigned i=0;i<light_fire_count && v.n<PSX_MOD_MAX_LIGHTS;++i) {
        const FireLight& f=light_fire[i];PSXModLight& l=v.l[v.n++];
        l.x=f.p[0];l.y=f.p[1];l.z=f.p[2];l.radius=(float)f.radius;
        if(f.blue) {l.r=0.72f*f.k;l.g=0.34f*f.k;l.b=1.18f*f.k;}  // D41J: sampled 0.67 0.24 1.00
        else {l.r=1.15f*f.k;l.g=0.58f*f.k;l.b=0.22f*f.k;}
        l.flags=0;
    }
    for(unsigned i=0;i<v.n;++i) cand[i]=1;
    for(unsigned i=0;i<light_src_count && v.n<PSX_MOD_MAX_LIGHTS;++i) {
        const LightSrc& s=light_srcs[i];
        const double e=light_envelope(s);
        if(e<=0.01) continue;
        PSXModLight& l=v.l[v.n++];
        l.x=(float)s.p[0];l.y=(float)s.p[1];l.z=(float)s.p[2];
        l.radius=(float)(s.radius*(s.kind==light_kind_blast?0.75+0.25*e:1.0));
        l.r=(float)(s.rgb[0]*e);l.g=(float)(s.rgb[1]*e);l.b=(float)(s.rgb[2]*e);
        l.flags=0;
        cand[v.n-1]=s.kind==light_kind_blast || s.kind==light_kind_test;
    }
    shadows_assign(v,cand);
    light_view=v;
}

// D41I probe (developer, DNTTK_LIGHT_PROBE=<hex addr>[,<hex addr>...]):
// function entries counted per address, caller and a0, with a1's first three
// words when a1 points into RAM; printed every 60 Duke updates.
struct LightProbeKey {uint32_t fn,ra,a0,a2,s10;uint64_t n;int32_t p[3];};
static LightProbeKey light_probe_keys[96];static unsigned light_probe_n;
static void lights_probe_entry(CPUState* cpu,uint32_t fn) {
    const uint32_t ra=cpu->gpr[31],a0=cpu->gpr[4],a1=cpu->gpr[5];
    for(unsigned i=0;i<light_probe_n;++i) {LightProbeKey& k=light_probe_keys[i];if(k.fn==fn && k.ra==ra && k.a0==a0) {++k.n;return;}}
    if(light_probe_n<std::size(light_probe_keys)) {
        LightProbeKey& k=light_probe_keys[light_probe_n++];k.fn=fn;k.ra=ra;k.a0=a0;k.a2=cpu->gpr[6];k.s10=psx_mod_read_word(cpu->gpr[29]+0x10);k.n=1;
        for(int i=0;i<3;++i) k.p[i]=(a1>=0x80000000u && a1<0x801ffff0u)?(int32_t)psx_mod_read_word(a1+4*i):0;
    }
}
static void lights_probe_report() {
    static uint32_t t;if(++t%60) return;
    for(unsigned i=0;i<light_probe_n;++i) {const LightProbeKey& k=light_probe_keys[i];
        std::fprintf(stderr,"[TTK light] probe fn %08x ra %08x a0 %08x a2 %08x s10 %08x n %llu a1 %d %d %d\n",k.fn,k.ra,k.a0,k.a2,k.s10,(unsigned long long)k.n,k.p[0],k.p[1],k.p[2]);}
    light_probe_n=0;
}
// D41I probe (developer, DNTTK_PARTICLE_TRACE=1): the live particle list
// (head 0x800dd7f0, +0 next; +8 the kind word from the spawn descriptor, +0x10
// position) summarised once a second: count per kind and the first position.
static void lights_particle_trace() {
    static uint32_t t;if(++t%30) return;
    struct K {uint32_t k,n;int32_t p[3];} ks[16];unsigned nk=0,total=0;
    uint32_t r=psx_mod_read_word(0x800dd7f0u);
    for(unsigned i=0;i<400 && r>=0x80000000u && r<0x801fff80u;++i,r=psx_mod_read_word(r)) {
        ++total;const uint32_t k=psx_mod_read_word(r+8);unsigned j=0;
        while(j<nk && ks[j].k!=k) ++j;
        if(j==nk && nk<16) {ks[nk].k=k;ks[nk].n=0;for(int c=0;c<3;++c) ks[nk].p[c]=(int32_t)psx_mod_read_word(r+0x10+4*c);++nk;}
        if(j<nk) ++ks[j].n;
    }
    {   // Burning objects: flag word +0 bit 0x800 (the draw adds flame sprites per joint).
        const uint32_t table=psx_mod_read_word(0x800de720),count=psx_mod_read_word(0x800de724);
        unsigned burning=0;char bb[256];int bn=0;
        for(uint32_t i=0;table>=0x80000000u && i<count && i<1024;++i) {
            const uint32_t a=psx_mod_read_word(table+i*0x30+4);
            if(a<0x80000000u || a>0x801fff00u || !object_burning(psx_mod_read_word(a))) continue;
            ++burning;if(bn<200) bn+=std::snprintf(bb+bn,sizeof bb-bn," %08x@%d,%d,%d",a,(int32_t)psx_mod_read_word(a+4),(int32_t)psx_mod_read_word(a+8),(int32_t)psx_mod_read_word(a+12));
        }
        if(burning || object_burning(psx_mod_read_word(player))) std::fprintf(stderr,"[TTK light] burning %u%s duke %u\n",burning,bn?bb:"",object_burning(psx_mod_read_word(player))?1u:0u);
    }
    char b[512];int n=std::snprintf(b,sizeof b,"[TTK light] particles %u:",total);
    for(unsigned j=0;j<nk && n<(int)sizeof b-64;++j) n+=std::snprintf(b+n,sizeof b-n," %x x%u @%d,%d,%d",ks[j].k,ks[j].n,ks[j].p[0],ks[j].p[1],ks[j].p[2]);
    std::fprintf(stderr,"%s\n",b);
}
// Once per Duke update (game tick).
static void lights_tick() {
    if(!lights_enabled() || !gameplay_context()) {
        light_src_count=0;light_fire_count=0;light_view=LightView{};return;
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
        const double p[3]={light_console_at?light_console_pos[0]:duke[0],light_console_at?light_console_pos[1]:duke[1]-900,light_console_at?light_console_pos[2]:duke[2]};
        light_add(p,light_console_rgb[0],light_console_rgb[1],light_console_rgb[2],light_console_radius,0,light_kind_test);
        break;
    }
    case 2: {
        unsigned j=0;for(unsigned i=0;i<light_src_count;++i) if(light_srcs[i].kind!=light_kind_test) light_srcs[j++]=light_srcs[i];
        light_src_count=j;break;
    }
    }
    ++light_ticks;
    shadow_mesh_tick();   // D41F
    lights_fire_collect();
    if(std::getenv("DNTTK_LIGHT_PROBE")) lights_probe_report();
    if(std::getenv("DNTTK_PARTICLE_TRACE")) lights_particle_trace();
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
        const unsigned held=psx_mod_read_byte(player+0x3b9),w=base_weapon(held);
        if(w==10) light_add(p,0.45f,1.15f,0.40f,3200,3,light_kind_muzzle);
        else if(w==9) {
            const float k=0.85f+0.3f*(float)((light_ticks*2654435761u>>24)&255)/255.0f;
            if(fl>1e-3) {p[0]+=f[0]/fl*700;p[2]+=f[2]/fl*700;}
            // D41J: the HiTemp Flamethrower (0x8003df40 resolves 9 with the
            // upgrade bit 8 to record 27; its hand draw picks the blue
            // flame sprites) lights blue.
            const bool hitemp=held==27 || (held==9 && (psx_mod_read_half(player+0x2c4+4*9)&8));
            if(hitemp) light_add(p,0.75f*k,0.38f*k,1.20f*k,5200,2,light_kind_muzzle);
            else light_add(p,1.15f*k,0.62f*k,0.24f*k,5200,2,light_kind_muzzle);
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
    if(!valid || !lights_enabled()) {psx_mod_gpu_set_shadows(nullptr,0);psx_mod_gpu_set_lights(nullptr,0,1.0f);return;}
    for(int i=0;i<3;++i) shadow_probe_eye[i]=eye[i];   // D41F developer probe
    PSXModLight l[PSX_MOD_MAX_LIGHTS];
    const unsigned n=std::min<unsigned>(light_view.n,PSX_MOD_MAX_LIGHTS);
    for(unsigned i=0;i<n;++i) {
        l[i]=light_view.l[i];
        l[i].x=(float)(light_view.l[i].x-eye[0]);l[i].y=(float)(light_view.l[i].y-eye[1]);l[i].z=(float)(light_view.l[i].z-eye[2]);
    }
    psx_mod_gpu_set_shadows(light_view.sh,(int)light_view.nsh);   // D41F: before the lights that name them
    psx_mod_gpu_set_lights(l,(int)n,light_view.amb);++light_uploads;
    near_clip_light_probe_lights(l,(int)n,light_view.amb);
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
    // D41F: "light shadows [on|off|default]" (light_shadows.inc).
    if(!std::strncmp(arg,"shadows",7) && (!arg[7] || arg[7]==' ')) return shadows_console(arg+7,say);
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
    // D41I: "light fire on|off" - fire lights (flames, burning objects) for
    // this session (matched comparisons).
    if(!std::strncmp(arg,"fire",4) && (!arg[4] || arg[4]==' ')) {
        const char* v=arg+4;while(*v==' ')++v;
        if(!std::strcmp(v,"on")) light_fire_on.store(1);
        else if(!std::strcmp(v,"off")) light_fire_on.store(0);
        else if(*v) {say("Light: light fire [on|off]");return true;}
        say(light_fire_on.load()?"Light: fire lights on":"Light: fire lights off (session only)");
        return true;
    }
    // D41K: "light at <x> <y> <z> <r> <g> <b> [radius]" - a test light at a
    // world position (colour 0-32, for measurements).
    light_console_at=false;
    if(!std::strncmp(arg,"at ",3)) {
        double x,y,z;float r,g,b;double radius=light_console_radius;
        const int got=std::sscanf(arg+3,"%lf %lf %lf %f %f %f %lf",&x,&y,&z,&r,&g,&b,&radius);
        if(got<6 || r<0 || g<0 || b<0 || r>32 || g>32 || b>32 || radius<100 || radius>30000) {say("Light: light at <x> <y> <z> <r> <g> <b> [radius] (colour 0-32, radius 100-30000)");return true;}
        light_console_rgb[0]=r;light_console_rgb[1]=g;light_console_rgb[2]=b;light_console_radius=radius;
        light_console_pos[0]=x;light_console_pos[1]=y;light_console_pos[2]=z;light_console_at=true;
        light_console.store(1);
        char reply[128];std::snprintf(reply,sizeof reply,"Light: %.2f %.2f %.2f radius %.0f at %.0f %.0f %.0f",r,g,b,radius,x,y,z);
        say(reply);close=true;return true;
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
