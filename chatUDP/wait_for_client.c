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
	
	struct sockaddr_in6 *source =
	    (struct sockaddr_in6 *)malloc(sizeof(struct sockaddr_in6));
	
	socklen_t length = (socklen_t) sizeof *source;

	if (recvfrom
	    (sfd, NULL, 0, MSG_PEEK, (struct sockaddr *)source, &length)) {

		return -1;

	}

	if (connect(sfd, (struct sockaddr *)source, length)) {

		return -1;
	}

	return 0;

}
