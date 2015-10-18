#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <CUnit/Basic.h>
#include "sel_repeat_write.c"

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

void test_modify_buffer(void){


 struct timepsec a;
 struct timespec b;
 
 uint8_t seqnum = 0;
 struct pkt_timer * window[31];
 
 for(int i=0;i<20;i++){
  
  pkt_t packet = pkt_new();
  pkt_set_seqnum(&packet,seqnum);
  seqnum = (seqnum+1)%256;
  pkt_timer bbb = {a,b,packet};
  window[i] = &bbb;

 }

 //Imaginons un ack avec comme numero de sequence 12 et qui valide donc le seqnum 11.
 
 int ack_position = get_ack_position(19,11,19);
 modify_buffer(ack_postion,19,window);

 for(int i=0;i<8;i++){
 printf("Seqnum of %d th element : %d\n", i, window[i]->packet->seqnum);
 CU_ASSERT(window[i]->packet->seqnum == 12+i);
 }

 seqnum = 250;

 for(int i=0;i<31;i++){
  
  
  window[i] = NULL;

 }

 for(int i=0;i<20 ;i++){
  
  pkt_t packet = pkt_new();
  pkt_set_seqnum(&packet,seqnum);
  seqnum = (seqnum+1)%256;
  pkt_timer bbb = {a,b,packet};
  window[i] = &bbb;

 }

 
 //Imaginons un ack avec comme numero de sequence 253 et qui valide donc le seqnum 252.
 
 int ack_position = get_ack_position(13,252,19);
 modify_buffer(ack_postion,19,window);

 for(int i=0;i<17;i++){
 printf("Seqnum of %d th element : %d\n", i, window[i]->packet->seqnum);
 if(i<2){
 CU_ASSERT(window[i]->packet->seqnum == 253+i);
 }else{
  CU_ASSERT(window[i]->packet->seqnum == i-2);
 }
 }


}
