#include "bot.h"
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>

const char *IRCcommands[] = {"PRIVMSG", "JOIN", "NICK", "SEND", "MODE",
	"NOTICE", "INVITE", "TOPIC", "PART", NULL};
const char *specialcommands[] = {"QUIT", "LORO", "NLORO", "RTFM", "NRTFM", NULL};

bool check_usr(char * usr){
	size_t size = 0;
	char *buff = NULL;
	while(getline(&buff, &size, pconff) != -1){
		char *p = NULL;
		if((p = strchr(buff, '\n')) != NULL) *p = '\0';
		if(strcmp(usr, buff) == 0){
			free(buff);
			fseek(pconff, 0, SEEK_SET);
			return true;
		}
		free(buff);
		buff = NULL;
	}
	fseek(pconff, 0, SEEK_SET);
	free(buff);
	return false;
}

void printsendrecv(char* s, bool sent){
	char *p, *aux = malloc(strlen(s) + strlen("Received: ") + 1);
	strcpy(aux, s);
	while((p = strchr(aux, '\r')) != NULL) *p = '/';
	if (!sent){
		sprintf(aux, "%s", aux);
		if(strstr(aux, " PONG") == NULL) printout(false, aux);
		if(strstr(aux, "NOTICE") < strstr(aux, ":Connection statistics: ")){
			free(aux);
			aux = NULL;
			kill(getpid(), SIGINT);
		}
	} else {
		sprintf(aux, "%s", aux);
		printout(false, aux);
	}
	free(aux);
}

void obey(char* s){
	if(!strncmp(s, "SEND", strlen("SEND"))){
		sendv = true;
		printout(false, "SEND\n");
	}else if(!strncmp(s, "NSEND", strlen("NSEND"))){
		sendv = false;
		printout(false, "NSEND\n");
	} else if(!strncmp(s, "LORO", strlen("LORO"))){
		loro = true;
		printout(false, "LORO\n");
	} else if(!strncmp(s, "NLORO", strlen("NLORO"))){
		loro = false;
		printout(false, "NLORO\n");
	} else if (!strncmp(s, "RTFM", strlen("RTFM"))){
		rtfmv = true;
		printout(false, "RTFM\n");
	} else if (!strncmp(s, "NRTFM", strlen("NRTFM"))){
		rtfmv = false;
		printout(false, "NRTFM\n");
	} else if (sendv && iscommand(s)){
		socketwrite(sockfd, s);
		printsendrecv(s, true);
	}
}

void giveops(char* ch, char* usr){
	if(check_usr(usr)){
		char *s = malloc(strlen("MODE  +o \n") + strlen(ch) +  strlen(usr) + 2);
		sprintf(s, "MODE %s +o %s%c%c", ch, usr, 0x0d, 0x0a);
		socketwrite(sockfd, s);
		printsendrecv(s, true);
		free(s);
	}
}

void *servRecv(void *args){
	char buf[BUFFER_SIZE], ch[128];
	char *command, *usr, *info, *p;
	while(true){
		info = usr = command = p = NULL;
		if(0 < (socketrecv(sockfd, buf))){
			printsendrecv(buf, false);
			usr = strtok (buf,"!") + 1;
			if(usr == NULL) continue;
			info = strtok (NULL," ");
			if(info == NULL) continue;
			command = strtok (NULL," ");
			if(command == NULL) continue;
			info = strtok (NULL," ");
			strcpy(ch, info);
			while((p = strchr(ch, '\n')) != NULL) *p = '\0';
			while((p = strchr(ch, '\r')) != NULL) *p = '\0';
			info = strtok(NULL, "");
			if(!strcmp(command, "JOIN")){
				giveops(&ch[1], usr);
			} else if(!strcmp(command, "PRIVMSG")){
				char *msg = malloc(strlen(info) + strlen(usr) + strlen("PRIVMSG") + 5);
				if(!iscommand(&info[1])){
					if(loro){
						sprintf(msg, "PRIVMSG %s :%s", ch, &info[1]);
						socketwrite(sockfd, msg);
						printsendrecv(msg, true);
					}
					if(rtfmv){
						sprintf(msg, "PRIVMSG %s :RTFM\r\n", ch);
						socketwrite(sockfd, msg);
						printsendrecv(msg, true);
					}
				} else if(check_usr(usr)){
					obey(&info[1]);
				}
				free(msg);
			}
		}
	}
	printout(false, "Conection terminated.\n");
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
		free(buff);
		buff = NULL;
	}
	free(buff);
	buff = NULL;
	fseek(pconff, 0, SEEK_SET);
}

