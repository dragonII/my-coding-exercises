#include <stdio.h>
#include <stdlib.h>

#define myrandom(x) (int)random()%x

static const int ARRAY_MAX = 11;

void init_array(int a[], int array_max)
{
    int i;
    for(i = 0; i < array_max; i++)
    {
        a[i] = myrandom(ARRAY_MAX * 2);
        printf("%d ", a[i]);
    }
    printf("\n");
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

    swap(&a[center], &a[right]);
    return a[right];
}
      

void quicksort(int a[], int left, int right)
{
    if(right - left < 1)
        return;

    int i, j, k;
    //int pivotIndex = (left + right) / 2;
    //swap(&a[pivotIndex], &a[right]);
    //pivotIndex = right;

    int pivot = media3(a, left, right);

    i = left;
    j = right - 1;

    for(;;)
    {
        while(a[i] < pivot)
            i++;
        while(a[j] > pivot)
            j--;
        if(i < j)
            swap(&a[i], &a[j]);
        else
            break;
    }
    swap(&a[i], &a[right]);
    //pivotIndex = i;

    for(k = left; k <= right; k++)
        printf("%d ", a[k]);
    printf("\n");

    quicksort(a, left, i - 1);
    quicksort(a, i + 1, right);
}


int main()
{
    int i;
    int array[ARRAY_MAX];
    init_array(array, ARRAY_MAX);
    quicksort(array, 0, ARRAY_MAX - 1);
    for(i = 0; i < ARRAY_MAX; i++)
        printf("%d ", array[i]);
    printf("\n");
    return 0;
}


