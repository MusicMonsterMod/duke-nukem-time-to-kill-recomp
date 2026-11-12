// D23T (developer): proof that the object renderer's shortened pass is exact.
// Included by frame_replay.cpp (worker_draw), after redraw_check.inc. Linux
// workers only. Note 458.
//
// Since unified geometry our host code draws every face of a prop or actor
// joint. When none is left to the game's object renderer (0x80010000) it used
// to get a mesh copy with an empty polygon list and still projected every
// vertex (RTPT, its vertex pass). It now gets only the last two vertex
// triples (near_clip.cpp, object_null_mode). The whole pass leaves behind:
// the GTE data registers (VXY/VZ, IR, SXY, SZ, MAC) and FLAG, the vertex
// buffer at render context +0x10, its register save area below the caller's
// stack pointer, a store of the unchanged packet cursor, and v0, v1, t0-t7.
// The shortened pass leaves all of it identically except the vertex buffer,
// which holds only the two triples. This check proves that nothing reads
// the rest, and that everything else is identical.
//
// For every Nth gameplay job (DNTTK_T_CHECK=N), the worker forks at the job's
// start. The copy (the reference) runs the job with the whole pass (mode 0).
// The job itself runs it in DNTTK_T_CHECK_MODE (default 2: shortened, with the
// part of the buffer it no longer writes poisoned with pseudo-random words
// before each call; 4: a null mesh with every leftover poisoned, which shows
// what reads them). DNTTK_T_CHECK_START=1 also poisons, at the job's start,
// every vertex buffer range seen in earlier checked jobs, so the composition
// is shown not to read what the previous frame left there either. Both sides
// log every top-level store of the game's code (address, value, size, PC) and
// the CPU state at every top-level hook entry, and record their whole GP0
// stream. Compared after the submit:
//   - the streams, item by item, every word (PGXP records included: both
//     sides run the same game code);
//   - the store logs, every store, after removing the stores the vertex pass
//     makes itself (register saves and vertex buffer 0x80010014-0x800101ec,
//     the cursor store 0x80010cd0) from both; differing stores are counted
//     by the PC that made them;
//   - the hook sequence, and the registers at every hook entry;
//   - at the flip: CPU state, main RAM and the scratchpad (inside the vertex
//     buffer ranges the job's object calls used, or anywhere else) and mod
//     memory.
// "exact": stream, store log, hook entries and flip state identical and no
// memory outside the vertex buffer ranges different at the flip. Results:
// "[t check]" lines on stderr every 16 checked jobs per worker.
#if defined(__linux__)
namespace tchk {
struct Store {uint32_t addr,value,size,pc;};
struct Hook {uint32_t addr,stores;CPUState cpu;};
constexpr uint32_t max_stores=1u<<20,max_hooks=8192,max_words=1u<<21,max_ranges=4096;
struct Side {
    uint32_t nstores,nhooks,overflow,words,ok,done,nranges;
    uint64_t nulls;
    CPUState flip;
    uint32_t pad[256];
    uint32_t mod_bytes;
    uint32_t range[max_ranges][2];
    Store store[max_stores];
    Hook hook[max_hooks];
    uint32_t stream[max_words];
    uint8_t ram[0x200000];
    uint8_t mod[8u<<20];
};
Side* side[2]={nullptr,nullptr};   // 0 the reference (the copy), 1 the job
int role=-1;                       // in a checked job: which side this process is
bool logging=false;
pid_t child=-1;
int prev_mode=1;
std::vector<std::pair<uint32_t,uint32_t>> seen;   // vertex buffer ranges seen in earlier checked jobs (merged)
void (*orig_ww)(uint32_t,uint32_t)=nullptr;
void (*orig_wh)(uint32_t,uint16_t)=nullptr;
void (*orig_wb)(uint32_t,uint8_t)=nullptr;
inline void note(uint32_t a,uint32_t v,uint32_t size) {
    if(!logging || g_psx_mod_hook_depth) return;
    Side& s=*side[role];
    if(s.nstores<max_stores) s.store[s.nstores++]={a,v,size,g_debug_last_store_pc}; else s.overflow|=1;
}
void ww(uint32_t a,uint32_t v) {note(a,v,4);orig_ww(a,v);}
void wh(uint32_t a,uint16_t v) {note(a,v,2);orig_wh(a,v);}
void wb(uint32_t a,uint8_t v) {note(a,v,1);orig_wb(a,v);}
void observe(CPUState* cpu,uint32_t addr,int phase) {
    if(!logging || phase!=0) return;
    Side& s=*side[role];
    if(s.nhooks>=max_hooks) {s.overflow|=2;return;}
    s.hook[s.nhooks++]={addr,s.nstores,*cpu};
    // The vertex buffer range this object call's whole pass would write.
    if(addr==0x80010000u && cpu->gpr[4] && s.nranges<max_ranges) {
        const uint32_t buffer=psx_mod_read_word(cpu->gpr[6]+0x10),count=psx_mod_read_byte(cpu->gpr[4]+6);
        s.range[s.nranges][0]=buffer&0x1fffffffu;s.range[s.nranges][1]=(buffer&0x1fffffffu)+(count+2)/3*0x18;++s.nranges;
    }
}
int every() {static const int v=[]{const char* t=std::getenv("DNTTK_T_CHECK");const long n=t?std::strtol(t,nullptr,10):0;return n>0 && n<100000 ? (int)n : 0;}();return v;}
int check_mode() {static const int v=[]{const char* t=std::getenv("DNTTK_T_CHECK_MODE");const int n=t?std::atoi(t):2;return n==4?4:n==3?3:2;}();return v;}
bool start_poison() {static const bool v=[]{const char* t=std::getenv("DNTTK_T_CHECK_START");return t && t[0]=='1';}();return v;}
struct Totals {
    uint64_t jobs=0,exact=0,stream_differ=0,stores_differ=0,hooks_differ=0,memory_other=0,failed=0,overflow=0,nulls=0,pass_stores=0,stores=0;
    uint64_t hook_entries=0,hook_entries_other=0,flip_differ=0,buffer_words=0,other_words=0,mod_differ=0,start_poisoned=0,buffer_tail_words=0,buffer_tail_jobs=0,buffer_tail_shown=0,mod_bytes_max=0,mod_span_max=0;
    uint32_t hook_gpr=0,hook_gte_data=0,hook_gte_ctrl=0,flip_gpr=0,flip_gte=0;
    std::unordered_map<uint32_t,uint64_t> store_pc;   // differing stores by PC
} totals;
bool in_pass(uint32_t pc) {return (pc>=0x80010010u && pc<0x80010200u) || pc==0x80010cd0u;}

void begin(CPUState& cpu,const Params& p) {
    role=-1;
    if(!every() || n1::every() || n1::recording() || (p.flags&4)) return;
    static unsigned count=0;
    if(++count%(unsigned)every()) return;
    for(Side*& s:side) if(!s) {
        void* m=mmap(nullptr,sizeof(Side),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE,-1,0);
        if(m==MAP_FAILED) return;
        s=(Side*)m;
    }
    if(g_psx_mod_hook_depth) g_psx_mod_hook_depth=0;
    for(Side* s:side) {s->nstores=s->nhooks=s->overflow=s->words=s->ok=s->done=s->nranges=0;s->nulls=0;}
    uint32_t n=0; const uint32_t* stream=render_replay_record_words(&n);
    orig_ww=cpu.write_word;orig_wh=cpu.write_half;orig_wb=cpu.write_byte;
    const pid_t pid=fork();
    if(pid<0) return;
    if(pid==0) {
        // The reference: the whole vertex pass, its stream into the shared side.
        role=0;
        near_clip_object_null(0);
        std::vector<uint32_t> head(stream,stream+n);
        render_replay_record_begin(side[0]->stream,max_words);
        render_replay_record_splice(head.data(),n);
    } else {
        role=1;child=pid;
        prev_mode=near_clip_object_null(check_mode());
        if(start_poison() && !seen.empty()) {
            uint32_t x=0x2545f491u^(uint32_t)count;
            for(const auto& [lo,hi]:seen) for(uint32_t a=lo;a<hi;a+=4) {x^=x<<13;x^=x>>17;x^=x<<5;psx_mod_write_word(a<0x200000u?0x80000000u|a:a,x);}
            ++totals.start_poisoned;
        }
    }
    side[role]->nulls=near_clip_object_null_count();
    cpu.write_word=ww;cpu.write_half=wh;cpu.write_byte=wb;
    g_psx_mod_hook_observer=observe;logging=true;
}

void at_flip(const CPUState& cpu) {
    if(role<0) return;
    logging=false;g_psx_mod_hook_observer=nullptr;
    Side& s=*side[role];
    s.nulls=near_clip_object_null_count()-s.nulls;
    s.flip=cpu;
    std::memcpy(s.ram,g_psx_ram,0x200000);
    for(uint32_t i=0;i<256;++i) s.pad[i]=psx_mod_read_word(0x1f800000u+4*i);
    s.mod_bytes=psx_mod_memory_snapshot_bytes();
    if(s.mod_bytes<=sizeof s.mod) psx_mod_memory_snapshot_write(s.mod); else s.mod_bytes=0;
}

std::vector<std::pair<uint32_t,uint32_t>> items(const uint32_t* s,uint32_t n) {
    std::vector<std::pair<uint32_t,uint32_t>> out;
    for(uint32_t i=0;i<n;) {
        const uint32_t count=s[i]&0xffffffu;
        if(i+1+count>n) {out.push_back({i,~0u});break;}
        out.push_back({i,1+count});i+=1+count;
    }
    return out;
}
uint32_t mask32(const uint32_t* a,const uint32_t* b) {uint32_t m=0;for(int r=0;r<32;++r) if(a[r]!=b[r]) m|=1u<<r;return m;}

void compare() {
    const Side& r=*side[0]; const Side& j=*side[1];
    ++totals.jobs;
    if(r.overflow || j.overflow || r.words==0) {++totals.overflow;return;}
    totals.nulls+=j.nulls;
    bool exact=true;
    // Streams: every item, every word.
    uint32_t jn=0;const uint32_t* js=render_replay_record_words(&jn);
    const auto a=items(r.stream,r.words),b=items(js,jn);
    size_t k=0;
    while(k<a.size() && k<b.size() && a[k].second==b[k].second && !std::memcmp(r.stream+a[k].first,js+b[k].first,4u*a[k].second)) ++k;
    if(k!=a.size() || k!=b.size()) {
        exact=false;++totals.stream_differ;
        if(totals.stream_differ<=4) {
            std::fprintf(stderr,"[t check] STREAM DIFFERS at item %zu of %zu/%zu",k,a.size(),b.size());
            for(int s=0;s<2;++s) {
                const auto& v=s?b:a;const uint32_t* w=s?js:r.stream;
                std::fprintf(stderr," %s:",s?"job":"reference");
                if(k<v.size()) for(uint32_t q=0;q<v[k].second && q<8;++q) std::fprintf(stderr," %08x",w[v[k].first+q]);
            }
            std::fprintf(stderr,"\n");
        }
    }
    // Store logs without the pass's own stores: every store compared.
    std::vector<const Store*> rs,jsv;rs.reserve(r.nstores);jsv.reserve(j.nstores);
    for(uint32_t q=0;q<r.nstores;++q) if(!in_pass(r.store[q].pc)) rs.push_back(&r.store[q]);
    for(uint32_t q=0;q<j.nstores;++q) if(!in_pass(j.store[q].pc)) jsv.push_back(&j.store[q]);
    totals.pass_stores+=(r.nstores-rs.size())-(j.nstores-jsv.size());
    totals.stores+=rs.size();
    bool stores_same=rs.size()==jsv.size();
    if(!stores_same && totals.stores_differ<4) std::fprintf(stderr,"[t check] STORE LOG LENGTH: reference %zu, job %zu\n",rs.size(),jsv.size());
    for(size_t q=0;q<rs.size() && q<jsv.size();++q) {
        const Store& x=*rs[q];const Store& y=*jsv[q];
        if(x.addr!=y.addr || x.value!=y.value || x.size!=y.size || x.pc!=y.pc) {
            if(stores_same && totals.stores_differ<4) std::fprintf(stderr,"[t check] STORE DIFFERS at %zu: reference %08x=%08x (%u, pc %08x), job %08x=%08x (%u, pc %08x)\n",
                q,x.addr,x.value,x.size,x.pc,y.addr,y.value,y.size,y.pc);
            // Where the job's differing value came from: the last earlier
            // store of the same value (poison is unique) and its PC.
            if(stores_same && totals.stores_differ<8) {
                for(size_t z=q;z-->0;) if(jsv[z]->value==y.value) {std::fprintf(stderr,"[t check]   job value %08x stored earlier at %08x by pc %08x (%zu stores before)\n",y.value,jsv[z]->addr,jsv[z]->pc,q-z);break;}
                for(size_t z=q;z-->0;) if(jsv[z]->size==2 && jsv[z]->value==(y.value&0xffffu)) {std::fprintf(stderr,"[t check]   job low half %04x stored earlier at %08x by pc %08x (%zu stores before); buffer ranges:",y.value&0xffffu,jsv[z]->addr,jsv[z]->pc,q-z);
                    for(uint32_t u=0;u<j.nranges && u<6;++u) std::fprintf(stderr," %08x-%08x",j.range[u][0],j.range[u][1]);std::fprintf(stderr,"\n");break;}
                for(size_t z=q;z-->0;) if(rs[z]->value==x.value && rs[z]->addr!=x.addr) {std::fprintf(stderr,"[t check]   reference value %08x stored earlier at %08x by pc %08x (%zu stores before)\n",x.value,rs[z]->addr,rs[z]->pc,q-z);break;}
            }
            stores_same=false;++totals.store_pc[x.pc];
        }
    }
    if(!stores_same) {exact=false;++totals.stores_differ;}
    // Hooks: the same sequence and the same registers at every entry.
    bool hooks_same=r.nhooks==j.nhooks;
    for(uint32_t q=0;hooks_same && q<r.nhooks;++q) hooks_same=r.hook[q].addr==j.hook[q].addr;
    if(!hooks_same) {exact=false;++totals.hooks_differ;}
    else {
        bool other=false;
        for(uint32_t q=0;q<r.nhooks;++q) {
            const CPUState& x=r.hook[q].cpu;const CPUState& y=j.hook[q].cpu;
            ++totals.hook_entries;
            const uint32_t g=mask32(x.gpr,y.gpr),d=mask32(x.gte_data,y.gte_data),c=mask32(x.gte_ctrl,y.gte_ctrl);
            if(g|d|c|(x.hi!=y.hi)|(x.lo!=y.lo)) {other=true;++totals.hook_entries_other;totals.hook_gpr|=g;totals.hook_gte_data|=d;totals.hook_gte_ctrl|=c;}
        }
        if(other) {exact=false;++totals.hooks_differ;}
    }
    // At the flip.
    {
        const uint32_t g=mask32(r.flip.gpr,j.flip.gpr),d=mask32(r.flip.gte_data,j.flip.gte_data),c=mask32(r.flip.gte_ctrl,j.flip.gte_ctrl);
        if(g|d|c|(r.flip.hi!=j.flip.hi)|(r.flip.lo!=j.flip.lo)) {exact=false;++totals.flip_differ;totals.flip_gpr|=g;totals.flip_gte|=d|(c?0x80000000u:0);}
    }
    // Memory at the flip, by region.
    std::vector<std::pair<uint32_t,uint32_t>> ranges;
    for(uint32_t q=0;q<r.nranges;++q) ranges.push_back({r.range[q][0],r.range[q][1]});
    std::sort(ranges.begin(),ranges.end());
    for(const auto& rg:ranges) if(rg.second-rg.first<=0x10000u) seen.push_back(rg);
    std::sort(seen.begin(),seen.end());
    {std::vector<std::pair<uint32_t,uint32_t>> m;for(const auto& rg:seen) {if(!m.empty() && rg.first<=m.back().second) m.back().second=std::max(m.back().second,rg.second); else m.push_back(rg);}seen.swap(m);}
    auto inside=[&](uint32_t phys) {
        auto it=std::upper_bound(ranges.begin(),ranges.end(),std::make_pair(phys,~0u));
        while(it!=ranges.begin()) {--it;if(phys<it->second) return true;if(phys-it->first>0x10000u) break;}
        return false;
    };
    uint32_t other=0,shown=0;bool exact_buffer=true;
    auto word=[&](uint32_t addr,uint32_t x,uint32_t y) {
        if(x==y) return;
        if(inside(addr&0x1fffffffu)) {
            // Slots 0-5 hold the shortened pass's own two triples (mode 2).
            const uint32_t phys=addr&0x1fffffffu;bool head=false;
            for(const auto& rg:ranges) if(phys>=rg.first && phys<rg.first+0x30u) {head=true;break;}
            if(head || check_mode()!=2) {++totals.buffer_words;return;}
            ++totals.buffer_tail_words;
            if(totals.buffer_tail_shown++<12) std::fprintf(stderr,"[t check] BUFFER SLOT DIFFERS at the flip: %08x=%08x/%08x\n",addr,x,y);
            exact_buffer=false;return;
        }
        ++other;++totals.other_words;
        if(shown<8 && totals.memory_other<4) {if(!shown) std::fprintf(stderr,"[t check] OTHER MEMORY at the flip:");std::fprintf(stderr," %08x=%08x/%08x",addr,x,y);++shown;}
    };
    for(uint32_t q=0;q<0x200000;q+=4) {uint32_t x,y;std::memcpy(&x,r.ram+q,4);std::memcpy(&y,j.ram+q,4);word(0x80000000u|q,x,y);}
    for(uint32_t q=0;q<256;++q) word(0x1f800000u+4*q,r.pad[q],j.pad[q]);
    if(shown) std::fprintf(stderr,"\n");
    if(other) {exact=false;++totals.memory_other;}
    if(!exact_buffer) {exact=false;++totals.buffer_tail_jobs;}
    if(r.mod_bytes!=j.mod_bytes) ++totals.mod_differ;
    else if(std::memcmp(r.mod,j.mod,r.mod_bytes)) {
        // Expected: the guest copy's header (vertex count byte and vertex
        // pointer of the last shortened object), a few bytes.
        ++totals.mod_differ;uint32_t nb=0,lo=~0u,hi=0;
        for(uint32_t q=0;q<r.mod_bytes;++q) if(r.mod[q]!=j.mod[q]) {++nb;lo=std::min(lo,q);hi=q;}
        totals.mod_bytes_max=std::max<uint64_t>(totals.mod_bytes_max,nb);totals.mod_span_max=std::max<uint64_t>(totals.mod_span_max,hi-lo+1);
    }
    if(exact) ++totals.exact;
}

void finish(bool ok) {
    if(role<0) return;
    if(role==0) {
        int overflow=0;
        side[0]->words=render_replay_record_end(&overflow);
        if(overflow) side[0]->overflow|=4;
        side[0]->ok=ok?1u:0u;
        __atomic_store_n(&side[0]->done,1u,__ATOMIC_RELEASE);
        _exit(0);
    }
    role=-1;
    near_clip_object_null(prev_mode);
    const pid_t pid=child;child=-1;
    for(int t=0;t<4000 && !__atomic_load_n(&side[0]->done,__ATOMIC_ACQUIRE);++t) {
        int status=0;if(waitpid(pid,&status,WNOHANG)==pid) break;
        usleep(500);
    }
    const bool done=__atomic_load_n(&side[0]->done,__ATOMIC_ACQUIRE)!=0;
    kill(pid,SIGKILL);waitpid(pid,nullptr,0);
    if(!done || !side[0]->ok || !ok) {++totals.failed;++totals.jobs;}
    else compare();
    if(totals.jobs%16==0) {
        std::vector<std::pair<uint32_t,uint64_t>> pcs(totals.store_pc.begin(),totals.store_pc.end());
        std::sort(pcs.begin(),pcs.end(),[](auto& x,auto& y){return x.second>y.second;});
        char by_pc[256]="";size_t at=0;
        for(size_t q=0;q<pcs.size() && q<6 && at<sizeof by_pc;++q) at+=(size_t)std::snprintf(by_pc+at,sizeof by_pc-at," %08x:%llu",pcs[q].first,(unsigned long long)pcs[q].second);
        std::fprintf(stderr,"[t check] mode %d jobs=%llu exact=%llu stream_differ=%llu stores_differ=%llu hooks_differ=%llu flip_differ=%llu other_memory=%llu failed=%llu overflow=%llu | shortened=%llu stores=%llu pass_stores_saved=%llu start_poisoned=%llu | differing stores by pc:%s | hook entries %llu, other registers %llu (gpr %08x gte data %08x ctrl %08x) | flip gpr %08x gte %08x | words differ: buffer slots 0-5 %llu, later slots %llu (jobs %llu), other %llu; mod memory differs %llu (at most %llu bytes within %llu)\n",
            check_mode(),(unsigned long long)totals.jobs,(unsigned long long)totals.exact,(unsigned long long)totals.stream_differ,(unsigned long long)totals.stores_differ,
            (unsigned long long)totals.hooks_differ,(unsigned long long)totals.flip_differ,(unsigned long long)totals.memory_other,(unsigned long long)totals.failed,(unsigned long long)totals.overflow,
            (unsigned long long)totals.nulls,(unsigned long long)totals.stores,(unsigned long long)totals.pass_stores,(unsigned long long)totals.start_poisoned,by_pc,
            (unsigned long long)totals.hook_entries,(unsigned long long)totals.hook_entries_other,totals.hook_gpr,totals.hook_gte_data,totals.hook_gte_ctrl,
            totals.flip_gpr,totals.flip_gte,(unsigned long long)totals.buffer_words,(unsigned long long)totals.buffer_tail_words,(unsigned long long)totals.buffer_tail_jobs,(unsigned long long)totals.other_words,(unsigned long long)totals.mod_differ,(unsigned long long)totals.mod_bytes_max,(unsigned long long)totals.mod_span_max);
    }
}
}  // namespace tchk
#else
namespace tchk {
inline void begin(CPUState&,const Params&) {}
inline void at_flip(const CPUState&) {}
inline void finish(bool) {}
}
#endif
