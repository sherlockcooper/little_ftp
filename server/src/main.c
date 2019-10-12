
#include "server.h"

int establish(int port, char* root)
{
    int err;

    struct sockaddr_in srv_addr;
    socklen_t srv_len;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd<0)
    {
        printf("creating socket failed\n");
        return 0;
    }

    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    srv_len = sizeof(srv_addr);
    err = bind(socket_fd, (struct sockaddr *)&srv_addr, srv_len);
    if(err<0)
    {
        printf("binding failed\n");
        return 0;
    }

    err = listen(socket_fd, MAX_FD);
    if(err<0)
    {
        printf("listening failed\n");
        return 0;
    }

//    memset(listening_fd, -1, sizeof(listening_fd));
//    memset(command_fd, -1, sizeof(command_fd));
//    memset(data_fd, -1, sizeof(data_fd));

    //strcpy(default_cwd, "/home/apple/Desktop");
    strcpy(default_cwd, root);
    for(int i=0; i<MAX_FD; i++)
    {
        clients[i].listening_fd=-1;
        clients[i].command_fd=-1;
        clients[i].data_fd=-1;
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
        clients[i].data_transfersd = 0;
    }

    chdir(default_cwd);
    FD_ZERO(&fd_read);
    FD_SET(socket_fd, &fd_read);

    srand((unsigned int) (int)time(NULL));

    timeout.tv_sec=0;
    timeout.tv_usec=0;

    return 1;
}

