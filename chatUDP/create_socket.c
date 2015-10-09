//#include "create_socket.h"
#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int create_socket(struct sockaddr_in6 *source_addr, int src_port,
		  struct sockaddr_in6 *dest_addr, int dst_port)
{

	int fDescr = socket(AF_INET6, SOCK_DGRAM, 17);	//17 est le nombre correspondant au protocole UDP
	if (fDescr == -1) {
		perror(strerror(errno));
		return -1;
	}

	// si serveur
	if (source_addr != NULL) {
		//on insère les numéros de ports dans les structures sockaddr
		if (src_port > 0) {
			source_addr->sin6_port = src_port;
		}
		if (bind(fDescr, (struct sockaddr *)source_addr, sizeof(struct sockaddr_in6))) {	//on écoute le client
			perror(strerror(errno));
			return -1;
		}
	}

	else {			//client

		if (bind(fDescr, (struct sockaddr *)dest_addr, sizeof(struct sockaddr_in6))) {	//on écoute le serveur, le port est assigné aléatoirement du coté client 
			perror(strerror(errno));
			return -1;
		}

		if (dst_port > 0 && dest_addr != NULL) {	//on veut se connecter à un port spécifique du serveur
			dest_addr->sin6_port = dst_port;
		}

		if (connect(fDescr, (struct sockaddr *)dest_addr, sizeof(struct sockaddr_in6))) {	// on se connecte au serveur
			perror(strerror(errno));
			return -1;
		}

	}
	return fDescr;
}
