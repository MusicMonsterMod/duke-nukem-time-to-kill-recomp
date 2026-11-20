#pragma once
// D60: the FAMILY JEWEL flag rules (jewel_device.inc, note 516), on the low
// halfwords of Duke's mission flags for items 14, 15 and 16 (f[0..2]). Pure,
// so the native tests check them without the game.
//
// Bit 0 found (the game's), 0x4000 used (D08A19's marker), 0x0c00 the item
// this jewel is lent to (W - 13), 0x3000 the place it went in (1..3).
#include <cstdint>

namespace ttk::jewel_rules {
constexpr unsigned first_item=14,items=3;
constexpr uint16_t found_bit=1,used_bit=0x4000,lend_mask=0x0c00,order_mask=0x3000;
constexpr unsigned lend_shift=10,order_shift=12;

inline bool is_jewel(int item) {return item>=(int)first_item && item<(int)(first_item+items);}
// The item jewel f is lent to, or -1.
inline int lent_to(uint16_t f) {
    const unsigned v=(f&lend_mask)>>lend_shift;
    return v && v<=items?(int)(first_item+v-1):-1;
}
inline bool used(uint16_t f) {return !(f&found_bit) && (f&used_bit);}
inline unsigned order(uint16_t f) {return (f&order_mask)>>order_shift;}
inline bool lend_active(const uint16_t* f) {
    for(unsigned i=0;i<items;++i) if(lent_to(f[i])>=0)return true;
    return false;
}
// The jewel lent to item w, or -1.
inline int lent_from(const uint16_t* f,unsigned w) {
    for(unsigned i=0;i<items;++i) if(lent_to(f[i])==(int)w)return (int)(first_item+i);
    return -1;
}
// One after the other used jewels that have a place.
inline unsigned next_order(const uint16_t* f,unsigned except) {
    unsigned placed=0;
    for(unsigned i=0;i<items;++i) if(first_item+i!=except && used(f[i]) && order(f[i]))++placed;
    return placed+1<items?placed+1:items;
}
// Trade the low halfwords of W and J; J records the lend when lending.
inline void trade(uint16_t* f,unsigned w,unsigned j,bool lend) {
    uint16_t& fw=f[w-first_item];uint16_t& fj=f[j-first_item];
    const uint16_t a=fw,b=(uint16_t)(fj&~lend_mask);
    fw=b;fj=(uint16_t)(a|(lend?((w-first_item+1)<<lend_shift):0));
}
// The jewel that answers a lock wanting W: the preferred one when found,
// else W itself when found, else the first found jewel; -1 when none.
inline int choice(const uint16_t* f,unsigned w,int preferred) {
    if(is_jewel(preferred) && (f[preferred-first_item]&found_bit))return preferred;
    if(f[w-first_item]&found_bit)return (int)w;
    for(unsigned i=0;i<items;++i) if(f[i]&found_bit)return (int)(first_item+i);
    return -1;
}
enum Event : unsigned {returned=1,undone=2,placed=4};
// Once per player update. use_running: a use is in progress (stage
// +0x27d nonzero). held/request: the hand's item (+0x3b9) and the use's
// item (+0x3bb), rewritten from W to J on a return.
inline unsigned update(uint16_t* f,bool use_running,uint8_t& held,uint8_t& request) {
    unsigned events=0;
    for(unsigned i=0;i<items;++i) {
        const unsigned j=first_item+i;
        const int w=lent_to(f[i]);
        if(w<0)continue;
        if(f[w-first_item]&found_bit) {
            // Not taken in hand yet: wait while the use runs, else undo.
            if(!use_running) {trade(f,(unsigned)w,j,false);events|=undone;}
            continue;
        }
        // Used up as W: J is the used jewel, in the next place.
        trade(f,(unsigned)w,j,false);
        const unsigned place=next_order(f,j);
        f[i]=(uint16_t)(((f[i]|used_bit)&~order_mask&~found_bit)|(place<<order_shift));
        if(held==(unsigned)w)held=(uint8_t)j;
        if(request==(unsigned)w)request=(uint8_t)j;
        events|=returned;
    }
    if(lend_active(f))return events;
    for(unsigned i=0;i<items;++i) {
        if(used(f[i]) && !order(f[i])) {f[i]=(uint16_t)(f[i]|(next_order(f,first_item+i)<<order_shift));events|=placed;}
        else if(!used(f[i]) && order(f[i]))f[i]=(uint16_t)(f[i]&~order_mask);   // found again: no place
    }
    return events;
}
// The jewels as they are, a lend included, without the host's bits
// (mission[] indexed by item).
inline void mission_view(uint16_t* mission) {
    for(unsigned j=first_item;j<first_item+items;++j) {
        const int w=lent_to(mission[j]);
        if(w<0)continue;
        const uint16_t fw=mission[w],fj=(uint16_t)(mission[j]&~lend_mask);
        mission[w]=fj;mission[j]=fw;
    }
    for(unsigned i=first_item;i<first_item+items;++i) mission[i]=(uint16_t)(mission[i]&~(lend_mask|order_mask));
}
}
