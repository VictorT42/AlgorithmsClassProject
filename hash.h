//File hash.h

#define BUCKET_SIZE 32

typedef struct Bucket
{
	Vector *key[BUCKET_SIZE];
	int data[BUCKET_SIZE];
	struct Bucket *overflow;
} Bucket;

typedef struct HashTable
{
	int size;
	Bucket **table;
} HashTable;

