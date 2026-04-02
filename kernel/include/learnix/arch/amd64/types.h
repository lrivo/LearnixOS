/*
 * This header defines architecture dependent C types.
 * learnix/types.h will include this file on amd64 machines.
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Memory types */
typedef uint64_t physaddr_t; // physical address
typedef uint64_t vaddr_t;    // virtual address
