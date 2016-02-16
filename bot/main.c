#include "funciones.h"
#include <getopt.h>

void intHandler(int);
void iniGlobales();
void initCurses();
int getCommand(char* msg);

pthread_t h1, h2;
char *whitespaces = NULL;

int main(int argc, char *argv[]){
    static struct option long_options[] = {
        {"raw", no_argument, &mode, STDOUT},
        {"silent", no_argument, &mode, SILENT},
        {0, 0, 0, 0}
    };
    pid_t pid = 0;
    int c, option_index = 0;
    mode = NCURSES;
    while(1){
        c = getopt_long(argc, argv, "rs", long_options, &option_index);
        if (c == -1) break;
    }
    if(IS_SILENT(mode)) pid = fork();
    if(pid == 0){
        char msg[512];
        int i = LINES - 5;
        if(IS_CURSES(mode)) initCurses();
        iniGlobales();
        connect_client(&h1, &h2);
        signal(SIGINT, intHandler);
        while(1){
            if(IS_CURSES(mode)){
                char* p;
                mvwgetnstr(input_win, LINES - 3, strlen("Message: "), msg, 512);
                if((p = strchr(msg, '\n')) != NULL) *p = '\0';
                if(strcmp(msg, "QUIT") == 0) break;
                mvwprintw(input_win, i, 0, "%s", msg);
                mvwprintw(input_win, LINES - 3, strlen("Message: "), whitespaces);
                mvwprintw(input_win, LINES - 2, 0, whitespaces);
                wrefresh(input_win);
                i -= 2;
                if(i - 2 <= 0) i = LINES - 5;
                mvwprintw(input_win, i, 0, whitespaces);
                mvwprintw(input_win, i + 1, 0, whitespaces);
                wrefresh(input_win);
                if(getCommand(msg)) continue;
                sprintf(msg, "%s%c%c", msg, 0X0d, 0X0d);
                escribir(sockfd, msg);
                wrefresh(output_win);
            } else {
                pause();
            }
        }
        if(whitespaces) free(whitespaces);
        pthread_cancel(h1);
        pthread_join(h1, NULL);
        pthread_cancel(h2);
        pthread_join(h2, NULL);
        if(IS_CURSES(mode)){
            delwin(input_win);
            delwin(title_win);
            delwin(output_win);
            endwin();
        }
        fclose(plogf);		/* End curses mode		  */
        fclose(pconff);
        return EXIT_SUCCESS;
    } else if(pid == -1){
        perror("fork");
        return EXIT_FAILURE;
    } else{
        return EXIT_SUCCESS;
    }
}

void intHandler(int k) {
    if(whitespaces) free(whitespaces);
    pthread_cancel(h1);
    pthread_join(h1, NULL);
    pthread_cancel(h2);
    pthread_join(h2, NULL);
    if(IS_CURSES(mode)){
        delwin(input_win);
        delwin(title_win);
        delwin(output_win);
        endwin();
    }
    fclose(plogf);
    fclose(pconff);
    exit(0);
}

void iniGlobales(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char logname[128];
    pconff = fopen("conf", "r");
    readconf();
    chdir(getenv("HOME"));
    sprintf(logname, "logs/%d-%.2d-%.2d%.2d%.2d%.2d.log", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    plogf = fopen(logname, "w+");
    if(plogf == NULL){
        exit(EXIT_FAILURE);
    }
    sendv = 1;
    rtfmv = 0;
    loro = 0;
    excptloro = 0;

    if(IS_CURSES(mode)){
        title_win = newwin(3, 3*COLS/4, 0, 0);
        input_win = newwin(LINES, (COLS/4), (LINES - LINES) / 2, (COLS - (COLS/4)));
        output_win = newwin(LINES-3, (3*COLS/4) - 2, 3, 1);
        scrollok(output_win, TRUE);

        mvwprintw(title_win, 1, 1, "IRCBot (QUIT to exit)");
        wrefresh(title_win);

        mvwprintw(input_win, LINES - 3, 0, "Message:");
        wrefresh(input_win);
        wrefresh(output_win);

        whitespaces = malloc((COLS/4) + 1);
        memset(whitespaces, ' ', (COLS/4));
        whitespaces[(COLS/4)] = '\0';
    }
}

void initCurses(){
    title_win = NULL;
    input_win = NULL;
    output_win = NULL;
    plogf = NULL;
    initscr();			/* Start curses mode 		*/
    cbreak();
    echo();
    keypad(stdscr, TRUE);
}

int getCommand(char* msg){
    int ret = 0;
    if(strcmp(msg, "SEND") == 0){
        sendv = SEND;
        ret = 1;
    }else if(strcmp(msg, "NSEND") == 0){
        sendv = NSEND;
        ret = 1;
    }else if(strcmp(msg, "LORO") == 0){
        loro = LORO;
        excptloro = 0;
        ret = 1;
    }else if(strcmp(msg, "NLORO") == 0){
        loro = NLORO;
        ret = 1;
    }else if(strcmp(msg, "RTFM") == 0){
        rtfmv = 1;
        ret = 1;
    }else if(strcmp(msg, "NTRFM") == 0){
        rtfmv = 0;
        ret = 1;
    }


    if (ret == 1) printout(0, msg);
    return ret;
}
