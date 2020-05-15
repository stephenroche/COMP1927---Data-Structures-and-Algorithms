#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {

   char *array = malloc(12 * sizeof(char));
   strcpy(array, "abcdefghijk");
   int i;

   //strcpy(&array[7], "h");
   array[8] = '!';

   for (i = 0; i < 12/sizeof(char); i++) {
      printf("%2d: %c %3d %2x\n", i, array[i], array[i], array[i]);
   }

   int *array2 = (int *)array;

   for (i = 0; i < 12/sizeof(int); i++) {
      printf("%2d: %c %10d %8x\n", i, array2[i], array2[i], array2[i]);
   }

   //printf("%d\n", sizeof(float));

   float *array3 = (float *)array2;

   for (i = 0; i < 12/sizeof(float); i++) {
      printf("%2d: %c %f\n", i, array[i*sizeof(float)], array3[i]);
   }

   return 0;
}
