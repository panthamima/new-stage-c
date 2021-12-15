#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>
#define int intptr_t

int token;                   // текущий токен
int token_val;               // значение текущего токена в основном для числа
char *src, *old_src;         // указатель на строку исходного кода 
int poolsize;                // стандартный размер текста/значения/стэка
int line;                    // номер строки 
int *text,                   // часть текста 
    *old_text,               // для кучи(хранилище памяти) сегмента текста
    *stack;                  // стэк(упорядоченный набор элементов)
char *data;                  // сегмент данных
int *pc, *bp, *sp, ax, cycle;// регистры вирутальной машины (??)
int *current_id,             // текущий проанализированный идентификатор
    *symbols;                // таблица символов ??
int *idmain;                 // указатель на main функцию


// инструкции 
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };


// классы и токены , операторы последними и в порядке приоритета
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, 
  Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

// поля идентификатора ??
enum { Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize };

// типы переменных и функций
enum { CHAR, INT, PTR };

int basetype;    // тип объявления , для удобства сделан глобальным
int expr_type;   // тип выражения

int index_of_bp; // индекс указателя стандартного указателя в стеке

void next() {
    char *last_pos;
    int hash;

    while (token = *src) {
        ++src;

        // парсинг токена
        if (token == '\n') {
            ++line;
        }
        else if (token == '#') {
            while (*src != 0 && *src != '\n') {
                src++;
            }
        }
        else if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {

            //парсинг идентификатора
            last_pos = src - 1;
            hash = token;

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) {
                hash = hash * 147 + *src;
                src++;
            }

            //поиск существующего идентификатора, линейный поиск
            current_id = symbols;
            while (current_id[Token]) {
                if (current_id[Hash] == hash && !memcmp((char *)current_id[Name], last_pos, src - last_pos)) {
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }

            //сохранение нового ID
            current_id[Name] = (int)last_pos;
            current_id[Hash] = hash;
            token = current_id[Token] = Id;
            return;
        }
        else if (token >= '0' && token <= '9') {
            // парсинг числа трех видов dec(123) hex(0x123) oct(017)
            token_val = token - '0';
            if (token_val > 0) {
                // dec - десятичное начинается с [1-9]
                while (*src >= '0' && *src <= '9') {
                    token_val = token_val*10 + *src++ - '0';
                }
            } else {
                // dec начинается с 0 
                if (*src == 'x' || *src == 'X') {
                    // hex - шестнадцатеричное
                    token = *++src;
                    while ((token >= '0' && token <= '9') || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F')) {
                        token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
                        token = *++src;
                    }
                } else {
                    // oct - восьмеричное
                    while (*src >= '0' && *src <= '7') {
                        token_val = token_val*8 + *src++ - '0';
                    }
                }
            }

            token = Num;
            return;
        }
        else if (token == '"' || token == '\'') {
            // парсим " и \ 
            last_pos = data;
            while (*src != 0 && *src != token) {
                token_val = *src++;
                // подстановка символа
                if (token_val == '\\') {
                    token_val = *src++;
                    if (token_val == 'n') {
                        token_val = '\n';
                    }
                }

                if (token == '"') {
                    *data++ = token_val;
                }
            }

            src++;
            // если это одиночный символ, возвращаем Num токен
            if (token == '"') {
                token_val = (int)last_pos;
            } else {
                token = Num;
            }

            return;
        }
        else if (token == '/') {
            if (*src == '/') {
                // пропуск комментариев
                while (*src != 0 && *src != '\n') {
                    ++src;
                }
            } else {
                // применяем оператор деления
                token = Div;
                return;
            }
        }
        else if (token == '=') {
            // парсим '==' и '='
            if (*src == '=') {
                src ++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        }
        else if (token == '+') {
            // парсим '+' и '++'
            if (*src == '+') {
                src ++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        }
        else if (token == '-') {
            // парсим '-' и '--'
            if (*src == '-') {
                src ++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        }
        else if (token == '!') {
            // парсим !=
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        }
        else if (token == '<') {
            // парсим <=, <<, <
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
            // парсим >=, >>, >
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
            // парсим | и ||
            if (*src == '|') {
                src ++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        }
        else if (token == '&') {
            // парсим & и &&
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
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            // если ничего из этого, напрямую возвращаем токен
            return;
        }
    }
    return;
}

void match(int tk) {
    if (token == tk) {
        next();
    } else {
        printf("%d: expected token: %d\n", line, tk);
        exit(-1);
    }
}

void expression(int level) {
    // выражения имеют различный формат
    // но в основном их можно поделить на две части
    // модуль и оператор, например:
    // (char) * a[10] = (int*)func(b>0 ? 10 : 20);
    // a[10] это единица измеренеия а * это оператор
    // результат func(...) это единица измерения
    // поэтому мы сначала парсим единичные и унарные операторы
    // а потом двоичные,
    // 
    // так же выражения могут быть след. типов:
    //
    // 1. unit_unary ::= unit | unit unary_op | unary_op unit
    // 2. expr ::= unit_unary (bin_op unit_unary ...)


    // инициализация унарного оператора
    int *id;
    int tmp;
    int *addr;
    {
        if (!token) {
            printf("%d: unexpected token EOF of expression\n", line);
            exit(-1);
        }
        if (token == Num) {
            match(Num);


            // вывод кода
            *++text = IMM;
            *++text = token_val;
            expr_type = INT;
        }
        else if (token == '"') {


            *++text = IMM;
            *++text = token_val;

            match('"');
            // хранить остальные строки
            while (token == '"') {
                match('"');
            }

            // смещение всех данных на 1 позицию вперед,
            // чтобы начиналось не с 0 а с 1
            data = (char *)(((int)data + sizeof(int)) & (-sizeof(int)));
            expr_type = PTR;
        }
        else if (token == Sizeof) {
            // sizeof унарный оператор, который поддерживает только :
            // sizeof(int), sizeof(char) ...
            match(Sizeof);
            match('(');
            expr_type = INT;

            if (token == Int) {
                match(Int);
            } else if (token == Char) {
                match(Char);
                expr_type = CHAR;
            }

            while (token == Mul) {
                match(Mul);
                expr_type = expr_type + PTR;
            }

            match(')');

            // вывод кода
            *++text = IMM;
            *++text = (expr_type == CHAR) ? sizeof(char) : sizeof(int);

            expr_type = INT;
        }
        else if (token == Id) {
            // есть несколько типов, которые могут быть ??
            // 1)вызов функции
            // 2)enum переменная
            // 3)глобальная или локальная переменная
            match(Id);

            id = current_id;

            if (token == '(') {
                // вызов функции
                match('(');

                // передаем аргументы
                tmp = 0; // количество аргументов
                while (token != ')') {
                    expression(Assign);
                    *++text = PUSH;
                    tmp ++;

                    if (token == ',') {
                        match(',');
                    }

                }
                match(')');

                // вывод кода
                if (id[Class] == Sys) {
                    // системная функция 
                    *++text = id[Value];
                }
                else if (id[Class] == Fun) {
                    // вызов функции
                    *++text = CALL;
                    *++text = id[Value];
                }
                else {
                    printf("%d: bad function call\n", line);
                    exit(-1);
                }

                // очистить стек для аргументов
                if (tmp > 0) {
                    *++text = ADJ;
                    *++text = tmp;
                }
                expr_type = id[Type];
            }
            else if (id[Class] == Num) {
                // enum переменная
                *++text = IMM;
                *++text = id[Value];
                expr_type = INT;
            }
            else {
                // проверка переменной
                if (id[Class] == Loc) {
                    *++text = LEA;
                    *++text = index_of_bp - id[Value];
                }
                else if (id[Class] == Glo) {
                    *++text = IMM;
                    *++text = id[Value];
                }
                else {
                    printf("%d: undefined variable\n", line);
                    exit(-1);
                }

                // вывод кода, по умолчанию закгржается значение 
                // адрес которого хранится в 'ax'
                expr_type = id[Type];
                *++text = (expr_type == CHAR) ? LC : LI;
            }
        }
        else if (token == '(') {
            // приведение или скобка
            match('(');
            if (token == Int || token == Char) {
                tmp = (token == Char) ? CHAR : INT; // тип приведения
                match(token);
                while (token == Mul) {
                    match(Mul);
                    tmp = tmp + PTR;
                }

                match(')');

                expression(Inc); // приведение имеет приоритет как (Inc++)

                expr_type  = tmp;
            } else {
                // если просто скобка
                expression(Assign);
                match(')');
            }
        }
        else if (token == Mul) {
            // разыменовывание *<addr>
            match(Mul);
            expression(Inc); 

            if (expr_type >= PTR) {
                expr_type = expr_type - PTR;
            } else {
                printf("%d: bad dereference\n", line);
                exit(-1);
            }

            *++text = (expr_type == CHAR) ? LC : LI;
        }
        else if (token == And) {
            // получение адреса
            match(And);
            expression(Inc); // получение адреса
            if (*text == LC || *text == LI) { 
                text --;
            } else {
                printf("%d: bad address of\n", line);
                exit(-1);
            }

            expr_type = expr_type + PTR;
        }
        else if (token == '!') {
            // отрицание
            match('!');
            expression(Inc);

            // вывод кода, используем <expr> == 0
            *++text = PUSH;
            *++text = IMM;
            *++text = 0;
            *++text = EQ;

            expr_type = INT;
        }
        else if (token == '~') {
            // побитовое отрицание
            match('~');
            expression(Inc);

            // вывод кода, используем <expr> XOR -1
            *++text = PUSH;
            *++text = IMM;
            *++text = -1;
            *++text = XOR;

            expr_type = INT;
        }
        else if (token == Add) {
            // добавляем переменную. ничего не делаем
            match(Add);
            expression(Inc);

            expr_type = INT;
        }
        else if (token == Sub) {
            // -переменная
            match(Sub);

            if (token == Num) {
                *++text = IMM;
                *++text = -token_val;
                match(Num);
            } else {

                *++text = IMM;
                *++text = -1;
                *++text = PUSH;
                expression(Inc);
                *++text = MUL;
            }

            expr_type = INT;
        }
        else if (token == Inc || token == Dec) {
            tmp = token;
            match(token);
            expression(Inc);
            if (*text == LC) {
                *text = PUSH; // дублировать адресс
                *++text = LC;
            } else if (*text == LI) {
                *text = PUSH;
                *++text = LI;
            } else {
                printf("%d: bad lvalue of pre-increment\n", line);
                exit(-1);
            }
            *++text = PUSH;
            *++text = IMM;
            *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
            *++text = (tmp == Inc) ? ADD : SUB;
            *++text = (expr_type == CHAR) ? SC : SI;
        }
        else {
            printf("%d: bad expression\n", line);
            exit(-1);
        }
    }
    // бинарный оператор и постфиксные операторы
    {
        while (token >= level) {
            // обрабатывать в соответствии с приоритетом текущего оператора
            tmp = expr_type;
            if (token == Assign) {
                // переменная = выражение
                match(Assign);
                if (*text == LC || *text == LI) {
                    *text = PUSH; // сохранить указатель lvalue(данных строки)
                } else {
                    printf("%d: bad lvalue in assignment\n", line);
                    exit(-1);
                }
                expression(Assign);

                expr_type = tmp;
                *++text = (expr_type == CHAR) ? SC : SI;
            }
            else if (token == Cond) {
                // выражение ? a : b;
                match(Cond);
                *++text = JZ;
                addr = ++text;
                expression(Assign);
                if (token == ':') {
                    match(':');
                } else {
                    printf("%d: missing colon in conditional\n", line);
                    exit(-1);
                }
                *addr = (int)(text + 3);
                *++text = JMP;
                addr = ++text;
                expression(Cond);
                *addr = (int)(text + 1);
            }
            else if (token == Lor) {
                // логическое or
                match(Lor);
                *++text = JNZ;
                addr = ++text;
                expression(Lan);
                *addr = (int)(text + 1);
                expr_type = INT;
            }
            else if (token == Lan) {
                // логическое and
                match(Lan);
                *++text = JZ;
                addr = ++text;
                expression(Or);
                *addr = (int)(text + 1);
                expr_type = INT;
            }
            else if (token == Or) {
                // побитовое or
                match(Or);
                *++text = PUSH;
                expression(Xor);
                *++text = OR;
                expr_type = INT;
            }
            else if (token == Xor) {
                // побитовое xor
                match(Xor);
                *++text = PUSH;
                expression(And);
                *++text = XOR;
                expr_type = INT;
            }
            else if (token == And) {
                // побитовое and
                match(And);
                *++text = PUSH;
                expression(Eq);
                *++text = AND;
                expr_type = INT;
            }
            else if (token == Eq) {
                // сравнение ==
                match(Eq);
                *++text = PUSH;
                expression(Ne);
                *++text = EQ;
                expr_type = INT;
            }
            else if (token == Ne) {
                // не равно !=
                match(Ne);
                *++text = PUSH;
                expression(Lt);
                *++text = NE;
                expr_type = INT;
            }
            else if (token == Lt) {
                // меньше чем
                match(Lt);
                *++text = PUSH;
                expression(Shl);
                *++text = LT;
                expr_type = INT;
            }
            else if (token == Gt) {
                // больше чем
                match(Gt);
                *++text = PUSH;
                expression(Shl);
                *++text = GT;
                expr_type = INT;
            }
            else if (token == Le) {
                // меньше или равно
                match(Le);
                *++text = PUSH;
                expression(Shl);
                *++text = LE;
                expr_type = INT;
            }
            else if (token == Ge) {
                // больше или равно
                match(Ge);
                *++text = PUSH;
                expression(Shl);
                *++text = GE;
                expr_type = INT;
            }
            else if (token == Shl) {
                // сдвиг влево
                match(Shl);
                *++text = PUSH;
                expression(Add);
                *++text = SHL;
                expr_type = INT;
            }
            else if (token == Shr) {
                // сдвиг вправо
                match(Shr);
                *++text = PUSH;
                expression(Add);
                *++text = SHR;
                expr_type = INT;
            }
            else if (token == Add) {
                // добавление
                match(Add);
                *++text = PUSH;
                expression(Mul);

                expr_type = tmp;
                if (expr_type > PTR) {
                    // тип указателя, не `char *`
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                }
                *++text = ADD;
            }
            else if (token == Sub) {
                // sub
                match(Sub);
                *++text = PUSH;
                expression(Mul);
                if (tmp > PTR && tmp == expr_type) {
                    // вычитание указателя
                    *++text = SUB;
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = DIV;
                    expr_type = INT;
                } else if (tmp > PTR) {
                    // перемещение указателя
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                    *++text = SUB;
                    expr_type = tmp;
                } else {
                    // числовое вычитание
                    *++text = SUB;
                    expr_type = tmp;
                }
            }
            else if (token == Mul) {
                // умножение
                match(Mul);
                *++text = PUSH;
                expression(Inc);
                *++text = MUL;
                expr_type = tmp;
            }
            else if (token == Div) {
                // деление
                match(Div);
                *++text = PUSH;
                expression(Inc);
                *++text = DIV;
                expr_type = tmp;
            }
            else if (token == Mod) {
                // деление по модулю ( с остатком)
                match(Mod);
                *++text = PUSH;
                expression(Inc);
                *++text = MOD;
                expr_type = tmp;
            }
            else if (token == Inc || token == Dec) {
                if (*text == LI) {
                    *text = PUSH;
                    *++text = LI;
                }
                else if (*text == LC) {
                    *text = PUSH;
                    *++text = LC;
                }
                else {
                    printf("%d: bad value in increment\n", line);
                    exit(-1);
                }

                *++text = PUSH;
                *++text = IMM;
                *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
                *++text = (token == Inc) ? ADD : SUB;
                *++text = (expr_type == CHAR) ? SC : SI;
                *++text = PUSH;
                *++text = IMM;
                *++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
                *++text = (token == Inc) ? SUB : ADD;
                match(token);
            }
            else if (token == Brak) {
                // доступ к массиву var[xx]
                match(Brak);
                *++text = PUSH;
                expression(Assign);
                match(']');

                if (tmp > PTR) {
                    // указатель, `не char *`
                    *++text = PUSH;
                    *++text = IMM;
                    *++text = sizeof(int);
                    *++text = MUL;
                }
                else if (tmp < PTR) {
                    printf("%d: pointer type expected\n", line);
                    exit(-1);
                }
                expr_type = tmp - PTR;
                *++text = ADD;
                *++text = (expr_type == CHAR) ? LC : LI;
            }
            else {
                printf("%d: compiler error, token = %d\n", line, token);
                exit(-1);
            }
        }
    }
}

void statement() {
    // виды statements(6)
    // 1) if(...) <statement> [else <statment>]
    // 2) while(...) <statement>
    // 3) { <statement> }
    // 4) return ...;
    // 5) <empty statement>
    // 6) expression (конец выражения точкой с запятой)

    int *a, *b; // для управления фильтрами

    if (token == If) {
        match(If);
        match('(');
        expression(Assign);  // парсинг состояния
        match(')');

        *++text = JZ;
        b = ++text;

        statement();  // парсинг statement
        if (token == Else) { // парсинг else
            match(Else);

            *b = (int)(text + 3);
            *++text = JMP;
            b = ++text;

            statement();
        }

        *b = (int)(text + 1);
    }
    else if (token == While) {
        match(While);

        a = text + 1;

        match('(');
        expression(Assign);
        match(')');

        *++text = JZ;
        b = ++text;

        statement();

        *++text = JMP;
        *++text = (int)a;
        *b = (int)(text + 1);
    }
    else if (token == '{') {
        // { <statement> ...}
        match('{');

        while (token != '}') {
            statement();
        }

        match('}');
    }
    else if (token == Return) {
        // return [expression];
        match(Return);

        if (token != ';') {
            expression(Assign);
        }

        match(';');

        // вывод кода для return
        *++text = LEV;
    }
    else if (token == ';') {
        // пустой statement
        match(';');
    }
    else {
        expression(Assign);
        match(';');
    }
}

void function_parameter() {
    int type;
    int params;
    params = 0;
    while (token != ')') {
        // int name, ...
        type = INT;
        if (token == Int) {
            match(Int);
        } else if (token == Char) {
            type = CHAR;
            match(Char);
        }

        // тип указателя
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        // имя параметра
        if (token != Id) {
            printf("%d: bad parameter declaration\n", line);
            exit(-1);
        }
        if (current_id[Class] == Loc) {
            printf("%d: duplicate parameter declaration\n", line);
            exit(-1);
        }

        match(Id);
        // хранить локальную переменную
        current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
        current_id[BType]  = current_id[Type];  current_id[Type]   = type;
        current_id[BValue] = current_id[Value]; current_id[Value]  = params++;   // index of current parameter

        if (token == ',') {
            match(',');
        }
    }
    index_of_bp = params+1;
}

void function_body() {
    // тип func_name (...) {...}
    //   -->|   |<--

    // ... {
    // 1. локальное объявление
    // 2. statements
    // }
    int pos_local; // позиция локальной переменной в стэке
    int type;
    pos_local = index_of_bp;

    while (token == Int || token == Char) {
        // локальное объявление переменной, как и глобальной
        basetype = (token == Int) ? INT : CHAR;
        match(token);

        while (token != ';') {
            type = basetype;
            while (token == Mul) {
                match(Mul);
                type = type + PTR;
            }

            if (token != Id) {
                // неправильное объявление
                printf("%d: bad local declaration\n", line);
                exit(-1);
            }
            if (current_id[Class] == Loc) {
                // идентификатор существует
                printf("%d: duplicate local declaration\n", line);
                exit(-1);
            }
            match(Id);

            // хранение локальной переменной
            current_id[BClass] = current_id[Class]; current_id[Class]  = Loc;
            current_id[BType]  = current_id[Type];  current_id[Type]   = type;
            current_id[BValue] = current_id[Value]; current_id[Value]  = ++pos_local;   // index of current parameter

            if (token == ',') {
                match(',');
            }
        }
        match(';');
    }

    // сохранить размер стэка для локальной переменной
    *++text = ENT;
    *++text = pos_local - index_of_bp;

    // statements
    while (token != '}') {
        statement();
    }

    // вывод кода для покидающих sub функций
    *++text = LEV;
}

void function_declaration() {
    // type func_name (...) {...}
    //               | this part

    match('(');
    function_parameter();
    match(')');
    match('{');
    function_body();

    // развернуть объявления локальных переменных для
    // всех локальных переменных

    current_id = symbols;
    while (current_id[Token]) {
        if (current_id[Class] == Loc) {
            current_id[Class] = current_id[BClass];
            current_id[Type]  = current_id[BType];
            current_id[Value] = current_id[BValue];
        }
        current_id = current_id + IdSize;
    }
}

void enum_declaration() {
    // парсинг enum[id] {a = 1, b = 2, ...}
    int i;
    i = 0;
    while (token != '}') {
        if (token != Id) {
            printf("%d: bad enum identifier %d\n", line, token);
            exit(-1);
        }
        next();
        if (token == Assign) {
            // как {a=10}
            next();
            if (token != Num) {
                printf("%d: bad enum initializer\n", line);
                exit(-1);
            }
            i = token_val;
            next();
        }

        current_id[Class] = Num;
        current_id[Type] = INT;
        current_id[Value] = i++;

        if (token == ',') {
            next();
        }
    }
}

void global_declaration() {
    // global_declaration ::= enum_decl | variable_decl | function_decl
    //
    // enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'} '}'
    //
    // variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
    //
    // function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'

    int type; // фактический тип переменной
    int i; // tmp 

    basetype = INT;

    // парсим enum
    if (token == Enum) {
        // enum [id] { a = 10, b = 20, ... }
        match(Enum);
        if (token != '{') {
            match(Id); // пропустить [id] часть
        }
        if (token == '{') {
            // парсим присвоенную часть
            match('{');
            enum_declaration();
            match('}');
        }

        match(';');
        return;
    }

    // парсим тип информации
    if (token == Int) {
        match(Int);
    }
    else if (token == Char) {
        match(Char);
        basetype = CHAR;
    }

    // парсим объяление переменной разделенной запятыми
    while (token != ';' && token != '}') {
        type = basetype;
        // парсим тип указателя ,обратите внимание что может существовать `int ****x;`
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }

        if (token != Id) {
            // неправильное объявление
            printf("%d: bad global declaration\n", line);
            exit(-1);
        }
        if (current_id[Class]) {
            // идентификатор найден
            printf("%d: duplicate global declaration\n", line);
            exit(-1);
        }
        match(Id);
        current_id[Type] = type;

        if (token == '(') {
            current_id[Class] = Fun;
            current_id[Value] = (int)(text + 1); // адрес функции в памяти
            function_declaration();
        } else {
            // объявление переменной
            current_id[Class] = Glo; // глобальная переменная
            current_id[Value] = (int)data; // назначить адрес в памяти
            data = data + sizeof(int);
        }

        if (token == ',') {
            match(',');
        }
    }
    next();
}

void program() {
    // получить следующий токен
    next();
    while (token > 0) {
        global_declaration();
    }
}


int eval() {
    int op, *tmp;
    while (1) {
        op = *pc++; // получить код следующей операции

        if (op == IMM)       {ax = *pc++;}                                     
        else if (op == LC)   {ax = *(char *)ax;}                              
        else if (op == LI)   {ax = *(int *)ax;}                                
        else if (op == SC)   {ax = *(char *)*sp++ = ax;}                      
        else if (op == SI)   {*(int *)*sp++ = ax;}                           
        else if (op == PUSH) {*--sp = ax;}                                     
        else if (op == JMP)  {pc = (int *)*pc;}                              
        else if (op == JZ)   {pc = ax ? pc + 1 : (int *)*pc;}                   
        else if (op == JNZ)  {pc = ax ? (int *)*pc : pc + 1;}                  
        else if (op == CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;}        
        else if (op == ENT)  {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}     
        else if (op == ADJ)  {sp = sp + *pc++;}                                
        else if (op == LEV)  {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;}  
        else if (op == LEA)  {ax = (int)(bp + *pc++);}                         

        else if (op == OR)  ax = *sp++ | ax;
        else if (op == XOR) ax = *sp++ ^ ax;
        else if (op == AND) ax = *sp++ & ax;
        else if (op == EQ)  ax = *sp++ == ax;
        else if (op == NE)  ax = *sp++ != ax;
        else if (op == LT)  ax = *sp++ < ax;
        else if (op == LE)  ax = *sp++ <= ax;
        else if (op == GT)  ax = *sp++ >  ax;
        else if (op == GE)  ax = *sp++ >= ax;
        else if (op == SHL) ax = *sp++ << ax;
        else if (op == SHR) ax = *sp++ >> ax;
        else if (op == ADD) ax = *sp++ + ax;
        else if (op == SUB) ax = *sp++ - ax;
        else if (op == MUL) ax = *sp++ * ax;
        else if (op == DIV) ax = *sp++ / ax;
        else if (op == MOD) ax = *sp++ % ax;


        else if (op == EXIT) { printf("exit(%d)\n", *sp); return *sp;}
        else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); }
        else if (op == CLOS) { ax = close(*sp);}
        else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
        else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); }
        else if (op == MALC) { ax = (int)malloc(*sp);}
        else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
        else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}
        else {
            printf("unknown instruction:%d\n", op);
            return -1;
        }
    }
    return 0;
}

