#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "cvmkernel.h"

#include "extclib/type/hashtab.h"
#include "extclib/type/stack.h"

// RESULT = INSTR.CODE | RETURN.CODE
#define WRAP(x, y) ((x) << 8 | (y))

// Comment this line if you are
// need use only main inctructions.
#define ADDITIONAL_INSTRUCTION

// Num of all instructions.
#ifdef ADDITIONAL_INSTRUCTION
	#define INSTR_SIZE 29
#else
	#define INSTR_SIZE 15
#endif

// 4096 BYTE
#define MEMRY_SIZE (4 << 10)

// 1024 INT32
#define STACK_SIZE (1 << 10)

// 1024 LIST
#define HSTAB_SIZE (1 << 10)

// N - number
// C - char
enum {
	// 0xNN 
	// PSEUDO INSTRUCTIONS (3)
	C_PASS = 0x00, // 0 bytes
	C_CMNT = 0x11, // 0 bytes
	C_LABL = 0x22, // 0 bytes
	// 0xNC 
	// MAIN INSTRUCTIONS (12)
	C_PUSH = 0x0A, // 5 bytes
	C_POP  = 0x0B, // 1 bytes
	C_ADD  = 0x0C, // 1 bytes
	C_SUB  = 0x0D, // 1 bytes
	C_JL   = 0x0E, // 1 bytes
	C_JG   = 0x0F, // 1 bytes
	C_JE   = 0x1A, // 1 bytes
	C_STOR = 0x1B, // 1 bytes
	C_LOAD = 0x1C, // 1 bytes
	C_CALL = 0x1D, // 1 bytes
	C_RET  = 0x1E, // 1 bytes
	C_HLT  = 0x1F, // 1 bytes
#ifdef ADDITIONAL_INSTRUCTION
	// 0xCN 
	// ADD INSTRUCTIONS (14)
	C_MUL  = 0xA0, // 1 bytes
	C_DIV  = 0xB0, // 1 bytes
	C_MOD  = 0xC0, // 1 bytes
	C_SHR  = 0xD0, // 1 bytes
	C_SHL  = 0xE0, // 1 bytes
	C_XOR  = 0xF0, // 1 bytes
	C_AND  = 0xA1, // 1 bytes
	C_OR   = 0xB1, // 1 bytes
	C_NOT  = 0xC1, // 1 bytes
	C_JMP  = 0xD1, // 1 bytes
	C_JNE  = 0xE1, // 1 bytes
	C_JLE  = 0xF1, // 1 bytes
	C_JGE  = 0xA2, // 1 bytes
	C_ALLC = 0xB2, // 1 bytes
#endif
	// 0xCC 
	// NOT USED
};

static struct virtual_machine {
	int32_t mmused;
	uint8_t memory[MEMRY_SIZE];
	struct {
		int  bcode;
		char *mnem;
	} bclist[INSTR_SIZE];
} VM = {
	.mmused = 0,
	.bclist = {
		// PSEUDO INSTRUCTIONS
		{ C_PASS,  ""    }, // 0 arg
		{ C_CMNT, ";"    }, // 0 arg
		{ C_LABL, "labl" }, // 1 arg
		// MAIN INSTRUCTIONS
		{ C_PUSH, "push" }, // 1 arg, 0 stack
		{ C_POP,  "pop"  }, // 0 arg, 1 stack
		{ C_ADD,  "add"  }, // 0 arg, 2 stack
		{ C_SUB,  "sub"  }, // 0 arg, 2 stack
		{ C_JL,   "jl"   }, // 0 arg, 3 stack
		{ C_JG,   "jg"   }, // 0 arg, 3 stack
		{ C_JE,   "je"   }, // 0 arg, 3 stack
		{ C_STOR, "stor" }, // 0 arg, 2 stack
		{ C_LOAD, "load" }, // 0 arg, 1 stack
		{ C_CALL, "call" }, // 0 arg, 1 stack
		{ C_RET,  "ret"  }, // 0 arg, 1 stack
		{ C_HLT,  "hlt"  }, // 0 arg, 0 stack
#ifdef ADDITIONAL_INSTRUCTION
		// ADD INSTRUCTIONS
		{ C_MUL,  "mul"  }, // 0 arg, 2 stack
		{ C_DIV,  "div"  }, // 0 arg, 2 stack
		{ C_MOD,  "mod"  }, // 0 arg, 2 stack
		{ C_SHR,  "shr"  }, // 0 arg, 2 stack
		{ C_SHL,  "shl"  }, // 0 arg, 2 stack
		{ C_XOR,  "xor"  }, // 0 arg, 2 stack
		{ C_AND,  "and"  }, // 0 arg, 2 stack
		{ C_OR,   "or"   }, // 0 arg, 2 stack
		{ C_NOT,  "not"  }, // 0 arg, 1 stack
		{ C_JMP,  "jmp"  }, // 0 arg, 1 stack
		{ C_JNE,  "jne"  }, // 0 arg, 3 stack
		{ C_JLE,  "jle"  }, // 0 arg, 3 stack
		{ C_JGE,  "jge"  }, // 0 arg, 3 stack
		{ C_ALLC, "allc" }, // 0 arg, 1 stack
#endif
	},
};

