
#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>

#include "hashmap.h"

struct pawn_t;
struct server_t;
struct place_t;
struct champion_deal_t;
struct item_deal_t;
struct fight_t;

/**
 * 
 *  Attention, la quantité de mémoire requise pour stocker le monde est fonction cubique de cette constante.
 * 	Une constante trop grande va provoquer un seg fault.
 * 
 * 	L'analyse du fichier dump correspondant donne:
 * 
 * 		Program terminated with signal SIGSEGV, Segmentation fault.
#0  0x0000561e6c08569a in mainServer (argc=<error reading variable: Cannot access memory at address 0x7ff5b5e5c53c>,
    argv=<error reading variable: Cannot access memory at address 0x7ff5b5e5c530>) at src/networking/server.c:224
224     int mainServer(int argc, const char** argv) {
(gdb) info locals
server = <error reading variable server (value of type `server_t' requires 34358689880 bytes, which is more than max-value-size)>
 * 
 */
#define WORLD_SIZE 600

typedef struct {
    struct place_t* places[WORLD_SIZE][WORLD_SIZE]; // possède la valeur
	hashmap_t pawns; // pawn_t*, possède la valeur
	hashmap_t championDeals; // champion_deal_t*, possède la valeur
	hashmap_t itemDeals; // item_deal_t*, possède la valeu
	hashmap_t fights; /** 	fight_t*, possède la valeur, ensemble des combats en cours dans le monde,  */ 
} world_t;

void initWorld(world_t* world);
void dropWorld(world_t* world);

void addPawnToWorld(world_t* world, struct pawn_t* pawn);
void removePawnFromWorld(world_t* world, struct pawn_t* pawn);

void generateWorld(unsigned seed, world_t* world);
void spawnPawn(struct server_t* server, struct place_t* place, struct pawn_t* pawn);

void addChampionDealToWorld(world_t* world, struct champion_deal_t* deal);
void removeChampionDealFromWorld(world_t* world, struct champion_deal_t* deal);

void addItemDealToWorld(world_t* world, struct item_deal_t* deal);
void removeItemDealFromWorld(world_t* world, struct item_deal_t* deal);

void addFightToWorld(world_t* world, struct fight_t* fight);
void removeFightFromWorld(world_t* world, struct fight_t* fight);

/** applique les opérations routinières sur les objets qui composent le monde */
void updateWorld(struct server_t* server, world_t* world);

#endif
