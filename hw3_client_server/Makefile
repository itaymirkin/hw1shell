CC = gcc
CFLAGS = -Wall -pthread
TARGET_SERVER = hw3server
TARGET_CLIENT = hw3client
SRC_SERVER = hw3server.c
SRC_CLIENT = hw3client.c

# Default target (build both server and client)
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Build the server executable
$(TARGET_SERVER): $(SRC_SERVER)
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(SRC_SERVER)

# Build the client executable
$(TARGET_CLIENT): $(SRC_CLIENT)
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) $(SRC_CLIENT)

# Clean up the compiled files
clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT)

.PHONY: all clean
