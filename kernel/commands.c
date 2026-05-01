#include "kernel.h"

#define MAX_ARGS 16
#define MAX_ARG_LEN 64

static char* args[MAX_ARGS];
static int arg_count = 0;

static void parse_command(char* cmd) {
    arg_count = 0;
    int in_arg = 0;
    int arg_pos = 0;
    
    for (int i = 0; cmd[i] != '\0'; i++) {
        if (cmd[i] == ' ' || cmd[i] == '\t') {
            if (in_arg) {
                args[arg_count][arg_pos] = '\0';
                arg_count++;
                in_arg = 0;
                arg_pos = 0;
            }
        } else {
            if (!in_arg) {
                args[arg_count] = &cmd[i];
                in_arg = 1;
            }
            if (arg_pos < MAX_ARG_LEN - 1) {
                args[arg_count][arg_pos++] = cmd[i];
            }
        }
    }
    if (in_arg) {
        args[arg_count][arg_pos] = '\0';
        arg_count++;
    }
}

static void cmd_echo() {
    if (arg_count > 1) {
        for (int i = 1; i < arg_count; i++) {
            print_string(args[i], 0x07);
            if (i < arg_count - 1) print_string(" ", 0x07);
        }
    }
    print_string("\n", 0x07);
}

static void cmd_ls() {
    extern int file_count;
    extern char file_names[][64];
    extern int file_sizes[];
    extern char file_data[][4096];
    
    for (int i = 0; i < file_count; i++) {
        if (file_names[i][0] != '\0') {
            print_string(file_names[i], 0x07);
            print_string("\n", 0x07);
        }
    }
}

static void cmd_cat() {
    extern int file_count;
    extern char file_names[][64];
    extern int file_sizes[];
    extern char file_data[][4096];
    
    if (arg_count < 2) {
        print_string("Usage: cat <filename>\n", 0x0C);
        return;
    }
    
    for (int i = 0; i < file_count; i++) {
        if (file_names[i][0] != '\0') {
            int match = 1;
            for (int j = 0; args[1][j] != '\0' || file_names[i][j] != '\0'; j++) {
                if (args[1][j] != file_names[i][j]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                int copy_size = 256 < file_sizes[i] ? 256 : file_sizes[i];
                char buffer[256];
                for (int j = 0; j < copy_size; j++) {
                    buffer[j] = file_data[i][j];
                }
                buffer[copy_size] = '\0';
                print_string(buffer, 0x07);
                print_string("\n", 0x07);
                return;
            }
        }
    }
    print_string("File not found\n", 0x0C);
}

static void cmd_help() {
    print_string("Available commands:\n", 0x07);
    print_string("  echo <text> - Print text\n", 0x07);
    print_string("  ls          - List files\n", 0x07);
    print_string("  cat <file>  - Read file\n", 0x07);
    print_string("  help        - Show this help\n", 0x07);
}

void process_command(char* cmd) {
    if (cmd[0] == '\0') return;
    
    parse_command(cmd);
    
    if (arg_count == 0) return;
    
    if (strcmp(args[0], "echo") == 0) {
        cmd_echo();
    } else if (strcmp(args[0], "ls") == 0) {
        cmd_ls();
    } else if (strcmp(args[0], "cat") == 0) {
        cmd_cat();
    } else if (strcmp(args[0], "help") == 0) {
        cmd_help();
    } else {
        print_string("Unknown command: ", 0x0C);
        print_string(args[0], 0x0C);
        print_string("\n", 0x07);
    }
}

// strcmp implementation
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}
