#include <stdio.h>
#include <time.h>

int main (void)
{
  clock_t rowtime;
  struct tm * timeinfo;

  time ( &rowtime );
  timeinfo = localtime ( &rowtime );
  printf ( "Current local time and date: %s", asctime (timeinfo) );

  return 0;
}