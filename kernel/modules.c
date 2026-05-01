#include "kernel.h"

#define MAX_MODULES 8
#define MODULE_NAME_LEN 32
#define MODULE_SIZE 4096

typedef struct {
    char name[MODULE_NAME_LEN];
    void* entry;
    int loaded;
} module_t;

static module_t modules[MAX_MODULES];
static int module_count = 0;

void modules_init() {
    for (int i = 0; i < MAX_MODULES; i++) {
        modules[i].name[0] = '\0';
        modules[i].entry = NULL;
        modules[i].loaded = 0;
    }
    module_count = 0;
}

int modules_load(const char* name, void* entry) {
    if (module_count >= MAX_MODULES) return -1;
    
    for (int i = 0; i < module_count; i++) {
        if (modules[i].loaded && strcmp(modules[i].name, name) == 0) {
            return -1;  // Module already loaded
        }
    }
    
    int idx = module_count++;
    for (int i = 0; name[i] != '\0' && i < MODULE_NAME_LEN - 1; i++) {
        modules[idx].name[i] = name[i];
    }
    modules[idx].name[MODULE_NAME_LEN - 1] = '\0';
    modules[idx].entry = entry;
    modules[idx].loaded = 1;
    
    return 0;
}

int modules_unload(const char* name) {
    for (int i = 0; i < module_count; i++) {
        if (modules[i].loaded && strcmp(modules[i].name, name) == 0) {
            modules[i].loaded = 0;
            modules[i].entry = NULL;
            return 0;
        }
    }
    return -1;
}

void* modules_get(const char* name) {
    for (int i = 0; i < module_count; i++) {
        if (modules[i].loaded && strcmp(modules[i].name, name) == 0) {
            return modules[i].entry;
        }
    }
    return NULL;
}

int modules_list() {
    print_string("Loaded modules:\n", 0x07);
    for (int i = 0; i < module_count; i++) {
        if (modules[i].loaded) {
            print_string("  ", 0x07);
            print_string(modules[i].name, 0x07);
            print_string("\n", 0x07);
        }
    }
    return 0;
}
