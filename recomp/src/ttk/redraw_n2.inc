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
//   4  rooms and visibility (pieces 2 and 3): the game's portal walk
//      0x8006276c run at the later job's start on a scratch CPU (its writes
//      undone afterwards) with the predicted camera, and the room selection
//      inputs of our host walk (camera, source room, clip rectangle, GTE
//      projection, and the view rectangle the earlier job had at the call),
//      against what the job really had at the room walk entry.
// Results: "[n2 check]" lines on stderr.
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

// Called from the D23N1 observer for each top-level hook entry.
void n2_observe(CPUState* cpu,uint32_t addr,int phase) {
    if(n2_mask()&1) {
        if(phase==0) xf::settle(cpu,addr);
        else if(addr==0x800292a0u) xf::on_entry_done(cpu);
    }
    if((n2_mask()&2) && phase==1 && addr==0x800292a0u) in::on_entry_done(cpu);
    if(n2_mask()&4) {
        if(phase==1 && addr==0x8006276cu && cpu->gpr[31]==0x8002e650u) vis::portal_entry(cpu);
        if(phase==0 && addr==0x80062b48u && cpu->gpr[31]==0x8002e688u) vis::walk_entry(cpu);
    }
}
uint64_t n2_serial=0; float n2_alpha=0;
void n2_begin(const CPUState& job) {
    if(n2_mask()&6) in::begin(job);
    if(n2_mask()&4) vis::begin(job,n2_serial,n2_alpha);
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
}
