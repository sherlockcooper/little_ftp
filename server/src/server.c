//
// Created by apple on 10/26/18.
//

#include "server.h"

int str_to_int(const char *src)
{
    int len = (int)strlen(src);
    int res = 0;
    for(int i=0; i<len; i++)
    {
        if(src[i]>'9'||src[i]<'0')
        {
            res = -1;
            return res;
        }
        res=res*10+(int)(src[i]-'0');
    }
    return res;
}

int int_to_str(int num, char *buf)
{
    int len=0;
    char temp[IP_SIZE];
    while(num>=10)
    {
        temp[len]=(num%10)+'0';
        num/=10;
        len++;
    }
    temp[len]=num+'0';
    len++;
    for(int i=0; i<len; i++)
    {
        buf[i]=temp[len-1-i];
    }
    buf[len]=0;
    return len;
}


int parse_command(char *origin, char *content)
{
    char *command = strtok(origin, " ");
    char *params = strtok(NULL, " ");
    for(int i=0; i<strlen(command); i++)
    {
        if(command[i]=='\r'||command[i]=='\n')
            command[i]=0;
        else
        {
            command[i]=(char)toupper(command[i]);
        }
    }
    if(params!=NULL)
    {
        for(int i=0; i<strlen(params); i++)
        {
            if(params[i]=='\r'||params[i]=='\n')
                params[i]=0;
        }
        strcpy(content, params);
        printf("command %s, param info %s\n", command, content);
    }
    else
    {
        strcpy(content, "");
        printf("command %s\n", command);
    }

    if(!strcmp(origin, "USER"))
    {
        return USER;
    }
    else if(!strcmp(origin, "PASS"))
    {
        return PASS;
    }
    else if(!strcmp(origin, "RETR"))
    {
        return RETR;
    }
    else if(!strcmp(origin, "STOR"))
    {
        return STOR;
    }
    else if(!strcmp(origin, "QUIT"))
    {
        return QUIT;
    }
    else if(!strcmp(origin, "SYST"))
    {
        return SYST;
    }
    else if(!strcmp(origin, "TYPE"))
    {
        return TYPE;
    }
    else if(!strcmp(origin, "PORT"))
    {
        return PORT;
    }
    else if(!strcmp(origin, "PASV"))
    {
        return PASV;
    }
    else if(!strcmp(origin, "MKD"))
    {
        return MKD;
    }
    else if(!strcmp(origin, "CWD"))
    {
        return CWD;
    }
    else if(!strcmp(origin, "PWD"))
    {
        return PWD;
    }
    else if(!strcmp(origin, "LIST"))
    {
        return LIST;
    }
    else if(!strcmp(origin, "RMD"))
    {
        return RMD;
    }
    else if(!strcmp(origin, "RNFR"))
    {
        return RNFR;
    }
    else if(!strcmp(origin, "RNTO"))
    {
        return RNTO;
    }
    else if(!strcmp(origin, "REST"))
    {
        return REST;
    }

    return -1;
}

int make_response(int code, char *msg, char *write_buf)
{
    char temp[BUF_SIZE]={0};

    strcpy(temp, msg);
    write_buf[2]=(code%10)+'0';
    code/=10;
    write_buf[1]=(code%10)+'0';
    code/=10;
    write_buf[0]=code+'0';

    write_buf[3]=' ';
    for(int i=0; i<strlen(temp); i++)
    {
        write_buf[4+i]=temp[i];
    }
    write_buf[strlen(write_buf)]='\r';
    write_buf[strlen(write_buf)]='\n';

    return strlen(write_buf);
}

