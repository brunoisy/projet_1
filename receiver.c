#include "chatUDP/real_address.c"
#include "chatUDP/create_socket.c"
#include "sel_repeat_read.c"
#include "chatUDP/wait_for_client.c"


//#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>


int main(int argc, char *argv[])
{

	char * hostname= argv[1];
	int port = atoi(argv[2]);


	/* Resolve the hostname */
	struct sockaddr_in6 addr;
	const char *err = real_address(hostname, &addr);
	if (err) {
		fprintf(stderr, "Could not resolve hostname %s: %s\n", hostname,
			err);
		return EXIT_FAILURE;
	}

	/* Get a socket */
	int sfd= create_socket(&addr, port, NULL, -1);	/* Connected */
	if (sfd < 0) {
		fprintf(stderr, "Failed to create the socket!\n");
		return EXIT_FAILURE;
	}
        


printf("before_sel_read\n");
	sel_repeat_read(sfd);

	close(sfd);
	return EXIT_SUCCESS;
}
