#ifndef FTPCOM_H
#define FTPCOM_H

#include "ftpconn.h"

#include <stdio.h>

#define CODE_SIZE 3
#define LINE_START_SIZE 4
#define BASE_REPLY_LEN 100
#define SERVER_DISC 1
#define READ_ERROR 2
#define IMPL_ERROR 3

#define LINE_SEPARATOR(line) (line[3])
#define VALID_LINE_START(line) isdigit(line[0]) && isdigit(line[1]) \
                               && isdigit(line[2]) && (line[3] == ' ' \
                               || line[3] == '-')

typedef struct {
  char *buf;
  int real_size;
  int strlen;
  char code[CODE_SIZE+1];
} ftp_reply;

typedef enum {
  START,
  AWAITING_LINE_START,
  REPLY_COMPLETE,
  AWAITING_NEWLINE,
  AWAITING_END_NEWLINE,
} ftp_reply_state;

#define MAX_SIZE 1024

int login(int socket_fd, ftp_client_info *info);  // USER PASS

//int enter_passive_mode(int ctrl_socket_fd);  // send PASV\r\n (without space in the middle)

int retrieve(int socket_fd, ftp_client_info *info);

int send_command(int socket_fd, char* message);

int save_file(int socket_fd, char* filename);

ftp_reply *create_reply();

void concat_to_reply(ftp_reply *reply, char *str, int n);

int read_reply(int ctrl_socket_fd, ftp_reply **reply);

#endif