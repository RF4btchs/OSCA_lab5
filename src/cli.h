#ifndef CLI_H
#define CLI_H

#include "contract.h"

/* Набор функций контракта, через который работают обе программы */
typedef struct {
    PrimeCountFn prime_count;
    EFn e;
} Contract;

enum { CMD_SWITCH = 0, CMD_DONE = 1, CMD_BAD = -1 };

/* Разобрать строку пользователя и выполнить её:
   "1 A B" -> PrimeCount(A, B), "2 x" -> E(x), результат печатается.
   Возвращает CMD_DONE, CMD_SWITCH (введён "0") или CMD_BAD (ошибка ввода, сообщение уже выведено). */
int run_command(const char *line, const Contract *c);

#endif
