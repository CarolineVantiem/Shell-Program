/*
    UMBC CMSC 421
    Spring 2021
    Project 1

    Due Date: 2/28/21 11:59:00 pm

    Author Name: Caroline Vantiem
    Author email: cvantie1@umbc.edu
    Description: a simple linux shell designed to perform basic linux commands
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include "utils.h"

// CONSTANTS
#define FIRST_ARG 0
#define SECOND_ARG 1

#define EXIT_PASS 0
#define EXIT_FAIL 1

#define TRUE 0
#define FALSE 1

#define EXIT_CODE 255

/*
    In this project, you are going to implement a number of functions to
    create a simple linux shell interface to perform basic linux commands.
    Please note that the below set of functions may be modified as you see fit.
    We are just giving the following as a suggestion. If you do use our
    suggested design, you *will* need to modify the function signatures (adding
    arguments and return types).
*/

// DEFINE FUNCTION PROTOTYPES
void user_prompt_loop();
char *get_user_command();
char **parse_command(char *input);
void execute_command();

void proc_command(char *input, char **args);
void exit_command(char *input, char **args);
void free_memory(char *input, char **args);

int main(int argc, char *argv[])
{
	// check if command line arguments were passed
	if (argc > 1) {
		fprintf(stderr,
			"Error. Command line arguments are not accepted.\n");
		exit(1);
		return (1);
	}

	// call main loop
	else {
		user_prompt_loop();
	}

	return 0;
}

/*
    user_prompt_loop():
    Get the user input using a loop until the user exits, prompting the user for
    a command. Gets command and sends it to a parser, then compares the first
    element to the two built-in commands ("proc", and "exit"). If it's none of
    the built-in commands, send it to the execute_command() function. If the
    user decides to exit, then exit 0 or exit with the user given value.
*/
void user_prompt_loop()
{
	// user input
	char *input = NULL;
	input = get_user_command();

	// parse user input into args
	char **args = NULL;
	args = parse_command(input);

	// loop until user exits
	while (!(strcmp(args[FIRST_ARG], "exit") == TRUE)) {
		// proc command
		if (strcmp(args[FIRST_ARG], "proc") == TRUE) {
			proc_command(input, args);
			free_memory(input, args);
		}

		// exit command
		else if (strcmp(args[FIRST_ARG], "exit") == TRUE) {
			exit_command(input, args);
			free_memory(input, args);
		}

		// execute other commands
		else {
			execute_command(input, args);
			free_memory(input, args);
		}

		user_prompt_loop();
	}

	// exit
	if (strcmp(args[FIRST_ARG], "exit") == TRUE) {
		exit_command(input, args);
	}
}

/*
    get_user_command():
    Take input of arbitrary size from the user and return to the
    user_prompt_loop()
*/
char *get_user_command()
{
	// allocate memory for user input
	char *input = NULL;
	input = (char *)malloc(sizeof(char));
	size_t size = sizeof(char);

	// error allocating memory
	if (input == NULL) {
		printf("Unable to allocate memory. \n");
		free(input);
		user_prompt_loop();
	}

	// get user input
	else {
		printf("$ ");
		getline(&input, &size, stdin);

		// if nothing was entered - reprompt
		if (input[0] == '\n') {
			free(input);
			user_prompt_loop();
		}
	}

	return (input);
}

/*
    parse_command():
    Take command input read from the user and parse appropriately.
*/
char **parse_command(char *input)
{
	int len = strlen(input);

	// copy user input
	char *copy = NULL;
	copy = (char *)malloc(sizeof(char *) * (len + 1));
	strncpy(copy, input, len);
	copy[len] = '\0';

	int new_len = strlen(copy);

	// allocate memory
	char **args = NULL;
	args = malloc(sizeof(char *) * (new_len + 1));

	// get arg num for looping
	int counter = 0;
	char *end = copy + new_len;
	char *start = copy;

	while (start < end) {
		int first = first_unquoted_space(start);
		if (first == -1) {
			counter++;
			start = end;
		} else {
			counter++;
			start += (first + 1);
		}
	}

	// put user input into args[]
	int i = 0;
	int index_args = 0;
	char *starting = copy;

	// populate args from user input
	while (i < counter) {
		int first_un = first_unquoted_space(starting);

		if (first_un == -1) {
			int len_starting = strlen(starting);
			args[index_args] =
				(char *)calloc(len_starting + 1, sizeof(char));
			strncpy(args[index_args], starting, len_starting);
			index_args++;
			i = counter;
		} else {
			args[index_args] =
				(char *)calloc(first_un + 1, sizeof(char));
			strncpy(args[index_args], starting, first_un);
			index_args++;
			i++;
			starting += (first_un + 1);
		}
	}

	args[i] = NULL;

	// call unescape after parse
	int index = 0;
	while (args[index] != NULL) {
		// temp to store args[index]
		int len = strlen(args[index]);
		char *temp = NULL;
		temp = (char *)calloc(len + 1, sizeof(char));
		strncpy(temp, args[index], len);
		temp[len] = '\0';

		char *unescaped = unescape(temp, stderr);

		// bad user input - reprompt
		if (unescaped == NULL) {
			free(temp);
			free(unescaped);
			free(copy);
			free_memory(input, args);
			user_prompt_loop();
		}

		int unescape_len = strlen(unescaped);

		// reallocate and copy unescaped string
		args[index] = (char *)realloc(
			args[index], (sizeof(char) * unescape_len + 1));
		strncpy(args[index], unescaped, unescape_len + 1);
		index++;

		// free allocated memory
		free(unescaped);
		free(temp);
	}

	// free allocated memory
	free(copy);

	return (args);
}

