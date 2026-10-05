/* D23F calibration build (developer only: cmake -DTTK_FT_CALIBRATE=ON).
 * Force-included instead of fast_timing.h. The game code runs on the accurate
 * model unchanged; this only measures it. Between two branch edges of game
 * code it records the guest cycles the accurate model charged, minus the
 * charges the fast model keeps (mult/div and GTE stalls, LWC2, non-RAM loads),
 * against the executed instructions, main-RAM loads and instruction-cache
 * misses. fast_timing.c accumulates least-squares sums (ttk_input JSON). */
#ifndef TTK_FAST_TIMING_CALIBRATE_H
#define TTK_FAST_TIMING_CALIBRATE_H

#include "../../generated/SLUS_005.83_decls.h"

#ifdef __cplusplus
extern "C" {
#endif
extern uint32_t g_ttk_cal_instr, g_ttk_cal_loads, g_ttk_cal_misses, g_ttk_cal_kept;
extern uint64_t g_ttk_cal_mark;
void ttk_cal_edge(void);
void ttk_cal_begin(void);
#ifdef __cplusplus
}
#endif

static inline void ttk_cal_begin_inline(void) { psx_cyc_bb_defer_begin(); ttk_cal_begin(); }
static inline uint64_t ttk_cal_now(void) { return psx_cycle_count + g_psx_cyc_batch; }

static inline int ttk_cal_block(CPUState *cpu, uint32_t addr, uint32_t bcyc, int se) {
    g_ttk_cal_instr += bcyc;
    return psx_slice_block(cpu, addr, bcyc, se);
}
static inline void ttk_cal_icache(CPUState *cpu, uint32_t addr) {
    if (g_psx_icache_active > 0 && g_psx_icache_tv[(addr & 0xFFCu) >> 2] != addr) ++g_ttk_cal_misses;
    psx_icache_fetch(cpu, addr);
}
static inline int ttk_cal_is_ram(uint32_t addr) {
    return (addr & 0x1FFFFFFFu) < 0x00800000u && g_ls_mode == 0 && !g_ds_recording;
}
#define TTK_CAL_LOAD(T, fn) \
    static inline T ttk_cal_##fn(CPUState *cpu, uint32_t addr, uint32_t rt, uint32_t mask) { \
        if (ttk_cal_is_ram(addr)) { ++g_ttk_cal_loads; return fn(cpu, addr, rt, mask); } \
        const uint64_t t0 = ttk_cal_now(); T v = fn(cpu, addr, rt, mask); \
        g_ttk_cal_kept += (uint32_t)(ttk_cal_now() - t0); return v; }
TTK_CAL_LOAD(uint32_t, psx_cyc_load_word)
TTK_CAL_LOAD(uint16_t, psx_cyc_load_half)
TTK_CAL_LOAD(uint8_t, psx_cyc_load_byte)
#define TTK_CAL_KEEP(call) do { const uint64_t t0 = ttk_cal_now(); call; \
    g_ttk_cal_kept += (uint32_t)(ttk_cal_now() - t0); } while (0)
static inline void ttk_cal_muldiv_stall(CPUState *cpu) { TTK_CAL_KEEP(psx_muldiv_stall(cpu)); }
static inline void ttk_cal_gte_stall(CPUState *cpu) { TTK_CAL_KEEP(psx_gte_stall(cpu)); }
static inline void ttk_cal_gte_read(CPUState *cpu, uint32_t rt) { TTK_CAL_KEEP(psx_gte_read(cpu, rt)); }
static inline void ttk_cal_gte_execute(CPUState *cpu, uint32_t cmd) { TTK_CAL_KEEP(gte_execute(cpu, cmd)); }
static inline uint32_t ttk_cal_lwc2_read(CPUState *cpu, uint32_t addr) {
    const uint64_t t0 = ttk_cal_now(); uint32_t v = psx_cyc_lwc2_read(cpu, addr);
    g_ttk_cal_kept += (uint32_t)(ttk_cal_now() - t0); return v;
}
/* Measure up to the edge, then let the accurate check run (interrupt handlers
 * charge their own cycles, outside the interval). */
static inline void ttk_cal_edge_at(CPUState *cpu, uint32_t pc) {
    ttk_cal_edge();
    psx_check_interrupts_at(cpu, pc);
    g_ttk_cal_mark = ttk_cal_now();
}

#define psx_cyc_bb_defer_begin() ttk_cal_begin_inline()
#define psx_slice_block(cpu, addr, bcyc, se) ttk_cal_block((cpu), (addr), (bcyc), (se))
#define psx_icache_fetch(cpu, addr) ttk_cal_icache((cpu), (addr))
#define psx_cyc_load_word(cpu, addr, rt, mask) ttk_cal_psx_cyc_load_word((cpu), (addr), (rt), (mask))
#define psx_cyc_load_half(cpu, addr, rt, mask) ttk_cal_psx_cyc_load_half((cpu), (addr), (rt), (mask))
#define psx_cyc_load_byte(cpu, addr, rt, mask) ttk_cal_psx_cyc_load_byte((cpu), (addr), (rt), (mask))
#define psx_muldiv_stall(cpu) ttk_cal_muldiv_stall(cpu)
#define psx_gte_stall(cpu) ttk_cal_gte_stall(cpu)
#define psx_gte_read(cpu, rt) ttk_cal_gte_read((cpu), (rt))
#define gte_execute(cpu, cmd) ttk_cal_gte_execute((cpu), (cmd))
#define psx_cyc_lwc2_read(cpu, addr) ttk_cal_lwc2_read((cpu), (addr))
#define psx_check_interrupts_at(cpu, pc) ttk_cal_edge_at((cpu), (pc))

#endif