bool iscommand(char* s){
	int i;
	for(i = 0; IRCcommands[i] != NULL; i++) if(!strncmp(s, IRCcommands[i], strlen(IRCcommands[i]))) return true;
	for(i = 0; specialcommands[i]  != NULL; i++) if(!strncmp(s, specialcommands[i], strlen(specialcommands[i]))) return true;
	return false;
}

void printout(bool err, char *s){
	if(s == NULL) return;
	if(IS_CURSES(mode)){
		wprintw(output_win, "%s", s);
		wrefresh(output_win);
	} else if((IS_SILENT(mode) && err) || !IS_SILENT(mode)){
		fprintf(stdout, "%s", s);
		fflush(stdout);
	}
	fprintf(plogf, "%s", s);
}

void connect_client(pthread_t* h1, pthread_t* h2){
	struct addrinfo hints, *res = NULL;
	int nick;// user;
	char port_s[10];
	char command[256];
	char printbuf[128];
	char* p = NULL;
	printbuf[0] = '\0';
	sprintf(printbuf, "Connecting %s:%i\n", servername, port);
	printout(false, printbuf);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	/*Comenzamos la conexion TCP*/
	printout(false, "Getting information...\n");
	sprintf(port_s, "%i", port);
	if(0 != getaddrinfo(servername, port_s, &hints, &res)){
		printout(true, "Error getting information from the server.\n");
		return;
	}
	printout(false, "socket...\n");
	sockfd = openTCPsocket();
	if(sockfd == -1){
		printout(true, "Error creating socket.\n");
		return;
	}
	printout(false, "connect...\n");

	if(-1 == openConnect(sockfd, *(res->ai_addr))){
		printout(true, "Error connecting.\n");
		return;
	}
	freeaddrinfo(res);
	res = NULL;

	/*Conexion IRC*/
	printout(false, "IRC...\n");

	pthread_create(h1,NULL, servRecv, (void *)NULL );
	pthread_detach(*h1);

	sprintf(command, "NICK %s%c%cUSER %s %s %s :%s%c%c", nick_s, 0X0d, 0X0d, nick_s, nick_s, servername, nick_s, 0X0d, 0X0d);
	nick = socketwrite(sockfd, command);
	while((p = strchr(command, '\r')) != NULL) *p = '/';
	printbuf[0] = '\0';
	sprintf(printbuf, "%s, %d\n", command, nick);
	printout(false, printbuf);

	pthread_create(h2, NULL, ping, (void *) NULL);

	freeaddrinfo(res);
}

void *ping(void *args){
	while(1){
		char command[30];
		sleep(20);
		sprintf(command,"PING %s%c%c", servername, 0X0d, 0X0d);
		socketwrite(sockfd, command);
	}
}

