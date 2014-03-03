#include "util.h"
#include <sys/time.h>
#include <stdio.h>

unsigned long start_sec = 0; 
unsigned long start_msec = 0;  

unsigned long current_time()
{
  struct timeval tp;

  /* Return the time, in milliseconds, elapsed after the first call
   * to this routine.
   */  
  gettimeofday(&tp, NULL);
  if (!start_sec) {
    start_sec = tp.tv_sec;
    start_msec = (unsigned long) (tp.tv_usec / 1000);
  }
  return (1000 * (tp.tv_sec - start_sec) 
	 + (tp.tv_usec / 1000 - start_msec));
}

