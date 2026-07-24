/*
 *  x86_64 types, included automatically by learnix/types.h
 */
#pragma once

/* Those are included by the compiler for the
 * target architecture. */
#include <stdbool.h>
#include <stdint.h>

typedef uint64_t size_t;
typedef int64_t ssize_t;

/* Memory */
typedef uint64_t paddr_t; // a physical address
typedef uint64_t vaddr_t; // a virtual address

#define NULL (void*)0
