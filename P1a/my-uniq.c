#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void uniq_util(FILE *f) {
	char *line = NULL;
	// start to keep a tab whether it is 1st line read or subsequent lines
	int line_length, start = 0;
	size_t buffer = 0;
	while(getline(&line, &buffer, f) > 0) {
		char *line2;
		line_length = strlen(line);
		// Only if there us more than 1 line, do we compare the lines
		if(start == 1) {
			if (strcmp(line, line2) == 0) {
				continue;
			}
			// freeing the space of the old line
			free(line2);
		}
		start = 1;
		// Add 1 for the null terminating value
		line2 =  (char*) malloc((line_length + 1) * sizeof(char));
		// Copying the new line (line) to (line2)
		strcpy(line2, line);
		// printing the new line (line2)
		printf("%s", line2);
	}
}

int main(int argc, char *argv[]) {
	// Checks for file input
	if(argc > 1) {
		int i;
		// Iterate through the files
		for(i = 1; i < argc; i++) {
			FILE *f = fopen(argv[i], "r");
			// Check if file is opened successfully
			if (f == NULL) {
				printf("my-uniq: cannot open file\n");
				exit(1);
			}
			uniq_util(f);
		}
	}
	// Stdin input
	else {
		uniq_util(stdin);
	}
	return 0;
}