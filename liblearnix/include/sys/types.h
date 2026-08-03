/*
 * @brief sys/types.h - userspace libc types
 *
 * I don't really care about full compatibility so I
 * assume a 64 bit system.
 */
#pragma once
#include <stdint.h>
#include <stddef.h>

typedef int64_t ssize_t;
typedef int pid_t;
typedef uint64_t off_t;
