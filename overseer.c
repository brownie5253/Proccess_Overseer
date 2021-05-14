/*
*  Materials downloaded from the web.
*  Collected and modified for teaching purpose only.
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <time.h>

#define BACKLOG 5 /* how many pending connections queue will hold */

#define RETURNED_ERROR -1

int *getTime()//int * currTime) 
{
    time_t rawtime = time(NULL);
    
    if (rawtime == -1) {
        
        perror("Time():");
        exit(1);
    }
    
    struct tm *ptm = localtime(&rawtime);
    
    if (ptm == NULL) {
        
        perror("Localtime():");
        exit(1);
    }
    
    int tempTime[6] = {(ptm->tm_year)+1900, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec};

    int *currTime = tempTime;

    return currTime;
}

void Receive_Array_Int_Data(int socket_identifier, int* size)
{
    
    int number_of_bytes;
    char recive[300];
    int arraySize[4];
    char *results;
        if ((number_of_bytes = recv(socket_identifier, recive, 300, 0)) == RETURNED_ERROR)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        if ((number_of_bytes = recv(socket_identifier, arraySize, 16, 0)) == RETURNED_ERROR)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }

    int sizeIndex = 0;
    int oIndex = 1;
    int logIndex = 2;
    int fileIndex = 3;
    *size = arraySize[sizeIndex];
    int oPos = arraySize[oIndex];
    int logPos = arraySize[logIndex];
    int filePos = arraySize[fileIndex];

    const char *s = "^"; 
    char* tok; 

    int sizeArr = *size;

    char argsPrint[200] = {};
    
    // Use of strtok 
    // get first token 
    tok = strtok(recive, s); 
    
    char* recivedInput[*size];
    char* inputArr[*size-filePos+1];
    
    int i = 0;
    int indexer = 0;
    int pastFile = 0;

    // Checks for delimeter 
    while (tok != 0) { 
        if (i >= filePos) {
            inputArr[indexer] = tok;
            strcat(argsPrint, tok);
            strcat(argsPrint, " ");
            indexer++;
        }
        recivedInput[i] = tok;
 
        // Use of strtok to go through other tokens 
        tok = strtok(NULL, s); 
        i++;
    } 
    inputArr[indexer] = NULL; // need to pass in NULL terminated array
    


    //Branches
    //if (*recivedInput[filePos] == '/') {

        if (logPos > 0) {
            int fd = open(recivedInput[logPos], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
            dup2(fd,1);
            dup2(fd,2);
            close(fd);
        }

        time_t start,end;
        pid_t pid2;
        int status;
        int killTrigger = 0;

        pid2 = fork();

        if (pid2 == -1){ // pid == -1; means error occured 
            printf("can't fork, error occured\n"); 
            exit(EXIT_FAILURE); 
        } 
        if (pid2 == 0) { /* pid == 0; child process */
            ptrace(PTRACE_TRACEME, 0, NULL, NULL);
            int *currTime = getTime();
            printf("%d-%d-%d %d:%d:%d - attempting to execute %s\n",currTime[0],currTime[1],currTime[2],currTime[3],currTime[4],currTime[5], argsPrint);
            
            if (oPos > 0) {
                int fd = open(recivedInput[oPos], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                dup2(fd,1);
                dup2(fd,2);
                close(fd);
            }
            raise(SIGSTOP);
            execv(recivedInput[filePos], inputArr);
            printf("File Errored\n");
            exit(127); /* only if execv fails */
        } 

        else { /* pid!=0; parent process */

            while(waitpid(pid2, &status, 0) && !WIFEXITED(status)) {
                ptrace(PTRACE_SETOPTIONS, pid2, NULL, PTRACE_O_TRACEEXEC);
                if ((status >> 8) == (SIGTRAP | PTRACE_EVENT_EXEC << 8)){
                    int *currTime = getTime();
                    printf("%d-%d-%d %d:%d:%d - %s has been executed with pid %d\n", currTime[0],currTime[1],currTime[2],currTime[3],currTime[4],currTime[5],
                    argsPrint, (int)pid2);
                    start=clock();
                    sleep(2);
                    ptrace(PTRACE_CONT, pid2, NULL, NULL);
                    break;
                }
                ptrace(PTRACE_CONT, pid2, NULL, NULL);
            }
            while(1) {
                waitpid(pid2, &status, WNOHANG);
                end = clock();

                if (WIFEXITED(status)) {
                    break;
                }
                else if (((end-start)/CLOCKS_PER_SEC) >= 10 && killTrigger == 0) {
                    kill(pid2, SIGTERM);
                    int *currTime = getTime();
                    printf("%d-%d-%d %d:%d:%d - sent SIGTERM to %d\n", currTime[0],currTime[1],currTime[2],currTime[3],currTime[4],currTime[5],
                    (int)pid2);
                    start = clock();
                    killTrigger = 1;
                }
                else if (((end-start)/CLOCKS_PER_SEC) >= 5 && killTrigger == 1) {
                    kill(pid2, SIGKILL);
                    int *currTime = getTime();
                    printf("%d-%d-%d %d:%d:%d - sent SIGKILL to %d\n", currTime[0],currTime[1],currTime[2],currTime[3],currTime[4],currTime[5],
                    (int)pid2);
                    kill(getpid(), SIGKILL);
                    //return;
                }
            }

            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) != 127) {
                    int *currTime = getTime();
                    printf("%d-%d-%d %d:%d:%d - %d has terminated with status code %d\n", currTime[0],currTime[1],currTime[2],currTime[3],currTime[4],currTime[5],
                    (int)pid2, WEXITSTATUS(status));
                }
                else {
                    int *currTime = getTime();
                    printf("%d-%d-%d %d:%d:%d - could not execute %s\n", currTime[0],currTime[1],currTime[2],currTime[3],currTime[4],currTime[5],
                    argsPrint);
                }
            }
        }

}


int main(int argc, char * argv[])
{


    int sockfd, new_fd;            /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in my_addr;    /* my address information */
    struct sockaddr_in their_addr; /* connector's address information */
    socklen_t sin_size;

    /* Get port number for server to listen on */
    if (argc != 2)
    {
        fprintf(stderr, "usage: port_number\n");
        exit(1);
    }

    /* generate the socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    /* Enable address/port reuse, useful for server development */
    int opt_enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_enable, sizeof(opt_enable));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt_enable, sizeof(opt_enable));

    /* clear address struct */
    memset(&my_addr, 0, sizeof(my_addr));

    /* generate the end point */
    my_addr.sin_family = AF_INET;            /* host byte order */
    my_addr.sin_port = htons(atoi(argv[1])); /* short, network byte order */
    my_addr.sin_addr.s_addr = INADDR_ANY;    /* auto-fill with my IP */

    /* bind the socket to the end point */
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    /* start listnening */
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    int *currTime = getTime();

    printf("server starts listnening ...\n");


    while (1)
    { /* main accept() loop */
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr,
                             &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }
        int *currTime = getTime();
        printf("%d-%d-%d %d:%d:%d - connection received from ",currTime[0],currTime[1],currTime[2],currTime[3],currTime[4],currTime[5]);
        printf("%s\n",inet_ntoa(their_addr.sin_addr));

        if (!fork())
        { /* this is the child process */
            int temp;
            int* arraySize = &temp;

            /* Call method to recieve array data */
            Receive_Array_Int_Data(new_fd, arraySize);
            close(new_fd);
            exit(0);
        }
        else
        {
            close(new_fd); /* parent doesn't need this */
        }
        while (waitpid(-1, NULL, WNOHANG) > 0)
            ; /* clean up child processes */
    }
}