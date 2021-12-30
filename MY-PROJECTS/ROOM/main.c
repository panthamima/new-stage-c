#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

int main() {
    char room[10][21];
    int i;
    char key;

    int x = 10, y = 5;

    sprintf(room[0], "/------------------\\");
    for(i = 1; i < 9; i++) {
        sprintf(room[i], "|                  |");
    }
    sprintf(room[9], "\\------------------/");

    room[y][x] = '@';

    

    for(i = 0; i < 10; i++) {
        printf("%s\n", room[i]);
    }

    key = getchar();

    if(key == "w") { y--; };
    // if(key == "w" || "W" || "Ц" || "ц") { y--; };
}