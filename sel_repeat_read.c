#include "sel_repeat_read.h"
#include "packet_interface.h"


#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

void printBits(size_t const size, void const *const ptr);


void sel_repeat_read(const int sfd)
{
	int lastack=255;
	int window_size = MAX_WINDOW_SIZE;
	int max_sdu_size = MAX_PAYLOAD_SIZE + 8;
	char pkt_buffer[max_sdu_size];
	pkt_t * receive_buffer[MAX_WINDOW_SIZE];
	int j;
	for(j=0; j<MAX_WINDOW_SIZE; j++){
		receive_buffer[j]=NULL;
	}

	pkt_status_code err;
	ssize_t sdu_size;
	while (1) {
printf("before read\n");
		sdu_size = read(sfd, pkt_buffer, max_sdu_size); // buffer contient un nouveau packet



		pkt_t * pkt = pkt_new();
		err=pkt_decode(pkt_buffer, sdu_size, pkt);
printf("pkt_type : %d \n", pkt_get_type(pkt));
printf("pkt_seqnum : %d\n", pkt_get_seqnum(pkt));
		if(err){
			if(err!=E_NOHEADER){
				send_nack(pkt_get_seqnum(pkt), sfd, window_size);
			}
			pkt_del(pkt);
		}




		if(compare_seqnums(lastack, pkt_get_seqnum(pkt), lastack+1) < window_size){ //si le numéro de seq est dans la fenêtre
			insert_pkt(lastack, receive_buffer, pkt);
			window_size--;
		}
		else{
			pkt_del(pkt);
			send_nack(pkt_get_seqnum(pkt), sfd, window_size);
		}


		int i;
		for(i=0; i<window_size; i++){// on écrit les packets de receive buffer qui sont succéssifs à lastack
			if(receive_buffer[i]==NULL){
				break;
			}
			if(pkt_get_seqnum(receive_buffer[i]) == (lastack+1)%256){
printf("after if in while\n");
				write_payload(1, receive_buffer[i]); // 1 ou -f ?
printf("after write payload\n");
				pkt_del(receive_buffer[i]);
				receive_buffer[i]=NULL;
				window_size++;
				lastack=(lastack+1)%256;
				send_ack(lastack, sfd, window_size);
			}
			else{
				break;
			}
		}
printf("before refresh\n");
		refresh(receive_buffer); //mettre a jour receive_buffer en enlevant les éléments lus, renvoi nbr élements vides
		
	}
	
}


/*
* écrit dans fds le contenu du payload de pkt
*
*/
int write_payload(int fds, pkt_t * pkt){
	write(fds, pkt_get_payload(pkt), pkt_get_length(pkt));
}

/*
* envoi un packet ack au sender, contenant le numéro de seqnum du dernier packet reçu
*/

int send_ack(int lastack, int sfd, int window_size){
	pkt_t * pkt = pkt_new();
	pkt_set_type(pkt, PTYPE_ACK);
	pkt_set_window(pkt, window_size);
	pkt_set_seqnum(pkt, (lastack+1)%256);
	pkt_set_length(pkt, 0);
	
	size_t length = 8;
	char data[length];
	pkt_encode(pkt, data, &length);
	pkt_del(pkt);
	write(sfd, data, length);
}

/*
* envoi un packet nack au sender, contenant le numéro de seqnum du packet non reçu
*
*/
int send_nack(int seqnum, int sfd, int window_size){
	pkt_t * pkt = pkt_new();
	pkt_set_type(pkt, PTYPE_NACK);
	pkt_set_window(pkt, window_size);
	pkt_set_seqnum(pkt, seqnum);
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
			pkt_t * this = buffer[i];
			pkt_t * next;
			buffer[i]=pkt;
			int j;
			for(j=1; j<MAX_WINDOW_SIZE-i; j++){
				next=buffer[i+j];
				buffer[i+j]=this;
				this=next;
				
			}
			break;
		}
	}
	return 0;
}


/*
* enlève tous les NULLs se situant en début de buffer en décalant les éléments du buffer
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
			int k;
			for(k=j; k<MAX_WINDOW_SIZE; k++){
				buffer[k]=NULL;
			}
			break;
		}
	}
	int window= 0;
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
