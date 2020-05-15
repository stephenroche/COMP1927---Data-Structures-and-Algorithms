// generate a list of alternating integers
//

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int max, i;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s #values\n", argv[0]);
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

	for (i = 0; i < max; i++)
        if (i % 2 == 0) {
            printf("7641\n");
        } else {
            printf("1467\n");
        }
	return 0;
}
