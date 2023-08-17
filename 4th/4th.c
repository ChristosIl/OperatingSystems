#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/select.h>  
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <ctype.h>
#include <stddef.h> 

#define MAX(a, b) ((a) > (b) ? (a) : (b))
//#define default_HOST "iot.dslab.pub.ds.open-cloud.xyz"
//#define default_PORT 18080  

int isNumber(char s[]) { 
    for (int i = 0; s[i]; i++) {
        if (!isdigit(s[i]) && !isspace(s[i]) && s[i]!='\0') return 0;
    }
    return 1;
}

void checkHost(char* host){
    struct hostent *host_check = gethostbyname(host); //if gethostbyname returns NULL
    if(!host_check) {                                 //host_check will be null so !host_check == true  
        printf("Invalid host: %s\n", host);
        exit(EXIT_FAILURE);
    }
}

void checkPort(int port){
    if(port <= 0 || port > 65535) {
        printf("Invalid port: %d\n", port);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {
    bool flag = false;
    char *HOST = "iot.dslab.pub.ds.open-cloud.xyz";
    int PORT = 18080  ;


    //checking parameters
    if(argc>4) {
        printf("Usage: %s [--host HOST] [--port PORT] [--debug]\n", argv[0]);
        exit(EXIT_FAILURE);
    } else {
        for (int i = 1; i < argc; i++){
            if (strcmp(argv[i], "--host") == 0) {
                    if (i+1 == argc || argv[i+1][0] == '-') printf("Usage: %s [--host HOST] [--port PORT] [--debug]\n", argv[0]);
                    else HOST = argv[++i];
            }   else if (strcmp(argv[i], "--port") == 0) {
                    if (i+1 == argc || argv[i+1][0] == '-') printf("Usage: %s [--host HOST] [--port PORT] [--debug]\n", argv[0]);
                    else PORT = atoi(argv[++i]);
            }   else if (strcmp(argv[i], "--debug") == 0) {
                flag = true;   //--debug flag
            }   else {
                    printf("Usage: %s [--host HOST] [--port PORT] [--debug]\n", argv[0]);
                    exit(EXIT_FAILURE);
            }
        }
    }

    checkHost(HOST);
    checkPort(PORT); 

    //creating socket
    int domain =AF_INET;
    int type = SOCK_STREAM;
    int sock_fd = socket(domain, type, 0);

    //error checking
    if(sock_fd<0){
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    //implement struct sin
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(0);  /* Let the system choose */
    sin.sin_addr.s_addr = htonl(INADDR_ANY); //IP address of the socket einai opoiadhpote
                                             //address antistoixei sto susthma! //tha mporouse kai me gethostbyname and kserame to address mas?

    //socket gets address with this call
    int get_addr = bind(sock_fd, (struct sockaddr *) &sin, sizeof(sin)); 
    //error checking for bind
    if(get_addr < 0){
        perror("error in bind");
        exit(EXIT_FAILURE);
    }


    //trying to get host data
    struct hostent *hostp ;//using this struct to get host details
    hostp = gethostbyname(HOST); //me thn gethostbyname pairnoume to onoma tou host (orisma string) kai epistrefei pointer se mia domh tupou hostent
    struct sockaddr_in sa; 
    sa.sin_family = AF_INET;
    sa.sin_port = htons((u_short) PORT);
    bcopy(hostp->h_addr, &sa.sin_addr, hostp->h_length); //copying the host ip address  from host details to host socket 
    /* Σε αυτό το σημείο στο sa υπάρχει η πλήρης διεύθυνση του ζητούμενου εξυπηρετητή,
    οπότε με χρήση της connect() ζητείται σύνδεση */

    //trying to connect 
    if(connect(sock_fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0)  //2o to onoma toy socket toy eksuphrethth
        {                                                           //1o o socket descriptor toy socket(me auton exoume prosbash sto kanali)        
            perror("connect failed");
            exit(EXIT_FAILURE);
        } else 
                {
                    printf("Connecting to the server: 'iot.dslab.pub.ds.open-cloud.xyz'!\n");
                }
    /* Στο σημείο αυτό -αν βέβαια η connect() έχει επιτύχει- το socket sd του πελάτη έχει
    συνδεθεί με ένα αξιόπιστο κανάλι επικοινωνίας με το socket που περιγράφεται στο sa */            
    //end connecting part

    char userbuffer[101];
    while(1){
        fd_set inset;  //set of the fds
        int max_fd = STDIN_FILENO;
    
        FD_ZERO(&inset);            // we must initialize before each call to select
        FD_SET(STDIN_FILENO, &inset);// select will check for input from stdin
        FD_SET(sock_fd, &inset);//select will check for input from the sd

        max_fd = MAX(STDIN_FILENO, sock_fd); 
        max_fd = max_fd + 1;    //its +1 because select checks from the range 0 to max_fd-1
                                //you have to add 1 to have checkec all fds.    


        int ready_fds = select(max_fd, &inset, NULL, NULL, NULL);
            if(ready_fds <= 0) {
                    perror("select");
                    continue;      
            }

            //user typing to the terminal
            if(FD_ISSET(STDIN_FILENO, &inset)) {
               
                int n_read = read(STDIN_FILENO, userbuffer, 100);
               

                //error checking
                if(n_read < 0) {
                    perror("Error in reading the STDIN from buffer\n");
                    exit(EXIT_FAILURE);
                }

                //placing in the character that tell us the string is over in the last place of the array
                userbuffer[n_read] = '\0';
                //new line is also read from the string, discard it
                if(n_read > 0 && userbuffer[n_read-1] == '\n') {
                    userbuffer[n_read-1] = '\0';
                }    

                //exit 
                if(strcmp(userbuffer, "exit") == 0) {       
                    //shuting down (using 2 to close read and writing in socket)
                    if(shutdown(sock_fd, 2) < 0) {
                        perror("error in shuting down");
                    }

                    close(sock_fd);
                    printf("Exiting...\n");
                    close(sock_fd); //change
                    exit(0);
                }

                //help
                else if(strcmp(userbuffer, "help") == 0) {
                    printf("Available Commands:\n");
                    printf("'help'                      : Print this help message\n");
                    printf("'exit'                      : Exit\n");
                    printf("'get'                       : Retrieve sensor data\n");
                    printf("'N name surname reason'     : Ask permision to go out\n");        
                } 

                //get
                else if(strcmp(userbuffer, "get") == 0) {

                    if(flag) printf("[Debug] sent '%s'\n", userbuffer);
                    
                    if(write(sock_fd, userbuffer, 3) < 0) {
                        perror("failing to get");
                        exit(EXIT_FAILURE);
                    }
                }

                //sending everything else
                else if(n_read >=1 ) {
                    if(write(sock_fd, userbuffer, n_read) < 0) {
                        perror("failing to get");
                        exit(EXIT_FAILURE);
                    }

                    if(flag) printf("[Debug] sent '%s'\n", userbuffer);
                }
            } //typing done 
        
        //server returns data and reading them from socket file descriptor
        if(FD_ISSET(sock_fd, &inset)) {
            
            char serverbuffer[501]; 
            memset(serverbuffer, 0, sizeof(serverbuffer));
            int N_READ = read(sock_fd, serverbuffer, 500);

            if(N_READ>=1 && strncmp(serverbuffer, "try again", 9) == 0) {
                    printf("server sent: Try again\n");
                    memset(serverbuffer, 0, sizeof(serverbuffer));
                    continue;     
            } else if(N_READ >= 1 && isNumber(serverbuffer)) { //server sent informations about temp, l.event, ...(data is only numeric)

                if(flag) printf("[DEBUG] read: %s \n", serverbuffer);

                serverbuffer[N_READ] = '\0'; //adding null terminator

                char latest_event;
                char temperature[50];
                char brightness[50];
                char timestamp[50];

                /*copying from buffer to tables*/  
                latest_event = serverbuffer[0] - '0';
                strncpy(temperature, serverbuffer + 6, 4); 
                temperature[4] = '\0';
                strncpy(brightness, serverbuffer + 2, 3);
                brightness[3] = '\0';
                strncpy(timestamp, serverbuffer + 11, 10); 
                timestamp[10] = '\0';

                time_t system_time = atoi(timestamp);  //creating the time
                struct tm *info = localtime(&system_time);

                /*choosing latest ivent*/  
                printf("------------\n");

                switch(latest_event) {
                    case 0:
                        printf("Latest event: boot (0)\n");
                        break;
                    case 1:
                        printf("Latest event: setup (1)\n");
                        break;
                    case 2:
                        printf("Latest event: interval (2)\n");
                        break;
                    case 3:
                        printf("Latest event: button (3)\n");
                        break;
                    case 4:
                        printf("Latest event: motion (4)\n");
                        break;
                    default:
                        printf("Unknown event\n");
                }

                printf("Temperature is: %.2f\n", atoi(temperature)/100.0);
                printf("Light level is: %d\n", atoi(brightness));
                printf("Timestamp is: %s\n", asctime(info));
                memset(serverbuffer, 0, sizeof(serverbuffer)); //clear bufferserver
                }   else if(N_READ >= 1 && strncmp(serverbuffer, "ACK", 3) == 0) {
                
                    if(flag) printf("[DEBUG] read: %s \n", serverbuffer);
                    
                    printf("Response: %s\n", serverbuffer);
                    memset(serverbuffer, 0, sizeof(serverbuffer));
                    continue;
                }   else if(N_READ >= 1) {
                        
                        if(flag) printf("[DEBUG] read: %s \n", serverbuffer); 

                        printf("Send verification code: %s\n", serverbuffer);
                        memset(serverbuffer, 0, sizeof(serverbuffer));   
                        continue;
                    }    
        }


} //end while


return 0;
     //printf("Connecting!\n");
}
