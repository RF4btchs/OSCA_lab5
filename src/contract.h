#ifndef CONTRACT_H
#define CONTRACT_H

/* Контракты варианта 18 (функции 3 и 6). Обе библиотеки, mathlib1.dll и mathlib2.dll,
   экспортируют одинаковые имена с одинаковыми сигнатурами, различается только реализация.

   PrimeCount(A, B) - количество простых чисел на отрезке [A, B] (A, B натуральные).
                      A > B -> 0; нехватка памяти -> -1.
   E(x)             - приближение числа e; некорректный x -> NAN. */

#ifdef MATHLIB_BUILD
#define MATHLIB_API __declspec(dllexport)
#else
#define MATHLIB_API __declspec(dllimport)
#endif

MATHLIB_API int PrimeCount(int A, int B);
MATHLIB_API float E(int x);

/* типы указателей на функции контракта - для загрузки во время исполнения (prog2) */
typedef int (*PrimeCountFn)(int A, int B);
typedef float (*EFn)(int x);

#endif
