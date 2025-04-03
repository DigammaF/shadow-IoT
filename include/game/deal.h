
#ifndef DEAL_H
#define DEAL_H

struct champion_t;
struct item_t;
struct pawn_t;

typedef struct champion_deal_t {
	struct champion_t* champion; // possède la valeur
	unsigned price;
	struct pawn_t* seller;
	unsigned key;
} champion_deal_t;

void initChampionDeal(champion_deal_t* championDeal);
void dropChampionDeal(champion_deal_t* championDeal);

typedef struct item_deal_t {
	struct item_t* item; // possède la valeur
	unsigned price;
	struct pawn_t* seller;
	unsigned key;
} item_deal_t;

void initItemDeal(item_deal_t* itemDeal);
void dropItemDeal(item_deal_t* itemDeal);

#endif
