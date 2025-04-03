
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

char** splitString(const char* str, unsigned* count);
void freeSplit(char** result, unsigned count);

char* joinString(char** str, const char* joiner);
void freeJoin(char* str);

int safeStrToUnsigned(const char *str, unsigned *out);

#endif
