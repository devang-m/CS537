// Code for P4a
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mapreduce.h"

struct pairs {
	char *key;
	char *value;
};

struct mapperThreadArgs {
	Mapper m;
	char *filename;
};

int curCount = 0;
struct pairs** partitions;
int* pairCountInPartition;
int* pairAllocatedInPartition;
pthread_mutex_t lock;
Partitioner p;
int numberPartitions;

// Helper function to be called by pthread_create which calls the mapper function
void* mapperHelper(void *arg) {
	struct mapperThreadArgs *t = (struct mapperThreadArgs *)arg;
	t->m(t->filename);
	return arg;
}

void MR_Emit(char *key, char *value) {
	// Check for words with no length
	pthread_mutex_lock(&lock); 
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
	pthread_mutex_init(&lock, NULL);
	struct mapperThreadArgs t[num_mappers];
	p = partition;
	numberPartitions = num_reducers;
	partitions = malloc(num_reducers * sizeof(struct pairs*));
	pairCountInPartition = malloc(num_reducers * sizeof(int));
	pairAllocatedInPartition = malloc(num_reducers * sizeof(int));

	// Initialising the arrays needed to store the key value pairs in the partitions
	for(int i = 0; i < num_reducers; i++) {
		partitions[i] = (struct pairs*)malloc(1024 * sizeof(struct pairs));
		pairCountInPartition[i] = 0;
		pairAllocatedInPartition[i] = 1024;
	}

	// Creating the threads for the number of mappers
	for (int i = 0; i < num_mappers; i++){
		t[i].m = map;
		t[i].filename = argv[i+1];
	    if(pthread_create(&mapperThreads[i], NULL, mapperHelper, &t[i])) {
	    	printf("Error\n");
	    }
	}

	//Waiting for the threads to finish
	for(int i = 0; i < num_mappers; i++) {
		pthread_join(mapperThreads[i], NULL); 
	}

	// Printing to debug
	for(int i = 0; i < num_reducers; i++) {
		printf("Reducer number: %d\n", i);
		for(int j = 0; j < pairCountInPartition[i]; j++) {
			printf("%s ", (partitions[i][j].key));
			printf("%s\n", (partitions[i][j].value));
		}
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
