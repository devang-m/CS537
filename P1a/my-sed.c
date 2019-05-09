#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_line(char *line, char *find, char *replace, int blank) {
	char* substr;
	// Check if the find term is there in the line
	substr = strstr(line, find);
	if (substr != NULL) {
		// Prints the characters before the find term
		printf("%.*s", (int)(substr - line), line);
		// Prints the replace term
		if(blank == 0) {
			printf("%s", replace);
		}
		// Prints the characters after the find term
		printf("%.*s", (int)(strlen(substr) - strlen(find)), substr + (int)(strlen(find)));
	}
	else {
		printf("%s", line);
	}
}

int main(int argc, char *argv[]) {
	// Check if mandatory arguments exist
	if (argc < 3) {
		printf("my-sed: find_term replace_term [file ...]\n");
		exit(1);
	}
	// Check whether replace term is blank
	int blank = 0;
	if (strcmp(argv[2], "”“") == 0) {
		blank = 1;
	}
	// File input
	if (argc > 3) {
		int i;
		// Iterate through the files
		for(i = 3; i < argc; i++) {
			FILE *f = fopen(argv[i], "r");
			// Check if file is opened successfully
			if (f == NULL) {
				printf("my-sed: cannot open file\n");
				exit(1);
			}
			char *line = NULL;
  			size_t buffer = 0;
  			// Read each line of the file
  			while(getline(&line, &buffer, f) > 0) {
  				print_line(line, argv[1], argv[2], blank);
  			}
		}
	}
	// Standard input
	if (argc == 3) {
		char *line = NULL;
		size_t buffer = 0;
		// Read each line of stdin
		while(getline(&line, &buffer, stdin) > 0) {
			print_line(line, argv[1], argv[2], blank);
		}
	}
	return 0;
}