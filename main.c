#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> 
#include <fcntl.h> 
#include <signal.h>

#define TRUE          1
#define MAX_COMMANDS 20
#define MAX_ARGS     20

// *** GLOBAL VARIABLES ***
int numberOfCommands;
char* commands[MAX_COMMANDS] = { NULL };
int background = 0;

char* commandList[] = {};
// ************

int fileInput(char* command[], char* inputFile) {

    pid_t pid;
    int file;
    int err;

    pid = fork();

    if (pid == 0) {
        file = open(inputFile, O_RDONLY, 0600);
        dup2(file, STDIN_FILENO);
        close(file);

        if (strcmp(command[0], "increment") == 0) {
            char* args[] = { "increment", inputFile, NULL };
            err = execvp("./increment", args);
            return -1;
        }
        else {
            err = execvp(command[0], command);
            return -1;
        }

        if (err == -1) {
            printf("Error::error 404");
            return 2;
        }
    }
    else {
        int wstatus;

        wait(&wstatus);
        if (WIFEXITED(wstatus)) {
            int statusCode = WEXITSTATUS(wstatus);
            if (statusCode == 0) {
            }
            else {
                printf("Error::Failure with status code %d", statusCode);
            }
        }
    }
}

int fileOutput(char* command[], char* outputFile) {

    pid_t pid;
    int file;

    pid = fork();

    if (pid == 0) {
        file = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
        dup2(file, STDOUT_FILENO);
        close(file);

        int err = execvp(command[0], command);
        if (err == -1) {
            printf("Error::error 404");
            return 2;
        }
    }
    else {
        int wstatus;

        wait(&wstatus);
        if (WIFEXITED(wstatus)) {
            int statusCode = WEXITSTATUS(wstatus);
            if (statusCode == 0) {
                printf("Success!\n");
            }
            else {
                printf("Error::Failure with status code %d", statusCode);
            }
        }
    }
}