int parse_ip_port(char *params, char *ip_addr, int *port_ptr)
{
    char *dot = ".";
    char *h1, *h2, *h3, *h4, *p1, *p2;
    char temp[32]={0};
    h1 = strtok(params, ",");
    if(h1==NULL)
        return 0;
    strcpy(temp, h1);
    strcat(temp, dot);
    h2 = strtok(NULL, ",");
    if(h2==NULL)
        return 0;
    strcat(temp, h2);
    strcat(temp, dot);
    h3 = strtok(NULL, ",");
    if(h3==NULL)
        return 0;
    strcat(temp, h3);
    strcat(temp, dot);
    h4 = strtok(NULL, ",");
    if(h4==NULL)
        return 0;
    strcat(temp, h4);
    strcpy(ip_addr, temp);

    *port_ptr = 0;
    p1 = strtok(NULL, ",");
    if(p1==NULL)
        return 0;
    if(str_to_int(p1)==-1)
        return 0;
    *port_ptr = str_to_int(p1)*256;
    p2 = strtok(NULL, ",");
    if(p2==NULL)
        return 0;
    if(str_to_int(p2)==-1)
        return 0;
    *port_ptr += str_to_int(p2);

    return 1;
}

int port_connect(int index)
{
    if(clients[index].port_to_connect<20000||clients[index].port_to_connect>65536)
        return 0;
    if(!strcmp(clients[index].ip_addr, ""))
        return 0;

    int sockfd;
    struct sockaddr_in addr;
    int connfd=-1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket\n");
        return 0;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(clients[index].port_to_connect);

    if (inet_pton(AF_INET, clients[index].ip_addr, &addr.sin_addr) <= 0) {
        printf("Error inet_pton()\n");
        return 0;
    }

    if ((connect(sockfd, (struct sockaddr*)&addr, sizeof(addr))) < 0) {
        printf("Error connect()\n");
        return 0;
    }

    clients[index].data_fd=sockfd;
    FD_SET(sockfd, &fd_read);
    if(max_fd<sockfd)
        max_fd = sockfd;

    return 1;
}

int pasv_connect(int index)
{
    socklen_t cli_len;
    int connfd;
    struct sockaddr_in cli_addr;
    cli_len = sizeof(cli_addr);
    connfd = accept(clients[index].listening_fd, (struct sockaddr *)&cli_addr, &cli_len);
    if(connfd < 0)
    {
        printf("fail to accept %d data connection\n", index);
        return 0;
    }
    else
    {
        printf("%d data connection succeed\n", index);
        clients[index].data_fd = connfd;
        FD_SET(connfd, &fd_read);
        if(max_fd<connfd)
            max_fd = connfd;
        FD_CLR(clients[index].listening_fd, &fd_read);
        close(clients[index].listening_fd);
        clients[index].listening_fd = -1;
    }
    return 1;
}

int create_listening(int port)
{
    int err;

    struct sockaddr_in srv_addr;
    socklen_t srv_len;
    int new_fd=-1;
    new_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(new_fd<0)
    {
        printf("creating socket failed\n");
        return new_fd;
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    srv_len = sizeof(srv_addr);
    err = bind(new_fd, (struct sockaddr *)&srv_addr, srv_len);
    if(err<0)
    {
        printf("binding failed\n");
        return -1;
    }

    err = listen(new_fd, MAX_FD);
    if(err<0)
    {
        printf("listening failed\n");
        return -1;
    }

    FD_SET(new_fd, &fd_read);
    if(max_fd<new_fd)
        max_fd = new_fd;
    return new_fd;
}

int get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

    close(sd);
    return 0;
}

int remove_dir(char *dir)
{
    char cur_dir[] = ".";
    char up_dir[] = "..";
    char dir_name[BUF_SIZE];
    DIR *dirp;
    struct dirent *dp;
    struct stat dir_stat;

    if ( 0 != access(dir, F_OK) ) {
        return 0;
    }

    if ( 0 > stat(dir, &dir_stat) ) {
        perror("get directory stat error");
        return -1;
    }

    if ( S_ISREG(dir_stat.st_mode) ) {
        remove(dir);
    } else if ( S_ISDIR(dir_stat.st_mode) ) {
        dirp = opendir(dir);
        while ( (dp=readdir(dirp)) != NULL ) {

            if ( (0 == strcmp(cur_dir, dp->d_name)) || (0 == strcmp(up_dir, dp->d_name)) ) {
                continue;
            }

            sprintf(dir_name, "%s/%s", dir, dp->d_name);
            remove_dir(dir_name);   // 递归调用
        }
        closedir(dirp);

        rmdir(dir);
    } else {
        perror("unknow file type!");
    }

    return 0;
}

