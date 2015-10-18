#include "real_address.h"
#include "create_socket.h"
#include "sel_repeat_write.h"

#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{

	char *hostname = argv[1];
	int port = atoi(argv[2]);

	if (argc == 4) {
		fprintf(stderr, "Invalid number of arguments\n");
		return EXIT_FAILURE;
	}

	char *filename = NULL;

	if (argc == 5) {
		char *option = argv[3];
		if (strcmp(option, "-f") != 0
		    && strcmp(option, "--filename") != 0) {
			fprintf(stderr, "Invalid argument %s\n", option);
			return EXIT_FAILURE;
		}
		filename = argv[4];
	}

	struct sockaddr_in6 addr;

	const char *error = real_address(hostname, &addr);

	if (error != NULL) {
		fprintf(stderr, "Could not resolve hostname %s\n", hostname);
		return EXIT_FAILURE;
	}

	int socket = create_socket(NULL, -1, &addr, port);

	if (socket < 0) {
		fprintf(stderr, "Failed to create the socket!\n");
		return EXIT_FAILURE;
	}

	int fd = 0;
	if (filename != NULL) {
		fd = open(filename, O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, "File '%s' does not exist\n", filename);
		}
	}
	printf("Before write\n");
	sel_repeat_write(fd, socket);

}
