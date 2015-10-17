#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "packet_implem.c"
#include <fcntl.h>
#include <sys/stat.h>

void sel_repeat_write(int fd, int socket){

 uint8_t max_window_size = 31;
 int max_payload_size = 512;
 int max_sdu_size = 520;
 char buffer[512];
 int real_payload_size;
 uint8_t real_window_size=max_window_size;
 uint8_t seqnum = 0;
 pkt_t * window[real_window_size];
 int current_index_window = 0;
 
 fd_set rdfs;
 ssize_t sdu_size;

 int max_fd;
        if(socket>fd){
           max_fd=socket;
        }else{
           max_fd = fd;
 }

 while (1) { 

	FD_ZERO(&rdfs);
        if(current_index_window<real_window_size){ //On peut envoyer un packet
	FD_SET(fd, &rdfs);
        }
        FD_SET(socket,&rdfs);
  
        

 if (select(max_fd + 1, &rdfs, NULL, NULL, NULL) == -1) {
			fprintf(stderr, "Error select");
			exit(errno);
		}

 if (FD_ISSET(fd, &rdfs)) {

 	real_payload_size = read(fd, buffer, max_payload_size);
        size_t data_size = (size_t)real_payload_size+8;
        char data[data_size];
        pkt_t * packet = pkt_new();
        pkt_set_type(packet,PTYPE_DATA);
        pkt_set_seqnum(packet,seqnum);
        pkt_set_window(packet,real_window_size);
        pkt_set_length(packet,(uint16_t)real_payload_size);
        pkt_set_payload(packet,buffer,real_payload_size);
        pkt_encode(packet,data,&data_size);
        
        seqnum = (seqnum+1)%256;
        window[current_index_window] = packet;
        current_index_window++;    
        write(socket,data,data_size);
        
 }


 if (FD_ISSET(socket, &rdfs)){

  char buffer[8];
  int length = read(socket,buffer,8);
  pkt_t * packet = pkt_new();
  int status = pkt_decode(buffer,length,packet);
  int seqnum_validated = pkt_get_seqnum(packet) - 1;
  int last_seqnum_send = seqnum - 1;
  int pointeur_last_send = current_index_window - 1;
  int ack_position;
  if(last_seqnum_send>=seqnum_validated){
  
   ack_position = pointeur_last_send - (last_seqnum_send - seq_num_validated);

  }else{

   ack_position = pointeur_last_send - last_seqnum_send - (real_window_size - seqnum_validated); 

 }
  
  modify_buffer(ack_position,pointeur_last_send,buffer);

  
  
 }

}
}

void modify_buffer(int ack, int last, pkt_t * buffer[]){

 int i;
 for(i=ack; i>=0 ; i--){

  buf[i] = NULL;

 }

 for(i=0; i<(last-ack);i++){

  buf[i] = buf[ack+1+i];

 }


}


int main(int argc, char * argv[]){

int fd = open("haha.txt", O_RDONLY);

sel_repeat_write(fd,0);



}



