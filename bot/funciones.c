#include "funciones.h"
/*anadido return -1*/

char * posiciona(char crt, char *str){
    uint i = 0;
    if(str == NULL) return NULL;
    for(i = 0; i < strlen(str); i++){
        if(str[i] == crt) break;
    }
    return &str[i+1];
}

int check_usr(char * usr){
    size_t size = 0;
    char *buff = NULL;
    while(getline(&buff, &size, pconff) != -1){
        char *p = NULL;
        if((p = strchr(buff, '\n')) != NULL) *p = '\0';
        if(strcmp(usr, buff) == 0){
            free(buff);
            fseek(pconff, 0, SEEK_SET);
            return 1;
        }
        if(buff) free(buff);
        buff = NULL;
    }
    fseek(pconff, 0, SEEK_SET);
    free(buff);
    return 0;
}

void * servRecv(void *args){
    char buf[BUFFER_SIZE], aux2[BUFFER_SIZE], ch[128], aux3[BUFFER_SIZE], printbuf[BUFFER_SIZE + 16];
    char *command, *usr, *trash, *ultra_trash, *maximum_trash, *p;
    int aux;
    while(1){
        printbuf[0] = '\0';
        ultra_trash = trash = usr = command = p = maximum_trash = NULL;
        if(0 < (aux = recibir(sockfd, buf))){
            strcpy(aux3, buf);
            while((p = strchr(aux3, '\r')) != NULL) *p = '/';
            sprintf(printbuf, "Recibido: %s", aux3);
            if (strstr(aux3, " PONG") == NULL) printout(0, printbuf);
            //if (strstr(aux3, "raspberry@") != NULL) excptloro = 1;
            usr = strtok (buf,"!");
            if(usr == NULL) continue;
            trash = strtok (NULL," ");
            if(trash == NULL) continue;
            command = strtok (NULL," ");
            if(command == NULL) continue;
            trash = strtok (NULL," ");
            strcpy(ch, trash);
            if(strcmp(command, "JOIN") == 0){
                aux2[0] = '0';
                sprintf(aux2, "MODE #cosas +o %s%c%c", usr, 0x0d, 0x0a);
                printbuf[0] = '\0';
                sprintf(printbuf, "Enviado: MODE #cosas +o %s\n", usr);
                printout(0, printbuf);
                escribir(sockfd, aux2);
            } else if(strcmp(command, "PRIVMSG") == 0){
                trash = strtok(NULL, "");
                ultra_trash = malloc(sizeof(char) * strlen(trash) + 3);
                strcpy(ultra_trash, &trash[1]);
                maximum_trash = malloc(strlen(trash) + strlen(usr) + strlen("PRIVMSG") + 5);
                //wprintw(output_win, "EXCPTLORO=%i\n", excptloro);
                if(iscommand(ultra_trash) == 0 && loro == 1){
                    sprintf(maximum_trash, "PRIVMSG %s :%s", ch, ultra_trash);
                    p = strchr(maximum_trash, '\r');
                    *(p + 2) = '\0';
                    escribir(sockfd, maximum_trash);
                    strcpy(aux3, maximum_trash);
                    while((p = strchr(aux3, '\r')) != NULL) *p = '/';
                    printbuf[0] = '\0';
                    sprintf(printbuf, "Enviado: %s", aux3);
                    printout(0, printbuf);
                    if(maximum_trash){
                        free(maximum_trash);
                        maximum_trash = NULL;
                    }
                } else if(iscommand(ultra_trash) == 0 && rtfmv == 1){
                    sprintf(maximum_trash, "PRIVMSG %s :RTFM\n\r", ch);
                    fprintf(stderr, "%s\n", maximum_trash);
                    escribir(sockfd, maximum_trash);
                    strcpy(aux3, maximum_trash);
                    while((p = strchr(aux3, '\r')) != NULL) *p = '/';
                    printbuf[0] = '\0';
                    sprintf(printbuf, "Enviado: %s", aux3);
                    printout(0, printbuf);
                    if(maximum_trash){
                        free(maximum_trash);
                        maximum_trash = NULL;
                    }
                } else if(check_usr(&usr[1]) != 0){
                    if(strncmp(ultra_trash, "SEND", strlen("SEND")) == 0){
                        sendv = 1;
                        printout(0, "SEND\n");
                    }else if(strncmp(ultra_trash, "NSEND", strlen("NSEND")) == 0){
                        sendv = 0;
                        printout(0, "NSEND\n");
                    } else if(strncmp(ultra_trash, "LORO", strlen("LORO")) == 0){
                        loro = 1;
                        printout(0, "LORO\n");
                    } else if(strncmp(ultra_trash, "NLORO", strlen("NLORO")) == 0){
                        loro = 0;
                        printout(0, "NLORO\n");
                    } else if (strncmp(ultra_trash, "RTFM", strlen("RTFM")) == 0){
                        rtfmv = 1;
                        printout(0, "RTFM\n");
                    } else if (strncmp(ultra_trash, "NRTFM", strlen("NRTFM")) == 0){
                        rtfmv = 0;
                        printout(0, "NRTFM\n");
                    }else if (sendv == 1 && iscommand(ultra_trash) == 1){
                        escribir(sockfd, ultra_trash);
                        strcpy(aux3, ultra_trash);
                        while((p = strchr(aux3, '\r')) != NULL) *p = '/';
                        printbuf[0] = '\0';
                        sprintf(printbuf, "Enviado: %s", aux3);
                        printout(0, printbuf);
                    }
                }
                if(maximum_trash){
                    free(maximum_trash);
                    maximum_trash = NULL;
                }
                excptloro = 0;
                if(ultra_trash){
                    free(ultra_trash);
                    ultra_trash = NULL;
                }
            } /*else{
                if(strstr(usr, "raspberry") == NULL){
                maximum_trash[0] = '\0';
                char dest[] = "dr_nick";
                sprintf(maximum_trash, "PRIVMSG %s :<%s@%s> %s", dest, &usr[1], ch, command);
                char* p;
                while((p = strchr(maximum_trash, '\n')) != NULL) *p = '/';
                wprintw(output_win, "maximum_trash:%s\n", maximum_trash);
                fflush(stdout);
                escribir(sockfd, maximum_trash);
                }
                }*/
        }
    }
    printout(0, "Terminada la conexion\n");
}

