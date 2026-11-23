#include "common.h"

#include <stdarg.h>
#include <stdio.h>

void print_last_error(const char *what)
{
    DWORD err = GetLastError();
    char msg[512] = "";

    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                   msg, sizeof(msg), NULL);
    warn("error: %s failed (code %lu): %s", what, err, msg[0] ? msg : "\n");
}

void say_to(DWORD std_handle, const char *fmt, ...)
{
    char buf[2048];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n < 0)
        return;
    if (n >= (int)sizeof(buf))
        n = sizeof(buf) - 1;

    HANDLE h = GetStdHandle(std_handle);
    DWORD mode;
    if (GetConsoleMode(h, &mode)) {
        WCHAR wbuf[2048];
        int wn = MultiByteToWideChar(CP_UTF8, 0, buf, n, wbuf, 2048);
        DWORD written;
        if (wn > 0)
            WriteConsoleW(h, wbuf, (DWORD)wn, &written, NULL);
    } else {
        write_all(h, buf, (DWORD)n);
    }
}

int write_all(HANDLE h, const char *buf, DWORD len)
{
    while (len > 0) {
        DWORD written = 0;
        if (!WriteFile(h, buf, len, &written, NULL))
            return -1;
        buf += written;
        len -= written;
    }
    return 0;
}

void lr_init(LineReader *lr, HANDLE h)
{
    DWORD mode;
    lr->h = h;
    lr->console = GetConsoleMode(h, &mode) != 0;
    lr->pos = lr->len = 0;
}

/* Дочитать порцию данных в lr->buf. Возвращает число байт, 0 - конец, -1 - ошибка */
static int lr_fill(LineReader *lr)
{
    DWORD rd = 0;
    if (lr->console) {
        WCHAR w[1024];   /* 1024 символа в UTF-8 занимают не больше 3 КБ */
        if (!ReadConsoleW(lr->h, w, 1024, &rd, NULL))
            return -1;
        if (rd == 0)
            return 0;
        int n = WideCharToMultiByte(CP_UTF8, 0, w, (int)rd, lr->buf, sizeof(lr->buf), NULL, NULL);
        if (n <= 0)
            return -1;
        rd = (DWORD)n;
    } else if (!ReadFile(lr->h, lr->buf, sizeof(lr->buf), &rd, NULL)) {
        /* у pipe конец данных выглядит как ERROR_BROKEN_PIPE */
        if (GetLastError() != ERROR_BROKEN_PIPE)
            return -1;
        rd = 0;
    }
    lr->pos = 0;
    lr->len = rd;
    return (int)rd;
}

int lr_getline(LineReader *lr, char **line, DWORD *len)
{
    HANDLE heap = GetProcessHeap();
    SIZE_T cap = 64, n = 0;
    char *s = HeapAlloc(heap, 0, cap);
    int got_any = 0;

    if (s == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return -1;
    }
    for (;;) {
        if (lr->pos == lr->len) {
            int rc = lr_fill(lr);
            if (rc < 0) {
                HeapFree(heap, 0, s);
                return -1;
            }
            if (rc == 0) {
                if (!got_any) {
                    HeapFree(heap, 0, s);
                    return 0;
                }
                break;
            }
        }
        char c = lr->buf[lr->pos++];
        got_any = 1;
        if (c == '\n')
            break;
        if (c == '\r')
            continue;
        if (n + 2 >= cap) {   /* +2: место под '\n' и '\0' */
            char *t = HeapReAlloc(heap, 0, s, cap * 2);
            if (t == NULL) {
                HeapFree(heap, 0, s);
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return -1;
            }
            s = t;
            cap *= 2;
        }
        s[n++] = c;
    }
    s[n] = '\0';
    *line = s;
    *len = (DWORD)n;
    return 1;
}
