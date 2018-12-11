#include "kheap.h"
#include "offsets.h"

static const uint8_t *KHEAP_END = (uint8_t *) (FIXED_DATA_OFFSET + 0x1000000);
uint8_t *kheap_top = KERNEL_HEAP;

uint8_t *kmalloc(uint64_t size) {
    if(kheap_top + size < kheap_top || kheap_top + size > KHEAP_END) {
        return 0;
    }
    uint8_t *ret = kheap_top;
    kheap_top += size;
    return ret;
}

void kfree(uint8_t *ptr) {
    // TODO
}
