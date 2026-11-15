// D23N2 (developer): pieces of a later redraw job rebuilt in host code, each
// checked word for word against what the game's own code produced in the
// same job. Included by redraw_check.inc inside namespace n1 (it uses the
// D23N1 record: the job's top-level stores with their PCs and its top-level
// hook entries). Linux workers only; nothing here runs unless
// DNTTK_N2_CHECK is set.
//
// DNTTK_N2_CHECK=mask records every gameplay job (as DNTTK_N2_DIFF does) and
// runs the checks named by the mask:
//   1  the object transform 0x800292a0 (piece 1, note 456 section 2): the host
//      port below, from the call's inputs, against the routine's own stores
//      and the GTE rotation and translation the next hook receives.
//   2  the transform's inputs in a later job of the frame (piece 1b): each
//      call's camera and matrix predicted from the previous job of the same
//      frame (same call order) and the later job's own start state: its
//      memory after the job's camera and pose patches, and its transform
//      substitutions (the same object and call index).
//   8  actor joints, first person (D23N2B piece 4a, D23Y7 session): Duke's
//      joint loop (0x800348d8, the transform call with ra 0x80034c6c) passes
//      a stack copy of the joint's table entry scaled by ScaleMatrix
//      0x800b4b5c (scale the half at 0x800c0bf4). The copy as the transform
//      entry finds it before our callbacks, against a host port of
//      ScaleMatrix applied to the table entry and scale as at the job's start;
//      then what our own callbacks did to it before the routine reads it
//      (left alone, changed in place, or another matrix passed).
//  16  actor joints from the job start (D23N2B piece 4b): a copy of the
//      worker forked at the job's start (its memory as at the start, before
//      any of the game's stores) runs every top-level hook of the job with
//      the CPU state the job recorded, without applying the game's stores;
//      each matrix our joint callbacks pass for Duke's joints (the transform
//      0x800292a0 from the joint loop, ra 0x80034c6c) is compared word for
//      word with the job's (the address passed and its 8 words); the copy
//      first takes the eye piece 2 predicts (bit 8 runs it). Every 4th
//      gameplay job; by the kind of matrix the job passed (the table entry,
//      our mod memory, the camera block).
//  64  with 16: every call of the transform, by caller (piece 4d): the
//      matrix the call passes, from the job start, against the job's.
// 256  with 16 and 64: the actor joint table rebuilt in the copy before the
//      actor draw's 0x80032bd8 hook (D23N2B piece 6, rebuild_table below).
// 512  with 16 and 64: the stack matrices of actors, props, 0x8003133c and the
//      sky built in host code in the copy before each transform (piece 7).
// 1024 a whole later job rebuilt from its start and compared hook by hook
//      (piece 8, namespace wj; DNTTK_N2_WHOLE selects the stores and CPU state).
// 2048 with 1024: the transform's scratchpad stores and GTE results from the
//      host port (piece 1) after each transform hook.
// 128  the sky's three camera-built matrices built in host code (D23N2B
//      piece 5, namespace sk), checked at each transform call's entry.
//  32  with 16: the hooks' CPU state taken from the earlier job of the same
//      frame instead of the later job's own (D23N2B piece 4c): jobs whose
//      hook list differs from the earlier job's, call for call, are counted
//      and skipped.
//   4  rooms and visibility (pieces 2 and 3): the game's portal walk
//      0x8006276c run at the later job's start on a scratch CPU (its writes
//      undone afterwards) with the predicted camera, and the room selection
//      inputs of our host walk (camera, source room, clip rectangle, GTE
//      projection, and the view rectangle the earlier job had at the call),
//      against what the job really had at the room walk entry.
// Results: "[n2 check]" lines on stderr.
uint64_t n2_serial=0; float n2_alpha=0;   // the job being recorded (its frame and fraction)
int n2_mask() {static const int v=[]{const char* t=std::getenv("DNTTK_N2_CHECK");return t?(int)std::strtol(t,nullptr,0):0;}();return v;}

namespace xf {
// 0x800292a0(camera a0, matrix a1), CONFIRMED by its disassembly:
//   d = matrix.t (+0x14) - camera.eye (+0x14), stored at 0x1f800320; when a
//     component is 32768 or more from the eye (toward zero), all three are
//     halved (toward zero) and stored again;
//   ApplyMatrixLV 0x800b444c(camera +0x20, d, 0x1f800108): each component split
//     into v / 32768 and v % 32768 (toward zero), MVMVA rotation x IR with sf
//     0 on the high parts and sf 1 on the low parts, result low + (high << 3);
//   halved: the result doubled in place;
//   the result to 0x1f800148 +0x14 (translation);
//   0x80010d60(matrix, 0x1f8001e8): the 3x3 transposed (the pad half kept);
//   MulMatrix0 0x800b41dc(camera +0x20, 0x1f8001e8, 0x1f800148): MVMVA sf 1,
//     IR saturated to 16 bits, column by column: out = view x matrix^T;
//   SetRotMatrix / SetTransMatrix 0x800b4c9c / 0x800b4ccc from 0x1f800148;
//   returns the translation's z.
struct Result {
    uint32_t d[3];    // 0x1f800320 (final)
    uint32_t lv[3];   // 0x1f800108 (final)
    uint32_t tr[5];   // 0x1f8001e8 (transposed matrix)
    uint32_t out[8];  // 0x1f800148: matrix words 0-4, translation 5-7
    int32_t z;        // the return value
};
constexpr uint32_t a_d=0x1f800320u, a_lv=0x1f800108u, a_tr=0x1f8001e8u, a_out=0x1f800148u;
inline int16_t sat16(int64_t v) {return (int16_t)std::clamp<int64_t>(v,-0x8000,0x7fff);}
// Pure: view rows v[9], eye e[3], matrix halves m[10] (9 + pad), translation t[3].
Result compute(const int16_t v[9],const int32_t e[3],const uint16_t m[10],const int32_t t[3]) {
    Result r{};
    int32_t d[3];
    bool over=false;
    for(int i=0;i<3;++i) {
        d[i]=(int32_t)((uint32_t)t[i]-(uint32_t)e[i]);
        const int32_t q=(int32_t)((uint32_t)d[i]+(d[i]<0?0x7fffu:0u))>>15;
        over|=q!=0;
    }
    if(over) for(int i=0;i<3;++i) d[i]=(int32_t)((uint32_t)d[i]+((uint32_t)d[i]>>31))>>1;
    for(int i=0;i<3;++i) r.d[i]=(uint32_t)d[i];
    int32_t hi[3],lo[3];
    for(int i=0;i<3;++i) {
        if(d[i]<0) {const uint32_t u=0u-(uint32_t)d[i];hi[i]=-(int32_t)(u>>15);lo[i]=-(int32_t)(u&0x7fffu);}
        else {hi[i]=d[i]>>15;lo[i]=d[i]&0x7fff;}
    }
    for(int row=0;row<3;++row) {
        int64_t sh=0,sl=0;
        for(int k=0;k<3;++k) {sh+=(int64_t)v[3*row+k]*hi[k];sl+=(int64_t)v[3*row+k]*lo[k];}
        uint32_t x=(uint32_t)(int32_t)(sl>>12)+((uint32_t)(int32_t)sh<<3);
        if(over) x<<=1;
        r.lv[row]=x;
    }
    r.tr[0]=m[0]|(uint32_t)m[3]<<16; r.tr[1]=m[6]|(uint32_t)m[1]<<16; r.tr[2]=m[4]|(uint32_t)m[7]<<16;
    r.tr[3]=m[2]|(uint32_t)m[5]<<16; r.tr[4]=m[8]|(uint32_t)m[9]<<16;
    int16_t o[9];
    for(int row=0;row<3;++row) for(int col=0;col<3;++col) {
        int64_t s=0;
        for(int k=0;k<3;++k) s+=(int64_t)v[3*row+k]*(int16_t)m[3*col+k];
        o[3*row+col]=sat16(s>>12);
    }
    auto pk=[](int16_t a,int16_t b){return (uint16_t)a|(uint32_t)(uint16_t)b<<16;};
    r.out[0]=pk(o[0],o[1]); r.out[1]=pk(o[2],o[3]); r.out[2]=pk(o[4],o[5]); r.out[3]=pk(o[6],o[7]);
    r.out[4]=(uint32_t)(int32_t)o[8];
    for(int i=0;i<3;++i) r.out[5+i]=r.lv[i];
    r.z=(int32_t)r.lv[2];
    return r;
}
Result from_memory(uint32_t cam,uint32_t mat) {
    int16_t v[9]; int32_t e[3],t[3]; uint16_t m[10];
    for(int k=0;k<9;++k) v[k]=(int16_t)psx_mod_read_half(cam+0x20+2*k);
    for(int k=0;k<3;++k) {e[k]=(int32_t)psx_mod_read_word(cam+0x14+4*k);t[k]=(int32_t)psx_mod_read_word(mat+0x14+4*k);}
    for(int k=0;k<10;++k) m[k]=psx_mod_read_half(mat+2*k);
    return compute(v,e,m,t);
}
// The routine and the leaves it calls: stores made from these PCs belong to it.
bool own_pc(uint32_t pc) {
    return (pc>=0x800292a0u && pc<0x80029438u) || (pc>=0x80010d60u && pc<0x80010dc0u) ||
           (pc>=0x800b444cu && pc<0x800b45acu) || (pc>=0x800b41dcu && pc<0x800b42e8u);
}
struct Stats {uint64_t calls=0,exact=0,differ=0,missing=0,gte_checked=0,gte_exact=0,halved=0,substituted=0;uint32_t field_differ[5]={};} st;
bool pending=false; uint32_t pend_from=0; Result pend{}; uint32_t pend_cam=0,pend_mat=0,pend_ra=0;
void report_one(const char* what,uint32_t addr,uint32_t want,uint32_t got) {
    static unsigned shown=0;
    if(shown++<24) std::fprintf(stderr,"[n2 check] transform %s %08x: host %08x game %08x (camera %08x matrix %08x ra %08x)\n",what,addr,want,got,pend_cam,pend_mat,pend_ra);
}
// Check the pending call against the stores logged since its entry.
void settle(const CPUState* next_cpu,uint32_t next_addr) {
    if(!pending) return;
    pending=false;
    std::unordered_map<uint32_t,uint32_t> last;
    for(uint32_t i=pend_from;i<sh->nstores;++i) {
        const Store& s=sh->store[i];
        if(s.size==4 && own_pc(s.pc)) last[s.addr]=s.value;
    }
    bool ok=true,miss=false;
    auto check=[&](int field,uint32_t base,const uint32_t* want,int n) {
        for(int k=0;k<n;++k) {
            const auto it=last.find(base+4u*k);
            if(it==last.end()) {miss=true;report_one("missing",base+4u*k,want[k],0);continue;}
            if(it->second!=want[k]) {ok=false;++st.field_differ[field];report_one("differs",base+4u*k,want[k],it->second);}
        }
    };
    check(0,a_d,pend.d,3); check(1,a_lv,pend.lv,3); check(2,a_tr,pend.tr,5); check(3,a_out,pend.out,8);
    if(miss) ++st.missing; else if(ok) ++st.exact; else ++st.differ;
    // The object renderer right after: its GTE rotation and translation.
    if(next_cpu && next_addr==0x80010000u) {
        ++st.gte_checked;
        bool g=true;
        for(int k=0;k<4;++k) g&=next_cpu->gte_ctrl[k]==pend.out[k];
        g&=(int16_t)next_cpu->gte_ctrl[4]==(int16_t)pend.out[4];
        for(int k=0;k<3;++k) g&=next_cpu->gte_ctrl[5+k]==pend.out[5+k];
        if(g) ++st.gte_exact; else {++st.field_differ[4];report_one("gte differs",0,pend.out[0],next_cpu->gte_ctrl[0]);}
    }
}
void on_entry_done(CPUState* cpu) {
    ++st.calls;
    pend_cam=cpu->gpr[4]; pend_mat=cpu->gpr[5]; pend_ra=cpu->gpr[31];
    pend=from_memory(pend_cam,pend_mat);
    if(pend.d[0]!=(uint32_t)((int32_t)psx_mod_read_word(pend_mat+0x14)-(int32_t)psx_mod_read_word(pend_cam+0x14))) ++st.halved;
    pend_from=sh->nstores; pending=true;
}
void report() {
    std::fprintf(stderr,"[n2 check %ld] transform calls=%llu exact=%llu differ=%llu missing=%llu halved=%llu next-hook GTE checked=%llu exact=%llu (differing: d %u lv %u transposed %u out %u gte %u)\n",
        (long)getpid(),(unsigned long long)st.calls,(unsigned long long)st.exact,(unsigned long long)st.differ,(unsigned long long)st.missing,(unsigned long long)st.halved,
        (unsigned long long)st.gte_checked,(unsigned long long)st.gte_exact,st.field_differ[0],st.field_differ[1],st.field_differ[2],st.field_differ[3],st.field_differ[4]);
}
}  // namespace xf

