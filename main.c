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
    char bank_hand[10];
    int bhand_value;
    int bet;
    int gain;
    int ntoken;    
    char* path;
} filevars_t;

int ppow(int x,int n){
    int res=1;
    for (int i=0; i<n; i++){
        res*=x;
    }
    return res;
}


char * itoa(int i){ // DONE ça a l'air bien
    char a[21];
    memset(a,0,21);
    int k=0;
    for (int j=0; j<20; j++){
        a[j]=i/(ppow(10,20-j));
        i-=i/(ppow(10,20-j));
        if (a[j] != 0){
            a[k]=a[j]+'0';
            k+=1;
        }
    }
    return a;
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

card_t* addCard(struct card_t* hand, int value){
    
    if(value == -1){
        printf("Deck is empty\n");
        exit(1);
    }
    
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
    if(getValueFromCardID(card) == 11 ){
        if( hand <= 10){
        return 11;
        } 
        else return 1;}
    else return getValueFromCardID(card);  
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
    
    
    int liste_file[gamefile.numofplayers];
    filevars_t filevars[gamefile.numofplayers];
    
    pid_t l_pid[gamefile.numofplayers];
    
    /*
     * pipe_fd[x][0][0] PARENT READ
     * pipe_fd[x][0][1] CHILD WRITE
     * pipe_fd[x][1][0] CHILD READ
     * pipe_fd[x][1][1] PARENT WRITE
     */
    
    int pipe_fd[gamefile.numofplayers][2][2];
    int waiting[gamefile.numofplayers];
    int exited[gamefile.numofplayers];
    int players_hand[gamefile.numofplayers];

    int i;
    int pid;
    
    for(i = 0; i < gamefile.numofplayers; i++) raiseError(pipe(pipe_fd[i][0]) + pipe(pipe_fd[i][1]) != 0, "PIPE");

    for(i = 0; i<gamefile.numofplayers; i++){

        raiseError((pid = fork()) == -1, "FORK");

        l_pid[i] = pid;
        waiting[i] = 0;
        players_hand[i] = 0;
        exited[i] = 0;

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
        int sig;
        int tmp;
        int n_round;
        
        
        for(int j = 0; j<2; j++){

            bank_hand = addCard(bank_hand, drawCard(deck));
            hand_value += getCardValue(bank_hand->value, hand_value);
            for(i=0; i<gamefile.numofplayers; i++){
                filevars[i].bank_hand[j]=itoa(bank_hand->value);
                filevars[i].bhand_value=hand_value;
            }

            for(i = 0; i<gamefile.numofplayers; i++){
                sig = SEND_CARD;
                raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) != sizeof(int), "WRITE1");
                sig = drawCard(deck);
                players_hand[i]+=getCardValue(sig, hand_value);
                filevars[i].player_hand[j]=itoa(sig);
                filevars[i].phand_value=players_hand[i];
                raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) != sizeof(int), "WRITE2");      
                filevars[i].path=malloc(2*sizeof(char*));
                filevars[i].path=strcat("fichierjoueur",itoa(i)); //ça passe là non ?
                /*
                 * Mauvaise technique. Selon le man, le premier pointeur, doit avoir une taille suffisante pour contenir la concaténation des deux chaines de caractère.
                 * Pourquoi ? Parce srtcat fonctionne comme ça :
                 * 
                 * Mes deux chaînes
                 * 
                 * 0 1 2 3 4 5 6 7 8    0 1 2 3 4 5 6 7 8 9
                 * M a c h a i n e 0    M o n   a j o u t 0
                 * 
                 * Après strcat("Machaine", "Mon ajout")
                 * 
                 * 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17
                 * M a c h a i n e M o n     a  j  o  u  t  0
                 * 
                 * Le problème est que à l'adresse du pointeur vers "Machaine", seulement 8 octets de mémoire sont aloués pour ce pointeur
                 * Le comportement est alors imprédictible, tu peux écrire par dessus le contenu d'un autre pointeur par exemple, faille qui peut être exploitée
                 * Je te laisse corriger ça ;)
                 */                                
            }
        }
        
        n_round = 0;
        
        while(n_round < gamefile.numofhands){

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
                            int j=0; // Ici tu initialise j à chaque tour de boucle, tu ne peux pas initialiser deux fois la même variable
                            //Je capte pas en quoi ça marche pas
                            while (filevars[i].player_hand[j] != NULL){ 
                                j=j+1;
                            }
                            filevars[i].player_hand[j]=itoa(sig);//Ici tu mets l'ID alors que l'on veut le caractère associé à l'ID
                            raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE5");
                            break;

                        case END_GAME :
                            exited[i] = 1;
                            break;
                    }
                }

                i = (i+1)%gamefile.numofplayers;

            }

            while(hand_value < 17){
                bank_hand = addCard(bank_hand, drawCard(deck));
                int i,j=0; //Tu réinitialise i qui est déjà initialisé au début du programme. Et tu réinitialise j
                //Pareil
                while (filevars[i].bank_hand[j]){
                    j=j+1;
                }  
                hand_value += getCardValue(bank_hand->value, hand_value);
                for (i=0; i < gamefile.numofplayers; i++){
                    filevars[i].bank_hand[j]=itoa(bank_hand->value); // Idem, on veut le caractère associé à l'id
                    filevars[i].bhand_value=hand_value;
                }
            }
            
            for(i = 0; i < gamefile.numofplayers; i++){
                if(!exited[i]){
                    sig = SEND_MONEY;
                    raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE9");
                    raiseError(read(pipe_fd[i][0][0], &tmp, sizeof(int)) == -1, "READ23");
                    filevars[i].bet=tmp;

                    if((players_hand[i] > hand_value && players_hand[i] <= 21) || (hand_value > 21 && players_hand[i] <= 21)){

                        sig = WIN; tmp*= 2;
                        raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE6");
                        raiseError(write(pipe_fd[i][1][1], &tmp, sizeof(int)) == -1, "WRITE11");
                        filevars[i].gain=tmp;

                    } else if(players_hand[i] == hand_value){ 

                        sig = WIN; 
                        raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE6");                
                        raiseError(write(pipe_fd[i][1][1], &tmp, sizeof(int)) == -1, "WRITE12");  
                        filevars[i].gain=tmp;
                        raiseError(read(pipe_fd[i][0][0], &tmp, sizeof(int)) == -1, "READ42"); //Ducoup j'ai rajouté des pipes pour avoir le nombre de jetons
                        
                        /*
                         * J'vais faire mon chieur mais ce sont pas des pipes que tu rajoutes, t'écris/lis juste :P
                         * Ça fonctionne bien comme ça, mais je me demande si créer un signal spécial pour request le nombre de jeton ne serait pas plus propre
                         * à lire pour le programmeur.
                         */
                        
                        filevars[i].ntoken=tmp;

                    } else{

                        sig = LOSE;
                        raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE12");    
                        filevars[i].gain=0;
                        raiseError(read(pipe_fd[i][0][0], &tmp, sizeof(int)) == -1, "READ88"); // SS o/
                        filevars[i].ntoken=tmp;
                    }
                }
            } 
            
            // T'écris pas dans la fichier ?
            
            end_round = 0;
            hand_value = 0;
            n_round++;
            memset(waiting, 0, gamefile.numofplayers*sizeof(int));
            
        }

        for(i = 0; i<gamefile.numofplayers; i++){
            sig = END_GAME;
            raiseError(write(pipe_fd[i][1][1], &sig, sizeof(int)) == -1, "WRITE12");    
            wait(NULL);                                
        }

        for(i = 0; i<gamefile.numofplayers; i++){
            close(pipe_fd[i][0][0]);
            close(pipe_fd[i][1][0]);
            close(pipe_fd[i][0][1]);
            close(pipe_fd[i][1][1]);
        }
        
        free_playervars(gamefile.next);
        freeHand(bank_hand);
        exit(EXIT_SUCCESS);
    } 
//#############################################################################//
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
                    raiseError(write(player.pipe_fd_write, &n, sizeof(int)) == -1, "WRITE42"); // n = player.money non ?
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

