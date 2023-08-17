#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>


void create_child(char state, int id);
void sigusr1_handler(int sig);
void sigusr2_handler(int sig);
void sigterm_handler(int sig);
void sigchld_handler(int sig);

int num_children;
pid_t *children;
int *states;
long int start_time;
int term_flag; //checks if SIGTERM was sent to parent or exclussively to a child
int new_flag; //checks if a child is created or recreated to print the appropriate message

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./gates <states(i.e. ttf where t means open and f means closed)>\n");
        return 1;
    }

    num_children = strlen(argv[1]);
    start_time = time(NULL);
    term_flag=0;
    new_flag=0;

    for(int i=0; i<num_children; i++) {
        if(argv[1][i]!='t' && argv[1][i]!='f') {
            printf("Usage: ./gates <states(i.e. ttf where t means open and f means closed)>\n");
            return 1;
        }
    }

    //Malloc for children array
    children = malloc(num_children * sizeof(pid_t));
    if(children == NULL) {
        printf("Malloc error");
        exit(1);
    }

    //Malloc for states array
    states = malloc(num_children * sizeof(int));
    if(states == NULL) {
        printf("Malloc error");
        exit(1);
    }


    struct sigaction sa_usr1, sa_usr2, sa_term, sa_chld;
    sa_usr1.sa_handler = sigusr1_handler;
    sa_usr2.sa_handler = sigusr2_handler;
    sa_term.sa_handler = sigterm_handler;
    sa_chld.sa_handler = sigchld_handler;

    sigaction(SIGUSR1, &sa_usr1, NULL);
    sigaction(SIGUSR2, &sa_usr2, NULL);
    sigaction(SIGTERM, &sa_term, NULL);
    sigaction(SIGCHLD, &sa_chld, NULL);

    for (int i = 0; i < num_children; i++) {
        create_child(argv[1][i], i);
    }

    while(1) {
        pause();    
    }

    return 0;
}

void create_child(char state, int id) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Error: Cannot create child process");
        exit(1);
    } else if (pid == 0) {
        char id_str[10];
        sprintf(id_str,"%d", id); //convert id int->string
        char state_str[2];
        sprintf(state_str,"%c", state);//convert state char->string
        char *args[] = {"child", state_str, id_str, NULL};
        execv("./child", args);
        printf("Error: Cannot execute child process"); //runs only if execv returns, when there is an error
        exit(1);    
    } else {
        printf("[PARENT/PID=%d] Created %s %d (PID=%d) and initial state '%c'\n", getpid(), (new_flag==1)?"new child for gate":"child", id, pid, state);
        children[id] = pid;
        states[id] = (state == 't') ? 1 : 0;
    }
}

void sigusr1_handler(int sig) {
    //pid_t sender = getpid(); //doesnt need
    for (int i = 0; i < num_children; i++) {
        if (children[i] != 0) {
            kill(children[i], SIGUSR1);
        }
    }
}

void sigusr2_handler(int sig) {
    for (int i = 0; i < num_children; i++) {
        if (children[i] != 0) {
            kill(children[i], SIGUSR2);
        }
    }
}

void sigterm_handler(int sig) {
    int status;
    int num_left = num_children;
    term_flag=1;
    for (int i = 0; i < num_children; i++) {                //if number of children is 1 then it will print child, else children
        printf("Waiting for %d child%s to exit\n",num_left ,(num_left==1)?"":"ren");
        if (children[i] != 0) {
            kill(children[i], SIGTERM);
            printf("[PARENT/PID=%d] Child %d (PID=%d) terminated successfully with exit status code %d!\n", getpid(), i, children[i], WEXITSTATUS(status));
            num_left--;
        }
    }
    printf("All children exited, terminating as well\n");
    free(children); //Free malloc before exiting
    free(states);
    exit(0);
}

void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WUNTRACED)) > 0) {
        int id = -1;
        for (int i = 0; i < num_children; i++) {  //enters a loop onto the children array to find what is the kid that has been terminated or has been stopped.        
            if (children[i] == pid) {
                id = i;                                     
                break;
            }
        }
        if (id == -1) {
            printf("Error: Child process %d not found\n", pid);
            return;
        }
        if (WIFEXITED(status)&&term_flag==0) {
            printf("[PARENT/PID=%d] Child %d (PID=%d) exited\n", getpid(), id, pid);
            children[id] = 0; //remove pid of exited child
            new_flag=1; //since a child died new_flag is set to 1 since children created by calling create_child on the next line will be new
            create_child((states[id]==1)?'t':'f', id); //state of the new child should be the one child was assigned at creation and share the same id
        } else if (WIFSTOPPED(status)) {
            printf("[PARENT/PID=%d] Child %d (PID=%d) was stopped, continuing...\n", getpid(), id, pid);
            kill(pid, SIGCONT);
        }
    }
}
