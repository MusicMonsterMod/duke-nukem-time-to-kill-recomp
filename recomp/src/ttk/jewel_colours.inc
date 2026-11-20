// D08A45: the three Family Jewels of level 6 in the energy crystals' blue,
// green and red (Modernized with Polymorph; DNTTK_JEWEL_COLOURS=original keeps
// them pink). Included in modern_controls.cpp after keycard_colours.inc. Note
// 501.
//
// Types 854, 855 and 856 (inventory items 14, 15, 16) each have their own
// model (1595-1597 in level 6), all drawn with the same 13 textured faces and
// four 4-bit palettes, every one a single-hue pink ramp. A ramp of the same
// brightness at another hue recolours a palette entry for entry, as D08A46 did
// for the key cards: 854 blue, 855 green, 856 red. The recoloured ramps (ours:
// hue set to the crystal icon's, saturation lifted so the pale facets do not
// read pink again) sit in the second texture bank at y 481; a jewel face keeps
// its retail texels and reads only its palette from there (near_clip,
// PSX_MOD_HOST_CLUT_BANK). The test is the mesh being drawn, not the object
// type, so any object showing a jewel model takes its colour.
//
// Placed jewels: the level's holder (the nuke-symbol wheel) does not draw the
// jewel models; it switches to its own models with 1, 2, 3 jewels set in
// (1632-1634 in level 6), whose jewel faces use the jewel's palettes. Any
// other mesh with 4-bit faces on those palettes is such a holder: its jewel
// faces are grouped into sockets by shared vertices, and the sockets are put
// in fill order by comparing with the model before it (the holder with one
// jewel fewer). The k-th socket takes the colour of the k-th jewel set in
// (D60 records the order, since Modernized takes them in any order), so the
// wheel always shows the jewels that went in. Pickups, mission flags and the level's
// logic are untouched; Classic and Vanilla keep pink.
namespace {
constexpr int jewel_clut_x=256,jewel_clut_y=481;   // 12 palettes, x 256-447
constexpr unsigned jewel_type_first=854,jewel_palettes=4;
// [colour][palette in first-use order within the jewel model][entry].
constexpr uint16_t jewel_ramps[3][jewel_palettes][16]={
    {   // blue
        {0xf1c4,0xf5c4,0xf9e5,0xf9e5,0xfdc3,0xfdc3,0xfdc3,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
        {0xe9a4,0xed60,0xf160,0xf580,0xf580,0xf980,0xf980,0xf980,0xfd80,0xfd80,0xfd80,0xfd80,0x8000,0x8000,0x8000,0x8000},
        {0xd920,0xe140,0xe540,0xe940,0xed60,0xf160,0xf160,0xf580,0xf980,0xf980,0xfd80,0xfd80,0x8000,0x8000,0x8000,0x8000},
        {0xf68e,0xfeae,0xfe6c,0xfe6c,0xfe6c,0xfe8d,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
    },
    {   // green
        {0x9b24,0x9b43,0x9f44,0x9f44,0x9763,0x9763,0x9763,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
        {0x9ae3,0x8f00,0x8f20,0x8f40,0x8f40,0x8f40,0x8f40,0x8f40,0x8f60,0x8f60,0x8f60,0x8f60,0x8000,0x8000,0x8000,0x8000},
        {0x8a60,0x8aa0,0x8ec0,0x8ee0,0x8f00,0x8f20,0x8f20,0x8f40,0x8f40,0x8f40,0x8f60,0x8f60,0x8000,0x8000,0x8000,0x8000},
        {0xbb4c,0xbb6d,0xb36a,0xb36a,0xb36a,0xb76b,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
    },
    {   // red
        {0x90bc,0x90bd,0x94be,0x94be,0x8c9f,0x8c9f,0x8c9f,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
        {0x90ba,0x803b,0x803c,0x803d,0x803d,0x803e,0x803e,0x803e,0x803f,0x803f,0x803f,0x803f,0x8000,0x8000,0x8000,0x8000},
        {0x8036,0x8038,0x8039,0x803a,0x803b,0x803c,0x803c,0x803d,0x803e,0x803e,0x803f,0x803f,0x8000,0x8000,0x8000,0x8000},
        {0xb9dd,0xb9ff,0xb19f,0xb19f,0xb19f,0xb5bf,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000},
    },
};
uint64_t jewel_frames[3]={0,0,0},jewel_holder_frames=0;
// The mesh of model m, or 0.
uint32_t jewel_model_mesh(unsigned m) {
    const uint32_t models=psx_mod_read_word(0x800ddb18);
    if(m>=4096 || models<0x80010000u || models>=0x801f0000u) return 0;
    const uint32_t mesh=psx_mod_read_word(models+4*m);
    return mesh>=0x80010000u && mesh<=0x801ffc00u && !(mesh&3)?mesh:0;
}
unsigned jewel_type_model(unsigned t) {
    const uint32_t types=psx_mod_read_word(0x800d2660);
    if(types<0x80010000u || types>=0x801f0000u) return 4096;
    return psx_mod_read_half(types+28*t+16);
}
// The mesh of type t's model, or 0 (the original's own tables, as SetObjectType).
uint32_t jewel_type_mesh(unsigned t) {return jewel_model_mesh(jewel_type_model(t));}
unsigned jewel_record_size(uint32_t type) {
    return type==0x24 || type==0x2c?16:type==0x34 || type==0x3c?20:type==0x20 || type==0x28?8:type==0x30 || type==0x38?12:0;
}
// Walks a mesh's textured faces: fn(clut, page, vertex indices, corners).
// False on a list that is not a plain object list.
template<class Fn> bool jewel_faces(uint32_t mesh,Fn fn) {
    const uint32_t list=psx_mod_read_word(mesh+0x14);
    if(list<0x80010000u || list>0x801ffc00u || (list&3)) return false;
    uint32_t cursor=list;
    for(unsigned guard=0;;++guard) {
        if(guard>=32) return false;
        const uint32_t header=psx_mod_read_word(cursor);cursor+=4;
        const uint32_t type=header&0xffff,n=header>>16;
        if(type==0xff) return true;
        const unsigned size=jewel_record_size(type);
        if(!size || n>512) return false;
        for(uint32_t k=0;k<n;++k,cursor+=size) {
            if(!(type&4)) continue;   // untextured
            const uint32_t w0=psx_mod_read_word(cursor);
            const uint8_t vi[4]={(uint8_t)w0,(uint8_t)(w0>>8),(uint8_t)(w0>>16),(uint8_t)(w0>>24)};
            fn((uint16_t)(psx_mod_read_word(cursor+4)>>16),(uint16_t)(psx_mod_read_word(cursor+8)>>16),vi,(type&8)?4u:3u);
        }
    }
}
// A jewel model's four palettes in first-use order.
bool jewel_palettes_of(uint32_t mesh,uint16_t clut[4]) {
    unsigned found=0;bool ok=true;
    if(!jewel_faces(mesh,[&](uint16_t c,uint16_t page,const uint8_t*,unsigned) {
        if((page>>7)&3) {ok=false;return;}   // the ramps are 16 entries: 4-bit only
        for(unsigned j=0;j<found;++j) if(clut[j]==c) return;
        if(found==jewel_palettes) {ok=false;return;}   // not the model measured
        clut[found++]=c;
    })) return false;
    return ok && found==jewel_palettes;
}
// A holder's sockets: each vertex's socket (connected jewel faces), -1 for none.
struct JewelSockets {int8_t of_vertex[256];int count=0;};
bool jewel_holder_sockets(uint32_t mesh,const uint16_t clut[4],JewelSockets& s) {
    uint8_t parent[256];for(int i=0;i<256;++i) parent[i]=(uint8_t)i;
    bool used[256]={};
    auto root=[&](int v) {while(parent[v]!=v) v=parent[v]=parent[parent[v]];return v;};
    bool any=false;
    if(!jewel_faces(mesh,[&](uint16_t c,uint16_t page,const uint8_t* vi,unsigned corners) {
        if((page>>7)&3) return;
        bool jewel=false;for(unsigned k=0;k<jewel_palettes;++k) jewel|=c==clut[k];
        if(!jewel) return;
        any=true;
        for(unsigned j=0;j<corners;++j) {used[vi[j]]=true;parent[root(vi[j])]=(uint8_t)root(vi[0]);}
    }) || !any) return false;
    int id_of_root[256];for(int i=0;i<256;++i) id_of_root[i]=-1;
    s.count=0;
    for(int v=0;v<256;++v) {
        s.of_vertex[v]=-1;
        if(!used[v]) continue;
        const int r=root(v);
        if(id_of_root[r]<0) {if(s.count==3) return false;id_of_root[r]=s.count++;}
        s.of_vertex[v]=(int8_t)id_of_root[r];
    }
    return true;
}
// Socket ids in fill order: the holder model before this one (one jewel
// fewer) has the earlier sockets on the same vertices; depth at most 3.
void jewel_fill_order(unsigned model,const uint16_t clut[4],const JewelSockets& s,int order[3],int depth=0) {
    for(int k=0;k<s.count;++k) order[k]=k;   // fallback: lowest vertex first
    if(s.count<2 || depth>=3 || !model) return;
    const uint32_t prev_mesh=jewel_model_mesh(model-1);
    JewelSockets p;
    if(!prev_mesh || !jewel_holder_sockets(prev_mesh,clut,p) || p.count!=s.count-1) return;
    int prev_order[3];jewel_fill_order(model-1,clut,p,prev_order,depth+1);
    // Map each earlier socket to ours through a shared vertex.
    int mapped[3]={-1,-1,-1};bool taken[3]={};
    for(int k=0;k<p.count;++k)
        for(int v=0;v<256;++v) if(p.of_vertex[v]==k) {
            if(s.of_vertex[v]<0) return;
            mapped[k]=s.of_vertex[v];break;
        }
    int n=0;
    for(int k=0;k<p.count;++k) {const int m=mapped[prev_order[k]];if(m<0 || taken[m]) return;taken[m]=true;order[n++]=m;}
    for(int k=0;k<s.count;++k) if(!taken[k]) order[n++]=k;
}
// Per mesh, per thread (redraw workers draw too): jewel model, holder or
// neither, kept until the level, a savestate load or the mesh's list changes.
struct JewelMeshInfo {uint32_t list=0,level=0,loads=0;int colour=-1;bool holder=false;JewelSockets sockets;int order[3]={0,1,2};};
thread_local std::unordered_map<uint32_t,JewelMeshInfo> jewel_mesh_info;
// Model id of a mesh (the holder's predecessor is the model before it).
unsigned jewel_model_of(uint32_t mesh) {
    const uint32_t models=psx_mod_read_word(0x800ddb18);
    if(models<0x80010000u || models>=0x801f0000u) return 0;
    for(unsigned m=1;m<4096;++m) if(psx_mod_read_word(models+4*m)==mesh) return m;
    return 0;
}
}

bool jewel_colours_on() {
    if(!input_modernized() || ttk_world_classic()) return false;
    const char* v=ttk_live_env("DNTTK_JEWEL_COLOURS");
    return !(v && !std::strcmp(v,"original"));
}
void jewel_paint_bank(uint16_t* bank) {
    for(int c=0;c<3;++c) for(unsigned k=0;k<jewel_palettes;++k) for(int i=0;i<16;++i)
        bank[(size_t)jewel_clut_y*1024+jewel_clut_x+16*(c*jewel_palettes+k)+i]=jewel_ramps[c][k][i];
}
// For the mesh being drawn: a jewel model (one colour) or a holder (a colour
// per vertex, by socket), with the jewel's four palettes in first-use order
// and their bank replacements for each colour.
bool jewel_frame(uint32_t mesh,bool duke_frame,JewelRecolour& out) {
    out={};
    if(mesh<0x80010000u || mesh>0x801ffc00u || (mesh&3)) return false;
    const uint32_t first=jewel_type_mesh(jewel_type_first);
    if(!first) return false;   // no jewels in this level
    const uint32_t list=psx_mod_read_word(mesh+0x14),level=psx_mod_read_word(0x800be570),loads=psx_mod_savestate_loads();
    auto& info=jewel_mesh_info[mesh];
    if(info.list!=list || info.level!=level || info.loads!=loads) {
        info=JewelMeshInfo{};info.list=list;info.level=level;info.loads=loads;
        for(int c=0;c<3;++c) if(jewel_type_mesh(jewel_type_first+c)==mesh) info.colour=c;
        uint16_t clut[4];
        if(info.colour<0 && jewel_palettes_of(first,clut) && jewel_holder_sockets(mesh,clut,info.sockets)) {
            info.holder=true;
            jewel_fill_order(jewel_model_of(mesh),clut,info.sockets,info.order);
        }
    }
    if((info.colour<0 && !info.holder) || !jewel_colours_on() || !jewel_palettes_of(first,out.clut)) return false;
    for(int c=0;c<3;++c) for(unsigned k=0;k<jewel_palettes;++k)
        out.bank_clut[c][k]=(uint16_t)((jewel_clut_y<<6)|((jewel_clut_x+16*(c*jewel_palettes+k))>>4));
    if(!info.holder) {
        // In Duke's hand (the hand-in at the wheel) the game draws the first
        // jewel's model for every jewel; the held item says which one it is:
        // equipment +0x3b8 1 (upper 47/48) or the insertion pose 266, item
        // +0x3b9 14..16 (measured at the wheel, as the key cards).
        int colour=info.colour;
        const uint32_t o=nv_object;
        if(duke_frame || (o && o==player)) {
            const unsigned item=psx_mod_read_byte(player+0x3b9);
            if((psx_mod_read_byte(player+0x3b8)==1 || psx_mod_read_half(player+0x74)==266) && item>=14 && item<=16) {
                // D60: a jewel lent to the lock's item is the one in his hand.
                const int lent=jewel_lent_from(item);
                colour=(lent>=0?lent:(int)item)-14;
            }
        }
        out.colour=(int8_t)colour;++jewel_frames[colour];return true;
    }
    // Holder: the k-th socket in fill order shows the k-th used jewel, in the
    // order they went in (D60, jewel_device.inc; item order for a used jewel
    // with no recorded place); sockets past the used ones take the colours
    // not yet shown.
    int colours[3],n=0;bool shown[3]={};
    auto used=[](int c){const uint16_t v=psx_mod_read_half(0x800d7198u+0x354+4*(14+c));return !(v&1) && (v&0x4000);};
    for(unsigned k=1;k<=3;++k) for(int c=0;c<3;++c) if(!shown[c] && used(c) && jewel_order(14+c)==k) {colours[n++]=c;shown[c]=true;}
    for(int c=0;c<3;++c) if(!shown[c] && used(c)) {colours[n++]=c;shown[c]=true;}
    for(int c=0;c<3 && n<3;++c) if(!shown[c]) colours[n++]=c;
    int8_t socket_colour[3];
    for(int k=0;k<info.sockets.count;++k) socket_colour[info.order[k]]=(int8_t)colours[k];
    for(int v=0;v<256;++v) {const int s=info.sockets.of_vertex[v];out.vertex_colour[v]=s<0?-1:socket_colour[s];}
    out.holder=true;++jewel_holder_frames;
    return true;
}
void jewel_colour_stats(uint64_t out[3]) {for(int c=0;c<3;++c) out[c]=jewel_frames[c];}
