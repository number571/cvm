#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "cvmkernel.h"

#include "typeslib/hashtab.h"
#include "typeslib/stack.h"

// Number of all instructions.
#ifdef CVM_KERNEL_IAPPEND
	#define CVM_KERNEL_ISIZE 31
#else
	#define CVM_KERNEL_ISIZE 14
#endif

// N - number
// C - char
enum {
	// 0xNN 
	// PSEUDO INSTRUCTIONS (2)
	C_CMNT = 0x11, // 0 bytes
	C_LABL = 0x22, // 0 bytes
	// 0xCC 
	// NULL INSTRUCTIONS (2)
	C_UNDF = 0xAA, // 0 bytes
	C_VOID = 0xBB, // 0 bytes
	// 0xNC 
	// MAIN INSTRUCTIONS (10)
	C_PUSH = 0x0A, // 5 bytes
	C_POP  = 0x0B, // 1 byte
	C_INC  = 0x0C, // 1 byte
	C_DEC  = 0x0D, // 1 byte
	C_JMP  = 0x0E, // 1 byte
	C_JG   = 0x0F, // 1 byte
	C_STOR = 0x1A, // 1 byte
	C_LOAD = 0x1B, // 1 byte
	C_CALL = 0x1C, // 1 byte
	C_HLT  = 0x1D, // 1 byte
#ifdef CVM_KERNEL_IAPPEND
	// 0xCN 
	// ADD INSTRUCTIONS (17)
	C_ADD  = 0xA0, // 1 byte
	C_SUB  = 0xB0, // 1 byte
	C_MUL  = 0xC0, // 1 byte
	C_DIV  = 0xD0, // 1 byte
	C_MOD  = 0xE0, // 1 byte
	C_SHR  = 0xF0, // 1 byte
	C_SHL  = 0xA1, // 1 byte
	C_XOR  = 0xB1, // 1 byte
	C_AND  = 0xC1, // 1 byte
	C_OR   = 0xD1, // 1 byte
	C_NOT  = 0xE1, // 1 byte
	C_JE   = 0xF1, // 1 byte
	C_JL   = 0xA2, // 1 byte
	C_JNE  = 0xB2, // 1 byte
	C_JLE  = 0xC2, // 1 byte
	C_JGE  = 0xD2, // 1 byte
	C_ALLC = 0xE2, // 1 byte
#endif
};

static struct virtual_machine {
	int32_t cmused;
	uint8_t memory[CVM_KERNEL_CMEMORY];
	struct {
		uint8_t bcode;
		char *mnem;
	} bclist[CVM_KERNEL_ISIZE];
} VM = {
	.cmused = 0,
	.bclist = {
		// PSEUDO INSTRUCTIONS
		{ C_CMNT, ";"    }, // 0 arg
		{ C_LABL, "labl" }, // 1 arg
		// NULL INSTRUCTIONS
		{ C_VOID, "\0"   }, // 0 arg
		{ C_UNDF, "\1"   }, // 0 arg
		// MAIN INSTRUCTIONS
		{ C_PUSH, "push" }, // 1 arg, 0 stack
		{ C_POP,  "pop"  }, // 0 arg, 1 stack
		{ C_INC,  "inc"  }, // 0 arg, 1 stack
		{ C_DEC,  "dec"  }, // 0 arg, 1 stack
		{ C_JMP,  "jmp"  }, // 0 arg, 1 stack
		{ C_JG,   "jg"   }, // 0 arg, 3 stack
		{ C_STOR, "stor" }, // 0 arg, 2 stack
		{ C_LOAD, "load" }, // 0 arg, 1 stack
		{ C_CALL, "call" }, // 0 arg, 1 stack
		{ C_HLT,  "hlt"  }, // 0 arg, 0 stack
#ifdef CVM_KERNEL_IAPPEND
		// ADD INSTRUCTIONS
		{ C_ADD,  "add"  }, // 0 arg, 2 stack
		{ C_SUB,  "sub"  }, // 0 arg, 2 stack
		{ C_MUL,  "mul"  }, // 0 arg, 2 stack
		{ C_DIV,  "div"  }, // 0 arg, 2 stack
		{ C_MOD,  "mod"  }, // 0 arg, 2 stack
		{ C_SHR,  "shr"  }, // 0 arg, 2 stack
		{ C_SHL,  "shl"  }, // 0 arg, 2 stack
		{ C_XOR,  "xor"  }, // 0 arg, 2 stack
		{ C_AND,  "and"  }, // 0 arg, 2 stack
		{ C_OR,   "or"   }, // 0 arg, 2 stack
		{ C_NOT,  "not"  }, // 0 arg, 1 stack
		{ C_JE,   "je"   }, // 0 arg, 3 stack
		{ C_JL,   "jl"   }, // 0 arg, 3 stack
		{ C_JNE,  "jne"  }, // 0 arg, 3 stack
		{ C_JLE,  "jle"  }, // 0 arg, 3 stack
		{ C_JGE,  "jge"  }, // 0 arg, 3 stack
		{ C_ALLC, "allc" }, // 0 arg, 1 stack
#endif
	},
};

