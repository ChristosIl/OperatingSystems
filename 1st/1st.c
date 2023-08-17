#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

    

int main(int argc, char *argv[]) {

    //Valid command check
    if(argc != 2) {
        printf("Usage: ./a.out <filename>\n");
        return 1;
    }

    //--help
    if(strcmp(argv[1], "--help")==0) {
        printf("Usage: ./a.out <filename>\n");
        return(0);
    }

    //Existing file check
    struct stat buffer;
    if(stat(argv[1], &buffer)==0) {
        printf("Error: %s already exists\n" , argv[1]);
        return 1;
    }

    //child
    pid_t child_pid;
    char buf[64];
    child_pid=fork();
    int status;

    if(child_pid<0) {
        printf("Error creating child\n");
        return 1;
    }
    
    
    if(child_pid==0) { //Child code runs
        //getpid and getppid loaded in child's buffer
        snprintf(buf, sizeof(buf), "[CHILD]getpid()=%d, getppid()=%d\n", getpid(), getppid());

        //child fopen
        int fd = open(argv[1], O_CREAT | O_APPEND | O_WRONLY, 0644);

        if(fd ==-1) {
            printf("File is already open\n");
            return 1;
        }
        if(write(fd, buf, strlen(buf)) < strlen(buf)) {
            printf("Error occured while writing\n");
            return 1;
        }

        close(fd);
        exit(0);
    }
    else { //Parent code runs
        //getpid and getppid loaded in parent's buffer
        snprintf(buf, sizeof(buf), "[PARENT]getpid()=%d, getppid()=%d\n", getpid(), getppid());

        //parent fopen
        int fd = open(argv[1], O_CREAT | O_APPEND | O_WRONLY, 0644);

        if(fd ==-1) {
            printf("File is already open\n");
            return 1;
        }
        if(write(fd, buf, strlen(buf)) < strlen(buf)) {
            printf("Error occured while writing\n");
            return 1;
        }

	    close(fd);
        wait(&status);
        exit(0);
    }

}

