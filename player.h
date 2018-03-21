/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   player.h
 * Author: tama
 *
 * Created on 19 mars 2018, 14:28
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "deck.h"

typedef struct player_t{
    int pid;
    int pipe_fd_read;
    int pipe_fd_write;
    struct card* hand;
    int money;
    char bet_symbol;
    int goal;
    int stop;
    int bet;
} player_t;



#endif /* PLAYER_H */

