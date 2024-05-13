/*
 * @file supervisor.c
 * @brief  sets up the shared memory and the semaphores and initializes the circular buffer required
 *  for the communication with the generators. It then waits for the generators to write solutions to the
 *   circular buffer.
 * @author Vorobeva Aksinia 12044614
 * @date 11.12.2023
 */

#include "common.h"
#include <signal.h>

#define DEFAULT_LIMIT INT_MAX
#define DEFAULT_DELAY 0

char *prog_name;
volatile sig_atomic_t quit = 0;
int best_solution = INT_MAX;

/**
 * @brief Signal handler function to handle termination signals.
 *
 * @details This function sets the quit flag to 1 upon receiving a termination signal.
 *
 * @param signal The signal number received.
 */
void handle_sign(int signal){
	quit = 1;
}

/**
 * @struct supervisor_t
 * @brief Structure to represent supervisor configuration parameters.
 * @details The structure includes limit and delay parameters.
 */
typedef struct{
	int limit;	
	int delay;
} supervisor_t;

supervisor_t supervisor;


/**
 * @brief Reads an integer from the command-line option argument.
 *
 * @details This function uses the strtol function to convert the command-line option argument to an integer.
 * It performs error checking to ensure the validity of the integer and its range.
 *
 * @return The integer value parsed from the command-line option argument.
 *
 * @throws Exits the program with an error message if the argument is not a valid integer or exceeds INT_MAX.
 *          
 *         
 */
int readIntOptarg(void){
	char *ptr;
	long int ret = strtol(optarg, &ptr, 10);
	if(*ptr != '\0' || ret > INT_MAX)
		printErrorAndExit(prog_name, "limit is invalid");
	return (int) ret;
}

/**
 * @brief Parses command-line arguments to set the supervisor configuration
 *
 * @details This function parses the command-line arguments using getopt to set the values of the supervisor
 * structure based on the provided options. It supports the '-n' and '-w' options to set the limit and delay
 * parameters, respectively. If an option is specified more than once, an error is reported and the program exits.
 * The default values are used if an option is not provided.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @param supervisor A pointer to the supervisor_t structure to store the parsed configuration.
 */
void getArgrumentsSetSupervisor(int argc, char *argv[], supervisor_t *supervisor){
	int opt;
	int count_n = 0;
	int count_w = 0;
	while((opt = getopt(argc, argv, "n:w:p")) != -1){
		switch(opt){
			case 'n':{
				if(count_n == 0){
						count_n++;
						supervisor->limit = readIntOptarg();
				}
				else
					printErrorAndExit(prog_name, "more than one n");
				break;
			}
			case 'w':{
				if(count_w == 0){
					count_w++;
					supervisor->delay = readIntOptarg();
				}
				else
					printErrorAndExit(prog_name, "more than one w");
				break;
			}
			case ':':{
				printErrorAndExit(prog_name, "option requires argument");
				break;
			}
			case 'p':{
				break;
			}
			case '?':{
				printErrorAndExit(prog_name, "option is invalid");
				break;
			}
			default:
				printErrorAndExit(prog_name, "unknown error");
		}
	}

	if(count_n == 0)
		supervisor->limit = INT_MAX;
	if(count_w == 0)
		supervisor->delay = 0;
}

/**
 * @brief Sets up signal actions for handling termination signals (SIGINT and SIGTERM).
 *
 * @details This function configures signal actions for SIGINT and SIGTERM to call the handle_sign function.
 * It uses sigaction to set the corresponding signal handlers.
 */
void setUpSignalAction(void){
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa)); //initialize sa to 0
	sa.sa_handler = handle_sign;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		printErrorAndExit(prog_name, "sigaction with SIGINT is failed");
	if (sigaction(SIGTERM, &sa, NULL) == -1)
		printErrorAndExit(prog_name, "sigaction with SIGTERM is failed");
}

/**
 * @brief Prints the list of edges to the standard output.
 *
 * @details This function prints each edge in the provided list_of_edges to the standard output
 * in the format "start-end ".
 *
 * @param edges The list_of_edges to be printed.
 */
void printListOfEdges(list_of_edges_t edges){
	int size = edges.size;
	for(int i = 0; i < size; i++){
		fprintf(stdout, "%d-%d ", edges.list[i].start, edges.list[i].end);
	}
}

