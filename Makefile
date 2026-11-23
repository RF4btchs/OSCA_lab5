CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2
BIN     = bin

all: $(BIN)/mathlib1.dll $(BIN)/mathlib2.dll $(BIN)/prog1.exe $(BIN)/prog2.exe

# -shared: собрать DLL; --out-implib: заодно библиотека импорта для линковки prog1
$(BIN)/mathlib1.dll: src/mathlib1.c src/contract.h | $(BIN)
	$(CC) $(CFLAGS) -shared -o $@ src/mathlib1.c -Wl,--out-implib,$(BIN)/libmathlib1.dll.a

$(BIN)/mathlib2.dll: src/mathlib2.c src/contract.h | $(BIN)
	$(CC) $(CFLAGS) -shared -o $@ src/mathlib2.c -Wl,--out-implib,$(BIN)/libmathlib2.dll.a

# программа 1: линкуется с mathlib1 (импорт PrimeCount и E попадает в таблицу импорта)
$(BIN)/prog1.exe: src/prog1.c src/cli.c src/common.c src/cli.h src/common.h src/contract.h $(BIN)/mathlib1.dll
	$(CC) $(CFLAGS) -o $@ src/prog1.c src/cli.c src/common.c -L$(BIN) -lmathlib1

# программа 2: ни с какой библиотекой не линкуется, грузит их сама через LoadLibraryEx
$(BIN)/prog2.exe: src/prog2.c src/cli.c src/common.c src/cli.h src/common.h src/contract.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ src/prog2.c src/cli.c src/common.c

# папка создаётся, только если её ещё нет (иначе make -B падает)
$(BIN):
	$(if $(wildcard $(BIN)),,mkdir $(BIN))

clean:
	-del /q $(BIN)\*.exe $(BIN)\*.dll $(BIN)\*.a

.PHONY: all clean
