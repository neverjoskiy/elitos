#include "kernel.h"

// PS/2 keyboard ports
#define KEYBOARD_DATA_PORT  0x60
#define KEYBOARD_STATUS_PORT 0x64

// PIC ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define PIC_EOI      0x20

// Input buffer
#define INPUT_BUF_SIZE 256
static char input_buf[INPUT_BUF_SIZE];
static volatile int buf_head = 0;
static volatile int buf_tail = 0;

// US QWERTY scancode set 1 → ASCII (lowercase)
static const char scancode_map[128] = {
    0,   0,  '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0,  ' '
};

static const char scancode_map_shift[128] = {
    0,   0,  '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,   'A','S','D','F','G','H','J','K','L',':','"','~',
    0,  '|','Z','X','C','V','B','N','M','<','>','?', 0,
    '*', 0,  ' '
};

static int shift_pressed = 0;
static int caps_lock = 0;

// I/O port helpers
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

// Remap PIC so IRQs don't conflict with CPU exceptions
void pic_init() {
    // ICW1: start init
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    // ICW2: vector offsets — IRQ0-7 → INT 0x20-0x27, IRQ8-15 → INT 0x28-0x2F
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);
    // ICW3: cascade
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    // ICW4: 8086 mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    // Mask all except IRQ1 (keyboard)
    outb(PIC1_DATA, 0xFD); // 1111 1101 — only IRQ1 unmasked
    outb(PIC2_DATA, 0xFF);
}

static void pic_send_eoi() {
    outb(PIC1_COMMAND, PIC_EOI);
}

// Called from IRQ1 handler in idt.c
void keyboard_handler() {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    // Key release (bit 7 set)
    if (scancode & 0x80) {
        uint8_t key = scancode & 0x7F;
        if (key == 0x2A || key == 0x36) shift_pressed = 0; // L/R shift
        pic_send_eoi();
        return;
    }

    // Shift keys
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        pic_send_eoi();
        return;
    }

    // Caps Lock toggle
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        pic_send_eoi();
        return;
    }

    if (scancode >= 128) {
        pic_send_eoi();
        return;
    }

    char c = 0;
    if (shift_pressed) {
        c = scancode_map_shift[scancode];
    } else {
        c = scancode_map[scancode];
        // Apply caps lock to letters only
        if (caps_lock && c >= 'a' && c <= 'z') c -= 32;
    }

    if (c == 0) {
        pic_send_eoi();
        return;
    }

    // Put into ring buffer
    int next = (buf_head + 1) % INPUT_BUF_SIZE;
    if (next != buf_tail) {
        input_buf[buf_head] = c;
        buf_head = next;
    }

    pic_send_eoi();
}

// Returns 1 if a char is available
int keyboard_has_char() {
    return buf_head != buf_tail;
}

// Blocking read — spins until a char arrives
char keyboard_getchar() {
    while (!keyboard_has_char()) {
        __asm__ volatile ("hlt");
    }
    char c = input_buf[buf_tail];
    buf_tail = (buf_tail + 1) % INPUT_BUF_SIZE;
    return c;
}

void keyboard_init() {
    pic_init();
    // Flush any pending keystroke
    while (inb(KEYBOARD_STATUS_PORT) & 0x01) {
        inb(KEYBOARD_DATA_PORT);
    }
}
