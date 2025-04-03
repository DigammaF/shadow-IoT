
#ifndef ACCOUNT_H
#define ACCOUNT_H

#define ADMIN_FLAG (1 << 0)
#define BANNED_FLAG (1 << 1)

struct pawn_t;

typedef struct account_t {
	char* name;
	char* password;
	unsigned flags;
	unsigned id;
	struct pawn_t* pawn;
} account_t;

void initAccount(account_t* account);
void dropAccount(account_t* account);

#endif
