#include "packet_interface.h"
//#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "sel_repeat_write.c"
#include "chatUDP/wait_for_client.c"
#include "chatUDP/real_address.c"
#include "chatUDP/create_socket.c"


int main(int argc , char * argv[]){


  char * hostname = argv[1];
  int port = atoi(argv[2]);

  if(argc == 4){
	fprintf(stderr, "Invalid number of arguments\n");
	return EXIT_FAILURE;
  }  

  char * filename = NULL;
  
  if(argc == 5){
  char * option = argv[3];
  if( strcmp(option, "-f") != 0 && strcmp(option,"--filename")!=0){
  	fprintf(stderr, "Invalid argument %s\n" , option);
	return EXIT_FAILURE;
  }
  filename = argv[4];
  }

  struct sockaddr_in6 addr;  

  const char * error = real_address(hostname,&addr);

  if(error!=NULL){
   fprintf(stderr, "Could not resolve hostname %s\n", hostname);
		return EXIT_FAILURE;
  }

  int socket = create_socket(&addr,port,NULL,-1);
  if (socket > 0 && wait_for_client(socket) < 0) { 
		fprintf(stderr,"Could not connect the socket after the first message.\n");
		close(socket);
		return EXIT_FAILURE;
  }

  if (socket < 0) {
		fprintf(stderr, "Failed to create the socket!\n");
		return EXIT_FAILURE;
  }

  int fd = 0;
  if(filename != NULL){
  fd = open(filename, O_RDONLY);
  if(fd<0){
    fprintf(stderr, "File '%s' does not exist\n", filename);
  }
  }

  sel_repeat_write(fd,socket);
  

}