int openTCPsocket(){
	int sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sockfd < 0){
		switch(errno){
			case EACCES:
				printout(true, "Permission to create a socket of the specified type and/or  protocol is denied.\n");
				break;
			case EAFNOSUPPORT:
				printout(true, "The  implementation  does not support the specified address family.\n");
				break;
			case EINVAL:
				printout(true, "Unknown protocol, or protocol family not available. or Invalid flags in type.\n");
				break;
			case EMFILE:
				printout(true, "Process file table overflow.\n");
				break;
			case ENFILE:
				printout(true, "The system limit on the total number  of  open  files  has  been reached.\n");
				break;
			case ENOBUFS:
				printout(true, "Insufficient  memory is available.  The socket cannot be created until sufficient resources are freed.\n");
				break;
			case ENOMEM:
				printout(true, "Insufficient  memory is available.  The socket cannot be created until sufficient resources are freed.\n");
				break;
			case EPROTONOSUPPORT:
				printout(true, "The protocol type or the specified  protocol  is  not  supportedwithin this domain.\n" );
				break;
		}
		return -1;
	}
	return sockfd;

}
/*anadido return -1*/
int openUDPsocket(){
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0){
		switch(errno){
			case EACCES:
				printout(true, "Permission to create a socket of the specified type and/or  protocol is denied.\n");
				break;
			case EAFNOSUPPORT:
				printout(true, "The  implementation  does not support the specified address family.\n");
				break;
			case EINVAL:
				printout(true, "Unknown protocol, or protocol family not available. or Invalid flags in type.\n");
				break;
			case EMFILE:
				printout(true, "Process file table overflow.\n");
				break;
			case ENFILE:
				printout(true, "The system limit on the total number  of  open  files  has  been reached.\n");
				break;
			case ENOBUFS:
				printout(true, "Insufficient  memory is available.  The socket cannot be created until sufficient resources are freed.\n");
				break;
			case ENOMEM:
				printout(true, "Insufficient  memory is available.  The socket cannot be created until sufficient resources are freed.\n");
				break;
			case EPROTONOSUPPORT:
				printout(true, "The protocol type or the specified  protocol  is  not  supportedwithin this domain.\n" );
				break;

		}
		return -1;
	}
	return sockfd;

}
int openConnect(int sockfd, struct sockaddr res){
	int cnct=0;
	cnct = connect(sockfd, &res, sizeof(res));
	if (cnct == -1){
		switch(errno){
			case EACCES:
				printout(true, "For UNIX domain sockets, which are identified by pathname: Write permission is denied on the socket file, or search permission is denied for one of the directories in the path prefix. (See also path_resolution(7).) \n");
				printout(true, "or The user tried to connect to a broadcast address without having the socket broadcast flag enabled or the connection request failed because of a local firewall rule.\n" );
				break;
			case EPERM:
				printout(true, "The user tried to connect to a broadcast address without having the socket broadcast flag enabled or the connection request failed because of a local firewall rule.\n");
				break;
			case EADDRINUSE:
				printout(true, "Local address is already in use. \n");
				break;
			case EAFNOSUPPORT:
				printout(true, "The passed address didn't have the correct address family in its sa_family field. \n");
				break;
			case EAGAIN:
				printout(true, "No more free local ports or insufficient entries in the routing cache. For AF_INET see the description of /proc/sys/net/ipv4/ip_local_port_range ip(7) for information on how to increase the number of local ports. \n");
				break;
			case EALREADY:
				printout(true, "The socket is nonblocking and a previous connection attempt has not yet been completed.\n");
				break;
			case EBADF:
				printout(true, "The file descriptor is not a valid index in the descriptor table.\n");
				break;
			case ECONNREFUSED:
				printout(true, "No-one listening on the remote address. \n");
				break;
			case EFAULT:
				printout(true, "The socket structure address is outside the user's address space. \n");
				break;
			case EINPROGRESS:
				printout(true, "The socket is nonblocking and the connection cannot be completed immediately. It is possible to select(2) or poll(2) for completion by selecting the socket for writing. After select(2) indicates writability, use getsockopt(2) to read the SO_ERROR option at level SOL_SOCKET to determine whether connect() completed successfully (SO_ERROR is zero) or unsuccessfully (SO_ERROR is one of the usual error codes listed here, explaining the reason for the failure). \n");
				break;
			case EINTR:
				printout(true, "The system call was interrupted by a signal that was caught; see signal(7).\n");
				break;
			case EISCONN:
				printout(true, "The socket is already connected.\n");
				break;
			case ENETUNREACH:
				printout(true, "Network is unreachable. \n");
				break;
			case ENOTSOCK:
				printout(true, "The file descriptor is not associated with a socket.\n");
				break;
			case ETIMEDOUT:
				printout(true, "Timeout while attempting connection. The server may be too busy to accept new connections. Note that for IP sockets the timeout may be very long when syncookies are enabled on the server.\n");
				break;
		}
		return -1;
	}
	return 0;
}

