#pragma once
#include <stddef.h>
#include <string.h>

size_t
strlen(const char *s) {
    const char *tmp = s;
    while (*tmp != '\0')
        tmp++;
    return (size_t)tmp - (size_t)s;
}

char*
strcpy(char *restrict dst, const char *restrict src) {
    char *tmp = dst;
    while (*src != '\0') {
        *tmp = *src; src++; tmp++;
    }
    return dst;
}

int
strcmp(const char *s1, const char *s2)
{
  while (*s1 && (*s1 == *s2))
  {
    s1++; s2++;
  }
  return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void *
memcpy (void *restrict dest, const void *restrict src, size_t n)
{
  asm volatile ("rep movsb" : "+D"(dest), "+S"(src), "+c"(n)::"memory");
  return dest;
}

void *
memset (void *s, int c, size_t n)
{
  asm volatile ("rep stosb" ::"D"(s), "a"(c), "c"(n) : "cc", "memory");
  return s;
}
