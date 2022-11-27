#include <kernel/tty.h>
#include <stdio.h>
#include <stdint-gcc.h>
#include <kernel/multiboot.h>
#include <kernel/common.h>
#include <demo/demo.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/memory/heap.h>
#include <stdlib.h>

multiboot_info_t* mbd;
unsigned int grub_checkvalue;
extern uintptr_t kernel_start;
extern uintptr_t kernel_end;

extern "C" void kernel_early(multiboot_info_t * _mbd, unsigned int magic) {
    mbd = _mbd;
    grub_checkvalue = magic;
}

extern "C" void kernel_main(void) {
    terminal_initialize();

    if (grub_checkvalue != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("invalid magic number!");
    }
    if (!(mbd->flags >> 6 & 0x1)) {
        panic("invalid memory map given by GRUB bootloader");
    }

    printf("System memory map:\n");
    // printf("--------------------------------------------------\n");
    uint32_t availRamStart, availRamSize, totalRamSize = 0;
    for (int i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt =
            (multiboot_memory_map_t*)(mbd->mmap_addr + i);

        // printf("Start Addr: %u,%u | Length: %u,%u | Size: %u | Type: %u\n",
        //     mmmt->addr_high, mmmt->addr_low, mmmt->len_high, mmmt->len_low, mmmt->size, mmmt->type);
        totalRamSize += mmmt->len_low;
        if (mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {
            /*
             * Do something with this memory block!
             * BE WARNED that some of memory shown as availiable is actually
             * actively being used by the kernel! You'll need to take that
             * into account before writing to memory!
             */
            if (mmmt->addr_low + mmmt->len_low < (uint32_t)&kernel_end) // skip (< 1Mb areas)
                continue;
            availRamStart = mmmt->addr_low;
            availRamSize = mmmt->len_low;
        }
    }
    printf("--------------------------------------------------\n");
    printf("Kernel memory range: [%u, %u)\n", &kernel_start, &kernel_end);
    printf("Total RAM: [0, %u)\n", totalRamSize);
    printf("Available ram range: [%u, %u)\n", availRamStart, availRamStart + availRamSize);
    printf("--------------------------------------------------\n");

    PMM::get().init(availRamStart + availRamSize, &kernel_end, availRamStart + availRamSize);

    void* testFrame = PMM::get().allocFrame();
    printf("Allocated memory: %u\n", testFrame);

    VMM::get().init();
    void* newPage = VMM::get().allocPage();
    printf("New Page is at: %u\n", newPage);

    MallocHeap::init();

    int* ptr = (int*)malloc(5000);
    *ptr = 5;
    printf("Some data I have at addr: %u is: %d\n", ptr, *ptr);
    free(ptr);
    ptr = new int(10);
    printf("Some data I have at addr: %u is: %d\n", ptr, *ptr);

    int* arr = new int[5];
    for (int i = 0; i < 5; i++) arr[i] = i + 1;
    for (int i = 0; i < 5; i++)
        printf("Array at idx %d is: %d\n", i, arr[i]);

    MallocHeap::get().print();

    panic("Nothing to do");
}
