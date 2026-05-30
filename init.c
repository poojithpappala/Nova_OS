#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define R_GREEN   "\033[32m"
#define RESET     "\033[0m"
#define R_YELLOW  "\033[33m"
#define VERSION "1.2"   

int main(){
    printf(R_YELLOW"\n\n\n-------NOVA OS v%s-------\n\n\n"RESET, VERSION);

    while(1){

        //printf("shellpid: %d", getpid()); 
        printf(R_GREEN"nova"RESET);
        printf(": ");
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
        else{
            char *args[10];
            int i = 0;

            char* token = strtok(buffer, " ");
            while(token != NULL && i<9){
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            pid_t pid = fork(); //childpid
            if(pid < 0 ){
                perror("Fork Failed!!!");
            }else if(pid == 0){
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
                    wait(NULL);
            }

        }

    }
    printf("\n\nThank you!\n\n");
    fflush(stdout);
    printf("system is shutting down...\n");
    sleep(1);

    return 0;
}