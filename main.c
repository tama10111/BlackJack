/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: tama
 *
 * Created on 19 mars 2018, 12:22
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>

#include "player.h"
#include "gamesig.h"
#include "constant.h"

typedef struct gamevars_t{
    int numofplayers;
    int numofdecks;
    int numofhands;
    struct playervars_t* next;
} gamevars_t;

typedef struct playervars_t{
    int numoftokens;
    int bet;
    int stop;
    int goal;
    char bet_symbol;
    struct playervars_t* next;
} playervars_t;

typedef struct filevars_t{
    char player_hand[10]; 
    int phand_value;
    int bet;
    int gain;
    int ntoken;    
    char* path;
} filevars_t;

int ppow(int x,int n){
    int res=1;
    int i;
    for(i = 0; i<n; i++) res*=x;
    return res;
}


char* itoa(int i){
    char* a  = calloc(21, sizeof(char));
    int j, k;
    
    if(i == 0){
        a[0] = '0';
        return a;
    }

    for (j=0; j<20; j++){
        a[j] = i/(ppow(10,19-j));
        i -= (i/(ppow(10,19-j)))*ppow(10,19-j);
    }
    
    for(j=0; a[j] == 0; j++);
    for(k = 0; j < 20; j++, k++){
        a[k] = a[j] + '0'; 
        a[j] = 0;
    } return a;
}

int raiseError(int condition, char* error){
    if(condition){
        perror(error);
        printf("[%i] Exiting (%i)\n", getpid(), errno);
        exit(errno);
    } else return 1;
}

void free_playervars(playervars_t* p){
    playervars_t* tmp;
    while(p != 0){
        tmp = p->next;
        free(p);
        p = tmp;
    }
}

void freeHand(card_t* hand){
    card_t* tmp;    
    while(hand != NULL){
        tmp = hand->prev;
        free(hand);
        hand = tmp;
    }
}

void BUFFEROVERFLOOOOOOOW(){
    printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHHHHHHHHHHH BUFFER OVERFLOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOWWWWWWW :'(\n");
    exit(666);
}

//Fonction à rendre plus élégante
void load_file(gamevars_t* gfile, char* buf, int size){

    char str[22];
    memset(str, 0, 22);
    int comut = 1;
    int str_i = 0, gfile_i = 0, buf_i = 0, pfile_i = 0;
    
    while(comut){
        switch(buf[buf_i]){
            case ';' :
hell:
                *(((int*) gfile)+gfile_i) = atoi(str);
                if(gfile_i > 2) BUFFEROVERFLOOOOOOOW();
                str_i = 0; buf_i++, gfile_i++;
                memset(str, 0, 22);
                break;
            
            case '\n' :
                comut = 0;
                goto hell;
                break;
                
            default :
                str[str_i] = buf[buf_i];
                buf_i++; str_i++;
                if(str_i > 21) BUFFEROVERFLOOOOOOOW();
                break;
        }
    }  
    
    if(gfile->numofplayers > MAX_PLAYER){
        printf("Max number of players has been reached\nCurrent : %i\nMax : %i\n", gfile->numofplayers, MAX_PLAYER);
        exit(1);
    }
    
    playervars_t* pfile = calloc(1, sizeof(playervars_t));
    gfile->next = pfile;
    comut = 1;
    
    while(comut){
        switch(buf[buf_i]){
            case ';' :
                *(((int*) pfile)+pfile_i) = atoi(str);
                str_i = 0; buf_i++, pfile_i++;
                memset(str, 0, 22);
                break;
            
            case '\n' :
                *(((int*) pfile)+pfile_i) = atoi(str);
                if(pfile_i > 3) BUFFEROVERFLOOOOOOOW();
                str_i = 0; buf_i++, pfile_i = 0;
                memset(str, 0, 22);
                if(buf_i != size){
                    pfile->next = calloc(1, sizeof(playervars_t));
                    pfile = pfile->next;                                        
                } else{
                    comut = 0;
                }
                break;
                                                
            case '+' :
                pfile->bet_symbol = '+'; buf_i++;                
                break;

            case '-':
                pfile->bet_symbol = '-'; buf_i++;                
                break;
                
            default :
                str[str_i] = buf[buf_i];
                buf_i++; str_i++;
                if(str_i > 21) BUFFEROVERFLOOOOOOOW();                
                break;
        }
    }
}

