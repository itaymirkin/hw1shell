CC = gcc
CFLAGS = -Wall -pthread
TARGET = hw3server
SRC = hw3server.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean