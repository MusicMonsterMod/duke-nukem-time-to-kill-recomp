/* SLUS-00583 only: accelerate the movie player's empty STR-ring polling loop.
 * No decoder, guest instruction, CD cadence, or video frame is replaced.
 * See documentation/13-fmv-fidelity-pass.md for the disassembly evidence. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu_state.h"
#include "psx_cycles.h"
/* Runtime CPU overclock bookkeeping (patches/time-to-kill-*-cpu-overclock). */
extern uint64_t g_psx_overclock_rewound, g_psx_overclock_exempt;
#include "interrupts.h"
#include "dma.h"
#include "mod_plugins.h"
#include "fmv_poll_math.h"
#include "overlay_loader.h"

extern uint64_t g_mmio_access_count;
extern int g_psx_call_bail, g_precise_mode, g_ls_mode;
extern int psx_selfcheck_enabled(void);
extern int psx_netplay_active(void);

static CPUState previous;
static uint64_t last_cycle, last_mmio;
static uint32_t quantum, streak;
static int have_previous, enabled = -1;
static uint64_t calls, skips, iterations, cycles;

/* During movie playback MDEC continuously writes pixels to RAM. Those writes
 * invalidate a generic no-stores detector but do not change this poll. Only
 * allow known sequential DMA channels, with their entire remaining destination
 * disjoint from the caller, callee, ring status and ring pointer/index. */
static int dma_disjoint(uint32_t entry) {
    DMADebugState state;
    dma_debug_get_state(&state);
    for(unsigned i=0;i<7;i++) {
        DMAChannelDebugState *c=&state.channels[i];
        if(!ttk_poll_dma_safe(i,c->active,c->madr,c->chcr,
                              c->remaining_words,entry))return 0;
    }
    return 1;
}

static int same_state(const CPUState *cpu) {
    for (unsigned i=1;i<32;i++)
        if (i!=16 && cpu->gpr[i]!=previous.gpr[i]) return 0;
    return cpu->hi==previous.hi && cpu->lo==previous.lo &&
        memcmp(cpu->cop0,previous.cop0,sizeof(cpu->cop0))==0 &&
        memcmp(cpu->read_absorb,previous.read_absorb,sizeof(cpu->read_absorb))==0 &&
        cpu->read_absorb_which==previous.read_absorb_which &&
        cpu->read_fudge==previous.read_fudge && cpu->ld_which_t==previous.ld_which_t &&
        cpu->ld_absorb==previous.ld_absorb;
}

static int code_matches(void) {
    /* Unique resident caller: poll; decrement s0; loop until ready or timeout. */
    static const uint32_t expected[]={0x0c02e06f,0x27a50014,0x10400005,
        0x2610ffff,0x1600fffb,0x27a40010};
    for(unsigned i=0;i<sizeof(expected)/sizeof(expected[0]);i++)
        if(psx_mod_read_word(0x800ab370u+i*4)!=expected[i])return 0;
    static const uint32_t callee[]={
        0x00803821u,0x3c02800fu,0x8c42811cu,0x3c03800fu,0x8c638130u,0x00021140u,
        0x00623021u,0x94c30000u,0x24020001u,0x1462000du,0x00a04021u,0x3c02800fu,
        0x8c428124u,0x3c01800fu,0x10400002u,0xac20811cu,0xa4c00000u,0x3c02800fu,
        0x8c42811cu,0x3c03800fu,0x8c638130u,0x00021140u,0x00623021u,0x94c30000u,
        0x24020002u,0x14620012u,0x24020001u,0x24020004u,0xa4c20000u,0x00001021u,
        0x3c03800fu,0x8c638134u,0x3c04800fu,0x8c848130u,0x3c05800fu,0x8ca5811cu,
        0x00031940u,0x00832021u,0x00051980u,0x00651823u,0x00031940u,0x00832021u,
        0xace40000u,0xad060000u,0x03e00008u,0x00000000u,
    };
    for(unsigned i=0;i<sizeof(callee)/sizeof(callee[0]);i++)
        if(psx_mod_read_word(0x800b81bcu+i*4)!=callee[i])return 0;
    return 1;
}

static void report(void) {
    fprintf(stderr,"ttk-fmv-poll: calls=%llu skips=%llu iterations=%llu guest_cycles=%llu\n",
        (unsigned long long)calls,(unsigned long long)skips,
        (unsigned long long)iterations,(unsigned long long)cycles);
}

/* The native MOVIE.OVR shard is found by a cache namespace that includes the
 * game.local.toml hook list; a stale one leaves the VLC decoder interpreted
 * (about 52 fps plus audio underruns on the intro). The framework loader
 * reports that only over the debug port, so say it in the session log
 * whenever the state changes during playback.
 * See documentation/59-fmv-shard-namespace.md. */
