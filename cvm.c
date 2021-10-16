#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cvmkernel.h"

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

    retcode = ERR_COMMAND;
    outfile = CVM_OUTFILE;

    if (argc < 3) {
        return ERR_ARGLEN;
    }

    if (strcmp(argv[1], CVM_BUILD) == 0) {
        if (argc >= 5 && strcmp(argv[3], "-o") == 0) {
            outfile = argv[4];
        }
        retcode = file_compile(outfile, argv[2]);
    }

    if (strcmp(argv[1], CVM_RUN) == 0) {
        input[0] = argc-3;
        for (int i = 0; i < argc-3; ++i) {
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
