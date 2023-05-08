#include <kernel/memory/kheap.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void* malloc(uint64_t size) {
#if defined(__is_libk)
    return kmalloc(size);
#else
    printf("Unimplemented malloc in libc\n");
#endif
    while (1) {
    }
    __builtin_unreachable();
}
