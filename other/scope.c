#include <stdio.h>
int main(void)
{ 
    char x = 'm';
    printf("%c\n", x);
    {
        printf("%c\n", x);
        char x = 'b';
        printf("%c\n", x);
    }
    printf("%c\n", x);
}
