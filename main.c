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
#include "player.h"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>

#define END 666

typedef struct gamevars_t{
    int numofplayers;
    int numofdecks;
    int numofhands;
    struct playervars_t* next;
} gamevars_t;

//Lis le PDF du projet pour comprendre à quel champ correspond quel variable
//pour les deux structures

typedef struct playervars_t{
    int numoftokens;
    int bet;
    int stop;
    int goal;
    char bet_symbol;
    struct playervars_t* next;
} playervars_t;

void free_playervars(playervars_t* p){
    playervars_t* tmp;
    while(p != 0){
        tmp = p->next;
        free(p);
        p = tmp;
    }
}
void BUFFEROVERFLOOOOOOOW(){
    printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHHHHHHHHHHH BUFFER OVERFLOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOWWWWWWW :'(\n");
    exit(666);
}

//Fonction à rendre plus élégante
int load_file(gamevars_t* gfile, char* buf, int size){
    char str[22];
    memset(str, 0, 22);
    int comut = 1;
    int str_i = 0, gfile_i = 0, buf_i = 0, pfile_i = 0;
    
    while(comut){ //CHARGE LA PREMIERE LIGNE (variables associées au jeu)
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
    
    playervars_t* pfile = calloc(1, sizeof(playervars_t));  
    gfile->next = pfile;
    comut = 1;
    
    while(comut){//CHARGE LES LIGNES SUIVANTES (variables associées aux joueurs)
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
    return 0;
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
    if((fd = open(argv[1], O_RDONLY)) == -1){
        perror("OPEN");
        return errno;
    }

    struct stat st;
    if(fstat(fd, &st) == -1){
        perror("STAT");
        close(fd);
        return errno;
    }
    
    char buf[st.st_size];
    if(read(fd, &buf, st.st_size) == -1){
        perror("READ");
        close(fd);
        return errno;
    } close(fd);

    gamevars_t gamefile;
    if(load_file(&gamefile, buf, st.st_size) == -1){
        printf("LOAD_FILE : An error occured when I was loading this file :'(\n");
        close(fd);
        return 1;
    }
            
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
    int i;
    int pid;

    for(i = 0; i < gamefile.numofplayers; i++){
        if(pipe(pipe_fd[i][0]) + pipe(pipe_fd[i][1]) != 0){
            perror("PIPE :");
            return errno;
        }
    }

    
    for(i = 0; i<gamefile.numofplayers; i++){
        
        if((pid = fork()) == -1){
            perror("FORK");
            return errno;
        } l_pid[i] = pid;

        if(pid == 0){
            player.money = p->numoftokens;
            player.pid = getpid();
            player.bet_symbol = p->bet_symbol;
            player.goal = p->goal;
            player.stop = p->stop;
            player.bet = p->bet;
            player.pipe_fd_read = pipe_fd[i][1][0];
            player.pipe_fd_write = pipe_fd[i][0][1];
            
            //On ferme les fichiers dont le process ne se servira pas;

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

        for(i = 0; i<gamefile.numofplayers; i++){
 
            int ret = 0;
            int end = END;
            int card1 = drawCard(deck);
            int card2 = drawCard(deck);
            
            ret += write(pipe_fd[i][1][1], &card1, sizeof (int)); 
            ret += write(pipe_fd[i][1][1], &card2, sizeof (int));
            ret += write(pipe_fd[i][1][1], &end, sizeof (int));
            
            if(ret != 3*sizeof(int)){
                perror("WRITE");
                return errno;
            }
        }
        
        for(i = 0; i<gamefile.numofplayers; i++){
            close(pipe_fd[i][0][0]);
            close(pipe_fd[i][1][0]);
            close(pipe_fd[i][0][1]);
            close(pipe_fd[i][1][1]);
        }
                
        int stat;
        for(i = 0; i<gamefile.numofplayers; i++) wait(&stat);
        free(gamefile.next);
        
    } else{
        sleep(1);
        int n = 0;
        while(n != END){
            if(read(player.pipe_fd_read, &n, sizeof(int)) == -1){    
                perror("READ");
                return errno;
            }else{
                printf("[%i] Card : %i \n", getpid(), n);
            }
        }
        
    }
    
    return EXIT_SUCCESS;
}