namespace in {
// One transform call's inputs, as the routine reads them (after our own
// callbacks, which may substitute the matrix).
struct Call {
    uint32_t ra,cam,mat,obj; uint16_t seq; uint8_t sub,pad;
    int16_t v[9]; int32_t e[3]; uint16_t m[10]; int32_t t[3];
    bool same_camera(const Call& o) const {return !std::memcmp(v,o.v,sizeof v) && !std::memcmp(e,o.e,sizeof e);}
    bool same_matrix(const Call& o) const {return !std::memcmp(m,o.m,sizeof m) && !std::memcmp(t,o.t,sizeof t);}
};
std::vector<Call> cur,prev; uint64_t prev_serial=~0ull; float prev_alpha=0;
// The later job's start state: main RAM and the scratchpad after its patches.
std::vector<uint8_t> start_ram; uint32_t start_pad[256];
bool in_ram(uint32_t a,uint32_t n) {a&=0x1fffffffu;return a+n<=0x200000u;}
uint32_t start_word(uint32_t a) {
    if((a&0x1fffffffu)>=0x1f800000u && (a&0x1fffffffu)<0x1f800400u) return start_pad[((a&0x3ffu)>>2)];
    uint32_t w;std::memcpy(&w,start_ram.data()+(a&0x1ffffcu),4);return w;
}
uint16_t start_half(uint32_t a) {return (uint16_t)(start_word(a&~3u)>>(8*(a&2)));}
bool pad_addr(uint32_t a) {return (a&0x1fffffffu)>=0x1f800000u && (a&0x1fffffffu)<0x1f800400u;}
// Piece 1c: the camera a later job's calls see, predicted at its start: the
// view rotation as patched from the job's parameters (or, for Duke's own
// draws in a late first-person redraw, the body camera's, which our host
// code switches in per draw), and the eye the composition derives from the
// anchor, distance and rotation through the game's own 0x80039c7c (run here
// on a scratch CPU with a private stack; it is a pure leaf of the camera
// block).
int32_t pred_eye[3]; int16_t pred_view[2][9]; bool pred_body=false, pred_ok=false;
void predict_camera(const CPUState& job) {
    pred_ok=false;
    const uint32_t mem=psx_mod_alloc_guest_memory(0x800,16);
    if(!mem) return;
    CPUState c=job;
    c.pc=0; c.gpr[31]=sentinel; c.gpr[29]=mem+0x7f0; c.gpr[4]=camera_base; c.gpr[5]=mem;
    psx_dispatch_call(&c,0x80039c7c,sentinel);
    if(c.pc!=0 || g_psx_call_bail) {g_psx_call_bail=0;return;}
    for(int k=0;k<3;++k) pred_eye[k]=(int32_t)psx_mod_read_word(mem+4*k);
    for(int k=0;k<9;++k) pred_view[0][k]=(int16_t)psx_mod_read_half(camera_base+0x20+2*k);
    pred_body=body_camera.valid;
    if(pred_body) std::memcpy(pred_view[1],body_camera.view[1],sizeof pred_view[1]);
    pred_ok=true;
}
uint64_t cam_pred_world=0,cam_pred_body=0,cam_pred_miss=0,cam_pred_other_block=0;
// Actor joints (0x800348d8, ra 0x80034c6c): the matrix is the actor's table
// entry (+0x3c, s4, plus s6), passed directly or as a scaled stack copy.
uint64_t joint_calls=0,joint_direct=0,joint_copy=0,joint_entry_as_start=0,joint_direct_as_start=0,joint_copy_rot_as_start=0,joint_copy_t_as_start=0;
std::unordered_map<uint32_t,uint64_t> cam_miss_ra;
void begin(const CPUState& job) {
    cur.clear();
    predict_camera(job);
    if(start_ram.size()!=0x200000) start_ram.resize(0x200000);
    std::memcpy(start_ram.data(),g_psx_ram,0x200000);
    for(uint32_t i=0;i<256;++i) start_pad[i]=psx_mod_read_word(0x1f800000u+4*i);
}
void on_entry_done(CPUState* cpu) {
    Call c{};
    c.ra=cpu->gpr[31];c.cam=cpu->gpr[4];c.mat=cpu->gpr[5];c.obj=xf_obj;c.seq=(uint16_t)(xf_seq-1);
    c.sub=scratch && c.mat>=scratch && c.mat<scratch+0x20u*scratch_n;
    for(int k=0;k<9;++k) c.v[k]=(int16_t)psx_mod_read_half(c.cam+0x20+2*k);
    for(int k=0;k<3;++k) {c.e[k]=(int32_t)psx_mod_read_word(c.cam+0x14+4*k);c.t[k]=(int32_t)psx_mod_read_word(c.mat+0x14+4*k);}
    for(int k=0;k<10;++k) c.m[k]=psx_mod_read_half(c.mat+2*k);
    cur.push_back(c);
    if(c.ra==0x80034c6cu) {
        ++joint_calls;
        const uint32_t tab=psx_mod_read_word(cpu->gpr[20]+0x3c)+cpu->gpr[22];
        bool entry_start=in_ram(tab,0x20);
        for(int k=0;k<8 && entry_start;++k) entry_start=start_word(tab+4*k)==psx_mod_read_word(tab+4*k);
        joint_entry_as_start+=entry_start;
        if(c.mat==tab) {++joint_direct;joint_direct_as_start+=entry_start;}
        else {
            ++joint_copy;
            bool t_ok=true;
            for(int k=0;k<3;++k) t_ok&=(int32_t)start_word(tab+0x14+4*k)==c.t[k];
            joint_copy_t_as_start+=t_ok;
        }
    }
    if(c.cam!=camera_base) ++cam_pred_other_block;
    else if(pred_ok && !std::memcmp(c.e,pred_eye,sizeof c.e)) {
        if(!std::memcmp(c.v,pred_view[0],sizeof c.v)) ++cam_pred_world;
        else if(pred_body && !std::memcmp(c.v,pred_view[1],sizeof c.v)) ++cam_pred_body;
        else {++cam_pred_miss;++cam_miss_ra[c.ra];}
    } else {++cam_pred_miss;++cam_miss_ra[c.ra];}
}
struct Stats {uint64_t pairs=0,same_count=0,calls=0,cam_start=0,cam_built=0,cam_same_a=0,
    mat_same_a=0,mat_start=0,mat_sub=0,mat_sub_ok=0,mat_built=0,mat_built_same_a=0,predicted=0;} st;
std::unordered_map<uint32_t,uint64_t> cam_built_ra,mat_built_ra,cam_ptr;
uint64_t cam_first=0,cam_first_n=0,cam_eye_only=0,cam_view_only=0,cam_both=0;
// Inputs predicted for a later job's call k from the earlier job's call k.
void compare(float alpha) {
    ++st.pairs;
    if(cur.size()!=prev.size()) return;
    ++st.same_count;
    for(size_t k=0;k<cur.size();++k) {
        const Call& a=prev[k]; const Call& b=cur[k];
        ++st.calls;
        st.cam_same_a+=b.same_camera(a);
        // Camera: the later job's camera block at its start (patched from its
        // parameters), unless the composition derives it.
        bool cam_ok=in_ram(b.cam,0x40) || pad_addr(b.cam);
        if(cam_ok) {
            for(int i=0;i<9 && cam_ok;++i) cam_ok=(int16_t)start_half(b.cam+0x20+2*i)==b.v[i];
            for(int i=0;i<3 && cam_ok;++i) cam_ok=(int32_t)start_word(b.cam+0x14+4*i)==b.e[i];
        }
        if(cam_ok) ++st.cam_start; else {
            ++st.cam_built;++cam_built_ra[b.ra];++cam_ptr[b.cam];
            bool ev=true,vv=true;
            for(int i=0;i<9;++i) vv&=(int16_t)start_half(b.cam+0x20+2*i)==b.v[i];
            for(int i=0;i<3;++i) ev&=(int32_t)start_word(b.cam+0x14+4*i)==b.e[i];
            if(vv && !ev) ++cam_eye_only; else if(ev && !vv) ++cam_view_only; else ++cam_both;
        }
        ++cam_first_n; cam_first+=b.same_camera(cur[0]);
        // Matrix: a substitution of the same object and call index; else the
        // matrix the later job held at its start.
        bool mat_ok=false;
        st.mat_same_a+=b.same_matrix(a);
        if(a.sub) {
            ++st.mat_sub;
            Frame::Xf x{}; bool found=false;
            for(uint32_t i=0;i<sub_n && !found;++i) {std::memcpy(&x,sub_table+i*sizeof(Frame::Xf),sizeof x);found=x.obj==a.obj && x.seq==a.seq;}
            if(found) {
                mat_ok=true;
                for(int i=0;i<9;++i) mat_ok&=x.m.m[i]==(int16_t)b.m[i];
                for(int i=0;i<3;++i) mat_ok&=x.m.t[i]==b.t[i];
            }
            st.mat_sub_ok+=mat_ok;
        } else if(in_ram(a.mat,0x20) || pad_addr(a.mat)) {
            mat_ok=true;
            for(int i=0;i<10 && mat_ok;++i) mat_ok=start_half(a.mat+2*i)==b.m[i];
            for(int i=0;i<3 && mat_ok;++i) mat_ok=(int32_t)start_word(a.mat+0x14+4*i)==b.t[i];
            if(mat_ok) ++st.mat_start;
        }
        if(!mat_ok && !a.sub) {++st.mat_built;st.mat_built_same_a+=b.same_matrix(a);++mat_built_ra[b.ra];}
        st.predicted+=cam_ok && mat_ok;
    }
    (void)alpha;
}
void report() {
    std::fprintf(stderr,"[n2 check %ld] inputs: pairs=%llu same_call_count=%llu calls=%llu predicted=%llu | camera: as at the job start %llu, derived in the job %llu, same as the earlier job %llu | matrix: substituted %llu (predicted %llu), as at the job start %llu, built in the job %llu (of which same as the earlier job %llu), same as the earlier job %llu\n",
        (long)getpid(),(unsigned long long)st.pairs,(unsigned long long)st.same_count,(unsigned long long)st.calls,(unsigned long long)st.predicted,
        (unsigned long long)st.cam_start,(unsigned long long)st.cam_built,(unsigned long long)st.cam_same_a,
        (unsigned long long)st.mat_sub,(unsigned long long)st.mat_sub_ok,(unsigned long long)st.mat_start,(unsigned long long)st.mat_built,(unsigned long long)st.mat_built_same_a,(unsigned long long)st.mat_same_a);
    auto top=[](const char* what,const std::unordered_map<uint32_t,uint64_t>& m) {
        std::vector<std::pair<uint32_t,uint64_t>> v(m.begin(),m.end());
        std::sort(v.begin(),v.end(),[](auto& x,auto& y){return x.second>y.second;});
        std::fprintf(stderr,"[n2 check %ld]   %s by caller:",(long)getpid(),what);
        for(size_t i=0;i<v.size() && i<10;++i) std::fprintf(stderr," %08x:%llu",v[i].first,(unsigned long long)v[i].second);
        std::fprintf(stderr,"\n");
    };
    std::fprintf(stderr,"[n2 check %ld]   camera predicted at the job start: world %llu, body %llu, missed %llu, other camera blocks %llu\n",(long)getpid(),
        (unsigned long long)cam_pred_world,(unsigned long long)cam_pred_body,(unsigned long long)cam_pred_miss,(unsigned long long)cam_pred_other_block);
    top("camera missed",cam_miss_ra);
    std::fprintf(stderr,"[n2 check %ld]   actor joints: calls %llu, table entry as at the job start %llu; direct %llu (entry as at the start %llu), scaled copy %llu (translation as at the start %llu)\n",(long)getpid(),
        (unsigned long long)joint_calls,(unsigned long long)joint_entry_as_start,(unsigned long long)joint_direct,(unsigned long long)joint_direct_as_start,(unsigned long long)joint_copy,(unsigned long long)joint_copy_t_as_start);
    top("camera derived in the job",cam_built_ra); top("matrix built in the job",mat_built_ra); top("camera derived in the job by camera block",cam_ptr);
    std::fprintf(stderr,"[n2 check %ld]   derived cameras: eye only %llu, view only %llu, both %llu; calls with the job's first call's camera %llu of %llu\n",(long)getpid(),
        (unsigned long long)cam_eye_only,(unsigned long long)cam_view_only,(unsigned long long)cam_both,(unsigned long long)cam_first,(unsigned long long)cam_first_n);
}
}  // namespace in

