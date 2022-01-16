#ifndef FTP_CONNECTION_H
#define FTP_CONNECTION_H

#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <regex.h>

/**
 * Client information.
 **/
typedef struct {
  char *user;
  char *pass;
  char *host;
  char *path;
} ftp_client_info;

/** @brief 
 *  @param host Host.
 *  @param port Port.
 *  @return 
 */
struct addrinfo *host_IPaddrinfos(char *host, char *port);

/** @brief Connects to host.
 *  @param info The ftp_client_info struct.
 *  @return Returns file descriptor of socket upon success, -1 otherwise.
 */
int connect_to_host(ftp_client_info *info);

/** @brief 
 *  @param capt_group
 *  @param url
 *  @return 
 */
char *get_client_param(regmatch_t capt_group, const char *url);

/** @brief Parses given URL and updates ftp_client_info struct.
 *  @param info The ftp_client_info struct.
 *  @param url Given URL.
 *  @return Returns 0 upon success, 1 otherwise.
 */
int parse_URL(ftp_client_info *info, const char *url);

/** @brief 
 *  @param ip Ip.
 *  @param port Port.
 *  @return Returns file descriptor of data socket.
 */
int connect_to_host_data_port(unsigned char *ip, unsigned char *port);

#endif