
#ifndef PLACE_H
#define PLACE_H

#include "hashmap.h"
#include "vector.h"

struct pawn_t;
struct server_t;
struct event_t;
struct feature_t;

typedef struct place_t {
    char* name;
    hashmap_t links; // link_t*, possède la valeur
    hashmap_t pawns; // pawn_t*
    hashmap_t features; // feature_t*, possède la valeur
} place_t;

typedef struct link_t {
    char* name;
    place_t* target;
	unsigned id;
} link_t;

void initPlace(place_t* place);
void dropPlace(place_t* place);

void initLink(link_t* link);
void dropLink(link_t* link);

link_t* createLink(place_t* source, place_t* destination, char* name);
void deleteLink(place_t* place, unsigned key);

void makePlain(place_t* place);
void makeDesert(place_t* place);

void addPawnToPlace(place_t* place, struct pawn_t* pawn);
void removePawnFromPlace(place_t* place, struct pawn_t* pawn);

void addFeatureToPlace(place_t* place, struct feature_t* feature);
void removeFeatureFromPlace(place_t* place, struct feature_t* feature);

/** propage un évènement à tous les pions se trouvant dans la place */
void notifyPlace(struct server_t* server, place_t* place, struct event_t* event);

void notifyPawnLeft(struct server_t* server, place_t* place, struct pawn_t* pawn);
void notifyPawnArrived(struct server_t* server, place_t* place, struct pawn_t* pawn);
void notifyPawnSpawned(struct server_t* server, place_t* place, struct pawn_t* pawn);

void movePawn(struct server_t* server, struct pawn_t* pawn, link_t* link);

#endif