/*
    execute_command():
    Execute the parsed command if the commands are neither proc nor exit;
    fork a process and execute the parsed command inside the child process
*/
void execute_command(char *input, char **args)
{
	// vals for forking
	pid_t pid;
	int status;

	pid = fork();

	// unable to fork
	if (pid == -1) {
		fprintf(stderr, "Unable to fork");
		free_memory(input, args);
		user_prompt_loop();
	}

	// create child process
	else if (pid == 0) {
		execvp(args[FIRST_ARG], args);
		int errno;
		if (errno == 2) {
			free_memory(input, args);
			user_prompt_loop();
		}
	}

	// parent process
	else {
		// wait for process to end
		if (waitpid(pid, &status, 0) > 0) {
			// program executed
			if (WIFEXITED(status) && !WEXITSTATUS(status)) {
				printf("\n");
			}

			// execution failed
			else if (WIFEXITED(status) && WEXITSTATUS(status)) {
				if (WEXITSTATUS(status) == 127) {
					exit(EXIT_FAILURE);
				}
			}
		}
	}
}

/*
    proc_command():
    Execute the parsed command if the first command is proc.
*/
void proc_command(char *input, char **args)
{
	char *unescaped = unescape(input, stderr);
	int len = strlen(unescaped);

	// copy input to concatenate
	char *string = (char *)calloc(len + 2, sizeof(char));
	strcat(string, "/");
	int index = 0;

	// concatenate args
	while (args[index] != NULL) {
		// concatenate extra slash
		if (strcmp(args[index], "proc") == 0) {
			strcat(string, args[index]);
			strcat(string, "/");
			index++;
		}

		strcat(string, args[index]);
		index++;
	}

	string[len] = '\0';

	// open file
	FILE *fp = fopen(string, "r");
	char c;

	// invalid path
	if (fp == NULL) {
		perror("Error: ");
		free_memory(input, args);
		free(unescaped);
		free(string);
		user_prompt_loop();
	}

	// open path and display information
	c = fgetc(fp);
	while (c != EOF) {
		putchar(c);
		c = fgetc(fp);
	}
	printf("\n");
	fclose(fp);

	// free allocated memory
	free(unescaped);
	free(string);
}

/*
    exit_command():
    Execute the exit function if the first command is exit.
    Exit or reprompt based on the users input.
*/
void exit_command(char *input, char **args)
{
	int index = 0;
	int counter = 0;

	// get num of args passed to exit
	while (args[index] != NULL) {
		counter++;
		index++;
	}

	// zero args - exit
	if (counter == 1) {
		free_memory(input, args);
		exit(0);
	}

	// exit with user val
	else if (counter == 2) {
		int digit = atoi(args[SECOND_ARG]);

		// check if second arg digit
		int len = strlen(args[SECOND_ARG]);
		char *temp = (char *)calloc(len + 1, sizeof(char));
		strncpy(temp, args[SECOND_ARG], len);
		temp[len] = '\0';

		int i = 0;
		int not_digit = 0;
		while (i < len) {
			if (!(isdigit(temp[i]))) {
				not_digit++;
				i++;
			} else {
				i++;
			}
		}

		// free allocated memory
		free(temp);

		// cant parse exit code
		if (not_digit > 0) {
			printf("Unable to exit. \n");
			free_memory(input, args);
			user_prompt_loop();
		}

		// exit with user code
		else if (digit < EXIT_CODE) {
			free_memory(input, args);
			exit(digit);
		}

		// exit code
		else if (digit > EXIT_CODE) {
			int new_code = digit % EXIT_CODE;
			free_memory(input, args);
			exit(new_code);
		}

	}

	// more than 2 args
	else {
		printf("Unable to exit. \n");
		free_memory(input, args);
		user_prompt_loop();
	}
}

/*
    free_memory():
    Free all allocated memory when needed.
    Call this function for easability when freeing memory.
*/
void free_memory(char *input, char **args)
{
	// free args array
	int index = 0;
	while (args[index] != NULL) {
		free(args[index]);
		index++;
	}

	// free remaining memory
	free(args);
	free(input);
}
