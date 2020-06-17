/* Daniel Menezes C. L. dos Passos 1913037  3WB */
/* Theo Falcato R. P. de Athayde   1912971 3WB */

/*  PARAMETROS:
 *  p1 ->  %edi
 *  p2 ->  %esi
 */

#include <stdlib.h>
#include <string.h>
#include "compilalinb.h"


/* Macro para o shift da variavel no registro de ativacao 
 *  v1 ->  -16(%rbp)
 *  v2 ->  -12(%rbp)
 *  v3 ->  -8(%rbp)
 *  v4 ->  -4(%rbp)
 */
#define V(x) (x)*4-20

typedef const unsigned char BYTECODE[];
typedef unsigned char * CURSOR;
typedef union {
    char c;
    int i;
} *MASK;

 /**************************************************
 *                                                 *
 *        INSTRUÇÕES EM CÓDIGO DE MÁQUINA          *
 *                                                 *
 **************************************************/

static BYTECODE prologo = {
    0x55,                   /* push %bp        */
    0x48, 0x89, 0xe5,       /* movq %rsp, %rbp */
    0x48, 0x83, 0xec, 0x10  /* subq $16, %rsp  */
};

static BYTECODE finalizacao = {
    0x8b, 0x45, 0xf0,      /* movl -16(%rbp), %eax */
    0xc9,                  /* leave */
    0xc3                   /* ret */ 
};

 /******************************************************
 *                                                     *
 *               FUNÇÕES AUXILIARES                    *
 *                                                     *
 ******************************************************/
static void error (const char *msg, int line) {
    fprintf(stderr, "erro %s na linha %d\n", msg, line);
    exit(EXIT_FAILURE);
}

static CURSOR grava_bytes(CURSOR cursor, BYTECODE source, int size) {
    while(size--) {
        *cursor++ = *source++;
    }
    return cursor;
}

static CURSOR mov2eax(CURSOR cursor, char var1, int idx1) {
    CURSOR end = cursor;
    int * ptrInt;
    switch (var1) {
        case '$':
            *(end++) = 0xb8;
            ptrInt = (int *) end;
            *ptrInt = idx1;
            end += 4;
            break;
        case 'v':
            *(end++) = 0x8b;
            *(end++) = 0x45;
            *(end++) = V(idx1);
            end += 3;
            break;
        case 'p':
            *(end++) = 0x89;
            if (idx1 == 1) {
                *(end++) = 0xf8;
                break;
            }
            else if (idx1 == 2){
                *(end++) = 0xf0;
                break;
            }
        default:
            return NULL;
    }
    return end;
}
/* FUNÇÕES DE CALCULO.
 * Fazem a operação em cima do valor em eax.
 * 
 * 3 casos:  
 * var2 == $
 * var2 == v
 * var2 == p
 */
static CURSOR add2eax(CURSOR cursor, char var2, int id2) {
    CURSOR end = cursor;
    int * ptrInt;
    if (var2 == '$' ){
        *(end++) = 0x5;
        ptrInt = (int*)end;
        *ptrInt = id2;
        end += 4;
    }
    else 
        end = NULL;

    return end;
}

static CURSOR sub2eax(CURSOR cursor, char var2, int id2) {
    /* implementar */
    return NULL;
}

static CURSOR imul2eax(CURSOR cursor, char var2, int id2) {
    /* implementar */
    return NULL;
}


static CURSOR grava_atribuicao(const CURSOR cursor, FILE **f) {
    int idx0, idx1, idx2;
    char var0, var1, var2, op;  
    CURSOR beginning, end;
    beginning = end = cursor;
    int * ptrInt;
    if (fscanf(*f, "%c%d = %c%d %c %c%d", &var0,  &idx0, &var1, &idx1, 
                &op, &var2, &idx2) != 7)
        return NULL;

    end = mov2eax(end, var1, idx1);
    switch (op) {
        case '+':
            end = add2eax(end, var2, idx2);
            break;
        case '*':
            end = imul2eax(end, var2, idx2);
            break;
        case '-':
            end = sub2eax(end, var2, idx2);
            break;
        default:
            return NULL;
    } 

    /* move o resultado em eax para o local certo: */
    if (var0 == 'v'){
        *(end++) = 0x89;
        *(end++) = 0x45;
        *(end++) = V(idx0);
    }
    else if (var0 == 'p') {
        *(end++) = 0x89;
        if (idx0 == 1){ 
            /* (p1) */
            *(end++) = 0xc7;  /* edi = eax */
        }
        else{           
            /* (p2) */
            *(end++) = 0xc6;  /* esi = eax */
        }
    }

    /*printf("%c%d = %c%d %c %c%d\n",*/
            /*var0, idx0, var1, idx1, op, var2, idx2);*/

    return end;
}




 /******************************************************
 *                                                     *
 *               FUNÇÕES PRINCIPAIS                    *
 *                                                     *
 ******************************************************/

funcp CompilaLinB (FILE *f) {
    int c, line;
    CURSOR cursor;
    funcp funcao = (funcp) malloc(10000);
    cursor = grava_bytes((CURSOR) funcao, prologo, sizeof prologo);

    for (line = 1;(c=fgetc(f)) != EOF; line++){
        switch (c) {
            case 'r': { /* retorno */
                char c0;
                if (fscanf(f, "et%c", &c0) != 1){
                    free(funcao);
                    error("comando invalido", line);
                }
                grava_bytes(cursor, finalizacao, sizeof finalizacao );
                return funcao;
            }
            case 'v':
            case 'p':{ /* atribuicao */
                ungetc(c, f);
                cursor = grava_atribuicao(cursor, &f);
                if (!cursor){
                    free(funcao);
                    error("comando invalido", line);
                }
                break;
            }
            case ' ':
            case '\n':
            case '\t':
                break;
            default:
                error("comando desconhecido", line);
        }
    }

    return NULL;
}

void LiberaFuncao (void *p){
    free(p);
}
