#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "extclib/type/hashtab.h"
#include "extclib/type/stack.h"

#define ILIST_SIZE 14
#define STACK_SIZE 5000

enum {
	PUSH_CODE    = 0,
	POP_CODE     = 1,
	ADD_CODE     = 2,
	SUB_CODE     = 3,
	JL_CODE      = 4,
	JG_CODE      = 5,
	JE_CODE      = 6,
	STORE_CODE   = 7,
	LOAD_CODE    = 8,
	CALL_CODE    = 9,
	RET_CODE     = 10,
	HLT_CODE     = 11,
	LABEL_CODE   = 12,
	COMMENT_CODE = 13,
	PASS_CODE    = 14,
};

static const char *opcodelist[ILIST_SIZE] = {
	[PUSH_CODE]    = "push",
	[POP_CODE]     = "pop",
	[ADD_CODE]     = "add",
	[SUB_CODE]     = "sub",
	[JL_CODE]      = "jl",
	[JG_CODE]      = "jg",
	[JE_CODE]      = "je",
	[STORE_CODE]   = "store",
	[LOAD_CODE]    = "load",
	[CALL_CODE]    = "call",
	[RET_CODE]     = "ret",
	[HLT_CODE]     = "hlt",
	[LABEL_CODE]   = "label",
	[COMMENT_CODE] = ";",
};

extern int readvm_src(FILE *output, FILE *input);
extern int readvm_exc(FILE *input, int *result);

static char *readcode(char *line, uint8_t *opcode);

static uint32_t join_8bits_to_32bits(uint8_t *bytes);
static void split_32bits_to_8bits(uint32_t num, uint8_t *bytes);

static char *strtolower(char *str);
static int strnull(char *str);

