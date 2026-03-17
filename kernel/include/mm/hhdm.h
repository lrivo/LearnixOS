/*
 *  Higher Half Direct Mapping conversion macros
 */
#pragma once

#include <stdint.h>

/* HHDM offset recovered from Limine in kmain(). */
extern uintptr_t hhdm_offset;

#define PA_TO_HHDM(pa) (pa + hhdm_offset)
#define HHDM_TO_PA(va) ((uintptr_t)(void*)va - hhdm_offset)
