#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
extern uint32_t fb_width;
extern uint32_t fb_height;
extern uint32_t fb_pitch;
extern uint8_t* fb_ptr;
int fb_init(uint32_t w, uint32_t h, uint32_t depth);
void fb_clear(uint32_t color);
void fb_rect(int x,int y,int w,int h,uint32_t color);
#ifdef __cplusplus
}
#endif
