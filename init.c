#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define R_GREEN   "\033[32m"
#define RESET     "\033[0m"
#define R_YELLOW  "\033[33m"
#define VERSION "v1.4"   

//structure for handling stopped jobs
typedef struct{
    pid_t pid;
    char command[100];
} Job;

int main(){
    printf(R_YELLOW"\n\n\n-------NOVA OS %s-------\n\n\n"RESET, VERSION);
    Job stopped_jobs[10];
    int stopped_jobs_count = 0;
    while(1){

        //checking background processes if any thing has completed 
        pid_t completed_pid;
        int status;

        while((completed_pid = waitpid(-1, &status, WNOHANG)) > 0){
            printf("\nBackground job %d is finished!\n", completed_pid);
        }
        
        printf(R_GREEN"nova"RESET": ");
        
        fflush(stdout);

        //creating a buffer for storing the commands
        char buffer[100];

        if(fgets(buffer, sizeof(buffer),stdin) == NULL){
            break;  //handling ctrl + D case
        }

        if(buffer[0] == '\n'){
            continue;   //handling empty commands
        }

        
        buffer[strcspn(buffer,"\n")] = '\0';//we are making it as a string by adding the null terminator


        //using if-else for checking basic commands directly from the shell 
        if(strcmp(buffer, "version") == 0){
            printf("\n\nNova OS\nVersion - %s\ncompiled: \"musl-gcc\"\ntested on: \"QEMU(virt)\"\n\n", VERSION);
        }
        else if(strcmp(buffer, "help") == 0){
            printf("\nAvailable Commands:\n1. version\n2. help\n3. quit(q)\n\n");
        }
        else if(strcmp(buffer, "q") == 0 || strcmp(buffer,"quit")==0){
            break;
        }
        //handling stopped fg jobs 
        else if(strcmp(buffer, "fg") == 0){
            if(stopped_jobs_count == 0){
                printf("There are no foreground jobs running!\n");
            }else{
                stopped_jobs_count--;
                pid_t pid = stopped_jobs[stopped_jobs_count].pid;

                printf("continuing %s", stopped_jobs[stopped_jobs_count].command);

                kill(pid, SIGCONT);
                waitpid(pid, &status, WUNTRACED);
                if(WIFSTOPPED(status)){
                    stopped_jobs_count++;
                    printf("Process %d Stopped again!\n", pid);
                }
            }
        }
        else{
            char *args[10];
            int i = 0;
            int bg_process = 0;

            //for handling the stopped processes
            char saved_command[100];
            strncpy(saved_command, buffer, sizeof(saved_command)-1);
            strncpy[sizeof(saved_command) - 1] = '\0';

            char* token = strtok(buffer, " ");


            while(token != NULL && i<9){
                    args[i++] = token;
                    token = strtok(NULL, " ");  
                }

            if(i>0 && strcmp(args[i-1], "&") == 0){
                bg_process=1;
                args[i-1] = NULL;
            }
                
            
            args[i] = NULL;

            pid_t pid = fork(); //childpid

            if(pid < 0 ){
                perror("Fork Failed!!!");
            }
            else if(pid == 0){
                if(execvp(args[0], args) < 0){
                    perror("command not found!");
                    _exit(1);
                    /* WHY ARE WE USING "_exit()" ?
                    we can even use return(1) or exit(1); but we will face an issue of duplicate buffer flushing,
                    let's say that the text "nova:" is still in the buffer even after using fflush() cause some buffers
                    still maintain the data in the buffer, when we fork for getting a child process the buffer is also copied
                    so when we use a normal return/exit, it will kill the child process successfully but it scans all the streams
                    like stdout, in, err and flushes the buffers to the screen

                    but _exit(1) doesn't do that, it just erases the child memory
                    */
                }
            }else{
                    if(bg_process){
                        printf("background process running....\n");
                    }else{
                        waitpid(pid, &status, WUNTRACED);
                        
                        if(WIFSIGNALED(status)){
                            printf("%d is killed by signal %d", pid, WTERMSIG(status));
                        }
                        else if(WIFSTOPPED(status)){
                            printf("\n[%d] %s with pid %d stopped by %d", stopped_jobs_count +1,args[0],pid , WSTOPSIG(status));
                            if(stopped_jobs_count < 10){
                                stopped_jobs[stopped_jobs_count].pid = pid;
                                strncpy(stopped_jobs[stopped_jobs_count].command, saved_command, 99);//here we gave 100 -1 because we are leaving room for null terminator, else execv() can't work for creating a new program even though its an error
                                stopped_jobs_count++;
                            }
                        }
                    }
            }

        }

    }
    printf("\n\nThank you!\n\n");
    fflush(stdout);
    printf("system is shutting down...\n");
    sleep(1);

    return 0;
}