namespace jt {
// ScaleMatrix 0x800b4b5c(m a0, v a1), CONFIRMED by its disassembly: every
// element m[i][j] (row-major halves) becomes the low word of m * v[j] (MULT,
// mflo) shifted right 12 (arithmetic); words 0-3 hold two halves each, word 4
// is the last element's full 32-bit result (its pad half takes the sign).
void scale(const uint16_t m[10],const int32_t v[3],uint16_t out[10]) {
    int32_t r[9];
    for(int i=0;i<9;++i) r[i]=(int32_t)((uint32_t)(int32_t)(int16_t)m[i]*(uint32_t)v[i%3])>>12;
    for(int i=0;i<9;++i) out[i]=(uint16_t)r[i];
    out[9]=(uint16_t)((uint32_t)r[8]>>16);
}
struct Stats {uint64_t calls=0,direct=0,direct_start=0,copies=0,game_exact=0,game_exact_start=0,scale_start=0,
    kept=0,changed_in_place=0,changed_t=0,changed_rot=0,other_matrix=0,other_in_mod=0,other_same_rot=0,other_same_t=0;} st;
std::unordered_map<uint32_t,uint64_t> other_where;
uint32_t pre_mat=0,pre_ra=0; uint16_t pre_m[10]; int32_t pre_t[3]; bool pre_valid=false;
// Phase 0: the transform entry before our callbacks.
void entry(CPUState* cpu) {
    pre_valid=false;
    if(cpu->gpr[31]!=0x80034c6cu) return;
    ++st.calls;
    const uint32_t tab=psx_mod_read_word(cpu->gpr[20]+0x3c)+cpu->gpr[22];
    const uint32_t mat=cpu->gpr[5];
    pre_mat=mat;pre_ra=cpu->gpr[31];
    for(int k=0;k<10;++k) pre_m[k]=psx_mod_read_half(mat+2*k);
    for(int k=0;k<3;++k) pre_t[k]=(int32_t)psx_mod_read_word(mat+0x14+4*k);
    pre_valid=true;
    if(mat==tab) {
        ++st.direct;
        bool same=in::in_ram(tab,0x20);
        for(int k=0;k<8 && same;++k) same=in::start_word(tab+4*k)==psx_mod_read_word(tab+4*k);
        st.direct_start+=same;
        return;
    }
    ++st.copies;
    if(!in::in_ram(tab,0x20)) return;
    // The scale vector the loop built (sp+0x58: the half at 0x800c0bf4, three times).
    const uint32_t sp=cpu->gpr[29];
    int32_t v[3];for(int k=0;k<3;++k) v[k]=(int32_t)psx_mod_read_word(sp+0x58+4*k);
    const int16_t sc=(int16_t)psx_mod_read_half(0x800c0bf4u),sc0=(int16_t)in::start_half(0x800c0bf4u);
    bool vs=true;for(int k=0;k<3;++k) vs&=v[k]==(int32_t)sc0;
    st.scale_start+=vs;
    uint16_t m_live[10],m_start[10],o[10];
    for(int k=0;k<10;++k) {m_live[k]=psx_mod_read_half(tab+2*k);m_start[k]=in::start_half(tab+2*k);}
    scale(m_live,v,o);
    bool ok=!std::memcmp(o,pre_m,sizeof o);
    for(int k=0;k<3;++k) ok&=(int32_t)psx_mod_read_word(tab+0x14+4*k)==pre_t[k];
    st.game_exact+=ok;
    const int32_t v0[3]={sc0,sc0,sc0};
    scale(m_start,v0,o);
    bool ok0=!std::memcmp(o,pre_m,sizeof o);
    for(int k=0;k<3;++k) ok0&=(int32_t)in::start_word(tab+0x14+4*k)==pre_t[k];
    st.game_exact_start+=ok0;
    (void)sc;
}
// Phase 1: after our callbacks, as the routine reads it.
void entry_done(CPUState* cpu) {
    if(!pre_valid || cpu->gpr[31]!=pre_ra) return;
    pre_valid=false;
    const uint32_t mat=cpu->gpr[5];
    if(mat!=pre_mat) {
        ++st.other_matrix;st.other_in_mod+=(mat&0xff000000u)==0x9f000000u;
        ++other_where[(mat&0xff000000u)==0x9f000000u ? 0x9f000000u : (mat>=cpu->gpr[29]-0x400 && mat<cpu->gpr[29]+0x400) ? 0x5ac0000u : mat];
        // The replacement against the entry: rotation / translation kept?
        uint16_t m[10];int32_t t[3];
        for(int k=0;k<10;++k) m[k]=psx_mod_read_half(mat+2*k);
        for(int k=0;k<3;++k) t[k]=(int32_t)psx_mod_read_word(mat+0x14+4*k);
        st.other_same_rot+=!std::memcmp(m,pre_m,18);st.other_same_t+=!std::memcmp(t,pre_t,sizeof t);
        return;
    }
    uint16_t m[10];int32_t t[3];
    for(int k=0;k<10;++k) m[k]=psx_mod_read_half(mat+2*k);
    for(int k=0;k<3;++k) t[k]=(int32_t)psx_mod_read_word(mat+0x14+4*k);
    const bool rot=!std::memcmp(m,pre_m,sizeof m),tr=!std::memcmp(t,pre_t,sizeof t);
    if(rot && tr) ++st.kept; else {++st.changed_in_place;st.changed_t+=!tr;st.changed_rot+=!rot;}
}
void report() {
    std::fprintf(stderr,"[n2 check %ld] joints (piece 4a): calls=%llu direct=%llu scaled copies=%llu | ScaleMatrix port exact from the live entry %llu, from the entry and scale as at the job start %llu (scale vector as at the start %llu) | our callbacks: copy kept %llu, changed in place %llu (translation %llu, rotation %llu), another matrix passed %llu (in mod memory %llu)\n",
        (long)getpid(),(unsigned long long)st.calls,(unsigned long long)st.direct,(unsigned long long)st.copies,
        (unsigned long long)st.game_exact,(unsigned long long)st.game_exact_start,(unsigned long long)st.scale_start,
        (unsigned long long)st.kept,(unsigned long long)st.changed_in_place,(unsigned long long)st.changed_t,(unsigned long long)st.changed_rot,
        (unsigned long long)st.other_matrix,(unsigned long long)st.other_in_mod);
    std::fprintf(stderr,"[n2 check %ld]   joints: direct entries as at the job start %llu of %llu; replacements with the entry's rotation %llu, translation %llu; where:",(long)getpid(),
        (unsigned long long)st.direct_start,(unsigned long long)st.direct,(unsigned long long)st.other_same_rot,(unsigned long long)st.other_same_t);
    for(auto& [k,v]:other_where) std::fprintf(stderr," %08x:%llu",k,(unsigned long long)v);
    std::fprintf(stderr,"\n");
}
}  // namespace jt

