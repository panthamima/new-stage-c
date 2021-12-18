#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

int main (void) {

    while (1)
    {   
        system("clear");
        
        time_t t = time(NULL);
        struct tm* aTm = localtime(&t);
        printf("%02d:%02d:%02d\n",aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
        sleep(1);
    }
    return 0;
}

