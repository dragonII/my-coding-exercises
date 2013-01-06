#include <stdio.h>

void func(int a, ...)
{
    int *temp = &a;
    int i;
    temp++;
    for(i = 0; i < a; i++)
    {
        printf("%d\n", *temp);
        temp++;
    }
}

int main()
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    func(4, a, b, c, d);
    return 0;
}