namespace sk {bool build_any(const CPUState* cpu,int16_t m[9],int32_t t[3]);}
namespace jr {
// What the pre-steps of a rebuilt job did (pieces 6 and 7).
struct Pre {uint32_t t_built[2],t_failed,t_skipped,m_built[4];};
// Piece 4b: the job's joint matrices, written by the job; the copy's results.
struct Out {uint32_t hook,mat,ra,w[8],tab,tw[8],wpc,wn;};
constexpr uint32_t max_out=8192;
struct Shared {uint32_t n,done,overflow,eye_predicted,from_prev,order_differs;uint32_t ra_n,ra[8],ra_exact[8],ra_differ[8];uint32_t exact[3],differ[3],missing[3],failed;uint32_t first_differ[4][12];uint32_t nfirst;uint32_t o_tab_differ,o_tab_same_mat_differ,o_tab_same_mat_same,o_stack,o_nfirst,o_first[4][20],o_pc_n,o_pc[8],o_pc_c[8],o_written,o_unwritten;Pre pre;uint32_t m_pad_only;Out real[max_out];};
Shared* s=nullptr; pid_t child=-1; int go=-1; bool active=false;
struct RaStat {uint32_t ra=0;uint64_t exact=0,differ=0;};
RaStat ra_st[16];unsigned ra_st_n=0;
uint32_t o_pc[8],o_pc_n=0;uint64_t o_pc_c[8];
struct Stats {uint64_t m_built[4]={},m_pad_only=0;uint64_t t_built[2]={},t_failed=0,t_skipped=0;uint64_t o_written=0,o_unwritten=0,o_tab_differ=0,o_ts_md=0,o_ts_ms=0,o_stack=0;uint64_t jobs=0,calls=0,exact[3]={},differ[3]={},missing[3]={},failed=0,timeouts=0,no_eye=0,from_prev=0,no_prev=0,order_differs=0;} st;
unsigned kind(uint32_t mat,uint32_t tab) {return mat==camera_base ? 2u : (mat&0xff000000u)==0x9f000000u ? 1u : mat==tab ? 0u : 1u;}
// Piece 6 (bit 256): the actor draw 0x80032e78 (a1 = s3 the actor) builds the
// actor's joint table at 0x80032ff8 just before it enters 0x80032bd8 (actor,
// table; ra 0x80033020, a top-level hook): 0x80098204 (actor, table), or the
// plain build 0x800987cc when the actor type's word has 0x20000, the actor's
// +0x60 is 0xdc or its flags have 2 (CONFIRMED by the disassembly). A later job
// rebuilt from the job start runs the same build there, after its own pose
// patches, on a private stack; a nonzero return from 0x80098204 means the game
// left the draw (counted, the hook is then not expected).
uint32_t t_stack=0,duke=0;bool sky_advanced=false;
void rebuild_table(const CPUState& at,Pre& p) {
    if(!t_stack) t_stack=psx_mod_alloc_guest_memory(0x1000,16);
    if(!t_stack) {++p.t_failed;return;}
    const uint32_t actor=at.gpr[4],table=at.gpr[5];
    // The draw gives an actor without a table the shared one (0x80032f9c).
    if(!psx_mod_read_word(actor+0x3c) && table==0x800d6a30u) orig_ww(actor+0x3c,table);
    if(!table || psx_mod_read_word(actor+0x3c)!=table) {++p.t_skipped;return;}
    const uint32_t type=psx_mod_read_word(0x800d2660u)+28u*psx_mod_read_half(actor+0x2c);
    const bool plain=(psx_mod_read_word(type)&0x20000u) || (int16_t)psx_mod_read_half(actor+0x60)==0xdc || (psx_mod_read_word(actor)&2u);
    CPUState c=at;
    c.pc=0;c.gpr[31]=sentinel;c.gpr[29]=t_stack+0xff0;c.gpr[4]=actor;c.gpr[5]=table;
    psx_dispatch_call(&c,plain?0x800987ccu:0x80098204u,sentinel);
    if(c.pc!=0 || g_psx_call_bail) {g_psx_call_bail=0;++p.t_failed;return;}
    ++p.t_built[plain?1:0];
}
// Before a hook of a job rebuilt from its start: piece 6's joint table and
// piece 7's stack matrices (mask bits 256 and 512), as a rebuild would do them.
void prestep(CPUState& cpu,uint32_t addr,Pre& p,int mask) {
    if((mask&256) && addr==0x80032bd8u && cpu.gpr[31]==0x80033020u) rebuild_table(cpu,p);
    if(addr==0x80032bd8u && cpu.gpr[31]==0x8003492cu) duke=cpu.gpr[4];   // Duke's draw 0x800348d8 (s5 the actor)
    // Bit 512 (D23N2B piece 7): the matrices the job builds on its stack
    // before the transform (actors, props, 0x8003133c, the sky), built
    // here in host code from the job-start memory, as a rebuild would.
    if((mask&512) && addr==0x800292a0u) {
        int16_t m[9];int32_t t[3];
        const uint32_t ra=cpu.gpr[31];
        const int kq=ra==0x80032f48u?0:ra==0x80031decu?1:ra==0x8003133cu?2:3;
        // 0x8003133c's height a2 is *(s5 +0x204) of its caller, Duke's
        // draw (0x80035408): in the job it is on the stack.
        // The sky routine 0x800388e4 first advances its timer 0x800c0c64 by
        // *0x800d21fc unless 0x800be568 or 0x800d2540 is set (CONFIRMED by
        // its disassembly): once a job, before its three matrices.
        if(kq==3 && !sky_advanced && (ra==0x80038c68u || ra==0x80038d7cu || ra==0x80038e18u)) {
            sky_advanced=true;
            if(!psx_mod_read_half(0x800be568u) && !psx_mod_read_half(0x800d2540u)) psx_mod_write_word(0x800c0c64u,psx_mod_read_word(0x800c0c64u)+psx_mod_read_word(0x800d21fcu));
        }
        if(sk::build_any(&cpu,m,t) && (kq!=2 || duke)) {
            if(kq==2) t[1]=(int32_t)psx_mod_read_word(duke+0x204);
            for(int q=0;q<9;++q) psx_mod_write_half(cpu.gpr[5]+2*q,(uint16_t)m[q]);
            for(int q=0;q<3;++q) psx_mod_write_word(cpu.gpr[5]+0x14+4*q,(uint32_t)t[q]);
            ++p.m_built[kq];
        }
    }
}
[[noreturn]] void run(int fd) {
    char c=0;
    if(read(fd,&c,1)!=1 || c!='g') _exit(0);
    logging=false;g_psx_mod_hook_observer=nullptr;
    static std::vector<uint32_t> buf(1u<<20);
    render_replay_record_begin(buf.data(),(uint32_t)buf.size());
    // The eye the composition derives before the draw (piece 2's prediction,
    // as vis::begin gives the portal walk): the job-start memory has the
    // patched camera, not yet the derived eye.
    s->eye_predicted=in::pred_ok?1u:0u;
    if(in::pred_ok) for(int k=0;k<3;++k) {orig_ww(camera_base+0x14+4*k,(uint32_t)in::pred_eye[k]);orig_ww(camera_base+0x34+4*k,(uint32_t)in::pred_eye[k]);}
    // Piece 4c (bit 32): the hooks' CPU state from the earlier job of the same
    // frame (another fraction: its own registers), as a rebuild (D23N4)
    // would have it, when its hook list is the later job's, call for call.
    const Hook* hooks=sh->hook;
    if(n2_mask()&32) {
        bool same=prev.serial==n2_serial && prev.alpha!=n2_alpha && prev.hooks.size()==sh->nhooks;
        for(uint32_t i=0;same && i<sh->nhooks;++i) same=prev.hooks[i].addr==sh->hook[i].addr && prev.hooks[i].cpu.gpr[31]==sh->hook[i].cpu.gpr[31];
        if(same) {hooks=prev.hooks.data();s->from_prev=1;}
        else {s->from_prev=0;s->order_differs=prev.serial==n2_serial && prev.alpha!=n2_alpha;__atomic_store_n(&s->done,1u,__ATOMIC_RELEASE);_exit(0);}
    }
    uint32_t k=0;
    for(uint32_t i=0;i<sh->nhooks && !g_psx_call_bail;++i) {
        const Hook& h=hooks[i];
        CPUState cpu=h.cpu;
        // Bit 64 (piece 4d): every caller of the transform, not only the joint loop.
        const bool joint=h.addr==0x800292a0u && (cpu.gpr[31]==0x80034c6cu || (n2_mask()&64));
        const uint32_t tab=joint ? psx_mod_read_word(cpu.gpr[20]+0x3c)+cpu.gpr[22] : 0;
        prestep(cpu,h.addr,s->pre,n2_mask());
        const uint32_t tab2=joint ? psx_mod_read_word(cpu.gpr[19]+0x3c)+cpu.gpr[18] : 0;   // piece 6, before the hook runs
        psx_mod_function_entry(&cpu,h.addr);
        if(!joint) continue;
        while(k<s->n && s->real[k].hook<i) ++k;
        if(k>=s->n || s->real[k].hook!=i) continue;
        const Out& r=s->real[k++];
        const unsigned kd=kind(r.mat,tab);
        bool same=cpu.gpr[5]==r.mat;
        uint32_t w[8];for(int j=0;j<8;++j) w[j]=psx_mod_read_word(cpu.gpr[5]+4*j);
        if(same && std::memcmp(w,r.w,sizeof w) && (n2_mask()&512)) {
            // The matrix's pad half (+0x12) is the caller's stack, never read
            // by the transform (its transpose keeps it, MulMatrix0 ignores it).
            uint32_t a[8],b[8];std::memcpy(a,w,sizeof a);std::memcpy(b,r.w,sizeof b);a[4]&=0xffffu;b[4]&=0xffffu;
            if(!std::memcmp(a,b,sizeof a)) {++s->m_pad_only;std::memcpy(w,r.w,sizeof w);}
        }
        same=same && !std::memcmp(w,r.w,sizeof w);
        if(r.ra!=0x80034c6cu) {   // piece 4d: by caller
            uint32_t j=0;while(j<s->ra_n && s->ra[j]!=r.ra) ++j;
            if(j==s->ra_n && j<8) {s->ra[j]=r.ra;++s->ra_n;}
            if(j<8) ++(same?s->ra_exact[j]:s->ra_differ[j]);
            if(r.ra==0x80033584u && r.tab) {
                uint32_t tw[8];for(int q=0;q<8;++q) tw[q]=psx_mod_read_word(tab2+4*q);
                const bool tsame=tab2==r.tab && !std::memcmp(tw,r.tw,sizeof tw);
                if((r.mat&0xff000000u)!=0x80000000u || r.mat!=r.tab) ++s->o_stack;
                if(!tsame) ++s->o_tab_differ; else ++(same?s->o_tab_same_mat_same:s->o_tab_same_mat_differ);
                ++(r.wn?s->o_written:s->o_unwritten);
                if(!same && r.wn) {uint32_t q=0;while(q<s->o_pc_n && s->o_pc[q]!=r.wpc) ++q;if(q==s->o_pc_n && q<8) {s->o_pc[q]=r.wpc;++s->o_pc_n;}if(q<8) ++s->o_pc_c[q];}
                if(!same && s->o_nfirst<4) {uint32_t* f=s->o_first[s->o_nfirst++];f[0]=i;f[1]=r.mat;f[2]=cpu.gpr[5];f[3]=r.tab;f[4]=tab2;f[5]=psx_mod_read_word(cpu.gpr[19]+0x2c);for(int q=0;q<7;++q) {f[6+q]=r.w[q];f[13+q]=w[q];}}
            }
            continue;
        }
        if(same) ++s->exact[kd]; else {
            ++s->differ[kd];
            if(s->nfirst<4) {uint32_t* f=s->first_differ[s->nfirst++];f[0]=i;f[1]=r.mat;f[2]=cpu.gpr[5];f[3]=cpu.gpr[23];for(int j=0;j<4;++j) {f[4+j]=r.w[j+4];f[8+j]=w[j+4];}}
        }
    }
    for(;k<s->n;++k) {
        if(s->real[k].ra!=0x80034c6cu) continue;
        const uint32_t i=s->real[k].hook;
        const uint32_t tab=i<sh->nhooks ? psx_mod_read_word(hooks[i].cpu.gpr[20]+0x3c)+hooks[i].cpu.gpr[22] : 0;
        ++s->missing[kind(s->real[k].mat,tab)];
    }
    s->failed=g_psx_call_bail?1u:0u;
    int overflow=0;render_replay_record_end(&overflow);
    __atomic_store_n(&s->done,1u,__ATOMIC_RELEASE);
    _exit(0);
}
void begin() {
    // A job that overflowed its record never reached n2_at_flip: its copy goes.
    if(child>0) {kill(child,SIGKILL);waitpid(child,nullptr,0);child=-1;if(go>=0) {close(go);go=-1;}}
    active=false;
    static unsigned count=0;
    if(++count%4) return;
    if(!s) {
        void* m=mmap(nullptr,sizeof(Shared),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE,-1,0);
        if(m==MAP_FAILED) return;
        s=(Shared*)m;
    }
    std::memset(s,0,offsetof(Shared,real));
    int fds[2];
    if(pipe(fds)!=0) return;
    const pid_t pid=fork();
    if(pid==0) {close(fds[1]);run(fds[0]);}
    close(fds[0]);
    if(pid<0) {close(fds[1]);return;}
    child=pid;go=fds[1];active=true;
}
// Phase 1 of the transform entry from the joint loop: what the job passes.
void entry_done(CPUState* cpu) {
    if(!active || (cpu->gpr[31]!=0x80034c6cu && !(n2_mask()&64))) return;
    if(s->n>=max_out) {s->overflow=1;return;}
    Out& o=s->real[s->n++];
    o.hook=sh->nhooks;o.mat=cpu->gpr[5];o.ra=cpu->gpr[31];
    for(int j=0;j<8;++j) o.w[j]=psx_mod_read_word(o.mat+4*j);
    // Piece 6 (bit 64): the actor joint loop at 0x80033584 passes its table
    // entry (*(s3 +0x3c) + s2) or a scaled stack copy of it; keep the entry.
    o.tab=0;o.wpc=0;o.wn=0;if(o.ra==0x80033584u) {o.tab=psx_mod_read_word(cpu->gpr[19]+0x3c)+cpu->gpr[18];for(int j=0;j<8;++j) o.tw[j]=psx_mod_read_word(o.tab+4*j);
        // The job's own stores into the entry before this call: the last one's PC and how many.
        for(uint32_t q=sh->nstores;q-- > 0;) {const Store& x=sh->store[q];if(x.addr+x.size>o.tab && x.addr<o.tab+32) {if(!o.wn) o.wpc=x.pc;++o.wn;}}}
}
void at_flip() {
    if(!active) return;
    active=false;
    const char g=(sh->overflow || s->overflow)?'x':'g';
    if(write(go,&g,1)!=1) {}
    close(go);go=-1;
    bool done=false;
    if(g=='g') for(int i=0;i<4000 && !(done=__atomic_load_n(&s->done,__ATOMIC_ACQUIRE));++i) usleep(500);
    if(!done) kill(child,SIGKILL);
    waitpid(child,nullptr,0);child=-1;
    if(g!='g') return;
    if(!done) {++st.timeouts;return;}
    if(n2_mask()&32) {
        if(!s->from_prev) {++(s->order_differs?st.order_differs:st.no_prev);return;}
        ++st.from_prev;
    }
    ++st.jobs;st.calls+=s->n;st.failed+=s->failed;st.no_eye+=!s->eye_predicted;
    for(int k=0;k<3;++k) {st.exact[k]+=s->exact[k];st.differ[k]+=s->differ[k];st.missing[k]+=s->missing[k];}
    for(uint32_t j=0;j<s->ra_n && j<8;++j) {
        unsigned q=0;while(q<ra_st_n && ra_st[q].ra!=s->ra[j]) ++q;
        if(q==ra_st_n && q<16) ra_st[ra_st_n++].ra=s->ra[j];
        if(q<16) {ra_st[q].exact+=s->ra_exact[j];ra_st[q].differ+=s->ra_differ[j];}
    }
    for(int q=0;q<4;++q) st.m_built[q]+=s->pre.m_built[q];st.m_pad_only+=s->m_pad_only;
    for(int q=0;q<2;++q) st.t_built[q]+=s->pre.t_built[q];st.t_failed+=s->pre.t_failed;st.t_skipped+=s->pre.t_skipped;
    st.o_written+=s->o_written;st.o_unwritten+=s->o_unwritten;
    for(uint32_t q=0;q<s->o_pc_n;++q) {unsigned k=0;while(k<o_pc_n && o_pc[k]!=s->o_pc[q]) ++k;if(k==o_pc_n && k<8) o_pc[o_pc_n++]=s->o_pc[q];if(k<8) o_pc_c[k]+=s->o_pc_c[q];}
    st.o_tab_differ+=s->o_tab_differ;st.o_ts_md+=s->o_tab_same_mat_differ;st.o_ts_ms+=s->o_tab_same_mat_same;st.o_stack+=s->o_stack;
    static unsigned oshown=0;
    for(uint32_t j=0;j<s->o_nfirst && oshown<12;++j,++oshown) {const uint32_t* f=s->o_first[j];
        std::fprintf(stderr,"[n2 check %ld]   actor joint 80033584 differs: hook %u passed %08x / start %08x, table %08x / %08x, actor word2c %08x | job %08x %08x %08x %08x %08x %08x %08x | start %08x %08x %08x %08x %08x %08x %08x\n",(long)getpid(),f[0],f[1],f[2],f[3],f[4],f[5],f[6],f[7],f[8],f[9],f[10],f[11],f[12],f[13],f[14],f[15],f[16],f[17],f[18],f[19]);}
    static unsigned shown=0;
    for(uint32_t j=0;j<s->nfirst && shown<8;++j,++shown) {const uint32_t* f=s->first_differ[j];
        std::fprintf(stderr,"[n2 check %ld]   joint differs: hook %u joint %u passed %08x, from the start %08x; t %d %d %d / %d %d %d\n",(long)getpid(),f[0],f[3],f[1],f[2],(int32_t)f[5],(int32_t)f[6],(int32_t)f[7],(int32_t)f[9],(int32_t)f[10],(int32_t)f[11]);}
}
void report() {
    std::fprintf(stderr,"[n2 check %ld] joints from the job start (piece 4b): jobs=%llu calls=%llu | table entry exact %llu differ %llu missing %llu | mod memory exact %llu differ %llu missing %llu | camera block exact %llu differ %llu missing %llu | copy failed %llu timeouts %llu, jobs without a predicted eye %llu\n",
        (long)getpid(),(unsigned long long)st.jobs,(unsigned long long)st.calls,
        (unsigned long long)st.exact[0],(unsigned long long)st.differ[0],(unsigned long long)st.missing[0],
        (unsigned long long)st.exact[1],(unsigned long long)st.differ[1],(unsigned long long)st.missing[1],
        (unsigned long long)st.exact[2],(unsigned long long)st.differ[2],(unsigned long long)st.missing[2],
        (unsigned long long)st.failed,(unsigned long long)st.timeouts,(unsigned long long)st.no_eye);
    if(n2_mask()&64) {
        std::string t;char b[64];
        for(unsigned q=0;q<ra_st_n;++q) {std::snprintf(b,sizeof b," %08x exact %llu differ %llu |",ra_st[q].ra,(unsigned long long)ra_st[q].exact,(unsigned long long)ra_st[q].differ);t+=b;}
        std::fprintf(stderr,"[n2 check %ld] piece 4d, other transform callers:%s\n",(long)getpid(),t.c_str());
        std::fprintf(stderr,"[n2 check %ld] piece 6, actor joints 80033584: table entry differs %llu | entry same, matrix differs %llu | both same %llu | a scaled or other copy passed %llu\n",(long)getpid(),
            (unsigned long long)st.o_tab_differ,(unsigned long long)st.o_ts_md,(unsigned long long)st.o_ts_ms,(unsigned long long)st.o_stack);
        if(n2_mask()&512) std::fprintf(stderr,"[n2 check %ld] piece 7, stack matrices built on the host in the copy: actors 80032f48 %llu, props 80031dec %llu, 8003133c %llu, sky %llu; equal but for the pad half %llu\n",(long)getpid(),
            (unsigned long long)st.m_built[0],(unsigned long long)st.m_built[1],(unsigned long long)st.m_built[2],(unsigned long long)st.m_built[3],(unsigned long long)st.m_pad_only);
        if(n2_mask()&256) std::fprintf(stderr,"[n2 check %ld] piece 6, joint tables rebuilt in the copy: 0x80098204 %llu, plain 0x800987cc %llu, failed %llu, skipped %llu\n",(long)getpid(),
            (unsigned long long)st.t_built[0],(unsigned long long)st.t_built[1],(unsigned long long)st.t_failed,(unsigned long long)st.t_skipped);
        {std::string t;char b[64];for(unsigned k=0;k<o_pc_n;++k) {std::snprintf(b,sizeof b," %08x x%llu",o_pc[k],(unsigned long long)o_pc_c[k]);t+=b;}
        std::fprintf(stderr,"[n2 check %ld] piece 6: entries the job wrote before the call %llu, not written %llu; differing calls by the PC of the job's last store into the entry:%s\n",(long)getpid(),(unsigned long long)st.o_written,(unsigned long long)st.o_unwritten,t.c_str());}
    }
    if(n2_mask()&32) std::fprintf(stderr,"[n2 check %ld] piece 4c: jobs run from the earlier job's record %llu, no earlier job of the frame %llu, hook lists differ %llu\n",
        (long)getpid(),(unsigned long long)st.from_prev,(unsigned long long)st.no_prev,(unsigned long long)st.order_differs);
}
}  // namespace jr

