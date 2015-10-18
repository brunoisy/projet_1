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
 struct pkt_timer * window[max_window_size];
 
 for(int i



















}
