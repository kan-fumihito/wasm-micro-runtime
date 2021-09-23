
/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "wasm_export.h"
#include "bh_read_file.h"
#include "bh_getopt.h"

#define BUF_SIZE 512 * 1024 + 0x1000

int
intToStr(int x, char *str, int str_len, int digit);
int
get_pow(int x, int y);
int32_t
calculate_native(int32_t n, int32_t func1, int32_t func2);

void
print_usage(void)
{
    fprintf(stdout, "Options:\r\n");
    fprintf(stdout, "  -f [path of wasm file] \n");
}

int
main(int argc, char *argv_main[])
{
    char *src, *src_aligned, *dst, *dst_aligned, *p, *global_heap_buf;
    char file_name[32] = "mem.img";
    size_t global_heap_size;
    size_t malloc_size;
    size_t free_size;
    FILE *fp;
    

    RuntimeInitArgs init_args;
    memset(&init_args, 0, sizeof(RuntimeInitArgs));

    if ((fp = fopen(file_name, "rb")) == NULL) {
        printf("file open error\n");
        return 0;
    }
    fread(&dst, sizeof(void *), 1, fp);
    fread(&global_heap_size, sizeof(size_t), 1, fp);
    fread(&malloc_size, sizeof(size_t), 1, fp);
    fread(&free_size, sizeof(size_t), 1, fp);

    src = (char *)calloc(global_heap_size + 0x2000, sizeof(char));
    src_aligned = (char *)(((unsigned long)src + 0xfff) & 0xfffffffff000);
    dst_aligned = (char *)((unsigned long)dst & 0xfffffffff000);
    p = mremap((void *)src_aligned, global_heap_size + 0x1000,
               global_heap_size + 0x1000, MREMAP_MAYMOVE | MREMAP_FIXED,
               dst_aligned);

    if (p == MAP_FAILED) {
        printf("failed allocating global_heap_buf, errno: %d\n", errno);
        return 0;
    }
    global_heap_buf = dst;
    fclose(fp);

    fp = fopen("static.img", "rb");
    fread(global_heap_buf, sizeof(char), global_heap_size, fp);
    fclose(fp);

    // Define an array of NativeSymbol for the APIs to be exported.
    // Note: the array must be static defined since runtime
    //            will keep it after registration
    // For the function signature specifications, goto the link:
    // https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md

    NativeSymbol native_symbols[] = {
        {
          "intToStr", // the name of WASM function name
          intToStr,   // the native function pointer
          "(i*~i)i",  // the function prototype signature, avoid to use i32
          NULL        // attachment is NULL
        },
        {
          "get_pow", // the name of WASM function name
          get_pow,   // the native function pointer
          "(ii)i",   // the function prototype signature, avoid to use i32
          NULL       // attachment is NULL
        },
        { "calculate_native", calculate_native, "(iii)i", NULL }
    };

    init_args.mem_alloc_type = Alloc_With_Pool;
    init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
    init_args.mem_alloc_option.pool.heap_size = global_heap_size;

    // Native symbols need below registration phase
    init_args.n_native_symbols = sizeof(native_symbols) / sizeof(NativeSymbol);
    init_args.native_module_name = "env";
    init_args.native_symbols = native_symbols;

    wasm_runtime_restore_init(&init_args, malloc_size, free_size);

    uint32 *argv;
    printf("calling wasm_runtime_restore\n");
    if(!wasm_runtime_restore(&argv)){
        printf("restore error\n");
        return;
    }

    float ret_val = *(float *)argv;
    printf("Native finished calling wasm function generate_float(), returned "
           "a float value: %ff\n",
           ret_val);

    /*
    buffer = bh_read_file_to_buffer(wasm_path, &buf_size);

    if (!buffer) {
        printf("Open wasm app file [%s] failed.\n", wasm_path);
        goto fail;
    }

    module = wasm_runtime_load(buffer, buf_size, error_buf, sizeof(error_buf));
    if (!module) {
        printf("Load wasm module failed. error: %s\n", error_buf);
        goto fail;
    }

    module_inst = wasm_runtime_instantiate(module, stack_size, heap_size,
                                           error_buf, sizeof(error_buf));

    if (!module_inst) {
        printf("Instantiate wasm module failed. error: %s\n", error_buf);
        goto fail;
    }

    exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);
    if (!exec_env) {
        printf("Create wasm execution environment failed.\n");
        goto fail;
    }

    uint32 argv[4];
    double arg_d = 0.000101;
    argv[0] = 10;
    // the second arg will occupy two array elements
    memcpy(&argv[1], &arg_d, sizeof(arg_d));
    *(float *)(argv + 3) = 300.002;

    if (!(func = wasm_runtime_lookup_function(module_inst, "generate_float",
                                              NULL))) {
        printf("The generate_float wasm function is not found.\n");
        goto fail;
    }

    // pass 4 elements for function arguments
    if (!wasm_runtime_call_wasm(exec_env, func, 4, argv)) {
        printf("call wasm function generate_float failed. %s\n",
               wasm_runtime_get_exception(module_inst));
        goto fail;
    }

    //printf("migr\n");
    //return 0;

    float ret_val = *(float *)argv;
    printf("Native finished calling wasm function generate_float(), returned "
           "a float value: %ff\n",
           ret_val);

    // Next we will pass a buffer to the WASM function
    uint32 argv2[4];

    // must allocate buffer from wasm instance memory space (never use pointer from host runtime)
    wasm_buffer =
      wasm_runtime_module_malloc(module_inst, 100, (void **)&native_buffer);

    *(float *)argv2 = ret_val; // the first argument
    argv2[1] = wasm_buffer; // the second argument is the wasm buffer address
    argv2[2] = 100;         //  the third argument is the wasm buffer size
    argv2[3] =
      3; //  the last argument is the digits after decimal point for converting float to string

    if (!(func2 = wasm_runtime_lookup_function(module_inst, "float_to_string",
                                               NULL))) {
        printf(
          "The wasm function float_to_string wasm function is not found.\n");
        goto fail;
    }

    if (wasm_runtime_call_wasm(exec_env, func2, 4, argv2)) {
        printf("Native finished calling wasm function: float_to_string, "
               "returned a formatted string: %s\n",
               native_buffer);
    }
    else {
        printf("call wasm function float_to_string failed. error: %s\n",
               wasm_runtime_get_exception(module_inst));
        goto fail;
    }

    wasm_function_inst_t func3 =
      wasm_runtime_lookup_function(module_inst, "calculate", NULL);
    if (!func3) {
        printf("The wasm function calculate is not found.\n");
        goto fail;
    }

    uint32_t argv3[1] = { 3 };
    if (wasm_runtime_call_wasm(exec_env, func3, 1, argv3)) {
        uint32_t result = *(uint32_t *)argv3;
        printf(
          "Native finished calling wasm function: calculate, return: %d\n",
          result);
    }
    else {
        printf("call wasm function calculate failed. error: %s\n",
               wasm_runtime_get_exception(module_inst));
        goto fail;
    }

fail:
    if (exec_env)
        wasm_runtime_destroy_exec_env(exec_env);
    if (module_inst) {
        if (wasm_buffer)
            wasm_runtime_module_free(module_inst, wasm_buffer);
        wasm_runtime_deinstantiate(module_inst);
    }
    if (module)
        wasm_runtime_unload(module);
    if (buffer)
        BH_FREE(buffer);
    */
    //wasm_runtime_destroy();

    return 0;
}
