// D17AT: the game's small-model renderer (blood pools, and other simple
// meshes drawn through 0x8002e02c from 0x800a27bc). Included inside
// near_clip.cpp's anonymous namespace after water_clip.inc.
//
// 0x8002e02c sets the model's GTE transform (0x8002df20), projects its
// vertices into 8-byte records (0x8002dce0: SXY, SZ >> 2, clip flags) and
// draws its faces with 0x8002d848 through the generic emitters
// 0x8002c000..0x8002d508, each linked into the ordering table by 0x8002bd48
// at SZ >> 5. None of it passed through the host renderer: since D17AM the
// floor under a blood pool is host geometry with depth, while the pool stayed
// a painter-only native packet. A floor polygon the original sorts nearer
// (another polygon across a floor edge) painted over it, and which one won
// changed as Duke moved: the blood popped in and out (slots 7 and 11). Faces
// with a corner nearer than the renderer's near limit (clip flag 0x10) were
// also dropped when the face asks for it (flag 0x400): blood under Duke's
// feet vanished.
//
// While the near path is live, each vertex is projected again here from the
// model data and the live GTE state and must match the game's record exactly,
// or the model stays as the game draws it. Near flags are cleared so every
// face is built; when a face reaches 0x8002bd48 its packet's corners are
// matched to the model's vertices and the face is drawn by the host from
// their view-space positions: same colours, texture, semi-transparency and
// ordering slot, host clipping, plane depth and the prop contact allowance
// (D17D), like any other prop. The native packet is then left as a
// zero-length link. A face the host cannot take stays native, unless it has
// a near corner (its native corners are saturated), which is dropped.
// DNTTK_MODEL_DEPTH=0 (developer) leaves these models to the game.
uint64_t model_seen=0,model_taken=0,model_mismatch=0,model_near_cleared=0,model_flat=0,model_sides_cleared=0;
uint64_t model_faces=0,model_faces_decal=0,model_faces_taken=0,model_faces_native=0,model_faces_dropped=0,model_faces_budget=0;
bool model_render_identity() {
    struct Guard {uint32_t address,size;const char* digest;};
    static const Guard guards[]={
        {0x8002bd48u,0x1b00u,"6116a797bf00bb5f68f19b40760f9beb01607d5d28e6f498ea9a47db9fa18165"},
        {0x8002d848u,0x838u,"710946d7ac90d8ce2bc90760e408ef2744943c6af1bad5863ce3f9e6240d4ce4"}};
    static std::array<std::vector<uint32_t>,2> words;
    static IdentityMemo memo;
    if(!memo.valid(input_host_frame(),g_dirty_ram_code_gen))
        memo.set(input_host_frame(),g_dirty_ram_code_gen,
            code_identity(guards,words,psx_mod_read_word,g_psx_ram));
    return memo.ok;
}
bool model_live() {
    static const bool on=env_int("DNTTK_MODEL_DEPTH",1,0,1)!=0;
    return on && input_modernized() && enabled() && unified() &&
        (first_person_view_live() || third_person_camera_live() || widescreen_near_clip_live()) &&
        identity() && model_render_identity();
}
// The model being drawn by 0x8002d848: its vertices in view space, and which
// of them the game flagged near (their projected corners are saturated).
struct ModelDraw {
    bool live=false;
    NearGte g{};
    uint32_t records=0;
    std::vector<NearProjected> verts;
    std::vector<bool> near;
    // Every vertex on one plane (a blood blob): drawn as a decal.
    bool flat=false;
    // D17AT9: the decal's ordering slot (see model_link).
    int slot=0;
    // D17AT12: the decal's fixed rank among overlapping blobs (see model_link).
    unsigned rank=0;
    uint32_t object=0; // the game object (trace identity)
    // D17AT8: faces the game tests with screen-space NCLIP (flag 0x200).
    struct Face {uint8_t corners;uint8_t idx[4];bool facing;};
    std::vector<Face> faces;
} model_draw;
// D17AT8: 0x8002d604, the facing test of a face flagged 0x200, drops it when
// its screen NCLIP is under 2 (back-facing) and also when it is over 120000:
// a face too big on screen, a limit for the original's small view. A large
// wall blob stretched across the wide 16:9, FOV 90 view, partly past the edge
// (corners clamped), passed it and the whole blob vanished (bloody.mp4,
// 8.4 s against 8.8 s). While a live model's faces are built the flag is
// cleared in its face flags (restored when the model ends, so the model data
// is unchanged for everything else) and the host tests facing itself, in
// view space and without a size limit (model_link).
std::vector<std::pair<uint32_t,uint32_t>> model_flag_saved;
uint64_t model_backface=0,model_flags_lifted=0;
void model_flags_restore() {
    for(auto it=model_flag_saved.rbegin();it!=model_flag_saved.rend();++it) psx_mod_write_word(it->first,it->second);
    model_flag_saved.clear();
}
constexpr unsigned model_vertex_limit=1024;
void object_room_restore();
// D17AT11: the object 0x800a2690 is drawing and the camera MATRIX its model
// is placed with (0x8002df20 reads it through the context's +0xc).
uint32_t model_object=0,model_camera=0;
uint64_t model_refined=0,model_refine_skipped=0;double model_refine_max=0;double model_refine_last[6];
void model_begin(CPUState* cpu) {
    model_draw.live=false;
    object_room_restore();model_flags_restore();model_draw.faces.clear();
    if(cpu->gpr[31]!=0x8002e06cu || !model_live()) return;
    const uint32_t model=cpu->gpr[4],records=cpu->gpr[5];
    auto valid=[](uint32_t p,uint32_t n){return (p>=0x80010000u && p<=0x80200000u-n && !(p&3)) || (p>=0x1f800000u && p<=0x1f800400u-n && !(p&3));};
    if(!valid(model,0x1c)) return;
    const unsigned count=psx_mod_read_half(model+2);
    const uint32_t vertices=psx_mod_read_word(model+0xc);
    if(!count || count>model_vertex_limit || !valid(vertices,8*count) || !valid(records,8*count)) return;
    ++model_seen;
    model_draw.g=read_gte(cpu);model_draw.records=records;
    if(model_draw.g.h<64) return;
    model_draw.verts.resize(count);model_draw.near.assign(count,false);
    for(unsigned i=0;i<count;++i) {
        const uint32_t v=vertices+8*i,r=records+8*i;
        const NearProjected p=near_project(model_draw.g,(int16_t)psx_mod_read_half(v),(int16_t)psx_mod_read_half(v+2),(int16_t)psx_mod_read_half(v+4));
        // Fail closed if the transform or record layout is not what the
        // game's projector (0x8002dce0) produced.
        if(p.sxy!=psx_mod_read_word(r) || (uint16_t)(p.sz>>2)!=psx_mod_read_half(r+4)) {++model_mismatch;return;}
        model_draw.verts[i]=p;
    }
    // D17AT11: 0x8002df20 places the model with TR = the camera matrix times
    // (object position - camera position) through ApplyMatrixLV
    // (0x800b444c), truncated to whole units. That error changes as the
    // camera moves, so every blob's plane jittered by up to about half a unit
    // from frame to frame: enough to change which of two nearly coplanar
    // surfaces (a blob and its wall, or two blobs) is nearer near where they
    // meet. The view positions are moved by the exact translation; screen
    // positions stay the game's. Only for the object 0x800a2690 announced,
    // and only when the difference is the expected truncation (under a unit;
    // anything under 2 is accepted).
    // DNTTK_MODEL_REFINE=0 (developer) keeps the game's translation.
    static const bool refine=env_int("DNTTK_MODEL_REFINE",1,0,1)!=0;
    if(refine && model_object && model_camera) {
        int32_t m[3][3],d[3];
        for(int r=0;r<3;++r) for(int k=0;k<3;++k) m[r][k]=(int16_t)psx_mod_read_half(model_camera+2*(3*r+k));
        for(int k=0;k<3;++k) d[k]=(int32_t)psx_mod_read_word(model_object+0x20+4*k)-(int32_t)psx_mod_read_word(model_camera+0x14+4*k);
        double delta[3];bool small=true;
        for(int r=0;r<3;++r) {
            const double exact=((double)m[r][0]*d[0]+(double)m[r][1]*d[1]+(double)m[r][2]*d[2])/4096.0;
            delta[r]=exact-model_draw.g.tr[r];small&=std::abs(delta[r])<2.0;
        }
        for(int k=0;k<3;++k){model_refine_last[k]=delta[k];model_refine_last[3+k]=model_draw.g.tr[k];}
        // D17AT12: a fixed rank per object (Fibonacci hash of its address):
        // the same in every frame and every redraw.
        model_draw.rank=(uint32_t)((model_object>>2)*2654435761u)>>28;
        if(small) {
            for(auto& v:model_draw.verts) for(int k=0;k<3;++k) v.view[k]+=delta[k];
            ++model_refined;
            for(int k=0;k<3;++k) model_refine_max=std::max(model_refine_max,std::abs(delta[k]));
        } else ++model_refine_skipped;
    }
    if(!(refine && model_object && model_camera)) model_draw.rank=0;
    model_draw.object=model_object;
    model_object=model_camera=0;
    // Build every near face: 0x8002d7a8 drops a face with a corner flagged
    // 0x10 (or 0x20 far); far rejection stays. A near corner's screen flags
    // come from saturated projections and go too.
    // D17AT7: so do the left/right flags (1/2) of every corner. 0x8002d7f8
    // drops a face wholly beyond one side of the game's cull rectangle,
    // whose horizontal half-width (about 341, measured from the flags) is
    // the original 4:3 view; the 16:9 view reaches about 480. Blood lying
    // in the outer strip at either side vanished when Duke turned it there
    // (the user's slot-7 clip, bloody.mp4). The view is wider, not taller,
    // so faces above or below the screen are still culled by the game; the
    // host clips the rest against its own guard band.
    for(unsigned i=0;i<count;++i) {
        const uint32_t flags=records+8*i+6;
        const uint16_t f=psx_mod_read_half(flags);
        if(f&0x10) {model_draw.near[i]=true;++model_near_cleared;}
        // DNTTK_MODEL_SIDE_CULL=1 (developer) keeps the game's side cull.
        static const bool side_cull=env_int("DNTTK_MODEL_SIDE_CULL",0,0,1)!=0;
        static const int keep_env=env_int("DNTTK_MODEL_KEEP_FLAGS",-1,-1,0x3f);
        const uint16_t keep=keep_env>=0?(uint16_t)keep_env:f&0x10?0x20:side_cull?0x2f:0x2c;
        if(f&~keep&0x3f) {psx_mod_write_half(flags,(uint16_t)(f&keep));model_sides_cleared+=(f&3) && !(f&0x10);}
    }
    // The face walk of 0x8002d848: the flag word advances unless 0x20, the
    // index list unless 0x40; the top two flag bits give 1-4 corners.
    {
        const unsigned faces=psx_mod_read_half(model);
        const uint32_t flag_list=psx_mod_read_word(model+4),index_list=psx_mod_read_word(model+8);
        if(faces && faces<=4096 && valid(flag_list,4) && valid(index_list,4)) {
            uint32_t fi=0,ip=index_list;
            for(unsigned i=0;i<faces;++i) {
                const uint32_t at=flag_list+4*fi,fl=psx_mod_read_word(at);
                if(valid(ip,4)) {
                    ModelDraw::Face face{};face.corners=(uint8_t)((fl>>30)+1);face.facing=fl&0x200;
                    for(int k=0;k<4;++k) face.idx[k]=psx_mod_read_byte(ip+k);
                    bool bad=false;
                    for(unsigned k=0;k<face.corners;++k) bad|=face.idx[k]>=count;
                    if(!bad) model_draw.faces.push_back(face);
                }
                if((fl&0x200) && valid(ip,4)) {
                    bool saved=false;
                    for(const auto& e:model_flag_saved) saved|=e.first==at;
                    if(!saved) {model_flag_saved.push_back({at,fl});psx_mod_write_word(at,fl&~0x200u);++model_flags_lifted;}
                }
                if(!(fl&0x40)) ip+=4;
                if(!(fl&0x20)) ++fi;
            }
        }
    }
    // A flat model (all vertices within half a unit of one plane) lies on a
    // surface as a decal.
    model_draw.flat=false;
    if(count>=3) {
        const NearDepthPlane plane=near_depth_plane(model_draw.verts[0].view,model_draw.verts[1].view,model_draw.verts[2].view);
        const double length=std::sqrt(plane.n[0]*plane.n[0]+plane.n[1]*plane.n[1]+plane.n[2]*plane.n[2]);
        model_draw.flat=length>1e-9;
        for(unsigned i=0;i<count && model_draw.flat;++i) {
            const double* p=model_draw.verts[i].view;
            model_draw.flat=std::abs(plane.n[0]*p[0]+plane.n[1]*p[1]+plane.n[2]*p[2]-plane.d)<=0.5*length;
        }
    }
    // D17AT9: a decal's ordering slot from the model's straight-line distance
    // (its vertices' mean), in 256-unit buckets at the bucket's far side.
    {
        double c[3]={0,0,0};
        for(const auto& v:model_draw.verts) for(int k=0;k<3;++k) c[k]+=v.view[k]/count;
        const double distance=std::sqrt(c[0]*c[0]+c[1]*c[1]+c[2]*c[2]);
        model_draw.slot=std::min(0x7ff,((int)distance>>5)|7);
    }
    model_draw.live=true;++model_taken;model_flat+=model_draw.flat;
}
void object_room_restore();
void model_end() {model_draw.live=false;model_flags_restore();object_room_restore();}
// D17AT6: 0x800a2690 draws a small-model object (blood pools) only if its
// room (object +0x38) is set in the portal walk's room bitset 0x800e1f10.
// Polymorph (D17AR) shows rooms the game's portal walk does not list (seen
// over walls, or through an opening at or past the edge of the view) but
// leaves the portal walk, and so this bitset, to the game. Blood in such a
// room vanished while the floor under it stayed: at the sides of the view
// when turning. For a room the host shows, the bit is set for this object's
// test only and cleared again when its faces start (model_begin), at the next
// object, or at the next scene, so nothing else in the frame (gameplay
// included) sees a changed bitset. Classic and Vanilla never reach this.
uint32_t object_room_word=0,object_room_bit=0;
uint64_t object_room_lifts=0;
void object_room_restore() {
    if(!object_room_word) return;
    psx_mod_write_word(object_room_word,psx_mod_read_word(object_room_word)&~object_room_bit);
    object_room_word=0;
}
void object_room_lift(CPUState* cpu) {
    object_room_restore();
    model_object=model_camera=0;
    if(!model_live()) return;
    const uint32_t object=cpu->gpr[5];
    const uint32_t context_struct=cpu->gpr[4];
    if(object>=0x80010000u && object<=0x801fffc0u && context_struct>=0x80010000u && context_struct<=0x801fffe0u) {
        const uint32_t camera=psx_mod_read_word(context_struct+0xc);
        if(camera>=0x80010000u && camera<=0x801fffe0u) {model_object=object;model_camera=camera;}
    }
    if(object<0x80010000u || object>0x801fffc0u) return;
    const uint8_t byte=psx_mod_read_byte(object+0x38);
    const int word_index=(int8_t)byte>>5;
    if(word_index<0 || !host_visibility_room_shown(byte)) return;
    const uint32_t word=0x800e1f10u+4*(uint32_t)word_index,bit=1u<<(byte&0x1f);
    const uint32_t value=psx_mod_read_word(word);
    if(value&bit) return;
    psx_mod_write_word(word,value|bit);object_room_word=word;object_room_bit=bit;++object_room_lifts;
}
// 0x8002bd48 (a0 = SZ key, a1 = packet) from one of the face emitters.
void model_link(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    if(!model_draw.live || ra<0x8002bf8cu || ra>=0x8002d508u) return;
    // Still this model: other users of the emitters (the 0x8009b1c0 sprite
    // queue) set their own transform first.
    {
        const NearGte g=read_gte(cpu);
        if(std::memcmp(g.r,model_draw.g.r,sizeof g.r) || std::memcmp(g.tr,model_draw.g.tr,sizeof g.tr)) {model_draw.live=false;return;}
    }
    const uint32_t packet=cpu->gpr[5];
    if(packet<0x80010000u || packet>0x80200000u-0x40 || (packet&3)) return;
    ++model_faces;
    const uint32_t command=psx_mod_read_word(packet+4);
    const unsigned size=psx_mod_read_word(packet)>>24;
    const bool polygon=(command>>29)==1;
    const bool gouraud=command&0x10000000u,quad=command&0x08000000u,textured=command&0x04000000u;
    const unsigned corners=quad?4:3;
    // Packet words after the tag: flat, the command then per corner xy
    // [uv]; gouraud, per corner colour (the command for the first) xy [uv].
    const unsigned per=1+(textured?1:0)+(gouraud?1:0);
    const bool shaped=polygon && size==(gouraud?0:1)+corners*per;
    bool ok=shaped;
    int idx[4]={-1,-1,-1,-1};bool near=false;
    uint32_t xys[4]={0xffffffffu,0xffffffffu,0xffffffffu,0xffffffffu};
    uint32_t colors[4]{},uv[4]{},clut=0,tpage=0;
    for(unsigned i=0;shaped && i<corners;++i) {
        const uint32_t at=packet+4+4*(gouraud?i*per+1:1+i*per);
        colors[i]=(gouraud?psx_mod_read_word(at-4):command)&0xffffffu;
        const uint32_t xy=psx_mod_read_word(at);
        if(textured) {
            const uint32_t w=psx_mod_read_word(at+4);uv[i]=w&0xffff;
            if(i==0) clut=w>>16; else if(i==1) tpage=w>>16;
        }
        xys[i]=xy;
        // The corner's vertex: the one whose record has this position.
        // Duplicate positions are only ambiguous if their depths differ.
        int found=-1;
        for(unsigned k=0;k<model_draw.verts.size() && ok;++k) {
            if(psx_mod_read_word(model_draw.records+8*k)!=xy) continue;
            if(found>=0 && std::abs(model_draw.verts[k].view[2]-model_draw.verts[found].view[2])>0.5) ok=false;
            if(found<0) found=(int)k;
        }
        if(found<0) ok=false;
        if(ok) {idx[i]=found;near|=model_draw.near[found];}
    }
    // D17AT8: better, the face itself: the walked face whose corners' records
    // hold exactly these positions (in order). Near corners are clamped, so
    // a corner alone can match several vertices; a whole face rarely does.
    const ModelDraw::Face* face=nullptr;
    if(shaped) {
        bool all=true;
        for(unsigned i=0;i<corners && all;++i) all=xys[i]!=0xffffffffu;
        unsigned hits=0;
        for(const auto& c:model_draw.faces) {
            if(!all || c.corners!=corners) continue;
            bool same=true;
            for(unsigned i=0;i<corners && same;++i) same=psx_mod_read_word(model_draw.records+8*c.idx[i])==xys[i];
            if(!same) continue;
            if(face && std::memcmp(face->idx,c.idx,corners)) {++hits;continue;}
            face=&c;++hits;
        }
        if(face && hits==1) {
            ok=true;near=false;
            for(unsigned i=0;i<corners;++i) {idx[i]=face->idx[i];near|=model_draw.near[idx[i]];}
        } else face=nullptr;
    }
    // A face with a near corner anywhere (matched or not) has saturated
    // native corners: never leave it to the game.
    for(unsigned i=0;polygon && i<corners && !near && i*per<size;++i) {
        const uint32_t xy=psx_mod_read_word(packet+4+4*(gouraud?i*per+1:1+i*per));
        for(unsigned k=0;k<model_draw.verts.size() && !near;++k)
            near=model_draw.near[k] && psx_mod_read_word(model_draw.records+8*k)==xy;
    }
    Frame f{};
    const bool begun=begin(f,cpu,true);
    if(begun) {f.g=model_draw.g;f.scene=true;f.verts=model_draw.verts;trace_begin(f,cpu);f.instance=model_draw.object;}
    auto leave=[&] {
        if(near) {psx_mod_write_word(packet,psx_mod_read_word(packet)&0xffffffu);++model_faces_dropped;trace_record(f,"model_dropped",packet);}
        else {++model_faces_native;trace_record(f,"model_native",packet);}
    };
    if(!ok) {leave();return;}
    // D17AT8: the facing test the game would have made (flag 0x200), in view
    // space: front-facing when det(v0,v1,v2) > 0, as NCLIP >= 2 for corners
    // in front of the eye, and also right for corners beside or behind it.
    if(face && face->facing &&
       det(model_draw.verts[idx[0]].view,model_draw.verts[idx[1]].view,model_draw.verts[idx[2]].view)<=0) {
        psx_mod_write_word(packet,psx_mod_read_word(packet)&0xffffffu);++model_backface;
        if(f.trace) {std::ostringstream x;x<<",\"idx\":["<<idx[0]<<','<<idx[1]<<','<<idx[2]<<"],\"v\":[";
            for(int k=0;k<3;++k){const double* v=model_draw.verts[idx[k]].view;x<<(k?",":"")<<'['<<v[0]<<','<<v[1]<<','<<v[2]<<']';}
            x<<"]";trace_record(f,"model_backface",packet,x.str());}
        return;
    }
    if(!begun) {leave();return;}
    if(budget(f)<takeover_reserve) {++model_faces_budget;leave();return;}
    Poly p{};p.source=packet;p.count=corners;p.textured=textured;p.actor=false;
    p.native_order=true;p.native_slot=clamp((int32_t)cpu->gpr[4]>>3,0,0x7ff);
    p.command=0x30000000u|(textured?0x04000000u:0)|(command&0x03000000u);
    p.clut=clut;p.tpage=tpage;
    for(unsigned i=0;i<corners;++i) {p.idx[i]=idx[i];p.colors[i]=colors[i];p.uv[i]=uv[i];}
    trace_vertices(f);
    // D17AT5/AT12: a flat model's opaque faces are decals. Blobs of one
    // pool lie on the same surface, so plain depth (D17AT5 before) let
    // whichever was nearer by rounding win each pixel, and painter order
    // (D17AT5/AT9) followed sort slots that change as the eye moves: either
    // way overlapping blobs swapped while the view panned. With the exact
    // placement (D17AT11) a blob's plane is steady to about 0.05 units, so
    // depth can decide: each decal tests and writes depth at its plane lifted
    // decal_base units plus its fixed rank (0..15) times decal_step toward the
    // eye. Between overlapping blobs the higher rank wins in every frame and
    // from every angle; a surface drawn after a decal (decal_base units or
    // more behind, enough for a blob up to about 4 units inside it) never
    // covers it; anything nearer than the lift hides it. The lift is a depth
    // scale of the face's own plane (Poly::depth_scale), applied after the
    // raster fallback. Decals keep the game's ordering slots. Translucent
    // faces stay painter-only, as for any prop.
    // Developer: DNTTK_DECAL_RANK=0 restores the D17AT10 rule (a depth-only
    // pass decal_write up, a test-only colour pass decal_lift up, distance
    // slots); DNTTK_DECAL_LIFT=0 the plain prop rule.
    static const int lift=env_int("DNTTK_DECAL_LIFT",6,0,64),write=env_int("DNTTK_DECAL_WRITE",4,0,64);
    static const bool ranked=env_int("DNTTK_DECAL_RANK",0,0,1)!=0;
    static const double base=env_int("DNTTK_DECAL_BASE_X8",32,0,256)/8.0,step=env_int("DNTTK_DECAL_STEP_X64",8,0,64)/64.0;
    static const int decal_slot=env_int("DNTTK_DECAL_SLOT",2,0,2);
    const NearDepthPlane plane=corners>=3 ? near_depth_plane(f.verts[idx[0]].view,f.verts[idx[1]].view,f.verts[idx[2]].view) : NearDepthPlane{};
    const double distance=std::abs(plane.d);
    if(model_draw.flat && lift>write && !(command&0x02000000u) && distance>2.0*std::max<double>(lift,base+15*step)) {
        p.actor=true; // no prop contact shift: the decal offsets replace it
        if(ranked) {
            p.depth_scale=1-(base+step*model_draw.rank)/distance;
            draw(f,p);
        } else {
            if(decal_slot==1) p.native_slot=model_draw.slot;
            else if(decal_slot==2) p.native_slot=0x7ff;
            Poly depth=p;depth.depth_only=true;depth.depth_scale=1-write/distance;
            p.depth_test_only=true;p.depth_scale=1-lift/distance;
            draw(f,depth);draw(f,p);
        }
        ++model_faces_decal;
    } else draw(f,p);
    ++model_faces_taken;
    // Same as the water tiles: the native packet stays linked with no words.
    psx_mod_write_word(packet,psx_mod_read_word(packet)&0xffffffu);
}
