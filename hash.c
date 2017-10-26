//File hash.c

#include <stdio.h>
#include <stdlib.h>
#include "curves.h"
#include "hash.h"

#define M ((long)2<<31) - 5

int hash(Vector *u, int *r, int tableSize)
{
	long long i;
	int sum;
	
	for(i=0; i<u->length; i++)
	{
		sum += ((long)u->coordinates[i] * r[i]) % M;
	}
	sum = sum % M;	
	sum = sum % tableSize;
	
	return sum;
	
}

HashTable *createHashTable(int size)
{
	int i;
	
	HashTable *this = malloc(sizeof(HashTable));
	
	this->size = size;
	this->table = malloc(size*sizeof(Bucket*));
	
	for(i=0; i<size; i++)
		this->table[i] = malloc(sizeof(Bucket));
	
	return this;
	
}

