CC = gcc
CFLAGS = -Wall -Wextra -Werror -I./include
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests

SRCS = $(filter-out $(SRC_DIR)/client_example.c, $(wildcard $(SRC_DIR)/*.c))
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
SERVER_TARGET = $(BIN_DIR)/cileserver
CLIENT_TARGET = $(BIN_DIR)/cileclient

.PHONY: all clean test client

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

$(CLIENT_TARGET): $(OBJ_DIR)/client_example.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

test:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/test_runner $(TEST_DIR)/*.c $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) -lpthread
	$(BIN_DIR)/test_runner 