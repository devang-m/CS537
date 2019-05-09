1. HistoryNode and PathNode act as linkedlist elements for the path and history functions. The History LinkedList stores the commands in chronological order

2. getInputs() function splits the input by whitespace using strtok and saves the values in a; returning the number of arguments.

3. printHistory() function prints the last n commands or the entire history; whichever comes first. The print is after the recursive call because the History LinkedList stores the commands in chronological order

4. In main():
	
	1. Initially we check if it is batch mode, and if the file can be opened, we process accordingly, else we let the file be stdin

	2. Store the initial path as '/bin' and then we start processing the lines

	3. Creating a HistioryNode and adding the command to that node

	4. Putting spaces before and after '|' and '<' since getInputs() splits by whitespace.

	5. Getting the inputs by calling getInputs() and storing it in array 'a'

	6. Counting the number of pipes and redirects and returning standard error if there is more than 1 redirect or pipe or a redirect and pipe together

	7. Printing standard error if there is 0 or more than 1 file after redirect

	8. Printing standard error if there is 0 values before or after pipe, else forming storing the command after pipe in array 'b'

	9. Checking for exit command - if more than 1 variable inputted with exit, if yes returning error, else exiting the shell

	10. Checking for cd command - if 0 or more than 1 variable inputted with cd, if yes returning error, else changing into the required directory while checking 'chdir' error.

	11. Checking for history command - and number of variables inputted with it
		a) if more than 1 variable inputted with cd, returning error
		b) if no number inputted then printing the entire history
		c) Checking if the value inputted is a number, if not - returning an error, else printing those number of history commands.

	12. Checking for path command - removing all the previous paths and adding the new paths

	13. Checking for non-built in commands. 
		a) first we check if the command exists in any of the paths or not. If it does, we create a child process.

		b) Within the child process, if it is a redirection, we ensure that the output and the error is sent to the file and not to stdout and stderr by using dup2.
		If it is a pipe, this is the 1st command for the pipe, so we close the read end of the pipe and send output and error to the write end.
		After which we execute execv and if returned, store it in the flag variable.

		c) Within the parent process, we first wait for the previous child process to get over.
		Now, if it is a pipe that means there is another command, so we we check if the command exists in any of the paths or not. If it does, we create another child process.
			i) In this child process, since it needs to take input from the previous process, we close the write end of the pipe and get the input from the read end.
			ii) Back in the parent, we close both the read and writes ends wait for this child process to get over.

		d) We just check if there are any errors to be sent (the command is not in any of the paths/ or the command has not been executed properly etc.) and send those errors. If it is one of the above errors with a redirect, then we send the errors to the file.

	14. We free all the allocated space and make the pointers NULL.

