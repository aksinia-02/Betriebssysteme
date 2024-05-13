/*
 * @file forksort.c
 * @brief merge sort lines alphabetically using parallel processes
 * @author Vorobeva Aksinia 12044614
 * @date 11.11.2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Structure representing a child process with associated file descriptors and a FILE pointer.
 * 
 * @details This structure holds information about a child process, including a unique identifier, input and output
 * pipe file descriptors, and a FILE pointer for additional file I/O operations.
 */
typedef struct {
	int id;		///< Unique identifier for the child process.
	int fd_in[2];	///< Array containing input pipe file descriptors (read: fd_in[0], write: fd_in[1]).
	int fd_out[2];	///< Array containing output pipe file descriptors (read: fd_out[0], write: fd_out[1]).
	FILE *file;	///< Pointer to a FILE structure for additional file I/O operations.
} child_t;

/**
 * @brief Global variable to store the program name.
 * 
 * @details This variable is used to store the name of the program and is typically set using the value of argv[0].
 */
char *prog_name;

/**
 * @brief Prints an error message to stderr and exits the program with a failure status.
 * 
 * @details This function takes an error message as input, combines it with the program name and the corresponding
 * system error message, and prints the formatted message to the standard error stream (stderr). It then
 * exits the program with a failure status.
 * 
 * @param message The error message to be printed.
 */
void printMessageAndExit(char *message){
	fprintf(stderr, "%s: %s: %s\n", prog_name, message, strerror(errno));
	exit(EXIT_FAILURE);	
}

/**
 * @brief Create a child process with input and output pipes.
 * 
 * @details This function creates a child process, establishes input and output pipes for communication with the child,
 * and performs necessary redirections of standard input and output. It also closes unnecessary pipe ends in both
 * the parent and child processes.
 * 
 * @param child Pointer to a child_t structure representing the child process.
 */
void makeChildProcess(child_t *child){
	// Attempt to create a pipe for the child process's input.
        if(pipe(child->fd_in) == -1){
		printMessageAndExit("An error occurred with opening the pipe");
	}
	// Attempt to create a pipe for the child process's output.
	if(pipe(child->fd_out) == -1){
		printMessageAndExit("An error occurred with opening the pipe");
	}

	// Create a new process using fork.
	child->id = fork();
	// Check if fork was successful.
	if(child->id == -1){
		printMessageAndExit("Fork failed");
	}
	//child process
	if(child->id == 0){

		//close the ends of pipes that is not used by the child
		//close end of fd_in for writing
		if(close(child->fd_in[1])){
			printMessageAndExit("An error occurred with close");
		}
		//close end of fd_out for reading
		if(close(child->fd_out[0])){
			printMessageAndExit("An error occurred with close");
		}

		//redirect the standard input of the child process to the read end of fd_in
		//all input for the child will be read from this pipe
		if(dup2(child->fd_in[0], STDIN_FILENO) == -1){
			printMessageAndExit("An error occurred with dup2");
		}

		//redirect the standard input of the child process to the write end of the fd_out 
		if(dup2(child->fd_out[1], STDOUT_FILENO) == -1){
			printMessageAndExit("An error occurred with dup2");
		}

		//close unnecessary ends
		//close end of fd_in for reading
		if(close(child->fd_in[0])){
			printMessageAndExit("An error occurred with close");
		}
		//close end of fd_out for writing
		if(close(child->fd_out[1])){
			printMessageAndExit("An error occurred with close");
		}

		// Attempt to replace the current process with the forksort executable.					                 
		if (execlp("./forksort", "forksort", NULL) == -1) {
			printMessageAndExit("An error occurred with execlp");
		}
	}
	else{
		//close read end in fd_in of parent
		if(close(child->fd_in[0])){
			printMessageAndExit("An error occurred with close");
		}
		//close write end in fd_out of parent
		if(close(child->fd_out[1])){
			printMessageAndExit("An error occurred with close");
		}
	}
}

/**
 * @brief Open a file stream associated with the write end of the child's input pipe.
 * 
 * @details This function attempts to open a file stream associated with the write end of the child's input pipe.
 * It uses the fdopen function to create a FILE stream, which can then be used for writing to the child's input pipe.
 * 
 * @param child Pointer to a child_t structure representing the child process.
 */
void openChildFileToWrite(child_t *child){
	// Attempt to open a file stream associated with the write end of the child's input pipe.
	if(((child->file) = fdopen(child->fd_in[1], "w")) == NULL){
		printMessageAndExit("An error occurred with opening file to writing (Child)");
	}
}

/**
 * @brief Open a file stream associated with the read end of the child's output pipe.
 * 
 * @details This function attempts to open a file stream associated with the read end of the child's output pipe.
 * It uses the fdopen function to create a FILE stream, which can then be used for reading from the child's output pipe.
 * 
 * @param child Pointer to a child_t structure representing the child process.
 */
