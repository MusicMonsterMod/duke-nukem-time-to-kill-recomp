// D49A: the energy beam's impact glow and sparks at the drawn beam's end.
// Included by frame_replay.cpp after the D49 beam code (beam_rows).
//
// The beam updater 0x800733C8, when its line meets something, calls the effect
// spawner 0x8009cd74(0x307, 0x800c5814, beam+0x34, ...) every fourth tick
// (beam +0x4c): the effects system 0x8009c3fc takes a 0x54-byte record from
// the pool *0x800c5894 (count *0x800c5898, at most 64), links it on the active
// list 0x800deb50 (+0 next) and stores type +8 and the position +0x1c..+0x24,
// the beam's real end. Type 7 (0x307 & 0x3f) is the green glow: its draw case
// in 0x8009ca2c (called by the camera render at 0x8002e7e4) queues a billboard
// decal at +0x1c; it lives about four game frames. Its init 0x8009c0fc also
// starts ten sparks through 0x8009bcfc: bounce-engine particles (pool
// *0x800c4570, 0x180 records of 0x80, active list 0x800dd7f0, kind +8
// 0x04002706, position +0x10..+0x18) that fly off and fall for a second or
// more.
//
// D49 moves Duke's view-aimed beams in late first-person redraws by the view
// change since their shot, so on turns the beam's drawn end left the glow and
// sparks where the beam really hit. Here, on the live game just before each
// gameplay frame is published to the redraw workers, every new glow found at
// the end of one of Duke's unlocked view-aimed beams is tagged with that
// beam's shot view, and every new spark born beside a glow tagged in the same
// scan takes the glow's tag. A worker then moves each tagged glow (always) and
// spark (fully while young, fading to its real path) by the same view change
// as the beam, in its own copy only. Gameplay, hits, damage, lights and the
// game's records are untouched; 60 Hz, third person and Vanilla draw no such
// redraws. DNTTK_IMPACT_FOLLOW=0 keeps the world glow (developer A/B);
// "beamfollow off" turns both off.
//
// Bullet wall hits (D49D): the puff (effects 0x12/0x20/0x21/0x1c) and the
// bullet hole (a sprite on list 0x800e1ef8) that a round's wall impact
// 0x8006f1b8 makes at the same point in the same frame are not moved. D49A
// drew new puffs under the crosshair and eased them back, and D49D moved the
// holes with them; both read as the hit sliding across the wall. Like
// EDuke32, smoke and hole now appear together at the real hit (the world
// delay of the high-refresh redraws is what remains; see D10H5).
constexpr uint32_t fx_pool_word=0x800c5894u, fx_count_word=0x800c5898u, fx_active=0x800deb50u, fx_stride=0x54u;
constexpr uint32_t spark_pool_word=0x800c4570u, spark_active=0x800dd7f0u, spark_stride=0x80u, spark_slots=0x180u;
constexpr uint32_t fx_slots=64u, impact_tag=0x44343941u, impact_record=32u;
constexpr uint32_t beam_spark_kind=0x04002706u;
// Guest memory (visible to the workers' copies): +0 the scan counter, then
// one 32-byte tag per effect slot, then one per spark slot: +0 tag, +4 the
// scan that tagged it, +8 shot eye, +0x14 shot forward x4096.
uint32_t impact_tags=0;
void impact_follow_prepare() {
    if(!impact_tags) impact_tags=psx_mod_alloc_guest_memory(16+(fx_slots+spark_slots)*impact_record,16);
}
bool impact_follow_enabled() {
    static const bool on=[]{const char* v=std::getenv("DNTTK_IMPACT_FOLLOW");return !(v && v[0]=='0' && !v[1]);}();
    return on;
}
uint32_t impact_fx_tag(uint32_t slot) {return impact_tags+16+slot*impact_record;}
uint32_t impact_spark_tag(uint32_t slot) {return impact_tags+16+(fx_slots+slot)*impact_record;}

