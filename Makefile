CC = gcc
CFLAGS = -Wall -Werror -g -O3
TARGET = translate

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

format:
	clang-format -style=file -i *.c

clean:
	rm -f $(TARGET)
