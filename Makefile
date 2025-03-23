EXE=translate

$(EXE): main.c
	cc -O3 -Wall -o $(EXE) $<

format:
	clang-format -style=file -i *.c