int set_client_default(int i)
{
    clients[i].logged=0;
    clients[i].port_to_connect = 0;
    clients[i].transfer_state = -1;
    clients[i].last_command = -1;
    clients[i].list_dir = -1;
    clients[i].rw_offset = 0;
    clients[i].last_transfer = 0;
    clients[i].pasv_or_port=0;
    memset(clients[i].ip_addr, 0, sizeof(clients[i].ip_addr));
    memset(clients[i].file_path, 0, sizeof(clients[i].file_path));
    memset(clients[i].list_path, 0, sizeof(clients[i].list_path));
    memset(clients[i].trans_file, 0, sizeof(clients[i].trans_file));
    strcpy(clients[i].cwd, default_cwd);
    clients[i].trans_dir = NULL;
    clients[i].target_file = NULL;
    if(clients[i].listening_fd!=-1)
    {
        FD_CLR(clients[i].listening_fd, &fd_read);
        close(clients[i].listening_fd);
        clients[i].listening_fd = -1;
    }
    if(clients[i].data_fd!=-1)
    {
        FD_CLR(clients[i].data_fd, &fd_read);
        close(clients[i].data_fd);
        clients[i].data_fd=-1;
    }

    clients[i].data_transfersd = 0;
    return 1;
}

int user_command(int index, char* params)
{
    char response[BUF_SIZE]={0};
    int len=0;
    int num_write = 0;
    if(clients[index].logged==0)
    {
        if(!strcmp(params, "anonymous"))
        {
            clients[index].logged=1;
            len = make_response(331, "guest login ok, send email address as password", response);
            num_write = write(clients[index].command_fd, response, len);
            if(num_write<0)
            {
                printf("send failed\n");
                return 0;
            }
            return 1;
        }
        else
        {
            len = make_response(530, "incorrect username", response);
            num_write = write(clients[index].command_fd, response, len);
            if(num_write<0)
            {
                printf("send failed\n");
                return 0;
            }
            return 1;
        }
    }
    else
    {
        len = make_response(404, "USER already login", response);
        num_write = write(clients[index].command_fd, response, len);
        if(num_write>=0)
        {
            return 1;
        }
    }
    return 0;

}

int pass_command(int index, char* params) {
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    if (clients[index].logged == 1) {
        char *email_pattern = ".+@.+..+";
        regex_t reg;
        regmatch_t matchgroup[1];
        regcomp(&reg, email_pattern, REG_EXTENDED);
        int err = regexec(&reg, params, 1, matchgroup, 0);
        if (!err) {
            clients[index].logged = 2;
            len = make_response(230, "Guest login ok, access restrictions apply", response);
            num_write = write(clients[index].command_fd, response, len);
            if (num_write >= 0) {
                return 1;
            }
        }
        else
        {
            len = make_response(530, "password incorrect", response);
            write(clients[index].command_fd, response, len);
            return 1;
        }

    } else if (clients[index].logged == 0) {
        len = make_response(503, "lack USER command", response);
        num_write = write(clients[index].command_fd, response, len);
        if (num_write >= 0) {
            return 1;
        }
    } else if (clients[index].logged == 2) {
        len = make_response(503, "already login", response);
        num_write = write(clients[index].command_fd, response, len);
        if (num_write >= 0) {
            return 1;
        }
    }

    return 0;
}

int retr_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }
    chdir(clients[index].cwd);
    FILE* file_ptr;
    if((file_ptr=fopen(params, "r"))==NULL)
    {
        len = make_response(451, "no such file", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }
    fclose(file_ptr);

    if(clients[index].data_fd==-1)
    {
        if(clients[index].pasv_or_port && !port_connect(index))
        {
            len = make_response(425, "no TCP data connection", response);
            write(clients[index].command_fd, response, (size_t) len);
            return 1;
        }
        else if(!clients[index].pasv_or_port && !pasv_connect(index))
        {
            len = make_response(425, "no TCP data connection", response);
            write(clients[index].command_fd, response, (size_t) len);
            return 1;
        }
    }
    //all conditions satisfied
    strcpy(clients[index].trans_file, params);
    clients[index].transfer_state = 2;
    if(clients[index].last_command!=REST)
    {
        clients[index].rw_offset = 0;
    }

    len = make_response(150, "start transfering file", response);
    write(clients[index].command_fd, response, len);
    clients[index].last_transfer = RETR;
    return 1;

}

