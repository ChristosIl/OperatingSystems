#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))


int isNumeric(const char* str) { // Changed parameter type to const char*
    int i = 0;
    while (str[i]) {
        if (!isdigit(str[i])) {
            return 0;
        }
        i++;
    }
    return 1;
}

int main(int argc, char* argv[]){

    int val;
    int method;
    int num_children;

//checking arguments   
    if ((argc < 2)|| (argc > 3) ) {
        printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        exit(EXIT_FAILURE);
    } 

    if (!(isNumeric(argv[1])) )  {   //if not integer,exit
        printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
        exit(EXIT_FAILURE);
    }  

    else {
            num_children = atoi(argv[1]);

            if(num_children<=0){
                printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
                exit(EXIT_FAILURE);
            }       
                
    }

    if (argc == 3) 
        {
            if(strcmp(argv[2], "--round-robin") == 0)
                {
                    method = 0; // Use round-robin job assignment method
                }
            else if(strcmp(argv[2], "--random") == 0) // Added missing '--' before 'random'   
                {
                    method = 1; // Use random job assignment method
                }
            else 
                {
                    printf("Usage: ask3 <nChildren> [--random] [--round-robin]\n");
                    exit(EXIT_FAILURE);
                }    
        }

    if (argc == 2) { // Changed from assignment to comparison (== to =)
        method = 0; //default round-robin job assignment method;
    }

    
//end of checking arguments

    //create dynamic array of children's pids 
    pid_t *childrens_pid ;
    childrens_pid = malloc(num_children * sizeof(pid_t));
    if(childrens_pid == NULL)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }


    int pda[num_children][2];  //0: Child reads from this end, 1: Parent writes from this end 
    int pdb[num_children][2];  // 0: Parent reads from this end, 1: Child writes from this end                      
                            
                            //creating kids

    //pipe creation and parameters
    for(int k=0; k<num_children; k++){

        //pipe(&pda[k][0]); 
        if(pipe(pda[k]) == -1){
            perror("pipe. Reading point for father");
            exit(EXIT_FAILURE);
        }

      //pipe(&pdb[k][2]); 
      if(pipe(pdb[k]) == -1){
            perror("pipe.Reading point for child");
            exit(EXIT_FAILURE);
        } 
    }
        
                        
    for(int i=0; i < num_children; i++ ){
        
        
      pid_t  pid = fork();

                if(pid < 0)
                    {
                        perror("fork failed");
                        exit(EXIT_FAILURE);
                    }
                
                if(pid == 0){ //CHILD'S CODE
                    
                    int val;
                     
                    
                                    if (close(pdb[i][0]) == -1)     //closing father's reading point
                                        {
                                        perror("Error closing the file");
                                        return 1;
                                        }

                                    if (close(pda[i][1]) == -1)    //closing father's writing point
                                        {
                                        perror("Error closing the file");
                                        return 1;
                                        }
                                    //close(pdb[i][0]); //closing father's reading point
                                   // close(pda[i][1]); //closing father's writing point


                    while(1){

                      

                        //read(pd[i][2], &val, sizeof(int)); 
                                 
                                 if(read(pda[i][0], &val, sizeof(int)) == -1) //parameter if reading goes wrong in the child
                                        { 
                                            perror("read inside the child");
                                            exit(-1);
                                        }

                        printf("[Child %d with PID: %d]: Child received %d! \n", i, getpid(), val );
                        val++;
                        sleep(5);
                        
                        //write(pd[i][3], &val, sizeof(int)); //sending to the father the new value;
                        //close(pd[i][3]);

                        if(write(pdb[i][1], &val, sizeof(int)) != sizeof(int))
                                {
                                     perror("write inside the child");
                                     exit(-1);
                                }


                        printf("[Child %d with PID: %d]: Child finished hardwork, writing back %d! \n", i, getpid(), val );

                             
                    }

                }
   
        childrens_pid[i] = pid; //if i tried to collect childrens' pids inside the children i could because the father could not see the value and add it to the array. the same would happen if i tried to define 
                                //childrens_pid as a global var.
                                //otan ftiaxnetai paidi klhrwnomei ta panta apo ton patera(childrens_pid pinaka)
                                //kai otan paei na allaksei ton pinaka, allazei mono mesa sto paidi kai o pateras blepei to arxiko

    }

        //a little bit of father codes

        for(int j = 0; j<num_children; j++)
            {
                close(pda[j][0]); //We are closing child's reading point 
                close(pdb[j][1]); // We are closing child's writing point
            }


        int counter = 0; //which child 

        while(1){
            fd_set  inset; 
            int max_fd = STDIN_FILENO; 

            FD_ZERO(&inset);               // we must initialize before each call to select 
            FD_SET(STDIN_FILENO, &inset); // select will check for input from stdin

            for(int k = 0; k<num_children; k++)
                    {
                        FD_SET(pdb[k][0], &inset);          // select will check for input from pipe    
                        max_fd = MAX(max_fd, pdb[k][0]) ; // Updated to set max_fd correctly
                    }

            max_fd = max_fd + 1;
          
            // wait until any of the input file descriptors are ready to receive
            int ready_fds = select(max_fd, &inset, NULL, NULL, NULL);
                if (ready_fds <= 0) 
                {
                    perror("select");
                    continue;                                       // just try again
                }

              // user has typed something, we can read from stdin without blocking
        if (FD_ISSET(STDIN_FILENO, &inset)) {
            char buffer[101];
            int n_read = read(STDIN_FILENO, buffer, 100);   
                
                // error checking!
                if(n_read<0)
                    {
                        perror("Error in reading the STDIN from buffer\n");
                        exit(EXIT_FAILURE);    
                    }

                    
            buffer[n_read] = '\0';                          // placing in the character that tell us the string is over in the last place of the array

             // New-line is also read from the stream, discard it.
        if (n_read > 0 && buffer[n_read-1] == '\n') {
            buffer[n_read-1] = '\0';     
        }

        //adding the exit parameter 
        //printf("%d prwto\n", num_children);
        if(n_read > 4 && strncmp(buffer, "exit", 4) == 0) {
        //printf("%d deutero\n", num_children);    
            //printf("kalhspera\n");
           for(int i = 0; i<num_children; i++)
                    {
                        //printf("%d prwto", childrens_pid[i]);
                        //printf("%d deutero", getpid());

                       // printf("should go for termination\n");
                         

                        if(kill(childrens_pid[i], SIGTERM) == -1)
                            {
                                perror("SIGTERM FAILED\n");
                                exit(EXIT_FAILURE);
                            }

                        printf("[FAther process: %d] terminating child %d with PID: %d\n", getpid(), i,childrens_pid[i]);   
                         close(pdb[i][0]);  //we are closing the point where father reads
                         close(pda[i][1]);  //we are closing the point where father writes
                    }

             for (int i = 0; i < num_children; i++) 
                    {
                    waitpid(childrens_pid[i], NULL, 0);
                    }

                       exit(0);      

        }

        if(n_read>=1 && isNumeric(buffer) == 0)  //in case user types HELP or anything else than exit and number
            {
                printf("Type a number to send job to a child!\n");
            }
            
        if(n_read>=1 && isNumeric(buffer) == 1)       //here we will define what robin and random will do
        {
            int x = atoi(buffer);
            int ID = counter % num_children;
            
                if(method == 0)
                    {
                            //write(pd[ID][1], &x, sizeof(int)); // Updated to send to the correct pipe index

                            if(write(pda[ID][1], &x, sizeof(int)) != sizeof(int))
                                {
                                    perror("Error in write of father to child\n");
                                    exit(EXIT_FAILURE);
                                }

                            printf("[PARENT] assigned child %d the number %d\n", ID, x);    
                            counter++;
                    }
                else if(method == 1)
                    {       
                            int ID_2 =rand() % num_children;

                            //write(pda[ID_2][1], &x, sizeof(int)); // Updated to send to the correct pipe index

                            if(write(pda[ID_2][1], &x, sizeof(int)) != sizeof(int))
                                {
                                    perror("Error in write of father to child\n");
                                    exit(EXIT_FAILURE);
                                }

                            printf("[PARENT] assigned child %d the number %d\n", ID_2 , x);                                   
                    }    
        }     

        

       
    } //end of FD_ISSET(STDIN_FILENO, &inset)

    for(int j=0; j<num_children; j++)
        {
           if (FD_ISSET(pdb[j][0], &inset)) 
           {
                int val;
                //read(pd[j][0], &val, sizeof(int));  
                if(read(pdb[j][0], &val, sizeof(int)) == -1) // error checking!
                    {
                        perror( "father reading from pipes");
                        exit(EXIT_FAILURE);
                    }                  

                printf("[Parent] Received result from child %d ----> %d.\n", j,val);
            } 
        }

    
}

return 0;

}
