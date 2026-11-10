// D41E: Relit levels (documentation/441-d41e-relit-levels.md).
//
// LIGHTING (profile key dynamic_lights, DNTTK_LIGHTS): off = Original (no
// dynamic lights), on = Enhanced (D41A-D41L real lights on top of the painted
// light, which stays as it is), relit = Relit: the painted vertex light is
// dimmed so the placed lamps, shadows, fire and blasts carry the scene.
//
// How much the painted light is dimmed (the "ambient", 1 = as painted) is
// chosen per room, for the room the camera is in (the host visibility walk's
// source room), and eased over about half a second when it changes, like a
// player's eyes adjusting walking from a street into a bar. Per level, in the
// level's lamp file (level_lights.inc), lines
//   ambient P            rooms with lamps of their own (default 60)
//   ambient P unlit      rooms with no lamp of their own (default 80)
//   ambient P outdoor    sky rooms (default 85)
//   ambient P room N     one room (overrides the three above)
//   relit lamps G        the level's lamps, signs, screens and prop lamps
//                        are G percent as strong (default 170, 50-400)
//   relit reach R        and reach R percent as far (default 135, 50-300)
// with P in percent (20-100). Painted light is only taken away where real
// light replaces it: a room without a lamp keeps most of its own.
//
// Readability: the shader (gpu_gl_renderer.c lit_col) does not scale the
// painted colour flat. Dark texels keep more of their light than bright ones
// (factor amb + (1 - amb) k / (k + brightest channel), k 0.08, monotone), so
// dimming takes the painted pools down and never crushes a dark corner to
// black. Enhanced (ambient 1) is unchanged bit for bit.
//
// Console: "lamp ambient" (this room: its target and why), "lamp ambient P
// [unlit|outdoor|room N|here]" (edit; "here" is the camera's room), "lamp
// ambient clear [room N|here]", "lamp ambient lamps G", "lamp ambient reach R"; "lamp save" writes the lines with the lamps.
// Developer DNTTK_LIGHT_AMBIENT=<percent> still fixes the ambient in either
// mode (lab comparisons).
struct RelitLevel {
    float indoor=-1,unlit=-1,outdoor=-1;            // -1: the default
    std::vector<std::pair<int,float>> rooms;        // room, ambient
    float lamps=-1,reach=-1;                        // lamp gain and reach
};
static constexpr float relit_default_lamps=1.70f,relit_default_reach=1.35f;
static constexpr float relit_default_indoor=0.60f,relit_default_unlit=0.80f,relit_default_outdoor=0.85f;
static constexpr float relit_ease=1.0f/48;          // ambient change per game tick (0.4 in about half a second)
static RelitLevel relit_level;                      // game thread, with level_lamps
static std::vector<uint16_t> relit_room_lamps;      // lamps per room (this level)
static size_t relit_room_lamps_for=~size_t(0);
static uint32_t relit_room_lamps_tick;
static int relit_room=-1;static float relit_target=1;static const char* relit_why="";
static unsigned relit_level_seen=~0u;static uint32_t relit_loads_seen=~0u;
static bool lights_relit() {
    const char* t=ttk_live_env("DNTTK_LIGHTS");
    return lights_enabled() && t && !std::strcmp(t,"relit");
}
// level_lights.inc: a line of the lamp file that is an ambient directive.
static bool relit_parse_line(const char* line) {
    while(*line==' ' || *line=='\t') ++line;
    if(!std::strncmp(line,"relit lamps",11)) {
        const float g=(float)std::atof(line+11);
        if(g>=50 && g<=400) relit_level.lamps=g/100.0f;
        return true;
    }
    if(!std::strncmp(line,"relit reach",11)) {
        const float g=(float)std::atof(line+11);
        if(g>=50 && g<=300) relit_level.reach=g/100.0f;
        return true;
    }
    if(std::strncmp(line,"ambient",7) || (line[7]!=' ' && line[7]!='\t')) return false;
    float p=0;char what[16]={0};int room=-1;
    const int n=std::sscanf(line+7,"%f %15s %d",&p,what,&room);
    if(n<1 || p<20 || p>100) return true;           // a directive, but not a valid one: skipped
    const float a=p/100.0f;
    if(n==1) relit_level.indoor=a;
    else if(!std::strcmp(what,"unlit")) relit_level.unlit=a;
    else if(!std::strcmp(what,"outdoor")) relit_level.outdoor=a;
    else if(!std::strcmp(what,"room") && n==3 && room>=0 && room<256) {
        for(auto& r:relit_level.rooms) if(r.first==room) {r.second=a;return true;}
        relit_level.rooms.emplace_back(room,a);
    }
    return true;
}
static void relit_reset() {relit_level=RelitLevel{};relit_room_lamps.clear();relit_room_lamps_for=~size_t(0);}
static void relit_save_lines(std::ostream& f) {
    char b[64];
    auto line=[&](float a,const char* what){std::snprintf(b,sizeof b,"ambient %.0f%s%s\n",a*100,*what?" ":"",what);f<<b;};
    if(relit_level.indoor>=0) line(relit_level.indoor,"");
    if(relit_level.unlit>=0) line(relit_level.unlit,"unlit");
    if(relit_level.outdoor>=0) line(relit_level.outdoor,"outdoor");
    if(relit_level.lamps>=0) {std::snprintf(b,sizeof b,"relit lamps %.0f\n",relit_level.lamps*100);f<<b;}
    if(relit_level.reach>=0) {std::snprintf(b,sizeof b,"relit reach %.0f\n",relit_level.reach*100);f<<b;}
    auto rooms=relit_level.rooms;std::sort(rooms.begin(),rooms.end());
    for(const auto& r:rooms) {char w[16];std::snprintf(w,sizeof w,"room %d",r.first);line(r.second,w);}
}
static uint32_t relit_room_flags(int room) {
    const uint32_t table=psx_mod_read_word(0x800da454),rooms=psx_mod_read_word(0x800da47c);
    if(room<0 || (uint32_t)room>=rooms || rooms>256 || table<0x80010000u || table>0x80200000u-rooms*88 || (table&3)) return 0;
    return psx_mod_read_word(table+88*(uint32_t)room+12);
}
// Lamps per room, recounted when the lamp list changes and every 2 s (a
// lamp's room is found once, level_lamp_room).
static void relit_count_lamps() {
    if(relit_room_lamps_for==level_lamps.size() && light_ticks-relit_room_lamps_tick<60) return;
    relit_room_lamps_for=level_lamps.size();relit_room_lamps_tick=light_ticks;
    relit_room_lamps.assign(256,0);
    for(LevelLamp& l:level_lamps) {
        if(l.room==-2) l.room=level_lamp_room(l);
        if(l.room>=0 && l.room<256 && l.standing!=0 && relit_room_lamps[l.room]<65535) ++relit_room_lamps[l.room];
    }
}
static float relit_room_target(int room,const char** why) {
    for(const auto& r:relit_level.rooms) if(r.first==room) {*why="room";return r.second;}
    const float indoor=relit_level.indoor>=0?relit_level.indoor:relit_default_indoor;
    if(relit_room_flags(room)&8) {*why="outdoor";return relit_level.outdoor>=0?relit_level.outdoor:relit_default_outdoor;}
    relit_count_lamps();
    if(room>=0 && (size_t)room<relit_room_lamps.size() && relit_room_lamps[room]) {*why="lamps";return indoor;}
    *why="unlit";return std::max(indoor,relit_level.unlit>=0?relit_level.unlit:relit_default_unlit);
}
// The scene lights' gain: eased with the ambient (0 at ambient 1, the full
// gain at the room's dimmed value), so lamps strengthen as the level dims.
static bool light_relit_lamp_gain(float& gain,float& reach) {
    const float g=relit_level.lamps>=0?relit_level.lamps:relit_default_lamps;
    const float r=relit_level.reach>=0?relit_level.reach:relit_default_reach;
    const float e=std::clamp((1.0f-std::clamp(light_relit_amb,0.2f,1.0f))/0.4f,0.0f,1.0f);
    gain=1.0f+(g-1.0f)*e;reach=1.0f+(r-1.0f)*e;
    return e>0;
}
// Game tick (dynamic_lights.inc lights tick): the ambient the images use.
static void relit_tick() {
    const bool relit=lights_relit();
    float target=1;
    if(relit) {
        if(host_vis_shown_valid && host_vis_last_order_n) relit_room=host_vis_last_order[0]&0xff;
        target=relit_room>=0?relit_room_target(relit_room,&relit_why):(relit_level.indoor>=0?relit_level.indoor:relit_default_indoor);
    }
    if(static const bool trace=std::getenv("DNTTK_LAMP_TRACE")!=nullptr;trace && (target!=relit_target || (relit && light_ticks%300==0)))
        {float g=1,r=1;light_relit_lamp_gain(g,r);std::fprintf(stderr,"[TTK relit] room %d target %.2f (%s) now %.2f lamps x%.2f reach x%.2f\n",relit_room,target,relit?relit_why:"not relit",light_relit_amb,g,r);}
    relit_target=target;
    // A level or savestate load starts at its room's value; a room change or
    // a LIGHTING change eases.
    const unsigned level=psx_mod_read_word(0x800be570);const uint32_t loads=psx_mod_savestate_loads();
    if(level!=relit_level_seen || loads!=relit_loads_seen) {relit_level_seen=level;relit_loads_seen=loads;light_relit_amb=target;return;}
    const float d=target-light_relit_amb;
    light_relit_amb=std::fabs(d)<=relit_ease?target:light_relit_amb+(d>0?relit_ease:-relit_ease);
}
// "lamp ambient ..." (queued to the game thread by lamp_console_command).
static void relit_console(const char* rest) {
    while(*rest==' ') ++rest;
    const int room=relit_room;
    if(!*rest) {
        const char* why="";const float t=room>=0?relit_room_target(room,&why):relit_target;
        level_lamp_notice("Ambient: room %d %.0f%% (%s)%s, now %.0f%%. Level: lamps %.0f unlit %.0f outdoor %.0f",room,t*100,why,
                          lights_relit()?"":" - LIGHTING is not RELIT",light_relit_amb*100,
                          (relit_level.indoor>=0?relit_level.indoor:relit_default_indoor)*100,(relit_level.unlit>=0?relit_level.unlit:relit_default_unlit)*100,
                          (relit_level.outdoor>=0?relit_level.outdoor:relit_default_outdoor)*100);
        std::fprintf(stderr,"[TTK relit] ambient room %d target %.2f (%s) now %.2f\n",room,t,why,light_relit_amb);
        return;
    }
    if(!std::strncmp(rest,"clear",5)) {
        const char* w=rest+5;while(*w==' ') ++w;
        int r=-1;
        if(!std::strcmp(w,"here")) r=room;else if(!std::strncmp(w,"room",4)) r=std::atoi(w+4);
        else if(!*w) {relit_level=RelitLevel{};level_lamps_dirty=true;level_lamp_notice("Ambient: level defaults (lamp save keeps)");return;}
        auto& v=relit_level.rooms;v.erase(std::remove_if(v.begin(),v.end(),[&](const auto& x){return x.first==r;}),v.end());
        level_lamps_dirty=true;level_lamp_notice("Ambient: room %d follows the level again",r);return;
    }
    if(!std::strncmp(rest,"reach",5)) {
        char line[48];std::snprintf(line,sizeof line,"relit reach %s",rest+5);relit_parse_line(line);level_lamps_dirty=true;
        level_lamp_notice("Relit lamp reach %.0f%% (lamp save keeps it)",(relit_level.reach>=0?relit_level.reach:relit_default_reach)*100);return;
    }
    if(!std::strncmp(rest,"lamps",5)) {
        char line[48];std::snprintf(line,sizeof line,"relit lamps %s",rest+5);relit_parse_line(line);level_lamps_dirty=true;
        level_lamp_notice("Relit lamp strength %.0f%% (lamp save keeps it)",(relit_level.lamps>=0?relit_level.lamps:relit_default_lamps)*100);return;
    }
    float p=0;char what[16]={0};int r=-1;
    const int n=std::sscanf(rest,"%f %15s %d",&p,what,&r);
    if(n<1 || p<20 || p>100) {level_lamp_notice("Ambient: lamp ambient [20-100 [unlit|outdoor|room N|here]] | clear [room N|here]");return;}
    char line[48];
    if(n>=2 && !std::strcmp(what,"here")) {if(room<0) {level_lamp_notice("Ambient: the camera's room is not known yet");return;}std::snprintf(line,sizeof line,"ambient %.0f room %d",p,room);}
    else std::snprintf(line,sizeof line,"ambient %s",rest);
    if(!relit_parse_line(line)) return;
    level_lamps_dirty=true;
    const char* why="";const float t=room>=0?relit_room_target(room,&why):relit_target;
    level_lamp_notice("Ambient set (%s). This room: %.0f%% (%s). lamp save keeps it",line+8,t*100,why);
}
