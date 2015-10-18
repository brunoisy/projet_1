#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <CUnit/Basic.h>
#include "packet_interface.h"

void test_getters_setters(void);
void test_encode_decode(void);
int comparePayloads(void *, void *);

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
	if ((NULL == CU_add_test(pSuite, "test setters", test_getters_setters))
	    || (NULL == CU_add_test(pSuite, "test encode", test_encode_decode))) {
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

void test_getters_setters(void)
{
	pkt_t *pkt = pkt_new();

	const ptypes_t type = PTYPE_DATA;
	const uint8_t window = 19;
	const uint8_t seqnum = 7;
	const uint16_t length1 = 34;
	const uint16_t length2 = 5;
	const uint32_t crc = 123465;
	const char *payload = "abcd";

	const ptypes_t badType = 7;
	const uint8_t badWindow = 34;
	const uint16_t badLength = 515;
	//pas de badSeqnum, badCrc, impossible.

	// sets corrects 
	CU_ASSERT(pkt_set_type(pkt, type) == PKT_OK);
	CU_ASSERT(pkt_set_window(pkt, window) == PKT_OK);
	CU_ASSERT(pkt_set_seqnum(pkt, seqnum) == PKT_OK);
	CU_ASSERT(pkt_set_length(pkt, length1) == PKT_OK);
	CU_ASSERT(pkt_set_crc(pkt, crc) == PKT_OK);
	CU_ASSERT(pkt_set_payload(pkt, payload, length2) == PKT_OK);

	// gets
	CU_ASSERT(pkt_get_type(pkt) == type);
	CU_ASSERT(pkt_get_window(pkt) == window);
	CU_ASSERT(pkt_get_seqnum(pkt) == seqnum);
	CU_ASSERT(pkt_get_length(pkt) == length2);	//on vérifie que la longeur a bien été modifiée par setPayload
	CU_ASSERT(pkt_get_crc(pkt) == crc);
	//vérifier get payload? 

	CU_ASSERT(pkt_set_type(pkt, badType) == E_TYPE);
	CU_ASSERT(pkt_set_window(pkt, badWindow) == E_WINDOW);
	CU_ASSERT(pkt_set_length(pkt, badLength) == E_LENGTH);
	CU_ASSERT(pkt_set_payload(pkt, payload, badLength) == E_LENGTH);

	pkt_del(pkt);		//éviter memoryleaks
}

void test_encode_decode(void)
{

	pkt_t *pkt = pkt_new();
	pkt_t *pkt2 = pkt_new();

	pkt_set_type(pkt, PTYPE_DATA);
	pkt_set_window(pkt, 3);
	pkt_set_seqnum(pkt, 4);
	char data[2];
	data[0] = 'a';
	data[1] = 'r';

	pkt_set_payload(pkt, data, 2);

	int padding = 0;
	if (pkt_get_length(pkt) % 4 != 0) {
		padding = 4 - (pkt_get_length(pkt) % 4);
	}
	size_t buffersize = 8 + pkt_get_length(pkt) + padding;	// taille fixe + taille payload

	char buffer[buffersize];

	pkt_encode(pkt, buffer, &buffersize);
	CU_ASSERT(buffersize==8 + pkt_get_length(pkt) + padding);

	char goodbuffer [buffersize]; // buffer sensé être obtenu après encode
	goodbuffer[0]=0b00100011;
	goodbuffer[1]=0b00000100;
	goodbuffer[2]=0b00000000;
	goodbuffer[3]=0b00000010;
	goodbuffer[4]=0b01100001;
	goodbuffer[5]=0b01110010;
	goodbuffer[6]=0b00000000;
	goodbuffer[7]=0b00000000;
	
	uint32_t goodcrc = (uint32_t)crc32(0, (const Bytef *)goodbuffer, 8);
	uint32_t bigendiancrc = htonl(goodcrc);
	goodbuffer[8]=(char)(bigendiancrc);
	goodbuffer[9]=(char)(bigendiancrc >> 8);
	goodbuffer[10]=(char)(bigendiancrc >> 16);
	goodbuffer[11]=(char)(bigendiancrc >> 24);

	CU_ASSERT(compareData(buffer,goodbuffer, 12)==0);


	pkt_decode(buffer, buffersize, pkt2);

	

	CU_ASSERT(pkt_get_type(pkt)==pkt_get_type(pkt2));
	CU_ASSERT(pkt_get_window(pkt)==pkt_get_window(pkt2));
	CU_ASSERT(pkt_get_seqnum(pkt)==pkt_get_seqnum(pkt2));
	CU_ASSERT(pkt_get_length(pkt)==pkt_get_length(pkt2));	
	CU_ASSERT(compareData(pkt_get_payload(pkt),pkt_get_payload(pkt2), (int)pkt_get_length(pkt))==0);
	CU_ASSERT(pkt_get_crc(pkt2)==goodcrc);

	
	pkt_del(pkt);
	pkt_del(pkt2);

}

/* cette fonction renvoie 0 si les "lengths" premières données de x1 et x2 sont les mêmes, -1 si non
*/
int compareData(char * x1, char * x2, int length){
	int i;
	for(i=0; i<length; i++){
		if(x1[i] != x2[i]){
			return -1;
		}
	}

	return 0;	
}
