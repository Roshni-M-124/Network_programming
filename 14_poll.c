#include <stdio.h>
#include <poll.h>

int main(void)
{
    struct pollfd pfds[1];
    pfds[0].fd = 0;          
    pfds[0].events = POLLIN;
    printf("Hit RETURN or wait 10 seconds for timeout\n");
    int num_events = poll(pfds, 1, 10000); 
    if (num_events == 0)
    {
        printf("Poll timed out!\n");
    } 
    else
    {
        int pollin_happened = pfds[0].revents & POLLIN;
        if (pollin_happened) 
        {
            printf("File descriptor %d is ready to read\n",pfds[0].fd);
        } 
        else 
        {
            printf("Unexpected event occurred: %d\n",
                    pfds[0].revents);
        }
    }
    return 0;
}
