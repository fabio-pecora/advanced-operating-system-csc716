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

char history[HISTORY_COUNT][MAX_LINE];
int history_count = 0;

void add_to_history(const char *command)
{
    static int index = 0;
    if (strlen(command) == 0)
        return;

    strcpy(history[index % HISTORY_COUNT], command);
    index++;
    history_count++;
}

void handle_SIGINT(int sig)
{
    write(STDOUT_FILENO, "\n>>> Ctrl+C detected! Showing command history:\n", 48);

    char buffer[1024];
    int length = 0;

    int start_index = (history_count > HISTORY_COUNT) ? (history_count - HISTORY_COUNT) : 0;
    for (int i = start_index; i < history_count; i++)
    {
        length += snprintf(buffer + length, sizeof(buffer) - length, "%d. %s\n", i + 1, history[i % HISTORY_COUNT]);
    }
    write(STDOUT_FILENO, buffer, length);

    tcflush(STDIN_FILENO, TCIFLUSH);

    write(STDOUT_FILENO, "fabio :) ", 9);
    fflush(stdout);
}

int main(void)
{
    char *args[MAX_LINE / 2 + 1];
    char inputBuffer[MAX_LINE];
    int should_run = 1;
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
                break;
            clearerr(stdin);
            continue;
        }

        inputBuffer[strcspn(inputBuffer, "\n")] = '\0'; // Remove newline

        if (strlen(inputBuffer) == 0)
            continue;
        if (strcmp(inputBuffer, "exit") == 0)
        {
            printf(">>> Exiting shell...\n");
            should_run = 0;
            continue;
        }
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
        add_to_history(inputBuffer);

        int i = 0;
        char *token = strtok(inputBuffer, " ");
        while (token != NULL)
        {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        if (strcmp(args[0], "history") == 0)
        {
            printf(">>> Last 10 Commands:\n");

            int start_index = (history_count > HISTORY_COUNT) ? (history_count - HISTORY_COUNT) : 0;
            for (int i = start_index; i < history_count; i++)
            {
                printf("%d. %s\n", i + 1, history[i % HISTORY_COUNT]);
            }
            continue;
        }
        int run_in_background = (i > 0 && strcmp(args[i - 1], "&") == 0);
        if (run_in_background)
        {
            args[i - 1] = NULL;
        }
        pid_t pid = fork();
        if (pid < 0)
        {
            perror(">>> Fork failed");
            exit(1);
        }
        else if (pid == 0)
        {
            printf("Executing command: %s\n", args[0]);

            if (execvp(args[0], args) == -1)
            {
                perror(">>> execvp failed");
            }
            exit(1);
        }
        else
        {
            if (!run_in_background)
            {
                wait(NULL);
            }
        }
    }

    return 0;
}
