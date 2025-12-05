#include <stdint.h>
#include "mailbox.h"
volatile uint32_t mbox[64];
int mbox_call(uint8_t ch){
    uint32_t addr = (uint32_t)((uintptr_t)&mbox);
    while(MBOX_STATUS & MBOX_FULL){}
    MBOX_WRITE = (addr & ~0xF) | (ch & 0xF);
    for(;;){
        while(MBOX_STATUS & MBOX_EMPTY){}
        uint32_t r = MBOX_READ;
        if((r & 0xF) == ch && (r & ~0xF) == (addr & ~0xF)){
            return mbox[1] == 0x80000000;
        }
    }
}