int stor_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }
    chdir(clients[index].cwd);
    FILE* file_ptr;

    if(params[0]=='/')
    {
        char temp[PATH_SIZE]={0};
        int i=0;
        for(i=strlen(params)-1; i>=0; i--)
        {
            if(params[i]=='/')
                break;
        }
        i++;
        strcpy(temp, &params[i]);
        memset(params, 0, PATH_SIZE);
        strcpy(params, temp);
    }
//    int file_fd = open(params, O_WRONLY);
//    if(file_fd==-1)
//    {
//        len = make_response(451, "no such file", response);
//        write(clients[index].command_fd, response, len);
//        return 1;
//    }
//    close(file_fd);
    if((file_ptr=fopen(params, "a"))==NULL)
    {
        len = make_response(451, "no such file", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }
    fclose(file_ptr);


    if(clients[index].data_fd==-1)
    {
        if(clients[index].pasv_or_port && !port_connect(index))
        {
            len = make_response(425, "no TCP data connection", response);
            write(clients[index].command_fd, response, (size_t) len);
            return 1;
        }
        else if(!clients[index].pasv_or_port && !pasv_connect(index))
        {
            len = make_response(425, "no TCP data connection", response);
            write(clients[index].command_fd, response, (size_t) len);
            return 1;
        }
    }

    //all conditions satisfied

    strcpy(clients[index].trans_file, params);
    clients[index].transfer_state = 3;
    if(clients[index].last_command!=REST)
    {
        clients[index].rw_offset = 0;
    }
    len = make_response(150, "start transfering file", response);
    write(clients[index].command_fd, response, len);
    clients[index].last_transfer = STOR;
    return 1;
}

int rest_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    FILE* file_ptr;
    int required_offset = 0;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }
    if((required_offset = str_to_int(params))<0)
    {
        len = make_response(501, "invalid parameters", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }

//    if(clients[index].data_fd==-1)
//    {
//        if(!port_connect(index))
//        {
//            len = make_response(425, "no TCP data connection", response);
//            write(clients[index].command_fd, response, (size_t) len);
//            return 1;
//        }
//    }
    clients[index].rw_offset = required_offset;

//    if(clients[index].last_transfer==STOR)
//    {
//        clients[index].transfer_state=3;
//    }
//    else if(clients[index].last_transfer==RETR)
//    {
//        clients[index].transfer_state = 2;
//    }
    len = make_response(350, "REST accepted", response);
    write(clients[index].command_fd, response, len);
    return 1;
}

int quit_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;

    len = make_response(221, "Bye", response);
    num_write = write(clients[index].command_fd, response, len);

    FD_CLR(clients[index].command_fd, &fd_read);
    close(clients[index].command_fd);
    clients[index].command_fd=-1;

//    if(clients[index].listening_fd!=-1)
//    {
//        FD_CLR(clients[index].listening_fd, &fd_read);
//        close(clients[index].listening_fd);
//        clients[index].listening_fd = -1;
//    }
//    if(clients[index].data_fd!=-1)
//    {
//        FD_CLR(clients[index].data_fd, &fd_read);
//        close(clients[index].data_fd);
//        clients[index].data_fd = -1;
//    }
    set_client_default(index);
    return 1;
}

int type_command(int index, char* params)
{

    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }

    if(!strcmp(params, "I"))
    {
        len = make_response(200, "Type set to I.", response);
        num_write = write(clients[index].command_fd, response, len);
        if (num_write >= 0)
        {
            return 1;
        }
    }
    else
    {
        len = make_response(504, "Type not permitted", response);
        num_write = write(clients[index].command_fd, response, len);
        if (num_write >= 0)
        {
            return 1;
        }
    }
    return 0;
}

