#include "deck.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NBVALUESPERCOLOR 13

void initDeckLib () {
	srand (time (NULL));
}

deck_t * initDeck (
	const decktype_t type,
	const int nbPacks) {
	deck_t * deck;
	card_t * tmp;
	int i, j;

	deck = malloc (sizeof (deck_t) );
	deck->type = type;

	deck->drawPile = NULL;
	deck->discardPile = NULL;
	deck->handCards = NULL;

	for (j = 0; j < nbPacks; ++j) {
		switch (type) {
			case P32:
				for (i = 51; i >= 0; --i) {
					if (i % 13 > 0 && i % 13 < 6) continue;
					tmp = malloc (sizeof (card_t) );
					tmp->next = deck->drawPile;
					tmp->value = i;
					deck->drawPile = tmp;
				}
				break;
			case P52:
				for (i = 51; i >= 0; --i) {
					tmp = malloc (sizeof (card_t) );
					tmp->next = deck->drawPile;
					tmp->value = i;
					deck->drawPile = tmp;
				}
				break;
			case P54:
				for (i = 0; i < 2; ++i) {
					tmp = malloc (sizeof (card_t) );
					tmp->next = deck->drawPile;
					tmp->value = 52;
					deck->drawPile = tmp;
				}
				for (i = 51; i >= 0; --i) {
					tmp = malloc (sizeof (card_t) );
					tmp->next = deck->drawPile;
					tmp->value = i;
					deck->drawPile = tmp;
				}
				break;
		}
	}

	return deck;
}

void removeDeck (
	deck_t * deck) {
	card_t * tmp;

	while (deck->drawPile != NULL) {
		tmp = deck->drawPile;
		deck->drawPile = tmp->next;
		free (tmp);
	}
	while (deck->discardPile != NULL) {
		tmp = deck->discardPile;
		deck->discardPile = tmp->next;
		free (tmp);
	}
	while (deck->handCards != NULL) {
		tmp = deck->handCards;
		deck->handCards = tmp->next;
		free (tmp);
	}
}

void shuffleDeck (
	deck_t * deck) {
	card_t * tmp = deck->drawPile, * ptr, * pile;
	int cnt = 0, i;

	if (tmp == NULL) {
		tmp = deck->drawPile = deck->discardPile;
	} else {
		for (; tmp->next != NULL; tmp = tmp->next, ++cnt);
		tmp->next = deck->discardPile;
	}

	deck->discardPile = NULL;
	for (; tmp != NULL; tmp = tmp->next, ++cnt);

	pile = deck->drawPile;
	deck->drawPile = NULL;

	for (; cnt; --cnt) {
		tmp = pile;
		ptr = NULL;
		i = rand () % cnt;

		for (; i; ptr = tmp, tmp = tmp->next, --i);

		if (ptr == NULL)
			pile = tmp->next;
		else
			ptr->next = tmp->next;
		
		tmp->next = deck->drawPile;
		deck->drawPile = tmp;
	}
}

int drawCard (deck_t * deck) {
	card_t * tmp = deck->drawPile;

	if (tmp == NULL) return -1;

	deck->drawPile = tmp->next;
	tmp->next = deck->handCards;
	deck->handCards = tmp;

	return tmp->value;
}

void discardCard (deck_t * deck, const int value) {
	card_t * tmp = deck->handCards, * ptr = NULL;

	for (; tmp != NULL && tmp->value != value; ptr = tmp, tmp = tmp->next);

	if (tmp == NULL) return;

	if (ptr == NULL)
		deck->handCards = tmp->next;
	else
		ptr->next = tmp->next;

	tmp->next = deck->discardPile;
	deck->discardPile = tmp;
}

cardvalue_t getValueFromCardID (const int cardid) {
	if (cardid == 52) return VJOK;
	return 1 + (cardid % NBVALUESPERCOLOR);
}

cardcolor_t getColorFromCardID (const int cardid) {
	if (cardid == 52) return CJOKR;
	return 1 + (cardid / NBVALUESPERCOLOR);
}

int getDrawPileSize (
	deck_t * deck) {
	int nb = 0;
	card_t * tmp = deck->drawPile;

	while (tmp != NULL) {
		tmp = tmp->next;
		++nb;
	}

	return nb;
}

int getDiscardPileSize (
	deck_t * deck) {
	int nb = 0;
	card_t * tmp = deck->discardPile;

	while (tmp != NULL) {
		tmp = tmp->next;
		++nb;
	}

	return nb;
}

decktype_t getDeckType (
	deck_t * deck) {
	return deck->type;
}

void printCard (
	const int value) {
	if (value == 52) { printf ("J*"); return; }
	switch (value % 13) {
		case 0: printf ("A"); break;
		case 9: printf ("X"); break;
		case 10: printf ("J"); break;
		case 11: printf ("Q"); break;
		case 12: printf ("K"); break;
		default: printf ("%d", (value % 13) + 1);
	}

	switch (value / 13) {
		case 0: printf ("c"); break;
		case 1: printf ("d"); break;
		case 2: printf ("s"); break;
		case 3: printf ("h");
	}
}

void printDrawPile (
	deck_t * deck) {
	card_t * tmp = deck->drawPile;
	printCard (tmp->value);
	tmp = tmp->next;
	while (tmp != NULL) {
		printf (" ");
		printCard (tmp->value);
		tmp = tmp->next;
	}
	printf ("\n");
}
