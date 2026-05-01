#include "kernel.h"

extern void gdt_init();
extern void idt_init();
extern void paging_init();
extern void vfs_init();
extern void db_init();
extern void modules_init();
extern void keyboard_init();

#define CMD_BUF_SIZE 256

void kernel_main() {
    clear_screen();
    print_string("=== MiniKernel v0.1 ===\n", 0x0B);

    gdt_init();
    print_string("[OK] GDT\n", 0x07);

    idt_init();
    print_string("[OK] IDT + interrupts\n", 0x07);

    paging_init();
    print_string("[OK] Paging\n", 0x07);

    keyboard_init();
    print_string("[OK] Keyboard\n", 0x07);

    vfs_init();
    print_string("[OK] VFS\n", 0x07);

    db_init();
    print_string("[OK] DB\n", 0x07);

    modules_init();
    print_string("[OK] Modules\n", 0x07);

    print_string("\nType 'help' for available commands.\n\n", 0x0E);

    // Shell loop
    char cmd_buf[CMD_BUF_SIZE];
    while (1) {
        print_prompt();
        readline(cmd_buf, CMD_BUF_SIZE);
        process_command(cmd_buf);
    }
}
