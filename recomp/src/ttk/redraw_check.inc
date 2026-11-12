// D23N1 (developer): same-job capture and rebuild check for redraw jobs.
// Included by frame_replay.cpp (worker_draw). Linux workers only.
//
// Purpose. Later redraw jobs of a game frame are to skip the game's code and
// be rebuilt from what the frame's first job recorded (D23N, note 449). This
// check proves that a record holds everything our host code needs: a job is
// recorded while it runs, a forked copy of the worker taken at the job's
// start rebuilds the job from the record without running the game's own code
// at the top level, and the two GP0 streams are compared word for word.
//
// The record (shared memory, written by the job, read by the copy):
//   - every store the game's own code makes at the top level (not inside a
//     plugin callback or guest code a callback dispatches), in order, through
//     the job CPU's write pointers;
//   - every top-level hook entry (psx_mod_function_entry, through the
//     runtime's hook observer): address, CPU state before its callbacks, and
//     the stream positions before and after them;
//   - at the flip: the CPU state and the replay's GPU DMA channel registers.
// The rebuild applies the stores in order (except to I/O registers: the
// job's DMA kicks already reached its stream, and running them again would
// repeat them), calls each hook with its recorded CPU state (the callbacks,
// with any guest code they dispatch, run for real and produce our packets and
// stream items), splices in the stream items the game emitted between hooks
// (its own lists and uploads) from the job's stream, restores the channel
// registers, then runs the chain close and the submit itself, so every packet
// reaches the stream from the rebuilt memory.
//
// A job's picture depends only on its inputs: the draw clock and the lamp
// bake table are held for the whole job (live_env.h, lamp_bake_map.h).
//
// Compared: both streams item by item, except the PGXP precision records
// (kind 4) of the game's own packets (main RAM sources), which come from GTE
// work of game code the rebuild does not run (D23N3 gives them host
// equivalents), and the source annotations such a record switches on when it
// is a stream's first vertex record (counted separately). On a difference: the first differing item, main RAM,
// scratchpad and mod memory compared at the flip, and the first differing
// store after it.
//
// Timing: the composition in the job against the rebuild of it. The rebuild
// runs our hook work alone, so it is an upper bound on a rebuilt job before
// D23N2's re-projection.
//
// DNTTK_N1_CHECK=N checks every Nth gameplay job (0 or unset: off; nothing
// in this file runs). DNTTK_N1_DUMP=path also writes the first differing
// jobs' streams and hook table. Results: "[n1 check]" lines on stderr.
#if defined(__linux__)
namespace n1 {
struct Store {uint32_t addr,value,size;};
struct Hook {uint32_t addr,before,after,stores;CPUState cpu;};
constexpr uint32_t max_stores=4u<<20, max_hooks=12288, max_child_words=1u<<20, max_post=1u<<16;
struct Shared {
    uint32_t nstores,nhooks,prefix,flip_at,overflow,channel[3];
    uint32_t child_words,child_ok,child_overflow,child_done;
    double job_ms,rebuild_ms;
    CPUState flip_cpu;
    Store store[max_stores];
    Hook hook[max_hooks];
    uint32_t child[max_child_words];
    // Diagnostics for a differing job.
    uint8_t ram_at_flip[0x200000];uint32_t pad_at_flip[256];
    uint32_t mod_bytes,mod_diffs,mod_first;uint8_t mod_at_flip[8u<<20];
    uint32_t ram_diffs,ram_diff_addr[32],ram_diff_job[32],ram_diff_rebuilt[32];
    uint32_t npost[2];Store post[2][max_post];
};
Shared* sh=nullptr;
bool logging=false;
int post_log=-1;   // after the flip, every store: 0 the job's, 1 the rebuild's
void (*orig_ww)(uint32_t,uint32_t)=nullptr;
void (*orig_wh)(uint32_t,uint16_t)=nullptr;
void (*orig_wb)(uint32_t,uint8_t)=nullptr;
inline void note(uint32_t a,uint32_t v,uint32_t size) {
    if(post_log>=0 && sh->npost[post_log]<max_post) sh->post[post_log][sh->npost[post_log]++]={a,v,size};
    if(!logging || g_psx_mod_hook_depth) return;
    if(sh->nstores<max_stores) sh->store[sh->nstores++]={a,v,size}; else sh->overflow|=1;
}
void ww(uint32_t a,uint32_t v) {note(a,v,4);orig_ww(a,v);}
void wh(uint32_t a,uint16_t v) {note(a,v,2);orig_wh(a,v);}
void wb(uint32_t a,uint8_t v) {note(a,v,1);orig_wb(a,v);}
void observe(CPUState* cpu,uint32_t addr,int phase) {
    if(!logging) return;
    uint32_t n=0; render_replay_record_words(&n);
    if(phase==0) {
        if(sh->nhooks>=max_hooks) {sh->overflow|=2;return;}
        Hook& h=sh->hook[sh->nhooks];
        h.addr=addr;h.before=n;h.after=n;h.stores=sh->nstores;h.cpu=*cpu;
    } else if(sh->nhooks<max_hooks && sh->hook[sh->nhooks].addr==addr) {
        sh->hook[sh->nhooks++].after=n;
    }
}
int every() {static const int v=[]{const char* t=std::getenv("DNTTK_N1_CHECK");const long n=t?std::strtol(t,nullptr,10):0;return n>0 && n<100000 ? (int)n : 0;}();return v;}
struct Totals {uint64_t exact_pgxp_annotations=0,jobs=0,exact=0,differ=0,child_failed=0,overflow=0,stores=0,hooks=0,depth_resets=0;double job_ms=0,rebuild_ms=0;} totals;
pid_t child=-1; int go_fd=-1;
// D23N2 inventory (DNTTK_N2_DIFF=1): every gameplay job is recorded (no
// copy, no rebuild) and compared with the previous job of the same game
// frame recorded by this worker: which top-level hooks see other CPU state,
// and which of the game's top-level stores differ, by the hook they follow.
int diff_mode() {static const int v=[]{const char* t=std::getenv("DNTTK_N2_DIFF");return t && t[0]=='1' ? 1 : 0;}();return v;}
bool recording_only=false;
uint64_t cur_serial=0; float cur_alpha=0;
struct Record {uint64_t serial=~0ull;float alpha=0;std::vector<Store> stores;std::vector<Hook> hooks;} prev;
struct HookStat {uint64_t seen=0,differ=0;uint32_t gpr=0,gte_data=0,gte_ctrl=0,hilo=0;};
struct StoreStat {uint64_t stores=0,value=0,address=0,length=0;};
struct DiffTotals {uint64_t pairs=0,same_hooks=0,hook_seq=0,hooks_differ=0,hooks_seen=0,stores=0,value=0,address=0,length=0;} dt;
std::unordered_map<uint32_t,HookStat> hook_stats;
std::unordered_map<uint32_t,StoreStat> store_stats;   // by the hook the stores follow (0: before the first)
std::unordered_map<uint32_t,uint64_t> page_value;     // value differences by 4 KB page
void diff_records() {
    const Hook* hb=sh->hook; const uint32_t nh=sh->nhooks;
    ++dt.pairs;
    bool same=nh==prev.hooks.size();
    for(uint32_t i=0;same && i<nh;++i) same=hb[i].addr==prev.hooks[i].addr;
    if(!same) {++dt.hook_seq;return;}
    ++dt.same_hooks;
    auto mask32=[](const uint32_t* a,const uint32_t* b) {uint32_t m=0;for(int r=0;r<32;++r) if(a[r]!=b[r]) m|=1u<<r;return m;};
    for(uint32_t i=0;i<nh;++i) {
        const CPUState& x=hb[i].cpu; const CPUState& y=prev.hooks[i].cpu;
        HookStat& h=hook_stats[hb[i].addr];++h.seen;++dt.hooks_seen;
        const uint32_t g=mask32(x.gpr,y.gpr),d=mask32(x.gte_data,y.gte_data),c=mask32(x.gte_ctrl,y.gte_ctrl),hl=(x.hi!=y.hi?1u:0u)|(x.lo!=y.lo?2u:0u);
        if(g|d|c|hl) {++h.differ;++dt.hooks_differ;h.gpr|=g;h.gte_data|=d;h.gte_ctrl|=c;h.hilo|=hl;}
    }
    for(uint32_t k=0;k<=nh;++k) {
        const uint32_t a0=k?hb[k-1].stores:0,a1=k<nh?hb[k].stores:sh->nstores;
        const uint32_t b0=k?prev.hooks[k-1].stores:0,b1=k<nh?prev.hooks[k].stores:(uint32_t)prev.stores.size();
        StoreStat& st=store_stats[k?hb[k-1].addr:0];
        st.stores+=a1-a0;dt.stores+=a1-a0;
        if(a1-a0!=b1-b0) {++st.length;++dt.length;continue;}
        for(uint32_t j=0;j<a1-a0;++j) {
            const Store& x=sh->store[a0+j]; const Store& y=prev.stores[b0+j];
            if(x.addr!=y.addr || x.size!=y.size) {++st.address;++dt.address;}
            else if(x.value!=y.value) {++st.value;++dt.value;++page_value[x.addr>>12];}
        }
    }
}
void diff_report() {
    std::fprintf(stderr,"[n2 diff] pairs=%llu same_hook_sequence=%llu hook_sequence_differs=%llu hooks_with_other_state=%llu/%llu stores=%llu value_diffs=%llu address_diffs=%llu interval_length_diffs=%llu\n",
        (unsigned long long)dt.pairs,(unsigned long long)dt.same_hooks,(unsigned long long)dt.hook_seq,(unsigned long long)dt.hooks_differ,(unsigned long long)dt.hooks_seen,
        (unsigned long long)dt.stores,(unsigned long long)dt.value,(unsigned long long)dt.address,(unsigned long long)dt.length);
    std::vector<std::pair<uint32_t,HookStat>> hs(hook_stats.begin(),hook_stats.end());
    std::sort(hs.begin(),hs.end(),[](auto& a,auto& b){return a.second.differ>b.second.differ;});
    for(size_t i=0;i<hs.size() && i<14;++i) if(hs[i].second.differ)
        std::fprintf(stderr,"[n2 diff]   hook %08x: other state %llu of %llu, gpr %08x gte data %08x gte ctrl %08x hi/lo %x\n",hs[i].first,
            (unsigned long long)hs[i].second.differ,(unsigned long long)hs[i].second.seen,hs[i].second.gpr,hs[i].second.gte_data,hs[i].second.gte_ctrl,hs[i].second.hilo);
    std::vector<std::pair<uint32_t,StoreStat>> ss(store_stats.begin(),store_stats.end());
    std::sort(ss.begin(),ss.end(),[](auto& a,auto& b){return a.second.value+a.second.address+a.second.length*100>b.second.value+b.second.address+b.second.length*100;});
    for(size_t i=0;i<ss.size() && i<14;++i) if(ss[i].second.value+ss[i].second.address+ss[i].second.length)
        std::fprintf(stderr,"[n2 diff]   stores after %08x: %llu stores, value %llu, address %llu, interval length %llu\n",ss[i].first,
            (unsigned long long)ss[i].second.stores,(unsigned long long)ss[i].second.value,(unsigned long long)ss[i].second.address,(unsigned long long)ss[i].second.length);
    std::vector<std::pair<uint32_t,uint64_t>> pv(page_value.begin(),page_value.end());
    std::sort(pv.begin(),pv.end(),[](auto& a,auto& b){return a.second>b.second;});
    std::fprintf(stderr,"[n2 diff]   value differences by page:");
    for(size_t i=0;i<pv.size() && i<16;++i) std::fprintf(stderr," %08x:%llu",pv[i].first<<12,(unsigned long long)pv[i].second);
    std::fprintf(stderr,"\n");
}
const uint32_t* parent_stream=nullptr;
uint64_t t_begin=0;
double ms_since(uint64_t t) {return (double)(SDL_GetPerformanceCounter()-t)*1000.0/(double)SDL_GetPerformanceFrequency();}
bool io(uint32_t a) {a&=0x1fffffffu;return a>=0x1f801000u && a<0x1f803000u;}

void compare_memory() {
    sh->ram_diffs=0;
    auto diff=[&](uint32_t addr,uint32_t job,uint32_t rebuilt) {
        if(sh->ram_diffs<32) {sh->ram_diff_addr[sh->ram_diffs]=addr;sh->ram_diff_job[sh->ram_diffs]=job;sh->ram_diff_rebuilt[sh->ram_diffs]=rebuilt;}
        ++sh->ram_diffs;
    };
    for(uint32_t a=0;a<0x200000;a+=4) {
        uint32_t x,y;std::memcpy(&x,sh->ram_at_flip+a,4);std::memcpy(&y,g_psx_ram+a,4);
        if(x!=y) diff(0x80000000u|a,x,y);
    }
    for(uint32_t i=0;i<256;++i) {const uint32_t y=psx_mod_read_word(0x1f800000u+4*i);if(y!=sh->pad_at_flip[i]) diff(0x1f800000u+4*i,sh->pad_at_flip[i],y);}
    sh->mod_diffs=0;sh->mod_first=~0u;
    if(sh->mod_bytes && psx_mod_memory_snapshot_bytes()==sh->mod_bytes) {
        std::vector<uint8_t> mine(sh->mod_bytes);psx_mod_memory_snapshot_write(mine.data());
        for(uint32_t a=0;a<sh->mod_bytes;++a) if(mine[a]!=sh->mod_at_flip[a]) {if(!sh->mod_diffs) sh->mod_first=a;++sh->mod_diffs;}
    }
}

// The copy of the worker: rebuild the job from the record.
[[noreturn]] void rebuild(int fd) {
    char c=0;
    if(read(fd,&c,1)!=1 || c!='g') _exit(0);
    logging=false; g_psx_mod_hook_observer=nullptr;
    const uint64_t t0=SDL_GetPerformanceCounter();
    const uint32_t* parent=parent_stream;
    render_replay_record_begin(sh->child,max_child_words);
    render_replay_record_splice(parent,sh->prefix);
    uint32_t s=0,last=sh->prefix;
    auto apply=[&](uint32_t upto) {
        for(;s<upto;++s) {
            const Store& w=sh->store[s];
            if(io(w.addr)) continue;
            if(w.size==4) orig_ww(w.addr,w.value); else if(w.size==2) orig_wh(w.addr,(uint16_t)w.value); else orig_wb(w.addr,(uint8_t)w.value);
        }
    };
    for(uint32_t i=0;i<sh->nhooks;++i) {
        const Hook& h=sh->hook[i];
        apply(h.stores);
        render_replay_record_splice(parent+last,h.before-last);
        CPUState cpu=h.cpu;
        psx_mod_function_entry(&cpu,h.addr);
        last=h.after;
    }
    apply(sh->nstores);
    render_replay_record_splice(parent+last,sh->flip_at-last);
    sh->rebuild_ms=ms_since(t0);
    compare_memory();
    render_replay_channel_set(sh->channel);
    post_log=1;
    bool ok=!g_psx_call_bail;
    if(ok) {
        CPUState tail=sh->flip_cpu;
        tail.pc=0; tail.gpr[31]=sentinel;
        psx_dispatch_call(&tail,chain_close,sentinel);
        ok=tail.pc==0 && !g_psx_call_bail;
        if(ok) {
            tail.pc=0; tail.gpr[31]=sentinel;
            psx_dispatch_call(&tail,submit,sentinel);
            ok=tail.pc==0 && !g_psx_call_bail;
        }
    }
    int overflow=0;
    sh->child_words=render_replay_record_end(&overflow);
    sh->child_overflow=(uint32_t)overflow;
    sh->child_ok=ok?1u:0u;
    __atomic_store_n(&sh->child_done,1u,__ATOMIC_RELEASE);
    _exit(0);
}

// In the job, before the composition runs: fork the copy, start recording.
void begin(CPUState& cpu,const Params& p) {
    child=-1;recording_only=false;
    if((!every() && !diff_mode()) || (p.flags&4)) return;
    static unsigned count=0;
    if(!diff_mode() && ++count%(unsigned)every()) return;
    if(!sh) {
        void* m=mmap(nullptr,sizeof(Shared),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE,-1,0);
        if(m==MAP_FAILED) return;
        sh=(Shared*)m;
    }
    // A job starts at the top level (render_worker.c resets the depth for a
    // worker forked inside a callback); counted in case it ever does not.
    if(g_psx_mod_hook_depth) {++totals.depth_resets;g_psx_mod_hook_depth=0;}
    sh->nstores=sh->nhooks=sh->overflow=0;sh->npost[0]=sh->npost[1]=0;
    sh->child_words=sh->child_ok=sh->child_overflow=sh->child_done=0;sh->job_ms=sh->rebuild_ms=0;
    uint32_t n=0; parent_stream=render_replay_record_words(&n); sh->prefix=n;
    orig_ww=cpu.write_word;orig_wh=cpu.write_half;orig_wb=cpu.write_byte;
    auto unwrap=[&]{cpu.write_word=orig_ww;cpu.write_half=orig_wh;cpu.write_byte=orig_wb;};
    cpu.write_word=ww;cpu.write_half=wh;cpu.write_byte=wb;
    if(diff_mode()) {
        cur_serial=p.serial;cur_alpha=p.alpha;recording_only=true;
        g_psx_mod_hook_observer=observe;logging=true;
        return;
    }
    int fds[2];
    if(pipe(fds)!=0) {unwrap();return;}
    const pid_t pid=fork();
    if(pid==0) {close(fds[1]);rebuild(fds[0]);}
    close(fds[0]);
    if(pid<0) {close(fds[1]);unwrap();return;}
    child=pid;go_fd=fds[1];
    g_psx_mod_hook_observer=observe;logging=true;
    t_begin=SDL_GetPerformanceCounter();
}

// The composition reached the flip: stop recording, let the copy rebuild.
void at_flip(const CPUState& cpu) {
    if(recording_only) {
        recording_only=false;logging=false;g_psx_mod_hook_observer=nullptr;
        if(sh->overflow) {prev.serial=~0ull;return;}
        if(prev.serial==cur_serial && prev.alpha!=cur_alpha) {
            diff_records();
            if(dt.pairs%64==0) diff_report();
        }
        prev.serial=cur_serial;prev.alpha=cur_alpha;
        prev.stores.assign(sh->store,sh->store+sh->nstores);
        prev.hooks.assign(sh->hook,sh->hook+sh->nhooks);
        return;
    }
    if(child<0) return;
    sh->job_ms=ms_since(t_begin);
    logging=false;g_psx_mod_hook_observer=nullptr;
    uint32_t n=0; render_replay_record_words(&n);
    sh->flip_at=n;sh->flip_cpu=cpu;render_replay_channel_get(sh->channel);
    std::memcpy(sh->ram_at_flip,g_psx_ram,0x200000);
    for(uint32_t i=0;i<256;++i) sh->pad_at_flip[i]=psx_mod_read_word(0x1f800000u+4*i);
    sh->mod_bytes=psx_mod_memory_snapshot_bytes();
    if(sh->mod_bytes<=sizeof sh->mod_at_flip) psx_mod_memory_snapshot_write(sh->mod_at_flip); else sh->mod_bytes=0;
    post_log=0;
    const char g=sh->overflow?'x':'g';
    if(write(go_fd,&g,1)!=1) {}
    close(go_fd);go_fd=-1;
}

// Items of a stream (offset, words), without the game packets' PGXP records.
std::vector<std::pair<uint32_t,uint32_t>> items(const uint32_t* s,uint32_t n) {
    std::vector<std::pair<uint32_t,uint32_t>> out;
    for(uint32_t i=0;i<n;) {
        const uint32_t kind=s[i]>>24,count=s[i]&0xffffffu;
        if(i+1+count>n) {out.push_back({i,~0u});break;}
        if(!(kind==4 && count>=1 && (s[i+1]&0xffffffu)<0x200000u)) out.push_back({i,1+count});
        i+=1+count;
    }
    return out;
}
// Source annotations (kind 5) start at a stream's first vertex record. When
// that is a game packet's PGXP record (excluded above: the rebuild has none),
// the rebuilt stream starts them later and runs of GP0 words join. For such a
// job only, also compare without annotations and with runs joined.
bool pgxp_starts_annotations(const uint32_t* s,uint32_t n) {
    for(uint32_t i=0;i<n;i+=1+(s[i]&0xffffffu))
        if((s[i]>>24)==4) return (s[i]&0xffffffu)>=1 && i+1<n && (s[i+1]&0xffffffu)<0x200000u;
    return false;
}
std::vector<uint32_t> unannotated(const uint32_t* s,uint32_t n) {
    std::vector<uint32_t> out;size_t run=SIZE_MAX;
    for(uint32_t i=0;i<n;) {
        const uint32_t kind=s[i]>>24,count=s[i]&0xffffffu;
        if(i+1+count>n) break;
        if(kind==0) {
            if(run==SIZE_MAX) {run=out.size();out.push_back(0);}
            out[run]+=count;out.insert(out.end(),s+i+1,s+i+1+count);
        } else if(kind!=5 && !(kind==4 && count>=1 && (s[i+1]&0xffffffu)<0x200000u)) {
            run=SIZE_MAX;out.insert(out.end(),s+i,s+i+1+count);
        }
        i+=1+count;
    }
    return out;
}
void dump(const char* what,const uint32_t* s,uint32_t n) {
    static const char* path=std::getenv("DNTTK_N1_DUMP");
    if(!path) return;
    char name[600];std::snprintf(name,sizeof name,"%s-%ld-%llu-%s",path,(long)getpid(),(unsigned long long)totals.jobs,what);
    if(FILE* f=std::fopen(name,"wb")) {std::fwrite(s,4,n,f);std::fclose(f);}
}
void report_difference(const uint32_t* ps,uint32_t pn,const std::vector<std::pair<uint32_t,uint32_t>>& a,const std::vector<std::pair<uint32_t,uint32_t>>& b,size_t k) {
    auto show=[](const uint32_t* s,const std::vector<std::pair<uint32_t,uint32_t>>& v,size_t k,char* out,size_t cap) {
        if(k>=v.size()) {std::snprintf(out,cap,"(end)");return;}
        const uint32_t* w=s+v[k].first;size_t at=(size_t)std::snprintf(out,cap,"kind %u words %u:",w[0]>>24,v[k].second-1);
        for(uint32_t j=1;j<v[k].second && j<9 && at<cap;++j) at+=(size_t)std::snprintf(out+at,cap-at," %08x",w[j]);
    };
    char x[256],y[256];show(ps,a,k,x,sizeof x);show(sh->child,b,k,y,sizeof y);
    std::fprintf(stderr,"[n1 check] DIFFER at item %zu of %zu/%zu (stores %u, hooks %u)\n  job:     %s\n  rebuilt: %s\n",
                 k,a.size(),b.size(),sh->nstores,sh->nhooks,x,y);
    std::fprintf(stderr,"[n1 check]   at the flip: RAM and scratchpad words differ %u, mod memory bytes differ %u of %u (first %u)",sh->ram_diffs,sh->mod_diffs,sh->mod_bytes,sh->mod_first);
    for(uint32_t i=0;i<sh->ram_diffs && i<32;++i) std::fprintf(stderr," %08x=%08x/%08x",sh->ram_diff_addr[i],sh->ram_diff_job[i],sh->ram_diff_rebuilt[i]);
    uint32_t j=0;const uint32_t n0=sh->npost[0],n1=sh->npost[1];
    while(j<n0 && j<n1 && !std::memcmp(&sh->post[0][j],&sh->post[1][j],sizeof(Store))) ++j;
    std::fprintf(stderr,"; after it %u/%u stores, first difference at %u",n0,n1,j);
    if(j<n0) std::fprintf(stderr," job %08x=%08x",sh->post[0][j].addr,sh->post[0][j].value);
    if(j<n1) std::fprintf(stderr," rebuilt %08x=%08x",sh->post[1][j].addr,sh->post[1][j].value);
    std::fprintf(stderr,"\n");
    if(totals.differ<=2) {
        dump("job",ps,pn);dump("rebuilt",sh->child,sh->child_words);
        std::vector<uint32_t> t;t.push_back(sh->prefix);t.push_back(sh->flip_at);
        for(uint32_t i=0;i<sh->nhooks;++i) {t.push_back(sh->hook[i].addr);t.push_back(sh->hook[i].before);t.push_back(sh->hook[i].after);t.push_back(sh->hook[i].cpu.gpr[31]);}
        dump("hooks",t.data(),(uint32_t)t.size());
    }
}

// The job finished (after its submit): wait for the copy, compare.
void finish(bool job_ok) {
    if(child<0) return;
    post_log=-1;
    const pid_t pid=child;child=-1;
    if(go_fd>=0) {close(go_fd);go_fd=-1;}
    for(int t=0;t<400 && !__atomic_load_n(&sh->child_done,__ATOMIC_ACQUIRE);++t) {
        int status=0; if(waitpid(pid,&status,WNOHANG)==pid) break;
        usleep(500);
    }
    const bool done=__atomic_load_n(&sh->child_done,__ATOMIC_ACQUIRE)!=0;
    kill(pid,SIGKILL);waitpid(pid,nullptr,0);
    ++totals.jobs;totals.stores+=sh->nstores;totals.hooks+=sh->nhooks;
    if(sh->overflow) {++totals.overflow;return;}
    if(!job_ok) return;
    uint32_t pn=0;const uint32_t* ps=render_replay_record_words(&pn);
    if(!done || !sh->child_ok || sh->child_overflow) {
        ++totals.child_failed;
        if(totals.child_failed<=5) std::fprintf(stderr,"[n1 check] rebuild failed: done=%d ok=%u overflow=%u stores=%u hooks=%u\n",done,sh->child_ok,sh->child_overflow,sh->nstores,sh->nhooks);
    } else {
        totals.job_ms+=sh->job_ms;totals.rebuild_ms+=sh->rebuild_ms;
        const auto a=items(ps,pn),b=items(sh->child,sh->child_words);
        size_t k=0;
        while(k<a.size() && k<b.size() && a[k].second==b[k].second &&
              !std::memcmp(ps+a[k].first,sh->child+b[k].first,4u*a[k].second)) ++k;
        if(k==a.size() && k==b.size()) ++totals.exact;
        else if(pgxp_starts_annotations(ps,pn) && unannotated(ps,pn)==unannotated(sh->child,sh->child_words)) ++totals.exact_pgxp_annotations;
        else {++totals.differ;if(totals.differ<=8) report_difference(ps,pn,a,b,k);}
    }
    if(totals.jobs%16==0) {
        const double compared=(double)std::max<uint64_t>(1,totals.exact+totals.exact_pgxp_annotations+totals.differ);
        std::fprintf(stderr,"[n1 check] jobs=%llu exact=%llu exact_but_pgxp_annotations=%llu differ=%llu rebuild_failed=%llu overflow=%llu stores/job=%llu hooks/job=%llu depth_resets=%llu composition_ms=%.2f rebuild_ms=%.2f\n",
            (unsigned long long)totals.jobs,(unsigned long long)totals.exact,(unsigned long long)totals.exact_pgxp_annotations,(unsigned long long)totals.differ,(unsigned long long)totals.child_failed,
            (unsigned long long)totals.overflow,(unsigned long long)(totals.stores/totals.jobs),(unsigned long long)(totals.hooks/totals.jobs),
            (unsigned long long)totals.depth_resets,totals.job_ms/compared,totals.rebuild_ms/compared);
    }
}
}  // namespace n1
#else
namespace n1 {
inline void begin(CPUState&,const Params&) {}
inline void at_flip(const CPUState&) {}
inline void finish(bool) {}
}
#endif
