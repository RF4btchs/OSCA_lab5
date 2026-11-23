#include "cli.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "common.h"

static const char *skip_spaces(const char *s)
{
    while (*s == ' ' || *s == '\t')
        s++;
    return s;
}

/* Прочитать одно целое число. 0 - успех, -1 - не число или не влезает в int */
static int parse_int(const char **s, int *out)
{
    const char *p = skip_spaces(*s);
    char *end;
    errno = 0;
    long v = strtol(p, &end, 10);
    if (end == p || (*end != '\0' && *end != ' ' && *end != '\t'))
        return -1;
    if (errno == ERANGE || v < INT_MIN || v > INT_MAX)
        return -1;
    *out = (int)v;
    *s = end;
    return 0;
}

/* Разобрать ровно n целых аргументов, после них в строке ничего быть не должно */
static int parse_args(const char *s, int *args, int n)
{
    for (int i = 0; i < n; i++)
        if (parse_int(&s, &args[i]) != 0)
            return -1;
    return *skip_spaces(s) == '\0' ? 0 : -1;
}

int run_command(const char *line, const Contract *c)
{
    const char *s = skip_spaces(line);
    char cmd = *s;
    int a[2];

    if (cmd == '\0')
        return CMD_BAD;
    s++;
    if (*s != '\0' && *s != ' ' && *s != '\t')
        cmd = '?';   /* "12 5" - это не команда 1 */

    switch (cmd) {
    case '0':
        if (*skip_spaces(s) != '\0')
            break;
        return CMD_SWITCH;
    case '1':
        if (parse_args(s, a, 2) != 0) {
            warn("usage: 1 A B   (A, B - natural numbers)\n");
            return CMD_BAD;
        }
        if (a[0] < 1 || a[1] < 1) {
            warn("error: A and B must be natural (>= 1)\n");
            return CMD_BAD;
        }
        int cnt = c->prime_count(a[0], a[1]);
        if (cnt < 0) {
            warn("error: PrimeCount(%d, %d): not enough memory\n", a[0], a[1]);
            return CMD_BAD;
        }
        say("PrimeCount(%d, %d) = %d\n", a[0], a[1], cnt);
        return CMD_DONE;
    case '2':
        if (parse_args(s, a, 1) != 0) {
            warn("usage: 2 x   (x - integer)\n");
            return CMD_BAD;
        }
        float e = c->e(a[0]);
        if (isnan(e))
            say("E(%d) is undefined for this implementation\n", a[0]);
        else
            say("E(%d) = %.7f\n", a[0], (double)e);
        return CMD_DONE;
    }
    warn("unknown command; use: 0 | 1 A B | 2 x\n");
    return CMD_BAD;
}
