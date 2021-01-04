#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "extclib/type.h"

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

extern int readvm_src(FILE *output, FILE *input);
extern int readvm_exc(FILE *input);

static void _print_error(FILE *input, opcode_t opcode);
static char *_readcode(char *line, opcode_t *opcode);
static int32_t _join_8bits_to_32bits(uint8_t *bytes);
static void _split_32bits_to_8bits(int32_t num, uint8_t *bytes);
static _Bool _strnull(char *str);
static _Bool _isspace(char ch);

extern int readvm_exc(FILE *input) {
	type_stack *stack;
	int fsize;
	uint8_t ch;
	int32_t value;
	opcode_t opcode;

	value = 0;
	stack = type_stack_new(STACK_SIZE, sizeof(int32_t));

	fseek(input, 0, SEEK_END);
	fsize = ftell(input);
	fseek(input, 0, SEEK_SET);

	while(!feof(input) && ftell(input) != fsize) {
		fscanf(input, "%c", &ch);
		switch(opcode = ch) {
			case PUSH_CODE: {
				int32_t num;
				uint8_t bytes[4];
				if (type_stack_size(stack) == STACK_SIZE) {
					_print_error(input, opcode);
					return -1;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = _join_8bits_to_32bits(bytes);
				type_stack_push(stack, &num);
			}
			break;
			case POP_CODE: {
				int32_t *temp;
				if (type_stack_size(stack) == 0) {
					_print_error(input, opcode);
					return -1;
				}
				temp = type_stack_pop(stack);
				value = *temp;
			}
			break;
			case ADD_CODE: case SUB_CODE: case MUL_CODE: case DIV_CODE: {
				int32_t *temp;
				int32_t x, y;
				if (type_stack_size(stack) <= 1) {
					_print_error(input, opcode);
					return -1;
				}
				temp = type_stack_pop(stack);
				x = *temp;
				temp = type_stack_pop(stack);
				y = *temp;
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
				type_stack_push(stack, &y);
			}
			break;
			case JMP_CODE: {
				uint8_t bytes[4];
				int32_t *temp;
				int32_t num;
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = _join_8bits_to_32bits(bytes);
				if (num < 0) {
					int32_t pos = type_stack_size(stack) + num;
					if (pos < 0) {
						_print_error(input, opcode);
						return -1;
					}
					temp = type_stack_get(stack, pos);
					num = *temp;
				}
				if (num >= fsize) {
					_print_error(input, opcode);
					return -1;
				}
				fseek(input, num, SEEK_SET);
			}
			break;
			case JL_CODE: case JG_CODE: case JE_CODE: case JNE_CODE: {
				uint8_t bytes[4];
				int32_t *temp;
				int32_t num, pos, x, y;
				if (type_stack_size(stack) <= 1) {
					_print_error(input, opcode);
					return -1;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = _join_8bits_to_32bits(bytes);
				if (num < 0) {
					pos = type_stack_size(stack) + num;
					if (pos < 0) {
						_print_error(input, opcode);
						return -1;
					}
					temp = type_stack_get(stack, pos);
					num = *temp;
				}
				if (num >= fsize) {
					_print_error(input, opcode);
					return -1;
				}
				temp = type_stack_pop(stack);
				x = *temp;
				temp = type_stack_pop(stack);
				y = *temp;
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
				uint8_t bytes[8];
				int32_t *temp;
				int32_t num1, num2;
				int32_t pos1, pos2;
				fscanf(input, "%c%c%c%c%c%c%c%c", 
					&bytes[0], &bytes[1], &bytes[2], &bytes[3],
					&bytes[4], &bytes[5], &bytes[6], &bytes[7]);
				num1 = _join_8bits_to_32bits(bytes);
				num2 = _join_8bits_to_32bits(bytes+4);
				if (num1 < 0) {
					pos1 = type_stack_size(stack) + num1;
					if (pos1 < 0) {
						_print_error(input, opcode);
						return -1;
					}
				} else {
					pos1 = num1;
					if (pos1 >= type_stack_size(stack)) {
						_print_error(input, opcode);
						return -1;
					}
				}
				if (num2 < 0) {
					pos2 = type_stack_size(stack) + num2;
					if (pos2 < 0) {
						_print_error(input, opcode);
						return -1;
					}
				} else {
					pos2 = num2;
					if (pos2 >= type_stack_size(stack)) {
						_print_error(input, opcode);
						return -1;
					}
				}
				temp = type_stack_get(stack, pos2);
				num2 = *temp;
				type_stack_set(stack, pos1, &num2);
			}
			break;
			case LOAD_CODE: {
				uint8_t bytes[4];
				int32_t *temp;
				int32_t pos, num;
				if (type_stack_size(stack) == STACK_SIZE) {
					_print_error(input, opcode);
					return -1;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = _join_8bits_to_32bits(bytes);
				if (num < 0) {
					pos = type_stack_size(stack) + num;
					if (pos < 0) {
						_print_error(input, opcode);
						return -1;
					}
				} else {
					pos = num;
					if (pos >= type_stack_size(stack)) {
						_print_error(input, opcode);
						return -1;
					}
				}
				temp = type_stack_get(stack, pos);
				num = *temp;
				type_stack_push(stack, &num);
			}
			break;
			case CALL_CODE: {
				uint8_t bytes[4];
				int32_t *temp;
				int32_t num, pos;
				if (type_stack_size(stack) == STACK_SIZE) {
					_print_error(input, opcode);
					return -1;
				}
				fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
				num = _join_8bits_to_32bits(bytes);
				if (num < 0) {
					int32_t pos = type_stack_size(stack) + num;
					if (pos < 0) {
						_print_error(input, opcode);
						return -1;
					}
					temp = type_stack_get(stack, pos);
					num = *temp;
				}
				if (num >= fsize) {
					_print_error(input, opcode);
					return -1;
				}
				pos = ftell(input);
				type_stack_push(stack, &pos);
				fseek(input, num, SEEK_SET);
			}
			break;
			case RET_CODE: {
				int32_t *temp;
				int32_t num;
				if (type_stack_size(stack) == 0) {
					_print_error(input, opcode);
					return -1;
				}
				temp = type_stack_pop(stack);
				num = *temp;
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
	type_stack_free(stack);
	return (int)value;
}

extern int readvm_src(FILE *output, FILE *input) {
	type_hashtab *hashtab;
	char buffer[BUFSIZ];
	int line_index, curr_pos, err_exist;
	char *arg;
	opcode_t opcode;

	hashtab = type_hashtab_new(250);
	line_index = 0;
	curr_pos = 0;
	err_exist = 0;

	while(fgets(buffer, BUFSIZ, input) != NULL) {
		++line_index;
		arg = _readcode(buffer, &opcode);
		if((opcode == PASS_CODE && _strnull(arg)) || opcode == COMMENT_CODE) {
			continue;
		}
		if (opcode == PASS_CODE) {
			err_exist = 1;
			fprintf(stderr, "syntax error: line %d\n", line_index);
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
				type_hashtab_insert(hashtab, arg, &curr_pos, sizeof(curr_pos));
			break;
			default: ;
		}
	}

	if (err_exist) {
		type_hashtab_free(hashtab);
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
				uint8_t bytes[4];
				int32_t *temp;
				int32_t num;
				int size;
				temp = type_hashtab_select(hashtab, arg, &size);
				if (temp != NULL) {
					num = *temp;
				} else {
					num = atoi(arg);
				}
				_split_32bits_to_8bits(num, bytes);
				fprintf(output, "%c%c%c%c%c", opcode, bytes[0], bytes[1], bytes[2], bytes[3]);
			}
			break;
			case LOAD_CODE: {
				uint8_t bytes[4];
				int32_t num;
				num = atoi(arg);
				_split_32bits_to_8bits(num, bytes);
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

	type_hashtab_free(hashtab);
	return 0;
}

static void _print_error(FILE *input, opcode_t opcode) {
	fprintf(stderr, "Error: %s (0x%02X)\nPosition: %ld bytes\n", opcodelist[opcode], opcode, ftell(input));
}

static char *_readcode(char *line, opcode_t *opcode) {
	char *ptr;

	ptr = line;
	while(isspace(*ptr)) {
		++ptr;
	}
	line = ptr;
	while(!_isspace(*ptr)) {
		++ptr;
	}
	*ptr = '\0';
	*opcode = PASS_CODE;
	for (int i = 0; i < INSTRUCTION_NUM; ++i) {
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
	for (int i = 0; i < 4; ++i) {
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
