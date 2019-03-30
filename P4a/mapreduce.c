// Code for P4a
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mapreduce.h"

struct mapping {
	char *key;
	char *value;
};

int mappingCount = 1024;
int curCount = 0;
struct mapping* words;

void MR_Emit(char *key, char *value) {
	curCount++;
	if (curCount > mappingCount) {
		mappingCount *= 2;
		words = (struct mapping *) realloc(words, mappingCount * sizeof(struct mapping));
	}
	words[curCount-1].key = (char*)malloc(strlen(key) * sizeof(char));
	strcpy(words[curCount-1].key, key);
	words[curCount-1].value = (char*)malloc(strlen(value) * sizeof(char));
	strcpy(words[curCount-1].value, value);
}

void MR_Run(int argc, char *argv[], Mapper map, int num_mappers, Reducer reduce, int num_reducers, Partitioner partition) {
	words = (struct mapping*)malloc(mappingCount * sizeof(struct mapping));
	map(argv[1]);
	printf("%d\n", curCount);
	for(int i = 0; i < curCount; i++) {
		printf("%s ", (words[i].key));
		printf("%s\n", (words[i].value));
	}
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}