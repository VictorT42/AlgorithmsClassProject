//File hash.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "curves.h"
#include "hash.h"

#define M ((long)2<<31) - 5

int hash(Vector *u, int *r, int tableSize)
{
	long long i;
	long sum = 0;
	long product;
	
	for(i=0; i<u->length; i++)
	{
		product = ((long)(10000*(u->coordinates[i])) * r[i]) % M;
		if(product < 0)
			product = -product;
		sum +=product;
	}
	sum = sum % M;	
	sum = sum % tableSize;
	
	return sum;
	
}

void g(Vector *g_u, Vector *u, GParameters *h)
{
	int i, j;
	// Vector *g_u;
	double sum = 0;
	
	// g_u->length = K_VEC;
	// g_u->coordinates = malloc(g_u->length * sizeof(double));
	
	for(i=0; i<K_VEC; i++)
	{
		for(j=0; j<u->length; j++)
			sum += u->coordinates[i]* h[i].v->coordinates[i];
		sum += h[i].t;
		g_u->coordinates[i] = (int) (sum/W);
	}
	
	// return g_u;
	
}

HashTable *createHashTable(int size)
{
	int i;
	
	HashTable *this = malloc(sizeof(HashTable));
	
	this->size = size;
	this->table = malloc(size*sizeof(Bucket*));
	
	for(i=0; i<size; i++)
		this->table[i] = createBucket();
	
	return this;
	
}

int insertToTable(HashTable *hashTable, Vector *hashKey, Vector *tableKey, int data, int *r)
{
	Bucket *bucket;
	int position;
	
	position = hash(hashKey, r, hashTable->size);
	bucket = hashTable->table[position];
	
	insertToBucket(bucket, tableKey, data);
	
	return position;
}

void destroyHashTable(HashTable *hashTable)
{
	int i;
	
	for(i=0; i<hashTable->size; i++)
		destroyBucket(hashTable->table[i]);
	
	free(hashTable);
}

Bucket *createBucket()
{
	Bucket *newBucket;
	
	newBucket = malloc(sizeof(Bucket));
	newBucket->overflow = NULL;
	newBucket->entries = 0;
	
	return newBucket;
}

void insertToBucket(Bucket *bucket, Vector *key, int data)
{
	if(bucket->entries == BUCKET_SIZE)
	{
		if(bucket->overflow == NULL)
			bucket->overflow = createBucket();
		insertToBucket(bucket->overflow, key, data);
		return;
	}
	
	bucket->key[bucket->entries] = key;
	bucket->data[bucket->entries] = data;
	(bucket->entries)++;
	
}

void destroyBucket(Bucket *bucket)
{
	int i;
	
	for(i=0; i<bucket->entries; i++)
		free(bucket->key[i]);
	
	if(bucket->overflow)
		destroyBucket(bucket->overflow);
	
	free(bucket);
}

int countEntries(Bucket *b)
{
	if(!b)
		return 0;
	return b->entries + countEntries(b->overflow);
}

