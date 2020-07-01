#include <stdio.h>
#include <stdint.h>

extern int8_t readvm_src(FILE *output, FILE *input);
extern int32_t readvm_exc(FILE *input);

int32_t runvm_exc(const char *inputf);
int8_t compilevm_src(const char *outputf, const char *inputf);

int main(int argc, char const *argv[]) {
    compilevm_src("main.vme", "main.vms");
    printf("%d\n", runvm_exc("main.vme"));
    return 0;
}

int32_t runvm_exc(const char *inputf) {
    FILE *input = fopen(inputf, "r");
    if (input == NULL) {
        fprintf(stderr, "%s\n", "error: read input file");
        return 1;
    }
    int32_t res = readvm_exc(input);
    fclose(input);
    return res;
}

int8_t compilevm_src(const char *outputf, const char *inputf) {
    FILE *input = fopen(inputf, "r");
    if (input == NULL) {
        fprintf(stderr, "%s\n", "error: read input file");
        return 1;
    }
    FILE *output = fopen(outputf, "wb");
    if (output == NULL) {
        fprintf(stderr, "%s\n", "error: read output file");
        return 2;
    }
    readvm_src(output, input);
    fclose(input);
    fclose(output);
    return 0;
}
