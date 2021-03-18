#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "extclib/type/hashtab.h"
#include "extclib/type/stack.h"

#define OLIST_SIZE 16
#define STACK_SIZE 5000

enum {
	PUSH_CODE    = 0,
	POP_CODE     = 1,
	ADD_CODE     = 2,
	SUB_CODE     = 3,
	JMP_CODE     = 4,
	JL_CODE      = 5,
	JG_CODE      = 6,
	JE_CODE      = 7,
	STORE_CODE   = 8,
	LOAD_CODE    = 9,
	CALL_CODE    = 10,
	RET_CODE     = 11,
	HLT_CODE     = 12,
	ALLOC_CODE   = 13,
	LABEL_CODE   = 14,
	COMMENT_CODE = 15,
	PASS_CODE    = 16,
};

static const char *opcodelist[OLIST_SIZE] = {
	[PUSH_CODE]    = "push",
	[POP_CODE]     = "pop",
	[ADD_CODE]     = "add",
	[SUB_CODE]     = "sub",
	[JMP_CODE]     = "jmp",
	[JL_CODE]      = "jl",
	[JG_CODE]      = "jg",
	[JE_CODE]      = "je",
	[STORE_CODE]   = "store",
	[LOAD_CODE]    = "load",
	[CALL_CODE]    = "call",
	[RET_CODE]     = "ret",
	[HLT_CODE]     = "hlt",
	[ALLOC_CODE]   = "alloc",
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
			case ALLOC_CODE: {
				int32_t num, pad;
				uint8_t bytes[4];
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = (int32_t)join_8bits_to_32bits(bytes);
				pad = 0x00;
				for (int i = 0; i < num; ++i) {
					stack_push(stack, &pad);
				}
			}
			break;
			case PUSH_CODE: {
				int32_t num;
				uint8_t bytes[4];
				if (stack_size(stack) == STACK_SIZE) {
					return 1;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = (int32_t)join_8bits_to_32bits(bytes);
				stack_push(stack, &num);
			}
			break;
			case POP_CODE: {
				if (stack_size(stack) == 0) {
					return 2;
				}
				*result = *(int32_t*)stack_pop(stack);
			}
			break;
			case ADD_CODE: case SUB_CODE: {
				int32_t x, y;
				if (stack_size(stack) <= 1) {
					return 3;
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
			case JMP_CODE: {
				uint8_t bytes[4];
				int32_t num;
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = (int32_t)join_8bits_to_32bits(bytes);
				if (num < 0) {
					int32_t pos = stack_size(stack) + num;
					if (pos < 0) {
						return 4;
					}
					num = *(int32_t*)stack_get(stack, pos);
				}
				if (num >= fsize) {
					return 5;
				}
				fseek(input, num, SEEK_SET);
			}
			break;
			case JL_CODE: case JG_CODE: case JE_CODE: {
				uint8_t bytes[4];
				int32_t num, pos, x, y;
				if (stack_size(stack) <= 1) {
					return 6;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = (int32_t)join_8bits_to_32bits(bytes);
				if (num < 0) {
					pos = stack_size(stack) + num;
					if (pos < 0) {
						return 7;
					}
					num = *(int32_t*)stack_get(stack, pos);
				}
				if (num >= fsize) {
					return 8;
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
				int32_t pos1, pos2;
				fscanf(input, "%c%c%c%c%c%c%c%c", 
					&bytes[0], &bytes[1], &bytes[2], &bytes[3],
					&bytes[4], &bytes[5], &bytes[6], &bytes[7]);
				num1 = (int32_t)join_8bits_to_32bits(bytes);
				num2 = (int32_t)join_8bits_to_32bits(bytes+4);
				if (num1 < 0) {
					pos1 = stack_size(stack) + num1;
					if (pos1 < 0) {
						return 9;
					}
				} else {
					pos1 = num1;
					if (pos1 >= stack_size(stack)) {
						return 10;
					}
				}
				if (num2 < 0) {
					pos2 = stack_size(stack) + num2;
					if (pos2 < 0) {
						return 11;
					}
				} else {
					pos2 = num2;
					if (pos2 >= stack_size(stack)) {
						return 12;
					}
				}
				num2 = *(int32_t*)stack_get(stack, pos2);
				stack_set(stack, pos1, &num2);
			}
			break;
			case LOAD_CODE: {
				uint8_t bytes[4];
				int32_t pos, num;
				if (stack_size(stack) == STACK_SIZE) {
					return 13;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = (int32_t)join_8bits_to_32bits(bytes);
				if (num < 0) {
					pos = stack_size(stack) + num;
					if (pos < 0) {
						return 14;
					}
				} else {
					pos = num;
					if (pos >= stack_size(stack)) {
						return 15;
					}
				}
				num = *(int32_t*)stack_get(stack, pos);
				stack_push(stack, &num);
			}
			break;
			case CALL_CODE: {
				uint8_t bytes[4];
				int32_t num, pos;
				if (stack_size(stack) == STACK_SIZE) {
					return 16;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = (int32_t)join_8bits_to_32bits(bytes);
				if (num < 0) {
					int32_t pos = stack_size(stack) + num;
					if (pos < 0) {
						return 17;
					}
					num = *(int32_t*)stack_get(stack, pos);
				}
				if (num >= fsize) {
					return 18;
				}
				pos = ftell(input);
				stack_push(stack, &pos);
				fseek(input, num, SEEK_SET);
			}
			break;
			case RET_CODE: {
				int32_t num;
				if (stack_size(stack) == 0) {
					return 19;
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
	char buffer[BUFSIZ];
	int line_index, curr_pos, err_exist;
	char *arg;
	uint8_t opcode;

	hashtab = hashtab_new(250);
	line_index = 0;
	curr_pos = 0;
	err_exist = 0;

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
				curr_pos += 1;
			break;
			case PUSH_CODE: case JMP_CODE: case JL_CODE: case JG_CODE:
			case JE_CODE: case LOAD_CODE: case CALL_CODE: case ALLOC_CODE:
				curr_pos += 5;
			break;
			case STORE_CODE:
				curr_pos += 9;
			break;
			case LABEL_CODE:
				hashtab_insert(hashtab, arg, &curr_pos, sizeof(curr_pos));
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
			case PUSH_CODE: case JMP_CODE: case JL_CODE: case JG_CODE:
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
			case ALLOC_CODE: case LOAD_CODE: {
				uint8_t bytes[4];
				int32_t num;
				num = atoi(arg);
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
	for (int i = 0; i < OLIST_SIZE; ++i) {
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