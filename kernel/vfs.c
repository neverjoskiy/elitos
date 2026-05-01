#include "kernel.h"

#define MAX_FILES 16
#define MAX_FILE_SIZE 4096
#define MAX_PATH 64

char file_names[MAX_FILES][MAX_PATH];
int file_sizes[MAX_FILES];
char file_data[MAX_FILES][MAX_FILE_SIZE];
int file_count = 0;

void vfs_init() {
    for (int i = 0; i < MAX_FILES; i++) {
        file_names[i][0] = '\0';
        file_sizes[i] = 0;
    }
    file_count = 0;
}

int vfs_create_file(const char* name, const char* data) {
    if (file_count >= MAX_FILES) return -1;
    
    // Check if file already exists
    for (int i = 0; i < file_count; i++) {
        if (file_names[i][0] != '\0') {
            int match = 1;
            for (int j = 0; name[j] != '\0' || file_names[i][j] != '\0'; j++) {
                if (name[j] != file_names[i][j]) {
                    match = 0;
                    break;
                }
            }
            if (match) return -1;  // File exists
        }
    }
    
    int idx = file_count++;
    for (int i = 0; name[i] != '\0' && i < MAX_PATH - 1; i++) {
        file_names[idx][i] = name[i];
    }
    file_names[idx][MAX_PATH - 1] = '\0';
    
    if (data) {
        for (int i = 0; data[i] != '\0' && i < MAX_FILE_SIZE - 1; i++) {
            file_data[idx][i] = data[i];
        }
        file_data[idx][MAX_FILE_SIZE - 1] = '\0';
        file_sizes[idx] = 0;
        for (int i = 0; data[i] != '\0'; i++) file_sizes[idx]++;
    } else {
        file_sizes[idx] = 0;
    }
    
    return 0;
}

int vfs_read_file(const char* name, char* buffer, int size) {
    for (int i = 0; i < file_count; i++) {
        if (file_names[i][0] != '\0') {
            int match = 1;
            for (int j = 0; name[j] != '\0' || file_names[i][j] != '\0'; j++) {
                if (name[j] != file_names[i][j]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                int copy_size = size < file_sizes[i] ? size : file_sizes[i];
                for (int j = 0; j < copy_size; j++) {
                    buffer[j] = file_data[i][j];
                }
                buffer[copy_size] = '\0';
                return copy_size;
            }
        }
    }
    return -1;
}

int vfs_delete_file(const char* name) {
    for (int i = 0; i < file_count; i++) {
        if (file_names[i][0] != '\0') {
            int match = 1;
            for (int j = 0; name[j] != '\0' || file_names[i][j] != '\0'; j++) {
                if (name[j] != file_names[i][j]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                file_names[i][0] = '\0';
                file_sizes[i] = 0;
                return 0;
            }
        }
    }
    return -1;
}