static void compile_push(FILE *output, hashtab_t *hashtab, char *arg);
static char *read_opcode(char *line, uint8_t *opcode);
static uint8_t find_opcode(char *str);
static void split_32bits_to_8bits(uint32_t num, uint8_t *bytes);

static char *str_trim_spaces(char *str);
static char *str_set_end(char *str);
static char *str_to_lower(char *str);
static int str_is_number(char *str);

#ifdef CVM_KERNEL_IAPPEND
	static int exec_not(stack_t *stack);
	static int exec_binop(stack_t *stack, uint8_t opcode);
	static int exec_allc(stack_t *stack);
#endif 

static int exec_push(stack_t *stack, int32_t *mi);
static int exec_pop(stack_t *stack);
static int exec_incdec(stack_t *stack, uint8_t opcode);
static int exec_stor(stack_t *stack);
static int exec_load(stack_t *stack);
static int exec_jmp(stack_t *stack, int32_t *mi);
static int exec_jmpif(stack_t *stack, uint8_t opcode, int32_t *mi);
static int exec_call(stack_t *stack, int32_t *mi);

static uint32_t join_8bits_to_32bits(uint8_t *bytes);
static uint16_t wrap_return(uint8_t x, uint8_t y);

/// SECTION: COMPILE

// translate assembly mnemonics to byte codes
// example: ("PUSH 5" -> C_PUSH || 0x00 || 0x00 || 0x00 || 0x05)
// example: ("POP" -> C_POP)
extern int cvm_compile(FILE *output, FILE *input) {
	hashtab_t *hashtab;
	int32_t bindex;
	char buffer[BUFSIZ];
	char *arg;
	uint8_t opcode;

	hashtab = hashtab_new(512);
	bindex = 0;

	// save label addresses into hashtab
	while(fgets(buffer, BUFSIZ, input) != NULL) {
		arg = read_opcode(buffer, &opcode);

		// undefined instruction
		if (opcode == C_UNDF) {
			hashtab_free(hashtab);
			return 1;
		}

		// if comment or void string
		if(opcode == C_CMNT || opcode == C_VOID) {
			continue;
		}

		switch(opcode) {
			// label instruction -> save current address
			case C_LABL:
				if (strlen(arg) == 0 || str_is_number(arg)) {
					hashtab_free(hashtab);
					return 2;
				}
				hashtab_set(hashtab, arg, &bindex, sizeof(bindex));
			break;
			// push instruction -> +5 bytes 
			case C_PUSH:
				bindex += 5;
			break;
			// another instruction -> +1 byte
			default:
				bindex += 1;
			break;
		}
	}

	// read file from the beginning 
	fseek(input, 0, SEEK_SET);

	// write byte codes with saved label addresses
	while(fgets(buffer, BUFSIZ, input) != NULL) {
		arg = read_opcode(buffer, &opcode);
		switch (opcode) {
			// pass null and pseudo instructions
			case C_VOID: case C_CMNT: case C_LABL:
			break;
			// push instruction = 5 bytes 
			case C_PUSH: 
				if (strlen(arg) == 0) {
					hashtab_free(hashtab);
					return 3;
				}
				compile_push(output, hashtab, arg);
			break;
			// another instruction = 1 byte
			default:
				fprintf(output, "%c", opcode);
			break;
		}
	}

	hashtab_free(hashtab);
	return 0;
}

