#include <stdint.h>
#include "mailbox.h"
#include "framebuffer.h"
uint32_t fb_width;
uint32_t fb_height;
uint32_t fb_pitch;
uint8_t* fb_ptr;
int fb_init(uint32_t w, uint32_t h, uint32_t depth){
    mbox[0]=35*4;
    mbox[1]=0x00000000;
    mbox[2]=0x00048003; mbox[3]=8; mbox[4]=8; mbox[5]=w; mbox[6]=h;
    mbox[7]=0x00048004; mbox[8]=8; mbox[9]=8; mbox[10]=w; mbox[11]=h;
    mbox[12]=0x00048005; mbox[13]=4; mbox[14]=4; mbox[15]=depth;
    mbox[16]=0x00048006; mbox[17]=4; mbox[18]=4; mbox[19]=1;
    mbox[20]=0x00040001; mbox[21]=8; mbox[22]=8; mbox[23]=16; mbox[24]=0;
    mbox[25]=0x00040008; mbox[26]=4; mbox[27]=4; mbox[28]=0;
    mbox[29]=0x00000000;
    if(mbox_call(8)){
        fb_ptr=(uint8_t*)((uint64_t)(mbox[24] & 0x3FFFFFFF));
        fb_pitch=mbox[28];
        fb_width=w; fb_height=h;
        return fb_ptr!=0;
    }
    return 0;
}
void fb_clear(uint32_t color){
    uint32_t* p=(uint32_t*)fb_ptr; uint32_t sz=(fb_pitch/4)*fb_height; for(uint32_t i=0;i<sz;i++) p[i]=color;
}
void fb_rect(int x,int y,int w,int h,uint32_t color){
    for(int j=0;j<h;j++){ uint32_t* row=(uint32_t*)(fb_ptr + (y+j)*fb_pitch); for(int i=0;i<w;i++){ row[x+i]=color; } }
}
