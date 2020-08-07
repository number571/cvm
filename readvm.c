#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "extclib/hashtab.h"
#include "extclib/stack.h"

#define INSTRUCTION_NUM 18
#define STACK_SIZE 10000

typedef enum opcode_t {
	PUSH_CODE,
	POP_CODE,
	ADD_CODE,
	SUB_CODE,
	MUL_CODE,
	DIV_CODE,
	JMP_CODE,
	JL_CODE,
	JG_CODE,
	JE_CODE,
	JNE_CODE,
	STORE_CODE,
	LOAD_CODE,
	CALL_CODE,
	RET_CODE,
	HLT_CODE,
	LABEL_CODE,
	COMMENT_CODE,
	PASS_CODE,
} opcode_t;

static const char *opcodelist[INSTRUCTION_NUM] = {
	[PUSH_CODE]    = "push",
	[POP_CODE]     = "pop",
	[ADD_CODE]     = "add",
	[SUB_CODE]     = "sub",
	[MUL_CODE]     = "mul",
	[DIV_CODE]     = "div",
	[JMP_CODE]     = "jmp",
	[JL_CODE]      = "jl",
	[JG_CODE]      = "jg",
	[JE_CODE]      = "je",
	[JNE_CODE]     = "jne",
	[STORE_CODE]   = "store",
	[LOAD_CODE]    = "load",
	[CALL_CODE]    = "call",
	[RET_CODE]     = "ret",
	[HLT_CODE]     = "hlt",
	[LABEL_CODE]   = "label",
	[COMMENT_CODE] = ";",
};

extern int8_t readvm_src(FILE *output, FILE *input);
extern int32_t readvm_exc(FILE *input);

static void _print_error(FILE *input, opcode_t opcode);
static char *_readcode(char *line, opcode_t *opcode);
static int32_t _join_8bits_to_32bits(uint8_t *bytes);
static void _split_32bits_to_8bits(int32_t num, uint8_t *bytes);
static _Bool _strnull(char *str);
static _Bool _isspace(char ch);

