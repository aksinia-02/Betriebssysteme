/*
 * @file common.h
 * @author Vorobeva Aksinia 12044614
 * @date 11.12.2023
 */
#ifndef COMMON
#define COMMON

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <errno.h>

#define DEFAULT_EDGES_AMOUNT 40
#define BUFFER_SIZE 12

#define SHM_NAME "/myshm"

/**
 * @brief Structure representing an edge between two vertices.
 */
typedef struct{
	int start;
	int end;
} edge_t;

/**
 * @brief Structure representing a list of edges.
 */
typedef struct{
	edge_t list[DEFAULT_EDGES_AMOUNT];
	int size;
} list_of_edges_t;

/**
 * @brief Structure representing shared memory data.
 */
typedef struct{
	list_of_edges_t buffer[BUFFER_SIZE];
	int read_index;
	int write_index;
	bool stop;	
}myshm_t;

/**
 * @brief Macro for printing an error message and exiting the program.
 *
 * @param prog_name The name of the program.
 * @param msg The error message to be printed.
 */
#define printErrorAndExit(prog_name, msg) \
	do{ \
		fprintf(stderr, "%s: %s\n", prog_name, msg); \
		exit(EXIT_FAILURE); \
	}while(0)


#endif
