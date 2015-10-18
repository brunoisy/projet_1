sender:  sender.o create_socket.o real_address.o sel_repeat_write.o packet_implem.o -lz

receiver: receiver.o create_socket.o real_address.o sel_repeat_read.o packet_implem.o wait_for_client.o -lz


#test_implem: test_implem.c packet_implem.c packet_interface.h
#	gcc test_implem.c packet_implem.c -lz -I$(HOME)/local/include -L$(HOME)/local/lib -lcunit -o test_implem

#test_write: test_write.c sel_repeat_write.h packet_interface.h packet_implem.c
#	gcc test_write.c sel_repeat_write.c packet_implem.c -lz -I$(HOME)/local/include -L$(HOME)/local/lib -lcunit -o test_write

#test_read: test_read.c sel_repeat_read.h packet_interface.h
#	gcc test_read.c sel_repeat_read.c packet_implem.c -lz -I$(HOME)/local/include -L$(HOME)/local/lib -lcunit -o test_read


#gcc receiver.c create_socket.c real_address.c sel_repeat_read.c packet_implem.c wait_for_client.c -lz -g