void openChildFileToRead(child_t *child){
	// Attempt to open a file stream associated with the read end of the child's output pipe.
	if((child->file = fdopen(child->fd_out[0], "r")) == NULL){
		printMessageAndExit("An error occurred with opening file to writing (Child)");
	}
}

/**
 * @brief Split lines from standard input into two parts and write them to child processes.
 * 
 * @details This function reads lines from standard input using the getline function and alternately writes them
 * to two different child processes. Odd-numbered lines are written to child1, and even-numbered lines are
 * written to child2.
 * 
 * @param child1 Pointer to a child_t structure representing the first child process.
 * @param child2 Pointer to a child_t structure representing the second child process.
 */
void splitLinesInTwoParts(child_t *child1, child_t *child2){
	char *line =NULL;
	size_t line_buf_size = 0;
	int count = 0;
	while(getline(&line, &line_buf_size, stdin) != -1){
		if(count % 2 == 0)
			fprintf(child1->file, "%s", line);
		else
			fprintf(child2->file, "%s", line);
		count++;
	}
	free(line);
}

/**
 * @brief Merge lines from two child processes and print them to standard output in sorted order.
 * 
 * @details This function reads lines from two child processes and compares them to print the lines to standard output
 * in sorted order. It uses the getline function to read lines from the child processes' files and compares them
 * until both files are fully processed. The merged and sorted lines are printed to the standard output.
 * 
 * @param child1 Pointer to a child_t structure representing the first child process.
 * @param child2 Pointer to a child_t structure representing the second child process.
 */
void mergeLinesFromTwoChildren(child_t *child1, child_t *child2){
	char *line1 = NULL;
	char *line2 = NULL;
	size_t line_buf_size = 0;
	ssize_t read1, read2;
	read1 = getline(&line1, &line_buf_size, child1->file);
	read2 = getline(&line2, &line_buf_size, child2->file);

	// Compare and print lines in sorted order until both files are fully processed.
	while(read1 != -1 && read2 != -1){
		if(strcmp(line1, line2) <= 0){
			fprintf(stdout, "%s", line1);
			read1 = getline(&line1, &line_buf_size, child1->file);
		}
		else{
			fprintf(stdout, "%s", line2);
			read2 = getline(&line2, &line_buf_size, child2->file);
		}
	}
	
	// Print any remaining lines from child1's file.
	while(read1 != -1){
		fprintf(stdout, "%s", line1);
		read1 = getline(&line1, &line_buf_size, child1->file);
	}

	// Print any remaining lines from child2's file.
	while(read2 != -1){
		fprintf(stdout, "%s", line2);
		read2 = getline(&line2, &line_buf_size, child2->file);
	}

	free(line1);
	free(line2);
}

/**
 * @brief Wait for a specific child process to terminate and check its exit status.
 * 
 * @details This function waits for the specified child process to terminate using waitpid. It then checks
 * the exit status of the child process and prints an error message if the child process did not
 * terminate successfully.
 * 
 * @param child Pointer to a child_t structure representing the child process.
 */
void waitForChild(child_t *child){
	int status;
	if(waitpid(child->id, &status, 0) == -1){
		printMessageAndExit("waitpid failed");
	}
	if (WIFEXITED(status)) {
		if(WEXITSTATUS(status)!= EXIT_SUCCESS){
			printMessageAndExit("Child unsuccessfully terminated");
		}
	}
	else{
		printMessageAndExit("Child did not terminate normally");
	}
}

int main(int argc, char *argv[]){	
	if(argc > 1){
		printMessageAndExit("Arguments are not allowed");
	}
	
	prog_name = argv[0];	
	char *line1 = NULL;
	char *line2 = NULL;
	size_t line_buf_size = 0;
	ssize_t read1, read2;

	read1 = getline(&line1, &line_buf_size, stdin);
	read2 = getline(&line2, &line_buf_size, stdin);
	
	if(read2 != -1){
		child_t child1, child2;
		makeChildProcess(&child1);
		makeChildProcess(&child2);
		
		openChildFileToWrite(&child1);
		openChildFileToWrite(&child2);
		fprintf(child1.file, "%s", line1);
		fprintf(child2.file, "%s", line2);

		splitLinesInTwoParts(&child1, &child2);

		fclose(child1.file);
		fclose(child2.file);
		
		openChildFileToRead(&child1);
		openChildFileToRead(&child2);
		mergeLinesFromTwoChildren(&child1, &child2);
		fclose(child1.file);
		fclose(child2.file);

		waitForChild(&child1);
		waitForChild(&child2);

	}
	//only one line
	else if(read1 != -1){
		fprintf(stdout, "%s", line1);
	}
	
	//fflush(stdout);
	free(line1);
	free(line2);
	exit(EXIT_SUCCESS);
}
