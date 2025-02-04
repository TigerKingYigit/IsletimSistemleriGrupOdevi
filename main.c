#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> 
#include <fcntl.h> 
#include <signal.h>

void handle_child_termination(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("[%d] retval: %d\n", pid, WEXITSTATUS(status));
    }
}

void executeSingleCommand() {
    char * binaryPath = NULL;
    char * args[MAX_ARGS] = { NULL };
    
    // Take main command and append it to '/bin/' path
    binaryPath = malloc(6 + strlen(commands[0]));
    strcpy(binaryPath, "/bin/");
    strcat(binaryPath, commands[0]);

    // Check for arguments in entered commands
    int i;
    for(i = 0; i < numberOfCommands; i++)
    {   
        args[i] = strdup(commands[i]);
    }

    execv(binaryPath, args);
    
    // If it reaches this far, means that execv returned, so the command was not excecuted.
    puts("Error: Unknown command.");
    // The process kills itself, since execv does not get executed.
    kill(getpid(), SIGINT);
}


int pipeFonk(char* command[], char* command2[], int a){

    int fd[1][2];
    int err;
    int i;
    
    for(i = 0; i < 3; i++){
        if(pipe(fd[i]) < 0){
            return 1;
        }
    }

    int pid1 = fork();

    if(pid1 == 0){
        close(fd[0][0]);
        close(fd[1][0]);
        close(fd[1][1]);
        
        dup2(fd[0][1], STDOUT_FILENO);
        
        close(fd[0][1]);

        err = execvp(command[0], command);
    }
    
    int pid2 = fork();

    if(pid2 == 0){
        close(fd[0][1]);
        close(fd[1][0]);
        close(fd[1][0]);

        dup2(fd[0][0], STDIN_FILENO);
        
        close(fd[0][0]);

        if(strcmp(command2[a],"increment") == 0){
            char *args[] = {"increment", command[1], NULL};
            err = execvp("./increment", args);
        }
        else{
            char* args[] = {command2[a], command2[a+1], NULL};
            err = execvp(command2[a], args);
        }

        return 0;
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}

int fileInput(char* command[], char* inputFile){
    
    pid_t pid;
    int file;
    int err;

    pid = fork();

    if(pid == 0){
        file = open(inputFile, O_RDONLY, 0600);
        dup2(file, STDIN_FILENO);
        close(file);
        
        if(strcmp(command[0],"increment") == 0){
            char *args[] = {"increment", inputFile, NULL};
            err = execvp("./increment", args);
            return -1;
        }
        else{
            err = execvp(command[0], command);
            return -1;
        }
        
        if(err == -1){
            printf("Error::error 404");
            return 2;
        }
    }
    else{
        int wstatus;

        wait(&wstatus);
        if(WIFEXITED(wstatus)){
            int statusCode = WEXITSTATUS(wstatus);
            if(statusCode == 0){
            }
            else{
                printf("Error::Failure with status code %d", statusCode);
            }
        }
    }
}

int fileOutput(char* command[], char* outputFile){
    
    pid_t pid;
    int file;

    pid = fork();

    if(pid == 0){
        file = open(outputFile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
        dup2(file, STDOUT_FILENO);
        close(file);
        
        int err = execvp(command[0], command);
        if(err == -1){
            printf("Error::error 404");
            return 2;
        }
    }
    else{
        int wstatus;

        wait(&wstatus);
        if(WIFEXITED(wstatus)){
            int statusCode = WEXITSTATUS(wstatus);
            if(statusCode == 0){
                printf("Success!\n");
            }
            else{
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
        // Ebeveyn s�re�
        wait(NULL); // �lk �ocu�un bitmesini bekleyin

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
            wait(NULL); // �kinci �ocu�un bitmesini bekleyin

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

void sig_chld(int signo){
    int status, child_val,chid;
	chid = waitpid(-1, &status, WNOHANG);
	if (chid > 0)
	{
		if (WIFEXITED(status))
	    {
	        child_val = WEXITSTATUS(status);
	        printf("[%d] retval : %d \n",chid, child_val);
	    }
	}
}

int workOnBackground(char **args){
	pid_t pid;
	int status;

	struct sigaction act;
	act.sa_handler = sig_chld;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NOCLDSTOP;

	if (sigaction(SIGCHLD,&act,NULL)<0)
	{
		fprintf(stderr,"sigaction failed\n");
		return 1;
	}

	pid=fork();
	if (pid == 0)
	{
		if (execvp(args[0],args) == -1)
		{
			printf("Komut bulunamadi");
			kill(getpid(),SIGTERM);
            background = 0;
		}
	}
	else
	{
		printf("Proses PID:%d Degeriyle Olusturuldu\n",pid);
	}
	return 1; 
}

int execution(char **args){
	if (background==0)
	{
		pid_t pid;
		int status;
		pid=fork();
		if (pid == 0)
		{
			if (execvp(args[0],args) == -1)
			{
				printf("Komut Bulunamadi");
				kill(getpid(),SIGTERM);
			}
		}
		else
		{
			waitpid(pid,NULL,0);
		}
	}
	else
	{
		workOnBackground(args);
	}
	return 1; 
}

int detectCommand(char* args[]){
    int isPipe = 0;
    int j = 0;
    char* command[100];

    while(args[j] != NULL){
        if(strstr(args[j],";") != 0)
        {
            isPipe = 1;
            break;
        }
        j++;
    }

    j = 0;

    while(args[j] != NULL)
    {
        if((strcmp(args[j],">") == 0) || (strcmp(args[j],"<") == 0) || (strcmp(args[j],"&") == 0) || (strcmp(args[j],"|") == 0))
        {
            break;
        }
        command[j] = args[j];
        j++;
    }

    if(args[j] == NULL){
        if(strcmp(command[0], "quit") == 0){
            int status;
            while (!waitpid(-1,&status,WNOHANG)){}
            return 0;
        }
    }

    command[j] = NULL;

    if(isPipe == 1){
        char* commands[20];
        int r = 0;
        j = 0;
        while(args[j] != NULL){
            if(strstr(args[j], ";") != 0){
                args[j][strlen(args[j]) - 1] = '\0';
                commands[r] = args[j];
                commands[++r] = "+";
            }
            else{
                commands[r] = args[j];
            }
            r++;
            j++;
        }
        
        commands[r] = "+";
        pipeFonkOrder(commands);
        return 1;
    }
    else{
        while(args[j] != NULL && background == 0)
        {
            if(strcmp(args[j], ">") == 0){
                fileOutput(command, args[j+1]);
                return 1;
            }

            if(strcmp(args[j], "<") == 0){
                fileInput(command, args[j+1]);
                return 1;
            }

            if(strcmp(args[j], "|") == 0){
                pipeFonk(command, args, j+1);
                return 1;
            }

            if(strcmp(args[j], "&") == 0){
                background = 1;
                return 1;
            }
        }
    }

    command[j] = NULL;

	execution(command);
    
}

int main(int argc) 
{
    char line[100];
    char* tokens[100];
    int tokenSayisi;

    while(1){
        memset(line, '\0', 100);
        printf("> ");
        fflush(stdout);

        
        if(fgets(line, 100, stdin) == NULL)
        {
            printf("Error::");
            return 0;
        }

        tokens[0] = strtok(line," \n\t");
        tokenSayisi = 1;
        while((tokens[tokenSayisi] = strtok(NULL, " \n\t")) != NULL) {
            tokenSayisi++;
        }

        int err = detectCommand(tokens);

        if(err == 0)
            break;
    }

    return 0;
}