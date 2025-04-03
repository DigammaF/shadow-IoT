
#include <stdlib.h>

#include "deal.h"
#include "champion.h"
#include "item.h"

#define UNUSED(x) (void)(x)

void initChampionDeal(champion_deal_t* championDeal) {
	championDeal->champion = NULL;
	championDeal->seller = NULL;
}

void dropChampionDeal(champion_deal_t* championDeal) {
	champion_t* champion = championDeal->champion;

	if (champion != NULL) {
		dropChampion(champion);
		free(champion);
	}
}

void initItemDeal(item_deal_t* itemDeal) {
	itemDeal->item = NULL;
	itemDeal->seller = NULL;
}

void dropItemDeal(item_deal_t* itemDeal) {
	item_t* item = itemDeal->item;

	if (item != NULL) {
		dropItem(item);
		free(item);
	}
}