namespace wj {
// D23N2B piece 8 (bit 1024): a whole later job rebuilt from its start. A copy
// of the worker forked at a later job's start (its memory after the job's
// camera and pose patches) rebuilds the job's stream up to the flip: the
// stores of a chosen source applied in order, each top-level hook called with
// a chosen CPU state after the pre-steps of pieces 6 and 7 (bits 256 and 512),
// the game's own stream items between hooks spliced from that source's
// stream. DNTTK_N2_WHOLE="S C": stores S 0 none, 1 the earlier job's of the
// same frame (another fraction; the eye words of the camera block left out,
// the predicted eye written instead), 2 the job's own (the D23N1 rebuild, a
// sanity check); CPU state C 0 the job's own hooks, 1 the earlier job's
// (needs the same hook list, call for call). Then, for each hook, the packets
// it produced against the same hook's in the job's real stream, word for word
// (the game's PGXP records left out, as D23N1 does), and the game items
// between hooks likewise. Every 4th later job of a frame.
constexpr uint32_t max_words=1u<<20,max_seg=12288;
struct Shared {uint32_t done,ok,skip,n,flip,cflip,channel[3],seg[max_seg][2],nseg;double ms;CPUState flip_cpu;uint32_t words[max_words];};
Shared* s=nullptr;pid_t child=-1;int go=-1;bool active=false,waiting=false;uint64_t t_job=0;double ms_job=0;
int src_stores() {static const int v=[]{const char* t=std::getenv("DNTTK_N2_WHOLE");return t?std::atoi(t):0;}();return v;}
int src_cpu() {static const int v=[]{const char* t=std::getenv("DNTTK_N2_WHOLE");const char* sp=t?std::strchr(t,' '):nullptr;return sp?std::atoi(sp+1):0;}();return v;}
bool eye_word(uint32_t a) {a&=0x1fffffffu;const uint32_t c=camera_base&0x1fffffffu;return (a>=c+0x14 && a<c+0x20) || (a>=c+0x34 && a<c+0x40);}
jr::Pre pre_counts{};
[[noreturn]] void run(int fd) {
    char c=0;
    if(read(fd,&c,1)!=1 || c!='g') _exit(0);
    logging=false;g_psx_mod_hook_observer=nullptr;
    const uint64_t t0=SDL_GetPerformanceCounter();
    const int S=src_stores(),C=src_cpu();
    const bool earlier_ok=prev.serial==n2_serial && prev.alpha!=n2_alpha && prev.hooks.size()==sh->nhooks && !prev.words.empty();
    bool same_list=earlier_ok;
    for(uint32_t i=0;same_list && i<sh->nhooks;++i) same_list=prev.hooks[i].addr==sh->hook[i].addr && prev.hooks[i].cpu.gpr[31]==sh->hook[i].cpu.gpr[31];
    if((S==1 || C==1) && !same_list) {s->skip=earlier_ok?2u:1u;__atomic_store_n(&s->done,1u,__ATOMIC_RELEASE);_exit(0);}
    const Hook* H=C==1 ? prev.hooks.data() : sh->hook;
    const uint32_t* src=C==1 ? prev.words.data() : parent_stream;
    const uint32_t src_flip=C==1 ? (uint32_t)prev.words.size() : s->flip;   // record-only jobs leave sh->flip_at unset
    const Store* ST=S==1 ? prev.stores.data() : S==2 ? sh->store : nullptr;
    const uint32_t nst=S==1 ? (uint32_t)prev.stores.size() : S==2 ? sh->nstores : 0;
    // The hooks' stores positions are in their own record.
    if(S!=2 && in::pred_ok) for(int k=0;k<3;++k) {orig_ww(camera_base+0x14+4*k,(uint32_t)in::pred_eye[k]);orig_ww(camera_base+0x34+4*k,(uint32_t)in::pred_eye[k]);}
    render_replay_record_begin(s->words,max_words);
    render_replay_record_splice(parent_stream,sh->prefix);
    uint32_t si=0,last=sh->prefix;
    const Hook* SH=S==1 ? prev.hooks.data() : sh->hook;   // store positions by hook
    auto apply=[&](uint32_t upto) {
        for(;si<upto && si<nst;++si) {
            const Store& w=ST[si];
            if(io(w.addr) || (S==1 && eye_word(w.addr))) continue;
            if(w.size==4) orig_ww(w.addr,w.value); else if(w.size==2) orig_wh(w.addr,(uint16_t)w.value); else orig_wb(w.addr,(uint8_t)w.value);
        }
    };
    jr::Pre p{};uint32_t gte_out[8]={};bool have_gte=false;
    for(uint32_t i=0;i<sh->nhooks && !g_psx_call_bail;++i) {
        const Hook& h=H[i];
        if(ST) apply(SH[i].stores);
        render_replay_record_splice(src+last,h.before-last);
        uint32_t n=0;render_replay_record_words(&n);s->seg[i][0]=n;
        CPUState cpu=h.cpu;
        jr::prestep(cpu,h.addr,p,n2_mask());
        if(C==1 && have_gte) for(int k=0;k<8;++k) cpu.gte_ctrl[k]=gte_out[k];
        psx_mod_function_entry(&cpu,h.addr);
        // Bit 2048: the transform's own effects (its scratchpad stores and the
        // GTE rotation and translation it leaves), from piece 1's exact host
        // port on the matrix the routine reads after our callbacks.
        if((n2_mask()&2048) && h.addr==0x800292a0u) {
            const xf::Result r=xf::from_memory(cpu.gpr[4],cpu.gpr[5]);
            for(int k=0;k<3;++k) {orig_ww(xf::a_d+4*k,r.d[k]);orig_ww(xf::a_lv+4*k,r.lv[k]);}
            for(int k=0;k<5;++k) orig_ww(xf::a_tr+4*k,r.tr[k]);
            for(int k=0;k<8;++k) {orig_ww(xf::a_out+4*k,r.out[k]);gte_out[k]=r.out[k];}
            gte_out[4]=(uint32_t)(int32_t)(int16_t)r.out[4];have_gte=true;
        }
        render_replay_record_words(&n);s->seg[i][1]=n;
        last=h.after;
    }
    if(ST) apply(nst);
    render_replay_record_splice(src+last,src_flip-last);
    {uint32_t n=0;render_replay_record_words(&n);s->cflip=n;}
    s->ms=ms_since(t0);
    // The game's own packets: the chain close and the submit from the flip, as
    // D23N1's rebuild runs them (D23N3 lists what differs here).
    render_replay_channel_set(s->channel);
    bool ok=!g_psx_call_bail;
    for(uint32_t fn:{chain_close,submit}) {
        if(!ok) break;
        CPUState tail=s->flip_cpu;tail.pc=0;tail.gpr[31]=sentinel;
        psx_dispatch_call(&tail,fn,sentinel);
        ok=tail.pc==0 && !g_psx_call_bail;
    }
    int overflow=0;s->n=render_replay_record_end(&overflow);
    s->nseg=sh->nhooks;s->ok=ok && !overflow;
    __atomic_store_n(&s->done,1u,__ATOMIC_RELEASE);
    _exit(0);
}
void begin() {
    if(child>0) {kill(child,SIGKILL);waitpid(child,nullptr,0);child=-1;if(go>=0) {close(go);go=-1;}}
    active=waiting=false;
    if(!(prev.serial==n2_serial && prev.alpha!=n2_alpha)) return;   // later jobs only
    static unsigned count=0;
    if(++count%4) return;
    if(!s) {
        void* m=mmap(nullptr,sizeof(Shared),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS|MAP_NORESERVE,-1,0);
        if(m==MAP_FAILED) return;
        s=(Shared*)m;
    }
    s->done=s->ok=s->skip=s->n=s->nseg=0;
    int fds[2];
    if(pipe(fds)!=0) return;
    t_job=SDL_GetPerformanceCounter();
    const pid_t pid=fork();
    if(pid==0) {close(fds[1]);run(fds[0]);}
    close(fds[0]);
    if(pid<0) {close(fds[1]);return;}
    child=pid;go=fds[1];active=true;
}
// One stream range's items, without the game packets' PGXP records.
void items(const uint32_t* w,uint32_t a,uint32_t b,std::vector<uint32_t>& out) {
    out.clear();
    for(uint32_t i=a;i<b;) {
        const uint32_t kind=w[i]>>24,count=w[i]&0xffffffu;
        if(i+1+count>b) {out.push_back(0xdeadbeefu);break;}
        if(!(kind==4 && count>=1 && (w[i+1]&0xffffffu)<0x200000u)) out.insert(out.end(),w+i,w+i+1+count);
        i+=1+count;
    }
}
struct Stats {uint64_t host_exact_tail_differs=0,tail_items=0,tail_exact=0,tail_exact_jobs=0;uint64_t jobs=0,skipped_no_earlier=0,skipped_list=0,failed=0,timeouts=0,exact_jobs=0,host_exact_jobs=0,hooks=0,hooks_exact=0,hooks_empty=0,gaps=0,gaps_exact=0;double ms_job=0,ms_rebuild=0;} st;
struct ByAddr {uint32_t addr=0,ra=0;uint64_t differ=0;};
ByAddr by[24],first[24],tail_by[24];unsigned nby=0,nfirst=0,ntail=0,shown=0,tail_shown=0;
void at_flip() {
    if(!active) return;
    active=false;
    const char g=sh->overflow || !n2_flip_cpu ?'x':'g';
    ms_job=ms_since(t_job);
    {uint32_t n=0;render_replay_record_words(&n);s->flip=n;}
    if(n2_flip_cpu) s->flip_cpu=*n2_flip_cpu;
    render_replay_channel_get(s->channel);
    if(write(go,&g,1)!=1) {}
    close(go);go=-1;
    waiting=g=='g';
    if(!waiting) {kill(child,SIGKILL);waitpid(child,nullptr,0);child=-1;}
}
// After the job's submit: wait for the copy, compare.
void finish(bool job_ok) {
    if(!waiting) return;
    waiting=false;
    bool done=false,died=false;
    for(int i=0;i<8000 && !(done=__atomic_load_n(&s->done,__ATOMIC_ACQUIRE));++i) {int stt=0;if(waitpid(child,&stt,WNOHANG)==child) {died=true;break;}usleep(500);}
    done=__atomic_load_n(&s->done,__ATOMIC_ACQUIRE)!=0;
    if(!died) {if(!done) kill(child,SIGKILL);waitpid(child,nullptr,0);}
    child=-1;
    if(!job_ok) return;
    if(!done) {++(died?st.failed:st.timeouts);return;}
    if(s->skip) {++(s->skip==2?st.skipped_list:st.skipped_no_earlier);return;}
    if(!s->ok) {++st.failed;return;}
    ++st.jobs;st.ms_job+=ms_job;st.ms_rebuild+=s->ms;
    uint32_t pn=0;const uint32_t* ps=render_replay_record_words(&pn);
    std::vector<uint32_t> a,b;
    bool host_ok=true,all_ok=true;
    uint32_t lastr=sh->prefix,lastc=sh->prefix;
    for(uint32_t i=0;i<sh->nhooks && i<s->nseg;++i) {
        const Hook& h=sh->hook[i];
        // The game's items before this hook.
        items(ps,lastr,h.before,a);items(s->words,lastc,s->seg[i][0],b);
        if(!a.empty() || !b.empty()) {++st.gaps;if(a==b) ++st.gaps_exact; else all_ok=false;}
        items(ps,h.before,h.after,a);items(s->words,s->seg[i][0],s->seg[i][1],b);
        if(a.empty() && b.empty()) ++st.hooks_empty;
        else {
            ++st.hooks;
            if(a==b) ++st.hooks_exact;
            else {
                if(host_ok) {   // the job's first differing hook: a root, not a cascade
                    unsigned q=0;while(q<nfirst && !(first[q].addr==h.addr && first[q].ra==h.cpu.gpr[31])) ++q;
                    if(q==nfirst && q<24) {first[q].addr=h.addr;first[q].ra=h.cpu.gpr[31];++nfirst;}
                    if(q<24) ++first[q].differ;
                }
                host_ok=all_ok=false;
                unsigned q=0;while(q<nby && !(by[q].addr==h.addr && by[q].ra==h.cpu.gpr[31])) ++q;
                if(q==nby && q<24) {by[q].addr=h.addr;by[q].ra=h.cpu.gpr[31];++nby;}
                if(q<24) ++by[q].differ;
                if(shown<10) {
                    ++shown;size_t k=0;while(k<a.size() && k<b.size() && a[k]==b[k]) ++k;
                    std::fprintf(stderr,"[n2 check %ld]   whole job: hook %u %08x (ra %08x) differs: words %zu/%zu, first at %zu: job %08x rebuilt %08x\n",(long)getpid(),i,h.addr,h.cpu.gpr[31],a.size(),b.size(),k,k<a.size()?a[k]:0,k<b.size()?b[k]:0);
                }
            }
        }
        lastr=h.after;lastc=s->seg[i][1];
    }
    items(ps,lastr,s->flip,a);items(s->words,lastc,s->cflip,b);
    if(!a.empty() || !b.empty()) {++st.gaps;if(a==b) ++st.gaps_exact; else all_ok=false;}
    // After the flip: the game's own packets (chain close and submit), item by
    // item; a differing GP0 packet is counted by its command byte (D23N3).
    {
        std::vector<std::pair<uint32_t,uint32_t>> ia,ib;
        auto list=[](const uint32_t* w,uint32_t x,uint32_t y,std::vector<std::pair<uint32_t,uint32_t>>& out) {
            out.clear();
            for(uint32_t i=x;i<y;) {const uint32_t kind=w[i]>>24,count=w[i]&0xffffffu;if(i+1+count>y) break;
                if(!(kind==4 && count>=1 && (w[i+1]&0xffffffu)<0x200000u)) out.push_back({i,1+count});i+=1+count;}
        };
        list(ps,s->flip,pn,ia);list(s->words,s->cflip,s->n,ib);
        st.tail_items+=ia.size();
        bool tail_ok=ia.size()==ib.size();
        for(size_t k=0;k<ia.size() && k<ib.size();++k) {
            const bool eq=ia[k].second==ib[k].second && !std::memcmp(ps+ia[k].first,s->words+ib[k].first,4u*ia[k].second);
            if(eq) {++st.tail_exact;continue;}
            tail_ok=false;
            if(!host_ok) continue;   // after a host difference the chain's addresses cascade
            const uint32_t* w=ps+ia[k].first;const uint32_t kind=w[0]>>24;
            if(tail_shown<12) {
                ++tail_shown;const uint32_t* v=s->words+ib[k].first;
                std::fprintf(stderr,"[n2 check %ld]   game packet differs, item %zu kind %u words %u/%u: job",(long)getpid(),k,kind,ia[k].second,ib[k].second);
                for(uint32_t j=1;j<ia[k].second && j<14;++j) std::fprintf(stderr," %08x",w[j]);
                std::fprintf(stderr," | rebuilt");
                for(uint32_t j=1;j<ib[k].second && j<14;++j) std::fprintf(stderr," %08x",v[j]);
                std::fprintf(stderr,"\n");
            }
            // GP0 runs (kind 0): the first differing word's packet command.
            uint32_t key=kind<<8;
            if(kind==0 && ia[k].second==ib[k].second) {
                uint32_t j=1;while(j<ia[k].second && w[j]==s->words[ib[k].first+j]) ++j;
                key|=j<ia[k].second ? (w[j]>>24) : 0xff;
            }
            unsigned q=0;while(q<ntail && tail_by[q].addr!=key) ++q;
            if(q==ntail && q<24) {tail_by[q].addr=key;++ntail;}
            if(q<24) ++tail_by[q].differ;
        }
        st.tail_exact_jobs+=tail_ok;st.host_exact_tail_differs+=host_ok && !tail_ok;
        all_ok=all_ok && tail_ok;
    }
    st.host_exact_jobs+=host_ok;st.exact_jobs+=all_ok;
}
void report() {
    std::fprintf(stderr,"[n2 check %ld] piece 8, whole later jobs (stores %d, cpu %d): jobs=%llu host-exact=%llu all-exact=%llu | hooks with packets %llu exact %llu (empty %llu) | game items between hooks %llu exact %llu | skipped: no earlier job %llu, hook list differs %llu | failed %llu timeouts %llu | ms job %.2f rebuild %.2f\n",
        (long)getpid(),src_stores(),src_cpu(),(unsigned long long)st.jobs,(unsigned long long)st.host_exact_jobs,(unsigned long long)st.exact_jobs,
        (unsigned long long)st.hooks,(unsigned long long)st.hooks_exact,(unsigned long long)st.hooks_empty,(unsigned long long)st.gaps,(unsigned long long)st.gaps_exact,
        (unsigned long long)st.skipped_no_earlier,(unsigned long long)st.skipped_list,(unsigned long long)st.failed,(unsigned long long)st.timeouts,
        st.jobs?st.ms_job/st.jobs:0.0,st.jobs?st.ms_rebuild/st.jobs:0.0);
    std::string t;char b[64];
    for(unsigned q=0;q<nby;++q) {std::snprintf(b,sizeof b," %08x/%08x x%llu",by[q].addr,by[q].ra,(unsigned long long)by[q].differ);t+=b;}
    std::fprintf(stderr,"[n2 check %ld] piece 8, differing hooks by address/ra:%s\n",(long)getpid(),t.c_str());
    t.clear();
    for(unsigned q=0;q<nfirst;++q) {std::snprintf(b,sizeof b," %08x/%08x x%llu",first[q].addr,first[q].ra,(unsigned long long)first[q].differ);t+=b;}
    std::fprintf(stderr,"[n2 check %ld] piece 8, differing first, a job's first differing hook:%s\n",(long)getpid(),t.c_str());
    t.clear();
    for(unsigned q=0;q<ntail;++q) {std::snprintf(b,sizeof b," %04x x%llu",tail_by[q].addr,(unsigned long long)tail_by[q].differ);t+=b;}
    std::fprintf(stderr,"[n2 check %ld] piece 8, game packets after the flip: jobs exact %llu (host-exact jobs whose game packets differ %llu), items %llu exact %llu; differing in host-exact jobs, by stream kind and GP0 command:%s\n",(long)getpid(),
        (unsigned long long)st.tail_exact_jobs,(unsigned long long)st.host_exact_tail_differs,(unsigned long long)st.tail_items,(unsigned long long)st.tail_exact,t.c_str());
}
}  // namespace wj

