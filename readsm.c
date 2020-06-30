#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "extclib/bigint.h"
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

typedef enum opcode_t {
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
} opcode_t;

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

extern BigInt *open_sm(const char *filename);
extern BigInt *read_sm(FILE *file);

static char *_readcode(char *line, FILE *file, opcode_t *code);
static _Bool _strnull(char *str);
static _Bool _isspace(char ch);

extern BigInt *open_sm(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "%s\n", "error: read file");
        return NULL;
    }
    BigInt *result = read_sm(file);
    fclose(file);
    return result;
}

extern BigInt *read_sm(FILE *file) {
    HashTab *hashtab = new_hashtab(250, STRING_TYPE, DECIMAL_TYPE);
    char buffer[BUFSIZ] = {0};
    size_t line_index = 0;
    _Bool err_exist = 0;
    char *line;
    opcode_t code;

    // read labels, check syntax
    while(fgets(buffer, BUFSIZ, file) != NULL) {
        ++line_index;
        line = _readcode(buffer, file, &code);
        if((code == PASS_CODE && _strnull(line)) || code == COMMENT_CODE) {
            continue;
        }
        if (code == PASS_CODE) {
            err_exist = 1;
            fprintf(stderr, "syntax error: line %ld\n", line_index);
        }
        switch(code) {
            case LABEL_CODE:
                set_hashtab(hashtab, string(line), decimal((int32_t)ftell(file)));
            break;
            default: ;
        }
    }
    if (err_exist) {
        free_hashtab(hashtab);
        return NULL;
    }

    Stack *stack = new_stack(10000, BIGINT_TYPE);
    BigInt *value = new_bigint("0");
    fseek(file, 0, SEEK_SET);

    // read commands
    while(fgets(buffer, BUFSIZ, file) != NULL) {
        line = _readcode(buffer, file, &code);
        switch(code) {
            // push x
            case PUSH_CODE: {
                BigInt *num = new_bigint(line);
                push_stack(stack, num);
            }
            break;
            // pop
            case POP_CODE: {
                BigInt *num = pop_stack(stack).bigint;
                cpy_bigint(value, num);
                free_bigint(num);
            }
            break;
            // [add|sub|mul|div]
            case ADD_CODE: case SUB_CODE: case MUL_CODE: case DIV_CODE: {
                BigInt *x = pop_stack(stack).bigint;
                BigInt *y = pop_stack(stack).bigint;
                switch(code) {
                    case ADD_CODE:
                        add_bigint(x, x, y);
                    break;
                    case SUB_CODE:
                        sub_bigint(x, x, y);
                    break;
                    case MUL_CODE:
                        mul_bigint(x, x, y);
                    break;
                    case DIV_CODE:
                        div_bigint(x, x, y);
                    break;
                    default: ;
                }
                push_stack(stack, x);
                free_bigint(y);
            }
            break;
            // jmp label
            case JMP_CODE: {
                int32_t index = get_hashtab(hashtab, line).decimal;
                fseek(file, index, SEEK_SET);
            }
            break;
            // push x
            // push y
            // j[l|g|e|ne] label
            case JL_CODE: case JG_CODE: case JE_CODE: case JNE_CODE: {
                int32_t index = get_hashtab(hashtab, line).decimal;
                BigInt *x = pop_stack(stack).bigint;
                BigInt *y = pop_stack(stack).bigint;
                switch(code) {
                    case JL_CODE:
                        if (cmp_bigint(x, y) < 0) {
                            fseek(file, index, SEEK_SET);
                        }
                    break;
                    case JG_CODE:
                        if (cmp_bigint(x, y) > 0) {
                            fseek(file, index, SEEK_SET);
                        }
                    break;
                    case JE_CODE:
                        if (cmp_bigint(x, y) == 0) {
                            fseek(file, index, SEEK_SET);
                        }
                    break;
                    case JNE_CODE:
                        if (cmp_bigint(x, y) != 0) {
                            fseek(file, index, SEEK_SET);
                        }
                    break;
                    default: ;
                }
                free_bigint(y);
                free_bigint(x);
            }
            break;
            // store $x y
            // store $x $y
            // store $x $-y
            // store $-x y
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
                    size_t index = 0;
                    if (line[1] == '-') {
                        index = size_stack(stack) - atoi(line+2);
                    } else {
                        index = atoi(line+1);
                    }
                    BigInt *x = get_stack(stack, index).bigint;
                    if (arg[0] == '$') {
                        size_t i2 = 0;
                        if (arg[1] == '-') {
                            i2 = size_stack(stack) - atoi(arg+2);
                        } else {
                            i2 = (int32_t)atoi(arg+1);
                        }
                        BigInt *num = dup_bigint(get_stack(stack, i2).bigint);
                        set_stack(stack, index, num);
                        free_bigint(x);
                        break;
                    }
                    BigInt *num = new_bigint(arg);
                    set_stack(stack, index, num);
                    free_bigint(x);
                }
            break;
            // load $x
            // load $-x
            case LOAD_CODE:
                if (line[0] == '$') {
                    size_t index = 0;
                    if (line[1] == '-') {
                        index = size_stack(stack) - atoi(line+2);
                    } else {
                        index = atoi(line+1);
                    }
                    BigInt *num = dup_bigint(get_stack(stack, index).bigint);
                    push_stack(stack, num);
                }
            break;
            // call label
            case CALL_CODE: {
                BigInt *num = new_bigint("0");
                cpynum_bigint(num, (uint32_t)ftell(file));
                push_stack(stack, num);
                int32_t index = get_hashtab(hashtab, line).decimal;
                fseek(file, index, SEEK_SET);
            }
            break;
            // ret
            case RET_CODE: {
                BigInt *num = pop_stack(stack).bigint;
                fseek(file, getnum_bigint(num), SEEK_SET);
                free_bigint(num);
            }
            break;
            // stack
            case STACK_CODE: {
                BigInt *num = new_bigint("0");
                cpynum_bigint(num, (uint32_t)size_stack(stack));
                push_stack(stack, num);
            }
            break;
            // print
            case PRINT_CODE: {
                println_bigint(get_stack(stack, size_stack(stack)-1).bigint);
            }
            break;
            // hlt
            case HLT_CODE: {
                goto close;
            }
            break;
            default: ;
        }
    }
close:
    free_stack(stack);
    free_hashtab(hashtab);
    return value;
}

static char *_readcode(char *line, FILE *file, opcode_t *code) {
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
