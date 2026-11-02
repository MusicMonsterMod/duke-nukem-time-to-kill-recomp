/* D23F fast CPU timing for the recompiled game code (SLUS_005.83_full_*.c).
 *
 * CMakeLists.txt force-includes this header in front of every generated game
 * shard; the generated C stays untouched. It redirects the per-instruction
 * timing hooks the emitter writes into each block to the purpose-built model
 * below whenever ttk_ft_on is set (Modernized option, gameplay only; see
 * fast_timing.c). With the flag clear every hook calls the framework's
 * accurate model exactly as before, so Vanilla, menus, movies and loading are
 * unchanged. Not used by the BIOS, the overlay DLLs or the interpreter.
 *
 * Accurate model (framework psx_cyc.h): per instruction a pipeline step
 * (base cycle, load give-back, register dependencies), per block an
 * instruction-cache lookup, per load the full Beetle read sequence, and at
 * every branch a batch publish plus the full interrupt check. That is most of
 * the emulation thread (D23E/D23F profiles: about 44%; the game code is 4%).
 *
 * Fast model:
 *   - a block charges once, at its leader, from its static instruction count
 *     (g_ttk_ft_instr_q8, calibrated against the accurate model);
 *   - main-RAM loads read RAM directly and charge a flat extra
 *     (g_ttk_ft_load_cycles); other loads keep the accurate path;
 *   - no instruction-cache simulation, no per-instruction pipeline state;
 *   - the branch edge only publishes cycles and runs the full check when it
 *     can matter: a device deadline is reached, an interrupt is pending, a
 *     COP0 software interrupt is raised, or every g_ttk_ft_edge_period edges
 *     for host maintenance (staged savestates, debug requests, lease expiry).
 *     Devices therefore still see every event at the same block boundary as
 *     the accurate model would publish it;
 *   - mult/div and GTE completion stalls, MMIO and stores are unchanged.
 * Debug-server block/call observers are skipped only in player sessions
 * (PSX_FORENSICS=0), where the framework already makes them return at once.
 *
 * D23G step 5: the hooks test a copy of the flag, not the global. Every
 * generated function opens with psx_cyc_bb_defer_begin(), which declares a
 * local ttk_ft_on there (shadowing the global) and reads the global once; the
 * branch edge reads it again after the interrupt check. Calls leave the
 * function (CPS) and re-enter through the same opening, and the flag only
 * changes in host calls (player update lease, slow-edge lapse), so the copy
 * is current at every block start. The per-instruction tests then use a
 * register instead of reloading memory after each write; Vanilla, which
 * never sets the flag, no longer pays a load per instruction. Functions
 * without that opening (alias bodies) still test the global; their edge
 * stores back the value it already holds. */
#ifndef TTK_FAST_TIMING_H
#define TTK_FAST_TIMING_H

#include "../../generated/SLUS_005.83_decls.h"
#include <string.h>

#if defined(PSX_ENABLE_BLOCK_CYCLES) && !defined(PSX_OVERLAY_DLL_BUILD) && !defined(PSX_COSIM)

#ifdef __cplusplus
extern "C" {
#endif
extern int ttk_ft_on;                /* fast model active (fast_timing.c)     */
extern int g_ttk_ft_quiet;           /* player session: observers inert       */
extern uint32_t g_ttk_ft_budget;     /* edges left before a maintenance check */
extern uint32_t g_ttk_ft_load_cycles;
extern uint32_t g_ttk_ft_instr_q8;      /* guest cycles per instruction, 1/256 */
extern uint32_t g_ttk_ft_frac;          /* charged remainder, 1/256 cycle       */
extern uint32_t i_stat, i_mask;
extern volatile uint32_t g_psx_last_fn_entry;
extern uint8_t *g_psx_ram;
void ttk_ft_edge_slow(CPUState *cpu, uint32_t resume_pc);
#ifdef __cplusplus
}
#endif

#if defined(__GNUC__) || defined(__clang__)
#define TTK_FT_LIKELY(x) __builtin_expect(!!(x), 1)
#define TTK_FT_INLINE static inline __attribute__((always_inline))
#else
#define TTK_FT_LIKELY(x) (x)
#define TTK_FT_INLINE static inline
#endif

/* The global flag. Defined before any function can shadow the name. */
TTK_FT_INLINE int ttk_ft_now(void) { return ttk_ft_on; }

/* Generated functions run inside psx_cyc_bb_defer_begin/end, so a charge
 * only accumulates; the branch edge publishes it. */
TTK_FT_INLINE void ttk_ft_charge(uint32_t cycles) {
    if (TTK_FT_LIKELY(g_psx_cyc_bb_defer > 0)) g_psx_cyc_batch += cycles;
    else psx_cyc_charge(cycles);
}

/* Fractional per-instruction charge: keep the remainder in 1/256 cycles. */
TTK_FT_INLINE void ttk_ft_charge_q8(uint32_t q8) {
    const uint32_t f = g_ttk_ft_frac + q8;
    g_ttk_ft_frac = f & 0xFFu;
    ttk_ft_charge(f >> 8);
}

TTK_FT_INLINE int ttk_ft_block(int on, CPUState *cpu, uint32_t addr, uint32_t bcyc, int side_effects) {
    if (on) { ttk_ft_charge_q8(bcyc * g_ttk_ft_instr_q8); return 0; }
    return psx_slice_block(cpu, addr, bcyc, side_effects);
}

