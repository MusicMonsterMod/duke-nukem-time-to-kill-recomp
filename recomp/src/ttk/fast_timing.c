/* D23F fast CPU timing: runtime side of fast_timing.h.
 *
 * DNTTK_CPU_TIMING=fast (run.py passes it for Modernized when the profile's
 * cpu_timing is "fast") enables the model. Like the CPU overclock it is leased
 * from the player update: it is active during gameplay and lapses twelve video
 * fields after the updates stop, so boot, menus, movies and loading always run
 * on the accurate model. Lockstep replay and conservative event stepping
 * (diagnostics) also keep the accurate model.
 *
 * Developer overrides (not player settings): DNTTK_FT_INSTR_Q8 (guest cycles
 * per instruction in 1/256), DNTTK_FT_LOAD_CYCLES (flat extra per main-RAM
 * load) and DNTTK_FT_EDGE_PERIOD (edges between maintenance
 * checks, power of two not required). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu_state.h"
#include "psx_cycles.h"
#include "interrupts.h"

/* Extra guest cycles per main-RAM load on top of the block's per-instruction
 * charge (fast_timing.h TTK_FT_INSTR_Q8). Calibrated against the accurate
 * model: documentation/105-d23f-fast-timing.md. */
#define TTK_FT_LOAD_CYCLES 6u
#define TTK_FT_INSTR_Q8 400u
#define TTK_FT_EDGE_PERIOD 64u
/* Twelve video fields: longer than the slowest gameplay frame (6 fields at
 * 100% in the western town), so the model never lapses between two player
 * updates; menus, movies and loading still return to accurate within 0.2 s. */
#define TTK_FT_LEASE_CYCLES (12ull * 564480ull)

int g_ttk_ft_on = 0;
int g_ttk_ft_quiet = 0;
uint32_t g_ttk_ft_budget = TTK_FT_EDGE_PERIOD;
uint32_t g_ttk_ft_load_cycles = TTK_FT_LOAD_CYCLES;
uint32_t g_ttk_ft_instr_q8 = TTK_FT_INSTR_Q8;
uint32_t g_ttk_ft_frac = 0;

static int s_enabled = 0;
static uint32_t s_period = TTK_FT_EDGE_PERIOD;
static uint64_t s_until = 0;
static uint64_t s_slow_edges = 0, s_maintenance = 0, s_leases = 0, s_lapses = 0;

extern int g_ls_replay_active;
extern int g_event_step_conservative;

__attribute__((constructor)) static void ttk_ft_from_env(void) {
    const char *e = getenv("DNTTK_CPU_TIMING");
    s_enabled = e && strcmp(e, "fast") == 0;
    e = getenv("PSX_FORENSICS");
    g_ttk_ft_quiet = e && e[0] == '0';
    e = getenv("DNTTK_FT_LOAD_CYCLES");
    if (e && *e) g_ttk_ft_load_cycles = (uint32_t)strtoul(e, NULL, 10);
    e = getenv("DNTTK_FT_INSTR_Q8");
    if (e && *e) g_ttk_ft_instr_q8 = (uint32_t)strtoul(e, NULL, 10);
    e = getenv("DNTTK_FT_EDGE_PERIOD");
    if (e && *e) {
        unsigned long v = strtoul(e, NULL, 10);
        s_period = v ? (uint32_t)v : 1u;
    }
    g_ttk_ft_budget = s_period;
    if (s_enabled)
        fprintf(stderr, "[TTK cpu] timing: fast during gameplay (%.2f/instruction, load +%u, maintenance every %u edges)\n",
                g_ttk_ft_instr_q8 / 256.0, g_ttk_ft_load_cycles, s_period);
}

int ttk_fast_timing_enabled(void) { return s_enabled; }

/* Player update (Modernized gameplay), next to the overclock lease. */
void ttk_fast_timing_renew(void) {
    if (!s_enabled) return;
    s_until = psx_cycle_count + TTK_FT_LEASE_CYCLES;
    if (!g_ttk_ft_on && !g_ls_replay_active && !g_event_step_conservative) {
        g_ttk_ft_on = 1;
        g_ttk_ft_budget = s_period;
        ++s_leases;
    }
}

