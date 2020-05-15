#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	int i = 1;
    int n;
    int a[] = {10,20,30};

    n = a[i++];

    printf("%d, %d\n", n, i);
	
	return 0;
}
