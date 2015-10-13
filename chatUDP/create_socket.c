#include "create_socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
int
create_socket(struct sockaddr_in6 *source_addr, int src_port,
	      struct sockaddr_in6 *dest_addr, int dst_port)
{

	int fDescr = socket(AF_INET6, SOCK_DGRAM, 17);	//17 est le nombre correspondant au protocole UDP
	if (fDescr == -1) {
		perror("socket");

		perror(strerror(errno));
		return -1;
	}

	// si serveur
	if (source_addr != NULL) {
		//on insère les numéros de ports dans les structures sockaddr
		if (src_port > 0) {
			source_addr->sin6_port = htons(src_port);
		}
		if (bind(fDescr, (struct sockaddr *)source_addr, sizeof(*source_addr))) {	//on écoute le client
			perror("bindtoclient");

			perror(strerror(errno));
			return -1;
		}
	}

	else {			// si client

		if (bind(fDescr, (struct sockaddr *)dest_addr, sizeof(*dest_addr))) {	//on écoute le serveur, le port est assigné aléatoirement du coté client 
			perror("bindtoserver");

			perror(strerror(errno));
			return -1;
		}

		if (dst_port > 0 && dest_addr != NULL) {	//on veut se connecter à un port spécifique du server
			dest_addr->sin6_port = htons(dst_port);	        
		}

		if (connect(fDescr, (struct sockaddr *)dest_addr, sizeof(*dest_addr))) {	// on se connecte au serveur
			perror("connecttoserver");

			perror(strerror(errno));
			return -1;
		}

	}

	return fDescr;
}