int getCardValue(int card, int hand){   
    if(getValueFromCardID(card) == 1){
        if(hand <= 10) return 11;
        else return 1;
    } else return getValueFromCardID(card);  
}

char getCardChar(int id){
    char letter[13] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', 'X', 'J', 'Q', 'K'};
    return letter[id%13];
}

int main(int argc, char** argv) {    
       
    if(argc != 2){
        printf("SYNTAX : ./lv21 [FILE]\n");
        return EXIT_FAILURE;
    }
    
    int fd;
    raiseError((fd = open(argv[1], O_RDONLY)) == -1, "OPEN1");

    struct stat st;
    raiseError(fstat(fd, &st) == -1, "STAT");
    
    char buf[st.st_size];
    raiseError(read(fd, &buf, st.st_size) == -1, "READ");
    close(fd);

    gamevars_t gamefile;
    load_file(&gamefile, buf, st.st_size);
            
    playervars_t* p = gamefile.next;
    player_t player;
        
    pid_t l_pid[gamefile.numofplayers];
    
    /*
     * pipe_fd[x][0][0] PARENT READ
     * pipe_fd[x][0][1] CHILD WRITE
     * pipe_fd[x][1][0] CHILD READ
     * pipe_fd[x][1][1] PARENT WRITE
     */
    
    int pipe_fd[gamefile.numofplayers][2][2];
    
    int i, j;
    int pid;
    
    for(i = 0; i < gamefile.numofplayers; i++) raiseError(pipe(pipe_fd[i][0]) + pipe(pipe_fd[i][1]) != 0, "PIPE");

    for(i = 0; i<gamefile.numofplayers; i++){

        raiseError((pid = fork()) == -1, "FORK");

        l_pid[i] = pid;

        if(pid == 0){

            player.money = p->numoftokens;
            player.pid = getpid();
            player.bet_symbol = p->bet_symbol;
            player.goal = p->goal;
            player.stop = p->stop;
            player.bet = p->bet;
            player.current_bet = p->bet;
            player.pipe_fd_read = pipe_fd[i][1][0];
            player.pipe_fd_write = pipe_fd[i][0][1];
            player.hand = NULL;
            player.hand_value = 0;
            
            for(int j = 0; j<gamefile.numofplayers; j++){
                if(j == i){
                    close(pipe_fd[j][1][1]);
                    close(pipe_fd[j][0][0]);
                } else{
                    close(pipe_fd[j][0][0]);
                    close(pipe_fd[j][1][0]);
                    close(pipe_fd[j][0][1]);
                    close(pipe_fd[j][1][1]);                    
                }
            } break;
        } else p = p->next;
    }
    
    if(pid != 0){

        initDeckLib();
        deck_t* deck = initDeck(P52, gamefile.numofdecks);
        shuffleDeck(deck);

        int end_round = 0;        
        int hand_value = 0;
        int sig;
        int tmp;
        int n_round;
        
        char* ptr;
        
        char hand[10];
        int waiting[gamefile.numofplayers];
        int exited[gamefile.numofplayers];
        filevars_t filevars[gamefile.numofplayers];

        
        memset(hand,0,10);
        memset(waiting,0,gamefile.numofplayers*sizeof(int));
        memset(exited,0,gamefile.numofplayers*sizeof(int));
        memset(filevars,0,gamefile.numofplayers*sizeof(filevars_t));
        

        for(i = 0; i<gamefile.numofplayers; i++) filevars[i].path = itoa(l_pid[i]);
       
        n_round = 0;
        
        while(n_round < gamefile.numofhands){
            
            for(j = 0; j<2; j++){

                tmp = drawCard(deck);
                hand[j] = getCardChar(tmp);
                hand_value += getCardValue(tmp, hand_value);

                for(i = 0; i<gamefile.numofplayers; i++){

                    sig = SEND_CARD;
                    raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) != sizeof(int), "WRITE1");

                    sig = drawCard(deck);

                    filevars[i].phand_value += getCardValue(sig, hand_value);
                    filevars[i].player_hand[j] = getCardChar(sig);
                    raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) != sizeof(int), "WRITE2");      
                }
            }

            sig = PLAY;
            for(i = 0; i<gamefile.numofplayers; i++) raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) != sizeof(int), "WRITE3");
            i = 0;
            
            while(end_round < gamefile.numofplayers){
                if(!(waiting[i] || exited[i])){
                    raiseError(read(pipe_fd[i][0][0], &sig, sizeof(int)) == -1, "READ1");         

                    switch(sig){

                        case END_ROUND :
                            end_round++;
                            waiting[i] = 1;
                            break;

                        case REQUEST_CARD :
                            sig = drawCard(deck);
                            for(j = 0; filevars[i].player_hand[j]; j++);                            
                            filevars[i].player_hand[j] = getCardChar(sig);
                            filevars[i].phand_value += getCardValue(sig, filevars[i].phand_value);
                            raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE5");
                            break;

                        case END_GAME :
                            exited[i] = 1;
                            break;
                    }
                }

                i = (i+1)%gamefile.numofplayers;

            }
            
            for(j = 0; hand[j]; j++);

            while(hand_value < 17){
                tmp = drawCard(deck);
                hand_value += getCardValue(tmp, hand_value);
                hand[j] = getCardChar(tmp);
                j++;
            }
            
            for(i = 0; i < gamefile.numofplayers; i++){
                
                if(!exited[i]){
                    
                    sig = SEND_MONEY;
                    raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE9");
                    raiseError(read(pipe_fd[i][0][0], &tmp, sizeof(int)) == -1, "READ23");
                    filevars[i].bet = tmp;
                    
                    if(filevars[i].phand_value == 21 && strlen(filevars[i].player_hand) == 2){

                        sig = WIN; tmp*= 2;
                        raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE6");
                        raiseError(write(pipe_fd[i][1][1], &tmp, sizeof(int)) == -1, "WRITE11");
                        filevars[i].gain = tmp;
                        raiseError(read(pipe_fd[i][0][0], &tmp, sizeof(int)) == -1, "READ42");                        
                        filevars[i].ntoken = tmp;

                        
                    } else if((filevars[i].phand_value > hand_value && filevars[i].phand_value <= 21) || (hand_value > 21 && filevars[i].phand_value <= 21)){

                        sig = WIN; tmp*= 2;
                        raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE6");
                        raiseError(write(pipe_fd[i][1][1], &tmp, sizeof(int)) == -1, "WRITE11");
                        filevars[i].gain = tmp;
                        raiseError(read(pipe_fd[i][0][0], &tmp, sizeof(int)) == -1, "READ42");                        
                        filevars[i].ntoken = tmp;

                    } else if(filevars[i].phand_value == hand_value){ 

                        sig = WIN; 
                        raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE6");                
                        raiseError(write(pipe_fd[i][1][1], &tmp, sizeof(int)) == -1, "WRITE12");  
                        filevars[i].gain = tmp;
                        raiseError(read(pipe_fd[i][0][0], &tmp, sizeof(int)) == -1, "READ42");                        
                        filevars[i].ntoken = tmp;

                    } else{

                        sig = LOSE;
                        raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE12");    
                        filevars[i].gain = 0;
                        raiseError(read(pipe_fd[i][0][0], &tmp, sizeof(int)) == -1, "READ88");
                        filevars[i].ntoken = tmp;
                    }
                }
            }

            
            for(i = 0; i<gamefile.numofplayers; i++){
                
                raiseError((fd = open(filevars[i].path, O_CREAT|O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR)) == -1, "OPEN2");

                write(fd, filevars[i].player_hand, strlen(filevars[i].player_hand));
                write(fd, ";", 1);
                
                ptr = itoa(filevars[i].phand_value);
                write(fd, ptr, strlen(ptr));
                free(ptr);
                write(fd, ";", 1);
                
                hand;
                write(fd, hand, strlen(hand));
                write(fd, ";", 1);

                ptr = itoa(hand_value);                
                write(fd, ptr, strlen(ptr));
                free(ptr);
                write(fd, ";", 1);

                ptr = itoa(filevars[i].bet);
                write(fd, ptr, strlen(ptr));
                free(ptr);
                write(fd, ";", 1);

                ptr = itoa(filevars[i].gain);
                write(fd, ptr, strlen(ptr));
                free(ptr);
                write(fd, ";", 1);
                
                ptr = itoa(filevars[i].ntoken);
                write(fd, ptr, strlen(ptr));
                free(ptr);
                write(fd, "\n", 1);
                
                memset(filevars[i].player_hand, 0, strlen(filevars[i].player_hand));
                memset(hand, 0, strlen(hand));
                filevars[i].bet = 0;
                filevars[i].gain = 0;
                filevars[i].phand_value = 0;
                close(fd);
            }
                        
            end_round = 0;
            hand_value = 0;
            n_round++;
            memset(waiting, 0, gamefile.numofplayers*sizeof(int));
        }

        for(i = 0; i<gamefile.numofplayers; i++){
            sig = END_GAME;
            raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE12");    
            close(pipe_fd[i][0][0]);
            close(pipe_fd[i][1][0]);
            close(pipe_fd[i][0][1]);
            close(pipe_fd[i][1][1]);
            free(filevars[i].path);
            wait(NULL);                                
        }
                        
        free_playervars(gamefile.next);
        exit(EXIT_SUCCESS);
    } 
