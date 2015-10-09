#include "real_address.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

const char *real_address(const char *address, struct sockaddr_in6 *rval)
{

	struct addrinfo *myaddrinfo =
	    (struct addrinfo *)malloc(sizeof(struct addrinfo));
	int err = getaddrinfo(address, NULL, NULL, &myaddrinfo);	//service=NULL node=address ??
	struct sockaddr *addr = myaddrinfo->ai_addr;
	struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
	*rval = *addr6;

	free(myaddrinfo);
	if (err != 0) {
		return strerror(errno);
	}
	return NULL;
}
