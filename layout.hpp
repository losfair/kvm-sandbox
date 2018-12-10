#pragma once

/*
Memory layout:

0x0 - 0x3000: Stage 1 loader
0x3000 - 0x3010: GDTR
0x3010 - 0x14000: GDT
0x100000 - 0x2000000: Stage 2 loader
0x2000000 - 0x3000000: Fixed data (Page table, GDT, etc.)
0x3000000 - 0x5000000: Stage 3
*/

#define STAGE1_OFFSET 0x0
#define GDTR_OFFSET 0x3000
#define GDT_OFFSET 0x3010
#define STAGE2_OFFSET 0x100000
#define FIXED_DATA_OFFSET 0x2000000
#define STAGE3_OFFSET 0x3000000
