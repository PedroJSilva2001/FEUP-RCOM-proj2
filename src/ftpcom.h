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
  char *text;
  unsigned int real_len;
  unsigned int text_len;
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

#define CMD_MNEM_LEN 4  // each mnemonic if 4 chars wide
#define CRLF 2
#define SP 1
#define NULL_CH 1
#define CMD_BASE_LEN (CMD_MNEM_LEN + SP + CRLF + NULL_CH)
#define CMD_BASE_LEN_NO_PARAM (CMD_MNEM_LEN + CRLF + NULL_CH)

int login(int ctrl_socket_fd, ftp_client_info *info);  // USER PASS

int enter_passive_mode(int ctrl_socket_fd, unsigned char *ip, unsigned char *port);

int retrieve(int socket_fd, ftp_client_info *info);

int send_command(int ctrl_socket_fd, char* command);

int send_command_fmt(int ctrl_socket_fd, const char *format, int format_len, char *param);

int save_file(int socket_fd, char* filename);

void create_reply(ftp_reply *reply);

void concat_to_reply(ftp_reply *reply, char *str, int n);

void free_reply(ftp_reply *reply);

int read_reply(int ctrl_socket_fd, ftp_reply *reply);

int assert_valid_code(char *code, char **valid_codes, int n);

void dump_and_free(ftp_reply *reply);

#endif