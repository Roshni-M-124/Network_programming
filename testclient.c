#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define PORT "3490"
#define MAXBUF 1024

int sendall(int sock, const void *buf, size_t len)
{
    size_t total = 0;
    const char *p = buf;
    while (total < len) 
    {
      ssize_t n = send(sock, p + total, len - total, 0);
      if (n <= 0) return -1;
      total += n;
    }
    return 0;
}

int recvall(int sock, void *buf, size_t len)
{
    size_t total = 0;
    char *p = buf;
    while (total < len)
    {
      ssize_t n = recv(sock, p + total, len - total, 0);
      if (n <= 0) return -1;
      total += n;
    }
    return 0;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    struct addrinfo hints, *servinfo,*p;
    int sockfd;
    char input[MAXBUF];
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    if (argc != 2)
    {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    if (getaddrinfo(argv[1], PORT, &hints, &servinfo) != 0) 
    {
        perror("getaddrinfo");
        exit(1);
    }
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
        {
            perror("client: socket");
            continue;
        }
        inet_ntop(p->ai_family,get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
        printf("client: attempting connection to %s\n", s);
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
        {
            perror("client: connect");
            close(sockfd);
            continue;
        }
        break;
    }
    if (p == NULL) 
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    freeaddrinfo(servinfo);
    printf("Connected to server. Type messages (quit to exit).\n");
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(input, sizeof input, stdin))
            break;
        input[strcspn(input, "\n")] = '\0';
        if (strncmp(input, "quit", 4) == 0)
            break;
        struct timespec ts;
        struct tm tm;
        char  s[75];
        clock_gettime(CLOCK_REALTIME, &ts);
        localtime_r(&ts.tv_sec, &tm);
        snprintf(s,sizeof(s),"[%04d-%02d-%02d %02d:%02d:%02d]  ",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,   tm.tm_sec);  
        strcat(s,input);
        size_t msglen = strlen(s);
        uint32_t netlen = htonl(msglen);
        if (sendall(sockfd, &netlen, sizeof netlen) < 0)
            break;
        if (sendall(sockfd, s, msglen) < 0)
            break;
        uint32_t reply_len;
        if (recvall(sockfd, &reply_len, sizeof reply_len) < 0)
            break;
        reply_len = ntohl(reply_len);
        char *reply = malloc(reply_len + 1);
        if (!reply)
            break;
        if (recvall(sockfd, reply, reply_len) < 0) 
        {
            free(reply);
            break;
        }
        reply[reply_len] = '\0';
        printf("%s\n", reply);
        free(reply);
    }
    close(sockfd);
    printf("Disconnected.\n");
    return 0;
}

