#ifndef CVM_KERNEL_H
#define CVM_KERNEL_H

#include <stdio.h>
#include <stdint.h>

// Comment this line if you are need use only main inctructions.
#define CVM_KERNEL_IAPPEND

// Memory settings.
#define CVM_KERNEL_SMEMORY (1 << 10) // Stack   = 1024 INT32
#define CVM_KERNEL_CMEMORY (4 << 10) // Code    = 4096 BYTE

// Interface functions.
extern int cvm_compile(FILE *output, FILE *input);
extern int cvm_load(uint8_t *memory, int32_t msize);
extern int cvm_run(int32_t **output, int32_t *input);

#endif /* CVM_KERNEL_H */ 
