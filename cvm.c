#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cvmkernel.h"

#define CVM_HELP    "help"
#define CVM_RUN     "run"
#define CVM_BUILD   "build"
#define CVM_OUTFILE "main.vme"

enum {
    ERR_NONE    = 0x00,
    ERR_ARGLEN  = 0x01,
    ERR_COMMAND = 0x02,
    ERR_INOPEN  = 0x03,
    ERR_OUTOPEN = 0x04,
    ERR_COMPILE = 0x05,
    ERR_MEMSIZ  = 0x06,
    ERR_RUN     = 0x07,
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

static void print_json_failed(int retcode);
static void print_json_success(int *array, int size);

int main(int argc, char const *argv[]) {
    const char *outfile;

    int input[argc];
    int *output;
    int retcode;

    int is_build;
    int is_run;

    outfile = CVM_OUTFILE;
    retcode = ERR_COMMAND;

    // cvm help
    if (argc == 2 && strcmp(argv[1], CVM_HELP) == 0) {
        printf("help: \n\t$ cvm [build|run] file\n");
        return ERR_NONE;
    }

    // cvm | cvm undefined
    if (argc < 3) {
        fprintf(stderr, "error: %s\n", errors[ERR_ARGLEN]);
        return ERR_ARGLEN;
    }

    is_build = strcmp(argv[1], CVM_BUILD) == 0;
    is_run = strcmp(argv[1], CVM_RUN) == 0;

    // cvm undefined x
    if (!is_build && !is_run) {
        fprintf(stderr, "error: %s\n", errors[ERR_COMMAND]);
        return ERR_COMMAND;
    }

    // cvm build file [-o outfile]
    if (is_build) {
        if (argc == 5 && strcmp(argv[3], "-o") == 0) {
            outfile = argv[4];
        }

        retcode = file_compile(outfile, argv[2]);
        if (retcode != ERR_NONE) {
            fprintf(stderr, "error: %s\n", errors[retcode]);
        }
    }

    // cvm run file [args]
    if (is_run) {
        input[0] = argc-3;
        for (int i = 0; i < input[0]; ++i) {
            input[i+1] = atoi(argv[i+3]);
        }

    	retcode = file_run(argv[2], &output, input);
    	if (retcode == ERR_NONE) {
            print_json_success(output+1, output[0]);
            free(output);
    	} else {
            print_json_failed(retcode);
        }
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
    if (output == NULL) {
        fclose(input);
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

static int file_run(const char *inputf, int **output, int *input) {
    unsigned char *memory;
    int fsize, retcode;
    FILE *reader;

    reader = fopen(inputf, "rb");
    if (reader == NULL) {
        return ERR_INOPEN;
    }

    // read size of code
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

static void print_json_failed(int retcode) {
    // begin object
    printf("{\n");

    // error:string
    printf("\t\"error\": \"%s\",\n", errors[retcode]);

    // return:int
    printf("\t\"return\": %d\n", retcode);

    // end object
    printf("}\n");
}

static void print_json_success(int *array, int size) {
    // begin object
    printf("{\n");

    // result:array
    printf("\t\"result\": [");
    for (int i = 0; i < size; ++i) {
        printf("%d%c", array[i], (i == size-1) ? '\0' : ',');
    }
    printf("],\n");

    // return:int
    printf("\t\"return\": 0\n");

    // end object
    printf("}\n");
}