int port_command(int index, char* params)
{
    //to add: log state check
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    char ip_addr[IP_SIZE]={0};
    int port=0;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }

    if(parse_ip_port(params, ip_addr, &port))
    {
        clients[index].port_to_connect = port;
        strcpy(clients[index].ip_addr, ip_addr);
        if(clients[index].listening_fd!=-1)
        {
            FD_CLR(clients[index].listening_fd, &fd_read);
            close(clients[index].listening_fd);
            clients[index].listening_fd = -1;
        }
        if(clients[index].data_fd!=-1)
        {
            FD_CLR(clients[index].data_fd, &fd_read);
            close(clients[index].data_fd);
            clients[index].data_fd = -1;
        }

        len = make_response(200, "PORT request accepted", response);
        num_write = write(clients[index].command_fd, response, len);
        clients[index].pasv_or_port=1;
        if(num_write<0)
            return 0;
    }
    else
    {
        len = make_response(501, "wrong form of ip address and port", response);
        num_write = write(clients[index].command_fd, response, len);
        if(num_write<0)
            return 0;
    }

    return 1;
}

int pasv_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    int port=0;
    char ip_addr[IP_SIZE]={0};
    char p1[IP_SIZE]={0};
    char p2[IP_SIZE]={0};
    const char *local_eth="lo";

    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }

    if(strlen(params)!=0)
    {
        len = make_response(501, "parameter is forbidden", response);
        num_write = write(clients[index].command_fd, response, len);
        return 1;
    }

    if(clients[index].listening_fd!=-1)
    {
        FD_CLR(clients[index].listening_fd, &fd_read);
        close(clients[index].listening_fd);
        clients[index].listening_fd = -1;
    }
    if(clients[index].data_fd!=-1)
    {
        FD_CLR(clients[index].data_fd, &fd_read);
        close(clients[index].data_fd);
        clients[index].data_fd = -1;
    }

    if(get_local_ip(local_eth, ip_addr)<0)
    {
        return 0;
    }
    port = rand()%45536+20000;
    if((clients[index].listening_fd=create_listening(port))<0)
    {
        len = make_response(425, "fail to start listening", response);
        num_write = write(clients[index].command_fd, response, len);
        return 1;
    }
    printf("clients[%d] listening fd: %d\n", index, clients[index].listening_fd);
    for(int i=0; i<strlen(ip_addr); i++)
    {
        if(ip_addr[i]=='.')
            ip_addr[i]=',';
    }
    strcat(ip_addr, ",");
    int_to_str(port/256, p1);
    int_to_str(port%256, p2);
    strcat(ip_addr, p1);
    strcat(ip_addr,",");
    strcat(ip_addr, p2);

    len = make_response(227, ip_addr, response);
    num_write = write(clients[index].command_fd, response, len);
    clients[index].pasv_or_port=0;
    return 1;
}

int mkd_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    char mk_dir_path[PATH_SIZE]={0};
    int len = 0;
    int num_write = 0;
    int err;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }

    chdir(clients[index].cwd);

    err = mkdir(params, 0755);
    if(err<0)
    {
        len = make_response(550, "dir creation failed", response);
        num_write = write(clients[index].command_fd, response, len);
    }
    else
    {
        chdir(params);
        getcwd(mk_dir_path, PATH_SIZE);
        chdir(clients[index].cwd);
        len = make_response(250, mk_dir_path, response);
        num_write = write(clients[index].command_fd, response, len);
    }

    return 1;
}

int cwd_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    char mk_dir_path[PATH_SIZE]={0};
    int len = 0;
    int num_write = 0;
    int err;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }

