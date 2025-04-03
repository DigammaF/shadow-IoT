
#include <stdlib.h>

int getRandomInt(int min, int max) { return min + rand() % (max - min); }
int getRandomIntInclusive(int min, int max) { return min + rand() % (max - min + 1); }
