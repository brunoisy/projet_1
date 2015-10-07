#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include "packet_interface.h"


void test_new(void);
void test_setters(void);
void test_getters(void);
void test_del(void);
void test_encode(void);
void test_decode(void);

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
	if ((NULL == CU_add_test(pSuite, "test setters", test_setters)) ||
	    (NULL == CU_add_test(pSuite, "test getters", test_getters)) ||
	    (NULL == CU_add_test(pSuite, "test del", test_del)) ||
	    (NULL == CU_add_test(pSuite, "test encode", test_encode)) ||
	    (NULL == CU_add_test(pSuite, "test decode", test_decode))) {
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



void test_setters(void)
{
	pkt_t * pkt = pkt_new();
	const ptypes_t type = PTYPE_DATA;
	const uint8_t window = 19;
	const uint8_t seqnum = 5;
	const uint16_t length = 34;
	const uint32_t crc = 123456;

	const ptypes_t badType = 7;
	const uint8_t  badWindow = 34;
	const uint16_t badLength = 515;
	//pas de badSeqnum, badCrc, impossible.

	CU_ASSERT(pkt_set_type(pkt, type)==PKT_OK);
	CU_ASSERT(pkt_set_window(pkt, window)==PKT_OK);
	CU_ASSERT(pkt_set_seqnum(pkt, seqnum)==PKT_OK);
	CU_ASSERT(pkt_set_length(pkt, length)==PKT_OK);
	CU_ASSERT(pkt_set_crc(pkt, crc)==PKT_OK);

	
	CU_ASSERT(pkt_set_type(pkt, badType)==E_TYPE);
	CU_ASSERT(pkt_set_window(pkt, badWindow)==E_WINDOW);
	CU_ASSERT(pkt_set_length(pkt, badLength)==E_LENGTH);
}

void test_getters(void)
{
}

void test_del(void)
{
}

void test_encode(void)
{
}

void test_decode(void)
{
}

/*
	pkt_t *pkt = pkt_new();
	pkt_t *pkt2 = pkt_new();

	pkt_set_type(pkt, PTYPE_ACK);
	pkt_set_window(pkt, 3);
	pkt_set_seqnum(pkt, 4);
	char *data = (char *)malloc(2);
	data[0] = 'a';
	data[1] = 'r';
	if (pkt_set_payload(pkt, data, 2) != PKT_OK) {
		printf("ERROR SET PAYLOAD\n");
	}

	int padding = 0;
	if (pkt_get_length(pkt) % 4 != 0) {
		padding = 4 - (pkt_get_length(pkt) % 4);
	}
	size_t buffersize = 8 + pkt_get_length(pkt) + padding;	// taille fixe + taille payload

	char *buffer = (char *)malloc((size_t) buffersize);

	printf("inencode\n");
	pkt_encode(pkt, buffer, &buffersize);
	printf("outencode\n");

	printf("buffer après encode\n");
	int i;
	for (i = 1; i <= pkt_get_length(pkt) + padding + 8; i++) {
		printBits(1, &buffer[i - 1]);
		if (i % 4 == 0 && i != 0) {
			printf("\n");
		}
	}

	printf("indecode\n");
	pkt_decode(buffer, buffersize, pkt2);
	printf("outdecode\n");

	printf("type %d, window %d, seqnum %d, length %d, crc %u\n",
	       pkt_get_type(pkt2), pkt_get_window(pkt2), pkt_get_seqnum(pkt2),
	       pkt_get_length(pkt2), pkt_get_crc(pkt2));

	free(buffer);
	pkt_del(pkt);
	pkt_del(pkt2);
}
*/
