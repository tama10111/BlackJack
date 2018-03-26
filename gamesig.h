/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   gamesig.h
 * Author: tama
 *
 * Created on 24 mars 2018, 14:26
 */

#define END_GAME        0xfff
#define SEND_MONEY      0xff0 // L'envoyeur doit recevoir la mise
#define SEND_CARD       0xff1 // L'envoyeur doit écrire la valeur de la carte
#define REQUEST_CARD    0xff2 // L'envoyeur doit se mettre en lecture pour recevoir la valeur de la carte
#define PLAY            0xff3
#define END_ROUND       0xff4
#define WIN             0xff5 // L'envoyeur doit écrire l'argent gagné
#define LOSE            0xff6