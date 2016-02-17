#ifndef _BOT_H
#define _BOT_H
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>

#define IS_SILENT(k) (!k)
#define IS_CURSES(k) (k == 2) ? 1: 0

#define INIT_MODE int

#define LORO 1
#define NLORO 0
#define SEND 1
#define NSEND 0
#define SILENT 0
#define STDOUT 1
#define NCURSES 2

#define BUFFER_SIZE 8096


void connect_client(pthread_t* h1, pthread_t* h2);
int openTCPsocket();
int openUDPsocket();
int openBind(int sockfd,const struct sockaddr_in *addr);
int aceptar(int sockfd, struct sockaddr_in ip4addr);
int openConnect(int sockfd, struct sockaddr ip4addr);
int openListen(int sockfd);
int socketrecv(int sockfd,char *buf);
int socketwrite(int sockfd,char *msg);
void *ping(void *args);
void *servRecv(void *args);
int iscommand(char* s);
void printout(int err, char *s);
void readconf();


int sockfd, port;
int sendv, loro, rtfmv;
INIT_MODE mode;
WINDOW *input_win, *title_win, *output_win;
FILE *plogf, *pconff;
char servername[64], nick_s[16];

#endif

