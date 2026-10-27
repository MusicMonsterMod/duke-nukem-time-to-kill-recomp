/* Compile with -ffunction-sections -fdata-sections and --gc-sections. Only the
 * actual independent-reader/decoder functions are linked from the runtime. */
#include "../../psxrecomp/runtime/src/cdrom.c"
#include <assert.h>
static uint8_t test_sector[2352];
static int read_failure;
int iso_read_raw_sector(void* h,uint32_t lba,uint8_t* buffer,int size) {
    assert(h==(void*)1 && lba<200000 && size==2352);
    if(read_failure)return 0;
    memcpy(buffer,test_sector,2352);return 1;
}
uint32_t iso_sector_count(void* h){assert(h==(void*)1);return 200000;}
int main(void) {
    iso_handle=(void*)1;
    int16_t output[9408*2],native[XA_NATIVE_FRAMES*2],reference[9408*2];
    for(int coding=0;coding<=5;++coding) {
        if(coding==2 || coding==3)continue;
        memset(test_sector,0,sizeof(test_sector));
        test_sector[15]=2;test_sector[16]=1;test_sector[17]=2;test_sector[18]=0x64;test_sector[19]=coding;
        memcpy(test_sector+20,test_sector+16,4);
        for(int g=0;g<18;++g) {
            for(int j=0;j<16;++j)test_sector[24+g*128+j]=0x29;
            for(int j=16;j<128;++j)test_sector[24+g*128+j]=(uint8_t)(j*17+g);
        }
        xa_hist_l[0]=101;xa_hist_l[1]=-42;xa_hist_r[0]=302;xa_hist_r[1]=-203;
        int frames=(coding&1)?xa_decode_sector_4bit_stereo(test_sector+24,native):xa_decode_sector_4bit_mono(test_sector+24,native);
        int n=xa_resample_to_44100(native,frames,(coding&4)?18900:37800,reference,9408);
        int want_history[4]={xa_hist_l[0],xa_hist_l[1],xa_hist_r[0],xa_hist_r[1]};
        xa_hist_l[0]=7;xa_hist_l[1]=8;xa_hist_r[0]=9;xa_hist_r[1]=10;
        xa_stream_file=3;xa_stream_channel=7;xa_stream_coding=1;xa_stream_active=1;
        int history[4]={101,-42,302,-203};
        assert(cdrom_host_xa_read(144480,1,2,history,output)==n);
        assert(!memcmp(output,reference,(size_t)n*4));assert(!memcmp(history,want_history,sizeof(history)));
        assert(xa_hist_l[0]==7 && xa_hist_l[1]==8 && xa_hist_r[0]==9 && xa_hist_r[1]==10);
        assert(xa_stream_file==3 && xa_stream_channel==7 && xa_stream_coding==1 && xa_stream_active==1);
        assert(cdrom_host_xa_read(144480,1,3,history,output)==0);
        assert(!memcmp(history,want_history,sizeof(history)));
        test_sector[20]^=1;assert(cdrom_host_xa_read(144480,1,2,history,output)==-1);
        test_sector[20]^=1;test_sector[19]=test_sector[23]=0x10;
        assert(cdrom_host_xa_read(144480,1,2,history,output)==-1);
        assert(!memcmp(history,want_history,sizeof(history)));
    }
    assert(cdrom_host_xa_read(-1,1,2,(int[4]){0},output)==-1);
    assert(cdrom_host_xa_read(200000,1,2,(int[4]){0},output)==-1);
    read_failure=1;assert(cdrom_host_xa_read(0,1,2,(int[4]){0},output)==-1);
    puts("independent XA parity and native-state isolation PASS");
}