// load value from hashtab (if exists) 
// and convert int32->bytes[4]
static void compile_push(FILE *output, hashtab_t *hashtab, char *arg) {
	uint8_t bytes[4];
	int32_t *temp;
	int32_t num;

	temp = hashtab_get(hashtab, arg);
	if (temp == NULL) {
		num = atoi(arg);
	} else {
		num = *temp;
	}

	split_32bits_to_8bits((uint32_t)num, bytes);
	fprintf(output, "%c%c%c%c%c", C_PUSH, bytes[0], bytes[1], bytes[2], bytes[3]);
}

// read opcode from string and return
// pointer to first argument if exists
static char *read_opcode(char *line, uint8_t *opcode) {
	char *ptr;

	// get first word in line
	line = str_trim_spaces(line);
	ptr = str_set_end(line);

	// get opcode from word
	*opcode = find_opcode(line);
	switch(*opcode) {
		case C_PUSH: case C_LABL:
			break;
		default:
			return NULL;
	}

	// get second word in line
	line = str_trim_spaces(++ptr);
	str_set_end(line);

	// pointer to first arg
	return line;
}

// get instruction by mnemonic
// example: "push" -> C_PUSH
static uint8_t find_opcode(char *str) {
	uint8_t opcode;

	// default value
	opcode = C_UNDF;

	// opcode from word
	for (int i = 0; i < CVM_KERNEL_ISIZE; ++i) {
		if (strcmp(str_to_lower(str), VM.bclist[i].mnem) == 0) {
			opcode = VM.bclist[i].bcode;
			break;
		}
	}

	return opcode;
}

// example: "PUSH" -> "push"
static char *str_to_lower(char *str) {
	int len = strlen(str);

	for (int i = 0; i < len; ++i) {
		str[i] = tolower(str[i]);
	}

	return str;
}

// example: "12345" -> true
// example: "a12345", "12345a" -> false
static int str_is_number(char *str) {
	int len = strlen(str);
	if (len == 0) {
		return 0;
	}

	for (int i = 0; i < len; ++i) {
		if (!isdigit(str[i])) {
			return 0;
		}
	}

	return 1; 
}

// example: "  word1 word2 word3" -> "word1 word2 word3"
static char *str_trim_spaces(char *str) {
	while(isspace(*str)) {
		++str;
	}

	return str;
}

// example: "word1 word2 word3" -> "word1\0word2 word3"
static char *str_set_end(char *str) {
	char *ptr = str;

	while(!isspace(*ptr)) {
		++ptr;
	}

	*ptr = '\0';
	return ptr;
}

// return (x[0], x[1], x[2], x[3])
static void split_32bits_to_8bits(uint32_t num, uint8_t *bytes) {
	for (int i = 0; i < 4; ++i) {
		bytes[i] = (uint8_t)(num >> (24 - i * 8));
	}
}



/// SECTION: LOAD

// load byte codes to static memory of virtual machine
extern int cvm_load(uint8_t *memory, int32_t msize) {
	if (msize < 0 || msize >= CVM_KERNEL_CMEMORY) {
		return 1;
	}

	memcpy(VM.memory, memory, msize);
	VM.cmused = msize;

	return 0;
}



/// SECTION: RUN