//###########################################################################################################################################################

    else{
        int n;

        while(raiseError(read(player.pipe_fd_read, &n, sizeof(int)) == -1, "READ2")){            

            switch(n){
                
                case SEND_CARD :

                    raiseError(read(player.pipe_fd_read, &n, sizeof(int)) == -1, "READ3");
                    player.hand_value += getCardValue(n, player.hand_value);
                    break;

                case PLAY :
                    
                    if(player.money < player.current_bet || player.money >= player.goal){
                        n = END_GAME;
                        raiseError(write(player.pipe_fd_write, &n, sizeof(int)) == -1, "WRITE8");
                        goto end;
                    }
                    
                    while(player.hand_value < player.stop){
                        n = REQUEST_CARD;
                        raiseError(write(player.pipe_fd_write, &n, sizeof(int)) == -1, "WRITE6");
                        raiseError(read(player.pipe_fd_read, &n, sizeof(int)) == -1, "READ4");
                        player.hand_value += n;
                    }
                    n = END_ROUND;
                    raiseError(write(player.pipe_fd_write, &n, sizeof(int)) == -1, "WRITE7");                    
                    break;
                    
                case END_GAME :
end :
                    printf("[%i] Ending...\n", getpid());                    
                    free_playervars(gamefile.next);
                    exit(EXIT_SUCCESS);
                    
                case SEND_MONEY :
                    n = player.current_bet;
                    player.money -= n;
                    raiseError(write(player.pipe_fd_write, &n, sizeof(int)) == -1, "READ5");
                    break;
                
                case WIN :
                    
                    raiseError(read(player.pipe_fd_read, &n, sizeof(int)) == -1, "READ5");
                    player.money+=n;
                    player.current_bet = player.bet;
                    n = player.money;
                    raiseError(write(player.pipe_fd_write, &n, sizeof(int)) == -1, "WRITE42");
                    break;
                
                case LOSE :    
                    
                    if(player.bet_symbol == '+'){
                        player.current_bet *= 2;
                    } else if(player.bet_symbol == '-'){
                        player.current_bet = player.current_bet/2 + 1;
                    }
                    n = player.money;
                    raiseError(write(player.pipe_fd_write, &n, sizeof(int)) == -1, "WRITE88");
                    break;                    
            }
        }
    }
}

