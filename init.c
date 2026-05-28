#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main(){
    printf("\n\n\n-------NOVA OS v1.0-------\n\n\n");

    while(1){

        //printf("shellpid: %d", getpid()); 
        printf("nova> ");
        fflush(stdout);
        
        //creating a buffer for storing the commands
        char buffer[100];

        if(fgets(buffer, sizeof(buffer),stdin) == NULL){
            break;
        }//handling ctrl + D case

        if(buffer[0] == '\n'){
            continue;
        }//handling empty commands


        buffer[strcspn(buffer,"\n")] = '\0';//we are making it as a string by adding the null terminator


        //using if-else for checking basic commands directly from the shell 
        if(strcmp(buffer, "version") == 0){
            printf("\n\nNova OS\nVersion - 1.0\ncompiled - musl-gcc\ntested on: QEMU(virt)\n\n");
        }
        else if(strcmp(buffer, "help") == 0){
            printf("\nAvailable Commands:\n1. version\n2. help\n3. quit(q)\n\n");
        }
        else if(strcmp(buffer, "q") == 0 || strcmp(buffer,"quit")==0){
            break;
        }
        else{
            fprintf(stderr,"\"%s\" - command not found\n", buffer);
        }

    }
    printf("\n\nThank you!\n\n");
    fflush(stdout);
    printf("system is shutting down...\n");
    sleep(1);

    return 0;
}