#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res, *p;
    int status;
    int sockfd = -1;
    char ipstr[INET6_ADDRSTRLEN];
    if (argc != 3) 
    {
        fprintf(stderr, "usage: %s hostname port\n", argv[0]);
        return 1;
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;      
    hints.ai_socktype = SOCK_STREAM;  
    status = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 2;
    }
    for (p = res; p != NULL; p = p->ai_next) 
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) 
        {
            fprintf(stderr, "socket error: %s\n", strerror(errno));
            continue;
        }
        void *addr;
        char *ipver;
        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in *ipv4;
            ipv4= (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } 
        else 
        {
            struct sockaddr_in6 *ipv6;
            ipv6= (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("Trying %s address: %s\n", ipver, ipstr);
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            fprintf(stderr, "connect error: %s\n", strerror(errno));
            close(sockfd);
            sockfd = -1;
            continue;
        }
        printf("Connected to %s %s\n", ipver, ipstr);
        break;
    }
    freeaddrinfo(res);
    close(sockfd);
    return 0;
}