void ttk_ft_edge_slow(CPUState *cpu, uint32_t resume_pc) {
    ++s_slow_edges;
    if (g_ttk_ft_budget == 0u) ++s_maintenance;
    /* The clock can also move back (savestate load, overclock rewind). */
    if (psx_cycle_count > s_until || s_until - psx_cycle_count > 2u * TTK_FT_LEASE_CYCLES ||
        g_ls_replay_active || g_event_step_conservative) {
        g_ttk_ft_on = 0;
        ++s_lapses;
    }
    g_ttk_ft_budget = s_period;
    psx_cyc_batch_flush();
    psx_check_interrupts_at(cpu, resume_pc);
}

const char *ttk_fast_timing_calibration_json(void);
const char *ttk_fast_timing_json(void) {
    static char buffer[1536];
    snprintf(buffer, sizeof buffer,
             "{\"enabled\":%s,\"active\":%s,\"instr_q8\":%u,\"load_cycles\":%u,\"edge_period\":%u,\"slow_edges\":%llu,"
             "\"maintenance\":%llu,\"leases\":%llu,\"lapses\":%llu,\"calibration\":%s}",
             s_enabled ? "true" : "false", g_ttk_ft_on ? "true" : "false", g_ttk_ft_instr_q8, g_ttk_ft_load_cycles, s_period,
             (unsigned long long)s_slow_edges, (unsigned long long)s_maintenance,
             (unsigned long long)s_leases, (unsigned long long)s_lapses, ttk_fast_timing_calibration_json());
    return buffer;
}

#ifdef TTK_FT_CALIBRATE
/* Calibration build (fast_timing_calibrate.h): least-squares sums of the
 * accurate cycles per edge interval against instructions, RAM loads and
 * instruction-cache misses. */
uint32_t g_ttk_cal_instr, g_ttk_cal_loads, g_ttk_cal_misses, g_ttk_cal_kept;
uint64_t g_ttk_cal_mark;
static double s_xx[3][3], s_xy[3], s_x[3], s_y, s_yy, s_n, s_rejected;
void ttk_cal_begin(void) {
    g_ttk_cal_instr = g_ttk_cal_loads = g_ttk_cal_misses = g_ttk_cal_kept = 0;
    g_ttk_cal_mark = psx_cycle_count + g_psx_cyc_batch;
}
void ttk_cal_edge(void) {
    const uint64_t now = psx_cycle_count + g_psx_cyc_batch;
    const double x[3] = {g_ttk_cal_instr, g_ttk_cal_loads, g_ttk_cal_misses};
    const double y = (double)(int64_t)(now - g_ttk_cal_mark) - (double)g_ttk_cal_kept;
    if (g_ttk_cal_instr && y >= 0.0 && y < 20.0 * x[0] + 200.0) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) s_xx[i][j] += x[i] * x[j];
            s_xy[i] += x[i] * y;
            s_x[i] += x[i];
        }
        s_y += y;
        s_yy += y * y;
        s_n += 1.0;
    } else if (g_ttk_cal_instr) {
        s_rejected += 1.0;
    }
    g_ttk_cal_instr = g_ttk_cal_loads = g_ttk_cal_misses = g_ttk_cal_kept = 0;
}
const char *ttk_fast_timing_calibration_json(void) {
    static char b[1024];
    snprintf(b, sizeof b, "{\"n\":%.0f,\"rejected\":%.0f,\"xx\":[%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g],\"xy\":[%.17g,%.17g,%.17g],\"yy\":%.17g,\"x\":[%.17g,%.17g,%.17g],\"y\":%.17g}",
             s_n, s_rejected, s_xx[0][0], s_xx[0][1], s_xx[0][2], s_xx[1][0], s_xx[1][1], s_xx[1][2],
             s_xx[2][0], s_xx[2][1], s_xx[2][2], s_xy[0], s_xy[1], s_xy[2], s_yy, s_x[0], s_x[1], s_x[2], s_y);
    return b;
}
#else
const char *ttk_fast_timing_calibration_json(void) { return "null"; }
#endif
