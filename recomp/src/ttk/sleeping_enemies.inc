// D17BF sleeping enemies (Modernized "extended" draw distance; Vanilla and
// "original" are unchanged). Included after enemy_reach.inc. Note 431.
//
// TTK creates spawner enemies only near Duke, up to a per-level cap of live
// enemies, so security monitors and distant views showed empty rooms. As in
// Duke 3D, every spawn point without a live enemy now shows its enemy asleep:
// a stand-in actor in mod memory, drawn by the game's own actor renderer
// 0x80032e78 in the rooms the object pass draws, with no game slot, list
// entry or AI. The spawner rows stay the truth.
//
// - Rows (*0x800de720, 0x30 bytes): a row shows a copy when its type is a
//   skeletal creature (type table *0x800d2660: class byte 6 nonzero, kind
//   byte 8 == 2, flag bit 1 clear) and its flags +0x14 have none of 0x17
//   (spawned, dead, disabled: the bits the spawner tick 0x80099530 skips).
//   Types with flag bit 0 keep their position in the slot table 0x800d2548
//   (row +0x2a, 32 bytes, +0x1c the owner; a removed enemy's place) and the
//   creation 0x80098f3c puts the new enemy there; the copy does the same, and
//   shows only when the slot is free (0x80098eb0's test).
// - Stand-in: built by the creation's own type setup 0x80095504 (type fields,
//   skeleton template copy into +0x40, floor and light +0x1b, bounding box
//   and radius +0x5a), with flag 0x10 during the call so it never joins the
//   global list. Position as the creation's floor rule (0x800952ec), room
//   0x8005d790, facing from the row's quaternion (0x8002abec). Rest pose:
//   the renderer's static joint builder (+0x60 = 0xdc), no look target.
// - Built in the spawner tick (live frames only) and listed in a mod-memory
//   table, so the 120/180 Hz redraw workers (forked, fed a copy of guest
//   memory) draw the same copies. A copy whose row was spawned meanwhile is
//   skipped at draw time, so the real enemy replaces it in the same frame.
// - Drawn at the object pass entry 0x800374cc for each room on the pass's
//   list (0x800d68b0, 12-byte entries, room +8), nearest to Duke first, at
//   most 16 a frame, with that entry as the view rectangle, on a CPU copy
//   with stack, scratchpad and render state restored. +0x5c is the frame
//   counter, so the renderer's once-a-frame routines do not run for it.
// Developer: DNTTK_SLEEPING_ENEMIES=0 turns it off; =dynamic uses the
// look/aim joint builder instead of the static one.
static constexpr uint32_t sleep_actor_bytes=0x2d0,sleep_skeleton_bytes=0x5c0,sleep_block=sleep_actor_bytes+sleep_skeleton_bytes;
static constexpr unsigned sleep_max=48,sleep_draw_max=16;
static uint32_t sleep_table=0;              // guest: count, then {row, stand-in, room, distance} x sleep_max
static uint32_t sleep_pool[sleep_max]={};   // guest stand-in blocks, allocated on first use
struct SleepSlot {uint32_t row=0,type=0;int32_t pos[3]={};uint32_t room=0;int32_t floor=0,base=0,y=0;bool built=false,on_floor=false;};
// Resting height by type, learned from live enemies of that type: ground
// classes stand at floor (actor +0x204, from the type init) + offset, flyers
// (class 7) at their row's height + offset. Median of the last samples.
struct SleepRest {uint32_t type;std::vector<int32_t> samples;};
static std::vector<SleepRest> sleep_rest;
static SleepSlot sleep_slots[sleep_max];
static uint32_t sleep_rows_table=0,sleep_loads=0,sleep_level=0xffffffffu;
static uint64_t sleep_built=0,sleep_build_fail=0,sleep_drawn=0,sleep_frames=0,sleep_handover=0,sleep_alloc_fail=0;
static uint32_t sleep_last_listed=0,sleep_last_drawn=0,sleep_last_eligible=0;
static unsigned sleep_handover_logged=0;
static uint64_t sleep_update_us=0,sleep_draw_us=0,sleep_host_rooms=0;
static int sleep_mode() {
    static const int m=[]{const char* t=std::getenv("DNTTK_SLEEPING_ENEMIES");
        if(t && !std::strcmp(t,"0")) return 0;
        if(t && !std::strcmp(t,"dynamic")) return 2;
        return 1;}();
    return m;
}
static bool sleeping_live() {
    return sleep_mode() && input_modernized() && draw_distance_extended() && !monsters_hidden;
}
// A guest call on a copy of the CPU (a0..a3), stack and scratchpad restored.
static bool sleep_call(const CPUState* cpu,uint32_t address,uint32_t a0,uint32_t a1=0,uint32_t a2=0,uint32_t a3=0,uint32_t* v0=nullptr) {
    const uint32_t top=cpu->gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail) return false;
    static uint32_t stack[1024],scratch[256];
    const uint32_t base=(top-4096)&~3u;
    for(unsigned i=0;i<1024;++i) stack[i]=psx_mod_read_word(base+4*i);
    for(unsigned i=0;i<256;++i) scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    CPUState c=*cpu;c.pc=0;c.gpr[29]=top-0x100;c.gpr[31]=0x800000fc;
    c.gpr[4]=a0;c.gpr[5]=a1;c.gpr[6]=a2;c.gpr[7]=a3;
    guest_busy=true;psx_mod_host_call_begin();psx_dispatch_call(&c,address,0x800000fc);psx_mod_host_call_end();guest_busy=false;
    if(v0) *v0=c.gpr[2];
    for(unsigned i=0;i<1024;++i) if(psx_mod_read_word(base+4*i)!=stack[i]) psx_mod_write_word(base+4*i,stack[i]);
    for(unsigned i=0;i<256;++i) if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i]) psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    return c.pc==0 && !g_psx_call_bail;
}
static bool sleep_ram(uint32_t a,uint32_t n) {return a>=0x80010000u && a<=0x80200000u-n && !(a&3);}
// Where the row's enemy would stand (the creation's own choice).
static bool sleep_row_place(uint32_t row,uint32_t types,int32_t pos[3]) {
    const uint32_t type=psx_mod_read_word(row),tf=psx_mod_read_word(types+28*type);
    for(int k=0;k<3;++k) pos[k]=(int32_t)psx_mod_read_word(row+8+4*k);
    if(!(tf&1)) return true;
    const uint32_t slot=psx_mod_read_half(row+0x2a),slots=psx_mod_read_word(0x800d2548);
    if(!slot || !sleep_ram(slots,32*(slot+1))) return false;
    const uint32_t owner=psx_mod_read_word(slots+32*slot+0x1c);
    if(owner && owner!=slot) return false;   // 0x80098eb0: held by another
    for(int k=0;k<3;++k) pos[k]=(int32_t)psx_mod_read_word(slots+32*slot+4*k);
    return true;
}
static bool sleep_row_eligible(uint32_t row,uint32_t types) {
    const uint32_t type=psx_mod_read_word(row);
    if(!type || type>=1024 || (psx_mod_read_word(row+0x14)&0x17)) return false;
    const uint32_t e=types+28*type;
    return psx_mod_read_byte(e+6) && psx_mod_read_byte(e+8)==2 && !(psx_mod_read_word(e)&2);
}
static int32_t sleep_rest_median(uint32_t type,bool* found) {
    for(auto& r:sleep_rest) if(r.type==type && !r.samples.empty()) {
        std::vector<int32_t> v=r.samples;std::nth_element(v.begin(),v.begin()+v.size()/2,v.end());
        *found=true;return v[v.size()/2];
    }
    *found=false;return 0;
}
static bool sleep_flyer(uint32_t types,uint32_t type) {return psx_mod_read_byte(types+28*type+6)==7;}
static int32_t sleep_rest_y(const SleepSlot& s,uint32_t types,uint32_t a) {
    bool found=false;const int32_t off=sleep_rest_median(s.type,&found);
    if(sleep_flyer(types,s.type)) return s.base+(found?off:0);
    if(!s.on_floor) return s.base;
    // No live sample yet: the creation's floor target (0x800952ec), less the
    // 24 units ground enemies settle by on their first update (type 57, all
    // samples).
    return s.floor+(found?off:24-(int32_t)(int16_t)psx_mod_read_half(a+0x56));
}
static void sleep_rest_sample(uint32_t types,uint32_t row,uint32_t actor) {
    const uint32_t type=psx_mod_read_word(row);
    if((uint32_t)psx_mod_read_half(actor+0x2c)!=type || (int16_t)psx_mod_read_half(actor+0x32)<=0 || (psx_mod_read_word(actor)&6)) return;
    const int32_t y=(int32_t)psx_mod_read_word(actor+8);
    const int32_t off=sleep_flyer(types,type) ? y-(int32_t)psx_mod_read_word(row+0xc) : y-(int32_t)psx_mod_read_word(actor+0x204);
    if(off<-8192 || off>8192) return;
    SleepRest* r=nullptr;
    for(auto& x:sleep_rest) if(x.type==type) {r=&x;break;}
    if(!r) {sleep_rest.push_back({type,{}});r=&sleep_rest.back();}
    if(r->samples.size()>=31) r->samples.erase(r->samples.begin());
    r->samples.push_back(off);
}
static bool sleep_build(const CPUState* cpu,unsigned i,uint32_t row,uint32_t types,const int32_t pos[3]) {
    SleepSlot& s=sleep_slots[i];
    s.built=false;s.row=row;s.type=psx_mod_read_word(row);
    for(int k=0;k<3;++k) s.pos[k]=pos[k];
    if(!sleep_pool[i] && !(sleep_pool[i]=psx_mod_alloc_guest_memory(sleep_block,16))) {++sleep_alloc_fail;return false;}
    // The skeleton template must fit the block (0x80095244 copies count*40+0x44).
    const uint32_t te=types+28*s.type,templates=psx_mod_read_word(0x800ddb14);
    const uint32_t tmpl=sleep_ram(templates,4*1024) ? psx_mod_read_word(templates+4*psx_mod_read_half(te+0x10)) : 0;
    if(!sleep_ram(tmpl,0x44)) {++sleep_build_fail;return false;}
    const uint32_t parts=psx_mod_read_byte(tmpl);
    if(!parts || parts*40+0x44>sleep_skeleton_bytes) {++sleep_build_fail;return false;}
    const uint32_t a=sleep_pool[i];
    for(uint32_t o=0;o<sleep_block;o+=4) psx_mod_write_word(a+o,0);
    psx_mod_write_word(a,0x10);   // never joins the global list in 0x80095504
    for(int k=0;k<3;++k) psx_mod_write_word(a+4+4*k,(uint32_t)pos[k]);
    psx_mod_write_word(a+0x40,a+sleep_actor_bytes);
    uint32_t room=0;
    if(!sleep_call(cpu,0x8005d790,a+4,0xffffffffu,0,0,&room) || (int32_t)room<0 || room>=psx_mod_read_word(0x800da47c)) {++sleep_build_fail;return false;}
    psx_mod_write_byte(a+0x2e,(uint8_t)room);
    psx_mod_write_half(a+0x2c,s.type);
    if(!sleep_call(cpu,0x8002abec,row+0x18,a+0x1c)) {++sleep_build_fail;return false;}
    for(int k=0;k<3;++k) psx_mod_write_half(a+0x24+2*k,psx_mod_read_half(a+0x1c+2*k));
    if(!sleep_call(cpu,0x80095504,s.type,a) || psx_mod_read_word(a+0x40)!=a+sleep_actor_bytes || psx_mod_read_byte(a+0x14)!=2) {++sleep_build_fail;return false;}
    // Floor rule of 0x800952ec: settle onto the floor within 32 units.
    uint32_t out[3]={};const uint32_t buf=a+sleep_actor_bytes+sleep_skeleton_bytes-16;
    uint32_t code=0;
    s.base=(int32_t)psx_mod_read_word(a+8);
    if(sleep_call(cpu,0x8005d358,a+4,buf,buf+4,buf+8,&code) && code!=2) {
        for(int k=0;k<3;++k) out[k]=psx_mod_read_word(buf+4*k);
        s.floor=(int32_t)out[1];s.on_floor=true;
    }
    for(int k=0;k<3;++k) psx_mod_write_word(buf+4*k,0);
    psx_mod_write_byte(a+0x2e,(uint8_t)room);
    // As the creation 0x80096614: pitch and roll cleared, yaw kept.
    psx_mod_write_half(a+0x1e,0);psx_mod_write_half(a+0x20,0);psx_mod_write_half(a+0x26,0);psx_mod_write_half(a+0x28,0);
    s.y=sleep_rest_y(s,types,a);psx_mod_write_word(a+8,(uint32_t)s.y);
    psx_mod_write_word(a,1);
    if(sleep_mode()==1) psx_mod_write_byte(a+0x60,0xdc);
    s.room=room;s.built=true;++sleep_built;
    return true;
}
// Spawner tick (a0 Duke's position): choose the nearest sleeping rows and
// keep their stand-ins current.
static void sleeping_update_body(CPUState* cpu);
static void sleeping_update(CPUState* cpu) {
    if(psx_mod_replay_active()) return;
    const auto t0=std::chrono::steady_clock::now();
    sleeping_update_body(cpu);
    sleep_update_us+=(uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-t0).count();
}
static void sleeping_update_body(CPUState* cpu) {
    sleep_last_eligible=0;
    if(!sleeping_live()) {if(sleep_table) psx_mod_write_word(sleep_table,0);return;}
    if(!sleep_table) sleep_table=psx_mod_alloc_guest_memory(16+16*sleep_max,16);
    if(!sleep_table) {++sleep_alloc_fail;return;}
    psx_mod_write_word(sleep_table,0);
    const uint32_t duke=cpu->gpr[4],records=psx_mod_read_word(0x800de720),count=psx_mod_read_word(0x800c56a0),types=psx_mod_read_word(0x800d2660);
    if(!sleep_ram(duke,16) || !count || count>512 || !sleep_ram(records,count*0x30) || !sleep_ram(types,28*1024)) return;
    const uint32_t level=psx_mod_read_word(0x800be570);
    if(records!=sleep_rows_table || level!=sleep_level || sleep_loads!=psx_mod_savestate_loads()) {
        for(auto& s:sleep_slots) s=SleepSlot{};
        if(records!=sleep_rows_table || level!=sleep_level) sleep_rest.clear();
        sleep_rows_table=records;sleep_level=level;sleep_loads=psx_mod_savestate_loads();
    }
    // Handover: a row with a stand-in that the game has just spawned.
    for(unsigned i=0;i<sleep_max;++i) {
        SleepSlot& s=sleep_slots[i];
        if(!s.built || !(psx_mod_read_word(s.row+0x14)&1)) continue;
        const uint32_t actor=psx_mod_read_word(s.row+4),a=sleep_pool[i];
        if(sleep_ram(actor,0x60) && sleep_handover_logged<64) {
            ++sleep_handover_logged;
            std::fprintf(stderr,"[TTK sleeping] handover row %08x type %u class %u floor %d h56 %d/%d copy %d,%d,%d yaw %u actor %d,%d,%d yaw %u\n",s.row,s.type,psx_mod_read_byte(actor+0x15),s.floor,(int16_t)psx_mod_read_half(a+0x56),(int16_t)psx_mod_read_half(actor+0x56),
                (int32_t)psx_mod_read_word(a+4),(int32_t)psx_mod_read_word(a+8),(int32_t)psx_mod_read_word(a+12),psx_mod_read_half(a+0x1c)&4095,
                (int32_t)psx_mod_read_word(actor+4),(int32_t)psx_mod_read_word(actor+8),(int32_t)psx_mod_read_word(actor+12),psx_mod_read_half(actor+0x1c)&4095);
        }
        s=SleepSlot{};
    }
    // Learn resting heights from live enemies (every 8th tick).
    static unsigned sample_tick=0;
    if(!(++sample_tick&7)) for(uint32_t r=0;r<count;++r) {
        const uint32_t row=records+0x30*r,actor=psx_mod_read_word(row+4);
        if((psx_mod_read_word(row+0x14)&0x17)==1 && sleep_ram(actor,0x2cc)) sleep_rest_sample(types,row,actor);
    }
    for(unsigned i=0;i<sleep_max;++i) {
        SleepSlot& s=sleep_slots[i];
        if(!s.built) continue;
        const int32_t y=sleep_rest_y(s,types,sleep_pool[i]);
        if(y!=s.y) {s.y=y;psx_mod_write_word(sleep_pool[i]+8,(uint32_t)y);}
    }
    struct Cand {uint32_t row;int32_t pos[3];double d;};
    static std::vector<Cand> cand;cand.clear();
    int32_t dk[3];for(int k=0;k<3;++k) dk[k]=(int32_t)psx_mod_read_word(duke+4*k);
    for(uint32_t r=0;r<count;++r) {
        const uint32_t row=records+0x30*r;
        if(!sleep_row_eligible(row,types)) continue;
        Cand c{row,{},0};
        if(!sleep_row_place(row,types,c.pos)) continue;
        for(int k=0;k<3;++k) {const double x=(double)c.pos[k]-dk[k];c.d+=x*x;}
        cand.push_back(c);
    }
    sleep_last_eligible=(uint32_t)cand.size();
    std::sort(cand.begin(),cand.end(),[](const Cand& a,const Cand& b){return a.d<b.d;});
    if(cand.size()>sleep_max) cand.resize(sleep_max);
    // Keep the stand-in of a row still chosen at the same place; free the rest.
    bool keep[sleep_max]={},chosen_has[sleep_max]={};
    for(size_t c=0;c<cand.size();++c)
        for(unsigned i=0;i<sleep_max;++i) {
            const SleepSlot& s=sleep_slots[i];
            if(s.built && s.row==cand[c].row && s.type==psx_mod_read_word(cand[c].row) &&
               s.pos[0]==cand[c].pos[0] && s.pos[1]==cand[c].pos[1] && s.pos[2]==cand[c].pos[2]) {keep[i]=true;chosen_has[c]=true;break;}
        }
    for(size_t c=0;c<cand.size();++c) {
        if(chosen_has[c]) continue;
        for(unsigned i=0;i<sleep_max;++i) if(!keep[i]) {
            keep[i]=true;
            if(sleep_build(cpu,i,cand[c].row,types,cand[c].pos)) chosen_has[c]=true;
            else sleep_slots[i]=SleepSlot{};
            break;
        }
    }
    uint32_t n=0;
    for(unsigned i=0;i<sleep_max;++i) {
        const SleepSlot& s=sleep_slots[i];
        if(!keep[i] || !s.built) {if(!keep[i]) sleep_slots[i]=SleepSlot{};continue;}
        double d=0;for(int k=0;k<3;++k) {const double x=(double)s.pos[k]-dk[k];d+=x*x;}
        const uint32_t e=sleep_table+16+16*n;
        psx_mod_write_word(e,s.row);psx_mod_write_word(e+4,sleep_pool[i]);psx_mod_write_word(e+8,s.room);
        psx_mod_write_word(e+12,(uint32_t)std::min(d/256.0,4.0e9));
        ++n;
    }
    psx_mod_write_word(sleep_table,n);
}
// Object pass entry (a0 the camera): the copies in the pass's rooms.
static void sleeping_draw(CPUState* source) {
    if(!sleep_table || source->gpr[31]!=0x8002e6e4u || !sleeping_live()) return;
    const uint32_t n=psx_mod_read_word(sleep_table);
    if(!n || n>sleep_max) return;
    if(psx_mod_read_word(0x800c0bf8+8)!=0x80032e78u) return;
    const uint32_t top=source->gpr[29];
    if(guest_busy || top<0x801f4000 || top>0x801ffff0 || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    const uint32_t listed=psx_mod_read_word(0x800d68a8),rooms=psx_mod_read_word(0x800da47c);
    if(!listed || listed>64 || !rooms || rooms>256) return;
    uint32_t entry_of[256];for(auto& x:entry_of) x=0;
    for(uint32_t i=0;i<listed;++i) {
        const uint32_t e=0x800d68b0+12*i,room=psx_mod_read_word(e+8);
        if(room<rooms && !entry_of[room]) entry_of[room]=e;
    }
    // Host visibility (Polymorph, extended): its rooms beyond the game's list,
    // for the player's own camera only.
    const bool host_rooms=host_vis_shown_valid && source->gpr[4]==camera && host_vis_shown.size()==rooms;
    struct Pick {uint32_t actor,entry,d;};
    Pick picks[sleep_max];uint32_t np=0;
    for(uint32_t i=0;i<n;++i) {
        const uint32_t e=sleep_table+16+16*i,row=psx_mod_read_word(e),actor=psx_mod_read_word(e+4),room=psx_mod_read_word(e+8);
        if(room>=rooms) continue;
        uint32_t entry=entry_of[room];
        if(!entry && host_rooms && room<host_vis_shown.size() && host_vis_shown[room]) {
            // D17AZ-style: a room the host shows beyond the game's list, with
            // the whole view and the host's guard-banded view test.
            const double rad=(int16_t)psx_mod_read_half(actor+0x5a);
            double lo[3],hi[3];
            for(int k=0;k<3;++k) {const double p=(int32_t)psx_mod_read_word(actor+4+4*k);lo[k]=p-rad;hi[k]=p+rad;}
            if(rad>0 && host_vis_box(host_vis_frame_cam,lo,hi,host_vis_frame_guard)) entry=0x800d2210u;
        }
        if(!entry) continue;
        if(!sleep_ram(row,0x30) || (psx_mod_read_word(row+0x14)&0x17)) {++sleep_handover;continue;}
        picks[np++]={actor,entry,psx_mod_read_word(e+12)};
        if(entry==0x800d2210u) ++sleep_host_rooms;
    }
    sleep_last_listed=listed;sleep_last_drawn=0;++sleep_frames;
    if(!np) return;
    std::sort(picks,picks+np,[](const Pick& a,const Pick& b){return a.d<b.d;});
    if(np>sleep_draw_max) np=sleep_draw_max;
    static uint32_t stack[1024],scratch[256],state[0x88/4];
    const uint32_t base=(top-4096)&~3u;
    for(unsigned i=0;i<1024;++i) stack[i]=psx_mod_read_word(base+4*i);
    for(unsigned i=0;i<256;++i) scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    for(unsigned i=0;i<0x88/4;++i) state[i]=psx_mod_read_word(0x800d67a8+4*i);
    const auto t0=std::chrono::steady_clock::now();
    const uint32_t frame=psx_mod_read_word(0x800bcba4);
    for(uint32_t i=0;i<np;++i) {
        psx_mod_write_word(picks[i].actor+0x5c,frame);
        psx_mod_write_word(picks[i].actor+0x3c,0);
        CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=0x800000fc;
        cpu.gpr[4]=source->gpr[4];cpu.gpr[5]=picks[i].actor;cpu.gpr[6]=picks[i].entry;
        guest_busy=true;psx_mod_host_call_begin();psx_dispatch_call(&cpu,0x80032e78,0x800000fc);psx_mod_host_call_end();guest_busy=false;
        if(cpu.pc!=0 || g_psx_call_bail) break;
        ++sleep_last_drawn;
    }
    for(unsigned i=0;i<1024;++i) if(psx_mod_read_word(base+4*i)!=stack[i]) psx_mod_write_word(base+4*i,stack[i]);
    for(unsigned i=0;i<256;++i) if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i]) psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    for(unsigned i=0;i<0x88/4;++i) psx_mod_write_word(0x800d67a8+4*i,state[i]);
    sleep_drawn+=sleep_last_drawn;
    if(!psx_mod_replay_active()) sleep_draw_us+=(uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-t0).count();
}
static const char* sleeping_debug_json() {
    static char buffer[400];
    std::snprintf(buffer,sizeof buffer,"{\"live\":%s,\"table\":%u,\"eligible\":%u,\"built\":%llu,\"build_fail\":%llu,\"alloc_fail\":%llu,"
        "\"frames\":%llu,\"drawn\":%llu,\"last_drawn\":%u,\"listed\":%u,\"handover\":%llu,\"update_us\":%llu,\"draw_us\":%llu,\"host_rooms\":%llu}",
        sleeping_live()?"true":"false",sleep_table?psx_mod_read_word(sleep_table):0u,sleep_last_eligible,
        (unsigned long long)sleep_built,(unsigned long long)sleep_build_fail,(unsigned long long)sleep_alloc_fail,
        (unsigned long long)sleep_frames,(unsigned long long)sleep_drawn,sleep_last_drawn,sleep_last_listed,(unsigned long long)sleep_handover,(unsigned long long)sleep_update_us,(unsigned long long)sleep_draw_us,(unsigned long long)sleep_host_rooms);
    return buffer;
}
