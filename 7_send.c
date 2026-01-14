#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MYPORT "3490"
#define BACKLOG 10

int main()
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;
    char buf[1024];
    int numbytes;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, MYPORT, &hints, &res);
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);
    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    send(new_fd, "hello, world\n", 13, 0);
    numbytes = recv(new_fd, buf, sizeof buf - 1, 0);
    if (numbytes > 0) 
    {
        buf[numbytes] = '\0';
        printf("received: %s\n", buf);
    }
    close(new_fd);
    close(sockfd);
    return 0;
}

