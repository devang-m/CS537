# my-cat

1) If no files (argc =1) are specified, the program just returns 0.
2) Then, it iterates through every file given in the arguments.
3) Checks if the file can be opened, if not, it prints the error message and exits with status code 1.
4) Allocate a char array of size 1000, and then loop and as long as the read is successful, it prints the string read.
5) Closes the file and returns 0.

# my-sed
**main function**
1) Checks if mandatory arguments exist, if not, it prints the error message and exits with status code 1.
2) Checks if the replace term is ”“, if yes, changes the value of flag variable 'blank' to 1.
3) If there is file input, it iterates through every file given in the arguments.
4) Checks if the file can be opened, if not, it prints the error message and exits with status code 1.
5) Reads each line in a file and calls the print_line function.
6) If there is stdin input, reads each line in stdin and calls the print_line function.

**print_line function**
1) Checks if the term needed to be found, exists in the line using strstr function.
2) If it exists, then it prints out the line with the replaced word.
3) Else just prints the line.

# my-uniq
**main function**
1) Checks if there is file input, it iterates through every file given in the arguments.
2) Checks if the file can be opened, calls the uniq_util function; if not, it prints the error message and exits with status code 1.
3) If there is stdin input, calls the uniq_util function.

**uniq_util function**
1) Keeps a tab of the number of lines read in the 'start' variable. If it is the 1st line, start is 0, else it is 1.
2) The variable 'line' is current line, 'line2' is previous line.
3) If there is more than 1 line, checks if 'line' and 'line2' are the same. If yes, continues to the next line and does not print anything; else frees up the space held by 'line2'.
4) Allocated new space to 'line2' to store the current line 'line', and copies 'line' to 'line2'
5) Prints 'line2', which holds the new line now.
