/* Программа №2: загружает библиотеки во время исполнения (LoadLibraryEx / GetProcAddress).
   О библиотеках программа знает только их относительные имена и контракт (contract.h).
   Команда 0 переключает mathlib1.dll <-> mathlib2.dll. */
#include "cli.h"
#include "common.h"

static const WCHAR *const LIBS[2] = { L"mathlib1.dll", L"mathlib2.dll" };

typedef struct {
    HMODULE h;
    int idx;          /* какая из LIBS загружена */
    Contract c;
} Library;

/* Загрузить библиотеку LIBS[idx] и найти в ней функции контракта. 0 - успех */
static int lib_open(int idx, Library *lib)
{
    /* имя относительное: ищем только в папке программы, а не по PATH и текущему каталогу */
    HMODULE h = LoadLibraryExW(LIBS[idx], NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
    if (h == NULL) {
        print_last_error("LoadLibraryEx");
        warn("error: cannot load %ls\n", LIBS[idx]);
        return -1;
    }
    PrimeCountFn pc = (PrimeCountFn)(void *)GetProcAddress(h, "PrimeCount");
    EFn e = (EFn)(void *)GetProcAddress(h, "E");
    if (pc == NULL || e == NULL) {
        print_last_error("GetProcAddress");
        warn("error: %ls does not implement the contract\n", LIBS[idx]);
        FreeLibrary(h);
        return -1;
    }
    lib->h = h;
    lib->idx = idx;
    lib->c.prime_count = pc;
    lib->c.e = e;
    return 0;
}

static int lib_close(Library *lib)
{
    if (lib->h == NULL)
        return 0;
    if (!FreeLibrary(lib->h)) {
        print_last_error("FreeLibrary");
        return -1;
    }
    lib->h = NULL;
    return 0;
}

/* Сначала грузим новую библиотеку и только потом выгружаем старую:
   если новую загрузить не удалось, программа продолжает работать со старой. */
static void lib_switch(Library *cur)
{
    Library next;
    if (lib_open(1 - cur->idx, &next) != 0) {
        warn("still using %ls\n", LIBS[cur->idx]);
        return;
    }
    lib_close(cur);
    *cur = next;
    say("switched to %ls\n", LIBS[cur->idx]);
}

int main(void)
{
    Library lib = { 0 };
    int ret = 0;

    if (lib_open(0, &lib) != 0)
        return 1;
    say("Commands: 0 - switch library, 1 A B - PrimeCount(A, B), 2 x - E(x), Ctrl+Z - exit\n");

    LineReader in;
    lr_init(&in, GetStdHandle(STD_INPUT_HANDLE));
    for (;;) {
        char *line;
        DWORD len;
        say("[%ls] > ", LIBS[lib.idx]);
        int rc = lr_getline(&in, &line, &len);
        if (rc == 0)
            break;
        if (rc < 0) {
            print_last_error("ReadFile(stdin)");
            ret = 1;
            break;
        }
        if (line[0] == '\x1A') {
            HeapFree(GetProcessHeap(), 0, line);
            break;
        }
        if (run_command(line, &lib.c) == CMD_SWITCH)
            lib_switch(&lib);
        HeapFree(GetProcessHeap(), 0, line);
    }
    if (lib_close(&lib) != 0)
        ret = 1;
    return ret;
}
