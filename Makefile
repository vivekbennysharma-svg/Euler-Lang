CC := gcc
CFLAGS := -Wall -Wextra -O2
LDFLAGS := -lm

PARSER_SRC := lexer_parser.c
PARSER_BIN := euler_parser.exe

RUNTIME_SRC := euler_runtime.c
RUNTIME_BIN := euler_runtime.exe

TEMP_C := temp.c

.PHONY: all clean help

all: $(PARSER_BIN)

$(PARSER_BIN): $(PARSER_SRC)
	@echo [BUILD] Compiling Euler transpiler ($@)...
	$(CC) $(CFLAGS) $< -o $@

%.eul: $(PARSER_BIN) $(RUNTIME_BIN) $(RUNTIME_HDR)
	@echo [1/4] Transpiling $@ to intermediate C...
	@./$(PARSER_BIN) $@ > $(TEMP_C)
	@echo [2/4] Compiling native binary ($*.exe)...
	@$(CC) $(CFLAGS) $(TEMP_C) $(RUNTIME_SRC) -o $*.exe $(LDFLAGS)
	@echo [3/4] Cleaning up intermediate files...
	@if exist $(TEMP_C) del /F /Q $(TEMP_C)
	@echo [4/4] Running generated binary ($*.exe)...
	@echo --------------------------------------------------------
	@./$*.exe
	@echo --------------------------------------------------------

clean:
	@echo [CLEAN] Removing generated binaries and artifacts...
	@if exist $(PARSER_BIN) del /F /Q $(PARSER_BIN)
	@if exist $(TEMP_C) del /F /Q $(TEMP_C)
	@if exist *.exe del /F /Q *.exe
	@if exist *.o del /F /Q *.o
	@echo [CLEAN] Directory clean.
help:
	@echo Euler Language Build System
	@echo Usage:
	@echo   mingw32-make <file>.eul
	@echo   mingw32-make all
	@echo   mingw32-make clean