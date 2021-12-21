#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncurses.h>

// int center();

// // int main () {

// //     int center();%02d:%02d:%02d

// //     return 0;
// // }

// int main() {
//     int space = 92;
//     char x[] = "";

//     int length = strlen(x);

//     while (1) {   
//         system("clear");
        
//         time_t t = time(NULL);
//         struct tm* aTm = localtime(&t);
//         printf ("%d",aTm->tm_hour, aTm->tm_min, aTm->tm_sec,((space - length) >> 1) + length, x, ((space - length) >> 1) + ((space - length) & 1), ' ');
//         sleep(1);

//     }
//     return 0;
// }

int time_time()
{
    time_t t = time(NULL);
    struct tm* aTm = localtime(&t);
    printf("%02d:%02d:%02d\n", aTm->tm_hour, aTm->tm_min, aTm->tm_sec);

}

// int main (int argc, char **argv) {
//     struct winsize w;
//     int width = w.ws_col;
//     int height = w.ws_row;
//     ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

//     // printf("%d*%s*%d    "((space - length) >> 1) + length, x, 
//     // ((space - length) >> 1) + ((space - length) & 1), ' '););
//     printf ("lines %d\n", w.ws_row);
//     printf ("columns %d\n", w.ws_col);
//     return 0;  // make sure your main returns int
// }


int main () {

    while (1)
    {
        system("clear");
        time_time();
        sleep(1);
    }
    return 0;
}


