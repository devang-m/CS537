// Code for P4a
// Free allocated memory
// Write get_next function
// Maybe put everything in a struct (global vars)
// Check for the issue of words with no length
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mapreduce.h"

struct pairs {
	char *key;
	char *value;
};

struct pairs** partitions;
int* pairCountInPartition;
int* pairAllocatedInPartition;
pthread_mutex_t lock;
Partitioner p;
Reducer r;
Mapper m;
int numberPartitions;

// Helper function to be called by pthread_create which calls the mapper function
void* mapperHelper(void *arg) {
	char* filename = (char *)arg;
	m(filename);
	return arg;
}

// Helper function to be called by pthread_create which calls the get_next function for each reducer
void* reducerHelper(void *arg) {
	int* partitionNumber = (int *)arg;
	printf("%d\n", *partitionNumber);
	return arg;
}

// Sort the buckets by key and then by value in ascending order
int compare(const void* p1, const void* p2) {
	struct pairs *pair1 = (struct pairs*) p1;
	struct pairs *pair2 = (struct pairs*) p2;
	if (strcmp(pair1->key, pair2->key) == 0) {
		return strcmp(pair1->value, pair2->value);
	}
	return strcmp(pair1->key, pair2->key);
}

void MR_Emit(char *key, char *value) {
	pthread_mutex_lock(&lock); 
	// Checking to ensure words have a length
	if(strlen(key)!= 0) {
		// Getting the partition number
		unsigned long hashPartitionNumber = p(key, numberPartitions);
		pairCountInPartition[hashPartitionNumber]++;
		int curCount = pairCountInPartition[hashPartitionNumber];
		// Checking if allocated memory has been exceeded,if yes allocating more memory
		if (curCount > pairAllocatedInPartition[hashPartitionNumber]) {
			pairAllocatedInPartition[hashPartitionNumber] *= 2;
			partitions[hashPartitionNumber] = (struct pairs *) realloc(partitions[hashPartitionNumber], pairAllocatedInPartition[hashPartitionNumber] * sizeof(struct pairs));
		}
		partitions[hashPartitionNumber][curCount-1].key = (char*)malloc(strlen(key) * sizeof(char));
		strcpy(partitions[hashPartitionNumber][curCount-1].key, key);
		partitions[hashPartitionNumber][curCount-1].value = (char*)malloc(strlen(value) * sizeof(char));
		strcpy(partitions[hashPartitionNumber][curCount-1].value, value);
	}
	pthread_mutex_unlock(&lock); 
}

void MR_Run(int argc, char *argv[], Mapper map, int num_mappers, Reducer reduce, int num_reducers, Partitioner partition) {

	//Initialising all the required variables
	pthread_t mapperThreads[num_mappers];
	pthread_t reducerThreads[num_reducers];
	pthread_mutex_init(&lock, NULL);
	p = partition;
	m = map;
	r = reduce;
	numberPartitions = num_reducers;
	partitions = malloc(num_reducers * sizeof(struct pairs*));
	pairCountInPartition = malloc(num_reducers * sizeof(int));
	pairAllocatedInPartition = malloc(num_reducers * sizeof(int));
	int arrayPosition[num_reducers];

	// Initialising the arrays needed to store the key value pairs in the partitions
	for(int i = 0; i < num_reducers; i++) {
		partitions[i] = malloc(1024 * sizeof(struct pairs));
		pairCountInPartition[i] = 0;
		pairAllocatedInPartition[i] = 1024;
		arrayPosition[i] = i;
	}

	// Creating the threads for the number of mappers
	for (int i = 0; i < num_mappers; i++){
	    if(pthread_create(&mapperThreads[i], NULL, mapperHelper, argv[i+1])) {
	    	printf("Error\n");
	    }
	}

	//Waiting for the threads to finish
	for(int i = 0; i < num_mappers; i++) {
		pthread_join(mapperThreads[i], NULL); 
	}


	for(int i = 0; i < num_reducers; i++) {
		qsort(partitions[i], pairCountInPartition[i], sizeof(struct pairs), compare);
	}

	// Printing to debug
	for(int i = 0; i < num_reducers; i++) {
		printf("Reducer number: %d\n", i);
		for(int j = 0; j < pairCountInPartition[i]; j++) {
			printf("%s ", (partitions[i][j].key));
			printf("%s\n", (partitions[i][j].value));
		}
	}

	// Creating the threads for the number of reducers
	for (int i = 0; i < num_reducers; i++){
	    if(pthread_create(&reducerThreads[i], NULL, reducerHelper, &arrayPosition[i])) {
	    	printf("Error\n");
	    }
	}

	//Waiting for the threads to finish
	for(int i = 0; i < num_reducers; i++) {
		pthread_join(reducerThreads[i], NULL); 
	}

	pthread_mutex_destroy(&lock);
}

// Given default hash function
unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}
