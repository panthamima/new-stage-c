#ifndef UTILS_H
#define UTILS_H

#define MAX_LEN 1024
#define TOKEN_SEP

typedef struct {
    char* progname;
    int redirect[2];
    char* args[];



} cmd_struct;

typedef struct {
    int n_cmds;
    cmd_struct* cmds[];
} pipeline_struct;

cmd_struct* parse_command(char* str);
pipeline_struct* parse_pipeline(char* str);

void print_command(cmd_struct* command);
void print_pepeline(pipeline_struct* pipeline);


#endif
