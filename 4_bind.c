#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main()
{
  struct addrinfo hints, *res;
  int sockfd,s;
  char ipstr[INET6_ADDRSTRLEN];
  void *addr;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;  
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;    
  getaddrinfo(NULL, "3490", &hints, &res);
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  s=bind(sockfd, res->ai_addr, res->ai_addrlen);
  if (res->ai_family == AF_INET)
  {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    addr = &(ipv4->sin_addr);
  }
  else 
  {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
    addr = &(ipv6->sin6_addr);
  }
  inet_ntop(res->ai_family, addr, ipstr, sizeof ipstr);
  printf("Addr: %s\n", ipstr);
  printf("%d\n",s);
  return 0;
}  
