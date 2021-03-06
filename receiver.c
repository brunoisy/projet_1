#include "real_address.h"
#include "create_socket.h"
#include "sel_repeat_read.h"
#include "wait_for_client.h"

#include <zlib.h>
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
	int sfd = create_socket(&addr, port, NULL, -1); /* Bound */
	if (sfd > 0 && wait_for_client(sfd) < 0) { /* Connected */
		fprintf(stderr,
				"Could not connect the socket after the first message.\n");
		close(sfd);
		return EXIT_FAILURE;
	}
        

	sel_repeat_read(sfd);

	close(sfd);
	return EXIT_SUCCESS;
}
