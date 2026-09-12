CC := gcc
CFLAGS := -Wall -Wextra -O2
LDFLAGS := -lm

PARSER_SRC := lexer_parser.c
PARSER_BIN := euler_parser.exe

RUNTIME_SRC := euler_runtime.c
RUNTIME_BIN := euler_runtime.h

TEMP_C := temp.c

.PHONY: all clean

all: $(PARSER_BIN)

$(PARSER_BIN): $(PARSER_SRC)
	@echo [BUILD] Compiling Euler transpiler...
	$(CC) $(CFLAGS) $< -o $@

run: $(PARSER_BIN) $(RUNTIME_SRC) $(RUNTIME_HDR)
	@echo [1/4] Transpiling $(FILE)...
	@./$(PARSER_BIN) $(FILE) > $(TEMP_C)
	@echo [2/4] Compiling native binary...
	@$(CC) $(CFLAGS) $(TEMP_C) $(RUNTIME_SRC) -o program.exe $(LDFLAGS)
	@echo [3/4] Cleaning intermediate files...
	@if exist $(TEMP_C) del /F /Q $(TEMP_C)
	@echo [4/4] Running program...
	@echo ------------------------------------------------------------
	@program.exe
	@echo ------------------------------------------------------------
clean:
	@if exist $(PARSER_BIN) del /F /Q $(PARSER_BIN)
	@if exist $(TEMP_C) del /F /Q $(TEMP_C)
	@if exist *.exe del /F /Q *.exe