int socketrecv(int sockfd, char *buf){
	int aux=0;
	aux = recv(sockfd, buf, 1000, 0);
	buf[aux] = '\0';
	if (aux == -1){
		switch(errno){
			case (EAGAIN || EWOULDBLOCK):
				printout(true, "The socket is marked nonblocking and the receive operation would block, or a receive timeout had been set and the timeout"
						"expired before data was received.  POSIX.1-2001 allows either  error  to"
						"be  returned for this case, and does not require these constants to have the same value, so a portable application  should  check"
						"for both possibilities.\n");
				break;
			case EBADF:
				printout(true, "The argument sockfd is an invalid descriptor.\n");
				break;
			case ECONNREFUSED:
				printout(true, "A remote host refused to allow the network connection (typically because it is not running the requested service).\n");
				break;
			case EFAULT:
				printout(true, "The  receive  buffer  pointer(s)  point  outside  the  process's address space.\n");
				break;
			case EINTR:
				printout(true, "The  receive  was interrupted by delivery of a signal before any data were available; see signal(7).\n");
				break;
			case EINVAL:
				printout(true, "Invalid argument passed.\n");
				break;
			case ENOMEM:
				printout(true, "Could not allocate memory for recvmsg().\n");
				break;
			case ENOTCONN:
				printout(true, "The socket is associated with a connection-oriented protocol and has not been connected (see connect(2) and accept(2)).\n");
				break;
			case ENOTSOCK:
				printout(true, "The argument sockfd does not refer to a socket.\n");
				break;
		}
		return -1;
	}
	return aux;
}
int socketwrite(int sockfd, char *msg){
	int aux = send(sockfd, msg, strlen(msg), 0);
	if(-1 == aux){
		switch(errno){
			case EACCES:
				printout(true, "(For UNIX domain sockets, which are identified by pathname) Write permission is denied on the destination socket file, or search permission  is"
						"denied for one of the directories the path prefix.  (See path_resolution(7).)"\
						"(For UDP sockets) An attempt was made to send to a network/broadcast address as though it was a unicast address.\n");
				break;
			case (EAGAIN || EWOULDBLOCK):
				printout(true, " The  socket  is marked nonblocking and the requested operation would block.  POSIX.1-2001 allows either error to be returned for this case, and"
						"does not require these constants to have the same value, so a portable application should check for both possibilities.\n");
				break;
			case EBADF:
				printout(true, "An invalid descriptor was specified.\n");
				break;
			case ECONNRESET:
				printout(true, "Connection reset by peer.\n");
				break;
			case EDESTADDRREQ:
				printout(true, "The socket is not connection-mode, and no peer address is set.\n");
				break;
			case EFAULT:
				printout(true, "An invalid user space address was specified for an argument.\n");
				break;
			case EINTR:
				printout(true, "A signal occurred before any data was transmitted; see signal(7).\n");
				break;
			case EINVAL:
				printout(true, "Invalid argument passed.\n");
				break;
			case EISCONN:
				printout(true, "The connection-mode socket was connected already but a recipient was specified.  (Now either this error is returned, or the recipient  specifi‐"
						"cation is ignored.)\n");
				break;
			case EMSGSIZE:
				printout(true, "The socket type requires that message be sent atomically, and the size of the message to be sent made this impossible.\n");
				break;
			case ENOBUFS:
				printout(true, "The output queue for a network interface was full.  This generally indicates that the interface has stopped sending, but may be caused by tran‐"
						"sient congestion.  (Normally, this does not occur in Linux.  Packets are just silently dropped when a device queue overflows.)\n");
				break;
			case ENOMEM:
				printout(true, "No memory available.\n");
				break;
			case ENOTCONN:
				printout(true, "The socket is not connected, and no target has been given.\n");
				break;
			case ENOTSOCK:
				printout(true, "The argument sockfd is not a socket.\n");
				break;
			case EOPNOTSUPP:
				printout(true, "Some bit in the flags argument is inappropriate for the socket type.\n");
				break;
			case EPIPE:
				printout(true, "The local end has been shut down on a connection oriented socket.  In this case the process will also receive a SIGPIPE unless MSG_NOSIGNAL  is set.\n");
				break;
		}
		return -1;
	}
	return aux;
}
