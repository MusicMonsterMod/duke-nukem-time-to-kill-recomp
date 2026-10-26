#include "mod_memory.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern uint32_t psx_mod_memory_alloc(uint32_t,uint32_t);
extern size_t render_replay_image_bytes(void);
extern void render_replay_image_save(uint8_t*);
extern void render_replay_image_load(const uint8_t*);
void overlay_loader_note_code_write(void) {}
void overlay_loader_resync_validation_after_restore(void) {}
static void put(uint8_t*p,uint32_t v){for(unsigned i=0;i<4;++i)p[i]=v>>(8*i);}
static void biomask_load_first(void) {
    const uint32_t world=512u*1024u+16u,mask=188u*1024u,anim=320u*1024u;
    const uint32_t used=world+anim+mask,header=44;
    // Current startup owns world+mask before loading an older world+anim+mask.
    assert(psx_mod_gpu_dma_memory_alloc(world,16)==0x80f00000u);
    const uint32_t mask_addr=psx_mod_gpu_dma_memory_alloc(mask,16);
    assert(mask_addr==0x80f00000u+world);
    uint8_t *wire=calloc(1,header+used);assert(wire);
    put(wire,1);put(wire+12,used);put(wire+16,3);
    put(wire+20,0);put(wire+24,world);
    put(wire+28,world);put(wire+32,anim);
    put(wire+36,world+anim);put(wire+40,mask);
    memset(wire+header+world,0x5a,anim);
    assert(psx_mod_memory_snapshot_read(wire,header+used));
    assert(!strcmp(psx_mod_memory_snapshot_outcome(),"partial"));
    // D37A can still read the exact saved animation allocation for rebuilding.
    uint8_t source[32];
    assert(psx_mod_memory_saved_gpu_allocation(0x80f00000u+world,anim,source,sizeof source));
    assert(source[0]==0x5a);
    assert(psx_mod_gpu_dma_memory_alloc(anim,16)==mask_addr+mask);
    // Repeated old loads and warm current-layout loads keep the same ownership.
    assert(psx_mod_memory_snapshot_read(wire,header+used));
    unsigned n=psx_mod_memory_snapshot_bytes();uint8_t *live=malloc(n);assert(live);
    psx_mod_memory_snapshot_write(live);
    assert(psx_mod_memory_snapshot_read(live,n));
    assert(!strcmp(psx_mod_memory_snapshot_outcome(),"restored"));
    free(live);free(wire);
    puts("Bio Mask startup reservation survives old/warm DMA layouts with animation capacity PASS");
}
int main(int argc,char**argv) {
    if(argc==2 && !strcmp(argv[1],"biomask")){biomask_load_first();return 0;}
    // Actual memory.c snapshot functions: live layout has a 16-byte first arena,
    // save has a 32-byte arena followed by 32 bytes of non-packet data.
    assert(psx_mod_gpu_dma_memory_alloc(16,16)==0x80f00000);
    assert(psx_mod_gpu_dma_memory_alloc(64,16)==0x80f00010);
    uint8_t wire[100]={0};put(wire,1); // empty EXP1 header at 4..11
    put(wire+12,64);put(wire+16,2);
    put(wire+20,0);put(wire+24,32);put(wire+28,32);put(wire+32,32);
    memset(wire+36,0x5a,64);
    assert(psx_mod_memory_snapshot_read(wire,sizeof wire));
    assert(!strcmp(psx_mod_memory_snapshot_outcome(),"partial"));
    uint8_t got[32]={0};
    assert(psx_mod_memory_saved_gpu_allocation(0x80f00020,32,got,32));
    for(unsigned i=0;i<32;++i)assert(got[i]==0x5a);
    assert(!psx_mod_memory_saved_gpu_allocation(0x80f00024,32,got,16));
    assert(!psx_mod_memory_saved_gpu_allocation(0x80f00020,16,got,16));
    assert(!psx_mod_memory_saved_gpu_allocation(0x90f00020,32,got,16));
    // GPU packets are still cleared in live memory. Inspect serialized live data.
    unsigned n=psx_mod_memory_snapshot_bytes();uint8_t* live=malloc(n);
    psx_mod_memory_snapshot_write(live);
    for(unsigned i=36;i<n;i+=4)assert(live[i]==255 && live[i+1]==255 && live[i+2]==255 && live[i+3]==0);
    assert(psx_mod_memory_snapshot_read(live,n));
    assert(!strcmp(psx_mod_memory_snapshot_outcome(),"restored"));
    assert(psx_mod_memory_saved_gpu_allocation(0x80f00010,64,got,32));
    assert(got[0]==255 && got[3]==0);
    free(live);
    assert(psx_mod_memory_snapshot_read(wire,sizeof wire));
    // Render replay must not overwrite the immutable saved source.
    uint8_t* image=malloc(render_replay_image_bytes());
    render_replay_image_save(image);render_replay_image_load(image);free(image);
    assert(psx_mod_memory_saved_gpu_allocation(0x80f00020,32,got,32) && got[0]==0x5a);
    psx_mod_memory_snapshot_missing();
    assert(!psx_mod_memory_saved_gpu_allocation(0x80f00020,32,got,32));
    assert(psx_mod_memory_snapshot_read(wire,sizeof wire));
    put(wire+24,999); // malformed allocation must invalidate previous saved image
    assert(psx_mod_memory_snapshot_read(wire,sizeof wire));
    assert(!strcmp(psx_mod_memory_snapshot_outcome(),"malformed"));
    assert(!psx_mod_memory_saved_gpu_allocation(0x80f00020,32,got,32));
    put(wire+24,32);assert(psx_mod_memory_snapshot_read(wire,sizeof wire));
    put(wire,2);assert(psx_mod_memory_snapshot_read(wire,sizeof wire));
    assert(!strcmp(psx_mod_memory_snapshot_outcome(),"unknown_version"));
    assert(!psx_mod_memory_saved_gpu_allocation(0x80f00020,32,got,32));
    puts("mod memory snapshot recovery and GPU clearing PASS");
}
