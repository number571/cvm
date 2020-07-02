#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "type.h"
#include "stack.h"

typedef struct Stack {
    vtype_t tvalue;
    value_t *buffer;
    size_t size;
    size_t index;
} Stack;

static int8_t _insert_stack(Stack *stack, size_t index, void *value);
static void _free_stack(Stack *stack);

extern Stack *new_stack(size_t size, vtype_t tvalue) {
    switch(tvalue) {
        case DECIMAL_TYPE: case REAL_TYPE: case STRING_TYPE:
        break;
        default:
            fprintf(stderr, "%s\n", "tvalue type not found");
            return NULL;
    }
    Stack *stack = (Stack*)malloc(sizeof(Stack));
    stack->tvalue = tvalue;
    stack->buffer = (value_t*)malloc(sizeof(value_t)*size);
    stack->size = size;
    stack->index = 0;
    return stack;
}

extern void free_stack(Stack *stack) {
    _free_stack(stack);
    free(stack->buffer);
    free(stack);
}

extern size_t size_stack(Stack *stack) {
    return stack->index;
}

extern void set_stack(Stack *stack, size_t index, void *value) {
    _insert_stack(stack, index, value);
}

extern void push_stack(Stack *stack, void *value) {
    if (_insert_stack(stack, stack->index, value) != 0) {
        return;
    }
    stack->index += 1;
}

extern value_t get_stack(Stack *stack, size_t index) {
    if (index >= stack->size) {
        fprintf(stderr, "%s\n", "error: index >= stack size");
        value_t none = {
            .decimal = 0,
        };
        return none;
    }
    return stack->buffer[index];
}

extern value_t pop_stack(Stack *stack) {
    if (stack->index == 0) {
        fprintf(stderr, "%s\n", "error: stack overflow");
        value_t none = {
            .decimal = 0,
        };
        return none;
    }
    stack->index -= 1;
    return stack->buffer[stack->index];
}

static int8_t _insert_stack(Stack *stack, size_t index, void *value) {
    if (index >= stack->size) {
        fprintf(stderr, "%s\n", "error: index >= stack size");
        return 1;
    }
    switch(stack->tvalue) {
        case DECIMAL_TYPE:
            stack->buffer[index].decimal = (int32_t)(intptr_t)value;
        break;
        case REAL_TYPE:
            stack->buffer[index].real = *(double*)value;
            free((double*)value);
        break;
        case STRING_TYPE: {
            size_t size = strlen((char*)value);
            stack->buffer[index].string = (char*)malloc(sizeof(char)*size+1);
            strcpy(stack->buffer[index].string, (char*)value);
        }
        break;
        default: ;
    }
    return 0;
}

static void _free_stack(Stack *stack) {
    switch(stack->tvalue) {
        case STRING_TYPE:
            for (size_t i = 0; i < stack->index; ++i) {
                free(stack->buffer[i].string);
            }
        break;
        default: ;
    }
}
