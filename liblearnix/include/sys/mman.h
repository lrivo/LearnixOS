#pragma once
#include <sys/types.h>
#include <learnix/mman.h>

void* mmap(void *addr, size_t length, int prot, int flags);
int munmap(void *addr, size_t length);
