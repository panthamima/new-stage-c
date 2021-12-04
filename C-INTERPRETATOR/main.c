#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>

#define int intptr_t

int token, token_val;
char *src, *old_src;
int poolsize, line;
int *text, *old_text, *stack;
char *data;
int *pc, *bp, *sp, ax, cycle;
int *current_id, *symbols;
int *idmain;

enum { LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI,
       SC, PUSH, OR, SOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD,
       SUB, MUL, DIV, MOD, OPEN, READ, CLOC, PRTF, MALC, MSET, MCMP, EXIT };

enum { Num = 128, Fun, Sys, Glo, Loc, Id,
       Char, Else, Enum, If, Int, Return, Sizeof, While,
       Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, 
       Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak }; 

enum { Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize };

enum { CHAR, INT, PRT };