/*
 *  x86_64 types, included automatically by learnix/types.h
 */
#pragma once

/* Those are included by the compiler for the
 * target architecture. */
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Memory */
typedef uint64_t paddr_t; // a physical address
typedef uint64_t vaddr_t; // a virtual address
