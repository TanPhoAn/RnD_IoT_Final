// src/custom_sbrk.c
#include <stdint.h>

extern char _end; // Symbol that marks the end of the data segment

void *_sbrk(intptr_t increment) {
    static char *heap_end = &_end;
    char *prev_heap_end = heap_end;
    heap_end += increment;
    return (void *)prev_heap_end;
}
