#pragma once

#ifndef LONG_MODE_64
#error "kheap only supports long mode"
#endif

#include <stdint.h>

uint8_t *kmalloc(uint64_t size);
void kfree(uint8_t *ptr);
