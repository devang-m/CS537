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

int mappingCount = 1024;
int curCount = 0;
struct pairs* words;
pthread_mutex_t lock; 

void* mapperHelper(void *arg) {
	struct mapperThreadArgs *t = (struct mapperThreadArgs *)arg;
	t->m(t->filename);
	printf("%s\n", t->filename);
	return arg;
}

void MR_Emit(char *key, char *value) {
	pthread_mutex_lock(&lock); 
	curCount++;
	if (curCount > mappingCount) {
		mappingCount *= 2;
		words = (struct pairs *) realloc(words, mappingCount * sizeof(struct pairs));
	}
	words[curCount-1].key = (char*)malloc(strlen(key) * sizeof(char));
	strcpy(words[curCount-1].key, key);
	words[curCount-1].value = (char*)malloc(strlen(value) * sizeof(char));
	strcpy(words[curCount-1].value, value);
	pthread_mutex_unlock(&lock); 
}

void MR_Run(int argc, char *argv[], Mapper map, int num_mappers, Reducer reduce, int num_reducers, Partitioner partition) {
	words = (struct pairs*)malloc(mappingCount * sizeof(struct pairs));
	pthread_t mapperThreads[num_mappers];
	pthread_mutex_init(&lock, NULL);
	struct mapperThreadArgs t[num_mappers];

	for (int i = 0; i < num_mappers; i++){
		t[i].m = map;
		t[i].filename = argv[i+1];
		printf("%s\n", t[i].filename);
	    if(pthread_create(&mapperThreads[i], NULL, mapperHelper, &t[i])) {
	    	printf("Error\n");
	    }
	}
	for(int i = 0; i < num_mappers; i++) {
		pthread_join(mapperThreads[i], NULL); 
	}
	for(int i = 0; i < curCount; i++) {
		printf("%s ", (words[i].key));
		printf("%s\n", (words[i].value));
	}
	pthread_mutex_destroy(&lock);
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}
