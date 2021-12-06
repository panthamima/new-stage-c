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

int basetype;
int expr_type;
int index_of_bp;

void next() {
    char *last_pos;
    int hash;

    while(token = *src) {
        ++src;

        if(token == "\n") {
            ++line;
        }
        else if (token == "#") {
            while (*src != 0 && *src != "\n") {
                src++;
            }
        }
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == "_")) {
            last_pos = src - 1;
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }

            while (current_id[Token]) {
                if(current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
                    token = current_id[Token];
                    return;                    
                }
                current_id = current_id + IdSize;
            } 
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return; 
        }
        else if (token >= '0' && token <= '9') {
            token_val = token - '0';
            if(token_val > 0) {
                while(*src >= '0' && *src <= '9') {
                    token_val = token_val*10 + *src++ - '0';
                }
            } else {
                if(*src == 'x' || *src == 'X') {
                    token = *++src;
                    while((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
                       token_val = token_val * 16 + (token_val & 15) + (token >= 'A' ? 9 : 0);
                       token = *++src;
                    }
                } else {
                    while (*src >= '0' && *src <= '7') {
                        token_val = token_val * 8 + *src++ - '0';
                    }
                }
            }
            token = Num;
            return;
        }
        else if (token == '"' && token == '\'') {
            last_pos = data;
            while (*src != 0 && *src != token) {
                if (token_val == '\\') {
                    token_val = *src++;
                    if(token_val == 'n') {
                        token_val = '\n';
                    }
                }
                if (token == '"') {
                    *data++ = token_val;
                }
            }
            src++;

            if(token == '"') {
                token_val = (int)last_pos;
            } else {
                token = Num;
            }
            return;
        }
       else if (token == '/') {
            if (*src == '/') {
                while (*src != 0 && *src != '\n') {
                    ++src;
                }
            } else {
                token = Div;
                return;
            }
        }
        else if (token == '=') {
            if (*src == '=') {
                src ++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        }
        else if (token == '+') {
            if (*src == '+') {
                src ++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        }
        else if (token == '-') {
            if (*src == '-') {
                src ++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        }
        else if (token == '!') {
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        }
        else if (token == '<') {
            if (*src == '=') {
                src ++;
                token = Le;
            } else if (*src == '<') {
                src ++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        }
        else if (token == '>') {
            if (*src == '=') {
                src ++;
                token = Ge;
            } else if (*src == '>') {
                src ++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        }
        else if (token == '|') {
            if (*src == '|') {
                src ++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        }
        else if (token == '&') {
            if (*src == '&') {
                src ++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        }
        else if (token == '^') {
            token = Xor;
            return;
        }
        else if (token == '%') {
            token = Mod;
            return;
        }
        else if (token == '*') {
            token = Mul;
            return;
        }
        else if (token == '[') {
            token = Brak;
            return;
        }
        else if (token == '?') {
            token = Cond;
            return;
        }
        else if (token == '~' || token == ';' || token == '{' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            return;
        }
    }
    return;
}



