/**
 * @brief Reads and processes a solution from the shared memory buffer.
 *
 * @details This function reads a solution from the shared memory buffer specified by the provided myshm structure.
 * It checks if the limit parameter of the supervisor has been reached and prints the best solution found.
 * The read_index of the shared memory buffer is updated accordingly.
 *
 * @param myshm A pointer to the myshm_t structure representing the shared memory.
 *
 */
void readSolution(myshm_t *myshm){
	if(supervisor.limit == 0){
		fprintf(stdout, "The graph might not be acyclic, best solution removes %d edges.\n", best_solution);
		quit = 1;
		return;
	}
	list_of_edges_t solution = myshm->buffer[myshm->read_index];
	if(solution.size == 0){
		fprintf(stdout, "the graph is acyclic!\n");
		quit = 1;
		return;
	}
	else{
		if(solution.size < best_solution){
			best_solution = solution.size;
			//fprintf(stdout, "Solution with %d edges: ", best_solution);
			//printListOfEdges(solution);
			//fprintf(stdout, "\n");
		}
	}
	supervisor.limit--;
	myshm->read_index = (myshm->read_index + 1) % BUFFER_SIZE;

}


/**
 * @brief The main function of the program.
 *
 * @details This function serves as the entry point of the supervisor program. It performs the following steps:
 * 1. Sets the global variable `prog_name` to the program's name.
 * 2. Parses command-line arguments to set the supervisor configuration using getArgrumentsSetSupervisor.
 * 3. Sets up signal actions for handling termination signals (SIGINT and SIGTERM) using setUpSignalAction.
 * 4. Creates a shared memory object and maps it to the process's address space using shm_open and mmap.
 * 5. Initializes the shared memory structure and semaphores for synchronization.
 * 6. Waits for the specified delay time.
 * 7. Enters a loop to read solutions from the shared memory until the termination signal is received.
 * 8. Closes and unlinks semaphores, unmaps shared memory, unlinks shared memory object, and closes the shared memory descriptor.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @return The exit status of the program.
 */
int main(int argc, char *argv[]){
	prog_name = argv[0];
	
	getArgrumentsSetSupervisor(argc, argv, &supervisor);

	setUpSignalAction();

	int shmfd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
	
	if(shmfd == -1)
		printErrorAndExit(prog_name, "semaphore open is failed");
	if(ftruncate(shmfd, sizeof(myshm_t)) < 0)
		printErrorAndExit(prog_name, "ftruncate is failed");
	myshm_t *myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

	if(myshm == MAP_FAILED)
		printErrorAndExit(prog_name, "mmap is failed");

	myshm->read_index = 0;
	myshm->write_index = 0;
	myshm->stop = false;
	
	sem_t *free_sem = sem_open("/free_sem", O_CREAT, 0600, BUFFER_SIZE);
	if(free_sem == SEM_FAILED){
		sem_unlink("/free_sem");
		        sem_unlink("/used_sem");
		printErrorAndExit(prog_name, "free sem_open is failed");
	}
	sem_t *used_sem = sem_open("/used_sem", O_CREAT, 0600, 0);
	if(used_sem == SEM_FAILED)
		printErrorAndExit(prog_name, "used sem_open is failed");

	sleep(supervisor.delay);

	while(!quit){
		if(sem_wait(used_sem) == -1){
			if(errno == EINTR){
				myshm->stop = true;
				printErrorAndExit(prog_name, "interrupted by a signal");

			}
			printErrorAndExit(prog_name, "sem_wait is failed");
		}
		
		readSolution(myshm);

		if(quit)
			break;
		if(sem_post(free_sem) == -1)
			printErrorAndExit(prog_name, "sem_post is failed");
	}
	myshm->stop = true;

	if(sem_close(free_sem) == -1)
		printErrorAndExit(prog_name, "sem_close is failed");
	if(sem_close(used_sem) == -1)
		printErrorAndExit(prog_name, "sem_close is failed");
	sem_unlink("/free_sem");
	sem_unlink("/used_sem");

	if(munmap(myshm, sizeof(* myshm)) == -1)
		printErrorAndExit(prog_name, "munmap is failed");
	if(shm_unlink(SHM_NAME) == -1)
		printErrorAndExit(prog_name, "shm_unlink is failed");
	if(close(shmfd) == -1)
		printErrorAndExit(prog_name, "close of fd is failed");
	return EXIT_SUCCESS;

	
}
