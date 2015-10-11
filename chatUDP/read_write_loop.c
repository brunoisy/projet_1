#include "read_write_loop.h"

#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

void read_write_loop(int sfd)
{

	perror("inreadwriteloop");
	fd_set rdfs;

	int max_msg_size = 1024;
	char buffer[max_msg_size];
	char buffer2[max_msg_size];

	FD_ZERO(&rdfs);
	FD_SET(STDIN_FILENO, &rdfs);
	FD_SET(sfd, &rdfs);

	ssize_t msg_size;
	while (1) {
		perror("INwhile");

		if (select(sfd + 1, &rdfs, NULL, NULL, NULL) == -1) {
			perror("errorSelect()");
			exit(errno);
		}

		if (FD_ISSET(STDIN_FILENO, &rdfs)) {
			perror("instdinread");

			msg_size = recv(STDIN_FILENO, buffer2, max_msg_size, 0);
			perror("msg_lu");
			send(sfd, buffer2, msg_size, 0);
			perror("msg_ecrit dans socket");
			if (buffer2[msg_size - 1] == EOF) {	// faut il lire un EOF des msgs lus du socket aussi?
				exit(EXIT_SUCCESS);
			}
		}

		if (FD_ISSET(sfd, &rdfs)) {	//length = (socklen_t)sizeof sin;
			perror("sfd");
			msg_size = recv(sfd, buffer, max_msg_size, 0);
			send(STDOUT_FILENO, buffer, msg_size, 0);
			if (buffer[msg_size - 1] == EOF) {	// faut il lire un EOF des msgs lus du socket aussi?
				exit(EXIT_SUCCESS);
			}
		}
	}
}
