#include "kernel.h"

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr   idt_ptr;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

// Declared in keyboard.c
extern void keyboard_handler();

// Naked IRQ1 wrapper — saves regs, calls C handler, restores, iret
void irq1_wrapper();
__asm__ (
    ".global irq1_wrapper\n"
    "irq1_wrapper:\n"
    "  pusha\n"
    "  call keyboard_handler\n"
    "  popa\n"
    "  iret\n"
);

void idt_init() {
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    memset(idt, 0, sizeof(idt));

    // IRQ1 → INT 0x21 (after PIC remap: IRQ0=0x20, IRQ1=0x21)
    idt_set_gate(0x21, (uint32_t)irq1_wrapper, 0x08, 0x8E);

    __asm__ volatile ("lidt %0" :: "m"(idt_ptr));

    // Enable interrupts
    __asm__ volatile ("sti");
}
