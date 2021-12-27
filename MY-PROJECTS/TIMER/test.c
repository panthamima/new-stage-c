#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdarg.h>


// int main (int argc, char **argv) {
//     struct winsize w;
//     ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    
//         int heigth = w.ws_row;
//         int width = w.ws_col;
//         int width05 = w.ws_col / 2;
//         char x[] = " "; 
//         int word_size = strlen(x);

//     while (heigth == 21 & width == 92) {
//         system("clear");
//         time_t t = time(NULL);
//         struct tm* aTm = localtime(&t);
//         printf("%02d:%02d:%02d\n", aTm->tm_hour, aTm->tm_min, aTm->tm_sec);

//         sleep(1);
//     }
//     // printf ("height %d\n", height);
//     // printf ("width %d\n", width);
//     return 0;  // make sure your main returns int
// }

#include <ncurses.h>
#include <string.h>
#include <signal.h>

// SIGWINCH is called when the window is resized.
void handle_winch(int sig){
  signal(SIGWINCH, SIG_IGN);

  // Reinitialize the window to update data structures.
  endwin();
  initscr();
  refresh();
  clear();

  char tmp[128];
  sprintf(tmp, "%dx%d", COLS, LINES);

  // Approximate the center
  int x = COLS / 2 - strlen(tmp) / 2;
  int y = LINES / 2 - 1;

  mvaddstr(y, x, tmp);
  refresh();

  signal(SIGWINCH, handle_winch);
}

int main(int argc, char *argv[]){
  initscr();
  // COLS/LINES are now set

  signal(SIGWINCH, handle_winch);

  while(getch() != 27){
    /* Nada */
  }

  endwin();

  return(0);
}