int server_run()
{
    max_fd = socket_fd;
    int err;
    int connfd;
    struct sockaddr_in cli_addr;
    socklen_t cli_len;

    char buffer[BUF_SIZE]={0};
    char params[BUF_SIZE]={0};
    char response[BUF_SIZE]={0};

    while (1)
    {
        fd_select = fd_read;
        err = select(max_fd+1, &fd_select, NULL, NULL, &timeout);
        if(err<0)
        {
            printf("error selecting\n");
            return 0;
        }
//        if(err==0)
//        {
//            printf("timeout\n");
//            return 0;
//        }
        if(FD_ISSET(socket_fd, &fd_select))
        {
            cli_len = sizeof(cli_addr);
            connfd = accept(socket_fd, (struct sockaddr *)&cli_addr, &cli_len);
            if(connfd < 0)
            {
                printf("fail to accept\n");
                return 0;
            }
            for(int i=0; i<MAX_FD; i++)
            {
                if(clients[i].command_fd==-1)
                {
                    clients[i].command_fd=connfd;
                    memset(response, 0, sizeof(response));
                    int len=make_response(220, "FTP server ready", response);
                    int num_write= (int) write(clients[i].command_fd, response, len);

                    break;
                }
            }
            FD_SET(connfd, &fd_read);
            if(max_fd<connfd)
            {
                max_fd=connfd;
            }
        }

        //command socket receive message

        for(int i=0; i<MAX_FD; i++)
        {
            if(FD_ISSET(clients[i].command_fd, &fd_select))
            {
                memset(buffer,0, sizeof(buffer));
                memset(params, 0, sizeof(params));
                memset(response, 0, sizeof(response));

                int num_read = (int) read(clients[i].command_fd, buffer, BUF_SIZE);
                int command_id;

                int response_len=0;
                int num_write=0;
                if(num_read>0)
                {
                    command_id = parse_command(buffer, params);
                    //judge if this client is transfering data
                    if(clients[i].transfer_state==-1)
                    {
                        switch (command_id)
                        {
                            case USER:
                                if(!user_command(i,params))
                                {
                                    response_len = make_response(404, "guest login error", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = USER;
                                break;
                            case PASS:
                                if(!pass_command(i, params))
                                {
                                    response_len = make_response(404, "guest login error", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = PASS;
                                break;
                            case RETR:
                                if(!retr_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in RETR command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = RETR;
                                break;
                            case STOR:
                                if(!stor_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in STOR command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = STOR;
                                break;
                            case QUIT:
                                quit_command(i, params);
                                clients[i].last_command = QUIT;
                                break;
                            case SYST:
                                response_len = make_response(215, "UNIX Type: L8", response);
                                num_write = (int) write(clients[i].command_fd, response,  response_len);
                                clients[i].last_command = SYST;
                                break;
                            case TYPE:
                                if(!type_command(i,params))
                                {
                                    response_len = make_response(404, "unknown error in TYPE command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = TYPE;
                                break;
                            case PORT:
                                if(!port_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in PORT command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = PORT;
                                break;
                            case PASV:
                                if(!pasv_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in PASV command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = PASV;
                                break;
                            case MKD:
                                if(!mkd_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in MKD command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = MKD;
                                break;
                            case CWD:
                                if(!cwd_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in CWD command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = CWD;
                                break;
                            case PWD:
                                if(!pwd_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in PWD command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = PWD;
                                break;
                            case LIST:
                                if(!list_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in LIST command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = LIST;
                                break;
                            case RMD:
                                if(!rmd_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in RMD command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = RMD;
                                break;
                            case RNFR:
                                if(!rnfr_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in RNFR command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                break;
                            case RNTO:
                                if(!rnto_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in RNTO command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = RNTO;
                                break;
                            case REST:
                                if(!rest_command(i, params))
                                {
                                    response_len = make_response(404, "unknown error in REST command", response);
                                    num_write = (int) write(clients[i].command_fd, response, response_len);
                                }
                                clients[i].last_command = REST;
                                break;
                            default:
                                response_len = make_response(404, "incorrect command", response);
                                num_write = (int) write(clients[i].command_fd, response, response_len);
                                clients[i].last_command = -1;
                                break;
                        }
                    }

                }
                else if(num_read==0)
                {
                    printf("client:command_fd[%d] exit\n", i);
                    FD_CLR(clients[i].command_fd, &fd_read);
                    close(clients[i].command_fd);
                    clients[i].command_fd = -1;
                    set_client_default(i);
                }
            }
            if(FD_ISSET(clients[i].listening_fd, &fd_select))
            {
                cli_len = sizeof(cli_addr);
                connfd = accept(clients[i].listening_fd, (struct sockaddr *)&cli_addr, &cli_len);
                if(connfd < 0)
                {
                    printf("fail to accept %d data connection\n", i);
                }
                else
                {
                    printf("%d data connection succeed\n", i);
                    clients[i].data_fd = connfd;
                    FD_SET(connfd, &fd_read);
                    if(max_fd<connfd)
                        max_fd = connfd;
                    FD_CLR(clients[i].listening_fd, &fd_read);
                    close(clients[i].listening_fd);
                    clients[i].listening_fd = -1;

                }
            }
            if(clients[i].transfer_state==1)//LIST
            {
                transfer_list(i);
            }
            else if(clients[i].transfer_state==2)//RETR
            {
                retr_file(i);
            }
            else if(clients[i].transfer_state==3)
            {
//                if(FD_ISSET(clients[i].data_fd, &fd_select))
//                {
//                    storfile(i);
//                }
                storfile(i);
            }

        }
    }

    return 1;

}

int main(int argc, const char *argv[]) {

    int port=SERV_PORT;
    char root[PATH_SIZE] = {0};
    strcpy(root, "/tmp");
    //strcpy(root, "/home/apple/Desktop");
    for(int i=0; i<argc; i++)
    {
        if(!strcmp(argv[i], "-port"))
        {
            port = str_to_int(argv[i+1]);
            i++;
        }
        else if(!strcmp(argv[i], "-root"))
        {
            memset(root, 0, sizeof(root));
            strcpy(root, argv[i+1]);
            i++;
        }
    }
    if(!establish(port, root))
    {
        printf("error when setting up server");
        return 0;
    }

    server_run();

    return 0;
}

