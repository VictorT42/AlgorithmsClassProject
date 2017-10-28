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

QueryResult *query(Curve *queryCurve, int *numOfResults, Curve *curves, HashInfo *hashInfo, double radius, int k, int d,
	double (*distanceFunction)(Curve*, Curve*), int *nearest)
{
	int i, j;
	Vector *u;
	Vector g_u;
	int hashKey;
	Bucket *bucket, *currentBucket;
	QueryResult *result;
	int same;
	double distance;
	double minDistance;
	int arraySize;
	
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
	
	if(bucket->entries == 0)
		return NULL;
	
	if(radius == 0)
	{
		result = malloc(sizeof(QueryResult));
		*nearest = 0;
		*numOfResults = 1;
		
		//Try to find identical grid curve
		currentBucket = bucket;
		while(currentBucket != NULL)
		{
			for(i=0; i<currentBucket->entries; i++)
			{
				same = 0;
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
					result->curve = currentBucket->data[i];
					result->foundGridCurve = 1;
					result->distance = (*distanceFunction)(queryCurve, &(curves[result->curve]));
					free(u->coordinates);
					free(u);
					return result;
				}
			}
			currentBucket = currentBucket->overflow;
		}
		
		//Find nearest in bucket
		currentBucket = bucket;
		result->curve = currentBucket->data[0];
		result->foundGridCurve = 0;
		result->distance = (*distanceFunction)(queryCurve, &(curves[result->curve]));
		while(currentBucket != NULL)
		{
			for(i=0; i<currentBucket->entries; i++)
			{
				minDistance = (*distanceFunction)(queryCurve, &(curves[currentBucket->data[i]]));
				if(minDistance < result->distance)
				{
					result->curve = currentBucket->data[i];
					result->distance = minDistance;
				}
			}
			currentBucket = currentBucket->overflow;
		}
	}
	
	else
	{
		arraySize = INITIAL_RESULTS_SIZE;
		result = malloc(arraySize * sizeof(QueryResult));
		*numOfResults = 0;
		
		currentBucket = bucket;
		*nearest = 0;
		minDistance = INFINITY;
		
		while(currentBucket != NULL)
		{
			for(i=0; i<currentBucket->entries; i++)
			{
				distance = (*distanceFunction)(queryCurve, &(curves[currentBucket->data[i]]));
				if(distance <= radius)
				{
					result[*numOfResults].curve = currentBucket->data[i];
					result[*numOfResults].distance = distance;
					result[*numOfResults].foundGridCurve = 0;
					if(distance < minDistance)
					{
						*nearest = *numOfResults;
						minDistance = (*distanceFunction)(queryCurve, &(curves[result[*nearest].curve]));
					}
					(*numOfResults)++;
					if(*numOfResults == arraySize)
					{
						arraySize *= 2;
						result = realloc(result, arraySize*sizeof(QueryResult));
					}
				}
			}
			currentBucket = currentBucket->overflow;
		}
	}
	
	free(u->coordinates);
	free(u);
	return result;
	
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

