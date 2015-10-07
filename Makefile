test: test_implem.c packet_implem.c packet_interface.h
	gcc test_implem.c packet_implem.c -lz -I$(HOME)/local/include -L$(HOME)/local/lib -lcunit -o test



