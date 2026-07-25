#pragma once
#include <sys/types.h>

size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);

char *strcpy(char *restrict dst, const char *restrict src);

void *memcpy (void *restrict dest, const void *restrict src, size_t n);
void *memset (void *s, int c, size_t n);
