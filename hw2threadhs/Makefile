CC = gcc
CFLAGS = -Wall -g -pthread
SRC =  hw1shell.c 
OBJ = $(SRC:.c=.o)
TARGET = hw2

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)