// byte code interpretation 
extern int cvm_run(int32_t **output, int32_t *input) {
	stack_t *stack;
	uint8_t opcode;
	int32_t mi;
	int retcode;

	stack = stack_new(CVM_KERNEL_SMEMORY, sizeof(int32_t));
	for (int i = 1; i <= input[0]; ++i) {
		stack_push(stack, &input[i]);
	}

	mi = 0;
	while(mi < VM.cmused) {
		opcode = VM.memory[mi++];

		switch(opcode) {
		#ifdef CVM_KERNEL_IAPPEND
			case C_MUL: case C_DIV:
			case C_MOD: case C_AND: 
			case C_OR:  case C_XOR:
			case C_SHR: case C_SHL: 
			case C_ADD: case C_SUB: 
				retcode = exec_binop(stack, opcode);
			break;
			case C_NOT:
				retcode = exec_not(stack);
			break;
			case C_ALLC: 
				retcode = exec_allc(stack);
			break;
		#endif
		#ifdef CVM_KERNEL_IAPPEND
			case C_JGE: case C_JLE: case C_JNE: case C_JL: case C_JE: 
		#endif 
			case C_JG: 
				retcode = exec_jmpif(stack, opcode, &mi);
			break;
			case C_JMP: 
				retcode = exec_jmp(stack, &mi);
			break;
			case C_CALL: 
				retcode = exec_call(stack, &mi);
			break;
			case C_PUSH:
				retcode = exec_push(stack, &mi);
			break;
			case C_POP:
				retcode = exec_pop(stack);
			break;
			case C_INC: case C_DEC:
				retcode = exec_incdec(stack, opcode);
			break;
			case C_STOR: 
				retcode = exec_stor(stack);
			break;
			case C_LOAD: 
				retcode = exec_load(stack);
			break;
			case C_HLT:
				mi = VM.cmused;
				retcode = 0;
			break;
			default: 
				retcode = wrap_return(C_UNDF, 1);
			break;
		}
	
		if (retcode != 0) {
			stack_free(stack);
			return retcode;
		}
	}

	mi = stack_size(stack);

	*output = (int32_t*)malloc(sizeof(int32_t)*(mi+1));
	(*output)[0] = mi;

	for (int i = 1; i <= mi; ++i) {
		(*output)[i] = *(int32_t*)stack_pop(stack);
	}

	stack_free(stack);
	return 0;
}

// append new value in stack
static int exec_push(stack_t *stack, int32_t *mi) {
	int32_t num;
	uint8_t bytes[4];

	if (stack_size(stack) == CVM_KERNEL_SMEMORY) {
		return wrap_return(C_PUSH, 1);
	}

	memcpy(bytes, VM.memory + *mi, 4); *mi += 4;
	num = (int32_t)join_8bits_to_32bits(bytes);
	stack_push(stack, &num);

	return 0;
}

// delete last value from stack
static int exec_pop(stack_t *stack) {
	if (stack_size(stack) == 0) {
		return wrap_return(C_POP, 1);
	}

	stack_pop(stack);
	return 0;
}

// increment or decrement operation
static int exec_incdec(stack_t *stack, uint8_t opcode) {
	int32_t x;

	if (stack_size(stack) == 0) {
		return wrap_return(opcode, 1);
	}

	x = *(int32_t*)stack_pop(stack);

	switch(opcode) {
		case C_INC: ++x; break;
		case C_DEC: --x; break;
		default: 	return wrap_return(opcode, 2);
	}

	stack_push(stack, &x);
	return 0;
}

#ifdef CVM_KERNEL_IAPPEND
	// bitwise negation 
	static int exec_not(stack_t *stack) {
		int32_t x;

		if (stack_size(stack) == 0) {
			return wrap_return(C_NOT, 1);
		}

		x = ~*(int32_t*)stack_pop(stack);
		stack_push(stack, &x);

		return 0;
	}

	// binary operation @ -> y = y @ x
	static int exec_binop(stack_t *stack, uint8_t opcode) {
		int32_t x, y;

		if (stack_size(stack) < 2) {
			return wrap_return(opcode, 1);
		}

		x = *(int32_t*)stack_pop(stack);
		y = *(int32_t*)stack_pop(stack);

		switch(opcode) {
			case C_ADD:	y += x;		break;
			case C_SUB:	y -= x;		break;
			case C_MUL:	y *= x;		break;
			case C_DIV:	y /= x;		break;
			case C_MOD: y %= x;		break;
			case C_AND: y &= x;		break;
			case C_OR: 	y |= x;		break;
			case C_XOR: y ^= x;		break;
			case C_SHR:	y >>= x;	break;
			case C_SHL:	y <<= x;	break;
			default: 	return wrap_return(opcode, 2);
		}

		stack_push(stack, &y);
		return 0;
	}

	// allocate N values = 0 in stack
	static int exec_allc(stack_t *stack) {
		int32_t num, null;

		if (stack_size(stack) == 0) {
			return wrap_return(C_ALLC, 1);
		}

		num = *(int32_t*)stack_pop(stack);
		if (num < 0) {
			return wrap_return(C_ALLC, 2);
		}

		if (stack_size(stack)+num >= CVM_KERNEL_SMEMORY) {
			return wrap_return(C_ALLC, 3);
		}

		null = 0;
		for (int i = 0; i < num; ++i) {
			stack_push(stack, &null);
		}

		return 0;
	}
