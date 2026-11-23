/* Реализация 2: решето Эратосфена и e как сумма ряда 1/n!, n = 0 .. x */
#define MATHLIB_BUILD
#include "contract.h"

#include <math.h>
#include <windows.h>

int PrimeCount(int A, int B)
{
    if (A < 1)
        A = 1;
    if (A > B || B < 2)
        return 0;

    /* sieve[i] = 1 - число i составное */
    HANDLE heap = GetProcessHeap();
    unsigned char *sieve = HeapAlloc(heap, HEAP_ZERO_MEMORY, (SIZE_T)B + 1);
    if (sieve == NULL)
        return -1;

    for (long long i = 2; i * i <= B; i++)
        if (!sieve[i])
            for (long long j = i * i; j <= B; j += i)
                sieve[j] = 1;

    int count = 0;
    for (long long n = (A < 2 ? 2 : A); n <= B; n++)
        count += !sieve[n];
    HeapFree(heap, 0, sieve);
    return count;
}

float E(int x)
{
    if (x < 0)
        return NAN;
    double sum = 1.0, term = 1.0;     /* слагаемое n = 0: 1/0! = 1 */
    for (int n = 1; n <= x; n++) {
        term /= n;                    /* 1/n! = 1/(n-1)! / n */
        sum += term;
    }
    return (float)sum;
}
