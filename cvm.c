#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cvmkernel.h"

enum {
    ERR_NONE,
    ERR_ARGLEN,
    ERR_COMMAND,
    ERR_INOPEN,
    ERR_OUTOPEN,
    ERR_COMPILE,
    ERR_MEMSIZ,
    ERR_RUN,
};

static const char *errors[] = {
    [ERR_NONE]    = "",
    [ERR_ARGLEN]  = "len argc < 3",
    [ERR_COMMAND] = "unknown command",
    [ERR_INOPEN]  = "open input file",
    [ERR_OUTOPEN] = "open output file",
    [ERR_COMPILE] = "compile code",
    [ERR_MEMSIZ]  = "memory size overflow",
    [ERR_RUN]     = "run byte code",
};

static int file_compile(const char *outputf, const char *inputf);
static int file_run(const char *filename, int **output, int *input);

static void print_array(int *array, int size);

int main(int argc, char const *argv[]) {
    int input[argc];
    int *output;
    int retcode;

    retcode = ERR_NONE;
    if (argc < 3) {
        retcode = ERR_ARGLEN;
        goto close;
    } 

    if (strcmp(argv[1], "build") == 0) {
        if (argc >= 5 && strcmp(argv[3], "-o") == 0) {
            retcode = file_compile(argv[4], argv[2]);
        } else {
            retcode = file_compile("main.vme", argv[2]);
        }
        goto close;
    } 

    if (strcmp(argv[1], "run") == 0) {
        input[0] = argc-3;
        for (int i = 0; i < argc-3; ++i) {
            input[i+1] = atoi(argv[i+3]);
        }
    	retcode = file_run(argv[2], &output, input);
    	if (retcode != ERR_NONE) {
            goto close;
    	}
        print_array(output+1, output[0]);
        free(output);
        goto close;
    } 

    retcode = ERR_COMMAND;

close:
    if (retcode != ERR_NONE) {
        fprintf(stderr, "error: %s\n", errors[retcode]);
    }
    return retcode;
}

static int file_compile(const char *outputf, const char *inputf) {
    FILE *output, *input;
    int retcode;
    input = fopen(inputf, "r");
    if (input == NULL) {
        return ERR_INOPEN;
    }
    output = fopen(outputf, "wb");
    if (input == NULL) {
        return ERR_OUTOPEN;
    }
    retcode = cvm_compile(output, input);
    fclose(input);
    fclose(output);
    if (retcode != ERR_NONE) {
        return ERR_COMPILE;
    }
    return ERR_NONE;
}

static int file_run(const char *filename, int **output, int *input) {
    unsigned char *memory;
    FILE *reader;
    int fsize, retcode;
    reader = fopen(filename, "rb");
    if (reader == NULL) {
        return ERR_INOPEN;
    }
    // read len of code
    fseek(reader, 0, SEEK_END);
    fsize = ftell(reader);
    fseek(reader, 0, SEEK_SET);
    // insert code into memory
    memory = (unsigned char*)malloc(sizeof(char)*fsize);
    fread(memory, fsize, sizeof(char), reader);
    fclose(reader);
    fsize = cvm_load(memory, fsize);
    free(memory);
    if (fsize < 0) {
        return ERR_MEMSIZ;
    }
    // run code in memory
    retcode = cvm_run(output, input);
    if (retcode != ERR_NONE) {
        return ERR_RUN;
    }
    return ERR_NONE;
}

static void print_array(int *array, int size) {
    printf("[ ");
    for (int i = 0; i < size; ++i) {
        printf("%d ", array[i]);
    }
    printf("]\n");
}
