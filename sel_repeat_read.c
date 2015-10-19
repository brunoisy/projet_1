#include "packet_interface.h"

#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
/*
void printBits(size_t const size, void const *const ptr)
{
	unsigned char *b = (unsigned char *)ptr;
	unsigned char byte;
	int i, j;

	for (i = size - 1; i >= 0; i--) {
		for (j = 7; j >= 0; j--) {
			byte = b[i] & (1 << j);
			byte >>= j;
			printf("%u", byte);
		}
	}
	printf(" ");
}

void printBuffer(pkt_t * receive_buffer[MAX_WINDOW_SIZE])
{

	int i;
	for (i = 0; i < MAX_WINDOW_SIZE; i++) {
		if (receive_buffer[i] == NULL) {
			break;
		}
		printf("%d - ", pkt_get_seqnum(receive_buffer[i]));
	}
	printf("\n");

}
*/
void sel_repeat_read(const int sfd)
{
	int lastack = 255;
	int window_size = MAX_WINDOW_SIZE;
	int max_sdu_size = MAX_PAYLOAD_SIZE + 8;
	char pkt_buffer[max_sdu_size];
	pkt_t *receive_buffer[MAX_WINDOW_SIZE];
	int j;

	for (j = 0; j < MAX_WINDOW_SIZE; j++) {
		receive_buffer[j] = NULL;
	}
	pkt_status_code err;
	while (1) {
		ssize_t sdu_size = read(sfd, pkt_buffer, max_sdu_size);	// bufer contient un nouveau packet
		pkt_t *pkt = pkt_new();
		err = pkt_decode(pkt_buffer, sdu_size, pkt);
		if (err == E_NOHEADER) {	// Si seqnum n'est pas dans la fenetre
			pkt_del(pkt);
		} else if (compare_seqnums(lastack, pkt_get_seqnum(pkt), (lastack + 1) % 256) < MAX_WINDOW_SIZE) {	//si le numéro de seq est dans la fenêtre                 // Si il y a au moins le header
			printf("seqnum packet : %d, seqnum attendu : %d\n",
			       pkt_get_seqnum(pkt), (lastack + 1) % 256);

			if (err) {
				printf("PACKET INVALID\n");
				if (err == E_UNCONSISTENT && sdu_size == 4) {
					printf
					    ("RECEIVED ONLY HEADER. NACK SENT.\n");
					send_nack(pkt_get_seqnum(pkt),
						  sfd, window_size);
				}
				free(pkt);
				pkt == NULL;
			}

			else if (pkt_get_seqnum(pkt) == (lastack + 1) % 256) {	//si le packet est celui attendu
				write_payload(1, pkt);
				pkt_del(pkt);
				remove_from_buffer(receive_buffer,
						   (lastack + 1) % 256,
						   &window_size);

				lastack = (lastack + 1) % 256;

				int i;

				for (i = 0; i < MAX_WINDOW_SIZE; i++) {	// on écrit les packets de receive buffer qui sont succéssifs à lastack
					if (receive_buffer[i] == NULL) {
						send_ack(lastack, sfd,
							 window_size);
						break;
					}
					if (pkt_get_seqnum(receive_buffer[i]) ==
					    (lastack + 1) % 256) {
						write_payload(1, receive_buffer[i]);	// 1 ou -f ?
						pkt_del(receive_buffer[i]);
						receive_buffer[i] = NULL;
						window_size++;
						
						lastack = (lastack + 1) % 256;
						send_ack(lastack, sfd,
							 window_size);

					} else {
						send_ack(lastack, sfd,
							 window_size);
						break;
					}
				}

			} else {
				insert_pkt(lastack, receive_buffer, pkt,
					   &window_size);
			}

		}
		refresh(receive_buffer);	//mettre a jour receive_buffer en enlevant les éléments lus, renvoi nbr élements vides

		
		printf
		    ("_______________________________________________________________________________\n");
	}

}

