#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

// Declaration of bool enum m cdrgggbtgghtn
typedef enum { F, T } boolean; 

void Send_Array_Data(int socket_id, char *myArray[], int size, int oPos, int logPos, int filePos)
{
    int counter = 0;
    char sendMesg[300] = {};
    for (int i = 3; i < size; i++)
    {
        strcat(sendMesg, myArray[i]);
        strcat(sendMesg, "^");
        counter++;
    }

    int intSend[4] = {};
    intSend[0] = counter;

    //First 3 args are cut off so indexes need to be moved forward 
    int adjustIndex = 3;
    intSend[1] = oPos - adjustIndex;
    intSend[2] = logPos - adjustIndex;
    intSend[3] = filePos - adjustIndex;

    send(socket_id, sendMesg, sizeof(sendMesg), 0);
    send(socket_id, intSend, sizeof(intSend), 0);
}

char *get_filename_ext(char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

boolean orderCorrect(int argc, char* argv[], int* oPos, int* logPos, int* filePos) {
    int pathPos = 99;
    boolean order = T;

    char* typeInput[] = {"-o","txt", "-log", "txt", "/"};
    
    for (int i = 3; i < argc; i++) {

        if(strcmp(argv[i], typeInput[0]) == 0) {
            if(i != 3){
                order = F;
                break;
            }
            else {
                for (int add = 0; add <= i; add++) {
                    (*oPos)++;
                }
                (*oPos)++;
            }
        }
        
        else if(strcmp(get_filename_ext(argv[i]), typeInput[1]) == 0){
            if(strcmp(argv[i-1], typeInput[0]) != 0 && strcmp(argv[i-1], typeInput[2]) != 0){
                order = F;
                break;
            }
        }
        else if(strcmp(argv[i], typeInput[2]) == 0) {
            if((i != 3 && i != 5) || pathPos != 99){
                order = F;
                break;
            }
            else {
                for (int add = 0; add <= i; add++) {
                    (*logPos)++;
                }
                (*logPos)++;
                
            }
        }
        else if(*argv[i] == '/' || pathPos == 99){
            pathPos = i;
            if(i != 3 && i != 5 && i != 7){
                order = F;
                break;
            }
            else {
                for (int add = 0; add <= i; add++) {
                    (*filePos)++;
                }
            }
        }
        else {

        }
    }
    return order;
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */
    boolean isNum = T;

    char* port = argv[2];

    for( int i = 0; i < strlen(argv[2]); i++){
        if(!isdigit(port[i]) ){
            isNum = F;
        }
    }   
    int filePos = -1;
    int logPos = -1;
    int oPos = -1;
    int* file = &filePos;
    int* o = &oPos;
    int* log = &logPos;
    boolean correctOrder;

    correctOrder = orderCorrect(argc, argv, o, log, file);

    if (argc <= 3 || !isNum || !correctOrder)
    {
        fprintf(stderr, "Usage: controller <address> <port> {[-o out_file] [-log log_file] [-t seconds] <file> [arg...] | mem [pid] | memkill <percent>}\n");
        exit(1);
    }

    if ((he = gethostbyname(argv[1])) == NULL)
    { /* get the host info */
        herror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    /* clear address struct */
    memset(&their_addr, 0, sizeof(their_addr));

    their_addr.sin_family = AF_INET;            /* host byte order */
    their_addr.sin_port = htons(atoi(argv[2])); /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);

    if (connect(sockfd, (struct sockaddr *)&their_addr,
                sizeof(struct sockaddr)) == -1)
    {
        printf("Could not connect to overseer at %s, %s\n",argv[1], argv[2]);
        exit(1);
    }

    Send_Array_Data(sockfd, argv, argc, oPos, logPos, filePos);

    close(sockfd);

    return 0;
}