int iscommand(char* s){
    if(strncmp(s,"PRIVMSG",strlen("PRIVMSG"))==0){
        return 1;
    } else if(strncmp(s,"JOIN",strlen("JOIN"))==0){
        return 1;
    } else if(strncmp(s,"NICK",strlen("NICK"))==0){
        return 1;
    } else if(strncmp(s,"SEND",strlen("SEND"))==0){
        return 1;
    } else if(strncmp(s,"MODE",strlen("MODE"))==0){
        return 1;
    } else if(strncmp(s,"NOTICE",strlen("NOTICE"))==0){
        return 1;
    } else if(strncmp(s,"QUIT",strlen("QUIT"))==0){
        return 1;
    } else if(strncmp(s,"INVITE",strlen("INVITE"))==0){
        return 1;
    } else if(strncmp(s,"TOPIC",strlen("TOPIC"))==0){
        return 1;
    } else if(strncmp(s,"PART",strlen("PART"))==0){
        return 1;
    } else if(strncmp(s,"LORO",strlen("LORO"))==0){
        return 1;
    } else if(strncmp(s,"NLORO",strlen("NLORO"))==0){
        return 1;
    } else if(strncmp(s,"RTFM",strlen("RTFM"))==0){
        return 1;
    } else if(strncmp(s,"NRTFM",strlen("NRTFM"))==0){
        return 1;
    }
    return 0;
}

void printout(int err, char *s){
    if(s == NULL) return;
    if(IS_CURSES(mode)){
        wprintw(output_win, "%s", s);
        wrefresh(output_win);
    } else if((IS_SILENT(mode) && err == 1) || !IS_SILENT(mode)){
        fprintf(stdout, "%s", s);
        fflush(stdout);
    }
    fprintf(plogf, "%s", s);
}

void readconf(){
    size_t size = 0;
    char *buff = NULL;
    while(getline(&buff, &size, pconff) != -1){
        char *p;
        if((p = strchr(buff, '\n')) != NULL) *p = '\0';
        if(strncmp(buff, "SERVER:", strlen("SERVER:")) == 0){
            p = strchr(buff, ':');
            strcpy(servername, ++p);
        } else if(strncmp(buff, "PORT:", strlen("PORT:")) == 0){
            p = strchr(buff, ':');
            port = atoi(++p);
        } else if(strncmp(buff, "NICK:", strlen("NICK:")) == 0){
            p = strchr(buff, ':');
            strcpy(nick_s, ++p);
        }
        if(buff) free(buff);
        buff = NULL;
    }
    if(buff) free(buff);
    buff = NULL;
    fseek(pconff, 0, SEEK_SET);
}

