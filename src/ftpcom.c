#include "ftpcom.h"
#include "ftpconn.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>

int send_command(int socket_fd, char* message) {
    int bytes = write(socket_fd, message, strlen(message));
    if (bytes <= 0) {
      perror("write()");
      return -1;
    }
    return 0;
}

int save_file(int socket_fd, char* filename) {
  int file_fd = open(filename, O_WRONLY | O_CREAT, 0777);

  if (file_fd < 0)
    return -1;

  int bytes;
  char buf[1];

  while ((bytes = read(socket_fd, buf, sizeof(buf))) > 0) {
      if (write(file_fd, buf, bytes) < 0) {
        return -1;
      }
  }

  close(file_fd);
  return 0;
}

ftp_reply create_reply() {
  ftp_reply reply;
  
  reply.buf = malloc(sizeof(char)*BASE_REPLY_LEN+1);
  memset(reply.buf, 0, BASE_REPLY_LEN+1);
  memset(reply.code, 0, CODE_SIZE+1);
  reply.real_size = BASE_REPLY_LEN+1;
  reply.strlen = 0;

  return reply;
}

void concat_to_reply(ftp_reply *reply, char *str, int n) {
  if (reply->strlen+n > reply->real_size) {
    reply->buf = realloc(reply->buf, 2*(reply->strlen+n));
    reply->real_size = 2*(reply->strlen+n);
  }

  strncat(reply->buf, str, n);
  reply->strlen += n;
}

int valid_line_start(char *line) {
  return isdigit(line[0]) && isdigit(line[1])
          && isdigit(line[2]) && (line[3] == ' ' 
          || line[3] == '-');
}

#define LINE_SEPARATOR(line) (line[3])

int newline_1st_occurence(char *buf, int n) {
  for (int i = 0; i < n; i++) {
    if (buf[i] == '\n') {
      return i;
    }
  }

  return -1;
}

int read_reply(int ctrl_socket_fd, ftp_reply *reply) {
  ftp_reply_state reply_state = START;
  int single_line_reply = 1;

  char line_start[CODE_SIZE+1];
  int start_count = 0;

  while (reply_state != REPLY_COMPLETE) {
    char buf[BASE_REPLY_LEN];
    memset(buf, 0, BASE_REPLY_LEN);

    int bytes = recv(ctrl_socket_fd, buf, BASE_REPLY_LEN, 0);

    if (bytes == 0) {
      return SERVER_DISC;
    }

    if (bytes == -1) {
      return READ_ERROR;
    }

    /*buf[]
code[]
n = 0
single_line = 1;

while (state != REPLY_DONE) {
	memset(...)
	int bytes = recv;
	//erros
	for (sst i = 0; i < bytes; i++)
		if (state == Start) {
			if (n == 4) {
				if (valid_line_stae) {
					single_line = sep == ' '
					state = Awaiting_newline;
					n = 0;
				} else {
					return error
				}
			} else {
				code[n++] = buf[i];
			}
			line.append(buf[i])
			continue;	
		}

		if(state == awaiting_newline && single_line) {
			if (newline index != -1) {
				line.append(up to newline);
				state = REPLY_DONE;
				if (more byes) {
					return error;
				} else {
					break;
				}	
			} else {
				line.append(tudo);
				shift i;
			}
			if (buf[i] == '\n') {
				state = REPLY_DONE;
				if (more byes) {
					return error;
				} else {
					break;
				}
			} else {
				buf append
			}
			
		}


}*/
  }

  return 0;
}