//    if(!strcmp(params, ".."))
//    {
//        len = make_response(501, "you cannot change to parent directory", response);
//        write(clients[index].command_fd, response, len);
//        return 1;
//    }
    err = chdir(params);
    if(err<0)
    {
        len = make_response(550, "no such file or directory", response);
        num_write = (int) write(clients[index].command_fd, response, len);
    }
    else
    {
        getcwd(mk_dir_path, PATH_SIZE);
        memset(clients[index].cwd, 0, PATH_SIZE);
        strcpy(clients[index].cwd, mk_dir_path);
        len = make_response(250, "Okay", response);
        num_write = (int) write(clients[index].command_fd, response, len);
    }

    return 1;
}

int pwd_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    char cwd[PATH_SIZE]={0};
    int len = 0;
    int num_write = 0;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }

    chdir(clients[index].cwd);

    strcpy(cwd, "\"");
    getcwd(response, BUF_SIZE);
    strcat(cwd, response);
    strcat(cwd, "\"");
    memset(response, 0, sizeof(response));
    len = make_response(257, cwd, response);
    num_write = (int) write(clients[index].command_fd, response, len);

    return 1;
}

int list_command(int index, char* params)
{
    DIR *target_dir;

    char response[BUF_SIZE] = {0};
    char file_info[128]={0};
    int len = 0;
    int num_write = 0;
    int possible_fd;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }
    chdir(clients[index].cwd);
    if(!strcmp(params, ""))
    {
        getcwd(params, BUF_SIZE);
    }

    if((target_dir=opendir(params))==NULL)
    {
        possible_fd = open(params, O_RDONLY);
        if(possible_fd==-1)
        {
            len = make_response(451, "cannot get right target file/directory", response);
            num_write = (int) write(clients[index].command_fd, response, (size_t) len);
            return 1;
        }
        clients[index].list_dir = 0;//file
        close(possible_fd);
    }
    else{
        clients[index].list_dir = 1;
        clients[index].trans_dir = target_dir;
    }

    strcpy(clients[index].list_path, params);

    if(clients[index].data_fd==-1)
    {
        if(clients[index].pasv_or_port && !port_connect(index))
        {
            len = make_response(425, "no TCP data connection", response);
            write(clients[index].command_fd, response, (size_t) len);
            return 1;
        }
        else if(!clients[index].pasv_or_port && !pasv_connect(index))
        {
            len = make_response(425, "no TCP data connection", response);
            write(clients[index].command_fd, response, (size_t) len);
            return 1;
        }
    }

    clients[index].transfer_state = 1;
    len = make_response(150, "start transfering directory list", response);
    num_write = (int) write(clients[index].command_fd, response, (size_t) len);
    return 1;
}

int rmd_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }

    if(remove_dir(params)<0)
    {
        len = make_response(550, "removal failed", response);
        write(clients[index].command_fd, response, (size_t) len);
        return 1;
    }
    else
    {
        len = make_response(250, "directory successfully removed", response);
        write(clients[index].command_fd, response, (size_t) len);
        return 1;
    }
}

int rnfr_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }

    if(access(params, F_OK)==0)
    {
        len = make_response(350, "file found", response);
        write(clients[index].command_fd, response, len);
        clients[index].last_command = RNFR;
        strcpy(clients[index].file_path, params);
    }
    else
    {
        len = make_response(450, "no such file", response);
        write(clients[index].command_fd, response, len);
    }
    return 1;
}

int rnto_command(int index, char* params)
{
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    int err;
    char sh_command[BUF_SIZE] = {0};
    if(clients[index].logged!=2)
    {
        len = make_response(530, "please login with USER and PASS", response);
        write(clients[index].command_fd, response, len);
        return 1;
    }

    if(clients[index].last_command==RNFR)
    {
        if((access(clients[index].file_path, F_OK))==0)
        {
            strcpy(sh_command, "mv");
            strcat(sh_command, " ");
            strcat(sh_command, clients[index].file_path);
            strcat(sh_command, " ");
            strcat(sh_command, params);
            err = system(sh_command);
            if(WIFEXITED(err))
            {
                if(WEXITSTATUS(err)==0)
                {
                    len = make_response(250, "rename succeed", response);
                    write(clients[index].command_fd, response, len);
                }
                else
                {
                    len = make_response(550, "rename failed", response);
                    write(clients[index].command_fd, response, len);
                }
            }
            else
            {
                len = make_response(550, "rename failed", response);
                write(clients[index].command_fd, response, len);
            }
        }
        else
        {
            len = make_response(550, "no such file", response);
            write(clients[index].command_fd, response, len);
        }
    }
    else
    {
        len = make_response(503, "must follow RNFR", response);
        write(clients[index].command_fd, response, len);
    }
    return 1;
}



