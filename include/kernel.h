#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

// Video memory
#define VIDEO_MEM    ((uint16_t*)0xB8000)
#define VIDEO_WIDTH  80
#define VIDEO_HEIGHT 25

// Terminal / screen
void clear_screen();
void print_char(char c, uint8_t color);
void print_string(const char* str, uint8_t color);
void print_hex(uint32_t value, uint8_t color);
void print_prompt();
void readline(char* buf, int max_len);

// Memory
void* memset(void* dest, int val, size_t n);
void* memcpy(void* dest, const void* src, size_t n);

// GDT
void gdt_init();

// IDT
void idt_init();
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

// Paging
void paging_init();

// Keyboard
void keyboard_init();
int  keyboard_has_char();
char keyboard_getchar();

// Scheduler (kept for future use)
void scheduler_init();
void scheduler_start();
void scheduler_add_task(void* entry_point);
void scheduler_switch();

// VFS
void vfs_init();
int  vfs_create_file(const char* name, const char* data);
int  vfs_read_file(const char* name, char* buffer, int size);
int  vfs_delete_file(const char* name);

// DB
void  db_init();
void  db_create_table(const char* name);
int   db_add_column(const char* table_name, const char* col_name, int type);
int   db_create_index(const char* table_name, const char* col_name);
int   db_insert(const char* table_name, const char* data);
char* db_select(const char* table_name, const char* key);
int   db_begin();
int   db_commit();
int   db_rollback();

// Modules
void  modules_init();
int   modules_load(const char* name, void* entry);
int   modules_unload(const char* name);
void* modules_get(const char* name);
int   modules_list();

// Commands
void process_command(char* cmd);
int  strcmp(const char* s1, const char* s2);

#endif
