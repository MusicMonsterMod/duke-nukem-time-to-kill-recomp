// D49E: the Laser Gatling's bolts leave the drawn barrel.
// Included by frame_replay.cpp after the D49 beam code.
//
// The Laser Gatling (held slot 7 with the upgrade bit, record 28) launches
// projectile type 27 through 0x800718ac at Duke's muzzle point player
// +0xbc..+0xc4 (written by 0x80032364 in Duke's draw). Pool 0x800da978, 0x60
// bytes a round: +4 position, +0x10 owner, +0x14 previous position, +0x24
// yaw / +0x26 pitch, +0x30 type, +0x34 unit direction x4096, +0x40 distance
// flown, +0x46 drawn length. A bolt moves 2000 units a game tick. The type
// table 0x800c4bc0 (+6 model 0x76a, flag 0x1000 stretched) makes the
// projectile draw loop 0x8006d08c draw it through 0x8003177c as a streak:
// the matrix from the angles (0x8002afe4), its row 2 (the model z axis, the
// draw takes the transpose) scaled by +0x46/4096 (0x800b474c), the
// translation the bolt's position, loaded by 0x800292a0 (return 0x80031900).
// The streak reaches +0x46 back from the bolt, so on its first drawn tick it
// runs from the launch point to the bolt.
//
// First person shows the gun in its own presentation pose (D12/D36), so that
// launch point, the third-person skeleton's muzzle, sits under and beside the
// drawn gun: the bolt's tail came out of the gun's side. A shot from rest
// (the first of a press) launches from the previous frame's rest-pose muzzle
// (D47, note 422), about 800 units off in third person too.
//
// Modernized: at that transform load, a first-tick bolt of Duke's (distance
// flown not past its drawn length) is drawn from the drawn barrel tip to its
// real position: first person, the tip Duke's draw recorded (D49
// first_person_drawn_muzzle, the muzzle flash's point) under this draw's
// camera; third person, the muzzle this draw wrote. Only the transform copy
// on the draw's stack (or a redraw's substitute) is changed: the head stays
// at the bolt's real position, so its flight, hits, D41J lights and the
// game's records are untouched. Live frames and redraw workers alike; after
// the frame replay's own record/substitute step. Vanilla is unchanged.
// DNTTK_BOLT_BARREL=0 or console "boltbarrel off" turns it off (A/B); DNTTK_BOLT_BARREL_TRACE=1
// logs each of Duke's bolt draws (native tail, barrel, offset seen from the eye).
constexpr uint32_t bolt_draw_ret=0x80031900u, bolt_type=27u;
// Live decision (Modernized, console "boltbarrel" override, else the
// environment); a redraw worker takes it from its job (flag 64).
std::atomic<int> bolt_barrel_override{-1};
bool bolt_barrel_job=false;
bool bolt_barrel_enabled() {
    if(!input_modernized()) return false;
    const int o=bolt_barrel_override.load(std::memory_order_relaxed);
    if(o>=0) return o!=0;
    const char* v=ttk_live_env("DNTTK_BOLT_BARREL");
    return !(v && v[0]=='0' && !v[1]);
}
uint64_t bolt_moves=0;
void bolt_at_barrel(CPUState* cpu) {
    if(cpu->gpr[31]!=bolt_draw_ret || cpu->gpr[4]!=camera_base) return;
    const uint32_t a=cpu->gpr[19];   // 0x8003177c keeps the round in s3
    if(a<0x800da978u || a>=0x800da978u+96u*0x60u || (a-0x800da978u)%0x60u) return;
    if(psx_mod_read_byte(a+0x30)!=bolt_type || psx_mod_read_word(a+0x10)!=duke) return;
    if(psx_mod_replay_active() ? !bolt_barrel_job : !bolt_barrel_enabled()) return;
    const int32_t flown=(int32_t)psx_mod_read_word(a+0x40);
    const int len=(int16_t)psx_mod_read_half(a+0x46);
    static const bool trace=std::getenv("DNTTK_BOLT_BARREL_TRACE")!=nullptr;
    const uint32_t m=cpu->gpr[5];
    if(!((m>=0x80010000u && m+0x20u<=0x80200000u) || (m&0x1fffffffu)>=0x1f800000u)) return;
    double head[3],u[3],un=0;
    for(int k=0;k<3;++k) {head[k]=(int32_t)psx_mod_read_word(a+4+4*k);u[k]=(int16_t)psx_mod_read_half(a+0x34+4*k);un+=u[k]*u[k];}
    un=std::sqrt(un);
    if(flown<=0 || len<=0 || flown>len || un<1024) {
        if(trace) std::fprintf(stderr,"[TTK bolt] %08x skip flown %d len %d replay %d\n",a,flown,len,psx_mod_replay_active());
        return;
    }
    for(int k=0;k<3;++k) u[k]/=un;
    // The drawn barrel tip in the world.
    double q[3];
    double vm[3];
    const bool fp=first_person_drawn_muzzle(vm);
    if(fp) {
        double rl[9];
        for(int r=0;r<3;++r) {
            double n=0; for(int k=0;k<3;++k) {const double x=(int16_t)psx_mod_read_half(camera_base+2*(3*r+k)); n+=x*x;}
            n=std::sqrt(n); if(n<1024) return;
            for(int k=0;k<3;++k) rl[3*r+k]=(int16_t)psx_mod_read_half(camera_base+2*(3*r+k))/n;
        }
        for(int k=0;k<3;++k) q[k]=(int32_t)psx_mod_read_word(camera_base+0x14+4*k)+rl[k]*vm[0]+rl[3+k]*vm[1]+rl[6+k]*vm[2];
    } else for(int k=0;k<3;++k) q[k]=(int32_t)psx_mod_read_word(duke+0xbc+4*k);
    // The transform's translation is the drawn head (a redraw may have
    // substituted its blended copy); its model z axis lies along the flight.
    double mm[9],t[3];
    for(int i=0;i<9;++i) mm[i]=(int16_t)psx_mod_read_half(m+2*i);
    for(int k=0;k<3;++k) t[k]=(int32_t)psx_mod_read_word(m+0x14+4*k);
    // The streak's axis: 0x800b474c scales the rows, and the draw uses the
    // transpose, so row 2 is the model z axis in the world (along the flight).
    double cz[3],cn=0,dot=0;
    for(int k=0;k<3;++k) {cz[k]=mm[6+k];cn+=cz[k]*cz[k];}
    cn=std::sqrt(cn);
    if(cn<16) return;
    for(int k=0;k<3;++k) dot+=cz[k]/cn*u[k];
    double hd=0;for(int k=0;k<3;++k) hd+=(t[k]-head[k])*(t[k]-head[k]);
    double v[3],vn=0;
    for(int k=0;k<3;++k) {v[k]=t[k]-q[k];vn+=v[k]*v[k];}
    vn=std::sqrt(vn);
    if(trace) {
        double tail[3],td=0;
        double e[3],dq[3],dt[3],nq=0,nt=0,dd=0;
        for(int k=0;k<3;++k) {tail[k]=(int32_t)psx_mod_read_word(a+0x14+4*k);td+=(q[k]-tail[k])*(q[k]-tail[k]);
            e[k]=(int32_t)psx_mod_read_word(camera_base+0x14+4*k);dq[k]=q[k]-e[k];dt[k]=tail[k]-e[k];nq+=dq[k]*dq[k];nt+=dt[k]*dt[k];dd+=dq[k]*dt[k];}
        // Seen from the eye: degrees between the drawn barrel and the native tail.
        const double seen=std::acos(std::clamp(dd/std::sqrt(nq*nt+1e-9),-1.0,1.0))*57.29577951308232;
        // D49F straightness: the bend where the first tick's streak (drawn
        // barrel to head) meets the flight line (u), and how far the drawn
        // barrel sits off that line.
        double bh[3],bn=0,bu=0,off=0;
        for(int k=0;k<3;++k) {bh[k]=head[k]-q[k];bn+=bh[k]*bh[k];bu+=bh[k]*u[k];}
        bn=std::sqrt(bn);
        for(int k=0;k<3;++k) {const double x=q[k]-tail[k];double along=0;for(int j=0;j<3;++j) along+=(q[j]-tail[j])*u[j];const double y=x-along*u[k];off+=y*y;}
        const double kink=bn>1 ? std::acos(std::clamp(bu/bn,-1.0,1.0))*57.29577951308232 : 0.0;
        std::fprintf(stderr,"[TTK bolt] %08x flown %d len %d replay %d fp %d head %.0f,%.0f,%.0f xf %.0f tail %.0f,%.0f,%.0f barrel %.0f,%.0f,%.0f (%.0f) zrow %.0f dot %.3f new %.0f seen %.2f kink %.2f off %.0f\n",
            a,flown,len,psx_mod_replay_active(),fp?1:0,head[0],head[1],head[2],std::sqrt(hd),tail[0],tail[1],tail[2],q[0],q[1],q[2],std::sqrt(td),cn,dot,vn,seen,kink,std::sqrt(off));
    }
    if(std::fabs(dot)<0.95 || hd>600.0*600.0 || vn<64 || vn>4.0*len) return;
    for(int k=0;k<3;++k) v[k]/=vn;
    // Rotate the transform from u onto v (Rodrigues), then stretch its z
    // axis from the drawn length to the head-to-barrel distance.
    const double c=u[0]*v[0]+u[1]*v[1]+u[2]*v[2];
    if(c<0.2) return;
    const double kx=u[1]*v[2]-u[2]*v[1],ky=u[2]*v[0]-u[0]*v[2],kz=u[0]*v[1]-u[1]*v[0],h=1.0/(1.0+c);
    const double Q[9]={c+h*kx*kx,h*kx*ky-kz,h*kx*kz+ky,
                       h*kx*ky+kz,c+h*ky*ky,h*ky*kz-kx,
                       h*kx*kz-ky,h*ky*kz+kx,c+h*kz*kz};
    // Row form: M' = M Q^T (the world map M^T turned by Q), row 2 stretched.
    const double f=vn/len;
    for(int r=0;r<3;++r) for(int col=0;col<3;++col) {
        double x=mm[3*r]*Q[3*col]+mm[3*r+1]*Q[3*col+1]+mm[3*r+2]*Q[3*col+2];
        if(r==2) x*=f;
        psx_mod_write_half(m+2*(3*r+col),(uint16_t)(int16_t)std::clamp(std::lround(x),-32768L,32767L));
    }
    ++bolt_moves;
}
