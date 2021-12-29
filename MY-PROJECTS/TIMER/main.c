#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <ncurses.h>

// SIGWINCH is called when the window is resized.
void center(int sig){
	
  // Reinitialize the window to update data structures.
	signal(SIGWINCH, SIG_IGN);
	// endwin();
	// initscr();
	// refresh();
	// clear();

	char temp[128];
	int y = LINES / 2 - 1;
	int x = COLS / 2 - strlen(temp) / 2;

	while(1) {
		time_t t = time(NULL);
		struct tm* aTm = localtime(&t);
		sprintf(temp, "%02d:%02d:%02d\n", aTm->tm_hour, aTm->tm_min, aTm->tm_sec);

		mvaddstr(y, x, temp);
		refresh();
	}
	signal(SIGWINCH, center);
}


int main () {
	initscr();
	signal(SIGWINCH, center);
	curs_set(0); // убирает курсор терминала

	while(1) {
		refresh();
		void center();
		sleep(1);
	}

	endwin();
    return 0;
}
