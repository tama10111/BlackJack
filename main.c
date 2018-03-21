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

#define CARD_DIST_REQ 98
#define CARD_DIST_RESP 89

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

typedef struct gamefile_t{
    int nj;
    int nd;
    int nm;
    struct playerfile_t* next;
} gamefile_t;

typedef struct playerfile_t{
    int njet;
    int nmise;
    int nstop;
    int objeton;
    char smise;
    struct playerfile_t* next;
} playerfile_t;

void* CARD_SENDER(int i, siginfo_t* sinfo, void* ucontext){
    
}

void* CARD_RECEIVER(int i, siginfo_t* sifon, void* ucontext){
    
}


void free_playerfile(playerfile_t* p){
    playerfile_t* tmp;
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

int load_file(gamefile_t* gfile, char* buf, int size){
    char str[22];
    memset(str, 0, 22);
    int comut = 1;
    int str_i = 0, gfile_i = 0, buf_i = 0, pfile_i = 0;
    
    while(comut){ //CHARGE LA PREMIERE LIGNE (structure différente des suivantes)
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
    
    playerfile_t* pfile = calloc(1, sizeof(playerfile_t));  
    gfile->next = pfile;
    comut = 1;
    
    while(comut){//CHARGE LES LIGNES SUIVANTES
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
                    pfile->next = calloc(1, sizeof(playerfile_t));
                    pfile = pfile->next;                                        
                } else{
                    comut = 0;
                }
                break;
                                                
            case '+' :
                pfile->smise = '+'; buf_i++;                
                break;

            case '-':
                pfile->smise = '-'; buf_i++;                
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
        perror("OPEN :");
        return errno;
    }

    struct stat st;
    if(fstat(fd, &st) == -1){
        perror("STAT :");
        close(fd);
        return errno;
    }
    
    char buf[st.st_size];
    if(read(fd, &buf, st.st_size) == -1){
        perror("READ :");
        close(fd);
        return errno;
    } close(fd);

    gamefile_t gamefile;
    if(load_file(&gamefile, buf, st.st_size) == -1){
        printf("LOAD_FILE : An error occured when I was loading this file :'(\n");
        close(fd);
        return 1;
    }
        
    playerfile_t* p = gamefile.next;
    player_t player;

    pid_t pid;
        
    while(p != NULL){
        pid = fork();
        if(!pid){
            player.money = p->njet;
            player.pid = pid;
            player.tech = p->smise;
            player.obj = p->objeton;
            player.stop_val = p->nstop;
            player.mise = p->nmise;
            break;
        } else{
            p = p->next;
        }
    }
    
    if(pid != 0){
        initDeckLib();
        deck_t* deck = initDeck(P52, gamefile.nd);

        struct sigaction saction;
        saction.sa_flags = SA_SIGINFO;
        saction.sa_sigaction = CARD_RECEIVER;

        sigaction(CARD_DIST_REQ, &saction, NULL);
        sigaction(CARD_)
        

        free(gamefile.next);        
    } else{
        
    }
    
    return EXIT_SUCCESS;
}

