CC=gcc
SRC_DIR=src
BIN_DIR=bin

CFLAGS = -Wall

SRC_FILES =  ${SRC_DIR}/main.c ${SRC_DIR}/ftpconn.c ${SRC_DIR}/ftpcom.c 

make: ${SRC_FILES}
	  @mkdir -p $(BIN_DIR)
	  @$(CC) -o $(BIN_DIR)/download ${SRC_FILES} ${CFLAGS}

clean:
	  @rm -f $(BIN_DIR)/*
	  @rmdir $(BIN_DIR)
	  