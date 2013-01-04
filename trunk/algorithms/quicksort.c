#include <stdio.h>
#include <stdlib.h>

#define myrandom(x) (int)random()%x

static const int ARRAY_MAX = 100;

void init_array(int a[], int array_max)
{
    int i;
    for(i = 0; i < array_max; i++)
    {
        a[i] = myrandom(ARRAY_MAX * 2);
    }
}

void print_array(int a[], int left, int right)
{
    int i;
    printf("from [%d] to [%d]\n", left, right);
    for(i = left; i <= right; i++)
        printf("%d, ", a[i]);
    printf("\n\n");
    return;
}

void swap(int* a, int* b)
{
    int tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

int media3(int a[], int left, int right)
{
    int center = (left + right) / 2;

    if(a[left] > a[center])
        swap(&a[left], &a[center]);
    if(a[left] > a[right])
        swap(&a[left], &a[right]);
    if(a[center] > a[right])
        swap(&a[center], &a[right]);

    swap(&a[center], &a[right - 1]);
    return a[right - 1];
}
      

void quicksort(int a[], int left, int right)
{
    if(right == left)
        return;

    int i, j;

    int pivot = media3(a, left, right);

    i = left;
    j = right - 1;

    for(;;)
    {
        while(a[++i] < pivot) {}
        while(a[--j] > pivot) {}
        if(i < j)
            swap(&a[i], &a[j]);
        else
            break;
    }
    swap(&a[i], &a[right - 1]);

    if(i > left)
        quicksort(a, left, i - 1);
    if(right > i)
        quicksort(a, i + 1, right);
}


int main()
{
    int array[ARRAY_MAX];

    init_array(array, ARRAY_MAX);
    print_array(array, 0, ARRAY_MAX - 1);

    quicksort(array, 0, ARRAY_MAX - 1);

    print_array(array, 0, ARRAY_MAX - 1);

    return 0;
}


