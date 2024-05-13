/**
 * @file generator.c
 * @brief  repeatedly generates a random solution for arc set 
 * and writes its result to the circular buffer. It repeats this
 * procedure until it is notified by the supervisor to terminate.
 * @author Vorobeva Aksinia 12044614
 * @date 11.12.2023
 */
#include "common.h"
#include <time.h>

/**
 * @struct list_of_vertices_t
 * @brief Represents a list of vertices.
 * @details This structure includes an array of integers representing vertices and the size of the list.
 */
typedef struct{
	int list[DEFAULT_EDGES_AMOUNT];
	int size;
} list_of_vertices_t;

char *prog_name;
list_of_edges_t list_of_edges;
list_of_vertices_t list_of_vertices;
list_of_edges_t solution;

/**
 * @brief Parses a string into an integer.
 *
 * @details This function converts the input string to an integer using the strtol function.
 * It also performs error checking to ensure that the conversion is successful and the integer
 * value is within the valid range.
 *
 * @param str The input string to be converted.
 * @return The integer value parsed from the string.
 * @throws Exits the program with an error message if the string cannot be represented as an integer
 * or if the parsed value exceeds the maximum allowed integer value.
 */

int parseStringToInteger(char *str){
	char *ptr;
	long int ret = strtol(str, &ptr, 10);
	if(*ptr != '\0' || ret > INT_MAX){
		printErrorAndExit(prog_name, "string cannot represented as integer");
	}
	return (int) ret;
}

/**
 * @brief Validates and parses an edge represented by a string.
 *
 * @details This function takes an input string representing an edge in the format "start-end".
 * It validates the format, extracts the start and end vertices, and adds the edge to the global
 * list_of_edges.
 *
 * @param input The input string representing the edge.
 *
 */
void validateEdge(char *input){
	char *separator = strchr(input, '-');
	if(separator == NULL)
		printErrorAndExit(prog_name, "edge is invalid");
	char *part1;
	part1 = (char *)malloc(separator - input + 1);
	if(part1 == NULL)
		printErrorAndExit(prog_name, "split edge string is failed (part1)");
	strncpy(part1, input, separator - input);
	part1[separator - input] = '\0';
	
	char* part2 = strdup(separator + 1);
	if(part2 == NULL){
		free(part1);
		printErrorAndExit(prog_name, "split edge string is failed (part2)");
	}
	
	edge_t edge;
	edge.start = parseStringToInteger(part1);
	edge.end = parseStringToInteger(part2);
	list_of_edges.list[list_of_edges.size] = edge;
	list_of_edges.size++;
	free(part1);
	free(part2);
}

/**
 * @brief Validates and sets the list of edges based on command-line arguments.
 *
 * @details This function takes the command-line arguments and validates the number of edges and
 * their format. It then calls the validateEdge function for each edge to build the list_of_edges.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 */
void validateAndSetListOfEdges(int argc, char *argv[]){
	if(argc == 1)
		printErrorAndExit(prog_name, "requires list of edges");
	if((argc - 1) > DEFAULT_EDGES_AMOUNT)
		printErrorAndExit(prog_name, "the programm cannot process so many edges of the graph");
	list_of_edges.size = 0;
	for(int i = 1; i < argc; i++){
		validateEdge(argv[i]);
	}
}

/**
 * @brief Finds the index of an element in the list_of_vertices.
 *
 * @details This function searches for the specified element in the list_of_vertices.list.
 * If found, it returns the index of the element; otherwise, it returns -1.
 *
 * @param elem The element to search for in the list_of_vertices.
 * @return The index of the element in the list_of_vertices.list, or -1 if not found.
 *
 */
int indexOfElementInVertices(int elem){
	int size = list_of_vertices.size;
	for(int i = 0; i < size; i++){
		if(elem == list_of_vertices.list[i])
			return i;
	}
	return -1;
}

/**
 * @brief Adds an element to the list_of_vertices if it does not already exist.
 *
 * @details This function checks if the specified element already exists in the list_of_vertices.
 * If not, it adds the element to the list.
 *
 * @param elem The element to add to the list_of_vertices.
 *
 *  
 */
void addElemToListOfVertices(int elem){
	if(indexOfElementInVertices(elem) == -1){
		list_of_vertices.list[list_of_vertices.size] = elem;
		list_of_vertices.size++;
	}
}

/**
 * @brief Creates a list of vertices from the list_of_edges.
 *
 * @details This function initializes the list_of_vertices and populates it with unique vertices
 * from the list_of_edges. It calls addElemToListOfVertices for each start and end vertex in the edges.
 *
 */
void createListOfVertices(void){
	list_of_vertices.size = 0;
	int size = list_of_edges.size;
	for(int i = 0; i < size; i++){
		addElemToListOfVertices(list_of_edges.list[i].start);
		addElemToListOfVertices(list_of_edges.list[i].end);
	}
}

/**
 * @brief Generates a random integer within a specified range.
 *
 * @details This function generates a random integer between the given lower and upper bounds (inclusive).
 *
 * @param lower The lower bound of the random number range.
 * @param upper The upper bound of the random number range.
 * @return A random integer within the specified range.
 */
int generateRandomNumber(int lower, int upper){
	return rand() % (upper - lower + 1) + lower;
}

