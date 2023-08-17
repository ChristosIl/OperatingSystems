#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

char state;
int id;
pid_t parent_pid;
long int start_time;


void print_state(int sig) {
    printf("[GATE=%d/PID=%d/TIME=%lds] The gates are %s!\n", id, getpid(), time(NULL)-start_time, state == 't' ? "open" : "closed");
}

void alarm_handler(int sig) {
    print_state(sig);
    alarm(15);
}

void flip_state(int sig) {
    if (state == 't') {
        state = 'f';
    } else {
        state = 't';
    }
    print_state(sig);
}

void terminate_child(int sig) {
    exit(0);
}

int main(int argc, char* argv[]) {
    id = atoi(argv[2]);
    state = argv[1][0]; //0 because this time argv[1] is just one state converted to string
    parent_pid = getppid();
    start_time = time(NULL);

    struct sigaction sa_alarm, sa_print, sa_flip, sa_terminate;
    sa_alarm.sa_handler = alarm_handler;
    sa_print.sa_handler = print_state;
    sa_flip.sa_handler = flip_state;
    sa_terminate.sa_handler = terminate_child;

    sigaction(SIGALRM, &sa_alarm, NULL);
    sigaction(SIGUSR1, &sa_flip, NULL);
    sigaction(SIGUSR2, &sa_print, NULL);
    sigaction(SIGTERM, &sa_terminate, NULL);

    print_state(0); //initial TIME=0s print
    alarm(15); // first alarm. Afterwards there will be recursion withing the alarm_handler
    while(1) {
        sleep(1);
    }
    return 0;
}
