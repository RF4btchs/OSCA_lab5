# ЛР 5. Динамические библиотеки (вариант 18: функции 3 и 6)

| Контракт | mathlib1.dll | mathlib2.dll |
|---|---|---|
| `int PrimeCount(int A, int B)` | наивный перебор делителей 2..n-1 | решето Эратосфена |
| `float E(int x)` | (1 + 1/x)^x | сумма 1/n!, n = 0..x |

Сборка: `mingw32-make` (в `bin\` появятся обе DLL, библиотеки импорта и две программы).

- `bin\prog1.exe` — mathlib1.dll подключена на этапе линковки (`-lmathlib1`), команды `1 A B`, `2 x`.
- `bin\prog2.exe` — библиотеки грузятся во время работы (`LoadLibraryEx` + `GetProcAddress`),
  команда `0` переключает mathlib1 ↔ mathlib2.

Выход — Ctrl+Z, Enter.

Трасса загрузки DLL: `powershell -ExecutionPolicy Bypass -File trace\trace.ps1`
→ `trace\syscalls_prog1.txt`, `trace\syscalls_prog2.txt`.

## Process Monitor (на защите)

`..\lab1\tools\ProcessMonitor\Procmon64.exe` (от администратора).
Filter: `Process Name is prog1.exe` и `Process Name is prog2.exe` → Include,
оставить Show Process and Thread Activity и Show File System Activity.

Что смотреть:
- prog1: `Load Image ...\bin\mathlib1.dll` сразу после `Process Start`, до первого `ReadFile` с консоли —
  DLL загрузил загрузчик ОС;
- prog2: `CreateFile` / `Load Image mathlib1.dll` только после старта; после ввода `0` — `Load Image mathlib2.dll`
  (выгрузка DLL в Procmon не видна, её показывает `trace\trace.ps1`: `LdrUnloadDll`, `NtUnmapViewOfSection`);
- если переименовать mathlib1.dll, prog1 падает с кодом 0xC0000135 (STATUS_DLL_NOT_FOUND),
  в Procmon видно `CreateFile ... NAME NOT FOUND`.
