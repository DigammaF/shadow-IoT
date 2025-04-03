
#ifndef RANDOM_UTILS_H
#define RANDOM_UTILS_H

/**
 * 
 * 	Génère un nombre aléatoirement compris dans [min; max[
 *  à utiliser avec srand(unsigned)
 * 
 */
int getRandomInt(int min, int max);

/**
 * 
 * 	Même chose avec [min; max]
 * 
 */
int getRandomIntInclusive(int min, int max);

#endif
