#include <stdlib.h>
#include <assert.h>
#include "compilalinb.h"


FILE *try_open(const char *filename) {
    FILE *fileptr;
    if ( (fileptr = fopen(filename, "r")) ){
        return fileptr;
    }
    else{
        fprintf(stderr, "Erro: Não foi possível abrir o arquivo \"%s\"\n",
                    filename);
        exit(EXIT_FAILURE);
    }
}

int main( void ) {
    FILE *file;
    funcp func;
    char *strRodando = "Rodando teste %d: ";

    /* teste 0 (teste0.linb) */
    file = try_open("teste0.linb");
    printf(strRodando, 0);
    func = CompilaLinB(file);
    fclose(file);
    assert(func != NULL);
    assert(func() == 1);
    printf("OK.\n");


    /* teste 1 (teste1.linb) */
    file = try_open("teste1.linb");
    printf(strRodando, 1);
    func = CompilaLinB(file);
    fclose(file);
    assert(func != NULL);
    assert(func(-20) == -19);
    assert(func(314) == 315);
    assert(func(60) == 61);
    assert(func(10) == 11);
    assert(func(-40) == -39);
    printf("OK.\n");



    /* teste 2 (teste2.linb) */
    file = try_open("teste2.linb");
    printf(strRodando, 2);
    func = CompilaLinB(file);
    fclose(file);
    assert(func != NULL);
    assert(func(1,1) == 0);
    assert(func(1,2) == -3);
    assert(func(-1,1) == 0);
    assert(func(-12,13) == -25);
    assert(func(25,31) == -336);
    assert(func(3,1) == 8);
    printf("OK.\n");

    return 0;
}
