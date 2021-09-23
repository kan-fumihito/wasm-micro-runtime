/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _WASM_MEMORY_H
#define _WASM_MEMORY_H

#include "bh_common.h"
#include "../include/wasm_export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum Memory_Mode {
    MEMORY_MODE_UNKNOWN = 0,
    MEMORY_MODE_POOL,
    MEMORY_MODE_ALLOCATOR
} Memory_Mode;

void *
wasm_memory_get_heap_addr(void);

unsigned int
wasm_memory_get_heap_size(void);

void
wasm_memory_set_mode(Memory_Mode mode);
void
wasm_memory_set_heap_addr(void *addr);
void
wasm_memory_set_heap_size(unsigned int size);


bool
wasm_runtime_memory_init(mem_alloc_type_t mem_alloc_type,
                         const MemAllocOption *alloc_option);

void
wasm_runtime_memory_destroy();

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_MEMORY_H */

