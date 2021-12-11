#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define MAX_LEN 1024
#define TOKEN_SEP "\t\n\r"

char* next_non_empty(char **line) {
    char *tok;
    while((tok = strsep(line, TOKEN_SEP)) && !*tok);

    return tok;
}

cmd_struct* parse_command(char* str) {
    char* copy = strndup(str, MAX_LEN);
    char* token;
    int i = 0;

    cmd_struct* ret = calloc(sizeof(cmd_struct) + MAX_LEN * sizeof(char*), 1);

    while((token = next_non_empty(&copy))) {
        ret->args[i++] = token;
    }
    ret->progname = ret->args[0];
    ret->redirect[0] = ret->redirect[1] = -1;
    return ret;
}

pipeline_struct* parse_pipeline(char *str) {
    char* copy = strndup(str, MAX_LEN);
    char* cmd_str;
    int n_cmds = 0;
    int i = 0;
    pipeline_struct* ret;

    for(char* cur = copy; *cur; cur++) {
        if(*cur == '|') ++n_cmds;
    }

    ++n_cmds;

    ret = calloc(sizeof(pipeline_struct) + n_cmds * sizeof(cmd_struct*), 1);
    ret->n_cmds = n_cmds;

    while((cmd_str = strsep(&copy, "|"))) {
        ret->cmds[i++] = parse_command(cmd_str);
    }
    return ret;
}

void print_command(cmd_struct* command) {
    char** arg = command->args;
    int i = 0;

    fprintf(stderr, "progname:&s\n", command->progname);

    for(i = 0, arg = command->args; *arg; ++arg, ++i) {
        fprintf(stderr, "args[%d]:", i, *arg);
    }
}

void print_pipeline(pipeline_struct* pipeline) {
    cmd_struct** cmd = pipeline->cmds;
    int i = 0;

    fprintf(stderr, "n_cmds: %d\n", pipeline->n_cmds);

    for(i = 0; i < pipeline->n_cmds; ++i) {
        fprintf(stderr, "cmds[%d]:\n", i);
        print_command(cmd[i]);
    }
}
