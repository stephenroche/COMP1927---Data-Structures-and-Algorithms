// randList.c - generate a list of random integers
// Written by John Shepherd, July 2008
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
	int max, i;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s #values [seed]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	max = atoi(argv[1]);
	if (max < 1) {
		fprintf(stderr, "%s: too few values\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if (max > 10000000) {
		fprintf(stderr, "%s: too many values\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if (argc == 3) 
		srand(atoi(argv[2]));
	else
		srand(time(NULL)); // really random

	for (i = 0; i < max; i++)
      printf("%d    %c%c%c\n", 1+rand()%(max*2), 'a'+rand()%26, 'a'+rand()%26, 'a'+rand()%26);
      //printf("%d\n", 1+rand()%(max*2));
	return 0;
}