/*
*
* renvoi 1 si enlevé du buffer
*
*/
int remove_from_buffer(pkt_t * receive_buffer[MAX_WINDOW_SIZE], int seqnum,
		       int *window_size)
{
	if (receive_buffer[0] != NULL) {
		if (pkt_get_seqnum(receive_buffer[0]) == seqnum) {
			pkt_del(receive_buffer[0]);
			receive_buffer[0] == NULL;
			refresh(receive_buffer);
			*window_size = *window_size + 1;
		}
	}
}

/*
* écrit dans fds le contenu du payload de pkt
*
*/
int write_payload(int fds, pkt_t * pkt)
{
	write(fds, pkt_get_payload(pkt), pkt_get_length(pkt));
}

/*
* envoi un packet ack au sender, contenant le numéro de seqnum du dernier packet reçu
*/

int send_ack(int lastack, int sfd, int window_size)
{
	pkt_t *pkt = pkt_new();
	pkt_set_type(pkt, PTYPE_ACK);
	pkt_set_window(pkt, window_size);
	pkt_set_seqnum(pkt, (lastack + 1) % 256);
	pkt_set_length(pkt, 0);

	size_t length = 8;
	char data[length];
	pkt_encode(pkt, data, &length);
	free(pkt);
	pkt = NULL;
	write(sfd, data, length);
}

/*
* envoi un packet nack au sender, contenant le numéro de seqnum du packet non reçu
*
*/
int send_nack(int seqnum, int sfd, int window_size)
{
	pkt_t *pkt = pkt_new();
	pkt_set_type(pkt, PTYPE_NACK);
	pkt_set_window(pkt, window_size);
	pkt_set_seqnum(pkt, seqnum);
	pkt_set_length(pkt, 0);

	size_t length = 8;
	char data[length];
	pkt_encode(pkt, data, &length);
	
	free(pkt);
	pkt = NULL;

	write(sfd, data, length);
}

/*
* insère pkt dans buffer en créant un nouveau pointeur vers pkt et en respectant l'ordre des seqnums (buffer reste trié)
* si le pkt est déjà dans le buffer, il n'est pas inséré et est free
*
*/
int insert_pkt(int lastack, pkt_t * buffer[MAX_WINDOW_SIZE], pkt_t * pkt,
	       int *window_size)
{
	int i;
	for (i = 0; i < MAX_WINDOW_SIZE; i++) {
		if (buffer[i] == NULL) {
			buffer[i] = pkt;
			*window_size=*window_size-1;

			break;
		} else if (compare_seqnums(lastack, pkt_get_seqnum(pkt), pkt_get_seqnum(buffer[i])) == 0) {	// le packet est déjà dans le buffer
			pkt_del(pkt);
			break;
		} else
		    if (compare_seqnums
			(lastack, pkt_get_seqnum(pkt),
			 pkt_get_seqnum(buffer[i])) < 0) {
			pkt_t *this = buffer[i];
			pkt_t *next;
			buffer[i] = pkt;
			int j;
			for (j = 1; j < MAX_WINDOW_SIZE - i; j++) {
				next = buffer[i + j];
				buffer[i + j] = this;
				this = next;

			}
			*window_size=*window_size-1;

			break;
		}
	}
}

/*
* enlève tous les NULLs se situant en début de buffer en décalant les éléments du buffer
*
*/
int refresh(pkt_t * buffer[MAX_WINDOW_SIZE])
{
	int i;
	for (i = 0; i < MAX_WINDOW_SIZE; i++) {
		if (buffer[i] != NULL) {
			int j;
			for (j = 0; j + i < MAX_WINDOW_SIZE; j++) {
				buffer[j] = buffer[j + i];
			}
			int k;
			for (k = j; k < MAX_WINDOW_SIZE; k++) {
				buffer[k] = NULL;
			}
			break;
		}

	}
}

/*
* renvoie la différence entre seqnum1 et seqnum2 en prenant en compte la succéssion circulaire des seqnums
*
*/
int compare_seqnums(int lastack, int seqnum1, int seqnum2)
{
	if (seqnum1 <= lastack) {
		seqnum1 = seqnum1 + 256;
	}
	if (seqnum2 <= lastack) {
		seqnum2 = seqnum2 + 256;
	}
	return seqnum1 - seqnum2;
}
