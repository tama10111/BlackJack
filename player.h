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
    struct card* hand;
    int money;
    char tech;
    int obj;
    int stop_val;
    int mise;
} player_t;



#endif /* PLAYER_H */