namespace vis {
// Writes of the scratch run, undone in reverse order.
struct Undo {uint32_t addr,old,size;};
std::vector<Undo> undo;
void (*u_ww0)(uint32_t,uint32_t)=nullptr; void (*u_wh0)(uint32_t,uint16_t)=nullptr; void (*u_wb0)(uint32_t,uint8_t)=nullptr;
void u_ww(uint32_t a,uint32_t v) {undo.push_back({a,psx_mod_read_word(a),4});u_ww0(a,v);}
void u_wh(uint32_t a,uint16_t v) {undo.push_back({a,psx_mod_read_half(a),2});u_wh0(a,v);}
void u_wb(uint32_t a,uint8_t v) {undo.push_back({a,psx_mod_read_byte(a),1});u_wb0(a,v);}
void put_word(uint32_t a,uint32_t v) {u_ww(a,v);}
void rollback() {
    for(size_t i=undo.size();i-->0;) {
        const Undo& u=undo[i];
        if(u.size==4) u_ww0(u.addr,u.old); else if(u.size==2) u_wh0(u.addr,(uint16_t)u.old); else u_wb0(u.addr,(uint8_t)u.old);
    }
    undo.clear();
}
// What the earlier job of the frame had at its portal walk entry (after our
// callbacks): the context fields the composition and our draw distance set
// just before the call, the GTE, the arguments.
struct Entry {bool valid=false;uint64_t serial=~0ull;float alpha=0;uint32_t ctx[4],gpr[32],gte_d[32],gte_c[32],rect[2];};
Entry cur_e,prev_e;
struct List {uint32_t count=0;uint32_t w[32*3];bool operator==(const List& o) const {return count==o.count && !std::memcmp(w,o.w,4*3*std::min<uint32_t>(count,32));}};
List predicted; bool have_prediction=false; uint32_t pred_room=0;
int32_t pred_h=0,pred_ofx=0,pred_ofy=0;
List read_list() {List l;l.count=psx_mod_read_word(0x800d68a8);for(uint32_t i=0;i<3*std::min<uint32_t>(l.count,32);++i) l.w[i]=psx_mod_read_word(0x800d68b0+4*i);return l;}
constexpr uint32_t ctx_base=0x800d67a8u;
struct Stats {uint64_t jobs=0,no_prev=0,predicted=0,run_failed=0,checked=0,list_exact=0,list_count_differs=0,list_differs=0,inputs_exact=0,room_differs=0,cam_differs=0,rect_differs=0,proj_differs=0;} st;
void begin(const CPUState& job,uint64_t serial,float alpha) {
    have_prediction=false;
    ++st.jobs;
    if(!prev_e.valid || prev_e.serial!=serial || prev_e.alpha==alpha || !in::pred_ok) {++st.no_prev;return;}
    const uint32_t mem=psx_mod_alloc_guest_memory(0x1000,16);
    if(!mem) return;
    u_ww0=job.write_word;u_wh0=job.write_half;u_wb0=job.write_byte;
    if(orig_ww) {u_ww0=orig_ww;u_wh0=orig_wh;u_wb0=orig_wb;}
    undo.clear();
    // As the composition does before the call: the eye it derives, the
    // visited bitmap and the list cleared (0x8002e594.., 0x8002e480), the
    // context fields (+0x6c..+0x78, as the earlier job had them).
    for(int k=0;k<3;++k) {put_word(camera_base+0x14+4*k,(uint32_t)in::pred_eye[k]);put_word(camera_base+0x34+4*k,(uint32_t)in::pred_eye[k]);}
    for(int k=0;k<4;++k) put_word(0x800e1f10u+4*k,0);
    put_word(0x800d68a8u,0);
    for(int k=0;k<4;++k) put_word(ctx_base+0x6c+4*k,prev_e.ctx[k]);
    // The view rectangle as the earlier job had it at the call (the
    // widescreen reveal widens it during the composition).
    put_word(0x800d2210u,prev_e.rect[0]);put_word(0x800d2214u,prev_e.rect[1]);
    CPUState c=job;
    std::memcpy(c.gte_data,prev_e.gte_d,sizeof c.gte_data);std::memcpy(c.gte_ctrl,prev_e.gte_c,sizeof c.gte_ctrl);
    c.write_word=u_ww;c.write_half=u_wh;c.write_byte=u_wb;
    c.pc=0;c.gpr[31]=sentinel;c.gpr[29]=mem+0xff0;
    pred_room=psx_mod_read_word(camera_base+0x90);
    c.gpr[4]=pred_room;c.gpr[5]=camera_base;c.gpr[6]=0x800d2210u;c.gpr[7]=mem;
    put_word(mem,0);
    psx_dispatch_call(&c,0x8006276c,sentinel);
    const bool ok=c.pc==0 && !g_psx_call_bail;
    g_psx_call_bail=0;
    if(ok) {predicted=read_list();have_prediction=true;++st.predicted;} else ++st.run_failed;
    pred_h=(int32_t)(uint16_t)prev_e.gte_c[26];pred_ofx=(int32_t)prev_e.gte_c[24];pred_ofy=(int32_t)prev_e.gte_c[25];
    rollback();
}
void portal_entry(const CPUState* cpu) {
    cur_e.valid=true;
    for(int k=0;k<4;++k) cur_e.ctx[k]=psx_mod_read_word(ctx_base+0x6c+4*k);
    cur_e.rect[0]=psx_mod_read_word(0x800d2210);cur_e.rect[1]=psx_mod_read_word(0x800d2214);
    std::memcpy(cur_e.gpr,cpu->gpr,sizeof cur_e.gpr);std::memcpy(cur_e.gte_d,cpu->gte_data,sizeof cur_e.gte_d);std::memcpy(cur_e.gte_c,cpu->gte_ctrl,sizeof cur_e.gte_c);
}
// The room walk's entry, before our host walk: the list and its inputs.
void walk_entry(const CPUState* cpu) {
    if(!have_prediction) return;
    have_prediction=false;
    ++st.checked;
    const List real=read_list();
    static unsigned shown=0;
    if(real==predicted) ++st.list_exact;
    else {
        if(real.count!=predicted.count) ++st.list_count_differs; else ++st.list_differs;
        if(shown++<12) {
            std::fprintf(stderr,"[n2 check %ld] portal list differs: count %u/%u",(long)getpid(),predicted.count,real.count);
            for(uint32_t i=0;i<std::max(real.count,predicted.count) && i<6;++i) std::fprintf(stderr," [%08x %08x %u | %08x %08x %u]",predicted.w[3*i],predicted.w[3*i+1],predicted.w[3*i+2],real.w[3*i],real.w[3*i+1],real.w[3*i+2]);
            std::fprintf(stderr,"\n");
        }
    }
    bool in_ok=true;
    const uint32_t cam=cpu->gpr[4];
    if(psx_mod_read_word(cam+0x90)!=pred_room) {++st.room_differs;in_ok=false;}
    bool cam_ok=cam==camera_base;
    for(int k=0;k<3 && cam_ok;++k) cam_ok=(int32_t)psx_mod_read_word(cam+0x14+4*k)==in::pred_eye[k];
    for(int k=0;k<9 && cam_ok;++k) cam_ok=(int16_t)psx_mod_read_half(cam+0x20+2*k)==in::pred_view[0][k];
    if(!cam_ok) {++st.cam_differs;in_ok=false;}
    if(psx_mod_read_word(0x800d2210)!=prev_e.rect[0] || psx_mod_read_word(0x800d2214)!=prev_e.rect[1]) {++st.rect_differs;in_ok=false;}
    if((int32_t)(uint16_t)cpu->gte_ctrl[26]!=pred_h || (int32_t)cpu->gte_ctrl[24]!=pred_ofx || (int32_t)cpu->gte_ctrl[25]!=pred_ofy) {++st.proj_differs;in_ok=false;}
    st.inputs_exact+=in_ok;
}
void at_flip(uint64_t serial,float alpha) {
    cur_e.serial=serial;cur_e.alpha=alpha;
    prev_e=cur_e;cur_e.valid=false;
}
void report() {
    std::fprintf(stderr,"[n2 check %ld] visibility: jobs=%llu without an earlier job of the frame=%llu predicted=%llu scratch run failed=%llu checked=%llu portal list exact=%llu (count differs %llu, entries differ %llu) | host walk inputs exact=%llu (room %llu, camera %llu, clip rectangle %llu, projection %llu differ)\n",
        (long)getpid(),(unsigned long long)st.jobs,(unsigned long long)st.no_prev,(unsigned long long)st.predicted,(unsigned long long)st.run_failed,(unsigned long long)st.checked,
        (unsigned long long)st.list_exact,(unsigned long long)st.list_count_differs,(unsigned long long)st.list_differs,(unsigned long long)st.inputs_exact,
        (unsigned long long)st.room_differs,(unsigned long long)st.cam_differs,(unsigned long long)st.rect_differs,(unsigned long long)st.proj_differs);
}
}  // namespace vis