TTK_FT_INLINE void ttk_ft_icache_fetch(int on, CPUState *cpu, uint32_t addr) {
    if (!on) psx_icache_fetch(cpu, addr);
}

TTK_FT_INLINE void ttk_ft_step(int on, CPUState *cpu, uint32_t reg_mask) {
    if (!on) psx_cyc_step(cpu, reg_mask);
}

TTK_FT_INLINE int ttk_ft_ram(uint32_t addr, uint32_t *off) {
    const uint32_t phys = addr & 0x1FFFFFFFu;
    if (TTK_FT_LIKELY(phys < 0x00800000u && g_ls_mode == 0 && !g_ds_recording)) {
        *off = phys & 0x1FFFFFu;
        return 1;
    }
    return 0;
}

TTK_FT_INLINE uint32_t ttk_ft_load_word(int on, CPUState *cpu, uint32_t addr, uint32_t rt, uint32_t mask) {
    uint32_t off, v;
    if (on && ttk_ft_ram(addr, &off)) {
        ttk_ft_charge(g_ttk_ft_load_cycles);
        memcpy(&v, g_psx_ram + off, sizeof v);
        return v;
    }
    return psx_cyc_load_word(cpu, addr, rt, mask);
}

TTK_FT_INLINE uint16_t ttk_ft_load_half(int on, CPUState *cpu, uint32_t addr, uint32_t rt, uint32_t mask) {
    uint32_t off;
    uint16_t v;
    if (on && ttk_ft_ram(addr, &off)) {
        ttk_ft_charge(g_ttk_ft_load_cycles);
        memcpy(&v, g_psx_ram + off, sizeof v);
        return v;
    }
    return psx_cyc_load_half(cpu, addr, rt, mask);
}

TTK_FT_INLINE uint8_t ttk_ft_load_byte(int on, CPUState *cpu, uint32_t addr, uint32_t rt, uint32_t mask) {
    uint32_t off;
    if (on && ttk_ft_ram(addr, &off)) {
        ttk_ft_charge(g_ttk_ft_load_cycles);
        return g_psx_ram[off];
    }
    return psx_cyc_load_byte(cpu, addr, rt, mask);
}

/* The emitter writes "psx_cyc_bb_defer_flush(); psx_check_interrupts_at()"
 * at every branch edge. The fast model publishes inside the edge instead. */
TTK_FT_INLINE void ttk_ft_defer_flush(int on) {
    if (!on) psx_cyc_bb_defer_flush();
}

/* Returns the flag for the next block: unchanged on the inline fast path,
 * read again after a host call (lapse, interrupt check, lease). */
TTK_FT_INLINE int ttk_ft_edge(int on, CPUState *cpu, uint32_t resume_pc) {
    if (on) {
        if (TTK_FT_LIKELY(psx_cycle_count + g_psx_cyc_batch < psx_next_service_cycle &&
                          (i_stat & i_mask) == 0u &&
                          (cpu->cop0[13] & 0x300u) == 0u &&
                          --g_ttk_ft_budget != 0u))
            return 1;
        ttk_ft_edge_slow(cpu, resume_pc);
    } else {
        psx_check_interrupts_at(cpu, resume_pc);
    }
    return ttk_ft_now();
}

#ifndef PSX_NO_DEBUG_TOOLS
TTK_FT_INLINE void ttk_ft_observe(int on, uint32_t addr) {
    if (!(on && g_ttk_ft_quiet)) debug_server_cyc_observe(addr);
}
TTK_FT_INLINE void ttk_ft_call_entry(int on, uint32_t addr) {
    if (on && g_ttk_ft_quiet) g_psx_last_fn_entry = addr;
    else debug_server_log_call_entry(addr);
}
#define debug_server_cyc_observe(addr) ttk_ft_observe(ttk_ft_on, (addr))
#define debug_server_log_call_entry(addr) ttk_ft_call_entry(ttk_ft_on, (addr))
#endif

/* The local copy (see the top of this file); the self-reference inside the
 * macro is not expanded again, so it still calls the framework's function. */
#define psx_cyc_bb_defer_begin() psx_cyc_bb_defer_begin(); int ttk_ft_on = ttk_ft_now()
#define psx_slice_block(cpu, addr, bcyc, side_effects) ttk_ft_block(ttk_ft_on, (cpu), (addr), (bcyc), (side_effects))
#define psx_icache_fetch(cpu, addr) ttk_ft_icache_fetch(ttk_ft_on, (cpu), (addr))
#define psx_cyc_step(cpu, mask) ttk_ft_step(ttk_ft_on, (cpu), (mask))
#define psx_cyc_load_word(cpu, addr, rt, mask) ttk_ft_load_word(ttk_ft_on, (cpu), (addr), (rt), (mask))
#define psx_cyc_load_half(cpu, addr, rt, mask) ttk_ft_load_half(ttk_ft_on, (cpu), (addr), (rt), (mask))
#define psx_cyc_load_byte(cpu, addr, rt, mask) ttk_ft_load_byte(ttk_ft_on, (cpu), (addr), (rt), (mask))
#define psx_cyc_bb_defer_flush() ttk_ft_defer_flush(ttk_ft_on)
#define psx_check_interrupts_at(cpu, pc) (ttk_ft_on = ttk_ft_edge(ttk_ft_on, (cpu), (pc)))

#endif
#endif
