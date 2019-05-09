#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>

typedef struct HistoryNode {
	char *command;
	struct HistoryNode* next;
}HistoryNode;

typedef struct PathNode {
	char *path;
	struct PathNode* next;
}PathNode;


int getInputs(char* line, char *a[128]) {
	char *delim = "\n \t\r\f\v";
	int i = 0;
	char *input ;
	input = strtok(line, delim);
	while(input != NULL) {
		a[i] = input;
		i = i + 1;
		input = strtok(NULL, delim);
	}
	return i;
}

void printHistory(HistoryNode *iterator, int n) {
	if (iterator == NULL || n <= 0)
		return;
	printHistory(iterator->next, n-1);
	printf("%s\n", iterator->command);
	fflush(stdout);
}

int main(int argc, char *argv[]) {
	char *commandExit = "exit";
	char *commandCd = "cd";
	char *commandHistory = "history";
	char *commandPath = "path";
	char *initialPath = "/bin";
	char error_message[30] = "An error has occurred\n";
	if(argc <= 2) {

		FILE *f = NULL;
		if(argc == 2) {
			f = fopen(argv[1], "r");
			if(f == NULL) {
				write(STDERR_FILENO, error_message, strlen(error_message));
				exit(1);
			}
		}
		else {
			f = stdin;
		}

		HistoryNode *head = NULL;
		PathNode *paths = (PathNode *)malloc(sizeof(PathNode));
		paths->path = strdup(initialPath);
		paths->next = NULL;
		int numberOfCommandsCount = 0;

		while (1) {
			int redirectNum = 0, pipeNum = 0;
			int posOfRedirect = -1, posOfPipe = -1;
			int pipefd[2]; // 0 is for read, 1 is for write
			if (argc == 1) {
				printf("wish> ");
				fflush(stdout);
			}
			char *line = NULL;
			size_t buffer = 0;
			// Read each line of stdin                                      
			if(getline(&line, &buffer, f) == EOF) {
				exit(0);
			}

			// As history should not include blank lines
			if (strcmp(line, "\n") == 0) {
				continue;
			}

			// Important
			line = strtok(line, "\n");

			// History should include everything apart from only blank line
			numberOfCommandsCount = numberOfCommandsCount + 1;
			HistoryNode *newNode = (HistoryNode *)malloc(sizeof(HistoryNode));
			newNode->command = strdup(line);
			newNode->next = head;
			head = newNode;
			
			char *a[128];
			char *b[128];

			// To account for < and | being there without spaces
			char *newLine = (char *) malloc ((strlen(line) + 33) * sizeof(char));
			int c = 0;
			for(int i = 0; i < strlen(line); i++) {
				if(line[i] == '>' || line[i] == '|') {
					newLine[c++] = ' ';
					newLine[c++] = line[i];
					newLine[c++] = ' ';
					continue;
				}
				newLine[c++] = line[i];
			}
			newLine[c++] = '\0';
			
			free(line);

			// Getting the inputs
			int args = getInputs(newLine, a);

			if(args == 0) {
				continue;
			}

			// Counting number of pipes and redirections
			for (int i = 0; i < args; i++) {
				if (strcmp(a[i], ">") == 0) {
					redirectNum = redirectNum + 1;
					posOfRedirect = i;
				}
				if (strcmp(a[i], "|") == 0) {
					pipeNum = pipeNum + 1;
					posOfPipe = i;
				}
			}
			// If more than 1 pipe/redirect
			if (redirectNum > 1 || pipeNum > 1 || (redirectNum >= 1 && pipeNum >= 1)) {
				write(STDERR_FILENO, error_message, strlen(error_message));
				free(newLine);
				continue;
			}


			// Checking that there is only 1 file after redirect
			if (redirectNum == 1) {
				// If there is more than 1 value after > or no values before >
				if (args-1-posOfRedirect !=1 || posOfRedirect == 0) {
					write(STDERR_FILENO, error_message, strlen(error_message));
					free(newLine);
					continue;
				}
				// Redirect Command is OK
				else {
					a[posOfRedirect] = NULL;
				}
			}

			// Checking for piping
			if (pipeNum == 1) {
				// No values before and after pipe
				if (args-1-posOfPipe == 0 || posOfPipe == 0) {
					write(STDERR_FILENO, error_message, strlen(error_message));
					free(newLine);
					continue;
				}
				else {
					pipe(pipefd);
					int counter = 0;
					for(int i = posOfPipe + 1; i < args; i++) {
						b[counter] = a[i];
						counter++;
					}
					b[counter] = NULL;
					a[posOfPipe] = NULL;
				}
			}

			// If exit command
			if(strcmp(a[0], commandExit) == 0) {
				if(args > 1) {
					write(STDERR_FILENO, error_message, strlen(error_message));
					free(newLine);
					continue;
				}
				exit(0);
			}

			// If cd command
			else if(strcmp(a[0], commandCd) == 0) {
				if(args != 2) {
					write(STDERR_FILENO, error_message, strlen(error_message)); 
					free(newLine);
					continue;
				}
				int resultCd = chdir(a[1]);
				if(resultCd != 0) {
					write(STDERR_FILENO, error_message, strlen(error_message));
					free(newLine);
					continue;
				}
			}

			// If history command
			else if(strcmp(a[0], commandHistory) == 0) {
				if(args > 2) {
					write(STDERR_FILENO, error_message, strlen(error_message));
					free(newLine);
					continue;
				}
				if (args == 1) {
					HistoryNode *iterator = head;
					printHistory(iterator, numberOfCommandsCount);
					iterator = NULL;
				}
				if (args == 2) {
					int number = ceil(atof(a[1]));
					// If not integer or has 'e'
					if ((number == 0 && a[1][0]!='0') || (strchr(a[1], 'e')!=NULL)) {
						write(STDERR_FILENO, error_message, strlen(error_message));
						free(newLine);
						continue;
					}
					HistoryNode *iterator = head;
					printHistory(iterator, number);
					iterator = NULL;
				}
			}

			// If path command
			else if(strcmp(a[0], commandPath) == 0) {
				// Deleting previous space
				while(paths != NULL) {
					PathNode *cur = paths;
					paths = paths->next;
					free(cur);
				}
				for(int i = 1; i < args; i++) {
					PathNode *temp = (PathNode *)malloc(sizeof(PathNode));
					temp->path = strdup(a[i]);
					temp->next = paths;
					paths = temp;
				}
			}

			// Commands to find inside the paths
			else {
				PathNode *temp = paths;
				PathNode *temp2 = paths;
				int flag = 0, flagForSecondChild = 0, flagExecv = 0, flagFork = 0;
				while(temp!=NULL) {
					//printf("HI\n");
					char *pathToCheck = strdup(temp->path);
					strcat(pathToCheck, "/");
					strcat(pathToCheck, a[0]);
					a[args] = NULL;
					if(access(pathToCheck, X_OK) == 0) {
						flag = 1;
						int rc = fork();
						if (rc == 0) {
							// Reference - https://stackoverflow.com/questions/2605130/redirecting-exec-output-to-a-buffer-or-file
							if(redirectNum == 1) {
								int fd = open(a[args-1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
								dup2(fd, 1);
								dup2(fd, 2);
								close(fd);
							}
							if (pipeNum == 1) {
								// close the read for 1st one
								close(pipefd[0]);    // close reading end in the child
								dup2(pipefd[1], 1);  // send stdout to the pipe
								dup2(pipefd[1], 2);  // send stderr to the pipe
								close(pipefd[1]);
							}
							flagExecv = execv(pathToCheck, a);
						}
						// fork error
						else if (rc == -1) {
							flagFork = -1;
						}
						else {
							rc = (int) wait(NULL);
							if (pipeNum == 1) {
								while(temp2!=NULL) {
									char *pathToCheckForSecondChild = strdup(temp2->path);
									strcat(pathToCheckForSecondChild, "/");
									strcat(pathToCheckForSecondChild, b[0]);
									if(access(pathToCheckForSecondChild, X_OK) == 0) {
										flagForSecondChild = 1;
										int forkpipe = fork();
										// 2nd child
										if(forkpipe == 0) {
											close(pipefd[1]);    // close writing end in the child
											dup2(pipefd[0], 0);  // send stdin to the pipe
											close(pipefd[0]);
											flagExecv = execv(pathToCheckForSecondChild, b);
										}
										// fork error
										else if (forkpipe == -1) {
											flagFork = -1;
										}
										else {
											close(pipefd[1]);
											close(pipefd[0]);
											forkpipe = (int) wait(NULL);
										}
									}
									free(pathToCheckForSecondChild);
									temp2 = temp2->next;
								}
							}
						}
					}
					free(pathToCheck);
					temp = temp->next;
				}
				// Cannot access anywhere in the paths or the execv/fork returns as error
				if (flag == 0 || (pipeNum == 1 && flagForSecondChild == 0) || flagExecv == -1 || flagFork == -1) {
					// Incase the 1st one is a bad argument, we still write error message to the new file
					if(redirectNum == 1) {
						int fd = open(a[args-1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
						write(fd, error_message, strlen(error_message));
						close(fd);
					}
					else {
						write(STDERR_FILENO, error_message, strlen(error_message));
					}
				}
			}
			// Freeing space
			for(int i = 0; i < args; i++) {
				a[i] = NULL;
				b[i] = NULL;
			}
			free(newLine);
		}
	}
	else {
		write(STDERR_FILENO, error_message, strlen(error_message));
		exit(1);
	}
  return 0;
}
