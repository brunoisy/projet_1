#include "sel_repeat_write.h"
#include "packet_interface.h"


#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include<sys/time.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<math.h>
#include<sys/time.h>
#include<semaphore.h>
#include<unistd.h>
#include<stdint.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>

struct pkt_timer {

	struct timespec start;
	struct timespec finish;
	pkt_t *packet;

};

void print_buffer(struct pkt_timer * buffer[], int n){
	int i;
	for(i=0;i<n;i++){

		if(buffer[i]!=NULL && buffer[i]->packet != NULL){
			printf("Seqnum of %d th element = %d\n",i,pkt_get_seqnum(buffer[i]->packet));
			//printf("Length of %d th element = %d\n",i,pkt_get_length((buffer[i])->packet));
		}
		else{
			printf("Case number %d is empty",i);
		}
	}
}



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

void modify_buffer(int ack, int last, struct pkt_timer *buffer[])
{

	int i;
	for (i = ack; i >= 0; i--) {

		pkt_del(buffer[i]->packet);
		buffer[i] = NULL;

	}

	for (i = 0; i < (last - ack); i++) {

		buffer[i] = buffer[ack + 1 + i];

	}

	for (i = last; i >= (last - ack); i--) {

		buffer[i] = NULL;

	}
}

int get_ack_position(last_seqnum_send, seqnum_validated, pointeur_last_send)
{

	if (last_seqnum_send >= seqnum_validated) {

		return pointeur_last_send - (last_seqnum_send -
					     seqnum_validated);

	} else {

		return pointeur_last_send - last_seqnum_send -
		    (256 - seqnum_validated);

	}

}

void sel_repeat_write(int fd, int socket)
{

	uint8_t max_window_size = 31;
	int max_payload_size = 512;
	int max_sdu_size = 520;
	char buffer[max_payload_size];
	int real_payload_size;
	int limited_window_size = max_window_size;
	uint8_t seqnum = 0;
	struct pkt_timer *window[max_window_size];
	int current_index_window = 0;
        int finish=0;

	fd_set rdfs;
	ssize_t sdu_size;

	int max_fd;
	if (socket > fd) {
		max_fd = socket;
	} else {
		max_fd = fd;
	}

	while (1) {
		int i;
		for (i = 0; i < current_index_window; i++) {

			clock_gettime(CLOCK_MONOTONIC, &(window[i]->finish));

			if (window[i]->finish.tv_sec -
			    window[i]->start.tv_sec > 5.0) {
				clock_gettime(CLOCK_MONOTONIC,
					      &(window[i]->start));
				size_t data_size = max_sdu_size;
				char data[data_size];
				pkt_encode(window[i]->packet, data, &data_size);
				write(socket, data, data_size);

			}
		}


		FD_ZERO(&rdfs);
		if((current_index_window<limited_window_size && finish==0)||(limited_window_size == 0 && current_index_window==0 && finish==0)){ //On peut envoyer un packet
	       // printf("on met le fd du fichier/stdin dans rdfs\n");
		FD_SET(fd, &rdfs);
		}
		FD_SET(socket,&rdfs);

                struct timeval timer;
                timer.tv_sec=1;
                timer.tv_usec=0;
		if (select(max_fd + 1, &rdfs, NULL, NULL, &timer)
		    == -1) {
			fprintf(stderr, "Error select");
			exit(errno);
		}


		if (FD_ISSET(fd, &rdfs)) {

			real_payload_size = read(fd, buffer, max_payload_size);
                        if(real_payload_size == 0){
                        finish = 1;
                        }
			size_t data_size = max_sdu_size;
			char data[data_size];
			pkt_t *packet = pkt_new();
			pkt_set_type(packet, PTYPE_DATA);
			pkt_set_seqnum(packet, seqnum);
			printf("seqnum : %d \n", pkt_get_seqnum(packet));
			pkt_set_window(packet, 0);
			pkt_set_length(packet, (uint16_t)
				       real_payload_size);
			pkt_set_payload(packet, buffer, real_payload_size);
			pkt_encode(packet, data, &data_size);

			seqnum = (seqnum + 1) % 256;

			struct timespec start;
			clock_gettime(CLOCK_MONOTONIC, &start);
			struct timespec finish;
			struct pkt_timer *element = (struct pkt_timer *)
			    malloc(sizeof(struct pkt_timer));
			element->start = start;
			element->finish = finish;
			element->packet = packet;

			window[current_index_window] = element;
			current_index_window++;
			printf("lecture et ecriture de 1 packet\n");
			write(socket, data, data_size);

		}

		if (FD_ISSET(socket, &rdfs)) {

			char buffer[8];
			int length = read(socket, buffer, 8);
			pkt_t *packet = pkt_new();
			int status = pkt_decode(buffer, length, packet);
			if (status == PKT_OK) {

				ptypes_t type = pkt_get_type(packet);
				int last_seqnum_send = seqnum - 1;
				int pointeur_last_send =
				    current_index_window - 1;

				if (type == PTYPE_ACK) {

					int seqnum_validated = pkt_get_seqnum(packet) - 1;
					  printf("ack seqnum: %d\n",seqnum_validated+1);
					  
					  int ack_position = get_ack_position(last_seqnum_send,seqnum_validated,pointeur_last_send);
					  
					  //printf("ack position in the buffer: %d\n",ack_position);
					  //printf("pointeur_last_send:%d\n", pointeur_last_send);
					  //printf("last_seqnum_send:%d\n", last_seqnum_send);
					  printf("seqnum_validated:%d\n", seqnum_validated);
					  limited_window_size = pkt_get_window(packet);
					  printf("limited window size: %d\n", limited_window_size);
					  printf("Buffer before modify_buffer:\n");
					  print_buffer(window,current_index_window);
					  
					  modify_buffer(ack_position,pointeur_last_send,window);
					  printf("Buffer after modify_buffer:\n");
					  current_index_window = current_index_window - ack_position -1;
					  print_buffer(window,current_index_window);
                                        if(current_index_window == 0 && finish==1){
                                            exit(EXIT_SUCCESS);
                                        }

				} else {

					if (type == PTYPE_NACK) {
printf("NACK received\n");
						limited_window_size
						    = pkt_get_window(packet);
						int seqnum_to_send_again
						    = pkt_get_seqnum(packet);
						int nack_position;
						if (last_seqnum_send >=
						    seqnum_to_send_again) {

							nack_position
							    =
							    pointeur_last_send
							    -
							    (last_seqnum_send
							     -
							     seqnum_to_send_again);

						} else {

							nack_position
							    =
							    pointeur_last_send
							    -
							    last_seqnum_send
							    -
							    (256
							     -
							     seqnum_to_send_again);

						}

						size_t data_size = max_sdu_size;
						char data[data_size];
						pkt_encode
						    (window
						     [nack_position]->packet,
						     data, &data_size);
						clock_gettime
						    (CLOCK_MONOTONIC,
						     &(window
						       [nack_position]->start));
						write(socket, data, data_size);

					}

				}

			}

		}
	}
}
