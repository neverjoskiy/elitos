#include "kernel.h"

void* memset(void* dest, int val, size_t n) {
    unsigned char* p = (unsigned char*)dest;
    while (n--) *p++ = (unsigned char)val;
    return dest;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) *d++ = *s++;
    return dest;
}
