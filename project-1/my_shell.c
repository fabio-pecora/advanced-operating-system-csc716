#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>

#define MAX_LINE 80       /* The maximum length command */
#define HISTORY_COUNT 10  /* Number of commands to store in history */

char history[HISTORY_COUNT][MAX_LINE];  // Circular buffer for command history
int history_count = 0;

void add_to_history(char *command) {
    static int index = 0;  // Keeps track of where to insert the next command
    strcpy(history[index % HISTORY_COUNT], command);
    index++;
    if (history_count < HISTORY_COUNT) {
        history_count++;
    }
}


void print_history() {
    int start = (history_count > HISTORY_COUNT) ? history_count - HISTORY_COUNT : 0;
    for (int i = start; i < history_count; i++) {
        printf("%d. %s\n", i + 1, history[i % HISTORY_COUNT]);
    }
}


int main(void) {
    char *args[MAX_LINE / 2 + 1];  /* Command line arguments */
    char inputBuffer[MAX_LINE];    /* Buffer to hold the command */
    int should_run = 1;            /* Flag to determine when to exit the program */

    while (should_run) {
        printf("osh> ");
        fflush(stdout);

        /* Step 1: Read user input */
        ssize_t bytesRead = read(STDIN_FILENO, inputBuffer, MAX_LINE - 1);
        if (bytesRead < 0) {
            perror("read failed");
            continue;
        }
        inputBuffer[bytesRead] = '\0';  // Add null terminator
        inputBuffer[strcspn(inputBuffer, "\n")] = 0;  // Remove newline character

        /* Step 2: Check if the user wants to exit */
        if (strcmp(inputBuffer, "exit") == 0) {
            should_run = 0;
            continue;
        }

        /* Step 3: Handle the history command */
        if (strcmp(inputBuffer, "history") == 0) {
            print_history();
            continue;
        } else if (strcmp(inputBuffer, "!!") == 0) {
            if (history_count == 0) {
                printf("No commands in history.\n");
                continue;
            }
            strcpy(inputBuffer, history[(history_count - 1) % HISTORY_COUNT]);
            printf("%s\n", inputBuffer);  // Echo the command
        } else if (inputBuffer[0] == '!' && isdigit(inputBuffer[1])) {
            int command_number = atoi(&inputBuffer[1]);
            if (command_number < 1 || command_number > history_count) {
                printf("No such command in history.\n");
                continue;
            }
            strcpy(inputBuffer, history[(command_number - 1) % HISTORY_COUNT]);
            printf("%s\n", inputBuffer);  // Echo the command
        }

        /* Step 4: Add the command to history */
        add_to_history(inputBuffer);

        /* Step 5: Parse the input into tokens */
        int i = 0;
        char *token = strtok(inputBuffer, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;  // Null-terminate the args array

        /* Step 6: Fork a child process to execute the command */
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } else if (pid == 0) {  // Child process
            if (execvp(args[0], args) == -1) {
                perror("execvp failed");
            }
            exit(1);
        } else {  // Parent process
            wait(NULL);  // Wait for the child process to finish
        }
    }

    return 0;
}

