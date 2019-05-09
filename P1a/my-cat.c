#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	// Checks if file has been given in the arguments
	if (argc == 1) {
		exit(0);
	}
	int i;
	// Iterates through the files
	for(i = 1; i < argc; i++) {
		FILE *f = fopen(argv[i], "r");
		// Check if file is opened successfully
		if (f == NULL) {
			printf("my-cat: cannot open file\n");
			exit(1);
		}
		char buffer[1000];
		// Get the line from the file and display it to the user
		while (fgets(buffer, 1000, f) != NULL) {
			printf("%s", buffer);
		}
		// Closing the file
		fclose(f);
	}
	return 0;
}