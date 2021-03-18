#include <stdio.h>
#include <string.h>

#define ERROR_NUM 7

typedef enum error_t {
    NONE_ERR,
    ARGLEN_ERR,
    COMMAND_ERR,
    INOPEN_ERR,
    OUTOPEN_ERR,
    COMPILE_ERR,
    EXEC_ERR,
} error_t;

static const char *errors[ERROR_NUM] = {
    [NONE_ERR]    = "",
    [ARGLEN_ERR]  = "len argc < 3",
    [COMMAND_ERR] = "unknown command",
    [INOPEN_ERR]  = "open input file",
    [OUTOPEN_ERR] = "open output file",
    [COMPILE_ERR] = "compile file",
    [EXEC_ERR]    = "exec algorithm",
};

extern int readvm_src(FILE *output, FILE *input);
extern int readvm_exc(FILE *input, int *result);

static int compilevm_src(const char *outputf, const char *inputf);
static int runvm_exc(const char *filename, int *retcode);

int main(int argc, char const *argv[]) {
    int result, retcode;

    retcode = NONE_ERR;
    if (argc < 3) {
        retcode = ARGLEN_ERR;
        goto close;
    } 

    if (strcmp(argv[1], "build") == 0) {
        if (argc >= 5 && strcmp(argv[3], "-o") == 0) {
            retcode = compilevm_src(argv[4], argv[2]);
        } else {
            retcode = compilevm_src("main.vme", argv[2]);
        }
        goto close;
    } 

    if (strcmp(argv[1], "run") == 0) {
    	retcode = runvm_exc(argv[2], &result);
    	if (retcode != NONE_ERR) {
    		retcode = EXEC_ERR;
    	}
        printf("result exec: %d\n", result);
        goto close;
    } 

    retcode = COMMAND_ERR;

close:
    if (retcode != NONE_ERR) {
        fprintf(stderr, "> %s\n", errors[retcode]);
    }
    return retcode;
}

static int compilevm_src(const char *outputf, const char *inputf) {
    FILE *input = fopen(inputf, "r");
    if (input == NULL) {
        return INOPEN_ERR;
    }
    FILE *output = fopen(outputf, "wb");
    if (input == NULL) {
        return OUTOPEN_ERR;
    }
    int res = readvm_src(output, input);
    fclose(input);
    fclose(output);
    if (res != NONE_ERR) {
        return COMPILE_ERR;
    }
    return NONE_ERR;
}

static int runvm_exc(const char *filename, int *result) {
    FILE *input = fopen(filename, "rb");
    if (input == NULL) {
        return INOPEN_ERR;
    }
    int retcode = readvm_exc(input, result);
    fclose(input);
    return retcode;
}
