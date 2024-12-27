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



int pipeFonkOrder(char* command[]){

    int i = 0;
    int numberOfCommands = 0;
    while(command[i] != NULL){
        if(command[i] == "+")
            numberOfCommands++;

        i++;
    }
    pid_t pid1 = fork();

    if(pid1 == 0){

        char* command1[5];
        int r = 0;
        i = 0;
        int isSpecial1 = 0;
        int isSpecial2 = 0;
        int isSpecial3 = 0;
        while(command[i] != "+"){
        
            if(strcmp(command[i], ">") == 0){
                isSpecial1 = 1;
                break;
            }
            if(strcmp(command[i], "<") == 0){
                isSpecial2 = 1;
                break;
            }
            if(strcmp(command[r], "|") == 0){
                isSpecial3 = 1;
                break;
            }
            command1[r] = command[i];

            r++;
            i++;
        }

        command1[r] = NULL;

        if(isSpecial1 == 1){
            int err = fileOutput(command1, command[i+1]);
        }
        else if(isSpecial2 == 1){
            int err = fileInput(command1, command[i+1]);
        }
        else{
            int err = execvp(command1[0], command1);
        }

        exit(0);
    }
    else if (pid1 > 0) {
        // Ebeveyn süreç
        wait(NULL); // İlk çocuğun bitmesini bekleyin

        pid_t pid2 = fork();
        if (pid2 == 0) {
            
            char* command2[5];
            int r = 0;
            i = 0;
            int isSpecial1 = 0;
            int isSpecial2 = 0;
            int isSpecial3 = 0;

            while(command[i] != "+"){
                i++;
            }
            i++;
            
            while(command[i] != "+"){
                if(strcmp(command[i], ">") == 0){
                    isSpecial1 = 1;
                    break;
                }
                if(strcmp(command[i], "<") == 0){
                    isSpecial2 = 1;
                    break;
                }
                if(strcmp(command[r], "|") == 0){
                    isSpecial3 = 1;
                    break;
                }
                
                command2[r] = command[i];

                r++;
                command2[r] = NULL;
                i++;
            }

            if(isSpecial1 == 1){
                fileOutput(command2, command[i+1]);
            }
            else if(isSpecial2 == 1){
                fileInput(command2, command[i+1]);
            }
            else{
                int err = execvp(command2[0], command2);
            }

            exit(0);
        } else if (pid2 > 0) {
            wait(NULL); // İkinci çocuğun bitmesini bekleyin

            if(numberOfCommands == 3){
                char* command3[5];
                int r = 0;
                i = 0;
                int isSpecial1 = 0;
                int isSpecial2 = 0;
                int isSpecial3 = 0;

                while(command[i] != "+"){
                    i++;
                }
                i++;
                while(command[i] != "+"){
                    i++;
                }
                i++;

                while(command[i] != "+"){

                    if(strcmp(command[i], ">") == 0){
                        isSpecial1 = 1;
                        break;
                    }
                    if(strcmp(command[i], "<") == 0){
                        isSpecial2 = 1;
                        break;
                    }
                    if(strcmp(command[r], "|") == 0){
                        isSpecial3 = 1;
                        break;
                    }
                    
                    command3[r] = command[i];

                    r++;
                    command3[r] = NULL;
                    i++;
                }

                if(isSpecial1 == 1){
                    fileOutput(command3, command[i+1]);
                }
                else if(isSpecial2 == 1){
                    fileInput(command3, command[i+1]);
                }
                else{
                    int err = execvp(command3[0], command3);
                }
                
            }

            exit(0);
        } else {
            perror("Second fork failed");
        }
    } else {
        perror("First fork failed");
    }
    return 0;
}