#undef int 

int main(int argc, char **argv)
{
    #define int intptr_t

    int i, fd;
    int *tmp;

    argc--;
    argv++;

    poolsize = 256 * 1024; // произвольный размер
    line = 1;

    // выделение памяти для виртуальной машины
    if (!(text = old_text = malloc(poolsize))) {
        printf("could not malloc(%d) for text area\n", poolsize);
        return -1;
    }
    if (!(data = malloc(poolsize))) {
        printf("could not malloc(%d) for data area\n", poolsize);
        return -1;
    }
    if (!(stack = malloc(poolsize))) {
        printf("could not malloc(%d) for stack area\n", poolsize);
        return -1;
    }
    if (!(symbols = malloc(poolsize))) {
        printf("could not malloc(%d) for symbol table\n", poolsize);
        return -1;
    }

    memset(text, 0, poolsize);
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);
    memset(symbols, 0, poolsize);
    bp = sp = (int *)((int)stack + poolsize);
    ax = 0;

    src = "char else enum if int return sizeof while "
          "open read close printf malloc memset memcmp exit void main";

    i = Char; // добавить ключевые слова в таблицу символов
    while (i <= While) {
        next();
        current_id[Token] = i++;
    }

    // добавить библиотеку в таблицу символов
    i = OPEN;
    while (i <= EXIT) {
        next();
        current_id[Class] = Sys;
        current_id[Type] = INT;
        current_id[Value] = i++;
    }

    next(); current_id[Token] = Char; // обрабатывать тип void
    next(); idmain = current_id; // отслеживать main


    // прочитать исходный файл
    if ((fd = open(*argv, 0)) < 0) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }
    // прочитать исходный файл
    if ((i = read(fd, src, poolsize-1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // добавление EOF символа
    close(fd);

    program();

    if (!(pc = (int *)idmain[Value])) {
        printf("main() not defined\n");
        return -1;
    }

    // запуск стека
    sp = (int *)((int)stack + poolsize);
    *--sp = EXIT; // если выведена main функция return exit(-1)
    *--sp = PUSH; tmp = sp;
    *--sp = argc;
    *--sp = (int)argv;
    *--sp = (int)tmp;

    return eval();
}