static char *readcode(char *line, uint8_t *opcode);

static uint32_t join_8bits_to_32bits(uint8_t *bytes);
static void split_32bits_to_8bits(uint32_t num, uint8_t *bytes);

static char *strtolower(char *str);
static int strnull(char *str);

// pegjs
extern int cvm_run(int32_t **output, int32_t *input) {
	stack_t *stack;
	uint8_t opcode;
	int32_t mi;

	stack = stack_new(STACK_SIZE, sizeof(int32_t));
	for (int i = 0; i < input[0]; ++i) {
		stack_push(stack, &input[i+1]);
	}

	mi = 0;
	while(mi < VM.mmused) {
		opcode = VM.memory[mi++];
		switch(opcode) {
			case C_PUSH: {
				int32_t num;
				uint8_t bytes[4];
				if (stack_size(stack) == STACK_SIZE) {
					return WRAP(opcode, 1);
				}
				memcpy(bytes, VM.memory + mi, 4); mi += 4;
				num = (int32_t)join_8bits_to_32bits(bytes);
				stack_push(stack, &num);
			}
			break;
			case C_POP: {
				if (stack_size(stack) == 0) {
					return WRAP(opcode, 1);
				}
				stack_pop(stack);
			}
			break;
		#ifdef ADDITIONAL_INSTRUCTION
			case C_NOT: {
				int32_t x;
				x = ~*(int32_t*)stack_pop(stack);
				stack_push(stack, &x);
			}
			break;
		#endif
		#ifdef ADDITIONAL_INSTRUCTION
			case C_MUL: case C_DIV:
			case C_MOD: case C_AND: 
			case C_OR:  case C_XOR:
			case C_SHR: case C_SHL: 
		#endif
			case C_ADD: case C_SUB: {
				int32_t x, y;
				if (stack_size(stack) < 2) {
					return WRAP(opcode, 1);
				}
				x = *(int32_t*)stack_pop(stack);
				y = *(int32_t*)stack_pop(stack);
				switch(opcode) {
				#ifdef ADDITIONAL_INSTRUCTION
					case C_MUL:
						y *= x;
					break;
					case C_DIV:
						y /= x;
					break;
					case C_MOD:
						y %= x;
					break;
					case C_AND:
						y &= x;
					break;
					case C_OR:
						y |= x;
					break;
					case C_XOR:
						y ^= x;
					break;
					case C_SHR:
						y >>= x;
					break;
					case C_SHL:
						y <<= x;
					break;
				#endif
					case C_ADD:
						y += x;
					break;
					case C_SUB:
						y -= x;
					break;
					default: ;
				}
				stack_push(stack, &y);
			}
			break;
			case C_STOR: {
				int32_t num1, num2;
				if (stack_size(stack) < 2) {
					return WRAP(opcode, 1);
				}
				num1 = *(int32_t*)stack_pop(stack);
				num2 = *(int32_t*)stack_pop(stack);
				if (num1 < 0) {
					num1 = stack_size(stack) + num1;
					if (num1 < 0) {
						return WRAP(opcode, 2);
					}
				} else {
					if (num1 >= stack_size(stack)) {
						return WRAP(opcode, 3);
					}
				}
				if (num2 < 0) {
					num2 = stack_size(stack) + num2;
					if (num2 < 0) {
						return WRAP(opcode, 4);
					}
				} else {
					if (num2 >= stack_size(stack)) {
						return WRAP(opcode, 5);
					}
				}
				num2 = *(int32_t*)stack_get(stack, num2);
				stack_set(stack, num1, &num2);
			}
			break;
			case C_LOAD: {
				int32_t num;
				if (stack_size(stack) == 0) {
					return WRAP(opcode, 1);
				}
				num = *(int32_t*)stack_pop(stack);
				if (num < 0) {
					num = stack_size(stack) + num;
					if (num < 0) {
						return WRAP(opcode, 2);
					}
				} else {
					if (num >= stack_size(stack)) {
						return WRAP(opcode, 3);
					}
				}
				num = *(int32_t*)stack_get(stack, num);
				stack_push(stack, &num);
			}
			break;
		#ifdef ADDITIONAL_INSTRUCTION
			case C_JMP: {
				int32_t num;
				if (stack_size(stack) == 0) {
					return WRAP(opcode, 1);
				}
				num = *(int32_t*)stack_pop(stack);
				if (num < 0) {
					num = stack_size(stack) + num;
					if (num < 0) {
						return WRAP(opcode, 2);
					}
					num = *(int32_t*)stack_get(stack, num);
				}
				if (num < 0 || num >= VM.mmused) {
					return WRAP(opcode, 3);
				}
				mi = num;
			}
			break;
		#endif
		#ifdef ADDITIONAL_INSTRUCTION
			case C_JGE: case C_JLE: case C_JNE: 
		#endif 
			case C_JL: case C_JG: case C_JE: {
				int32_t num, x, y;
				if (stack_size(stack) < 3) {
					return WRAP(opcode, 1);
				}
				num = *(int32_t*)stack_pop(stack);
				if (num < 0) {
					num = stack_size(stack) + num;
					if (num < 0) {
						return WRAP(opcode, 2);
					}
					num = *(int32_t*)stack_get(stack, num);
				}
				if (num < 0 || num >= VM.mmused) {
					return WRAP(opcode, 3);
				}
				x = *(int32_t*)stack_pop(stack);
				y = *(int32_t*)stack_pop(stack);
				switch(opcode) {
				#ifdef ADDITIONAL_INSTRUCTION
					case C_JNE:
						if (y != x) {
							mi = num;
						}
					break;
					case C_JLE:
						if (y <= x) {
							mi = num;
						}
					break;
					case C_JGE:
						if (y >= x) {
							mi = num;
						}
					break;
				#endif
					case C_JL:
						if (y < x) {
							mi = num;
						}
					break;
					case C_JG:
						if (y > x) {
							mi = num;
						}
					break;
					case C_JE:
						if (y == x) {
							mi = num;
						}
					break;
					default: ;
				}
			}
			break;
		#ifdef ADDITIONAL_INSTRUCTION
			case C_ALLC: {
				int32_t num, null;
				if (stack_size(stack) == 0) {
					return WRAP(opcode, 1);
				}
				num = *(int32_t*)stack_pop(stack);
				if (num < 0) {
					return WRAP(opcode, 2);
				}
				if (stack_size(stack)+num >= STACK_SIZE) {
					return WRAP(opcode, 3);
				}
				null = 0;
				for (int i = 0; i < num; ++i) {
					stack_push(stack, &null);
				}
			}
			break;
		#endif
			case C_CALL: {
				int32_t num;
				if (stack_size(stack) == 0) {
					return WRAP(opcode, 1);
				}
				num = *(int32_t*)stack_pop(stack);
				if (num < 0) {
					num = stack_size(stack) + num;
					if (num < 0) {
						return WRAP(opcode, 2);
					}
					num = *(int32_t*)stack_get(stack, num);
				}
				if (num < 0 || num >= VM.mmused) {
					return WRAP(opcode, 3);
				}
				stack_push(stack, &mi);
				mi = num;
			}
			break;
			case C_RET: {
				int32_t num;
				if (stack_size(stack) == 0) {
					return WRAP(opcode, 1);
				}
				num = *(int32_t*)stack_pop(stack);
				mi = num;
			}
			break;
			case C_HLT: {
				goto close;
			}
			break;
			default: ;
		}
	}

close:
	mi = stack_size(stack);
	*output = (int32_t*)malloc(sizeof(int32_t)*(mi+1));
	(*output)[0] = mi;
	for (int i = 0; i < mi; ++i) {
		(*output)[i+1] = *(int32_t*)stack_pop(stack);
	}
	stack_free(stack);
	return WRAP(0x00, 0);
}

