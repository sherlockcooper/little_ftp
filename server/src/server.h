//
// Created by apple on 10/25/18.
//

#ifndef FTP_SERVER_H
#define FTP_SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <regex.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>


#define SERV_PORT 21
#define MAX_FD 10
#define BUF_SIZE 1024
#define PATH_SIZE 256
#define IP_SIZE 32



struct single_connection
{
    int listening_fd;
    int command_fd;
    int data_fd;
    int logged; //0 for not logged, 1 for waiting password, 2 for logged
    int port_to_connect;
    int transfer_state;//-1 for not transfer, 1 for list, 2 for retr, 3 for stor
    int last_command;
    int last_transfer;//0 for nothing
    int pasv_or_port;//1 for port, 0 for pasv
    int list_dir;//-1 for nothing, 0 for file, 1 for dir
    int rw_offset;
    char ip_addr[IP_SIZE];
    char cwd[PATH_SIZE];
    char file_path[PATH_SIZE];//for rnfr, rnto
    char list_path[PATH_SIZE];//for LIST
    char trans_file[PATH_SIZE];//for RETR, STOR
    DIR *trans_dir;
    FILE *target_file;//for RETR
    int data_transfersd;
};

int socket_fd;
int max_fd;
//int listening_fd[MAX_FD];
//int command_fd[MAX_FD];
//int data_fd[MAX_FD];
//bool logged[MAX_FD];
struct single_connection clients[MAX_FD];
struct timeval timeout;
char default_cwd[PATH_SIZE];

fd_set fd_read;
fd_set fd_select;

enum COMMANDS {USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV, MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, REST};

int str_to_int(const char *src);//if not '0'~'9', return -1
int int_to_str(int num, char *buf);
int parse_command(char *origin, char *content);//split client request into command and info
int make_response(int code, char *msg, char *write_buf);
int parse_ip_port(char *params, char *ip_addr, int *port_ptr);//success parsed return 1, otherwise 0
int port_connect(int index);
int pasv_connect(int index);
int get_local_ip(const char *eth_inf, char *ip);
int create_listening(int port);
int remove_dir(char *dir);
int set_client_default(int i);



// handle commands, if success return 1
int user_command(int index, char* params);
int pass_command(int index, char* params);
int retr_command(int index, char* params);
int stor_command(int index, char* params);
int quit_command(int index, char* params);
int type_command(int index, char* params);
int port_command(int index, char* params);
int pasv_command(int index, char* params);
int mkd_command(int index, char* params);
int cwd_command(int index, char* params);
int pwd_command(int index, char* params);
int list_command(int index, char* params);
int rmd_command(int index, char* params);
int rnfr_command(int index, char* params);
int rnto_command(int index, char* params);
int rest_command(int index, char* params);

int transfer_list(int index);
int retr_file(int index);
int storfile(int index);

#endif //FTP_SERVER_H
