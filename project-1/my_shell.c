#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <termios.h>

#define MAX_LINE 80
#define HISTORY_COUNT 10

char history[HISTORY_COUNT][MAX_LINE]; // Stores the last 10 commands
int history_count = 0;                 // Number of stored commands

// Add command to history
void add_to_history(const char *command)
{
    static int index = 0;
    if (strlen(command) == 0)
        return; // Don't store empty commands

    strcpy(history[index % HISTORY_COUNT], command);
    index++;
    if (history_count < HISTORY_COUNT)
    {
        history_count++;
    }
}

// Handle `Ctrl+C` (SIGINT) - Print history instead of exiting
void handle_SIGINT(int sig)
{
    write(STDOUT_FILENO, "\n>>> Ctrl+C detected! Showing command history:\n", 48);

    char buffer[1024];
    int length = 0;
    for (int i = 0; i < history_count; i++)
    {
        length += snprintf(buffer + length, sizeof(buffer) - length, "%d. %s\n", i + 1, history[i]);
    }
    write(STDOUT_FILENO, buffer, length);

    // Flush stdin to prevent issues
    tcflush(STDIN_FILENO, TCIFLUSH);

    // Restore shell prompt
    write(STDOUT_FILENO, "fabio :) ", 9);
    fflush(stdout);
}

int main(void)
{
    char *args[MAX_LINE / 2 + 1]; // Command arguments
    char inputBuffer[MAX_LINE];   // User input buffer
    int should_run = 1;

    // Register SIGINT handler using `sigaction()`
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = handle_SIGINT;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sigIntHandler, NULL);

    while (should_run)
    {
        printf("fabio :) ");
        fflush(stdout);

        if (fgets(inputBuffer, MAX_LINE, stdin) == NULL)
        {
            if (feof(stdin))
                break; // Handle Ctrl+D (EOF)
            clearerr(stdin);
            continue;
        }

        inputBuffer[strcspn(inputBuffer, "\n")] = '\0'; // Remove newline

        if (strlen(inputBuffer) == 0)
            continue;

        // Exit shell
        if (strcmp(inputBuffer, "exit") == 0)
        {
            printf(">>> Exiting shell...\n");
            should_run = 0;
            continue;
        }

        // Handle !! (repeat last command)
        if (strcmp(inputBuffer, "!!") == 0)
        {
            if (history_count == 0)
            {
                printf(">>> No commands in history.\n");
                continue;
            }
            strcpy(inputBuffer, history[(history_count - 1) % HISTORY_COUNT]);
            printf(">>> Executing last command: %s\n", inputBuffer);
        }
        // Handle !N (repeat Nth command)
        else if (inputBuffer[0] == '!' && isdigit(inputBuffer[1]))
        {
            int command_number = atoi(&inputBuffer[1]);
            if (command_number < 1 || command_number > history_count)
            {
                printf(">>> No such command in history.\n");
                continue;
            }
            strcpy(inputBuffer, history[(command_number - 1) % HISTORY_COUNT]);
            printf(">>> Executing command %d: %s\n", command_number, inputBuffer);
        }

        // Add command to history
        add_to_history(inputBuffer);

        // Tokenize input for execvp
        int i = 0;
        char *token = strtok(inputBuffer, " ");
        while (token != NULL)
        {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        // Handle the `history` command manually
        if (strcmp(args[0], "history") == 0)
        {
            printf(">>> Command history:\n");
            for (int i = 0; i < history_count; i++)
            {
                printf("%d. %s\n", i + 1, history[i]);
            }
            continue; // Skip execution since we already handled history
        }

        // Check if command runs in background (ends with `&`)
        int run_in_background = (i > 0 && strcmp(args[i - 1], "&") == 0);
        if (run_in_background)
        {
            args[i - 1] = NULL; // Remove `&` from args
        }

        // Fork a child process to execute the command
        pid_t pid = fork();
        if (pid < 0)
        {
            perror(">>> Fork failed");
            exit(1);
        }
        else if (pid == 0) // Child process
        {
            // Custom debug message to confirm my code
            printf(">>> DEBUG <<< Executing command: %s\n", args[0]);
            for (int j = 0; args[j] != NULL; j++)
            {
                printf(">>> DEBUG <<< args[%d] = %s\n", j, args[j]);
            }
            fflush(stdout); // Ensure debug prints before execvp

            if (execvp(args[0], args) == -1)
            {
                perror(">>> execvp failed");
            }
            exit(1);
        }
        else // Parent process
        {
            if (!run_in_background)
            {
                wait(NULL); // Wait for child to complete if not running in background
            }
        }
    }

    return 0;
}
