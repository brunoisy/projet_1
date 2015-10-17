test: test_implem.c packet_implem.c packet_interface.h
	gcc test_implem.c packet_implem.c -lz -I$(HOME)/local/include -L$(HOME)/local/lib -lcunit -o test

receiver: receiver.o create_socket.o real_address.o sel_repeat_read.o packet_implem.o -lz

