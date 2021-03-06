/*
    Beesham Sarendranauth
    2018/03/29
    Description: server implemented using sockets
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT  40041 // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

//get sockaddr
void *get_in_addr(struct sockaddr *sa) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
}

void handler(int signo){
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    socklen_t len;
    char clientAddStr[INET_ADDRSTRLEN];
        
    struct sockaddr_in servAdd;
    struct sockaddr_in clientAdd;

    char buf[1000];

    //Create socket
    //Create a socket with IPv4, bi-directional, default protocol of AF_NET and SOC_STREAM (TCP) 
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Error creating socket");
        exit(1);    
    }

    //Define socket endpoint/socket address
    servAdd.sin_family = AF_INET;   //IPv4
    servAdd.sin_addr.s_addr = INADDR_ANY;   //accept connection from any IP on this machine
    servAdd.sin_port = PORT;    //port to bind to

    //Associate the socket with its endpoint
    if((bind(sockfd, (struct sockaddr*)&servAdd, sizeof(servAdd))) == -1) {
        close(sockfd);
        perror("Error binding socket");
        exit(1);
    }

    //Announce that server is ready for connections 
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    signal(SIGPIPE, handler);
    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        socklen_t sin_size = sizeof clientAdd;
        new_fd = accept(sockfd, (struct sockaddr *)&clientAdd, &sin_size);
        int n;
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        
        inet_ntop(clientAdd.sin_family,
            get_in_addr((struct sockaddr *)&clientAdd), clientAddStr, sizeof clientAddStr);
        printf("server: got connection from %s\n", clientAddStr);
        
        int savedStdout = dup(STDOUT_FILENO);
        dup2(new_fd, STDOUT_FILENO);
        
        while(1){
            memset(&buf[0], 0, sizeof(buf));
            if((n = recv(new_fd, buf, sizeof(buf)-1, 0)) > 0){
               
                //execute cmd
                if(!fork()) {
                    execlp(buf, buf, NULL);
                    exit(1);
                }
                
                int status;
                wait(&status);
                fflush(stdout); 
            }
            if(n == 0){
                break;
            }else if(n < 0) {
                break;
            }
            
        }
        
        close(new_fd);
        dup2(savedStdout, STDOUT_FILENO);
        close(savedStdout);
        printf("server: waiting for connections...\n");
    }

    return 0;
}
