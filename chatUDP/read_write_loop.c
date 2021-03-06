#include "read_write_loop.h"
#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

void read_write_loop(const int sfd)
{

	fd_set rdfs;

	int max_msg_size = 1024;
	char buffer[max_msg_size];

	ssize_t msg_size;
	while (stdin) { // Quid si EOF ?

		FD_ZERO(&rdfs);
		FD_SET(0, &rdfs);
		FD_SET(sfd, &rdfs);
		if (select(sfd + 1, &rdfs, NULL, NULL, NULL) == -1) {
			perror("select()");
			exit(errno);
		}

		if (FD_ISSET(0, &rdfs)) {

			msg_size = read(0, buffer, max_msg_size);

			write(sfd, buffer, msg_size);
			

		} else if (FD_ISSET(sfd, &rdfs)) {

			msg_size = read(sfd, buffer, max_msg_size);

			write(1, buffer, msg_size);
			
		}
	}
}