// A record's slot in its pool, or -1.
int pool_slot(uint32_t a,uint32_t pool,uint32_t stride,uint32_t slots) {
    if(!pool || a<pool || a>=pool+stride*slots || (a-pool)%stride) return -1;
    return (int)((a-pool)/stride);
}
struct ImpactSeen {uint32_t scan=0,kind=0;int32_t pos[3]={0,0,0};};
ImpactSeen fx_seen[fx_slots], spark_seen[spark_slots];
uint64_t impact_epoch=~uint64_t(0);
uint64_t impact_glows=0, impact_sparks=0, impact_glow_misses=0;
void impact_tag_write(uint32_t at,uint32_t scan,const double eye[3],const double fwd[3]) {
    psx_mod_write_word(at,impact_tag);psx_mod_write_word(at+4,scan);
    for(int k=0;k<3;++k) {
        psx_mod_write_word(at+8+4*k,(uint32_t)(int32_t)std::lround(eye[k]));
        psx_mod_write_word(at+0x14+4*k,(uint32_t)(int32_t)std::lround(fwd[k]*4096));
    }
}
void impact_tags_scan() {
    if(!impact_tags) return;
    const uint64_t epoch=psx_mod_savestate_loads();
    const uint32_t fx_pool=psx_mod_read_word(fx_pool_word), spark_pool=psx_mod_read_word(spark_pool_word);
    const uint32_t fx_n=std::min<uint32_t>(psx_mod_read_word(fx_count_word),fx_slots);
    const bool reset=epoch!=impact_epoch;
    if(reset) {
        // A load: forget what was seen and drop every tag the state carried.
        impact_epoch=epoch;
        for(auto& s:fx_seen) s={};
        for(auto& s:spark_seen) s={};
        for(uint32_t i=0;i<fx_slots;++i) psx_mod_write_word(impact_fx_tag(i),0);
        for(uint32_t i=0;i<spark_slots;++i) psx_mod_write_word(impact_spark_tag(i),0);
    }
    const uint32_t scan=psx_mod_read_word(impact_tags)+1;
    psx_mod_write_word(impact_tags,scan);
    const bool tagging=!reset && aim_beam_follow_enabled() && impact_follow_enabled();
    // Duke's unlocked view-aimed beams: real end and shot view.
    struct Beam {double end[3],eye[3],fwd[3];};
    Beam beams[100];unsigned nb=0;
    if(tagging) for(uint32_t i=0;i<100;++i) {
        const uint32_t a=0x800da978u+i*0x60u;
        if(!psx_mod_read_half(a) || psx_mod_read_byte(a+0x30)!=15 || psx_mod_read_word(a+0x10)!=duke || psx_mod_read_word(a+0x14)) continue;
        uint32_t rec=0;Beam& b=beams[nb];
        if(!aim_beam_view(a,rec,b.eye,b.fwd)) continue;
        for(int k=0;k<3;++k) b.end[k]=(int32_t)psx_mod_read_word(a+0x34+4*k);
        ++nb;
    }
    // Glows tagged in this scan (sparks born beside them take their view).
    struct Fresh {double pos[3];unsigned beam;};
    Fresh fresh[fx_slots];unsigned nf=0;
    uint32_t a=psx_mod_read_word(fx_active);
    for(unsigned guard=0;a && guard<fx_slots;++guard) {
        const int slot=pool_slot(a,fx_pool,fx_stride,fx_n);
        if(slot<0) break;
        const uint32_t kind=psx_mod_read_word(a+8);
        int32_t pos[3];for(int k=0;k<3;++k) pos[k]=(int32_t)psx_mod_read_word(a+0x1c+4*k);
        ImpactSeen& s=fx_seen[slot];
        const bool same=s.scan==scan-1 && s.kind==kind && pos[0]==s.pos[0] && pos[1]==s.pos[1] && pos[2]==s.pos[2];
        s.scan=scan;s.kind=kind;std::memcpy(s.pos,pos,sizeof pos);
        if(!same && !reset) {
            const uint32_t at=impact_fx_tag((uint32_t)slot);
            psx_mod_write_word(at,0);
            if(tagging && (kind&0x3ffu)==0x307u) {
                unsigned best=nb;double bd=64.0;
                for(unsigned j=0;j<nb;++j) {
                    double d=0;for(int k=0;k<3;++k) d=std::max(d,std::fabs(beams[j].end[k]-pos[k]));
                    if(d<bd) {bd=d;best=j;}
                }
                if(best<nb) {
                    impact_tag_write(at,scan,beams[best].eye,beams[best].fwd);++impact_glows;
                    Fresh& f=fresh[nf++];for(int k=0;k<3;++k) f.pos[k]=pos[k];f.beam=best;
                } else ++impact_glow_misses;
            }
        }
        a=psx_mod_read_word(a);
    }
    a=psx_mod_read_word(spark_active);
    for(unsigned guard=0;a && guard<spark_slots;++guard) {
        const int slot=pool_slot(a,spark_pool,spark_stride,spark_slots);
        if(slot<0) break;
        const uint32_t kind=psx_mod_read_word(a+8);
        int32_t pos[3];for(int k=0;k<3;++k) pos[k]=(int32_t)psx_mod_read_word(a+0x10+4*k);
        ImpactSeen& s=spark_seen[slot];
        double step=0;for(int k=0;k<3;++k) step=std::max(step,std::fabs((double)pos[k]-s.pos[k]));
        const bool same=s.scan==scan-1 && s.kind==kind && step<3000;
        s.scan=scan;s.kind=kind;std::memcpy(s.pos,pos,sizeof pos);
        if(!same && !reset) {
            const uint32_t at=impact_spark_tag((uint32_t)slot);
            psx_mod_write_word(at,0);
            if(tagging && kind==beam_spark_kind) for(unsigned j=0;j<nf;++j) {
                double d=0;for(int k=0;k<3;++k) d=std::max(d,std::fabs(fresh[j].pos[k]-pos[k]));
                if(d<800) {impact_tag_write(at,scan,beams[fresh[j].beam].eye,beams[fresh[j].beam].fwd);++impact_sparks;break;}
            }
        }
        a=psx_mod_read_word(a);
    }
}
// Worker: weight of the move by age in game frames since the tag. Glows live
// about four frames and always follow; sparks follow fully for the first
// frames and then fade back onto their real path.
double impact_weight(uint32_t age,bool spark) {
    if(!spark) return age<=8 ? 1.0 : 0.0;
    static const double hold=[]{const char* t=std::getenv("DNTTK_IMPACT_SPARK_HOLD");return t?std::atof(t):3.0;}();
    static const double fade=[]{const char* t=std::getenv("DNTTK_IMPACT_SPARK_FADE");return t?std::atof(t):5.0;}();
    if(age<=hold) return 1.0;
    if(fade<=0 || age>=hold+fade) return 0.0;
    return 1.0-(age-hold)/fade;
}
uint64_t impact_moved_glows=0, impact_moved_sparks=0;
void impacts_follow_view(const CameraState& c) {
    if(!impact_tags || !impact_follow_enabled()) return;
    double rl[9];
    for(int r=0;r<3;++r) {
        double n=0; for(int k=0;k<3;++k) n+=(double)c.rot[3*r+k]*c.rot[3*r+k];
        n=std::sqrt(n); if(n<1024) return;
        for(int k=0;k<3;++k) rl[3*r+k]=c.rot[3*r+k]/n;
    }
    static const bool trace=std::getenv("DNTTK_BEAM_FOLLOW_TRACE")!=nullptr;
    const uint32_t scan=psx_mod_read_word(impact_tags);
    auto move=[&](uint32_t tag,uint32_t at,bool spark) {
        if(psx_mod_read_word(tag)!=impact_tag) return false;
        const uint32_t age=scan-psx_mod_read_word(tag+4);
        const double w=impact_weight(age,spark);
        if(w<=0) return false;
        double eye[3],fwd[3],rs[9],d[3],v[3];
        for(int k=0;k<3;++k) {eye[k]=(int32_t)psx_mod_read_word(tag+8+4*k);fwd[k]=(int32_t)psx_mod_read_word(tag+0x14+4*k)/4096.0;}
        beam_rows(fwd,rs);
        for(int k=0;k<3;++k) d[k]=(double)(int32_t)psx_mod_read_word(at+4*k)-eye[k];
        for(int r=0;r<3;++r) v[r]=rs[3*r]*d[0]+rs[3*r+1]*d[1]+rs[3*r+2]*d[2];
        for(int k=0;k<3;++k) {
            const double q=(int32_t)psx_mod_read_word(at+4*k);
            const double m=c.pos[k]+rl[k]*v[0]+rl[3+k]*v[1]+rl[6+k]*v[2];
            psx_mod_write_word(at+4*k,(uint32_t)(int32_t)std::lround(q+(m-q)*w));
        }
        if(trace) std::fprintf(stderr,"[TTK impact follow] %s %08x age %u w %.2f\n",spark?"fade":"glow",at,age,w);
        return true;
    };
    const uint32_t fx_pool=psx_mod_read_word(fx_pool_word), spark_pool=psx_mod_read_word(spark_pool_word);
    const uint32_t fx_n=std::min<uint32_t>(psx_mod_read_word(fx_count_word),fx_slots);
    uint32_t a=psx_mod_read_word(fx_active);
    for(unsigned guard=0;a && guard<fx_slots;++guard) {
        const int slot=pool_slot(a,fx_pool,fx_stride,fx_n);
        if(slot<0) break;
        const uint32_t kind=psx_mod_read_word(a+8);
        if((kind&0x3ffu)==0x307u && move(impact_fx_tag((uint32_t)slot),a+0x1c,false)) ++impact_moved_glows;
        a=psx_mod_read_word(a);
    }
    a=psx_mod_read_word(spark_active);
    for(unsigned guard=0;a && guard<spark_slots;++guard) {
        const int slot=pool_slot(a,spark_pool,spark_stride,spark_slots);
        if(slot<0) break;
        if(psx_mod_read_word(a+8)==beam_spark_kind && move(impact_spark_tag((uint32_t)slot),a+0x10,true)) ++impact_moved_sparks;
        a=psx_mod_read_word(a);
    }
}
