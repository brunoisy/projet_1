#include "sel_repeat_read.h"
#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

void sel_repeat_read(const int sfd)
{

	
	int max_window_size = 16; // 2^(5-1), plutôt 15?
	int max_sdu_size = 65467;
	char buffer[max_sdu_size];


	fd_set rdfs;
	ssize_t sdu_size;
	while (1) { // Quid si EOF ?

		FD_ZERO(&rdfs);
		FD_SET(sfd, &rdfs);
		if (select(sfd + 1, &rdfs, NULL, NULL, NULL) == -1) {
			perror("select()");
			exit(errno);
		}

		if (FD_ISSET(sfd, &rdfs)) {

			sdu_size = read(sfd, buffer, max_sdu_size); // buffer contient un nouveau packet

			

			write(1, buffer, sdu_size);		
		}
	}
}