extern int32_t readvm_exc(FILE *input) {
	fseek(input, 0, SEEK_END);
	size_t fsize = ftell(input);

	Stack *stack = new_stack(STACK_SIZE, DECIMAL_TYPE);
	fseek(input, 0, SEEK_SET);

	uint8_t ch;
	int32_t value = 0;
	opcode_t opcode;

	while(!feof(input) && ftell(input) != fsize) {
		fscanf(input, "%c", &ch);
		switch(opcode = ch) {
			case PUSH_CODE: {
				if (size_stack(stack) == STACK_SIZE) {
					_print_error(input, opcode);
					return -1;
				}
				uint8_t bytes[4];
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				int32_t num = _join_8bits_to_32bits(bytes);
				push_stack(stack, decimal(num));
			}
			break;
			case POP_CODE: {
				if (size_stack(stack) == 0) {
					_print_error(input, opcode);
					return -1;
				}
				int32_t num = pop_stack(stack).decimal;
				value = num;
			}
			break;
			case ADD_CODE: case SUB_CODE: case MUL_CODE: case DIV_CODE: {
				if (size_stack(stack) <= 1) {
					_print_error(input, opcode);
					return -1;
				}
				int32_t x = pop_stack(stack).decimal;
				int32_t y = pop_stack(stack).decimal;
				switch(opcode) {
					case ADD_CODE:
						y += x;
					break;
					case SUB_CODE:
						y -= x;
					break;
					case MUL_CODE:
						y *= x;
					break;
					case DIV_CODE:
						y /= x;
					break;
					default: ;
				}
				push_stack(stack, decimal(y));
			}
			break;
			case JMP_CODE: {
				uint8_t bytes[4];
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				int32_t num = _join_8bits_to_32bits(bytes);
				if (num < 0) {
					int32_t pos = size_stack(stack) + num;
					if (pos < 0) {
						_print_error(input, opcode);
						return -1;
					}
					num = get_stack(stack, pos).decimal;
				}
				if (num >= fsize) {
					_print_error(input, opcode);
					return -1;
				}
				fseek(input, num, SEEK_SET);
			}
			break;
			case JL_CODE: case JG_CODE: case JE_CODE: case JNE_CODE: {
				if (size_stack(stack) <= 1) {
					_print_error(input, opcode);
					return -1;
				}
				uint8_t bytes[4];
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				int32_t num = _join_8bits_to_32bits(bytes);
				if (num < 0) {
					int32_t pos = size_stack(stack) + num;
					if (pos < 0) {
						_print_error(input, opcode);
						return -1;
					}
					num = get_stack(stack, pos).decimal;
				}
				if (num >= fsize) {
					_print_error(input, opcode);
					return -1;
				}
				int32_t x = pop_stack(stack).decimal;
				int32_t y = pop_stack(stack).decimal;
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
					case JNE_CODE:
						if (y != x) {
							fseek(input, num, SEEK_SET);
						}
					break;
					default: ;
				}
			}
			break;
			case STORE_CODE: {
				int32_t num1, num2;
				uint8_t bytes[8];
				fscanf(input, "%c%c%c%c%c%c%c%c", 
					&bytes[0], &bytes[1], &bytes[2], &bytes[3],
					&bytes[4], &bytes[5], &bytes[6], &bytes[7]);
				num1 = _join_8bits_to_32bits(bytes);
				num2 = _join_8bits_to_32bits(bytes+4);
				if (num1 < 0) {
					num1 = size_stack(stack) + num1;
				}
				if (num2 < 0) {
					int32_t pos = size_stack(stack) + num2;
					if (pos < 0) {
						_print_error(input, opcode);
						return -1;
					}
					num2 = get_stack(stack, pos).decimal;
				}
				if (num1 >= size_stack(stack)) {
					_print_error(input, opcode);
					return -1;
				}
				set_stack(stack, num1, decimal(num2));
			}
			break;
			case LOAD_CODE: {
				if (size_stack(stack) == STACK_SIZE) {
					_print_error(input, opcode);
					return -1;
				}
				uint8_t bytes[4];
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				int32_t pos;
				int32_t num = _join_8bits_to_32bits(bytes);
				if (num < 0) {
					pos = size_stack(stack) + num;
					if (pos < 0) {
						_print_error(input, opcode);
						return -1;
					}
				} else {
					pos = num;
					if (pos >= size_stack(stack)) {
						_print_error(input, opcode);
						return -1;
					}
				}
				num = get_stack(stack, pos).decimal;
				push_stack(stack, decimal(num));
			}
			break;
			case CALL_CODE: {
				if (size_stack(stack) == STACK_SIZE) {
					_print_error(input, opcode);
					return -1;
				}
				uint8_t bytes[4];
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				int32_t num = _join_8bits_to_32bits(bytes);
				if (num < 0) {
					int32_t pos = size_stack(stack) + num;
					if (pos < 0) {
						_print_error(input, opcode);
						return -1;
					}
					num = get_stack(stack, pos).decimal;
				}
				if (num >= fsize) {
					_print_error(input, opcode);
					return -1;
				}
				int32_t pos = ftell(input);
				push_stack(stack, decimal(pos));
				fseek(input, num, SEEK_SET);
			}
			break;
			case RET_CODE: {
				if (size_stack(stack) == 0) {
					_print_error(input, opcode);
					return -1;
				}
				int32_t num = pop_stack(stack).decimal;
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
	free_stack(stack);
	return value;
}

extern int8_t readvm_src(FILE *output, FILE *input) {
	HashTab *hashtab = new_hashtab(250, STRING_TYPE, DECIMAL_TYPE);
	char buffer[BUFSIZ] = {0};

	size_t line_index = 0;
	size_t curr_pos = 0;
	_Bool err_exist = 0;

	char *arg;
	opcode_t opcode;

	while(fgets(buffer, BUFSIZ, input) != NULL) {
		++line_index;
		arg = _readcode(buffer, &opcode);
		if((opcode == PASS_CODE && _strnull(arg)) || opcode == COMMENT_CODE) {
			continue;
		}
		if (opcode == PASS_CODE) {
			err_exist = 1;
			fprintf(stderr, "syntax error: line %ld\n", line_index);
			continue;
		}
		switch(opcode) {
			case PUSH_CODE: case JMP_CODE: case JL_CODE: case JG_CODE: case JE_CODE:
			case JNE_CODE: case LOAD_CODE: case CALL_CODE:
				curr_pos += 5;
			break;
			case POP_CODE: case ADD_CODE: case SUB_CODE: case MUL_CODE: case DIV_CODE:
			case RET_CODE: case HLT_CODE:
				curr_pos += 1;
			break;
			case STORE_CODE:
				curr_pos += 9;
			break;
			case LABEL_CODE:
				set_hashtab(hashtab, string(arg), decimal(curr_pos));
			break;
			default: ;
		}
	}

	if (err_exist) {
		free_hashtab(hashtab);
		return 1;
	}

	fseek(input, 0, SEEK_SET);

	while(fgets(buffer, BUFSIZ, input) != NULL) {
		arg = _readcode(buffer, &opcode);
		switch (opcode) {
			case POP_CODE: case ADD_CODE: case SUB_CODE: case MUL_CODE: case DIV_CODE:
			case RET_CODE: case HLT_CODE: {
				fprintf(output, "%c", opcode);
			}
			break;
			case JMP_CODE: case JL_CODE: case JG_CODE: case JE_CODE: case JNE_CODE: 
			case CALL_CODE: case PUSH_CODE: {
				int32_t num;
				if (in_hashtab(hashtab, arg)) {
					num = get_hashtab(hashtab, arg).decimal;
				} else {
					num = atoi(arg);
				}
				uint8_t bytes[4];
				_split_32bits_to_8bits(num, bytes);
				fprintf(output, "%c%c%c%c%c", opcode, bytes[0], bytes[1], bytes[2], bytes[3]);
			}
			break;
			case LOAD_CODE: {
				int32_t num = atoi(arg);
				uint8_t bytes[4];
				_split_32bits_to_8bits(num, bytes);
				fprintf(output, "%c%c%c%c%c", opcode, bytes[0], bytes[1], bytes[2], bytes[3]);
			}
			break;
			case STORE_CODE: {
				char *arg2 = arg + strlen(arg) + 1;
				while(isspace(*arg2)) {
					++arg2;
				}
				char *ptr = arg2;
				while(!isspace(*ptr)) {
					++ptr;
				}
				*ptr = '\0';
				int32_t num1 = atoi(arg);
				int32_t num2 = atoi(arg2);
				uint8_t bytes[8];
				_split_32bits_to_8bits(num1, bytes);
				_split_32bits_to_8bits(num2, bytes+4);
				fprintf(output, "%c%c%c%c%c%c%c%c%c", opcode,
					bytes[0], bytes[1], bytes[2], bytes[3],
					bytes[4], bytes[5], bytes[6], bytes[7]);
			}
			break;
			default: ;
		}
	}

	free_hashtab(hashtab);
	return 0;
}

static void _print_error(FILE *input, opcode_t opcode) {
	fprintf(stderr, "Error: %s (0x%02X)\nPosition: %ld bytes\n", opcodelist[opcode], opcode, ftell(input));
}

static char *_readcode(char *line, opcode_t *opcode) {
	char *ptr = line;
	while(isspace(*ptr)) {
		++ptr;
	}
	line = ptr;
	while(!_isspace(*ptr)) {
		++ptr;
	}
	*ptr = '\0';
	*opcode = PASS_CODE;
	for (size_t i = 0; i < INSTRUCTION_NUM; ++i) {
		if (strcmp(line, opcodelist[i]) == 0) {
			*opcode = i;
			break;
		}
	}
	switch(*opcode) {
		case PASS_CODE:
		case POP_CODE:
		case COMMENT_CODE:
		case RET_CODE:
		case HLT_CODE:
		case ADD_CODE:
		case SUB_CODE: 
		case MUL_CODE:
		case DIV_CODE:
			return line;
		default: ;
	}

	++ptr;
	while(isspace(*ptr)) {
		++ptr;
	}

	line = ptr;
	while(!_isspace(*ptr)) {
		++ptr;
	}
	*ptr = '\0';
	return line;
}

static int32_t _join_8bits_to_32bits(uint8_t *bytes) {
	int32_t num;
	for (uint8_t *ptr = bytes; ptr < bytes + 4; ++ptr) {
		num = (num << 8) | *ptr;
	}
	return num;
}

static void _split_32bits_to_8bits(int32_t num, uint8_t *bytes) {
	for (size_t i = 0; i < 4; ++i) {
		bytes[i] = (uint8_t)(num >> (24 - i * 8));
	}
}

static _Bool _strnull(char *str) {
	while(isspace(*str)) {
		++str;
	}
	if(*str == '\0') {
		return 1;
	}
	return 0;
}

static _Bool _isspace(char ch) {
	if (ch == '\0' || isspace(ch)) {
		return 1;
	}
	return 0;
}
