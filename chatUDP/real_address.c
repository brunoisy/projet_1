#include "real_address.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char *real_address(const char *address, struct sockaddr_in6 *rval)
{

	struct addrinfo *myaddrinfo =
	    (struct addrinfo *)malloc(sizeof(struct addrinfo));
	int err = getaddrinfo(address, NULL, NULL, &myaddrinfo);
	struct sockaddr *addr = myaddrinfo->ai_addr;
	*rval = *((struct sockaddr_in6 *)addr);
	 

	free(myaddrinfo);
	if (err != 0) {
		return strerror(errno);
	}

	return NULL;
}
