#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main() 
{
    // IPv4  
    char ip4[INET_ADDRSTRLEN];
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, "10.12.110.57", &(sa.sin_addr));
    inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
    printf("The IPv4 address is: %s\n", ip4);

    //IPv6 
    char ip6[INET6_ADDRSTRLEN];
    struct sockaddr_in6 sa6;
    memset(&sa6, 0, sizeof sa6);
    sa6.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "2001:db8:63b3:1::3490", &(sa6.sin6_addr));
    inet_ntop(AF_INET6, &(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);
    printf("The IPv6 address is: %s\n", ip6);
    return 0;
}

