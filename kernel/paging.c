#include "kernel.h"

#define PAGE_DIR_ADDR 0x100000
#define PAGE_TABLE_ADDR 0x101000

static uint32_t* page_dir = (uint32_t*)PAGE_DIR_ADDR;
static uint32_t* page_table = (uint32_t*)PAGE_TABLE_ADDR;

void page_table_init() {
    // Map first 4MB of memory (kernel space)
    for (int i = 0; i < 1024; i++) {
        page_table[i] = (i * 0x1000) | 3;  // Present + writable
    }
}

void page_dir_init() {
    // Point page directory to page table
    page_dir[0] = PAGE_TABLE_ADDR | 3;  // Present + writable
}

void paging_init() {
    page_table_init();
    page_dir_init();
    
    // Load page directory
    __asm__ volatile ("movl %0, %%cr3" :: "r" (PAGE_DIR_ADDR));
    
    // Enable paging (CR0 bit 31)
    uint32_t cr0;
    __asm__ volatile ("movl %%cr0, %0" : "=r" (cr0));
    cr0 |= 0x80000000;
    __asm__ volatile ("movl %0, %%cr0" :: "r" (cr0));
}
