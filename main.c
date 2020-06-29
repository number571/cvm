#include "extclib/bigint.h"

extern BigInt *open_sm(const char *filename);

int main(int argc, char const *argv[]) {
    BigInt *res = open_sm(argv[1]);
    if (res == NULL) {
        return 1;
    }
    println_bigint(res);
    free_bigint(res);
    return 0;
}
