#include "ftpcom.h"
#include "ftpconn.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>

int login(int ctrl_socket_fd, ftp_client_info *info) {
  char *user = info->user == NULL? "anonymous" : info->user;

  if (send_command_fm(ctrl_socket_fd, "USER %s\r\n", CMD_BASE_LEN, user) == -1) {
    printf("error: could not send USER command to host\n");
	  return 1;
  }
 
  ftp_reply reply_user;
  int err = read_reply(ctrl_socket_fd, &reply_user);
  printf("%s", reply_user.text);
  if (err) { return err; }

  char *codes_user[7] = {"230", "331", "332", "421", "500", "501", "530"};

  if (assert_valid_code(reply_user.code, codes_user, 7)) {
    printf("error: host has sent a reply that does not match any valid one expected; host might be implementing incorrectly the FTP standard\n");
    printf("reply sent from host:\n%s", reply_user.text);
    free_reply(&reply_user);
	  return 1;
  }

  if (reply_user.code[0] == 5 || reply_user.code[0] == 4 || strcmp(reply_user.code, "332") == 0) {	
	  printf("error: something has gone wrong in FTP comunication with host while logging in\n");
    printf("reply sent from host:\n%s", reply_user.text);
    free_reply(&reply_user);
  	return 1;
  }

  if (strcmp(reply_user.code, "230") == 0) {
    if (info->pass != NULL) {
      printf("warning: a password was specified by user but not needed for log in\n");
    }
    free_reply(&reply_user);
	  return 0;
  }

  free_reply(&reply_user);

  char *pass = info->pass == NULL? " " : info->pass;

  if (send_command_fm(ctrl_socket_fd, "PASS %s\r\n", CMD_BASE_LEN, pass) == -1) {
    printf("error: could not send PASS command to host\n");
  }

  ftp_reply reply_pass;
  err = read_reply(ctrl_socket_fd, &reply_pass);
  printf("%s", reply_pass.text);
  if (err) { return err; }


  char *codes_pass[8] = {"202", "230", "332", "421", "500", "501", "503", "530"};

  if (assert_valid_code(reply_pass.code, codes_pass, 8)) {
    printf("error: host has sent a reply that does not match any valid one expected; host might be implementing incorrectly the FTP standard\n");
    printf("reply sent from host:\n%s", reply_pass.text);
    free_reply(&reply_pass);
	  return 1;
  }

  
  if (reply_pass.code[0] == 5 || reply_pass.code[0] == 4 || reply_pass.code[0] == 3) {	
	  printf("error: something has gone wrong in FTP comunication with host while logging in\n");
    printf("reply sent from host:\n%s", reply_pass.text);
    free_reply(&reply_pass);
    return 1;
  }

  free_reply(&reply_pass);

  return 0;
}

int enter_passive_mode(int socket_fd) {
  // TODO
  if (send_command(socket_fd, "pasv\r\n")) return 1;

  // TODO: RESPONSE READ -> resulta num buf

  // PORT h1,h2,h3,h4,p1,p2
  int ip[4], ports[2];

  //if (scanf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).", &ip[0], &ip[1], &ip[2], &ip[3], &ports[0], &ports[1]) < 0) return -1;

  int port = ports[0] * 256 + ports[1];

  // TODO: CONNECT SOCKET
  return 0;
}

int retrieve(int socket_fd, ftp_client_info *info) {
  char command[MAX_SIZE];
  sprintf(command, "retr %s\r\n", info->path);

  if (send_command(socket_fd, command)) return -1;

  // TODO: RESPONSE READ

  return 0;
}

int send_command(int ctrl_socket_fd, char* command) {
  int bytes = send(ctrl_socket_fd, command, strlen(command), 0);

  if (bytes < 0) {
    fprintf(stderr, "send: %s\n", strerror(errno));
    return 1;
  }
  // TODO: maybe enforce that all bytes are sent
  return 0;
}

int send_command_fm(int ctrl_socket_fd, const char *format, int format_len, char *param) {
  char *command = malloc(sizeof(char) * (format_len + strlen(param)));  
  sprintf(command, format, param);  	

  int err = send_command(ctrl_socket_fd, command);

  free(command);

  return err;
}

