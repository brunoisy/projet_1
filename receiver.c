#include "packet_interface.h"
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include "chatUDP/wait_for_client"
#include "chatUDP/real_address"
#include "chatUDP/create_socket"

int main(int argc, char *argv[])
{

	char * hostname= argv[1];
	int port = atoi(argv[2]);


	/* Resolve the hostname */
	struct sockaddr_in6 addr;
	const char *err = real_address(hostname, &addr);
	if (err) {
		fprintf(stderr, "Could not resolve hostname %s: %s\n", host,
			err);
		return EXIT_FAILURE;
	}

	/* Get a socket */
	int sfd= create_socket(NULL, -1, &addr, port);	/* Connected */
	if (sfd < 0) {
		fprintf(stderr, "Failed to create the socket!\n");
		return EXIT_FAILURE;
	}
	



	sel_repeat_read(sfd);

	close(sfd);
	return EXIT_SUCCESS;
}