static void note_movie_shard(void) {
    static int last=-1;
    int active=0,registered=0;
    overlay_loader_get_status(&active,&registered,NULL,NULL,0,NULL,0,NULL,0,NULL,NULL,NULL);
    int native=active && registered>0;
    if(native==last)return;
    last=native;
    if(native)
        fprintf(stderr,"ttk-fmv: native movie decoder active (%d functions)\n",registered);
    else
        fprintf(stderr,"ttk-fmv: WARNING native movie decoder NOT loaded (%s); the FMV runs "
            "interpreted and may stutter. Rebuild: python3 tools/local/build_movie_overlay.py "
            "[loader: %s]\n",active?"no shard for this config":"overlay cache disabled",
            overlay_loader_last_msg());
}

static void ttk_movie_poll(CPUState *cpu,uint32_t address) {
    (void)address;
    if(enabled<0) {
        const char *e=getenv("DNTTK_FMV_POLL"); enabled=!(e && strcmp(e,"0")==0);
        fprintf(stderr,"ttk-fmv-poll: %s (SLUS-00583 empty STR ring only)\n",enabled?"enabled":"disabled");
        atexit(report);
    }
    if(!enabled)return;
    calls++;
    if(cpu->gpr[31]!=0x800ab378u || g_psx_call_bail || g_precise_mode || g_ls_mode ||
       g_ls_replay_active || psx_get_in_exception() || psx_selfcheck_enabled() ||
       psx_netplay_active() || !code_matches()) {
        have_previous=0;streak=0;return;
    }
    note_movie_shard();
    /* The steady empty path never writes: status 1 wraps the ring; status 2
     * consumes a ready entry. Never skip either path. Validate RAM bounds. */
    uint32_t index=psx_mod_read_word(0x800e811cu);
    uint32_t ring=psx_mod_read_word(0x800e8130u);
    uint32_t phys=ring&0x1fffffffu;
    if(index>1024 || phys<0x10000 || phys+index*32u+32u>0x200000u) {
        have_previous=0;streak=0;return;
    }
    uint16_t status=psx_mod_read_half(ring+index*32u);
    if(status==1 || status==2) {have_previous=0;streak=0;return;}
    psx_cyc_batch_flush();
    /* Measure loop cost on the uncompressed clock (the runtime's CPU overclock
     * pulls psx_cycle_count back at device services). */
    uint64_t now=psx_cycle_count+g_psx_overclock_rewound;
    uint64_t delta=now-last_cycle;
    int stable=have_previous &&
        last_mmio==g_mmio_access_count && previous.gpr[16]>1 &&
        cpu->gpr[16]==previous.gpr[16]-1 && same_state(cpu) && delta>0 && delta<32768;
    if(stable) {
        if(quantum==(uint32_t)delta)streak++;
        else {quantum=(uint32_t)delta;streak=1;}
    } else streak=0;
    if(streak>=3) {
        /* Bring device clocks current before asking for a relative horizon.
         * Ordinary SPU samples do not change this RAM-only polling loop.
         * Stop before every observable IRQ. Disjoint DMA may progress in RAM.
         * Device catch-up still executes
         * all intermediate sample/timer events on their original guest clock. */
        psx_devices_service_to_now();
        extern uint32_t i_stat, i_mask;
        if((i_stat&i_mask) || !dma_disjoint(phys+index*32u) ||
           psx_mod_read_word(0x800e811cu)!=index ||
           psx_mod_read_word(0x800e8130u)!=ring ||
           psx_mod_read_half(ring+index*32u)!=status || !code_matches()) {
            have_previous=0;streak=0;return;
        }
        uint32_t distance=psx_idle_cycles_to_next_observable_event();
        uint32_t count=ttk_poll_iterations(quantum,distance,cpu->gpr[16]);
        if(count) {
            uint32_t advance=count*quantum;
            cpu->gpr[16]-=count;
            g_psx_overclock_exempt+=advance;   /* a skip, never scaled */
            psx_advance_cycles(advance);
            skips++;iterations+=count;cycles+=advance;
        }
    }
    previous=*cpu;last_cycle=psx_cycle_count+g_psx_overclock_rewound;
    last_mmio=g_mmio_access_count;have_previous=1;
}
PSX_MOD_CONSTRUCTOR(register_ttk_movie_poll) {
    psx_mod_register_function_entry_plugin("ttk.fmv.empty_ring",0x800b81bcu,ttk_movie_poll);
}
