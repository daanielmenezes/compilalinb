#include <stdio.h>
#include "compilalinb.h"

int teste0s(void);
int main(void) {
    funcp f = teste0s;
    printf("%d\n", f());
    return 0;
}