void connect_client(pthread_t* h1, pthread_t* h2){
    struct addrinfo hints, *res = NULL;
    int nick;// user;
    char port_s[10];
    char command[256];
    char printbuf[128];
    printbuf[0] = '\0';
    sprintf(printbuf, "Conectando a %s:%i\n", servername, port);
    printout(0, printbuf);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /*Comenzamos la conexion TCP*/
    printout(0, "Se obtiene iformacion\n");
    sprintf(port_s, "%i", port);
    if(0!=getaddrinfo(servername, port_s, &hints, &res)){
        printout(1, "Error al onbtener informacion del servidor\n");
        return;
    }
    printout(0, "socket\n");
    sockfd=abrirSocketTCP();
    if(sockfd==-1){
        printout(1, "Error al crear el socket\n");
        return;
    }
    printout(0, "connect\n");

    if(-1==abrirConnect(sockfd, *(res->ai_addr))){
        printout(1, "Error al conectar\n");
        return;
    }
    if(res){
        freeaddrinfo(res);
        res = NULL;
    }
    /*Conexion IRC*/
    printout(0, "IRC\n");

    pthread_create(h1,NULL, servRecv, (void *)NULL );
    pthread_detach(*h1);

    sprintf(command, "NICK %s%c%cUSER %s %s %s :%s%c%c", nick_s, 0X0d, 0X0d, nick_s, nick_s, servername, nick_s, 0X0d, 0X0d);
    nick=escribir(sockfd,command);
    char* p = NULL;
    while((p = strchr(command, '\r')) != NULL) *p = '/';
    printbuf[0] = '\0';
    sprintf(printbuf, "%s, %d\n", command,nick);
    printout(0, printbuf);

    //sleep(1);
    pthread_create(h2, NULL, Ping, (void *)NULL );

    if(res){
        freeaddrinfo(res);
        res = NULL;
    }
}

void * Ping(void *args){
    while(1){
        char command[30];
        sleep(20);
        sprintf(command,"PING metis.ii.uam.es%c%c",0X0d,0X0d);
        escribir(sockfd,command);
        //wprintw(output_win, "%s", command);
    }
}

int abrirSocketTCP(){
    int sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sockfd < 0){
        switch(errno){
            case EACCES:
                printout(1, "Permission to create a socket of the specified type and/or  protocol is denied.\n");
                break;
            case EAFNOSUPPORT:
                printout(1, "The  implementation  does not support the specified address family.\n");
                break;
            case EINVAL:
                printout(1, "Unknown protocol, or protocol family not available. or Invalid flags in type.\n");
                break;
            case EMFILE:
                printout(1, "Process file table overflow.\n");
                break;
            case ENFILE:
                printout(1, "The system limit on the total number  of  open  files  has  been reached.\n");
                break;
            case ENOBUFS:
                printout(1, "Insufficient  memory is available.  The socket cannot be created until sufficient resources are freed.\n");
                break;
            case ENOMEM:
                printout(1, "Insufficient  memory is available.  The socket cannot be created until sufficient resources are freed.\n");
                break;
            case EPROTONOSUPPORT:
                printout(1, "The protocol type or the specified  protocol  is  not  supportedwithin this domain.\n" );
                break;
        }
        return -1;
    }
    return sockfd;

}
/*anadido return -1*/
int abrirSocketUDP(){
    int sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(sockfd < 0){
        switch(errno){
            case EACCES:
                printout(1, "Permission to create a socket of the specified type and/or  protocol is denied.\n");
                break;
            case EAFNOSUPPORT:
                printout(1, "The  implementation  does not support the specified address family.\n");
                break;
            case EINVAL:
                printout(1, "Unknown protocol, or protocol family not available. or Invalid flags in type.\n");
                break;
            case EMFILE:
                printout(1, "Process file table overflow.\n");
                break;
            case ENFILE:
                printout(1, "The system limit on the total number  of  open  files  has  been reached.\n");
                break;
            case ENOBUFS:
                printout(1, "Insufficient  memory is available.  The socket cannot be created until sufficient resources are freed.\n");
                break;
            case ENOMEM:
                printout(1, "Insufficient  memory is available.  The socket cannot be created until sufficient resources are freed.\n");
                break;
            case EPROTONOSUPPORT:
                printout(1, "The protocol type or the specified  protocol  is  not  supportedwithin this domain.\n" );
                break;

        }
        return -1;
    }
    return sockfd;

}
int abrirConnect(int sockfd, struct sockaddr res){
    int cnct=0;
    cnct= connect(sockfd, &res, sizeof(res));
    if (cnct==-1){
        switch(errno){
            case EACCES:
                printout(1, "For UNIX domain sockets, which are identified by pathname: Write permission is denied on the socket file, or search permission is denied for one of the directories in the path prefix. (See also path_resolution(7).) \n");
                printout(1, "or The user tried to connect to a broadcast address without having the socket broadcast flag enabled or the connection request failed because of a local firewall rule.\n" );
                break;
            case EPERM:
                printout(1, "The user tried to connect to a broadcast address without having the socket broadcast flag enabled or the connection request failed because of a local firewall rule.\n");
                break;
            case EADDRINUSE:
                printout(1, "Local address is already in use. \n");
                break;
            case EAFNOSUPPORT:
                printout(1, "The passed address didn't have the correct address family in its sa_family field. \n");
                break;
            case EAGAIN:
                printout(1, "No more free local ports or insufficient entries in the routing cache. For AF_INET see the description of /proc/sys/net/ipv4/ip_local_port_range ip(7) for information on how to increase the number of local ports. \n");
                break;
            case EALREADY:
                printout(1, "The socket is nonblocking and a previous connection attempt has not yet been completed.\n");
                break;
            case EBADF:
                printout(1, "The file descriptor is not a valid index in the descriptor table.\n");
                break;
            case ECONNREFUSED:
                printout(1, "No-one listening on the remote address. \n");
                break;
            case EFAULT:
                printout(1, "The socket structure address is outside the user's address space. \n");
                break;
            case EINPROGRESS:
                printout(1, "The socket is nonblocking and the connection cannot be completed immediately. It is possible to select(2) or poll(2) for completion by selecting the socket for writing. After select(2) indicates writability, use getsockopt(2) to read the SO_ERROR option at level SOL_SOCKET to determine whether connect() completed successfully (SO_ERROR is zero) or unsuccessfully (SO_ERROR is one of the usual error codes listed here, explaining the reason for the failure). \n");
                break;
            case EINTR:
                printout(1, "The system call was interrupted by a signal that was caught; see signal(7).\n");
                break;
            case EISCONN:
                printout(1, "The socket is already connected.\n");
                break;
            case ENETUNREACH:
                printout(1, "Network is unreachable. \n");
                break;
            case ENOTSOCK:
                printout(1, "The file descriptor is not associated with a socket.\n");
                break;
            case ETIMEDOUT:
                printout(1, "Timeout while attempting connection. The server may be too busy to accept new connections. Note that for IP sockets the timeout may be very long when syncookies are enabled on the server.\n");
                break;
        }
        return -1;
    }
    return 0;
}