/**
 * @brief Generates a random list of vertices by shuffling the existing list_of_vertices.
 *
 * @details This function initializes a new list_of_vertices, shuffles the existing list_of_vertices,
 * and then copies the shuffled elements to the new list.
 *.
 */
void generateRandomListOfVertices(void){
	list_of_vertices_t list_of_vertices2;
	list_of_vertices2.size = 0;
	
	while(list_of_vertices.size != 0){
		int randomNumber = generateRandomNumber(0, list_of_vertices.size - 1);
		list_of_vertices2.list[list_of_vertices2.size] = list_of_vertices.list[randomNumber];
		list_of_vertices.list[randomNumber] = list_of_vertices.list[list_of_vertices.size - 1];
		list_of_vertices2.size++;
		list_of_vertices.size--;
	}	
	list_of_vertices = list_of_vertices2;
}

/**
 * @brief Prints the list of edges to the standard output.
 *
 * @details This function prints each edge in the provided list_of_edges in the format "start-end".
 *
 * @param edges The list_of_edges to be printed.
 */
void printListOfEdges(list_of_edges_t edges){
	int size = edges.size;
	for(int i = 0; i < size; i++){
		printf("%d-%d ", edges.list[i].start, edges.list[i].end);
	}
	printf("\n");
}


/**
 * @brief Prints the list of vertices to the standard output.
 *
 * @details This function prints each vertex in the global list_of_vertices to the standard output.
 */
void printVertices(void){
	int size = list_of_vertices.size;
	for(int i = 0; i < size; i++)
		printf("%d ", list_of_vertices.list[i]);
	printf("\n");
}

/**
 * @brief Generates a solution by selecting edges where the start vertex has a higher index than the end vertex.
 *
 * @details This function initializes the global solution list_of_edges by iterating through the list_of_edges.
 * For each edge, if the index of the start vertex is greater than the index of the end vertex in the
 *  list_of_vertices, the edge is added to the solution.
 *      
 */
list_of_edges_t generateSolution(void){
	generateRandomListOfVertices();
	list_of_edges_t solution_new;
	int size = list_of_edges.size;
	edge_t edge;
	solution_new.size = 0;
	for(int i = 0; i < size; i++){
		edge = list_of_edges.list[i];
		if(indexOfElementInVertices(edge.start) > indexOfElementInVertices(edge.end)){
			solution_new.list[solution_new.size] = edge;
			solution_new.size++;
		}
	}
	//printVertices();
	//printListOfEdges(solution_new);
	return solution_new;
	
}

/**
 * @brief Writes the generated solution to a shared memory buffer.
 *
 * @details This function generates a solution using the generateSolution function and then writes
 * the solution to the shared memory buffer specified by the provided myshm structure. The write_index
 * is updated accordingly.
 *
 * @param myshm A pointer to the myshm_t structure representing the shared memory.
 *
 */
void writeSolution(myshm_t *myshm){
	myshm->buffer[myshm->write_index] = generateSolution();
	myshm->write_index = (myshm->write_index + 1) % BUFFER_SIZE;
}

/**
 *  * @brief The main function of the program.
 *   *
 *    * @details This function serves as the entry point of the program. It performs the following steps:
 * 1. Sets the global variable `prog_name` to the program's name.
 * 2. Validates and sets the list_of_edges based on command-line arguments.
 * 3. Creates a list_of_vertices from the list_of_edges.
 * 4. Generates a random list_of_vertices.
 * 5. Opens a shared memory object and maps it to the process's address space using mmap.
 * 6. Opens semaphores for synchronization.
 * 7. Enters a loop to write solutions to the shared memory until the stop flag is set.
 * 8. Closes and unlinks semaphores, unmaps shared memory, and closes the shared memory descriptor.
 *             
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @return The exit status of the program.
 */
int main(int argc, char *argv[]){
	
	prog_name = argv[0];

	srand(time(NULL));
	
	validateAndSetListOfEdges(argc, argv);

	createListOfVertices();

	int fd = shm_open(SHM_NAME, O_RDWR, 0);
	if(fd == -1)
		printErrorAndExit(prog_name, "shm_open is failed");

	myshm_t *myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(myshm == MAP_FAILED)
		printErrorAndExit(prog_name, "mmap is failed");
	

	sem_t *free_sem = sem_open("/free_sem", 0);
	if(free_sem == SEM_FAILED)
		 printErrorAndExit(prog_name, "sem_open is failed");
	sem_t *used_sem = sem_open("/used_sem", 0);
	if(used_sem == SEM_FAILED)
		printErrorAndExit(prog_name, "sem_open is failed");
	while(!myshm->stop){
		if(sem_wait(free_sem) == -1)
		printErrorAndExit(prog_name, "sem_wait is failed");
		if(myshm->stop)
			break;
		writeSolution(myshm);

		if(sem_post(used_sem) == -1)
			printErrorAndExit(prog_name, "sem_post failed");
	}

	if(sem_close(free_sem) == -1)
		                printErrorAndExit(prog_name, "sem_close is failed");
	if(sem_close(used_sem) == -1)
		printErrorAndExit(prog_name, "sem_close is failed");

	sem_unlink("/free_sem");
	sem_unlink("/used_sem");
	
	if(munmap(myshm, sizeof(*myshm)) == -1)
		printErrorAndExit(prog_name, "munmap is failed");
	if(close(fd) == -1)
		printErrorAndExit(prog_name, "close of fd is failed");	

	return EXIT_SUCCESS;
}
