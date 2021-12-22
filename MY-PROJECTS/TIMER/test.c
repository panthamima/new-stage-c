#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdarg.h>


int main (int argc, char **argv) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int width = w.ws_col;
    int heigth = w.ws_row;
    int width05 = w.ws_col / 2;
    char x[] = " "; 
    int word_size = strlen(x);
    

    while (heigth == 21 & width == 92) {
        system("clear");
        time_t t = time(NULL);
        struct tm* aTm = localtime(&t);
        // printf ("%02d:%02d:%02d\n",aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
        printf("%d %02d:%02d:%02d %d", width05 ,aTm->tm_hour, aTm->tm_min, aTm->tm_sec, width05);
        // printf("%d %s %d",((width - word_size) >> 1) + word_size, x, ((width - word_size) >> 1) + ((width - word_size) & 1), ' ');

        sleep(1);
    }
    // printf ("height %d\n", height);
    // printf ("width %d\n", width);
    return 0;  // make sure your main returns int
}