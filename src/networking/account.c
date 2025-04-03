
#include <stdlib.h>

#include "account.h"

#define UNUSED(x) (void)(x)

void initAccount(account_t* account) {
	account->flags = 0;
	account->pawn = NULL;
}

void dropAccount(account_t* account) {
	UNUSED(account);
}
