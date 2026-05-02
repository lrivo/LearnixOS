/*
 * kheap.h defines a generic kernel heap interface that
 * different backend allocators can implement.
 */
#pragma once
#include <learnix/types.h>

/* Initializes the kernel heap, must be called after the PMM
   is operational.  */
void kmalloc_init (vaddr_t heap_start, size_t heap_size);

void *kmalloc (size_t size);
void *kzalloc (size_t size);
void *krealloc (void *ptr, size_t new_size);
void kfree (void *ptr);
