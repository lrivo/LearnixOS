#pragma once

#define kassert(cond)                                                          \
  do                                                                           \
  {                                                                            \
    if (!(cond))                                                               \
      kpanic ("assertion failed: %s at %s:%d\n", #cond, __FILE__, __LINE__);   \
  } while (0)

#define kassert_msg(cond, fmt, ...)                                            \
  do                                                                           \
  {                                                                            \
    if (!(cond))                                                               \
      kpanic ("assertion failed: %s at %s:%d \n" fmt "\n", #cond, __FILE__,    \
              __LINE__, ##__VA_ARGS__);                                        \
  } while (0)

void kpanic (const char *fmt, ...);
