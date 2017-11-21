//File hash.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "curves.h"
#include "hash.h"

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
	double sum = 0;
	
	for(i=0; i<K_VEC; i++)
	{
		for(j=0; j<u->length; j++)
			sum += u->coordinates[i]* h[i].v->coordinates[i];
		sum += h[i].t;
		g_u->coordinates[i] = (int) (sum/W);
	}
	
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

void query(Curve *queryCurve, QueryResult *nearest, int **rCurves, int *numOfRCurves, Curve *curves, HashInfo *hashInfo, double radius, int k, int d,
	double (*distanceFunction)(Curve*, Curve*))
{
	int i, j;
	Vector *u;
	Vector g_u;
	int hashKey;
	Bucket *bucket, *currentBucket;
	int same;
	double distance;
	int arraySize;
	int added;
	
	u = snapToGrid(queryCurve, k, d, hashInfo->grids);
	
	if(hashInfo->gParameters == NULL)  //Classic hashing
	{
		hashKey = hash(u, hashInfo->r, hashInfo->hashTable->size);
		bucket = hashInfo->hashTable->table[hashKey];
	}
	else  //LSH
	{
		g_u.length = K_VEC;
		g_u.coordinates = malloc(g_u.length * sizeof(double));
		g(&g_u, u, hashInfo->gParameters);
		hashKey = hash(&g_u, hashInfo->r, hashInfo->hashTable->size);
		bucket = hashInfo->hashTable->table[hashKey];
		free(g_u.coordinates);
	}
	
	
	nearest->curve = -1;
	nearest->foundGridCurve = 0;
	nearest->distance = INFINITY;
	
	if(bucket->entries == 0)  //If bucket is empty there are no results
	{
		free(u->coordinates);
		free(u);
		return;
	}
	
	if(radius != 0)
	{
		arraySize = countEntries(bucket);
		*rCurves = realloc(*rCurves, (*numOfRCurves + arraySize) * sizeof(int));
	}
	else
	{
		//Try to find identical grid curve if R=0
		currentBucket = bucket;
		while(currentBucket != NULL)
		{
			for(i=0; i<currentBucket->entries; i++)
			{
				if(u->length != currentBucket->key[i]->length)
					continue;
				
				same = 1;
				for(j=0; j<u->length; j++)
				{
					if(u->coordinates[j] != currentBucket->key[i]->coordinates[j])
					{
						same = 0;
						break;
					}
				}
				if(same == 1)
				{
					nearest->curve = currentBucket->data[i];
					nearest->foundGridCurve = 1;
					nearest->distance = (*distanceFunction)(queryCurve, &(curves[nearest->curve]));
					free(u->coordinates);
					free(u);
					return;
				}
			}
			currentBucket = currentBucket->overflow;
		}
	}
	
	
	//Find nearest and all R-curves in bucket
	currentBucket = bucket;
	while(currentBucket != NULL)
	{
		for(i=0; i<currentBucket->entries; i++)
		{
			distance = (*distanceFunction)(queryCurve, &(curves[currentBucket->data[i]]));
			if(distance < nearest->distance)
			{
				nearest->curve = currentBucket->data[i];
				nearest->distance = distance;
			}
			if(radius != 0 && distance <= radius)
			{
				added = 0;
				for(j=0; j<(*numOfRCurves); j++)  // Check if the curve is already in the results of a different hashtable
				{
					if((*rCurves)[j] == currentBucket->data[i])
					{
						added = 1;
						break;
					}
				}
				if(!added)
				{
					(*rCurves)[*numOfRCurves] = currentBucket->data[i];
					(*numOfRCurves)++;
				}
			}
		}
		currentBucket = currentBucket->overflow;
	}
	
	free(u->coordinates);
	free(u);
	return;
	
}

void destroyHashTable(HashTable *hashTable)
{
	int i;
	
	for(i=0; i<hashTable->size; i++)
		destroyBucket(hashTable->table[i]);
	free(hashTable->table);
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
	{
		free(bucket->key[i]->coordinates);
		free(bucket->key[i]);
	}
	
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

