// D08A47: the Holy Hand Grenade held and thrown looks like its pickup
// (Modernized; Vanilla never runs it). Included in modern_controls.cpp after
// crystal_slots.inc. Note 520.
//
// The grenade has three models. Weapon slot 13 (table 0x800c4590, 44 bytes a
// slot) names the held model at +0xe (945, drawn on Duke's hand by
// 0x80033e40) and the projectile type at +0 (11); projectile type 11 (table
// 0x800c4bc0, 64 bytes a type) names the thrown model at +6 (944, drawn by
// 0x8003177c from the projectile draw loop 0x8006d08c). The pickup type 608
// (type table +0x10) names model 1129. The held and thrown models are one
// 42-vertex, 26-face mesh (held = 0.67 x a quarter turn of thrown, in the
// hand's frame) textured from a small dark swatch (CLUT 0x193a); the pickup
// is a different 35-vertex, 23-face mesh in gold (CLUTs 0xc7a/0xcba, the same
// texture page). So the held and thrown grenade never looked like the one
// Duke picks up.
//
// Here the held and thrown meshes are rewritten in place (main RAM, inside
// their own vertex and face storage, which the pickup's fit) with the pickup's
// faces and vertices: the thrown mesh gets the pickup scaled to the thrown
// mesh's original height, standing where it stood (base at its base, centred
// across); the held mesh gets that with the transform that took the thrown
// mesh to the held one, solved from their 42 matching vertices (least squares,
// refused unless every vertex fits within 8 units). Faces, texture and CLUTs
// are the pickup's own, so the grenade in the hand, in flight and on the floor
// is one object at the game's own sizes. Models load with each level, so this
// runs again after every level or savestate load (a converted mesh is
// recognised and left alone). Developer: DNTTK_HHG_LOOK=0 off.
static uint64_t hhg_converts=0,hhg_refusals=0;
static double hhg_fit_error=0;
static void hhg_refuse(int why) {
    ++hhg_refusals;
    static int last=-1;
    if(why!=last) {std::fprintf(stderr,"[TTK hhg] not converted (%d)\n",why);last=why;}
}
static bool hhg_ram(uint32_t a,uint32_t n){return a>=0x80010000u && a+n<=0x80200000u && !(a&3);}
struct HhgMesh {uint32_t mesh=0,vp=0,faces=0,face_bytes=0;unsigned nv=0;};
static bool hhg_mesh(uint32_t models,unsigned id,HhgMesh& m) {
    if(!id || id>4096)return false;
    m.mesh=psx_mod_read_word(models+4*id);
    if(!hhg_ram(m.mesh,0x18))return false;
    m.nv=psx_mod_read_byte(m.mesh+6);m.vp=psx_mod_read_word(m.mesh+0x10);m.faces=psx_mod_read_word(m.mesh+0x14);
    if(!m.nv || !hhg_ram(m.vp,8*m.nv) || !hhg_ram(m.faces,4) || !hhg_ram(psx_mod_read_word(m.mesh+8),16))return false;
    uint32_t cur=m.faces;
    for(unsigned guard=0;guard<64;++guard) {
        if(!hhg_ram(cur,4))return false;
        const uint32_t header=psx_mod_read_word(cur);cur+=4;
        const uint32_t type=header&0xffff,n=header>>16;
        if(type==0xff){m.face_bytes=cur-m.faces;return true;}
        unsigned size=0;
        switch(type){case 0x20:case 0x28:size=8;break;case 0x30:case 0x38:size=12;break;case 0x24:case 0x2c:size=16;break;case 0x34:case 0x3c:size=20;break;}
        if(!size || !n || n>1024 || !hhg_ram(cur,n*size))return false;
        cur+=n*size;
    }
    return false;
}
// True when a textured face of the mesh names CLUT 0xffff: the level's model
// loader found no texture for it (D61B).
static bool hhg_unresolved(const HhgMesh& m) {
    for(uint32_t cur=m.faces;cur<m.faces+m.face_bytes;) {
        const uint32_t header=psx_mod_read_word(cur);cur+=4;
        const uint32_t type=header&0xffff,n=header>>16;
        if(type==0xff)break;
        const unsigned size=(type==0x20||type==0x28)?8:(type==0x30||type==0x38)?12:(type==0x24||type==0x2c)?16:20;
        if(type==0x24 || type==0x2c)
            for(uint32_t k=0;k<n;++k) if((psx_mod_read_word(cur+size*k+4)>>16)==0xffffu) return true;
        cur+=n*size;
    }
    return false;
}
static void hhg_read_verts(const HhgMesh& m,std::vector<std::array<double,4>>& v) {
    v.resize(m.nv);
    for(unsigned i=0;i<m.nv;++i) for(int k=0;k<4;++k) v[i][k]=(int16_t)psx_mod_read_half(m.vp+8*i+2*k);
}
// Least-squares affine map a -> b (rows: 3 outputs, columns x, y, z, 1).
static bool hhg_affine(const std::vector<std::array<double,4>>& a,const std::vector<std::array<double,4>>& b,double M[3][4]) {
    double N[4][4]={},R[4][3]={};
    for(size_t i=0;i<a.size();++i) {
        const double x[4]={a[i][0],a[i][1],a[i][2],1};
        for(int r=0;r<4;++r){for(int c=0;c<4;++c)N[r][c]+=x[r]*x[c];for(int k=0;k<3;++k)R[r][k]+=x[r]*b[i][k];}
    }
    for(int col=0;col<4;++col) {   // Gauss-Jordan with partial pivoting
        int p=col;for(int r=col+1;r<4;++r)if(std::fabs(N[r][col])>std::fabs(N[p][col]))p=r;
        if(std::fabs(N[p][col])<1e-9)return false;
        std::swap(N[p],N[col]);std::swap(R[p],R[col]);
        for(int r=0;r<4;++r) {
            if(r==col)continue;
            const double f=N[r][col]/N[col][col];
            for(int c=0;c<4;++c)N[r][c]-=f*N[col][c];
            for(int k=0;k<3;++k)R[r][k]-=f*R[col][k];
        }
    }
    for(int k=0;k<3;++k)for(int c=0;c<4;++c)M[k][c]=R[c][k]/N[c][c];
    return true;
}
static void hhg_write(const HhgMesh& to,const HhgMesh& pick,const std::vector<std::array<double,4>>& v) {
    double lo[3]={1e9,1e9,1e9},hi[3]={-1e9,-1e9,-1e9},r2=0;
    for(unsigned i=0;i<pick.nv;++i) {
        int16_t q[3];
        for(int k=0;k<3;++k){q[k]=int16_t(std::lround(std::clamp(v[i][k],-32768.0,32767.0)));lo[k]=std::min(lo[k],double(q[k]));hi[k]=std::max(hi[k],double(q[k]));}
        for(int k=0;k<3;++k)psx_mod_write_half(to.vp+8*i+2*k,uint16_t(q[k]));
        psx_mod_write_half(to.vp+8*i+6,uint16_t(int16_t(v[i][3])));
        r2=std::max(r2,double(q[0])*q[0]+double(q[1])*q[1]+double(q[2])*q[2]);
    }
    for(uint32_t off=0;off<pick.face_bytes;off+=4)psx_mod_write_word(to.faces+off,psx_mod_read_word(pick.faces+off));
    psx_mod_write_byte(to.mesh+6,uint8_t(pick.nv));
    (void)r2;
    // Bounds as the game stores them: min x, y, z, the radius across (x, z),
    // max x, y, z, the radius of the box corner (also header word 0's top half).
    const uint32_t b=psx_mod_read_word(to.mesh+8);
    const double ax=std::max(-lo[0],hi[0]),ay=std::max(-lo[1],hi[1]),az=std::max(-lo[2],hi[2]);
    const int across=int(std::sqrt(ax*ax+az*az)),radius=int(std::sqrt(ax*ax+ay*ay+az*az));
    const int16_t bw[8]={int16_t(lo[0]),int16_t(lo[1]),int16_t(lo[2]),int16_t(across),int16_t(hi[0]),int16_t(hi[1]),int16_t(hi[2]),int16_t(radius)};
    for(int k=0;k<8;++k)psx_mod_write_half(b+2*k,uint16_t(bw[k]));
    psx_mod_write_half(to.mesh+2,uint16_t(radius));   // header word 0's high half: the radius
}
static void hhg_look_update() {
    static const bool enabled=[]{const char* t=std::getenv("DNTTK_HHG_LOOK");return !t || std::strcmp(t,"0");}();
    if(!enabled)return;
    const uint32_t models=psx_mod_read_word(0x800ddb18),types=psx_mod_read_word(0x800d2660);
    if(!hhg_ram(models,4*4096) || !hhg_ram(types,28*1062))return;
    const uint32_t slot=0x800c4590+44*13;
    const unsigned held_id=psx_mod_read_half(slot+0xe),projectile=psx_mod_read_half(slot);
    if(projectile>=64)return;
    const unsigned thrown_id=psx_mod_read_half(0x800c4bc0+64*projectile+6),pick_id=psx_mod_read_half(types+28*608+0x10);
    HhgMesh held,thrown,pick;
    if(!hhg_mesh(models,held_id,held) || !hhg_mesh(models,thrown_id,thrown) || !hhg_mesh(models,pick_id,pick)) {
        static uint32_t told=0;if(told!=held_id*7919u+thrown_id*31u+pick_id){told=held_id*7919u+thrown_id*31u+pick_id;std::fprintf(stderr,"[TTK hhg] meshes not ready (held %u thrown %u pickup %u)\n",held_id,thrown_id,pick_id);}
        return;
    }
    if(held.mesh==pick.mesh || thrown.mesh==pick.mesh)return;
    // D61B: a level whose texture set lacks the pickup's art (Modern 4, the
    // animation test) loads it with unresolved faces, CLUT 0xffff on page
    // 0x20: copied onto Duke's hand they drew magenta. Keep the original
    // grenade there.
    if(hhg_unresolved(pick)) {hhg_refuse(6);return;}
    // Already converted (this level load): the held mesh's faces are the pickup's.
    if(held.nv==pick.nv && held.face_bytes==pick.face_bytes && psx_mod_read_word(held.faces+4)==psx_mod_read_word(pick.faces+4) &&
       thrown.nv==pick.nv && psx_mod_read_word(thrown.faces+4)==psx_mod_read_word(pick.faces+4))return;
    // The originals: one mesh in two frames; the pickup must fit their storage.
    if(held.nv!=thrown.nv || held.face_bytes!=thrown.face_bytes || pick.nv>held.nv || pick.face_bytes>held.face_bytes ||
       held.faces<held.vp+8*pick.nv || thrown.faces<thrown.vp+8*pick.nv) {hhg_refuse(1);return;}
    // Same faces on the same vertices (texture words may differ: the thrown
    // copy has some faces on texture page 0x22b, the held 0x2b).
    for(uint32_t a=held.faces,b=thrown.faces;;) {
        const uint32_t ha=psx_mod_read_word(a),hb=psx_mod_read_word(b);
        if(ha!=hb) {hhg_refuse(2);return;}
        a+=4;b+=4;
        const uint32_t type=ha&0xffff,n=ha>>16;
        if(type==0xff)break;
        const unsigned size=(type==0x20||type==0x28)?8:(type==0x30||type==0x38)?12:(type==0x24||type==0x2c)?16:20;
        for(uint32_t k=0;k<n;++k,a+=size,b+=size)
            if(psx_mod_read_word(a)!=psx_mod_read_word(b)) {hhg_refuse(2);return;}
    }
    std::vector<std::array<double,4>> vh,vt,vp;
    hhg_read_verts(held,vh);hhg_read_verts(thrown,vt);hhg_read_verts(pick,vp);
    double M[3][4];
    if(!hhg_affine(vt,vh,M)) {hhg_refuse(3);return;}
    double worst=0;
    for(size_t i=0;i<vt.size();++i) for(int k=0;k<3;++k)
        worst=std::max(worst,std::fabs(M[k][0]*vt[i][0]+M[k][1]*vt[i][1]+M[k][2]*vt[i][2]+M[k][3]-vh[i][k]));
    hhg_fit_error=worst;
    if(worst>8) {hhg_refuse(4);return;}
    // The pickup in the thrown frame: the thrown mesh's height, base and centre.
    double tlo[3]={1e9,1e9,1e9},thi[3]={-1e9,-1e9,-1e9},plo[3]={1e9,1e9,1e9},phi[3]={-1e9,-1e9,-1e9};
    for(auto& q:vt)for(int k=0;k<3;++k){tlo[k]=std::min(tlo[k],q[k]);thi[k]=std::max(thi[k],q[k]);}
    for(auto& q:vp)for(int k=0;k<3;++k){plo[k]=std::min(plo[k],q[k]);phi[k]=std::max(phi[k],q[k]);}
    if(phi[1]-plo[1]<1) {hhg_refuse(5);return;}
    const double s=(thi[1]-tlo[1])/(phi[1]-plo[1]);
    std::vector<std::array<double,4>> in_thrown(vp.size()),in_hand(vp.size());
    for(size_t i=0;i<vp.size();++i) {
        in_thrown[i][0]=(vp[i][0]-0.5*(plo[0]+phi[0]))*s+0.5*(tlo[0]+thi[0]);
        in_thrown[i][1]=(vp[i][1]-phi[1])*s+thi[1];
        in_thrown[i][2]=(vp[i][2]-0.5*(plo[2]+phi[2]))*s+0.5*(tlo[2]+thi[2]);
        in_thrown[i][3]=vp[i][3];
        for(int k=0;k<3;++k)in_hand[i][k]=M[k][0]*in_thrown[i][0]+M[k][1]*in_thrown[i][1]+M[k][2]*in_thrown[i][2]+M[k][3];
        in_hand[i][3]=vp[i][3];
    }
    hhg_write(thrown,pick,in_thrown);
    hhg_write(held,pick,in_hand);
    ++hhg_converts;
    std::fprintf(stderr,"[TTK hhg] held %u and thrown %u now the pickup %u (scale %.3f, fit %.1f)\n",held_id,thrown_id,pick_id,s,worst);
}