int transfer_list(int index)
{
    char temp[PATH_SIZE] = {0};
    char response[BUF_SIZE] = {0};
    int len = 0;
    int num_write = 0;
    struct dirent * file_ptr;
    int file_fd = -1;
    struct stat stbuf;
    chdir(clients[index].cwd);
    if(clients[index].list_dir==0)// LIST file
    {
        sleep(0);
        if((file_fd=open(clients[index].list_path, O_RDONLY))!=-1)
        {
            if ((fstat(file_fd, &stbuf) != 0) || (!S_ISREG(stbuf.st_mode))) {
                len = make_response(451, "get file information failed", response);
                write(clients[index].command_fd, response, len);
            }
            strcpy(response, "file: ");
            strcat(response, clients[index].list_path);
            int_to_str((int) stbuf.st_size, temp);
            strcat(response, " size: ");
            strcat(response, temp);

            write(clients[index].data_fd, response, strlen(response));
            memset(response, 0, sizeof(response));
            len = make_response(226, "LIST file done", response);
            write(clients[index].command_fd, response, len);

        }
        else
        {
            len = make_response(451, "file not found", response);
            write(clients[index].command_fd, response, len);
        }

        FD_CLR(clients[index].data_fd, &fd_read);
        close(clients[index].data_fd);
        clients[index].data_fd = -1;
        clients[index].list_dir = -1;
        clients[index].transfer_state = -1;
    }
    else if(clients[index].list_dir==1)
    {
        while (1)
        {
            file_ptr=readdir(clients[index].trans_dir);
            if(file_ptr==NULL)
            {
                //strcpy(response, "");
                //write(clients[index].data_fd, response, strlen(response));
                len = make_response(226, "LIST directory done", response);
                write(clients[index].command_fd, response, len);

                FD_CLR(clients[index].data_fd, &fd_read);
                close(clients[index].data_fd);
                clients[index].data_fd = -1;
                clients[index].list_dir = -1;
                clients[index].transfer_state = -1;
                break;
            }
            if(strcmp(file_ptr->d_name, ".")==0 || strcmp(file_ptr->d_name, "..")==0)
            {
                continue;
            }
            //one content each time
            strcpy(response, file_ptr->d_name);
            strcat(response, " ");
            if(file_ptr->d_type==DT_DIR)
            {
                strcat(response, "directory");
            }
            else
            {
                strcat(response, "file");
            }
            strcat(response, " ");
            stat(file_ptr->d_name, &stbuf);
            int_to_str((int) stbuf.st_size, temp);
            strcat(response, temp);
            strcat(response, "\r\n");
            write(clients[index].data_fd, response, strlen(response));
            break;
        }
    }
    return 1;

}

