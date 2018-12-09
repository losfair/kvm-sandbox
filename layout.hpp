#pragma once

/*
Memory layout:

0x0 - 0x1000: Stage 1 loader
0x1000 - 0x3000: Stage 2 loader
0x3000 - 0x3010: GDTR
0x3010 - 0x14000: GDT
0x14000 - 0x14010: IDTR
0x14010 - 0x14d00: IDT
0x14d00 - 0x15000: TSS
0x15000 - 0x16000: Page directory
0x100000 - 0x500000: Page tables
0x500000 - 0x1000000: Loader stack
0x1000000 - ?: Stage 3 loader
*/

#define STAGE1_OFFSET 0x0
#define STAGE2_OFFSET 0x1000
#define GDTR_OFFSET 0x3000
#define GDT_BASE 0x3010
#define IDTR_OFFSET 0x14000
#define IDT_BASE 0x14010
#define TSS_BASE 0x14d00
#define PAGE_DIRECTORY_BASE 0x15000
#define PAGE_TABLE_BASE 0x100000
#define STAGE3_OFFSET 0x1000000
