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

static BYTECODE movl_reg2ra = {
    /* movl %eax, 0(%rbp) */
    0x89,
    0x45,                  /* %eax */
    0x00                   /* shift no rbp */
};

static BYTECODE movl_num2ra = {
    /* movl $0, 0(%rbp) */
    0xc7, 0x45, 
    0x00,                  /* shift do rbp */
    0x00, 0x00, 0x00, 0x00 /* numero inteiro */
};

static BYTECODE addl_num2ra = {
    /* addl $0, 0(%rbp) */
    0x81, 0x45, 
    0x00,                  /* shift do rbp */
    0x00, 0x00, 0x00, 0x00 /* numero inteiro */
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

static CURSOR grava_atribuicao(CURSOR cursor, FILE **f) {
    int idx0, idx1, idx2;
    char var0, var1, var2, op;  
    CURSOR end;
    int * ptrInt;
    if (fscanf(*f, "%c%d = %c%d %c %c%d", &var0,  &idx0, &var1, &idx1, 
                &op, &var2, &idx2) != 7)
        return NULL;

    if ( var0 == 'v' && op == '+' && var1 == '$' && var2 == '$' ){
        end = grava_bytes(cursor, movl_num2ra, sizeof movl_num2ra); 
        cursor[2] = V(idx0); /* shift do rbp */
        ptrInt = (int*) &cursor[3];
        *ptrInt = idx1;

        cursor = end;
        end = grava_bytes(cursor, addl_num2ra, sizeof addl_num2ra);
        cursor[2] = V(idx0); /* shift do rbp */
        ptrInt =  (int*) &cursor[3];
        *ptrInt = idx2;
    }
    /*printf("%c%d = %c%d %c %c%d\n",*/
            /*var0, idx0, var1, idx1, op, var2, idx2);*/

    return end;
}

funcp CompilaLinB (FILE *f) {
    int c, line;
    CURSOR cursor;
    funcp funcao = (funcp) malloc(10000);
    cursor = grava_bytes((CURSOR) funcao, prologo, sizeof prologo);

    for (line = 1;(c=fgetc(f)) != EOF; line++){
        switch (c) {
            case 'r': { /* retorno */
                char c0;
                if (fscanf(f, "et%c", &c0) != 1)
                    error("comando invalido", line);
                /*printf("ret\n");*/
                break;
            }
            case 'v':
            case 'p':{ /* atribuicao */
                ungetc(c, f);
                cursor = grava_atribuicao(cursor, &f);
                if (!cursor)
                    error("comando invalido", line);
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

    grava_bytes(cursor, finalizacao, sizeof finalizacao );
    return funcao;
}

void LiberaFuncao (void *p){
    free(p);
}
