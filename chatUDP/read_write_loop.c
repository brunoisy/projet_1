#include "read_write_loop.h"

#include <netinet/in.h> /* * sockaddr_in6 */
#include <sys/types.h> /* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>


void read_write_loop(int sfd){
    
char buffer[1024];
char buffer2[1024];
    
while(read(0,buffer2,sizeof buffer2 -1)==0){}
    
    if(send(sfd, buffer2, strlen(buffer2), 0) < 0)
{
    perror("send()");
    exit(errno);
}

    
int n;
int j=1;
while(j==1){
if((n = recv(sfd, buffer, sizeof buffer - 1, 0)) < 0)
{
    perror("recv()");
    exit(errno);
}
write(1,buffer,sizeof buffer - 1);
}
    
    
    
}
