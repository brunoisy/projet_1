#include "sel_repeat_read.h"
#include "packet_interface.h"


#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

void printBuffer(pkt_t * receive_buffer[MAX_WINDOW_SIZE]){
	printf("buffer post refresh\n");
	int i;
	for(i=0; i<MAX_WINDOW_SIZE; i++){
		if(receive_buffer[i]==NULL){
			break;
		}
		printf("%d - ", pkt_get_seqnum(receive_buffer[i]));
	}
	printf("\n");

}


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
		sdu_size = read(sfd, pkt_buffer, max_sdu_size); // buffer contient un nouveau packet



		pkt_t * pkt = pkt_new();
		err=pkt_decode(pkt_buffer, sdu_size, pkt);
printf("ptype packet : %d, ", pkt_get_type(pkt));
printf("window packet : %d, ", pkt_get_window(pkt));
printf("seqnum packet : %d, seqnum attendu : %d\n", pkt_get_seqnum(pkt), (lastack+1)%256);
printf("length packet : %d\n", pkt_get_length(pkt));
		if(err){
printf(" error : %d\n", err);
			if(err!=E_NOHEADER){
printf("!=NOHEADER\n");
				send_nack(pkt_get_seqnum(pkt), sfd, window_size);
			}
                        printf("error in decode\n");
			free(pkt);
			pkt==NULL;
		}




		else if(compare_seqnums(lastack, pkt_get_seqnum(pkt), lastack+1) < window_size){ //si le numéro de seq est dans la fenêtre
printf("ingoodwindow\n");
			if(pkt_get_seqnum(pkt)==(lastack+1)%256){//si le packet est celui attendu
				write_payload(1, pkt);
				pkt_del(pkt);
printf("pkt_del\n");
				remove_from_buffer(receive_buffer, (lastack+1)%256);
				lastack=(lastack+1)%256;
				send_ack(lastack, sfd, window_size);
				
			}
			else{
printf("insert_in_buffer\n");
				insert_pkt(lastack, receive_buffer, pkt);
printf("inserted\n");
				window_size--;
			}
		}
		else{
	
printf("pkt_del\n");
			send_nack(pkt_get_seqnum(pkt), sfd, window_size);
			pkt_del(pkt);
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
printf("pkt_del\n");
				receive_buffer[i]=NULL;
				window_size++;
				lastack=(lastack+1)%256;
				send_ack(lastack, sfd, window_size);
			}
			else{
				break;
			}
		}
		send_ack(lastack, sfd, window_size);// par sécurité
printf("before refresh\n");
		refresh(receive_buffer); //mettre a jour receive_buffer en enlevant les éléments lus, renvoi nbr élements vides
		printBuffer(receive_buffer);

		printf("_______________________________________________________________________________\n");
	}
	
}


/*
*
*
*
*/
int remove_from_buffer(pkt_t * receive_buffer[MAX_WINDOW_SIZE], int seqnum){
	if(receive_buffer[0]!=NULL){
		if(pkt_get_seqnum(receive_buffer[0])==seqnum){
			pkt_del(receive_buffer[0]);
printf("pkt_del\n");
			refresh(receive_buffer);
		}
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
	free(pkt);
	pkt=NULL;
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
	free(pkt);
	pkt=NULL;
	write(sfd, data, length);
}


/*
* insère pkt dans buffer en créant un nouveau pointeur vers pkt et en respectant l'ordre des seqnums (buffer reste trié)
* si le pkt est déjà dans le buffer, il n'est pas inséré et est free
*
*/
int insert_pkt(int lastack, pkt_t * buffer[MAX_WINDOW_SIZE], pkt_t * pkt){
	int i;
	for(i=0; i<MAX_WINDOW_SIZE; i++){
		if(buffer[i]==NULL){
			buffer[i]=pkt;
			break;
		}
		else if(compare_seqnums(lastack, pkt_get_seqnum(pkt), pkt_get_seqnum(buffer[i])) == 0){// le packet est déjà dans le buffer
			pkt_del(pkt);
printf("pkt_del\n");
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
