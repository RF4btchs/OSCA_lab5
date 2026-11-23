#ifndef COMMON_H
#define COMMON_H

#include <windows.h>

/* Вывести сообщение об ошибке WinAPI (GetLastError) в stderr */
void print_last_error(const char *what);

/* printf (текст в UTF-8) в STD_OUTPUT_HANDLE / STD_ERROR_HANDLE.
   В консоль пишем через WriteConsoleW, чтобы нормально выводился русский текст.
   Вся строка уходит одним вызовом, чтобы не перемешиваться с выводом других процессов. */
void say_to(DWORD std_handle, const char *fmt, ...);
#define say(...)  say_to(STD_OUTPUT_HANDLE, __VA_ARGS__)
#define warn(...) say_to(STD_ERROR_HANDLE, __VA_ARGS__)

/* Записать в handle все len байт. 0 - успех, -1 - ошибка */
int write_all(HANDLE h, const char *buf, DWORD len);

/* Построчное чтение из handle. Из консоли читаем ReadConsoleW и переводим в UTF-8,
   из файла или pipe - ReadFile (данные уже в UTF-8). */
typedef struct {
    HANDLE h;
    int    console;
    char   buf[4096];
    DWORD  pos, len;
} LineReader;

void lr_init(LineReader *lr, HANDLE h);

/* Читает строку произвольной длины без '\n' и '\r'.
   *line выделяется через HeapAlloc, освобождать HeapFree(GetProcessHeap(), 0, *line).
   *len - длина строки в байтах.
   Возвращает 1 - строка прочитана, 0 - конец потока, -1 - ошибка. */
int lr_getline(LineReader *lr, char **line, DWORD *len);

#endif
