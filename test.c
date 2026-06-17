#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>//for handling the file descriptors in redirections
#include "token.h" 

#define R_GREEN   "\033[32m"
#define RESET     "\033[0m"
#define R_YELLOW  "\033[33m"
#define R_RED     "\033[31m"
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
        // pid_t completed_pid;
        int status;

        // while((completed_pid = waitpid(-1, &status, WNOHANG)) > 0){
        //     printf("\nBackground job %d is finished!\n", completed_pid);
        // }
        
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
        // else if(strcmp(buffer, "fg") == 0){
        //     if(stopped_jobs_count == 0){
        //         printf("There are no foreground jobs running!\n");
        //     }else{
        //         stopped_jobs_count--;
        //         pid_t pid = stopped_jobs[stopped_jobs_count].pid;

        //         printf("continuing %s", stopped_jobs[stopped_jobs_count].command);

        //         kill(pid, SIGCONT);
        //         waitpid(pid, &status, WUNTRACED);
        //         if(WIFSTOPPED(status)){
        //             stopped_jobs_count++;
        //             printf("Process %d Stopped again!\n", pid);
        //         }
        //     }
        // }
        else{
            //handle background process later
            int token_count = 0;
            Token *tokens = tokenize(buffer, &token_count);
            
            if(token_count == 0){
                free(tokens);
                continue;
            }

            char *input_file = NULL;
            char *output_file = NULL;
            int append_mode = 0; // 1? '>>' : '>'(for append and output redirection mode)
            int syntax_error = 0;

            char **args = malloc((token_count +1)* sizeof(char *));
            int arg_idx = 0;

            for(int t = 0; t<token_count; t++){

                if(tokens[t].type == TOKEN_WORD){
                    args[arg_idx++] = tokens[t].value;
                }

                else if(strcmp(tokens[t].value, "<") == 0){
                    if(t+1 < token_count && tokens[t+1].type == TOKEN_WORD){
                        input_file = tokens[++t].value;
                    }else{
                        printf(R_GREEN"Nova: "R_RED"syntax error near unexpected token '<'\n");
                        syntax_error = 1;
                        break;
                    }
                }

                else if(strcmp(tokens[t].value, ">") == 0){
                    if(t + 1 < token_count && tokens[t+1].type == TOKEN_WORD){
                        output_file = tokens[++t].value;
                    }else{
                        printf(R_GREEN"Nova: "R_RED"syntax error near unexpected token '>'\n");
                        syntax_error = 1;
                        break;
                    }
                }
                
                else if(strcmp(tokens[t].value, ">>") == 0){
                    if(t + 1 < token_count && tokens[t+1].type == TOKEN_WORD){
                        output_file = tokens[++t].value;
                        append_mode = 1;
                    }else{
                        printf(R_GREEN"Nova: "R_RED"syntax error near unexpected token '>>'\n");
                        syntax_error = 1;
                        break;
                    }

                }

            }

            
            args[arg_idx] = NULL;
            fflush(stdout);
            
            if(syntax_error){
                for(int i =0; i< token_count; i++){
                    free(tokens[i].value);
                }
                free(tokens);
                free(args);
                continue;

            }
            pid_t pid = fork(); //childpid

            if(pid < 0 ){
                perror("Fork Failed!!!");
            }
            else if(pid == 0){
                if(input_file != NULL){
                    int fd_in = open(input_file, O_RDONLY);
                    if(fd_in < 0){
                        perror(R_GREEN"Nova: "R_RED"input file error");
                        _exit(1);
                    }
                    dup2(fd_in, STDIN_FILENO);
                    close(fd_in);
                }

                if(output_file != NULL){
                    int flags = O_WRONLY | O_CREAT;
                    if(append_mode){
                        flags |= O_APPEND;
                    }else{
                        flags |= O_TRUNC;
                    }

                    int fd_out = open(output_file, flags, 0644); //owner can create files but not the other users by using 0644
                    if(fd_out < 0){
                        perror(R_GREEN"Nova: "R_RED"output file error");
                    }else{
                        dup2(fd_out, STDOUT_FILENO);
                        close(fd_out);
                    }
                }

                if(arg_idx > 0 && execvp(args[0], args) < 0){
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
                _exit(0);
            }
            else{
                    waitpid(pid, &status, 0);
                    // if(bg_process){
                    //     printf("background process running....\n");
                    // }else{
                    //     waitpid(pid, &status, 0);
                        // waitpid(pid, &status, WUNTRACED);
                        
                        // if(WIFSIGNALED(status)){
                        //     printf("%d is killed by signal %d", pid, WTERMSIG(status));
                        // }
                        // else if(WIFSTOPPED(status)){
                        //     printf("\n[%d] %s with pid %d stopped by %d", stopped_jobs_count +1,args[0],pid , WSTOPSIG(status));
                        //     if(stopped_jobs_count < 10){
                        //         stopped_jobs[stopped_jobs_count].pid = pid;
                        //         strncpy(stopped_jobs[stopped_jobs_count].command, saved_command, 99);//here we gave 100 -1 because we are leaving room for null terminator, else execv() can't work for creating a new program even though its an error
                        //         stopped_jobs_count++;
                        //     }
                        // }
                    }
                for (int t = 0; t < token_count; t++) {
                    free(tokens[t].value);
                }
                free(tokens);
                free(args);
            }
            
        }

        printf("\n\nThank you!\n\n");
        fflush(stdout);
        printf("system is shutting down...\n");
        sleep(1);
    
        return 0;
    }
