#ifndef CVM_KERNEL_H
#define CVM_KERNEL_H

#include <stdint.h>

extern int cvm_compile(FILE *output, FILE *input);
extern int cvm_load(uint8_t *memory, int32_t msize);
extern int cvm_run(int32_t **output, int32_t *input);

#endif /* CVM_KERNEL_H */ 
