CC=gcc
SRC_DIR=src
BIN_DIR=bin

CFLAGS = -Wall -g -o -lm

FILES = $(SRC_DIR)/ftpclient.c $(SRC_DIR)/main.c

make: ${FILES}
		@mkdir -p $(BIN_DIR)
		@$(CC) -o ${CFLAGS} $(BIN_DIR)/download ${FILES}

clean:
		@rm -f {BIN_DIR}/*