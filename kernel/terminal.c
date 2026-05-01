#include "kernel.h"

static int cursor_x = 0;
static int cursor_y = 0;

// Hardware cursor via VGA ports
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

static void update_hw_cursor() {
    uint16_t pos = cursor_y * VIDEO_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void scroll() {
    if (cursor_y >= VIDEO_HEIGHT) {
        for (int i = 0; i < (VIDEO_HEIGHT - 1) * VIDEO_WIDTH; i++) {
            VIDEO_MEM[i] = VIDEO_MEM[i + VIDEO_WIDTH];
        }
        for (int i = (VIDEO_HEIGHT - 1) * VIDEO_WIDTH; i < VIDEO_HEIGHT * VIDEO_WIDTH; i++) {
            VIDEO_MEM[i] = 0x0700;
        }
        cursor_y = VIDEO_HEIGHT - 1;
    }
}

void clear_screen() {
    for (int i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; i++) {
        VIDEO_MEM[i] = 0x0700;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_hw_cursor();
}

void print_char(char c, uint8_t color) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        scroll();
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VIDEO_WIDTH - 1;
        }
        VIDEO_MEM[cursor_y * VIDEO_WIDTH + cursor_x] = 0x0700;
    } else {
        VIDEO_MEM[cursor_y * VIDEO_WIDTH + cursor_x] = ((uint16_t)color << 8) | (uint8_t)c;
        cursor_x++;
        if (cursor_x >= VIDEO_WIDTH) {
            cursor_x = 0;
            cursor_y++;
            scroll();
        }
    }
    update_hw_cursor();
}

void print_string(const char* str, uint8_t color) {
    while (*str) print_char(*str++, color);
}

void print_hex(uint32_t value, uint8_t color) {
    const char hex[] = "0123456789ABCDEF";
    char buf[9];
    int i = 8;
    buf[i] = '\0';
    if (value == 0) {
        buf[--i] = '0';
    } else {
        while (value > 0) {
            buf[--i] = hex[value & 0xF];
            value >>= 4;
        }
    }
    print_string(&buf[i], color);
}

void print_prompt() {
    print_string("$ ", 0x0A); // green prompt
}

// Read a line from keyboard with echo, backspace support
// Returns when Enter is pressed; buf is null-terminated (newline stripped)
void readline(char* buf, int max_len) {
    int pos = 0;
    while (1) {
        char c = keyboard_getchar();

        if (c == '\n' || c == '\r') {
            buf[pos] = '\0';
            print_char('\n', 0x07);
            return;
        }

        if (c == '\b') {
            if (pos > 0) {
                pos--;
                print_char('\b', 0x07);
            }
            continue;
        }

        // Printable ASCII only
        if (c >= 0x20 && c < 0x7F && pos < max_len - 1) {
            buf[pos++] = c;
            print_char(c, 0x0F); // white input text
        }
    }
}
