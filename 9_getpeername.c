#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "3490"

int main()
{
    struct addrinfo hints, *res;
    struct sockaddr_storage peer_addr;
    socklen_t peer_len;
    char ipstr[INET6_ADDRSTRLEN];
    int sockfd, new_fd;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, PORT, &hints, &res);
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, 5);
    peer_len = sizeof peer_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&peer_addr, &peer_len);
    getpeername(new_fd, (struct sockaddr *)&peer_addr, &peer_len);
    void *addr;
    if (peer_addr.ss_family == AF_INET) 
    {
        addr = &((struct sockaddr_in *)&peer_addr)->sin_addr;
    } 
    else 
    {
        addr = &((struct sockaddr_in6 *)&peer_addr)->sin6_addr;
    }
    inet_ntop(peer_addr.ss_family, addr, ipstr, sizeof ipstr);
    printf("Client IP: %s\n", ipstr);
    close(new_fd);
    close(sockfd);
    return 0;
}