int recibir(int sockfd,char *buf){
    int aux=0;
    aux = recv(sockfd, buf, 1000, 0);
    buf[aux]='\0';
    if (aux==-1){
        switch(errno){
            case (EAGAIN || EWOULDBLOCK):
                printout(1, "The socket is marked nonblocking and the receive operation would block, or a receive timeout had been set and the timeout"
                        "expired before data was received.  POSIX.1-2001 allows either  error  to"
                        "be  returned for this case, and does not require these constants to have the same value, so a portable application  should  check"
                        "for both possibilities.\n");
                break;
            case EBADF:
                printout(1, "The argument sockfd is an invalid descriptor.\n");
                break;
            case ECONNREFUSED:
                printout(1, "A remote host refused to allow the network connection (typically because it is not running the requested service).\n");
                break;
            case EFAULT:
                printout(1, "The  receive  buffer  pointer(s)  point  outside  the  process's address space.\n");
                break;
            case EINTR:
                printout(1, "The  receive  was interrupted by delivery of a signal before any data were available; see signal(7).\n");
                break;
            case EINVAL:
                printout(1, "Invalid argument passed.\n");
                break;
            case ENOMEM:
                printout(1, "Could not allocate memory for recvmsg().\n");
                break;
            case ENOTCONN:
                printout(1, "The socket is associated with a connection-oriented protocol and has not been connected (see connect(2) and accept(2)).\n");
                break;
            case ENOTSOCK:
                printout(1, "The argument sockfd does not refer to a socket.\n");
                break;
        }
        return -1;
    }
    return aux;
}
int escribir(int sockfd,char *msg){
    int aux = send(sockfd, msg, strlen(msg), 0);
    if(-1 == aux){
        switch(errno){
            case EACCES:
                printout(1, "(For UNIX domain sockets, which are identified by pathname) Write permission is denied on the destination socket file, or search permission  is"
                        "denied for one of the directories the path prefix.  (See path_resolution(7).)"\
                        "(For UDP sockets) An attempt was made to send to a network/broadcast address as though it was a unicast address.\n");
                break;
            case (EAGAIN || EWOULDBLOCK):
                printout(1, " The  socket  is marked nonblocking and the requested operation would block.  POSIX.1-2001 allows either error to be returned for this case, and"
                        "does not require these constants to have the same value, so a portable application should check for both possibilities.\n");
                break;
            case EBADF:
                printout(1, "An invalid descriptor was specified.\n");
                break;
            case ECONNRESET:
                printout(1, "Connection reset by peer.\n");
                break;
            case EDESTADDRREQ:
                printout(1, "The socket is not connection-mode, and no peer address is set.\n");
                break;
            case EFAULT:
                printout(1, "An invalid user space address was specified for an argument.\n");
                break;
            case EINTR:
                printout(1, "A signal occurred before any data was transmitted; see signal(7).\n");
                break;
            case EINVAL:
                printout(1, "Invalid argument passed.\n");
                break;
            case EISCONN:
                printout(1, "The connection-mode socket was connected already but a recipient was specified.  (Now either this error is returned, or the recipient  specifi‐"
                        "cation is ignored.)\n");
                break;
            case EMSGSIZE:
                printout(1, "The socket type requires that message be sent atomically, and the size of the message to be sent made this impossible.\n");
                break;
            case ENOBUFS:
                printout(1, "The output queue for a network interface was full.  This generally indicates that the interface has stopped sending, but may be caused by tran‐"
                        "sient congestion.  (Normally, this does not occur in Linux.  Packets are just silently dropped when a device queue overflows.)\n");
                break;
            case ENOMEM:
                printout(1, "No memory available.\n");
                break;
            case ENOTCONN:
                printout(1, "The socket is not connected, and no target has been given.\n");
                break;
            case ENOTSOCK:
                printout(1, "The argument sockfd is not a socket.\n");
                break;
            case EOPNOTSUPP:
                printout(1, "Some bit in the flags argument is inappropriate for the socket type.\n");
                break;
            case EPIPE:
                printout(1, "The local end has been shut down on a connection oriented socket.  In this case the process will also receive a SIGPIPE unless MSG_NOSIGNAL  is set.\n");
                break;
        }
        return -1;
    }
    return aux;
}
/*int conexionPruebaCliente(char * direccion,char * puerto){
  struct addrinfo hints, *res;
  int sockfd, sock_server;
  char msg[1000];
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
  hints.ai_socktype = SOCK_STREAM;
  wprintw(output_win, "Abriendo socket\n");
  getaddrinfo("172.16.187.180", "2020", &hints, &res);
  sockfd = abrirSocketTCP();
  if (sockfd==-1){
  wprintw(output_win, "ERROR: socket\n");
  return -1;
  }

  wprintw(output_win, "Conectando\n");
  if(-1==(sock_server==abrirConnect(sockfd,*(res->ai_addr)))){
  wprintw(output_win, "ERROR: connect\n");
  return -1;
  }

// convert to network byte order
// send data normally:
while (1){
recibir(sockfd, msg);
wprintw(output_win, "Recibido:%s\n", msg);
strcpy(msg,"PONG");
sleep(2);
//send(sockfd,msg,strlen(msg),0);
escribir(sockfd, msg);
}
return 0;
}
*/

