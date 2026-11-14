// D41U: an explosion chain sounds as one blast (documentation/473-d41u-blast-chain-audio.md).
//
// Every explosion starts its own blast sound, so a chain of barrels stacked
// one full-volume voice per barrel (8 barrels: 8 voices, 5 started in one
// tick, MEASURED). The sound calls (note 370 D39F map, note 473):
// - barrels: 0x80098a88, the object death-sound dispatcher (a switch on the
//   type +0x2c), plays the type's sound 0x8006b270(id, object+4, 0x800)
//   (return 0x80098d5c; type 944 id 0x6002) or 0x8006b73c inside the same
//   routine. A barrel is a type whose flag word has bit 0x10000 (D41T).
// - rockets and grenades 0x8006b73c ra 0x80070418, pipe bombs 0x800708e4,
//   dynamite 0x80071044; under water 0x8006b270 ra 0x80070858 / 0x80070ff8.
//   Gib bursts (0x8006413c) are a splat, not a blast: left alone.
// The sound object (0x8006b270): +8 state, +0x18 the position it is mixed
// from (its own copy at +0x1c unless a2 has 0x100), +0x3c/+0x3d its SPU
// voices (-1 none), +0x5c id, +0x6a volume (table copy; the update ramps the
// voice to it). Voices come from a free list (0x800da5c0, 0x80068658) and are
// never stolen: with none free a new sound waits silent.
//
// The same rule as the D41T light, decided while the game runs (no level
// data): a barrel blast within chain_reach of a chain whose bang began less
// than bang_window tics ago joins it. Its call is refused (a0 = -1: both
// routines bail on bank 0xff), and the chain's voice swells (+12% volume per
// blast, up to the SPU's 0x3fff) and moves 65% of the way toward the blast.
// A chain still going after bang_window (each blast within chain_window of
// the last) bangs again: the new barrel plays as the chain's head and the
// previous voice is ducked to a tail, so the room hears one blast rolling,
// not a stack. Only barrels are set off by other blasts; rockets, grenades,
// pipe bombs and dynamite join a chain only when they go off together
// (within together_window tics: pipe bombs detonated at once, a rocket into a
// row of barrels), so rockets fired one after another each bang (measured:
// RPG blasts 13-14 tics apart). The first bang plays at once, and a blast
// farther than chain_reach from every live chain is its own.
// One voice cannot grow: a close barrel's voice is already at the SPU's
// per-voice ceiling (MEASURED 31148 of 32766, left + right), and barrels
// that join after the head's attack (the chain's next stage, 6 tics later in
// the lab) would be lost in its decay. So the chain's next stage (a merge
// blast_layer_after tics or more after the head, or its third blast) adds
// its one rumble: the merged blast's own sound through the pitched call
// 0x8006b73c (as kick.inc's thud) at pitch value blast_layer_pitch, the
// table pitch less 2 steps (MEASURED for 0x6002: SPU 0x2C9 against the
// head's 0x355-0x3E0, about 3 semitones down), at table volume. A chain holds
// at most these two voices; before, 20 bunched barrels took all 24 SPU
// voices and clipped the mix.
//
// Modernized only, Polymorph only (user 2026-11-14: Vanilla and Classic
// unchanged). Console "chainsound on|off|default" (session A/B), developer
// DNTTK_BLAST_AUDIO=0, DNTTK_BLAST_AUDIO_TRACE=1.
static constexpr double blast_chain_reach=4500;
static constexpr uint32_t blast_bang_window=15,blast_chain_window=12,blast_together_window=2;  // game tics (30 a second)
static constexpr double blast_swell_step=0.12,blast_move=0.65,blast_tail_gain=0.4,blast_layer_gain=1.0;
static constexpr int blast_layer_at=3,blast_layer_pitch=-2;static constexpr uint32_t blast_layer_after=3;
static constexpr uint32_t blast_volume_max=0x3fff,blast_sound_list=0x800da5b8;
struct BlastChain {double p[3],hp[3];uint32_t id,node,head_tick,last_tick,base_vol,layer,layer_id;int fed;bool live;};
static BlastChain blast_chains[6];
static uint32_t blast_audio_ticks;static uint64_t blast_audio_loads=~0ull;
static uint64_t blast_heads,blast_merges,blast_rebangs,blast_ducks,blast_layers;
static std::atomic<int> blast_audio_override{-1};
static bool blast_audio_enabled() {
    const int o=blast_audio_override.load(std::memory_order_relaxed);
    if(o>=0) return o!=0 && input_modernized() && !ttk_world_classic();
    static const bool env_off=[]{const char* t=std::getenv("DNTTK_BLAST_AUDIO");return t && !std::strcmp(t,"0");}();
    return !env_off && input_modernized() && !ttk_world_classic();
}
static bool blast_audio_trace() {static const bool on=std::getenv("DNTTK_BLAST_AUDIO_TRACE")!=nullptr;return on;}
// Game tick (Duke's update).
static void blast_audio_tick() {
    ++blast_audio_ticks;
    if(blast_audio_loads!=psx_mod_savestate_loads()) {blast_audio_loads=psx_mod_savestate_loads();for(BlastChain& c:blast_chains) c.live=false;}
}
// The chain head's sound object: found once in the playing list (its id and
// the head blast's position, which the routine copied), then checked on every use (the pool
// recycles objects).
static bool blast_node_valid(const BlastChain& c) {
    return c.node && psx_mod_read_word(c.node+0x5c)==c.id && psx_mod_read_half(c.node+8);
}
static uint32_t blast_find_node(BlastChain& c) {
    if(blast_node_valid(c)) return c.node;
    c.node=0;
    if(psx_mod_read_word(0x800c39b4)!=7) return 0;   // sound system running
    for(uint32_t node=psx_mod_read_word(blast_sound_list),n=0;node && n<64;++n) {
        if(node<0x80010000u || node>0x801fff80u) break;
        if(psx_mod_read_word(node+0x5c)==c.id && psx_mod_read_half(node+8) && psx_mod_read_word(node+0x18)==node+0x1c) {
            const double dx=(int32_t)psx_mod_read_word(node+0x1c)-c.hp[0],dy=(int32_t)psx_mod_read_word(node+0x20)-c.hp[1],dz=(int32_t)psx_mod_read_word(node+0x24)-c.hp[2];
            if(dx*dx+dy*dy+dz*dz<4.0) {c.node=node;c.base_vol=psx_mod_read_half(node+0x6a);return node;}
        }
        const uint32_t next=psx_mod_read_word(node);
        if(next==node) break;
        node=next;
    }
    return 0;
}
// A blast sound call (entry of 0x8006b270 or 0x8006b73c, a0 id, a1 position).
static void blast_audio_sound(CPUState* cpu,const char* why,bool barrel=false) {
    if(!blast_audio_enabled() || !gameplay_context()) return;
    const uint32_t id=cpu->gpr[4],pos=cpu->gpr[5];
    if((int32_t)id<0 || pos<0x80000000u || pos>0x801ffff0u) return;
    const double p[3]={(double)(int32_t)psx_mod_read_word(pos),(double)(int32_t)psx_mod_read_word(pos+4),(double)(int32_t)psx_mod_read_word(pos+8)};
    const uint32_t t=blast_audio_ticks;
    BlastChain* best=nullptr;double bd=blast_chain_reach*blast_chain_reach;
    for(BlastChain& c:blast_chains) {
        if(!c.live) continue;
        if(t-c.last_tick>blast_chain_window && t-c.head_tick>blast_bang_window) {c.live=false;continue;}
        const double dx=c.p[0]-p[0],dy=c.p[1]-p[1],dz=c.p[2]-p[2],d2=dx*dx+dy*dy+dz*dz;
        if(d2<bd) {bd=d2;best=&c;}
    }
    if(best && t-best->head_tick<=(barrel?blast_bang_window-1:blast_together_window)) {
        // Joins the chain: no new voice; the playing one swells and moves.
        BlastChain& c=*best;
        cpu->gpr[4]=0xffffffffu;++c.fed;c.last_tick=t;++blast_merges;
        for(int i=0;i<3;++i) c.p[i]+=(p[i]-c.p[i])*blast_move;
        if(const uint32_t node=blast_find_node(c)) {
            const double v=c.base_vol*(1.0+blast_swell_step*(c.fed-1));
            psx_mod_write_half(node+0x6a,(uint16_t)std::min<double>(v,blast_volume_max));
            for(int i=0;i<3;++i) psx_mod_write_word(node+0x1c+4*i,(uint32_t)(int32_t)std::lround(c.p[i]));
        }
        if(!c.layer && (c.fed>=blast_layer_at || t-c.head_tick>=blast_layer_after)) {
            uint32_t handle=0;
            if(kick_call(cpu,0x8006b73c,{id,pos,0x800,(uint32_t)blast_layer_pitch,0,0},nullptr,&handle) && handle) {
                const uint32_t node=0x80000000u|(handle&0xffffffu);
                if(node>=0x80010000u && node<=0x801fff80u && psx_mod_read_word(node+0x5c)==id) {
                    c.layer=node;c.layer_id=id;++blast_layers;
                    if(blast_layer_gain!=1.0) psx_mod_write_half(node+0x6a,(uint16_t)(psx_mod_read_half(node+0x6a)*blast_layer_gain));
                }
            }
        }
        if(blast_audio_trace()) std::fprintf(stderr,"[TTK blast audio] merge %s id %04x into %04x (blast %d, tick %u, head +%u) at %.0f %.0f %.0f node %08x vol %u layer %08x\n",why,id,c.id,c.fed,t,t-c.head_tick,p[0],p[1],p[2],c.node,c.node?psx_mod_read_half(c.node+0x6a):0u,c.layer);
        return;
    }
    // Plays: a new chain, or a long chain's next barrel bang.
    BlastChain* c=barrel?best:nullptr;
    if(c) {
        ++blast_rebangs;
        if(const uint32_t node=blast_find_node(*c)) {psx_mod_write_half(node+0x6a,(uint16_t)(psx_mod_read_half(node+0x6a)*blast_tail_gain));++blast_ducks;}
        if(c->layer && psx_mod_read_word(c->layer+0x5c)==c->layer_id && psx_mod_read_half(c->layer+8))
            psx_mod_write_half(c->layer+0x6a,(uint16_t)(psx_mod_read_half(c->layer+0x6a)*blast_tail_gain));
    } else {
        ++blast_heads;
        c=&blast_chains[0];
        for(BlastChain& o:blast_chains) {if(!o.live) {c=&o;break;} if(o.last_tick<c->last_tick) c=&o;}
        c->fed=0;
    }
    for(int i=0;i<3;++i) c->p[i]=c->hp[i]=p[i];
    c->id=id;c->node=0;c->base_vol=0;c->layer=0;c->head_tick=t;c->last_tick=t;++c->fed;c->live=true;
    if(blast_audio_trace()) std::fprintf(stderr,"[TTK blast audio] %s %s id %04x tick %u at %.0f %.0f %.0f\n",c==best && best?"rebang":"head",why,id,t,p[0],p[1],p[2]);
}
static bool blast_barrel_object(uint32_t pos) {
    if(pos<0x80000004u || pos>0x801ffff0u) return false;
    const unsigned type=psx_mod_read_half(pos-4+0x2c);
    const uint32_t types=psx_mod_read_word(0x800d2660);
    return types>=0x80000000u && type<2000 && (psx_mod_read_word(types+28*type)&0x10000u);
}
// Entry of 0x8006b73c.
static void blast_audio_positional(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    switch(ra) {
    case 0x80070418u: blast_audio_sound(cpu,"projectile");return;
    case 0x800708e4u: blast_audio_sound(cpu,"pipe bomb or grenade");return;
    case 0x80071044u: blast_audio_sound(cpu,"dynamite");return;
    }
    if(ra>0x80098a88u && ra<0x80098d70u && blast_barrel_object(cpu->gpr[5])) blast_audio_sound(cpu,"barrel",true);
}
// Entry of 0x8006b270.
static void blast_audio_plain(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    switch(ra) {
    case 0x80070858u: blast_audio_sound(cpu,"pipe bomb or grenade (water)");return;
    case 0x80070ff8u: blast_audio_sound(cpu,"dynamite (water)");return;
    }
    if(ra>0x80098a88u && ra<0x80098d70u && blast_barrel_object(cpu->gpr[5])) blast_audio_sound(cpu,"barrel",true);
}
// Backtick console: "chainsound [on|off|default]".
bool blast_audio_console_command(const char* line,void (*say)(const char*),bool& close) {
    close=false;
    if(std::strncmp(line,"chainsound",10) || (line[10] && line[10]!=' ')) return false;
    if(!input_modernized()) {say("Chain sound: Modernized profile only (Vanilla is unchanged)");return true;}
    const char* v=line+10;while(*v==' ')++v;
    if(!std::strcmp(v,"on")) blast_audio_override.store(1);
    else if(!std::strcmp(v,"off")) blast_audio_override.store(0);
    else if(!std::strcmp(v,"default")) blast_audio_override.store(-1);
    else if(*v) {say("Chain sound: chainsound [on|off|default]");return true;}
    char text[160];
    std::snprintf(text,sizeof text,"Chain sound: %s (heads %llu, merged %llu, rumbles %llu, rebangs %llu)",blast_audio_enabled()?"one voice per chain":"every blast its own voice",
        (unsigned long long)blast_heads,(unsigned long long)blast_merges,(unsigned long long)blast_layers,(unsigned long long)blast_rebangs);
    say(text);return true;
}
