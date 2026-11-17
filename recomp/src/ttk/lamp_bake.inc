// D41M: baked light of the level's fixed lamps (included by
// modern_controls.cpp after level_lights.inc and relit.inc; design and
// evidence in documentation/446-d41m-baked-lamps.md).
//
// Game thread, once a level's room mesh (light_shadows.inc) and lamps are
// known: the bake runs on its own thread (bake_state.cpp), or loads from the
// cache beside the executable (cache/lamp-bake/levelNN-<hash>.bake). When
// ready, the lightmap goes to the renderers (psx_mod_gpu_set_lightmap) and
// the face records to the shared map the redraw workers read
// (lamp_bake_map.cpp); world faces then carry their surface, the lamps the
// bake holds are marked (flag 128) so baked surfaces skip them in the live
// loop, and each image sends the lamps' current colours
// (psx_mod_gpu_set_lamp_levels). Models (Duke, enemies, props) keep the
// live lamps, each with the fixed reach the bake gave it (user choice: lamp
// reach no longer follows the camera's room). Full build (2026-11-11): on in
// Enhanced and Relit, on every level, with SHADOWS on or off (walls always
// stop baked lamps); developer DNTTK_LAMP_BAKE=0 or console "light bake
// off" for this session keeps the live lamps.
// D41P: models (Duke, his weapon, enemies, props) take the fixed lamps from
// the bake of the floor they stand on, as Quake's R_LightPoint; developer
// DNTTK_MODEL_BAKE=0 keeps them on the live lamps (as before) for an A/B.
static bool model_bake_on() {
    static const bool v=[]{const char* t=std::getenv("DNTTK_MODEL_BAKE");return !(t && t[0]=='0');}();
    return v;
}
static bool lamp_bake_wanted() {
    const int o=lamp_bake_override.load(std::memory_order_relaxed);
    if(o>=0) return o!=0;
    static const bool v=[]{const char* t=std::getenv("DNTTK_LAMP_BAKE");return !(t && t[0]=='0');}();
    return v;
}
struct LampBakeJob {ttk::BakeOutput out;bool ok=false,cached=false;std::string error,path;unsigned faces=0,lamps=0;ttk::LampFloorGrid floor;
    double t_read=0,t_floor=0,t_copy=0;};   // D41P: the floor grid; D41W: stage times
