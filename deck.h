#ifndef _H_DECK_
#define _H_DECK_

// Liste chaînée représentant une pile/suite de cartes
typedef struct card {
	struct card * next;		//< Carte suivante
	int value;				//< Valeur de la carte (entre 0 et 52)
} card_t;

// Type de deck
typedef enum {
	P32,					//< Paquet de 32 cartes (du 7 à l'As)
	P52,					//< Paquet de 52 cartes (de l'As au Roi)
	P54					//< Paquet de 54 cartes (de l'As au Roi avec 2 Jokers)
} decktype_t;

// Valeur de carte
typedef enum {
	VJOK,                                           //< Joker
	VA,						//< As
	V2,						//< Deux
	V3,						//< Trois
	V4,						//< Quatre
	V5,						//< Cinq
	V6,						//< Six
	V7,						//< Sept
	V8,						//< Huit
	V9,						//< Neuf
	VX,						//< Dix
	VJ,						//< Valet
	VQ,						//< Dame
	VK						//< Roi
} cardvalue_t;

// Couleur de carte
typedef enum {
	CJOKR,					//< Joker (sans couleur)
	CCLUB,					//< Trèfle
	CDIAM,					//< Carreau
	CSPAD,					//< Pique
	CHEAR					//< Coeur
} cardcolor_t;

// Représentation d'un deck
typedef struct {
	card_t * drawPile;		//< Pioche
	card_t * discardPile;           //< Défausse
	card_t * handCards;		//< Cartes en cours d'utilisation (dans les mains)
	decktype_t type;		//< Type de deck
} deck_t;

// Initialisation de la bibliothèque
// Génération de la graine aléatoire à partir de time()
void initDeckLib ();

// Initialisation d'un deck
// Création de la structure de deck et remplissage avec un nombre de paquets
// d'un type défini, ordonné
// \param	type			Type de paquet
// \param	nbPacks			Nombre de paquets dans le deck
// \return					Deck généré
deck_t * initDeck (const decktype_t type, const int nbPacks);


// Suppression d'un deck
// Désallocation de la mémoire utilisée par la structure de deck
// \param	deck			Deck à désallouer
void removeDeck (deck_t * deck);


// Mélange d'un deck
// \param	deck			Deck à mélanger
void shuffleDeck (deck_t * deck);


// Pioche d'une carte dans un deck donné
// La carte est placée dans la pile des cartes en cours d'utilisation
// \param	deck			Deck où la pioche est effectuée
// \return					Identifiant de la carte piochée, -1 si la pioche est vide
int drawCard (deck_t * deck);


// Mise d'une carte dans la défausse
// La carte est prise de la pile des cartes en cours d'utilisation, si possible
// \param	deck			Deck où la défausse est effectuée
// \param	cardid			Identifiant de la carte défaussée
void discardCard (deck_t * deck, const int cardid);


// Extraction de la valeur d'une carte à partir de son identifiant
// La valeur est extraite à l'aide de l'opération modulo
// \param	cardid			Identifiant de la carte
// \return					Valeur de la carte
cardvalue_t getValueFromCardID (const int cardid);


// Extraction de la couleur d'une carte à partir de son identifiant
// La couleur est extraite à l'aide de l'opération division
// \param	cardid			Identifiant de la carte
// \return					Couleur de la carte
cardcolor_t getColorFromCardID (const int cardid);


// Obtention de la taille de la pioche
// \param	deck			Deck
// \return					Taille de la pioche
int getDrawPileSize (deck_t * deck);


// Obtention de la taille de la défausse
// \param	deck			Deck
// \return					Taille de la défausse
int getDiscardPileSize (deck_t * deck);


// Obtention du type de paquet utilisé dans le deck
// \param	deck			Deck
// \return					Type de paquet
decktype_t getDeckType (deck_t * deck);

// Affichage de la pioche
// \param	deck			Deck
void printDrawPile (deck_t * deck);


// Affichage de la défausse
// \param	deck			Deck
void printDiscardPile (deck_t * deck);


// Affichage de la valeur d'une carte (Valeur + Couleur)
// Valeur : A23456789XJKQ
// Couleur : cdsh (club/trèfle, diamond/carreau, spade/pique, heart/coeur)
// \param	value			Valeur de carte
void printCard (const int value);

#endif
