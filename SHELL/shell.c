#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

ssize_t prompt_and_get_input(const char* prompt, char **line, size_t *len) {
    fputs(prompt, stderr);
    return getline(line, len, stdin);
}

void close_ALL_the_pipes(int n_pipes, int (*pipes)[2]) {
  for (int i = 0; i < n_pipes; ++i) {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }
}


int exec_with_redir(cmd_struct* command, int n_pipes, int (*pipes)[2]) {
  int fd = -1;
  if ((fd = command->redirect[0]) != -1) {
    dup2(fd, STDIN_FILENO);
  }
  if ((fd = command->redirect[1]) != -1) {
    dup2(fd, STDOUT_FILENO);
  }
  close_ALL_the_pipes(n_pipes, pipes);
  return execvp(command->progname, command->args);
}


pid_t run_with_redir(cmd_struct* command, int n_pipes, int (*pipes)[2]) {
  pid_t child_pid = fork();

  if (child_pid) {  
    switch(child_pid) {
      case -1:
        fprintf(stderr, "Oh dear.\n");
        return -1;
      default:
        return child_pid;
    }
  } else {  
    exec_with_redir(command, n_pipes, pipes);
    perror("OH DEAR");
    return 0;
  }
}

int main() {
    char *line = NULL;
    size_t len = 0;
    while(prompt_and_get_input("heee>", &line, &len) > 0) {
        pipeline_struct* pipeline = parse_pipeline(line);
        int n_pipes = pipeline->n_cmds - 1;

        int (*pipes)[2] = calloc(sizeof(int[2]), n_pipes);

        for(int i = 1; i < pipeline->n_cmds; ++i) {
            pipe(pipes[i-1]);
            pipeline->cmds[i]->redirect[STDIN_FILENO] = pipes[i-1][0];
            pipeline->cmds[i-1]->redirect[STDOUT_FILENO] = pipes[i-1][0];
        }
        for(int i = 0; i < pipeline->n_cmds; i++) {
            run_with_redir(pipeline->cmds[i], n_pipes, pipes);
        }

        close_ALL_the_pipes(n_pipes, pipes);

        for(int i = 0; i < pipeline ->n_cmds; ++i) {
            wait(NULL);
        }
    }
    fputs("\n", stderr);
    return 0;
}