// D41W: when each stage after a load happened (steady clock, seconds; the
// same clock as the renderer's [gl bake] line and Python's time.monotonic).
static double bake_now() {return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();}
static struct {uint64_t loads=~0ull;unsigned level=~0u;double t=0,inputs=0,start=0;bool open=false;} bake_load;
static struct {
    uint64_t key=0;                 // inputs of the bake shown (or being made)
    bool active=false;
    unsigned level=~0u,lamps=0;     // lamps baked (level_lamps[0..lamps))
    std::vector<uint8_t> channel;   // per baked lamp: has its own channel
    std::vector<float> reach;       // per baked lamp: the reach factor it was baked with
    std::future<LampBakeJob> job;uint64_t job_key=0;
    std::vector<std::pair<uint32_t,uint32_t>> job_records;
    uint64_t corners_seen=0;
    bool held=false;uint32_t hold_since=0;   // waiting for a reloaded level's inputs
    // D41W: a finished bake waits here until the renderer draws with it
    // (psx_mod_lightmap_shown): until then the live lamps (or the bake shown
    // before) keep lighting the level, never neither.
    bool waiting=false;double wait_since=0;LampBakeJob ready;
} bake_state;
// D23AF1 (on by default since 2026-11-17; DNTTK_BAKE_YIELD=0 turns it off): a level's first
// bake after an update ran 15-27 s on the field laptop (5-8 s on citadel) and
// held the game at 38-50 frames a second throughout, on its 2 cores beside
// the emulation thread; the redraw governor then took the original
// presentation and its backoff kept the redraws off for minutes after the
// bake ended (note 503). With the switch: one bake thread at full duty on
// CPUs of 2 physical cores; on more cores the bake works at a quarter duty
// while the game is behind (bake_behind, set by the per-second judge for 3
// s). Seconds while a bake runs are judged, but a fallback taken then does
// not lengthen the governor's backoff and the redraws are retried as soon as
// the bake ends (replay_load_bake, frame_replay.cpp). The bake's output is unchanged
// (charts are independent; only the pace of the work changes).
extern "C" int psx_host_physical_cores(void);
static std::atomic<int> bake_behind{0};
static bool bake_yield_on() {static const bool v=[]{const char* t=std::getenv("DNTTK_BAKE_YIELD");return !(t && t[0]=='0');}();return v;}
static bool lamp_bake_running() {
    return bake_state.job.valid() && bake_state.job.wait_for(std::chrono::seconds(0))!=std::future_status::ready;
}
static float lamp_style_mean(const LevelLamp& l) {
    switch(l.style) {
    case lamp_flicker: return 0.945f;
    case lamp_pulse: return 0.7f;
    case lamp_candle: return 0.89f;
    case lamp_strobe: return 2.0f/15;
    case lamp_broken: return 0.13f;
    case lamp_wave: return 0.8f;
    case lamp_signal: return 0.5f;
    default: return 1.0f;
    }
}
// D55A: how far a lamp's level strays from its mean (lamp_style_value), for
// the bake's choice of a face's four channel lamps. A steady lamp on a prop
// goes out when the prop breaks (D54).
static float lamp_style_swing(const LevelLamp& l) {
    switch(l.style) {
    case lamp_flicker: return 0.3f;
    case lamp_pulse: return 0.3f;
    case lamp_candle: return 0.11f;
    case lamp_strobe: return 0.87f;
    case lamp_broken: return 0.87f;
    case lamp_wave: return std::clamp(l.depth,0.05f,1.0f)*0.5f;
    case lamp_signal: return 0.5f;
    default: return l.props.empty()?0.0f:0.5f;
    }
}
// Each lamp's reach as the live path gives it standing in the lamp's own
// room (Relit, relit.inc light_relit_lamp_gain; 1 outside Relit).
static float lamp_bake_reach(LevelLamp& l) {
    if(!lights_relit()) return 1.0f;
    if(l.room==-2) l.room=level_lamp_room(l);
    const char* why="";
    const float a=l.room>=0?relit_room_target(l.room,&why):(relit_level.indoor>=0?relit_level.indoor:relit_default_indoor);
    const float r=relit_level.reach>=0?relit_level.reach:relit_default_reach;
    const float e=std::clamp((1.0f-std::clamp(a,0.2f,1.0f))/0.4f,0.0f,1.0f);
    return 1.0f+(r-1.0f)*e;
}
// Developer DNTTK_BAKE_WAIT=0: the pre-D41W timing (a bake published as soon
// as it is read, before the renderer has it; lamps' walls checked 8 a tick).
static bool bake_wait_on() {static const bool v=[]{const char* t=std::getenv("DNTTK_BAKE_WAIT");return !(t && t[0]=='0');}();return v;}
static bool lamp_bake_is_active() {return bake_state.active || !bake_wait_on();}
static void lamp_bake_stop(const char* why) {
    if(!bake_state.active) return;
    bake_state.active=false;bake_state.key=0;bake_state.held=false;
    ttk::lamp_bake_map_clear();psx_mod_gpu_set_lightmap(nullptr);
    std::fprintf(stderr,"[TTK bake] off (%s)\n",why);
}
static std::string lamp_bake_cache_dir() {
    // Developer DNTTK_LAMP_BAKE_CACHE: another cache folder (lab runs time
    // first bakes without touching the player's cache).
    if(const char* t=std::getenv("DNTTK_LAMP_BAKE_CACHE"); t && *t) return t;
    std::string dir="cache/lamp-bake";
    if(auto base=SDL_GetBasePath()) {
        dir=std::string(base)+"cache/lamp-bake";
#if !defined(PSX_SDL3)
        SDL_free(base);
#endif
    }
    return dir;
}
// Game tick, after shadow_mesh_tick.
static void lamp_bake_tick() {
    const unsigned level=psx_mod_read_word(0x800be570);
    if(const uint64_t ld=psx_mod_savestate_loads();ld!=bake_load.loads || level!=bake_load.level) {
        bake_load.loads=ld;bake_load.level=level;bake_load.t=bake_now();bake_load.inputs=bake_load.start=0;bake_load.open=true;
    }
    const bool wanted=lamp_bake_wanted() && lights_enabled() && shadow_mesh_tris && !shadow_mesh_faces.empty()
        && level==level_lamps_level && !level_lamps.empty();
    if(!wanted) {
        // A bake finishing after the switch-off published its lightmap: clear it.
        if(bake_state.job.valid() && bake_state.job.wait_for(std::chrono::seconds(0))==std::future_status::ready) {bake_state.job.get();psx_mod_gpu_set_lightmap(nullptr);bake_state.key=0;}
        if(bake_state.waiting) {bake_state.waiting=false;bake_state.ready=LampBakeJob{};psx_mod_gpu_set_lightmap(nullptr);bake_state.key=0;}
        // A load of the same level rebuilds the room mesh and lamps over a few
        // ticks: the bake shown stays (same inputs come back: nothing to redo)
        // instead of the live lamps showing for half a second after every load.
        if(lamp_bake_wanted() && lights_enabled() && bake_state.active && level==bake_state.level) {
            if(!bake_state.held) {bake_state.held=true;bake_state.hold_since=light_ticks;}
            if(light_ticks-bake_state.hold_since<300) return;   // about 5 s
        }
        lamp_bake_stop(lamp_bake_wanted()?"no room mesh or lamps":"switched off");return;
    }
    bake_state.held=false;
    if(bake_load.open && !bake_load.inputs) bake_load.inputs=bake_now();
    // Inputs: the mesh build, the lamps and the lighting mode.
    uint64_t key=1469598103934665603ull;
    auto mix=[&](uint64_t v){key=(key^v)*1099511628211ull;};
    mix(level);mix(shadow_mesh_sig);mix(lights_relit());   // the mesh content, not its rebuilds (a reload rebuilds it)
    mix(level_lamps.size());
    for(const LevelLamp& l:level_lamps) {
        for(int k=0;k<3;++k) {mix((uint64_t)std::llround(l.p[k]));mix((uint64_t)std::lround(l.rgb[k]*1000));}
        mix((uint64_t)std::llround(l.radius));mix((uint64_t)std::lround(l.k*1000));mix((uint64_t)l.style);mix(l.props.size());
    }
    if(bake_state.job.valid()) {
        if(bake_state.job.wait_for(std::chrono::seconds(0))!=std::future_status::ready) return;
        LampBakeJob r=bake_state.job.get();
        if(bake_state.job_key!=key) {psx_mod_gpu_set_lightmap(nullptr);return;}   // inputs changed meanwhile: bake again below
        if(!r.ok) {std::fprintf(stderr,"[TTK bake] level %u failed: %s\n",level,r.error.c_str());bake_state.key=key;return;}
        bake_state.ready=std::move(r);bake_state.waiting=true;bake_state.wait_since=bake_now();
    }
    if(bake_state.waiting) {
        if(bake_state.job_key!=key) {bake_state.waiting=false;bake_state.ready=LampBakeJob{};psx_mod_gpu_set_lightmap(nullptr);}   // inputs changed meanwhile: bake again below
        else if(bake_wait_on() && !psx_mod_lightmap_shown() && bake_now()-bake_state.wait_since<2.0) return;   // the renderer is still uploading it
        else {
        bake_state.waiting=false;
        LampBakeJob r=std::move(bake_state.ready);bake_state.ready=LampBakeJob{};
        if(model_bake_on()) ttk::lamp_bake_floor_stage(r.floor);   // D41P: before the publish that flips the table
        else ttk::lamp_bake_floor_stage(ttk::LampFloorGrid{});
        std::fprintf(stderr,"[TTK bake] level %u: models lit from the floors' bake %s (%ux%u cells of %.0f, %zu floor entries)\n",level,model_bake_on()?"on":"off",r.floor.w,r.floor.h,r.floor.cell,r.floor.y.size());
        ttk::lamp_bake_map_publish(bake_state.job_records);
        bake_state.active=true;bake_state.key=key;bake_state.level=level;
        if(bake_load.open) {
            const double t=bake_load.t,ms=1000;bake_load.open=false;
            std::fprintf(stderr,"[TTK bake] level %u after the load (t %.3f): inputs %.0f ms, start %.0f, read %.0f, store %.0f, floor %.0f, published %.0f ms\n",
                level,t,(bake_load.inputs-t)*ms,(bake_load.start-t)*ms,(r.t_read-t)*ms,(r.t_copy-t)*ms,(r.t_floor-t)*ms,(bake_now()-t)*ms);
        }
        if(r.cached) std::fprintf(stderr,"[TTK bake] level %u: %u faces, %u lamps, atlas %dx%d (%.1f MB), loaded from the cache in %.0f ms\n",
            level,r.faces,r.lamps,r.out.width,r.out.height,r.out.width*(double)r.out.height*8/1e6,r.out.ms);
        else std::fprintf(stderr,"[TTK bake] level %u: %u faces, %u lamps, atlas %dx%d (%.1f MB), baked in %.0f ms (%.1fM rays); %u faces lit, %u two-sided, %u channel overflows (%u on/off lamps); faces by varying lamps: 1-4 %u, 5-6 %u, 7-8 %u, 9+ %u\n",
            level,r.faces,r.lamps,r.out.width,r.out.height,r.out.width*(double)r.out.height*8/1e6,r.out.ms,r.out.rays/1e6,r.out.faces_lit,r.out.faces_two_sided,r.out.channel_overflows,r.out.strong_overflows,r.out.faces_vary[0],r.out.faces_vary[1],r.out.faces_vary[2],r.out.faces_vary[3]);
        return;
        }
    }
    if(key==bake_state.key) {
        if(bake_load.open && bake_state.active) {bake_load.open=false;std::fprintf(stderr,"[TTK bake] level %u after the load (t %.3f): the bake shown is kept\n",level,bake_load.t);}
        return;
    }
    // Wait until level_lamps_update has moved every lamp off its prop's wall
    // (a few a tick after a load): the inputs are then final.
    for(const LevelLamp& l:level_lamps) if(!l.wall_checked) return;
    // A new bake: copy the inputs now (game thread), work on its own thread.
    ttk::BakeInput in;in.tris=shadow_mesh_copy;
    bake_state.job_records.clear();
    for(size_t i=0;i<shadow_mesh_faces.size();++i) {
        in.faces.push_back({shadow_mesh_faces[i].first,shadow_mesh_faces[i].count});
        bake_state.job_records.push_back({shadow_mesh_faces[i].record,(uint32_t)(i+1)});
    }
    const unsigned nl=(unsigned)std::min<size_t>(level_lamps.size(),PSX_MOD_BAKE_LAMPS);
    bake_state.channel.assign(nl,0);
    bake_state.reach.assign(nl,1.0f);
    for(unsigned i=0;i<nl;++i) {
        LevelLamp& l=level_lamps[i];ttk::BakeLamp b{};
        for(int k=0;k<3;++k) {b.p[k]=(float)l.p[k];b.rgb[k]=l.rgb[k]*l.k;}
        bake_state.reach[i]=lamp_bake_reach(l);
        b.radius=(float)l.radius*bake_state.reach[i];
        b.channel=l.style!=lamp_steady || !l.props.empty();b.mean=lamp_style_mean(l);b.swing=lamp_style_swing(l);
        bake_state.channel[i]=b.channel;
        in.lamps.push_back(b);
    }
    bake_state.lamps=nl;
    if(const char* t=std::getenv("DNTTK_LAMP_BAKE_TEXEL")) in.texel=(float)std::clamp(std::atof(t),4.0,128.0);
    if(const char* t=std::getenv("DNTTK_LAMP_BAKE_SAMPLES")) in.samples=std::clamp(std::atoi(t),1,4);
    if(const char* t=std::getenv("DNTTK_LAMP_BAKE_THREADS")) in.threads=(unsigned)std::clamp(std::atoi(t),1,64);
    if(bake_yield_on()) {
        // D23AF1 refinement: on 2 physical cores one bake thread at full duty
        // (the quarter duty stretched TIME TO KILL's bake from 28 s to 110 s
        // on the field laptop); on more cores the bake yields while behind.
        if(psx_host_physical_cores()<=2) {if(!in.threads) in.threads=1;}
        else in.behind=&bake_behind;
    }
    char name[64];std::snprintf(name,sizeof name,"/level%02u-%016llx.bake",level,(unsigned long long)ttk::lamp_bake_hash(in));
    const std::string path=lamp_bake_cache_dir()+name;
    // A bake still shown from another level must not light this one meanwhile.
    if(bake_state.active && bake_state.level!=level) lamp_bake_stop("another level");
    bake_state.job_key=key;bake_state.key=key;
    if(bake_load.open) bake_load.start=bake_now();
    std::fprintf(stderr,"[TTK bake] level %u: %zu faces, %u lamps, texel %.0f: %s\n",level,in.faces.size(),nl,in.texel,path.c_str());
    bake_state.job=std::async(std::launch::async,[in=std::move(in),path]{
        // The renderers' copy is made here, off the game thread (90 MB on
        // level 3); the texels are then dropped, the statistics kept.
        auto publish=[&in](LampBakeJob& r){
            if(!r.ok) return;
            // D41W: the renderer's copy first, so its upload runs while the floors build.
            PSXModLightmap m{};m.width=r.out.width;m.height=r.out.height;m.steady=r.out.steady.data();m.channels=r.out.channels.data();
            m.charts=r.out.charts.data();m.nsurf=(uint32_t)(r.out.charts.size()/16);
            psx_mod_gpu_set_lightmap(&m);r.t_copy=bake_now();
            r.out.steady=std::vector<uint32_t>();r.out.channels=std::vector<uint32_t>();r.out.charts=std::vector<float>();
            // D41P: the floors models take their fixed-lamp light from.
            std::vector<std::pair<uint32_t,uint32_t>> faces;faces.reserve(in.faces.size());
            for(const auto& f:in.faces) faces.push_back({f.first,f.count});
            r.floor=ttk::lamp_bake_floor_build(in.tris,faces);r.t_floor=bake_now();
        };
        LampBakeJob r;r.path=path;r.faces=(unsigned)in.faces.size();r.lamps=(unsigned)in.lamps.size();
        const auto t0=std::chrono::steady_clock::now();
        const bool hit=ttk::lamp_bake_load(path,r.out);r.t_read=bake_now();
        if(hit && r.out.charts.size()==in.faces.size()*16) {
            r.ok=r.cached=true;r.out.ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t0).count();
            publish(r);return r;
        }
        r.ok=ttk::lamp_bake(in,r.out,r.error);
        if(r.ok) {std::error_code ec;std::filesystem::create_directories(std::filesystem::path(path).parent_path(),ec);ttk::lamp_bake_save(path,r.out);}
        publish(r);
        return r;
    });
}
// The reach factor of a published level lamp (index into level_lamps) while
// the bake shows: the one it was baked with, so a model lit by the live lamp
// sees the same pool as the wall beside it. -1 without a bake.
static float lamp_bake_published_reach(unsigned lamp) {
    if(!bake_state.active || lamp>=bake_state.reach.size()) return -1.0f;
    return bake_state.reach[lamp];
}
// D41P2: with a bake shown, the fixed lamps light every surface only through
// the bake (world faces their own, models, props and room-attached meshes the
// floor under them, near_clip.cpp): they leave the live light list (no
// shadow map, no shader loop; Quake's lightmaps and R_LightPoint). Returns 2
// for that, after clearing the lamps' shadow candidacy. Developer DNTTK_D41P:
// "live" keeps them in the list with their shadow maps (D41P as accepted),
// "noshadow" in the list without maps (note 470's measurement).
static int lamp_bake_d41p(uint8_t* cand,unsigned lamp0,unsigned lamp1) {
    static const int mode=[]{const char* t=std::getenv("DNTTK_D41P");return !t ? 2 : !std::strcmp(t,"live") ? 0 : !std::strcmp(t,"noshadow") ? 1 : 2;}();
    if(!mode || !bake_state.active) return 0;
    for(unsigned i=lamp0;i<lamp1;++i) cand[i]=0;
    return mode;
}
// Game tick, in lights_publish after the shadow slots: the image's lamp
// colours and gain, and the lamps the bake holds marked for the renderer.
static void lamp_bake_levels(LightStage& v,unsigned lamp0,unsigned lamp1) {
    v.bake_n=0;v.bake_gain=1;
    if(!bake_state.active) return;
    float g=1,rr=1;light_relit_lamp_gain(g,rr);
    v.bake_gain=g;
    const unsigned n=std::min<unsigned>(bake_state.lamps,(unsigned)level_lamps.size());
    const LampWave wave=lamp_wave_now();
    const bool breaks=level_lamps_breaks.load(std::memory_order_relaxed)!=0;
    for(unsigned i=0;i<n;++i) {
        const LevelLamp& l=level_lamps[i];
        float rgb[3]={l.rgb[0],l.rgb[1],l.rgb[2]};
        float e=l.k*(l.style==lamp_wave ? lamp_wave_value(wave,l.phase,l.depth) : lamp_style_value(l.style,light_ticks,i));
        if(l.style==lamp_signal) signal_lamp_now(i,rgb,e);   // D55A: colour and brightness (it lost the brightness)
        if(l.standing==0 && breaks) e*=prop_dying_level(light_ticks-l.broke_tick,i);
        for(int a=0;a<3;++a) v.bake_rgb[i*3+a]=rgb[a]*e*g;
    }
    v.bake_n=std::max(1u,n);
    // The live lamps (for models) are the bake's own: baked surfaces skip them.
    for(unsigned i=lamp0;i<lamp1 && i<v.n;++i) v.l[i].flags+=128.0f;
}
