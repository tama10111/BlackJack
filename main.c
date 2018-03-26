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

int raiseError(int condition, char* error){
    if(condition && errno != 11){ // C'EST DÉGUEULASSE :'(
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

card_t* addCard(struct card_t* hand, int value){
    if(hand == NULL){
        if((hand = malloc(sizeof(card_t))) == NULL){
            printf("MALLOC : NULL returned\n");
            exit(1);
        }
        hand->next = NULL;
        hand->prev = NULL;
        hand->value = value;
        return hand;
    } else{
        if((hand->next = malloc(sizeof(card_t))) == NULL){
            printf("MALLOC : NULL returned\n");
            exit(1);
        }
        hand->next->value = value;
        hand->next->prev = hand;
        hand->next->next = NULL;
        return hand->next;
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
                break;
        }
    }
}

int ppow(int n){
    int ret = 1;
    while(n){
        ret*=10; n--;
    } return ret;
}

int main(int argc, char** argv) {    
        
    if(argc != 2){
        printf("SYNTAX : ./lv21 [FILE]\n");
        return EXIT_FAILURE;
    }
    
    int fd;
    raiseError((fd = open(argv[1], O_RDONLY)) == -1, "OPEN");

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
    int waiting[gamefile.numofplayers];
    int players_hand[gamefile.numofplayers];

    int i;
    int pid;
    
    for(i = 0; i < gamefile.numofplayers; i++) raiseError(pipe(pipe_fd[i][0]) + pipe(pipe_fd[i][1]) != 0, "PIPE");

    for(i = 0; i<gamefile.numofplayers; i++){

        raiseError((pid = fork()) == -1, "FORK");
        l_pid[i] = pid;

        waiting[i] = 0;
        players_hand[i] = 0;
        
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
        struct card_t* bank_hand = NULL;
        int hand_value = 0;
        int w;
        
        
        for(int j = 0; j<2; j++){

            bank_hand = addCard(bank_hand, drawCard(deck));
            hand_value += bank_hand->value;

            for(i = 0; i<gamefile.numofplayers; i++){
                w = SEND_CARD;
                raiseError(write(pipe_fd[i][1][1], &w, sizeof(int)) != sizeof(int), "WRITE1");

                w = drawCard(deck);
                players_hand[i]+=w;
                raiseError(write(pipe_fd[i][1][1], &w, sizeof(int)) != sizeof(int), "WRITE2");                
            }
        }

        w = PLAY;
        for(i = 0; i<gamefile.numofplayers; i++) raiseError(write(pipe_fd[i][1][1], &w, sizeof(int)) != sizeof(int), "WRITE3");
        
        i = 0;

        while(end_round < gamefile.numofplayers){
            if(!waiting[i]){
                raiseError(read(pipe_fd[i][0][0], &w, sizeof(int)) == -1, "READ1");         

                switch(w){

                    case END_ROUND :
                        end_round++;
                        waiting[i] = 1;
                        printf("[%i] SEND END\n", end_round); 
                        w = END_GAME;
                        raiseError(write(pipe_fd[i][1][1], &w, sizeof(int)) == -1, "WRITE4");  
                        break;
                        
                    case REQUEST_CARD :
                        w = drawCard(deck);
                        raiseError(write(pipe_fd[i][1][1], &w, sizeof(int)) == -1, "WRITE5");
                        break;
                }
            }

            i = (i+1)%gamefile.numofplayers;

        }
        
        for(i = 0; i< gamefile.numofplayers; i++){
            if(players_hand[i] > bank_hand && players_hand[i] <= 21){
                w = WIN;
                raiseError(write(pipe_fd[i][1][1], &w, sizeof(int)) == -1, "WRITE6");
            }
        }
            
        for(i = 0; i<gamefile.numofplayers; i++) wait(NULL);                    
        for(i = 0; i<gamefile.numofplayers; i++){
            close(pipe_fd[i][0][0]);
            close(pipe_fd[i][1][0]);
            close(pipe_fd[i][0][1]);
            close(pipe_fd[i][1][1]);
        }
        free_playervars(gamefile.next);
        exit(EXIT_SUCCESS);         
    } 
    
    else{
        int n;
        while(raiseError(read(player.pipe_fd_read, &n, sizeof(int)) == -1, "READ2")){            

            switch(n){
                
                case SEND_CARD :
                    raiseError(read(player.pipe_fd_read, &n, sizeof(int)) == -1, "READ3");
                    player.hand = addCard(player.hand, n);
                    player.hand_value += n;
                    break;

                case PLAY :
                    while(player.hand_value < player.stop){
                        n = REQUEST_CARD;
                        raiseError(write(player.pipe_fd_write, &n, sizeof(int)) == -1, "WRITE6");
                        raiseError(read(player.pipe_fd_read, &n, sizeof(int)) == -1, "READ4");
                        player.hand = addCard(player.hand, n);
                        player.hand_value += n;
                    }
                    printf("[%i] SEND END\n", getpid());
                    n = END_ROUND;
                    raiseError(write(player.pipe_fd_write, &n, sizeof(int)) == -1, "WRITE7");                    
                    break;
                    
                case END_GAME :
                    free_playervars(gamefile.next);
                    freeHand(player.hand);
                    exit(EXIT_SUCCESS);
                    
                case SEND_MONEY :
                    n = player.current_bet;
                    raiseError(write(player.pipe_fd_write, &n, sizeof(int)) == -1, "READ5");
                    break;
                
                case WIN :
                    raiseError(read(player.pipe_fd_read, &n, sizeof(int)) == -1, "READ5");
                    player.money+=n;
                    player.current_bet = player.bet;
                    break;
                
                case LOSE :
                    if(player.bet_symbol == '+'){
                        player.current_bet *= 2;
                    } else if(player.bet_symbol == '-'){
                        player.current_bet = player.current_bet/2 + 1;
                    } break;                    
            }
        }
    }
}

