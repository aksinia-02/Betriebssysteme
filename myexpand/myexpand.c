/*
 * @file myexpand.c
 * @brief reads several files and replaces tabs with spaces
 * @author Vorobeva Aksinia 12044614
 * @date 29.10.2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#define DEFAULT_TABSTOP 8
#define MAX_TABSTOP 64

/**
 * @brief Replace tabs with spaces in a text file.
 *
 * @details This function reads characters from the input file stream and writes them to
 * the output file stream, replacing tab characters ('\t') with the appropriate
 * number of spaces based on the specified tabstop.
 *
 * @param input  Pointer to the input file stream.
 * @param output Pointer to the output file stream.
 * @param tabstop Number of spaces equivalent to a tab character.
 */
void replaceTabsWithSpaces(FILE *input, FILE *output, int tabstop){
			int character;					///< Variable to store each character read from the input.
			int position = 0;				///< Variable to track the position in the line.
			while((character = fgetc(input)) != EOF){
				if(character == '\t'){
					int spaces = tabstop - (position % tabstop);
					for(int i = 0; i < spaces; i++){
						fputc(' ', output);
						position++;
					}

				}
				else{
					fputc(character, output);
					position++;
					if(character == '\n')
						position = 0;
				}
			}	
		}

int main(int argc, char  *argv[]){
	int tabstop = DEFAULT_TABSTOP;
	char *outFilename = NULL;
	int opt;

	FILE *output = stdout;

	int count_t = 0;
	int count_o = 0;

	while(((opt = getopt(argc, argv, ":t:o:")) != -1)){
		switch(opt){
			case 't':{
					 char *ptr;
					 //converts a string representation of a number to a long integer
					 long int ret = strtol(optarg, &ptr, 10); 
		
					 if(*ptr != '\0' || ret <= 0 || ret > MAX_TABSTOP){
						 fprintf(stderr, "%s: Tabstop is invalid, negativ or more then %d.\n", argv[0], MAX_TABSTOP);
						 return EXIT_FAILURE;
					 }
					 if(count_t == 0){
					 	tabstop = (int) ret;
						count_t++;
					 }
					 else{
						 fprintf(stderr, "%s: More then one 't'.\n", argv[0]);
						 return EXIT_FAILURE;
					 }
				 break;
				 }
			case 'o':{
					 if(count_o == 0){
					 	outFilename = optarg;
						count_o++;
					 }
					 else{
						fprintf(stderr, "%s: More then one 'o'.\n", argv[0]);
						return EXIT_FAILURE;
					}
					 break;
				 }
			case ':': {
					// Handle case where an option requires an argument.
					fprintf(stderr, "%s: Option -%c requires an argument.\n", argv[0], optopt);
					return EXIT_FAILURE;
				  }
			case '?': {
					// Handle case where an invalid option is encountered.
					fprintf(stderr, "%s: Invalid option '-%c'\n", argv[0], optopt);
					return EXIT_FAILURE;	
				  }
			default:{
				fprintf(stderr, "%s: Unknown error.\n", argv[0]);
				 return EXIT_FAILURE;
				}
		}
	}

	// Open the output file if specified.
	if(outFilename){
		output = fopen(outFilename, "w");
		if(output == NULL){
			printf("%s: Failed when opening file.\n", argv[0]);
			return EXIT_FAILURE;
		}

	}

	// Process input files or standard input if no input files are specified.
	if(optind < argc){
		int i;
		for(i = optind; i < argc; i++){
			FILE *input = fopen(argv[i], "r");
			if(input != NULL){
				replaceTabsWithSpaces(input, output, tabstop);
				fclose(input);		
			}
			else{
				printf("%s: Failed when opening file.\n", argv[0]);
				return EXIT_FAILURE;
			}
		}
	} else {
		// If no input files specified, replace tabs with spaces from standard input.
		replaceTabsWithSpaces(stdin, output, tabstop);
	}

	// Close the output file if it was opened.
	if (outFilename != NULL) {
		    fclose(output);
	}


	return EXIT_SUCCESS;
}
