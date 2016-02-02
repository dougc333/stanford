
#include <stdio.h> /* for printf */

int main(void)
{
	double d1, d2, d3, d4;
	d1 = 0.2;
	d2 = 1.0/5.0;
	d3 = .02/0.1;
	d4 = 0.3 - 0.1; 
	printf("Does %g == %g? %d\n", d1, d2, d1 == d2);
	printf("Does %g == %g? %d\n", d1, d3, d1 == d3);
	printf("Does %g == %g? %d\n", d1, d4, d1 == d4);
	return 0;
}
