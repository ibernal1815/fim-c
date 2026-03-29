CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LIBS = -lssl -lcrypto

SRC_DIR = src
OBJ_DIR = obj
BIN = fim

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(OBJ_DIR) $(BIN)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN)

test:
	bash tests/test_hash.sh
	bash tests/test_baseline.sh
	bash tests/test_tamper.sh

.PHONY: all clean test
