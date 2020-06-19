/* Daniel Menezes C. L. dos Passos 1913037  3WB */
/* Theo Falcato R. P. de Athayde   1912971 3WB */

/*  PARAMETROS:
 *  p1 ->  %edi
 *  p2 ->  %esi
 */

#include <stdlib.h>
#include <string.h>
#include "compilalinb.h"

#define MAX_LINB_LINES 1000

/* Macro para o shift da variavel no registro de ativacao 
 *  v1 ->  -16(%rbp)
 *  v2 ->  -12(%rbp)
 *  v3 ->  -8(%rbp)
 *  v4 ->  -4(%rbp)
 */
#define V(x) (x)*4-20

typedef const unsigned char BYTECODE[];
typedef unsigned char * CURSOR;

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

static BYTECODE retorno = {
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

static CURSOR mov_varpc2eax(CURSOR cursor, char var1, int idx1) {
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
        ptrInt = (int*) end;
        *ptrInt = id2;
        end += 4;
    }
    else if ( var2 == 'v'){
        *(end++) = 0x03;
        *(end++) = 0x45;
        *(end++) = V(id2);
    }
    else if (var2 == 'p'){
        *(end++) = 0x01;
        if (id2 == 1) {
            *(end++) = 0xf8;
        }
        else if (id2 == 2) {
            *(end++) = 0xf0;
        }
        else
            end = NULL;
    }
    else 
        end = NULL;

    return end;
}

static CURSOR sub2eax(CURSOR cursor, char var2, int id2) {
    CURSOR end = cursor;
    int * ptrInt;
    if (var2 == '$' ){
        *(end++) = 0x2d;
        ptrInt = (int*) end;
        *ptrInt = id2;
        end += 4;
    }
    else if ( var2 == 'v'){
        *(end++) = 0x2b;
        *(end++) = 0x45;
        *(end++) = V(id2);
    }
    else if (var2 == 'p'){
        *(end++) = 0x29;
        if (id2 == 1) {
            *(end++) = 0xf8;
        }
        else if (id2 == 2) {
            *(end++) = 0xf0;
        }
        else
            end = NULL;
    }
    else 
        end = NULL;

    return end;
}

static CURSOR imul2eax(CURSOR cursor, char var2, int id2) {
    CURSOR end = cursor;
    int * ptrInt;
    if (var2 == '$' ){
        *(end++) = 0x69;
        *(end++) = 0xc0;
        ptrInt = (int*) end;
        *ptrInt = id2;
        end += 4;
    }
    else if ( var2 == 'v'){
        *(end++) = 0x0f;
        *(end++) = 0xaf;
        *(end++) = 0x45;
        *(end++) = V(id2);
    }
    else if (var2 == 'p'){
        *(end++) = 0x0f;
        *(end++) = 0xaf;
        if (id2 == 1) {
            *(end++) = 0xc7;
        }
        else if (id2 == 2) {
            *(end++) = 0xc6;
        }
        else
            end = NULL;
    }
    else 
        end = NULL;

    return end;
}

static CURSOR mov_eax2varp(CURSOR cursor, char var0, int idx0) {
    CURSOR end = cursor;
    if (var0 == 'v'){
        *(end++) = 0x89;
        *(end++) = 0x45;
        *(end++) = V(idx0);
    }
    else if (var0 == 'p') {
        *(end++) = 0x89;
        if (idx0 == 1){ 
            *(end++) = 0xc7;
        }
        else{           
            *(end++) = 0xc6;
        }
    }
    return end;
}

static CURSOR grava_atribuicao(const CURSOR cursor, FILE **f) {
    int idx0, idx1, idx2;
    char var0, var1, var2, op;  
    CURSOR end = cursor;
    if (fscanf(*f, "%c%d = %c%d %c %c%d", &var0,  &idx0, &var1, &idx1, 
                &op, &var2, &idx2) != 7)
        return NULL;

    /* move o primeiro operando para o eax */
    end = mov_varpc2eax(end, var1, idx1);

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
    if (!end) return NULL;
    /* atribui o resultado em eax para o varp dado: */
    end = mov_eax2varp(end, var0, idx0);

    return end;
}

static CURSOR grava_if(CURSOR cursor, FILE **f) {
    char varp;
    int id, line, *intptr;
    CURSOR end = cursor;
    if (fscanf(*f, "f %c%d %d", &varp, &id, &line) != 3){
        return NULL; 
    }

    /* write cmp $0, <varp> */
    *(end++) = 0x83;
    if (varp == 'v'){
        *(end++) = 0x7d;
        *(end++) = V(id);
    }
    else if (varp == 'p' && id == 1) {
        *(end++) = 0xff;
    }
    else if ( varp == 'p' && id == 2 ) {
        *(end++) = 0xfe;
    }
    else {
        return NULL;
    }
    *(end++) = 0x00;
    
    /* write je <line> */
    /* line will be translated to offset later */
    *(end++) = 0x0f;
    *(end++) = 0x84;
    intptr = (int*)end;
    *intptr = line;
    end += 4;
    return end;
}

void write_offsets(funcp funcao, void *lineAdress[MAX_LINB_LINES], int lines) {
    int i, *ptrInt;
    CURSOR cursor = (CURSOR)funcao;
    for (i = 0; i<lines; i++) {
        cursor = lineAdress[i];
        if (cursor[0] == 0x0f && cursor[1] == 0x84) {
            ptrInt = (int*)&cursor[2];
            *ptrInt = lineAdress[*ptrInt - 1] - (void*)(&cursor[6]); 
        }
    }
}

 /******************************************************
 *                                                     *
 *               FUNÇÕES PRINCIPAIS                    *
 *                                                     *
 ******************************************************/

funcp CompilaLinB (FILE *f) {
    int c, line;
    CURSOR cursor;
    void * lineAdress[MAX_LINB_LINES];
    funcp funcao = (funcp) malloc(MAX_LINB_LINES*10);
    cursor = grava_bytes((CURSOR) funcao, prologo, sizeof prologo);

    for (line = 0;(c=fgetc(f)) != EOF;line++){
        lineAdress[line] = cursor;
        printf("c: %c, line: %d\n", c, line);
        switch (c) {
            case 'i': { /* if */
                cursor = grava_if(cursor, &f);
                break;
            }
            case 'r': { /* retorno */
                char c0;
                if ( fscanf(f, "et%c", &c0) != 1 )
                    cursor = NULL;
                else
                    cursor = grava_bytes(cursor, retorno, sizeof retorno);
                break;
            }
            case 'v':
            case 'p': { /* atribuicao */
                ungetc(c, f);
                cursor = grava_atribuicao(cursor, &f);
                break;
            }
            default:
                error("comando desconhecido", line+1);
        }
        if (!cursor){
            free(funcao);
            error("comando invalido", line+1);
        }
        fscanf(f, " "); 
    }
    write_offsets(funcao, lineAdress, line);
    return funcao;
}

void LiberaFuncao (void *p){
    free(p);
}
