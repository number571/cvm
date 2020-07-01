#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "extclib/hashtab.h"
#include "extclib/stack.h"

#define OPERATOR_NUM 20
/*
| Operators:
| 1. push, pop
| 2. add, sub, mul, div
| 3. jmp, je, jne, jl, jg
| 4. load, store
| 5. label, call, ret
| 6. stack, print, hlt, ;
*/

enum {
    PUSH_CODE,
    POP_CODE,
    LABEL_CODE,
    JMP_CODE,
    JL_CODE,
    JG_CODE,
    JE_CODE,
    JNE_CODE,
    ADD_CODE,
    SUB_CODE,
    MUL_CODE,
    DIV_CODE,
    STORE_CODE,
    LOAD_CODE,
    CALL_CODE,
    RET_CODE,
    STACK_CODE,
    PRINT_CODE,
    HLT_CODE,
    COMMENT_CODE,
    PASS_CODE, // code undefined
};

const char *codes[OPERATOR_NUM] = {
    [PUSH_CODE]     = "push",
    [POP_CODE]      = "pop",
    [LABEL_CODE]    = "label",
    [JMP_CODE]      = "jmp",
    [JL_CODE]       = "jl",
    [JG_CODE]       = "jg",
    [JE_CODE]       = "je",
    [JNE_CODE]      = "jne",
    [ADD_CODE]      = "add",
    [SUB_CODE]      = "sub",
    [MUL_CODE]      = "mul",
    [DIV_CODE]      = "div",
    [STORE_CODE]    = "store",
    [LOAD_CODE]     = "load",
    [CALL_CODE]     = "call",
    [RET_CODE]      = "ret",
    [STACK_CODE]    = "stack",
    [PRINT_CODE]    = "print",
    [HLT_CODE]      = "hlt",
    [COMMENT_CODE]  = ";",
};

extern int32_t readvm_src(FILE *output, FILE *input);
extern int32_t readvm_mch(FILE *input);

static char *_readcode(char *line, FILE *file, uint8_t *code);
static _Bool _strnull(char *str);
static _Bool _isspace(char ch);

static int32_t _join_8bits_to_32bits(uint8_t * bytes);
static void _split_32bits_to_8bits(int32_t num, uint8_t * bytes);

extern int32_t readvm_mch(FILE *input) {
    fseek(input, 0, SEEK_END);
    size_t fsize = ftell(input);

    Stack *stack = new_stack(10000, DECIMAL_TYPE);
    fseek(input, 0, SEEK_SET);

    int32_t value = 0;
    uint8_t code;

    // read commands
    while(!feof(input) && ftell(input) != fsize) {
        fscanf(input, "%c", &code);

        switch(code) {
            // push x
            case PUSH_CODE: {
                uint8_t bytes[4];
                fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
                int32_t num = _join_8bits_to_32bits(bytes);
                push_stack(stack, decimal(num));
            }
            break;
            // pop
            case POP_CODE: {
                int32_t num = pop_stack(stack).decimal;
                value = num;
            }
            break;
            // [add|sub|mul|div]
            case ADD_CODE: case SUB_CODE: case MUL_CODE: case DIV_CODE: {
                int32_t x = pop_stack(stack).decimal;
                int32_t y = pop_stack(stack).decimal;
                switch(code) {
                    case ADD_CODE:
                        x += y;
                    break;
                    case SUB_CODE:
                        x -= y;
                    break;
                    case MUL_CODE:
                        x *= y;
                    break;
                    case DIV_CODE:
                        x /= y;
                    break;
                    default: ;
                }
                push_stack(stack, decimal(x));
            }
            break;
            case LABEL_CODE: {
                uint8_t bytes[4];
                fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
                _join_8bits_to_32bits(bytes);
            }
            break;
            // jmp label
            case JMP_CODE: {
                uint8_t bytes[4];
                fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
                int32_t index = _join_8bits_to_32bits(bytes);
                fseek(input, index, SEEK_SET);
            }
            break;
            // push x
            // push y
            // j[l|g|e|ne] label
            case JL_CODE: case JG_CODE: case JE_CODE: case JNE_CODE: {
                uint8_t bytes[4];
                fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
                int32_t index = _join_8bits_to_32bits(bytes);
                int32_t x = pop_stack(stack).decimal;
                int32_t y = pop_stack(stack).decimal;
                switch(code) {
                    case JL_CODE:
                        if (x < y) {
                            fseek(input, index, SEEK_SET);
                        }
                    break;
                    case JG_CODE:
                        if (x > y) {
                            fseek(input, index, SEEK_SET);
                        }
                    break;
                    case JE_CODE:
                        if (x == y) {
                            fseek(input, index, SEEK_SET);
                        }
                    break;
                    case JNE_CODE:
                        if (x != y) {
                            fseek(input, index, SEEK_SET);
                        }
                    break;
                    default: ;
                }
            }
            break;
            // store $x y
            // store $x $y
            // store $x $-y
            // store $-x y
            // store $-x $y
            // store $-x $-y
            case STORE_CODE: {
                int32_t mem1, mem2;
                uint8_t bytes[8];

                fscanf(input, "%c%c%c%c%c%c%c%c", 
                    &bytes[0], &bytes[1], &bytes[2], &bytes[3],
                    &bytes[4], &bytes[5], &bytes[6], &bytes[7]);

                mem1 = _join_8bits_to_32bits(bytes);
                mem2 = _join_8bits_to_32bits(bytes+4);

                if (mem1 < 0) {
                    mem1 = size_stack(stack) + mem1;
                }
                if (mem2 < 0) {
                    mem2 = size_stack(stack) + mem2;
                }
                // printf("%ld\n", size_stack(stack));
                // printf("%d\n", mem1);
                set_stack(stack, mem1, decimal(get_stack(stack, mem2).decimal));
            }
            break;
            // load $x
            // load $-x
            case LOAD_CODE: {
                uint8_t bytes[4];
                fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
                int32_t mem = _join_8bits_to_32bits(bytes);
                if (mem < 0) {
                    mem = size_stack(stack) + mem;
                }
                push_stack(stack, decimal(get_stack(stack, mem).decimal));
            }
            break;
            // call label
            case CALL_CODE: {
                uint8_t bytes[4];
                fscanf(input, "%c%c%c%c", &bytes[0], &bytes[1], &bytes[2], &bytes[3]);
                int32_t index = _join_8bits_to_32bits(bytes);
                // push callback current position for ret
                int32_t currpos = ftell(input);
                push_stack(stack, decimal(currpos));
                // goto label
                fseek(input, index, SEEK_SET);
            }
            break;
            // ret
            case RET_CODE: {
                int32_t index = pop_stack(stack).decimal;
                fseek(input, index, SEEK_SET);
            }
            break;
            // stack
            case STACK_CODE: {
                int32_t num = size_stack(stack);
                push_stack(stack, decimal(num));
            }
            break;
            // print
            case PRINT_CODE: {
                printf("%d\n", get_stack(stack, size_stack(stack)-1).decimal);
            }
            break;
            // hlt
            case HLT_CODE: {
                goto close;
            }
            break;
        }
    }
close:
    free_stack(stack);
    return value;
}