namespace sk {
// D23N2B piece 5 (bit 128): the matrices a job builds from the camera (and
// static data) before calling the transform, built in host code and checked
// word for word at the transform's entry, before our callbacks. CONFIRMED by
// the disassembly (the sky routine 0x800388e4, camera a0 = s2):
//   ra 0x80038c68 (the backdrop): v = (cam +0xc, 0, cam +0x10) as VECTOR,
//     VectorNormal 0x800b3d3c, angles 0x80066f88 = (ratan2 0x800b530c(v.x,
//     v.z), -0x8002b564(v.y low half), 0), RotMatrix-like 0x8002afe4, t = eye;
//   ra 0x80038d7c: angles ((int16)(*0x800c0c64 >> 5), 0, 0), 0x8002afe4,
//     t = eye, ScaleMatrix 0x800b4b5c by (0x1000, 0xa00, 0x1000);
//   ra 0x80038e18: angles ((int16)(*0x800c0c64 >> 3), 0, 0), 0x8002afe4, t = eye.
// 0x8002afe4: rsin/rcos 0x800b3a9c/0x800b3b6c of the three angles (table
// 0x800c8758), products truncated toward zero (>> 12), then MatrixNormal
// 0x800b3e5c (GTE OP sf 1 twice, each row through the VectorNormal core
// 0x800b3d9c: SQR, LZCS, table 0x800c8f8c, GPF sf 0, >> (31 - lzc & ~1) / 2).
// The matrix's pad half (+0x12) is stack and not compared.
constexpr uint32_t sin_tbl=0x800c8758u,atan_tbl=0x800c9428u,acos_tbl=0x800beb48u,rsq_tbl=0x800c8f8cu,sky_timer=0x800c0c64u;
inline int32_t T(uint32_t base,int32_t i) {return (int16_t)psx_mod_read_half(base+2u*(uint32_t)i);}
int32_t sin1(int32_t a) {
    if(a<=0x800) return a<=0x400 ? T(sin_tbl,a) : T(sin_tbl,0x800-a);
    return a<=0xc00 ? -T(sin_tbl,a-0x800) : -T(sin_tbl,0x1000-a);
}
int32_t rsin(int32_t a) {return a<0 ? -sin1((int32_t)((0u-(uint32_t)a)&0xfffu)) : sin1(a&0xfff);}
int32_t rcos(int32_t a) {
    a=(int32_t)((a<0?0u-(uint32_t)a:(uint32_t)a)&0xfffu);
    if(a<=0x800) return a<=0x400 ? T(sin_tbl,0x400-a) : -T(sin_tbl,a-0x400);
    return a<=0xc00 ? -T(sin_tbl,0xc00-a) : T(sin_tbl,a-0xc00);
}
bool ratan2(int32_t y,int32_t x,int32_t& out) {
    const bool nx=x<0,ny=y<0;
    if(nx) x=-x; if(ny) y=-y;
    if(x==0 && y==0) {out=0;return true;}
    int32_t r;
    if(y<x) {
        int32_t t;
        if(y&0x7fe00000) {if(!(x>>10)) return false;t=y/(x>>10);} else t=(int32_t)((uint32_t)y<<10)/x;
        r=T(atan_tbl,t);
    } else {
        int32_t t;
        if(x&0x7fe00000) {if(!(y>>10)) return false;t=x/(y>>10);} else t=(int32_t)((uint32_t)x<<10)/y;
        r=0x400-T(atan_tbl,t);
    }
    if(nx) r=0x800-r;
    if(ny) r=-r;
    out=r;return true;
}
int32_t acos_like(int32_t a) {   // 0x8002b564
    a=(int16_t)a;
    if(a<-0x1000) a=-0x1000; else if(a>0x1000) a=0x1000;
    if(a<0) return (int16_t)(psx_mod_read_half(acos_tbl+2u*(uint32_t)(a+0x1000))-0x400);
    if(a==0) return 0;
    return (int16_t)(0x400-psx_mod_read_half(acos_tbl+2u*(uint32_t)(0x1000-a)));
}
inline int32_t q12(int32_t a,int32_t b) {const int32_t p=(int32_t)((uint32_t)a*(uint32_t)b);return (p<0?p+0xfff:p)>>12;}
inline int32_t lzc(int32_t v) {uint32_t u=v<0?~(uint32_t)v:(uint32_t)v;int n=0;while(n<32 && !(u&0x80000000u)) {u<<=1;++n;}return n;}
// The VectorNormal core on (x, y, z) as IR1-3 take them (low 16 bits).
void vnorm(int32_t& x,int32_t& y,int32_t& z) {
    const int64_t ix=(int16_t)x,iy=(int16_t)y,iz=(int16_t)z;
    const int32_t s=(int32_t)((uint32_t)(ix*ix)+(uint32_t)(iy*iy)+(uint32_t)(iz*iz));
    const int32_t v1=lzc(s)&~1,t6=(31-v1)>>1;
    const int32_t t4=v1-24>=0 ? (int32_t)((uint32_t)s<<(v1-24)) : s>>(24-v1);
    const int64_t f=T(rsq_tbl,t4-64);
    x=(int32_t)(f*ix)>>(t6&31);y=(int32_t)(f*iy)>>(t6&31);z=(int32_t)(f*iz)>>(t6&31);   // GPF sf 0 (0x4b90003d)
}
inline int16_t sat(int64_t v) {return (int16_t)std::clamp<int64_t>(v,-0x8000,0x7fff);}
// RotMatrix-like 0x8002afe4 with MatrixNormal: the 9 halves.
void rot(const int16_t ang[3],int16_t m[9]) {
    const int32_t sx=rsin(ang[0]),sy=rsin(ang[1]),sz=rsin(ang[2]),cx=rcos(ang[0]),cy=rcos(ang[1]),cz=rcos(ang[2]);
    const int32_t t1=q12(sz,sx),a3=q12(sz,cx),a1=q12(cz,sx),t0=q12(cz,cx);
    int16_t r[9];
    r[0]=(int16_t)(t0+q12(t1,sy)); r[1]=(int16_t)q12(sz,cy); r[2]=(int16_t)(-a1+q12(a3,sy));
    r[3]=(int16_t)(-a3+q12(a1,sy)); r[4]=(int16_t)q12(cz,cy); r[5]=(int16_t)(t1+q12(t0,sy));
    r[6]=(int16_t)q12(cy,cx); r[7]=(int16_t)-sy; r[8]=(int16_t)q12(cy,sx);
    // MatrixNormal: c = r0 x r1 (OP sf 1, IR saturated), r0' = r1 x c.
    auto op=[](const int64_t d[3],const int64_t ir[3],int64_t mac[3]) {
        mac[0]=(d[1]*ir[2]-d[2]*ir[1])>>12;mac[1]=(d[2]*ir[0]-d[0]*ir[2])>>12;mac[2]=(d[0]*ir[1]-d[1]*ir[0])>>12;};
    const int64_t r0[3]={r[0],r[1],r[2]},r1[3]={r[3],r[4],r[5]};
    int64_t c[3],ci[3],a[3];
    op(r0,r1,c);for(int k=0;k<3;++k) ci[k]=sat(c[k]);
    op(r1,ci,a);
    int32_t x=(int32_t)a[0],y=(int32_t)a[1],z=(int32_t)a[2];vnorm(x,y,z);m[0]=(int16_t)x;m[1]=(int16_t)y;m[2]=(int16_t)z;
    x=r[3];y=r[4];z=r[5];vnorm(x,y,z);m[3]=(int16_t)x;m[4]=(int16_t)y;m[5]=(int16_t)z;
    x=(int32_t)c[0];y=(int32_t)c[1];z=(int32_t)c[2];vnorm(x,y,z);m[6]=(int16_t)x;m[7]=(int16_t)y;m[8]=(int16_t)z;
}
// The host build for the call at ra from the camera block cam: 9 halves and t.
bool build(uint32_t ra,uint32_t cam,int16_t m[9],int32_t t[3]) {
    int16_t ang[3]={0,0,0};
    if(ra==0x80038c68u) {
        int32_t x=(int16_t)psx_mod_read_half(cam+0xc),y=0,z=(int16_t)psx_mod_read_half(cam+0x10);
        vnorm(x,y,z);
        int32_t a=0;if(!ratan2(x,z,a)) return false;
        ang[0]=(int16_t)a;ang[1]=(int16_t)-acos_like((int16_t)y);
    } else if(ra==0x80038d7cu) ang[0]=(int16_t)((int32_t)psx_mod_read_word(sky_timer)>>5);
    else if(ra==0x80038e18u) ang[0]=(int16_t)((int32_t)psx_mod_read_word(sky_timer)>>3);
    else return false;
    rot(ang,m);
    for(int k=0;k<3;++k) t[k]=(int32_t)psx_mod_read_word(cam+0x14+4*k);
    if(ra==0x80038d7cu) {
        uint16_t in[10],out[10];for(int k=0;k<9;++k) in[k]=(uint16_t)m[k];in[9]=0;
        const int32_t v[3]={0x1000,0xa00,0x1000};jt::scale(in,v,out);
        for(int k=0;k<9;++k) m[k]=(int16_t)out[k];
    }
    return true;
}
// ra 0x8003133c, in 0x80030fe8(a0 camera, a1 object s1, a2 height): CONFIRMED
// by its disassembly, not from the camera (note 471 section 3 inferred that):
// the object's record r = *(s1 +0x3c) + 32 * byte[*(s1 +0x40) +0x24]; v =
// (r +0xc, 0, r +0x10) when |r +0xe| < |r +0x8|, else (-(r +0x6), 0, -(r +0xa))
// (halves), VectorNormal; the yaw matrix (v.z 0 -v.x / 0 0x1000 0 / v.x 0 v.z),
// t = (r +0x14, a2, r +0x1c). At the transform's entry s1 is still the
// object and a2 is in the caller's frame (sp +0x2c).
bool build_obj(const CPUState* cpu,int16_t m[9],int32_t t[3]) {
    const uint32_t s1=cpu->gpr[17];
    const uint32_t r=psx_mod_read_word(s1+0x3c)+32u*psx_mod_read_byte(psx_mod_read_word(s1+0x40)+0x24);
    const int32_t e=std::abs((int32_t)(int16_t)psx_mod_read_half(r+0xe)),h8=std::abs((int32_t)(int16_t)psx_mod_read_half(r+0x8));
    int32_t x,y=0,z;
    if(e<h8) {x=(int16_t)psx_mod_read_half(r+0xc);z=(int16_t)psx_mod_read_half(r+0x10);}
    else {x=-(int32_t)(int16_t)psx_mod_read_half(r+0x6);z=-(int32_t)(int16_t)psx_mod_read_half(r+0xa);}
    vnorm(x,y,z);
    m[0]=(int16_t)z;m[1]=0;m[2]=(int16_t)-x;m[3]=0;m[4]=0x1000;m[5]=0;m[6]=(int16_t)x;m[7]=0;m[8]=(int16_t)z;
    t[0]=(int32_t)psx_mod_read_word(r+0x14);t[1]=(int32_t)psx_mod_read_word(cpu->gpr[29]+0x2c);t[2]=(int32_t)psx_mod_read_word(r+0x1c);
    return true;
}
// D23N2B piece 7: an object's own matrix, built on the caller's stack before
// the transform, CONFIRMED by the disassembly: 0x8002afe4 (the angles at
// obj +0x1c) with t = (obj +4, +8, +0xc); the actor draw 0x80032e78 (ra
// 0x80032f48, the actor s3) as it is; the prop draw (ra 0x80031dec, the object
// s2) scaled by ScaleMatrix (0x1000, half obj +0x22, 0x1000) unless that half
// is 0x1000.
bool build_pose(uint32_t obj,bool scaled,int16_t m[9],int32_t t[3]) {
    int16_t ang[3];for(int k=0;k<3;++k) ang[k]=(int16_t)psx_mod_read_half(obj+0x1c+2*k);
    rot(ang,m);
    for(int k=0;k<3;++k) t[k]=(int32_t)psx_mod_read_word(obj+4+4*k);
    const int32_t sy=(int16_t)psx_mod_read_half(obj+0x22);
    if(scaled && sy!=0x1000) {
        uint16_t in[10],out[10];for(int k=0;k<9;++k) in[k]=(uint16_t)m[k];in[9]=0;
        const int32_t v[3]={0x1000,sy,0x1000};jt::scale(in,v,out);
        for(int k=0;k<9;++k) m[k]=(int16_t)out[k];
    }
    return true;
}
bool build_any(const CPUState* cpu,int16_t m[9],int32_t t[3]) {
    const uint32_t ra=cpu->gpr[31];
    if(ra==0x80032f48u) return build_pose(cpu->gpr[19],false,m,t);
    if(ra==0x80031decu) return build_pose(cpu->gpr[18],true,m,t);
    if(ra==0x8003133cu) return build_obj(cpu,m,t);
    if(ra==0x80038c68u || ra==0x80038d7cu || ra==0x80038e18u) return build(ra,cpu->gpr[4],m,t);
    return false;
}
struct RaStat {uint32_t ra=0;uint64_t exact=0,differ=0,failed=0;};
RaStat st[4];unsigned shown=0;
void entry(const CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    unsigned i=ra==0x80038c68u?0:ra==0x80038d7cu?1:ra==0x80038e18u?2:ra==0x8003133cu?3:4;
    if(i==4) return;
    st[i].ra=ra;
    int16_t m[9];int32_t t[3];
    if(!(i==3 ? build_obj(cpu,m,t) : build(ra,cpu->gpr[4],m,t))) {++st[i].failed;return;}
    const uint32_t mat=cpu->gpr[5];
    bool same=true;
    for(int k=0;k<9;++k) same&=(int16_t)psx_mod_read_half(mat+2*k)==m[k];
    for(int k=0;k<3;++k) same&=(int32_t)psx_mod_read_word(mat+0x14+4*k)==t[k];
    ++(same?st[i].exact:st[i].differ);
    if(!same && shown<6) {
        ++shown;std::fprintf(stderr,"[n2 check %ld]   built matrix differs at ra %08x: host",(long)getpid(),ra);
        for(int k=0;k<9;++k) std::fprintf(stderr," %d",m[k]);
        std::fprintf(stderr," | job");
        for(int k=0;k<9;++k) std::fprintf(stderr," %d",(int16_t)psx_mod_read_half(mat+2*k));
        std::fprintf(stderr," | t %d %d %d / %d %d %d\n",t[0],t[1],t[2],(int32_t)psx_mod_read_word(mat+0x14),(int32_t)psx_mod_read_word(mat+0x18),(int32_t)psx_mod_read_word(mat+0x1c));
    }
}
void report() {
    std::fprintf(stderr,"[n2 check %ld] piece 5, camera-built matrices on the host:",(long)getpid());
    for(const RaStat& s:st) if(s.ra) std::fprintf(stderr," %08x exact %llu differ %llu failed %llu |",s.ra,(unsigned long long)s.exact,(unsigned long long)s.differ,(unsigned long long)s.failed);
    std::fprintf(stderr,"\n");
}
}  // namespace sk

