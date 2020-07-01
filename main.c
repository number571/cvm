#include <stdio.h>
#include <stdint.h>

extern int32_t readvm_src(FILE *output, FILE *input);
extern int32_t readvm_mch(FILE *input);

int32_t run_mch(const char *inputf);
int8_t compilevm_src(const char *outputf, const char *inputf);

int main(int argc, char const *argv[]) {
    compilevm_src("main.vmm", "main.vms");
    printf("%d\n", run_mch("main.vmm"));
    return 0;
}

int32_t run_mch(const char *inputf) {
    FILE *input = fopen(inputf, "r");
    if (input == NULL) {
        fprintf(stderr, "%s\n", "error: read input file");
        return 1;
    }
    int32_t res = readvm_mch(input);
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
