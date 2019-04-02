#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "mapreduce.h"

#ifndef NUM_MAPPERS
    #define NUM_MAPPERS 1
#endif

#ifndef NUM_REDUCERS
    #define NUM_REDUCERS 1
#endif

#ifndef FILE_OUTPUT_SUFFIX
    #define FILE_OUTPUT_SUFFIX ""
#endif

int output_fd[NUM_REDUCERS];

void Map(char *file_name) {
    FILE *fp = fopen(file_name, "r");
    assert(fp != NULL);

    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, fp) != -1) {
        char *token, *dummy = line;
        while ((token = strsep(&dummy, "., \t\n\r")) != NULL) {
            MR_Emit(token, "1");
        }
    }
    free(line);
    fclose(fp);
}

void Reduce(char *key, Getter get_next, int partition_number) {
    int count = 0;
    char *value;
    while ((value = get_next(key, partition_number)) != NULL)
        count++;
    dprintf(output_fd[partition_number],"%s %d\n", key, count);
}

int main(int argc, char *argv[]) {
    clock_t start, end;
    double cpu_time_used;

    char buf[64];
    for(int i=0;i<NUM_REDUCERS;i++){
    	sprintf(buf, "wordcount_%s(%d).out", FILE_OUTPUT_SUFFIX,i);
    	if( (output_fd[i] = open(buf, O_CREAT|O_TRUNC|O_WRONLY,0664)) < 0 ){
    	    perror("file open error");
    	    exit(1);
    	}
    }

    start = clock();
    MR_Run(argc, argv, Map, NUM_MAPPERS, Reduce, NUM_REDUCERS, MR_DefaultHashPartition);
    end = clock();

    cpu_time_used = ((double) (end - start)) / (CLOCKS_PER_SEC/1000);
    printf("** %.3f ms elapsed **\n", cpu_time_used);

    for(int i=0;i<NUM_REDUCERS;i++){
        if( close(output_fd[i]) < 0 ){
            perror("file open error");
            exit(1);
        }
    }
}
