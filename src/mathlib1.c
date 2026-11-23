/* Реализация 1: наивный подсчёт простых чисел и e = (1 + 1/x)^x */
#define MATHLIB_BUILD
#include "contract.h"

#include <math.h>

/* Наивный алгоритм: n простое, если не делится ни на одно из чисел 2 .. n-1 */
static int is_prime_naive(int n)
{
    if (n < 2)
        return 0;
    for (int d = 2; d < n; d++)
        if (n % d == 0)
            return 0;
    return 1;
}

int PrimeCount(int A, int B)
{
    int count = 0;
    if (A < 1)
        A = 1;
    /* long long, чтобы при B = INT_MAX счётчик цикла не переполнился */
    for (long long n = A; n <= B; n++)
        count += is_prime_naive((int)n);
    return count;
}

float E(int x)
{
    if (x < 1)
        return NAN;
    /* в double: в float 1 + 1/x теряет точность при больших x */
    return (float)pow(1.0 + 1.0 / x, x);
}