extern int32_t readvm_src(FILE *output, FILE *input) {
    HashTab *hashtab = new_hashtab(250, STRING_TYPE, DECIMAL_TYPE);
    char buffer[BUFSIZ] = {0};

    size_t line_index = 0;
    _Bool err_exist = 0;
    int32_t currpos = 0;

    char *line;
    uint8_t code;

    // read labels, check syntax
    while(fgets(buffer, BUFSIZ, input) != NULL) {
        ++line_index;
        line = _readcode(buffer, input, &code);
        if((code == PASS_CODE && _strnull(line)) || code == COMMENT_CODE) {
            continue;
        }
        if (code == PASS_CODE) {
            err_exist = 1;
            fprintf(stderr, "syntax error: line %ld\n", line_index);
        }
        switch(code) {
            case PUSH_CODE: case JMP_CODE: case JL_CODE: case JG_CODE: case JE_CODE: case JNE_CODE:
            case LOAD_CODE: case CALL_CODE: {
                currpos += 5;
            }
            break;
            case POP_CODE: case ADD_CODE: case SUB_CODE: case MUL_CODE: case DIV_CODE: case RET_CODE: 
            case STACK_CODE: case PRINT_CODE: case HLT_CODE: {
                currpos += 1;
            }
            break;
            case STORE_CODE: {
                currpos += 9;
            }
            break;
            case LABEL_CODE: {
                currpos += 5;
                set_hashtab(hashtab, string(line), decimal(currpos));
            }
            break;
            default: ;
        }
    }
    if (err_exist) {
        free_hashtab(hashtab);
        return -1;
    }

    Stack *stack = new_stack(10000, DECIMAL_TYPE);
    int32_t value = 0;
    fseek(input, 0, SEEK_SET);

    // read commands
    while(fgets(buffer, BUFSIZ, input) != NULL) {
        line = _readcode(buffer, input, &code);

        switch(code) {
            // push x
            case PUSH_CODE: {
                int32_t num = atoi(line);
                uint8_t bytes[4];
                _split_32bits_to_8bits(num, bytes);
                fprintf(output, "%c%c%c%c%c", code, bytes[0], bytes[1], bytes[2], bytes[3]);
            }
            break;
            // pop
            case POP_CODE: {
                fprintf(output, "%c", code);
            }
            break;
            // [add|sub|mul|div]
            case ADD_CODE: case SUB_CODE: case MUL_CODE: case DIV_CODE: {
                fprintf(output, "%c", code);
            }
            break;
            // label x
            case LABEL_CODE: {
                int32_t index = ftell(output);
                uint8_t bytes[4];
                _split_32bits_to_8bits(index, bytes);
                fprintf(output, "%c%c%c%c%c", code, bytes[0], bytes[1], bytes[2], bytes[3]);
            }
            break;
            // jmp label
            case JMP_CODE: {
                int32_t index = get_hashtab(hashtab, line).decimal;
                uint8_t bytes[4];
                _split_32bits_to_8bits(index, bytes);
                fprintf(output, "%c%c%c%c%c", code, bytes[0], bytes[1], bytes[2], bytes[3]);
            }
            break;
            // push x
            // push y
            // j[l|g|e|ne] label
            case JL_CODE: case JG_CODE: case JE_CODE: case JNE_CODE: {
                int32_t index = get_hashtab(hashtab, line).decimal;
                uint8_t bytes[4];
                _split_32bits_to_8bits(index, bytes);
                fprintf(output, "%c%c%c%c%c", code, bytes[0], bytes[1], bytes[2], bytes[3]);
            }
            break;
            // store $x $y
            // store $x $-y
            // store $-x $y
            // store $-x $-y
            case STORE_CODE:
                if (line[0] == '$') {
                    char *arg = line + strlen(line) + 1;
                    while(isspace(*arg)) {
                        ++arg;
                    }
                    char *ptr = arg;
                    while(!isspace(*ptr)) {
                        ++ptr;
                    }
                    *ptr = '\0';
                    int32_t index = 0;
                    if (line[1] == '-') {
                        index = -atoi(line+2);
                    } else {
                        index = atoi(line+1);
                    }
                    int32_t i2 = 0;
                    if (arg[1] == '-') {
                        i2 = -atoi(arg+2);
                    } else {
                        i2 = atoi(arg+1);
                    }
                    uint8_t bytes[8];
                    _split_32bits_to_8bits(index, bytes);
                    _split_32bits_to_8bits(i2, bytes+4);

                    fprintf(output, "%c%c%c%c%c%c%c%c%c", code, 
                        bytes[0], bytes[1], bytes[2], bytes[3], 
                        bytes[4], bytes[5], bytes[6], bytes[7]);
                }
            break;
            // load $x
            // load $-x
            case LOAD_CODE:
                if (line[0] == '$') {
                    int32_t index = 0;
                    if (line[1] == '-') {
                        index = -atoi(line+2);
                    } else {
                        index = atoi(line+1);
                    }
                    uint8_t bytes[4];
                    _split_32bits_to_8bits(index, bytes);
                    fprintf(output, "%c%c%c%c%c", code, bytes[0], bytes[1], bytes[2], bytes[3]);
                }
            break;
            // call label
            case CALL_CODE: {
                int32_t index = get_hashtab(hashtab, line).decimal;
                uint8_t bytes[4];
                _split_32bits_to_8bits(index, bytes);
                fprintf(output, "%c%c%c%c%c", code, bytes[0], bytes[1], bytes[2], bytes[3]);
            }
            break;
            // ret
            case RET_CODE: case STACK_CODE: case PRINT_CODE: case HLT_CODE: {
                fprintf(output, "%c", code);
            }
            default: ;
        }
    }
    free_stack(stack);
    free_hashtab(hashtab);
    return value;
}

