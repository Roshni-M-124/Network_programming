#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define PORT "3490"  
#define BACKLOG 10

const char *inet_ntop2(void *addr, char *buf, size_t size)
{
  struct sockaddr_storage *sas = addr;
  struct sockaddr_in *sa4;
  struct sockaddr_in6 *sa6;
  void *src;
  switch (sas->ss_family) 
  {
    case AF_INET:
      sa4 = addr;
      src = &(sa4->sin_addr);
      break;
    case AF_INET6:
      sa6 = addr;
      src = &(sa6->sin6_addr);
      break;
    default:
      return NULL;
  }
  return inet_ntop(sas->ss_family, src, buf, size);
}
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size, int **count)
{
  if (*fd_count == *fd_size)
  {
    *fd_size *= 2; 
    *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    *count = realloc(*count, sizeof(int) * (*fd_size));
  }
  (*pfds)[*fd_count].fd = newfd;
  (*pfds)[*fd_count].events = POLLIN; 
  (*pfds)[*fd_count].revents = 0;
  (*count)[*fd_count]=0;
  (*fd_count)++;
}

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
  pfds[i] = pfds[*fd_count-1];
  (*fd_count)--;
}


void handle_new_connection(int sockfd, int* fd_count, int* fd_size, struct pollfd **pfds, int **count)
{
  struct sockaddr_storage clientaddr;
  socklen_t addrlen;
  int newfd;
  char clientIP[INET6_ADDRSTRLEN];
  addrlen=sizeof clientaddr;
  newfd=accept(sockfd,(struct sockaddr *)&clientaddr,&addrlen);
  if (newfd == -1) 
  {
    perror("accept");
  }
  else
  {
    add_to_pfds(pfds,newfd,fd_count,fd_size,count);
    printf("New connection from %s on socket %d\n",inet_ntop2(&clientaddr,clientIP,sizeof clientIP),newfd);
  }  
}  

int sendall(int sock, const void *buf, size_t len)
{
  size_t total = 0;
  const char *p = buf;
  while (total < len)
  {
   ssize_t n = send(sock, p + total, len - total, 0);
   if (n <= 0)
    return -1;
   total += n;
  }
 return 0;
}

int recvall(int sock, void *buff, size_t mlen) 
{
  size_t total = 0;
  char *p = buff;
  while (total < mlen)
  {
    int n = recv(sock, p + total, mlen - total, 0);
    if (n == 0) 
      return total;   
    if (n < 0) 
      return -1;       
    total += n;
  }
  return total;
}

void handle_client_data(int *fd_count,struct pollfd *pfds, int *pfd_i, int **count)
{
  uint32_t msglen; 
  int nbytes = recvall(pfds[*pfd_i].fd, &msglen, sizeof msglen);
  int sender_fd = pfds[*pfd_i].fd;
  if (nbytes <= 0)
  { 
    if (nbytes == 0)
    {
      printf("server: socket %d hung up\n", sender_fd);
    }
    else 
    {
      perror("recv");
    }
    close(pfds[*pfd_i].fd); 
    del_from_pfds(pfds, *pfd_i, fd_count);
    (*pfd_i)--;
  } 
  else
  {
    uint32_t len = ntohl(msglen);
    size_t bufsize = len + 18;
    char *buf = malloc(bufsize);
    if (!buf) 
      return;
    int r= recvall(sender_fd, buf, len);
    if(r!=(int)len)
    {
      free(buf);
      return;
    }
    (*count)[*pfd_i]++;
    size_t reply_len = len + 18;
    char *reply = malloc(reply_len);
    memcpy(reply, buf, len);
    int n = snprintf(reply + len, reply_len - len," echo no: %d", (*count)[*pfd_i]);
    reply_len = len + n;
    uint32_t net_len = htonl(reply_len);
    sendall(sender_fd, &net_len, sizeof net_len);
    sendall(sender_fd, reply, reply_len);

  }
}

void process_connections(int sockfd, int *fd_count, int *fd_size, struct pollfd **pfds, int **count)
{
  for(int i=0;i<*fd_count;i++)
  {
    if ((*pfds)[i].revents & (POLLIN | POLLHUP)) 
    {
      if ((*pfds)[i].fd == sockfd) 
      {
        handle_new_connection(sockfd, fd_count, fd_size,pfds,count);
      }
      else 
      {
        handle_client_data(fd_count, *pfds, &i,count);
      }
    }
  }
}
int main()
{
  int sockfd;
  struct addrinfo hints,*servinfo,*p;
  int rv;
  int yes=1;
  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET6;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_flags=AI_PASSIVE;
  if((rv=getaddrinfo(NULL,PORT,&hints,&servinfo))==-1)
  {
    fprintf(stderr, "server: %s\n", gai_strerror(rv));
    exit(1);
  }
  for(p=servinfo; p!=NULL; p=p->ai_next)
  {
    if((sockfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol))<0)
    {
      continue;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1)
    {
      perror("setsockopt");
      exit(1);
    }
    if((bind(sockfd,p->ai_addr, p->ai_addrlen))<0)
    {
      close(sockfd);
      continue;
    }
    break;
  }
  if(p==NULL)
  {
    fprintf(stderr, "error getting listening socket\n");
    exit(2);
  }
  freeaddrinfo(servinfo);
  if((listen(sockfd,BACKLOG))==-1)
  {
    fprintf(stderr, "error getting listening socket\n");
    exit(3);
  }
  
  int fd_size=5;
  int fd_count=1;
  int *count=malloc(sizeof (*count)* fd_size);
  struct pollfd *pfds = malloc(sizeof *pfds * fd_size);
  pfds[0].fd=sockfd;
  pfds[0].events=POLLIN;
  printf("Server waiting for connection");
  while(1)
  {
    int poll_count = poll(pfds, fd_count, -1);
    if (poll_count == -1) 
    {
      perror("poll");
      exit(1);
    }
    process_connections(sockfd, &fd_count, &fd_size, &pfds,&count);
  }
  free(pfds);
}
  
  