int save_file(int socket_fd, char* filename) {
  int file_fd = open(filename, O_WRONLY | O_CREAT, 0777);

  if (file_fd < 0)
    return -1;

  int bytes;
  char buf[MAX_SIZE];

  while ((bytes = read(socket_fd, buf, sizeof(buf))) > 0) {
      if (write(file_fd, buf, bytes) < 0) {
        return -1;
      }
  }

  close(file_fd);
  return 0;
}

void create_reply(ftp_reply *reply) {  
  reply->text = calloc(BASE_REPLY_LEN+1, sizeof(char));
  memset(reply->code, 0, CODE_SIZE+1);
  reply->real_len = BASE_REPLY_LEN+1;
  reply->text_len = 0;
}

void free_reply(ftp_reply *reply) {
  free(reply->text);
}

void concat_to_reply(ftp_reply *reply, char *str, int n) {
  if (reply->text_len+n >= reply->real_len) {  // >=
    reply->text = realloc(reply->text, 2*(reply->text_len+n));
    reply->real_len = 2*(reply->text_len+n);
  }

  strncat(reply->text, str, n);
  reply->text_len += n;
}

int nl_1st_occurence_offset(char *buf, int n) {
  for (int i = 0; i < n; i++) {
    if (buf[i] == '\n') {
      return i;
    }
  }

  return -1;
}

int read_reply(int ctrl_socket_fd, ftp_reply *reply) {
  create_reply(reply);
  ftp_reply_state reply_state = START;

  char lstart[LINE_START_SIZE+1];
  memset(lstart, 0, LINE_START_SIZE+1);
  int start_count = 0;

  char buf[BASE_REPLY_LEN];

  while (reply_state != REPLY_COMPLETE) {
    memset(buf, 0, BASE_REPLY_LEN);

    int bytes = recv(ctrl_socket_fd, buf, BASE_REPLY_LEN, 0);

    if (bytes == 0) { return SERVER_DISC; }

    if (bytes == -1) { return READ_ERROR; }

    for (ssize_t i = 0; i < bytes; i++) {
      if (reply_state == START) {
        if (start_count == CODE_SIZE) {

          lstart[start_count] = buf[i];

          // return error if we find more bytes after last newline 
          // (violation of ftp standard)
          if (!VALID_LINE_START(lstart)) { return IMPL_ERROR; }

          reply_state = LINE_SEPARATOR(lstart) == ' '?
                            AWAITING_END_NEWLINE : AWAITING_NEWLINE;
          start_count = 0;
          memcpy(reply->code, lstart, CODE_SIZE);
        } else {
          lstart[start_count++] = buf[i];
        }

        concat_to_reply(reply, &buf[i], 1);
      }

      else if (reply_state == AWAITING_LINE_START) {
        if (start_count == CODE_SIZE) {
          lstart[start_count] = buf[i];
          if (VALID_LINE_START(lstart) && LINE_SEPARATOR(lstart) == ' '
                && (strncmp(reply->code, lstart, CODE_SIZE) == 0)) {
            reply_state = AWAITING_END_NEWLINE;
          } else {
            reply_state = AWAITING_NEWLINE;
            start_count = 0;
          }
        } else {
          lstart[start_count++] = buf[i];
        }

        concat_to_reply(reply, &buf[i], 1);
      }

      else if (reply_state == AWAITING_NEWLINE) {
        int off = nl_1st_occurence_offset(&buf[i], bytes-i);

        // add all chars because we didnt find newline
        if (off == -1) {
          concat_to_reply(reply, &buf[i], bytes-i);
          break;
        }

        // add chars up to newline
        concat_to_reply(reply, &buf[i], off+1);
        reply_state = AWAITING_LINE_START;
        i += off;
      }

      else if (reply_state == AWAITING_END_NEWLINE) {
        int off = nl_1st_occurence_offset(&buf[i], bytes-i);

        if (off != -1 && i+off < bytes-1) { return IMPL_ERROR; }

        // add chars up to newline or end of buf (no newline was found)
        concat_to_reply(reply, &buf[i], bytes-i);

        if (off == -1) {
          continue;
        } else {
          reply_state = REPLY_COMPLETE;
          break;
        }
      }
    }
  }

  return 0;
}

int assert_valid_code(char *code, char **valid_codes, int n) {
	for (int i = 0; i < n; i++) {
		if (strcmp(valid_codes[i], code) == 0) {
			return 0;
		}
	}
	return 1;
}