static char *_readcode(char *line, FILE *file, uint8_t *code) {
    // pass spaces
    char *ptr = line;
    while(isspace(*ptr)) {
        ++ptr;
    }

    // read operator
    line = ptr;
    while(!_isspace(*ptr)) {
        ++ptr;
    }
    *ptr = '\0';

    // analyze operator
    *code = PASS_CODE;
    for (size_t i = 0; i < OPERATOR_NUM; ++i) {
        if (strcmp(line, codes[i]) == 0) {
            *code = i;
            break;
        }
    }

    // operators without args
    switch(*code) {
        case PASS_CODE:
        case POP_CODE:
        case COMMENT_CODE:
        case RET_CODE:
        case PRINT_CODE:
        case STACK_CODE:
        case HLT_CODE:
            return line;
        default: ;
    }

    // pass spaces after operator
    ++ptr;
    while(isspace(*ptr)) {
        ++ptr;
    }

    // read argument
    line = ptr;
    while(!_isspace(*ptr)) {
        ++ptr;
    }
    *ptr = '\0';

    // return first argument
    return line;
}

static int32_t _join_8bits_to_32bits(uint8_t * bytes) {
    uint64_t num;
    for (uint8_t *p = bytes; p < bytes + 4; ++p) {
        num = (num << 8) | *p;
    }
    return num;
}

static void _split_32bits_to_8bits(int32_t num, uint8_t * bytes) {
    for (uint8_t i = 0; i < 4; ++i) {
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
