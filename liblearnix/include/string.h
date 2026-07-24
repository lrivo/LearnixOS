#pragma once
#include <sys/types.h>

int strcmp(const char *s1, const char *s2);

void *memcpy (void *restrict dest, const void *restrict src, size_t n);
void *memset (void *s, int c, size_t n);
