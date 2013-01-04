#include <stdio.h>
#include <stdlib.h>

#define myrandom(x) (int)random()%x

//#define ARRAY_MAX 8
#define ARRAY_MAX 100

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

int partition(int a[], int low, int high)
{
    int i, j;

    int pivotIndex = low;
    int pivotValue = a[pivotIndex];

    i = low;
    j = high;

    while(i < j)
    {
        while(i < j && a[j] >= pivotValue) {j--;}
        a[i] = a[j];

        while(i < j && a[i] <= pivotValue) {i++;}
        a[j] = a[i];
    }
    a[i] = pivotValue;
    return i;
}

int partition_swap(int a[], int low, int high)
{
    int i, j;

    int pivotIndex = low;
    int pivotValue = a[pivotIndex];

    i = low;
    j = high;

    while(i < j)
    {
        while(i < j && a[j] >= pivotValue) {j--;}
        swap(&a[j], &a[pivotIndex]);
        pivotIndex = j;

        while(i < j && a[i] <= pivotValue) {i++;}
        swap(&a[i], &a[pivotIndex]);
        pivotIndex = i;
    }
    return i;
}


void quicksort(int a[], int low, int high)
{
    if(high == low)
        return;

    //int pivotLoc = partition(a, low, high);
    int pivotLoc = partition_swap(a, low, high);

    printf("pivotLoc = %d\n", pivotLoc);

    if(pivotLoc > low)
        quicksort(a, low, pivotLoc - 1);

    if(pivotLoc < high)
        quicksort(a, pivotLoc + 1, high);
}


int main()
{
    //int array[ARRAY_MAX] = {49, 38, 65, 97, 76, 13, 27, 49};

    int array[ARRAY_MAX];
    init_array(array, ARRAY_MAX);
    print_array(array, 0, ARRAY_MAX - 1);

    quicksort(array, 0, ARRAY_MAX - 1);

    print_array(array, 0, ARRAY_MAX - 1);

    return 0;
}


