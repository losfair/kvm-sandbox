#pragma once

#ifdef LONG_MODE_64
#define LAYOUT_BASE_OFFSET 0xffff800000000000
#else
#define LAYOUT_BASE_OFFSET 0x0
#endif

/*
Memory layout:

0x0 - 0x1000: Stage 1 loader
0x1000 - 0x1008: Address of kernel syscall stack
0x1008 - 0x1010: Saved address of user stack before entering syscall
0x3000 - 0x3010: GDTR
0x3010 - 0x14000: GDT
0x100000 - 0x2000000: Stage 2 loader
0x2000000 - 0x2400000: Fixed data (Page table, GDT, etc.)
0x2400000 - 0x4000000: Stage 3 application image
0x4000000 - *: User memory
*/

#define STAGE1_OFFSET (LAYOUT_BASE_OFFSET + 0x0)
#define GDTR_OFFSET (LAYOUT_BASE_OFFSET + 0x3000)
#define GDT_OFFSET (LAYOUT_BASE_OFFSET + 0x3010)
#define STAGE2_OFFSET (LAYOUT_BASE_OFFSET + 0x100000)
#define FIXED_DATA_OFFSET (LAYOUT_BASE_OFFSET + 0x2000000)
#define STAGE3_OFFSET (LAYOUT_BASE_OFFSET + 0x2400000)
#define USER_OFFSET (LAYOUT_BASE_OFFSET + 0x4000000)
