//test program for random numbers


#include <stdio.h>
#include <stdlib.h>



main() {

  int i;
  int l = 1;
  int h = 4;

  printf("the rand_max value is %d\n", RAND_MAX);
  for (i=0; i<100; i++)
   {
     //printf("Random number is %10d \n", rand());
     printf("RandomInteger between 1 and 4 %2d \n", RandomInteger(l,h));
   }
}



int RandomInteger( int low, int high) {

  int k;
  double d;
  
  
  d = (double) rand() / ( (double) RAND_MAX + 1);
  k = (int) (d* (high-low + 1));
  return (low + k);

  }


