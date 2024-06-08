#ifndef SVC_H
#define SVC_H

#include <stdint.h>

typedef void (*ThreadFunc)(void *);

int32_t svcCreateThread(uint32_t* thread, ThreadFunc entrypoint, uint32_t arg, uint32_t* stack_top, int32_t thread_priority, int32_t processor_id);
void svcExitThread(void) __attribute__((noreturn));
int32_t svcCreateMemoryBlock(uint32_t* memblock, uint32_t addr, uint32_t size, uint8_t my_perm, uint8_t other_perm);
void svcBreak(uint32_t breakReason);

SVC_H
#endif // LUA_HOOK_H
