#include "wait_for_client.h"

#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int wait_for_client(int sfd)
{
	perror("inwaitforclient");

	struct sockaddr *source =
	    (struct sockaddr *)malloc(sizeof(struct sockaddr));
	//uint32_t length = (uint32_t)sizeof(struct sockaddr); 
	socklen_t length = (socklen_t) sizeof *source;

	if (recvfrom(sfd, NULL, 0, 0, source, &length)) {
		perror(strerror(errno));
		perror("!!!!CARAMBA!");
		free(source);

		return -1;

	}
	if (connect(sfd, source, length)) {
		perror(strerror(errno));
		free(source);
		perror("CARAMBA!");
		return -1;
	}

	free(source);
	perror("wait_for_client_retourne_0");
	return 0;

}
