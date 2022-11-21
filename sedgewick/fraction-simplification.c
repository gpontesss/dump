#include <stdio.h>

typedef unsigned int uint;
typedef struct { uint num; uint den; } frac;

#define MAX_UINT (uint)(-1)

/* Calcultates the greatest common divisor between to numbers using Euclid's
 * algorithm. (Refer to his work, "Elements", for details.) */
uint gcd(uint u, uint v) {
    uint t;
    while (u != 0) {
        if (u < v) { t = u; u = v; v = t; }
        u = u - v;
    }
    return v;
}

void print_frac(frac *x) {
    printf("%d/%d\n", x->num, x->den);
}

void reduce_frac(frac *x) {
    uint div = gcd(x->num, x->den);
    (*x).num = x->num / div;
    (*x).den = x->den / div;
}

int main() {
    frac x = {6, 4};
    print_frac(&x);
    reduce_frac(&x);
    print_frac(&x);

    printf("Greatest uint: %u\n", MAX_UINT);
    uint pair[2] = {MAX_UINT, MAX_UINT-1};
    while(gcd(pair[0], pair[1]) != 1) {
        pair[0] = pair[1];
        pair[1] = pair[0] - 1;
    }

    printf("Greatest pair with GCD = 1: %u, %u\n", pair[0], pair[1]);
    return 0;
}