// Called from the D23N1 observer for each top-level hook entry.
void n2_observe(CPUState* cpu,uint32_t addr,int phase) {
    if((n2_mask()&128) && phase==0 && addr==0x800292a0u) sk::entry(cpu);
    if(n2_mask()&1) {
        if(phase==0) xf::settle(cpu,addr);
        else if(addr==0x800292a0u) xf::on_entry_done(cpu);
    }
    if((n2_mask()&2) && phase==1 && addr==0x800292a0u) in::on_entry_done(cpu);
    if((n2_mask()&8) && addr==0x800292a0u) {if(phase==0) jt::entry(cpu); else jt::entry_done(cpu);}
    if((n2_mask()&16) && addr==0x800292a0u && phase==1) jr::entry_done(cpu);
    if(n2_mask()&4) {
        if(phase==1 && addr==0x8006276cu && cpu->gpr[31]==0x8002e650u) vis::portal_entry(cpu);
        if(phase==0 && addr==0x80062b48u && cpu->gpr[31]==0x8002e688u) vis::walk_entry(cpu);
    }
}
void n2_finish(bool ok) {if(n2_mask()&1024) wj::finish(ok);}
void n2_begin(const CPUState& job) {
    if(n2_mask()&14) in::begin(job);
    if(n2_mask()&4) vis::begin(job,n2_serial,n2_alpha);
    if(n2_mask()&16) jr::begin();
    if(n2_mask()&1024) wj::begin();
}
void n2_at_flip(uint64_t serial,float alpha) {
    static unsigned jobs=0;
    ++jobs;
    if(n2_mask()&1) {
        xf::settle(nullptr,0);
        if(jobs%64==0) xf::report();
    }
    if(n2_mask()&2) {
        if(in::prev_serial==serial && in::prev_alpha!=alpha) in::compare(alpha);
        in::prev.swap(in::cur); in::prev_serial=serial; in::prev_alpha=alpha;
        if(jobs%64==0) in::report();
    }
    if(n2_mask()&4) {
        vis::at_flip(serial,alpha);
        if(jobs%64==0) vis::report();
    }
    if((n2_mask()&8) && jobs%64==0) jt::report();
    if(n2_mask()&16) {jr::at_flip();if(jobs%64==0) jr::report();}
    if(n2_mask()&1024) {wj::at_flip();if(jobs%64==0) wj::report();}
    if((n2_mask()&128) && jobs%64==0) sk::report();
}