#endif

// store value in stack by two addresses
// where first address = in, second address = out
static int exec_stor(stack_t *stack) {
	int32_t num1, num2;

	if (stack_size(stack) < 2) {
		return wrap_return(C_STOR, 1);
	}

	num1 = *(int32_t*)stack_pop(stack);
	num2 = *(int32_t*)stack_pop(stack);

	if (num1 < 0) {
		num1 = stack_size(stack) + num1;
		if (num1 < 0) {
			return wrap_return(C_STOR, 2);
		}
	} else {
		if (num1 >= stack_size(stack)) {
			return wrap_return(C_STOR, 3);
		}
	}

	if (num2 < 0) {
		num2 = stack_size(stack) + num2;
		if (num2 < 0) {
			return wrap_return(C_STOR, 4);
		}
	} else {
		if (num2 >= stack_size(stack)) {
			return wrap_return(C_STOR, 5);
		}
	}

	num2 = *(int32_t*)stack_get(stack, num2);
	stack_set(stack, num1, &num2);

	return 0;
}

// load value in stack by address
// where address is last value in stack
static int exec_load(stack_t *stack) {
	int32_t num;

	if (stack_size(stack) == 0) {
		return wrap_return(C_LOAD, 1);
	}

	num = *(int32_t*)stack_pop(stack);
	if (num < 0) {
		num = stack_size(stack) + num;
		if (num < 0) {
			return wrap_return(C_LOAD, 2);
		}
	} else {
		if (num >= stack_size(stack)) {
			return wrap_return(C_LOAD, 3);
		}
	}

	num = *(int32_t*)stack_get(stack, num);
	stack_push(stack, &num);

	return 0;
}

// jump to address in code memory
// where address is last value in stack
extern int exec_jmp(stack_t *stack, int32_t *mi) {
	int32_t num;

	if (stack_size(stack) == 0) {
		return wrap_return(C_JMP, 1);
	}

	num = *(int32_t*)stack_pop(stack);
	if (num < 0 || num >= VM.cmused) {
		return wrap_return(C_JMP, 2);
	}

	*mi = num;
	return 0;
}

// jump to address in code memory if condition = true
static int exec_jmpif(stack_t *stack, uint8_t opcode, int32_t *mi) {
	int32_t num, x, y;

	if (stack_size(stack) < 3) {
		return wrap_return(opcode, 1);
	}

	num = *(int32_t*)stack_pop(stack);
	if (num < 0 || num >= VM.cmused) {
		return wrap_return(opcode, 2);
	}

	x = *(int32_t*)stack_pop(stack);
	y = *(int32_t*)stack_pop(stack);

	switch(opcode) {
		case C_JG: 	if(y >  x) {*mi = num;} break;
	#ifdef CVM_KERNEL_IAPPEND
		case C_JL:	if(y <  x) {*mi = num;} break;
		case C_JE:	if(y == x) {*mi = num;} break;
		case C_JNE: if(y != x) {*mi = num;} break;
		case C_JLE: if(y <= x) {*mi = num;} break;
		case C_JGE:	if(y >= x) {*mi = num;} break;
	#endif
		default: 	return wrap_return(opcode, 3);
	}

	return 0;
}

// exec jmp instruction with save current position in stack
static int exec_call(stack_t *stack, int32_t *mi) {
	int retcode;
	int32_t num;

	num = *mi;
	
	retcode = exec_jmp(stack, mi);
	if (retcode != 0) {
		return wrap_return(C_CALL, retcode & 0xFF);
	}

	stack_push(stack, &num);	
	return 0;
}

// return (x[0] || x[1] || x[2] || x[3])
static uint32_t join_8bits_to_32bits(uint8_t *bytes) {
	uint32_t num;

	for (uint8_t *ptr = bytes; ptr < bytes + 4; ++ptr) {
		num = (num << 8) | *ptr;
	}

	return num;
}

// return (x || y)
static uint16_t wrap_return(uint8_t x, uint8_t y) {
	return ((uint16_t)x << 8) | y;
}
