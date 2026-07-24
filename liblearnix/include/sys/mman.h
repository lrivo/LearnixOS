#pragma once
#include <sys/types.h>

#define PROT_NONE (1 << 0U)
#define PROT_READ (1 << 1U)
#define PROT_WRITE (1 << 2U)
#define PROT_EXEC (1 << 3U)
#define MAP_PRIVATE 0
#define MAP_SHARED 1

void* mmap(void *addr, size_t length, int prot, int flags);
int munmap(void *addr, size_t length);