extern int cvm_load(uint8_t *memory, int32_t msize) {
	if (msize < 0 || msize >= MEMRY_SIZE) {
		return 1;
	}
	memcpy(VM.memory, memory, msize);
	VM.mmused = msize;
	return 0;
}

extern int cvm_compile(FILE *output, FILE *input) {
	hashtab_t *hashtab;
	int line_index, byte_index, err_exist;
	char buffer[BUFSIZ];
	char *arg;
	uint8_t opcode;

	hashtab = hashtab_new(HSTAB_SIZE);
	line_index = 0;
	byte_index = 0;
	err_exist  = 0;

	while(fgets(buffer, BUFSIZ, input) != NULL) {
		++line_index;
		arg = readcode(buffer, &opcode);
		if(opcode == C_CMNT || strnull(arg)) {
			continue;
		}
		if (opcode == C_PASS) {
			err_exist = 1;
			fprintf(stderr, "syntax error: line %d\n", line_index);
			continue;
		}
		switch(opcode) {
			case C_LABL: {
				hashtab_insert(hashtab, arg, &byte_index, sizeof(byte_index));
			}
			break;
			case C_PUSH: {
				byte_index += 5;
			}
			break;
			default: {
				byte_index += 1;
			}
		}
	}

	if (err_exist) {
		hashtab_free(hashtab);
		return 1;
	}

	fseek(input, 0, SEEK_SET);

	while(fgets(buffer, BUFSIZ, input) != NULL) {
		arg = readcode(buffer, &opcode);
		switch (opcode) {
			case C_PASS: case C_CMNT: case C_LABL: {
			}
			break;
			case C_PUSH: {
				uint8_t bytes[4];
				int32_t *temp;
				int32_t num;
				temp = hashtab_select(hashtab, arg);
				if (temp == NULL) {
					num = atoi(arg);
				} else {
					num = *temp;
				}
				split_32bits_to_8bits((uint32_t)num, bytes);
				fprintf(output, "%c%c%c%c%c", opcode, bytes[0], bytes[1], bytes[2], bytes[3]);
			}
			break;
			default: {
				fprintf(output, "%c", opcode);
			}
		}
	}

	hashtab_free(hashtab);
	return 0;
}

