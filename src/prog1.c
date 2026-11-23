/* Программа №1: использует mathlib1.dll, подключённую на этапе линковки.
   PrimeCount и E записаны в таблицу импорта prog1.exe (через библиотеку импорта
   libmathlib1.dll.a), DLL загружает загрузчик ОС при старте процесса. */
#include "cli.h"
#include "common.h"

int main(void)
{
    Contract c = { PrimeCount, E };   /* адреса из таблицы импорта (IAT) */
    LineReader in;

    say("Commands: 1 A B - PrimeCount(A, B), 2 x - E(x), Ctrl+Z - exit\n");
    lr_init(&in, GetStdHandle(STD_INPUT_HANDLE));
    for (;;) {
        char *line;
        DWORD len;
        say("> ");
        int rc = lr_getline(&in, &line, &len);
        if (rc == 0)
            break;
        if (rc < 0) {
            print_last_error("ReadFile(stdin)");
            return 1;
        }
        if (line[0] == '\x1A') {          /* Ctrl+Z в консоли - конец ввода */
            HeapFree(GetProcessHeap(), 0, line);
            break;
        }
        if (run_command(line, &c) == CMD_SWITCH)
            warn("unknown command; use: 1 A B | 2 x\n");
        HeapFree(GetProcessHeap(), 0, line);
    }
    return 0;
}
