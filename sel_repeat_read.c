#include "sel_repeat_read.h"
#include "packet_interface.h"

#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>



void sel_repeat_read(const int sfd)
{
	int lastack=0;
	int window_size = MAX_WINDOW_SIZE;
	int max_sdu_size = MAX_PAYLOAD_SIZE + 8; //520
	char pkt_buffer[max_sdu_size];
	pkt_t * receive_buffer[MAX_WINDOW_SIZE];
	int j;
	for(j=0; j<MAX_WINDOW_SIZE; j++){
		receive_buffer[j]=NULL;
	}


	ssize_t sdu_size;
	while (1) { // Quid si EOF ?

		sdu_size = read(sfd, pkt_buffer, max_sdu_size); // buffer contient un nouveau packet
	
		pkt_t * pkt = pkt_new();
		if(pkt_decode(pkt_buffer, sdu_size, pkt)){
			pkt_del(pkt);
			send_pkt(PTYPE_NACK, lastack);
		}
		else if(compare_seqnums(lastack, pkt_get_seqnum(pkt), lastack+1) < window_size){
			insert_pkt(lastack, receive_buffer, pkt);
		}
		else{
			pkt_del(pkt);
			send_pkt(PTYPE_NACK, lastack, sfd);
		}
		
		
		int i;
		for(i=0; i<window_size; i++){// on écrit les packets de receive buffer qui sont succéssifs à lastack
			if(pkt_get_seqnum(receive_buffer[i]) == (lastack+1)%256){
				//write(1, receive_buffer[i], sizeof(*(receive_buffer[i])));// write payload!
				pkt_del(receive_buffer[i]);
				lastack=(lastack+1)%256;
				send_pkt(PTYPE_ACK, lastack);
			}
			else{
				break;
			}
		}
		refresh(receive_buffer); //mettre a jour receive_buffer en enlevant les éléments lus
		
	}
	
}

int send_pkt(ptypes_t ptype, int lastack, int sfd){
	pkt_t * pkt = pkt_new();
	pkt_set_type(pkt, ptype);
	pkt_set_window(pkt, 0);
	pkt_set_seqnum(pkt, (lastack+1)%256);
	pkt_set_length(pkt, 0);
	
	size_t length = 8;
	char data[length];
	pkt_encode(pkt, data, &length);
	pkt_del(pkt);
	write(sfd, data, length);
}

/*
* insère pkt dans buffer en créant un nouveau pointeur vers pkt et en respectant l'ordre des seqnums (buffer reste trié)
*
*/
int insert_pkt(int lastack, pkt_t * buffer[MAX_WINDOW_SIZE], pkt_t * pkt){
	int i;
	for(i=0; i<MAX_WINDOW_SIZE; i++){
		if(buffer[i]==NULL){
			buffer[i]=pkt;
			break;
		}
		else if(compare_seqnums(lastack, pkt_get_seqnum(pkt), pkt_get_seqnum(buffer[i])) < 0){
			pkt_t * next = buffer[i];
			buffer[i]=pkt;
			int j;
			for(j=1; j<MAX_WINDOW_SIZE-i; j++){
				buffer[i+j]=next;
				next=buffer[i+j+1];
			}
			break;
		}
	}
	return 0;
}


/*
* enlève tous les NULLs se situant en début de buffer en décalant les éléments du buffer
*
*
*/
int refresh(pkt_t * buffer[MAX_WINDOW_SIZE]){
	int i;
	for(i=0; i<MAX_WINDOW_SIZE; i++){
		if(buffer[i]!=NULL){
			int j;
			for(j=0; j+i<MAX_WINDOW_SIZE; j++){
				buffer[j]=buffer[j+i];
			}
			break;
		}
	}
}


/*
* renvoie la différence entre seqnum1 et seqnum2 en prenant en compte la succéssion circulaire des seqnums
*
*/
int compare_seqnums(int lastack,  int seqnum1, int seqnum2){	
	if(seqnum1 < lastack){
		seqnum1=seqnum1+256;
	}
	if(seqnum2 < lastack){
		seqnum2=seqnum2+256;
	}
	return seqnum1-seqnum2;
}