/*int conexionPruebaServidor(){
  int sockfd, socketClient, aux = 1;
  struct sockaddr_in ip4addr;
  char buf[1000];
  wprintw(output_win, "Abriendo socket\n");
  sockfd = abrirSocketTCP();
  if (sockfd==-1){
  wprintw(output_win, "ERROR: socket\n");
  return -1;
  }
  wprintw(output_win, "Abriedo bind\n");
  if( -1==abrirBind(sockfd)){
  wprintw(output_win, "ERROR: bind\n");
  return -1;
  }
  wprintw(output_win, "Escuchando\n");
  if(-1==abrirListen(sockfd)){
  wprintw(output_win, "ERROR: listen\n");
  return -1;
  }
  wprintw(output_win, "Esperando accept\n");
  socketClient = aceptar(sockfd, ip4addr);
  if(socketClient ==-1){
  wprintw(output_win, "ERROR: accept\n");
  return -1;
  }

  wprintw(output_win, "sock_client:%d, sockfd:%d\n", socketClient,sockfd);
  while( aux > 0){
  sleep(2);
  strcpy(buf,"PING");
  write(socketClient, buf , strlen(buf));
  aux=recv(socketClient, buf,1000,0);
  if(aux == 0){
  wprintw(output_win, "fin de la conexion\n");
  return 0;
  }
  buf[aux]='\0';
  wprintw(output_win, "Recibido:%s\n",buf );
  }
//byte_count = recv(sockfd, buf, sizeof buf, 0);
//wprintw(output_win, "recv()'d %d bytes of data in buf\n", byte_count);

return 0;
}*/