int retr_file(int index)
{
    char response[BUF_SIZE] = {0};
    char data_response[BUF_SIZE]={0};
    int len = 0;
    int num_write = 0;
    int num_read = 0;

    chdir(clients[index].cwd);
    int tar_fd = open(clients[index].trans_file, O_RDONLY);

    if(tar_fd==-1)
    {
        len = make_response(451, "file not found", response);
        write(clients[index].command_fd, response, len);
        FD_CLR(clients[index].data_fd, &fd_read);
        close(clients[index].data_fd);
        clients[index].data_fd = -1;

        clients[index].rw_offset = 0;
        clients[index].transfer_state = -1;
        memset(clients[index].trans_file, 0, sizeof(clients[index].trans_file));
        return 1;
    }
    lseek(tar_fd, clients[index].rw_offset, SEEK_SET);
    num_read = read(tar_fd, data_response, BUF_SIZE-1);
    close(tar_fd);
    if(num_read==-1)
    {
        len = make_response(451, "read file failed", response);
        write(clients[index].command_fd, response, len);
        FD_CLR(clients[index].data_fd, &fd_read);
        close(clients[index].data_fd);
        clients[index].data_fd = -1;

        clients[index].rw_offset = 0;
        clients[index].transfer_state = -1;
        memset(clients[index].trans_file, 0, sizeof(clients[index].trans_file));
        return 1;
    }
    if(num_read==0||num_read<BUF_SIZE-1)
    {
        sleep(0);
        if(write(clients[index].data_fd, data_response, num_read)<0)
        {
            len = make_response(426, "connection is broken", response);

        }
        else{
            len = make_response(226, "RETR file done", response);
            memset(clients[index].trans_file, 0, sizeof(clients[index].trans_file));
        }
        //write(clients[index].data_fd, data_response, num_read);

        write(clients[index].command_fd, response, len);

        FD_CLR(clients[index].data_fd, &fd_read);
        close(clients[index].data_fd);
        clients[index].data_fd = -1;

        clients[index].rw_offset = 0;
        clients[index].transfer_state = -1;

    }
    else if(num_read==BUF_SIZE-1)
    {
        //write(clients[index].data_fd, data_response, num_read);
        if(write(clients[index].data_fd, data_response, num_read)<0)
        {
            len = make_response(426, "connection is broken", response);
            write(clients[index].command_fd, response, len);

            FD_CLR(clients[index].data_fd, &fd_read);
            close(clients[index].data_fd);
            clients[index].data_fd = -1;

            clients[index].transfer_state = -1;
        }
        else
        {
            clients[index].rw_offset+=num_read;
        }
    }
    return 1;
}

int storfile(int index)
{
    //FILE *tar_file=NULL;
    char response[BUF_SIZE] = {0};
    char data_response[BUF_SIZE]={0};
    int len = 0;
    int num_write = 0;
    int num_read = 0;
    //tar_file = fopen(clients[index].trans_file, "a+");
    chdir(clients[index].cwd);
    int file_fd = open(clients[index].trans_file, O_WRONLY);

    //if(tar_file==NULL)
    if(file_fd==-1)
    {
        clients[index].rw_offset = 0;
        len = make_response(451, "file not found", response);
        write(clients[index].command_fd, response, len);
        FD_CLR(clients[index].data_fd, &fd_read);
        close(clients[index].data_fd);
        clients[index].data_fd = -1;
        clients[index].transfer_state = -1;
        memset(clients[index].trans_file, 0, sizeof(clients[index].trans_file));
        return 1;
    }
    num_read = (int) read(clients[index].data_fd, data_response, BUF_SIZE);
    clients[index].data_transfersd += num_read;
    //printf("%d\n", clients[index].data_transfersd);
    if(num_read==-1)
    {
        clients[index].rw_offset = 0;
        len = make_response(426, "read data failed", response);
        write(clients[index].command_fd, response, len);
        FD_CLR(clients[index].data_fd, &fd_read);
        close(clients[index].data_fd);
        clients[index].data_fd = -1;
        clients[index].transfer_state = -1;
        memset(clients[index].trans_file, 0, sizeof(clients[index].trans_file));
        return 1;
    }
    if(num_read==0)
    {

        //printf("gonna close%d, %d \n", num_read, clients[index].rw_offset);
        //lseek(file_fd, clients[index].rw_offset, SEEK_SET);
        //num_write = write(file_fd, data_response, num_read);
        close(file_fd);
        clients[index].rw_offset = 0;
        len = make_response(226, "STOR file done", response);
        write(clients[index].command_fd, response, len);

        FD_CLR(clients[index].data_fd, &fd_read);
        close(clients[index].data_fd);
        clients[index].data_fd = -1;
        clients[index].transfer_state = -1;
        //memset(clients[index].trans_file, 0, sizeof(clients[index].trans_file));
        return 1;
    }

    lseek(file_fd, clients[index].rw_offset, SEEK_SET);
    num_write = write(file_fd, data_response, num_read);
    clients[index].rw_offset+=num_write;
    close(file_fd);
    return 1;
}
