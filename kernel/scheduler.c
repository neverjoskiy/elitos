#include "kernel.h"

#define MAX_TASKS 4
#define STACK_SIZE 4096

typedef struct {
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp;
    uint32_t eip, cs, eflags;
} context_t;

typedef struct {
    context_t context;
    uint8_t stack[STACK_SIZE];
    int active;
} task_t;

static task_t tasks[MAX_TASKS];
static int current_task = 0;
static int task_count = 0;

void scheduler_init() {
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].active = 0;
    }
    task_count = 0;
}

void scheduler_add_task(void* entry_point) {
    if (task_count >= MAX_TASKS) return;
    
    int task_id = task_count++;
    task_t* task = &tasks[task_id];
    
    // Initialize context
    task->context.eip = (uint32_t)entry_point;
    task->context.eflags = 0x200;  // Interrupt flag set
    task->context.cs = 0x08;
    
    // Set up stack
    uint32_t* stack_ptr = (uint32_t*)(task->stack + STACK_SIZE);
    stack_ptr[-1] = 0x08;  // CS
    stack_ptr[-2] = (uint32_t)entry_point;  // EIP
    task->context.esp = (uint32_t)stack_ptr - 8;
    task->context.ebp = task->context.esp;
    
    task->active = 1;
}

void scheduler_start() {
    // Enable interrupts
    __asm__ volatile ("sti");
    
    // Infinite loop - scheduler will run via timer interrupts
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void scheduler_switch() {
    // Save current context
    __asm__ volatile (
        "pusha\n\t"
        "movl %%esp, %0"
        : "=m" (tasks[current_task].context.esp)
        :
        : "memory"
    );
    
    // Switch to next task
    current_task = (current_task + 1) % task_count;
    while (!tasks[current_task].active) {
        current_task = (current_task + 1) % task_count;
    }
    
    // Restore next task context
    __asm__ volatile (
        "movl %0, %%esp\n\t"
        "popa"
        : 
        : "m" (tasks[current_task].context.esp)
        : "memory"
    );
    
    __asm__ volatile ("iret");
}