static char *readcode(char *line, uint8_t *opcode) {
	char *ptr;
	// pass spaces
	ptr = line;
	while(isspace(*ptr)) {
		++ptr;
	}
	// read chars of opcode
	line = ptr;
	while(!isspace(*ptr)) {
		++ptr;
	}
	*ptr = '\0';
	// get opcode int
	*opcode = C_PASS;
	for (int i = 0; i < INSTR_SIZE; ++i) {
		if (strcmp(strtolower(line), VM.bclist[i].mnem) == 0) {
			*opcode = VM.bclist[i].bcode;
			break;
		}
	}
	// pass if opcode without arguments 
	if (*opcode != C_PUSH && *opcode != C_LABL) {
		return line;
	}
	// pass spaces
	++ptr;
	while(isspace(*ptr)) {
		++ptr;
	}
	// read chars of first argument
	line = ptr;
	while(!isspace(*ptr)) {
		++ptr;
	}
	*ptr = '\0';
	return line;
}

static uint32_t join_8bits_to_32bits(uint8_t *bytes) {
	uint32_t num;
	for (uint8_t *ptr = bytes; ptr < bytes + 4; ++ptr) {
		num = (num << 8) | *ptr;
	}
	return num;
}

static void split_32bits_to_8bits(uint32_t num, uint8_t *bytes) {
	for (int i = 0; i < 4; ++i) {
		bytes[i] = (uint8_t)(num >> (24 - i * 8));
	}
}

static char *strtolower(char *str) {
	int len = strlen(str);
	for (int i = 0; i < len; ++i) {
		str[i] = tolower(str[i]);
	}
	return str;
}

static int strnull(char *str) {
	while(isspace(*str)) {
		++str;
	}
	if(*str == '\0') {
		return 1;
	}
	return 0;
}
