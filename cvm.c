#include <stdio.h>
#include <string.h>

#define ERROR_NUM 6

typedef enum error_t {
    NONE_ERR,
    ARGLEN_ERR,
    COMMAND_ERR,
    INOPEN_ERR,
    OUTOPEN_ERR,
    COMPILE_ERR,
} error_t;

static const char *errors[ERROR_NUM] = {
    [NONE_ERR] = "",
    [ARGLEN_ERR] = "len argc < 3",
    [COMMAND_ERR] = "unknown command",
    [INOPEN_ERR] = "open input file",
    [OUTOPEN_ERR] = "open output file",
    [COMPILE_ERR] = "compile file",
};

extern int readvm_src(FILE *output, FILE *input);
extern int readvm_exc(FILE *input);

static char *help(void);
static int compilevm_src(const char *outputf, const char *inputf);
static int runvm_exc(const char *filename, int *retcode);

int main(int argc, char const *argv[]) {
    int retcode;

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
        printf("Return code: %d\n", runvm_exc(argv[2], &retcode));
        goto close;
    } 

    retcode = COMMAND_ERR;

close:
    if (retcode != NONE_ERR) {
        fprintf(stderr, "> %s\n%s", errors[retcode], help());
    }

    return retcode;
}

static char *help(void) {
    return \
    "BEGIN _Help_info_\n" \
    "\t1. Compile:\n" \
    "\t\t$ cvm build main.vms\n" \
    "\t2. Run bytecode:\n" \
    "\t\t$ cvm run main.vme\n" \
    "END _Help_info_\n";
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
    if (res != 0) {
        return COMPILE_ERR;
    }
    return NONE_ERR;
}

static int runvm_exc(const char *filename, int *retcode) {
    FILE *input = fopen(filename, "r");
    if (input == NULL) {
        *retcode = INOPEN_ERR;
        return 0;
    }
    int res = readvm_exc(input);
    fclose(input);
    return res;
}
