/* Read-only Mode 2 EDC/ECC validator. ISO/IEC CD sector layout; no repair. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static uint32_t edc_table[256];
static uint8_t forward[256], backward[256];
static void init(void) {
    for (unsigned i=0;i<256;i++) {
        uint32_t x=i;
        for(unsigned j=0;j<8;j++) x=(x>>1)^((x&1)?0xd8018001u:0);
        edc_table[i]=x;
        unsigned y=i<<1; if(y&256)y^=0x11d;
        forward[i]=(uint8_t)y; backward[i^y]=(uint8_t)i;
    }
}
static uint32_t edc(const uint8_t *p,unsigned n) {
    uint32_t x=0; while(n--)x=(x>>8)^edc_table[(x^*p++)&255]; return x;
}
static uint32_t le32(const uint8_t *p) {
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void ecc(const uint8_t *src,unsigned major,unsigned minor,unsigned mult,unsigned inc,uint8_t *out) {
    unsigned size=major*minor;
    for(unsigned m=0;m<major;m++) {
        unsigned idx=(m>>1)*mult+(m&1); uint8_t a=0,b=0;
        for(unsigned k=0;k<minor;k++) {
            uint8_t v=src[idx]; idx=(idx+inc)%size; a^=v; b^=v; a=forward[a];
        }
        a=backward[forward[a]^b]; out[m]=a; out[m+major]=a^b;
    }
}
int main(int argc,char **argv) {
    if(argc!=2){fprintf(stderr,"usage: sector_check IMAGE\n");return 2;}
    FILE *f=fopen(argv[1],"rb");if(!f){perror("open");return 2;} init();
    uint8_t s[2352],parity[172]; size_t n; unsigned long sectors=0,f1=0,f2=0,bad=0,zero=0;
    unsigned long sync=0,sub=0,edcerr=0,perr=0,qerr=0,mode=0;
    while((n=fread(s,1,sizeof(s),f))==sizeof(s)) {
        int failed=0;
        if(s[0]||s[11]||memcmp(s+1,"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff",10)){sync++;failed=1;}
        if(s[15]!=2){mode++;failed=1;}
        else {
            if(memcmp(s+16,s+20,4)){sub++;failed=1;}
            if(s[18]&32){f2++;uint32_t stored=le32(s+2348);if(!stored)zero++;else if(edc(s+16,2332)!=stored){edcerr++;failed=1;}}
            else {
                f1++;if(edc(s+16,2056)!=le32(s+2072)){edcerr++;failed=1;}
                memset(s+12,0,4);ecc(s+12,86,24,2,86,parity);
                if(memcmp(parity,s+2076,172)){perr++;failed=1;}
                ecc(s+12,52,43,86,88,parity);
                if(memcmp(parity,s+2248,104)){qerr++;failed=1;}
            }
        }
        if(failed){if(bad<20)fprintf(stderr,"invalid sector LBA %lu\n",sectors);bad++;}sectors++;
    }
    if(ferror(f)){perror("read");fclose(f);return 2;}fclose(f);
    printf("{\"sectors\":%lu,\"form1\":%lu,\"form2\":%lu,\"bad_sectors\":%lu,\"bad_sync\":%lu,\"bad_mode\":%lu,\"bad_subheaders\":%lu,\"bad_edc\":%lu,\"bad_ecc_p\":%lu,\"bad_ecc_q\":%lu,\"form2_zero_edc\":%lu,\"trailing_bytes\":%zu}\n",sectors,f1,f2,bad,sync,mode,sub,edcerr,perr,qerr,zero,n);
    return bad||n?1:0;
}
