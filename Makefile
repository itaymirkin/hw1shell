CC = gcc
CFLAGS = -Wall -g
TARGET = hw1shell

all: $(TARGET)

$(TARGET): hw1shell.c
	$(CC) $(CFLAGS) -o $(TARGET) hw1shell.c

clean:
	rm -f $(TARGET)