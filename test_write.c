#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <CUnit/Basic.h>

#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>		/* * sockaddr_in6 */
#include <sys/types.h>		/* sockaddr_in6 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include<sys/time.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<math.h>
#include<sys/time.h>
#include<semaphore.h>
#include<unistd.h>
#include<stdint.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>


#include "packet_interface.h"
#include "sel_repeat_write.h"

void test_modify_buffer(void);

int main(int argc, char *argv[])
{
	CU_pSuite pSuite = NULL;

	/* Initialise le catalogue de tests */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* Ajoute la suite au catalogue */
	pSuite = CU_add_suite("Suite_1", NULL, NULL);
	if (NULL == pSuite) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Ajoute les tests à la suite */
	if ((NULL == CU_add_test(pSuite, "test modify buffer", test_modify_buffer))) {
		CU_cleanup_registry();
		return CU_get_error();
	}

	/* Exécute les tests et affiche les erreurs */
	CU_basic_run_tests();
	CU_basic_show_failures(CU_get_failure_list());

	/* Libère les ressources utilisées par le catalogue */
	CU_cleanup_registry();
	return CU_get_error();

}

void test_modify_buffer(void)
{

	struct timespec a;
	struct timespec b;

	uint8_t seqnum = 0;
	struct pkt_timer *window[31];
	int i;
	for (i = 0; i < 20; i++) {

		pkt_t * packet = pkt_new();
		pkt_set_seqnum(packet, seqnum);
		seqnum = (seqnum + 1) % 256;
		struct pkt_timer bbb = { a, b, packet };
		window[i] = &bbb;

	}

	//Imaginons un ack avec comme numero de sequence 12 et qui valide donc le seqnum 11.

	int ack_position = get_ack_position(19, 11, 19);
	modify_buffer(ack_position, 19, window);
	
	for (i = 0; i < 8; i++) {
		printf("Seqnum of %d th element : %d\n", i,
		       pkt_get_seqnum(window[i]->packet));
		CU_ASSERT((pkt_get_seqnum(window[i]->packet)) == 12 + i);
	}

	seqnum = 250;

	for (i = 0; i < 31; i++) {

		window[i] = NULL;

	}

	for (i = 0; i < 20; i++) {

		pkt_t * packet = pkt_new();
		pkt_set_seqnum(packet, seqnum);
		seqnum = (seqnum + 1) % 256;
		struct pkt_timer bbb = { a, b, packet };
		window[i] = &bbb;

	}

	//Imaginons un ack avec comme numero de sequence 253 et qui valide donc le seqnum 252.

	ack_position = get_ack_position(13, 252, 19);
	modify_buffer(ack_position, 19, window);

	for (i = 0; i < 17; i++) {
		printf("Seqnum of %d th element : %d\n", i,
		       pkt_get_seqnum(window[i]->packet));
		if (i < 2) {
			CU_ASSERT(pkt_get_seqnum(window[i]->packet) == 253 + i);
		} else {
			CU_ASSERT(pkt_get_seqnum(window[i]->packet) == i - 2);
		}
	}

}
