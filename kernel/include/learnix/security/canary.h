#pragma once
#include <learnix/types.h>

/* Generates a random 64 bit stack canary and writes it at the given
 * virtual address. */
void canary_generate(vaddr_t at);
