#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

int main(void) {

   /*for (int i = 0; i < 10; i++) {
      usleep(1000000);
      printf("%d\n", i);
      printf("%ld\n", time(0));
      
      if (getchar() != EOF) {
         break;
      }
   }*/

   double t, x, y;
   long radDiff;

   for (t = 0; t < 50; t++) {
      for (y = 0; y <= 40; y++) {
         for (x = 0; x <= 40; x++) {
            //printf("%lf\n", (sqrt((x-20)*(x-20) + (y-20)*(y-20) - t*t)));
            radDiff = (sqrt((x-20)*(x-20) + (y-20)*(y-20)) - t + 1000);
            if (radDiff % 10 < 5) {
               printf("# ");
            } else {
               printf("  ");
            }
         }
         printf("\n");
      }
      printf("\n");
      usleep(50000);
   }

   return 0;
}
