/* D34B: real one-seat encoder -> two-seat decoder, including rumble and
 * a mid-transaction snapshot. No retail data or player files. */
#include "sio.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint32_t sio_snapshot_bytes(void);
void sio_snapshot_write(uint8_t *p);
int sio_snapshot_read(const uint8_t *p, uint32_t len);

uint32_t i_stat = 0;
uint32_t i_mask = 0;
uint32_t g_debug_current_func_addr = 0;
uint32_t g_debug_last_store_pc = 0;
int psx_get_in_exception(void) { return 0; }
uint8_t psx_read_byte(uint32_t addr) { (void)addr; return 0; }
uint32_t psx_read_word(uint32_t addr) { (void)addr; return 0; }

/* sio.c gates card-transfer deferral on `psx_get_cycle_count() < deadline`, so
 * a constant clock would defer forever. Advance monotonically. */
uint64_t psx_get_cycle_count(void) {
    static uint64_t t;
    return t += 64;
}
uint32_t memory_get_sr(void) { return 0; }
void debug_server_poll(void) {}
void debug_server_log_sio_write(uint32_t a, uint32_t v, uint8_t w) {
    (void)a; (void)v; (void)w;
}
void starvation_ring_record(uint8_t k,uint8_t tx,uint8_t rx,uint16_t c,uint16_t st,
 int d,int ss,int ms,int mc,int sec,uint8_t di,uint32_t f,
 uint8_t a,uint8_t b,uint8_t x,uint8_t y,int t) {}
void card_read_summary_record(uint8_t s,uint8_t c,uint16_t sec,uint8_t e,uint8_t chk,const uint8_t *data) {}
void card_data_writes_arm(uint8_t v, uint16_t s, uint8_t i, uint8_t sl) {
    (void)v; (void)s; (void)i; (void)sl;
}
void event_ring_record_aux(uint16_t k, uint8_t d, uint32_t a) {
    (void)k; (void)d; (void)a;
}
void psx_irq_raise(uint32_t bit, uint32_t detail) {
    (void)detail; i_stat |= 1u << bit;
}
int memcard_is_present(int card) { (void)card; return 0; }
int memcard_read_sector(int card, int sector, uint8_t *buf) {
    (void)card; (void)sector; memset(buf, 0, 128); return -1;
}
int memcard_write_sector(int card, int sector, const uint8_t *buf) {
    (void)card; (void)sector; (void)buf; return -1;
}
void memcard_flush(int card) { (void)card; }

static int failures;

#define EXPECT(label, expected, actual) do {                                  \
    unsigned e_ = (unsigned)(expected), a_ = (unsigned)(actual);              \
    if (e_ != a_) {                                                           \
        fprintf(stderr, "FAIL %s: expected=0x%X actual=0x%X\n",             \
                label, e_, a_);                                               \
        failures++;                                                           \
    }                                                                         \
} while (0)

#include "../../psxrecomp/runtime/src/sio.c"
int main(int argc, char **argv) {
    if (argc != 3) return 2;
    sio_init();
    sio_connect_pad(0);
    sio_set_pad_analog(0, 1, 0x81, 0x82, 0x83, 0x84);
    if (!strcmp(argv[1], "write")) {
        pad_analog[0]=1;pad_in_config[0]=1;pad_type_req[0]=-1;
        pad_state=PAD_SEND_RESPONSE;selected_slot=0;
        pad_response[0]=0x73;pad_response[1]=0x5a;
        pad_response_len=8;pad_response_idx=2;pad_current_cmd=0x42;
        pad_rumble_map[0][0]=0;pad_rumble_map[0][1]=1;
        pad_rumble_small[0]=1;pad_rumble_large[0]=128;
        sio_stat=0x205;mc_slots[1].sector=123;mc_slots[1].data[37]=0xa5;
        uint32_t n=sio_snapshot_bytes();uint8_t *p=malloc(n);
        sio_snapshot_write(p);FILE *f=fopen(argv[2],"wb");
        if(!f || fwrite(p,1,n,f)!=n) return 2;
        fclose(f);free(p);return failures!=0;
    }
#if PSX_MAX_PLAYERS == 2
    FILE *f=fopen(argv[2],"rb");if(!f)return 2;
    uint8_t data[2048];size_t n=fread(data,1,sizeof data,f);fclose(f);
    EXPECT("old one-seat size",sio_snapshot_bytes()-12,n);
    EXPECT("old one-seat load",1,sio_snapshot_read(data,n));
    uint8_t small,large;sio_get_pad_rumble(0,&small,&large);
    EXPECT("preserve P1 small",1,small);EXPECT("preserve P1 large",128,large);
    sio_get_pad_rumble(1,&small,&large);
    EXPECT("P2 small neutral",0,small);EXPECT("P2 large neutral",0,large);
    EXPECT("P2 released",65535,sio_get_pad_buttons_slot(1));
    EXPECT("pre-rumble one-seat load",1,sio_snapshot_read(data,n-8));
    EXPECT("truncated reject",0,sio_snapshot_read(data,n-1));
    EXPECT("oversized reject",0,sio_snapshot_read(data,n+1));
    EXPECT("P1 config retained",1,pad_in_config[0]);
    EXPECT("mid-poll state",PAD_SEND_RESPONSE,pad_state);
    EXPECT("mid-poll response index",2,pad_response_idx);
    EXPECT("card sector retained",123,mc_slots[1].sector);
    EXPECT("card data retained",0xa5,mc_slots[1].data[37]);
    sio_connect_pad(1);pad_analog[1]=1;pad_type_req[1]=0;
    uint32_t size=sio_snapshot_bytes();uint8_t *round=malloc(size);
    sio_snapshot_write(round);pad_analog[1]=0;
    EXPECT("two-seat roundtrip",1,sio_snapshot_read(round,size));
    EXPECT("second seat retained",1,pad_analog[1]);
    free(round);return failures!=0;
#else
    return 2;
#endif
}