extern int readvm_exc(FILE *input, int *result) {
	stack_t *stack;
	int fsize;
	uint8_t opcode;

	stack = stack_new(STACK_SIZE, sizeof(int32_t));

	fseek(input, 0, SEEK_END);
	fsize = ftell(input);
	fseek(input, 0, SEEK_SET);

	while(!feof(input) && ftell(input) < fsize) {
		fscanf(input, "%c", &opcode);
		switch(opcode) {
			case PUSH_CODE: {
				int32_t num;
				uint8_t bytes[4];
				if (stack_size(stack) == STACK_SIZE) {
					return 2;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = (int32_t)join_8bits_to_32bits(bytes);
				stack_push(stack, &num);
			}
			break;
			case POP_CODE: {
				if (stack_size(stack) == 0) {
					return 3;
				}
				*result = *(int32_t*)stack_pop(stack);
			}
			break;
			case ADD_CODE: case SUB_CODE: {
				int32_t x, y;
				if (stack_size(stack) < 2) {
					return 4;
				}
				x = *(int32_t*)stack_pop(stack);
				y = *(int32_t*)stack_pop(stack);
				switch(opcode) {
					case ADD_CODE:
						y += x;
					break;
					case SUB_CODE:
						y -= x;
					break;
					default: ;
				}
				stack_push(stack, &y);
			}
			break;
			case JL_CODE: case JG_CODE: case JE_CODE: {
				uint8_t bytes[4];
				int32_t num, x, y;
				if (stack_size(stack) < 2) {
					return 5;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = (int32_t)join_8bits_to_32bits(bytes);
				if (num < 0) {
					num = stack_size(stack) + num;
					if (num < 0) {
						return 6;
					}
					num = *(int32_t*)stack_get(stack, num);
				}
				if (num >= fsize) {
					return 7;
				}
				x = *(int32_t*)stack_pop(stack);
				y = *(int32_t*)stack_pop(stack);
				switch(opcode) {
					case JL_CODE:
						if (y < x) {
							fseek(input, num, SEEK_SET);
						}
					break;
					case JG_CODE:
						if (y > x) {
							fseek(input, num, SEEK_SET);
						}
					break;
					case JE_CODE:
						if (y == x) {
							fseek(input, num, SEEK_SET);
						}
					break;
					default: ;
				}
			}
			break;
			case STORE_CODE: {
				uint8_t bytes[8];
				int32_t num1, num2;
				fscanf(input, "%c%c%c%c%c%c%c%c", 
					&bytes[0], &bytes[1], &bytes[2], &bytes[3],
					&bytes[4], &bytes[5], &bytes[6], &bytes[7]);
				num1 = (int32_t)join_8bits_to_32bits(bytes);
				num2 = (int32_t)join_8bits_to_32bits(bytes+4);
				if (num1 < 0) {
					num1 = stack_size(stack) + num1;
					if (num1 < 0) {
						return 8;
					}
				} else {
					if (num1 >= stack_size(stack)) {
						return 9;
					}
				}
				if (num2 < 0) {
					num2 = stack_size(stack) + num2;
					if (num2 < 0) {
						return 10;
					}
				} else {
					if (num2 >= stack_size(stack)) {
						return 11;
					}
				}
				num2 = *(int32_t*)stack_get(stack, num2);
				stack_set(stack, num1, &num2);
			}
			break;
			case LOAD_CODE: {
				uint8_t bytes[4];
				int32_t num;
				if (stack_size(stack) == STACK_SIZE) {
					return 12;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = (int32_t)join_8bits_to_32bits(bytes);
				if (num < 0) {
					num = stack_size(stack) + num;
					if (num < 0) {
						return 13;
					}
				} else {
					if (num >= stack_size(stack)) {
						return 14;
					}
				}
				num = *(int32_t*)stack_get(stack, num);
				stack_push(stack, &num);
			}
			break;
			case CALL_CODE: {
				uint8_t bytes[4];
				int32_t num, pos;
				if (stack_size(stack) == STACK_SIZE) {
					return 15;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = (int32_t)join_8bits_to_32bits(bytes);
				if (num < 0) {
					num = stack_size(stack) + num;
					if (num < 0) {
						return 16;
					}
					num = *(int32_t*)stack_get(stack, num);
				}
				if (num >= fsize) {
					return 17;
				}
				pos = ftell(input);
				stack_push(stack, &pos);
				fseek(input, num, SEEK_SET);
			}
			break;
			case RET_CODE: {
				int32_t num;
				if (stack_size(stack) == 0) {
					return 18;
				}
				num = *(int32_t*)stack_pop(stack);
				fseek(input, num, SEEK_SET);
			}
			break;
			case HLT_CODE:
				goto close;
			break;
			default: ;
		}
	}

close:
	stack_free(stack);
	return 0;
}

extern int readvm_src(FILE *output, FILE *input) {
	hashtab_t *hashtab;
	int line_index, byte_index, err_exist;
	char buffer[BUFSIZ];
	char *arg;
	uint8_t opcode;

	hashtab = hashtab_new(512);
	line_index = 0;
	byte_index = 0;
	err_exist  = 0;

	while(fgets(buffer, BUFSIZ, input) != NULL) {
		++line_index;
		arg = readcode(buffer, &opcode);
		if((opcode == PASS_CODE && strnull(arg)) || opcode == COMMENT_CODE) {
			continue;
		}
		if (opcode == PASS_CODE) {
			err_exist = 1;
			fprintf(stderr, "syntax error: line %d\n", line_index);
			continue;
		}
		switch(opcode) {
			case POP_CODE: case ADD_CODE: case SUB_CODE: 
			case RET_CODE: case HLT_CODE:
				byte_index += 1;
			break;
			case PUSH_CODE: case JL_CODE: case JG_CODE:
			case JE_CODE: case LOAD_CODE: case CALL_CODE:
				byte_index += 5;
			break;
			case STORE_CODE:
				byte_index += 9;
			break;
			case LABEL_CODE:
				hashtab_insert(hashtab, arg, &byte_index, sizeof(byte_index));
			break;
			default: ;
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
			case POP_CODE: case ADD_CODE: case SUB_CODE: 
			case RET_CODE: case HLT_CODE: {
				fprintf(output, "%c", opcode);
			}
			break;
			case PUSH_CODE: {
				uint8_t bytes[4];
				int32_t num;
				num = atoi(arg);
				split_32bits_to_8bits((uint32_t)num, bytes);
				fprintf(output, "%c%c%c%c%c", opcode, bytes[0], bytes[1], bytes[2], bytes[3]);
			}
			break;
			case LOAD_CODE: case JL_CODE: case JG_CODE:
			case JE_CODE: case CALL_CODE: {
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
			case STORE_CODE: {
				uint8_t bytes[8];
				int32_t num1, num2;
				char *arg2, *ptr;
				arg2 = arg + strlen(arg) + 1;
				while(isspace(*arg2)) {
					++arg2;
				}
				ptr = arg2;
				while(!isspace(*ptr)) {
					++ptr;
				}
				*ptr = '\0';
				num1 = atoi(arg);
				num2 = atoi(arg2);
				split_32bits_to_8bits((uint32_t)num1, bytes);
				split_32bits_to_8bits((uint32_t)num2, bytes+4);
				fprintf(output, "%c%c%c%c%c%c%c%c%c", opcode,
					bytes[0], bytes[1], bytes[2], bytes[3],
					bytes[4], bytes[5], bytes[6], bytes[7]);
			}
			break;
			default: ;
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
	*opcode = PASS_CODE;
	for (int i = 0; i < ILIST_SIZE; ++i) {
		if (strcmp(strtolower(line), opcodelist[i]) == 0) {
			*opcode = i;
			break;
		}
	}
	// pass if opcode without arguments 
	switch(*opcode) {
		case PASS_CODE:
		case POP_CODE:
		case COMMENT_CODE:
		case RET_CODE:
		case HLT_CODE:
		case ADD_CODE:
		case SUB_CODE:
			return line;
		default: ;
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