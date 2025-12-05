#pragma once
#include <stdint.h>
#define MMIO_BASE 0x3F000000UL
#define MAILBOX_BASE (MMIO_BASE + 0xB880)
#define MBOX_READ   (*(volatile uint32_t*)(MAILBOX_BASE + 0x00))
#define MBOX_STATUS (*(volatile uint32_t*)(MAILBOX_BASE + 0x18))
#define MBOX_WRITE  (*(volatile uint32_t*)(MAILBOX_BASE + 0x20))
#define MBOX_EMPTY  0x40000000
#define MBOX_FULL   0x80000000
extern volatile uint32_t mbox[64];
int mbox_call(uint8_t ch);
