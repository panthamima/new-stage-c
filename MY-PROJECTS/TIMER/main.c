#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
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

//     // while (heigth == 21 & width == 92) {
//     while(1) {
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


// SIGWINCH is called when the window is resized.
void center(int sig){
	signal(SIGWINCH, SIG_IGN);

  // Reinitialize the window to update data structures.
	endwin();
	initscr();
	refresh();
	clear();

	time_t t = time(NULL);
	struct tm* aTm = localtime(&t);
	// sprintf("%02d:%02d:%02d\n", aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
	// int time_size = 9; 

	char tmp[128];
	// sprintf(tmp, "%d x %d", COLS, LINES);
	char temp[128];
	while(1) {
		sprintf(temp, "%02d:%02d:%02d\n", aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
	}


	int y = LINES / 2 - 1;
	int x = COLS / 2 - strlen(tmp) / 2;
	// Approximate the center

	mvaddstr(y, x, temp);
	refresh();

	signal(SIGWINCH, center);
}

int main(int argc, char *argv[]){
	initscr();
	// COLS/LINES are now set

	signal(SIGWINCH, center);

	while(1){
		system("clear");
		void center();
		sleep(1);
	}

	endwin();

	return(0);
}