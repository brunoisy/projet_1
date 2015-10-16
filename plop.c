int main(int argc, char *argv[])
{

        pkt_t * pkt = pkt_new();
	pkt_t * pkt2 = pkt_new();
	
	pkt_set_type(pkt, PTYPE_ACK);
	pkt_set_window(pkt, 3);
	pkt_set_seqnum(pkt, 4);
        char * data = (char *)malloc(2);
        data[0] = 'a';
        data[1] = 'r';

        if(pkt_set_payload(pkt,data,2)!=PKT_OK){
         printf("ERROR SET PAYLOAD\n");
        } 
	

	size_t buffersize = 8 + pkt->length;	// taille fixe + taille payload
	if (pkt->length % 4 != 0) {
		buffersize = buffersize + 4 - (pkt->length % 4);	// + padding
	}


	char *buffer = (char *)malloc((size_t) buffersize);
	int padding = 0;
	if (pkt->length % 4 != 0) {
		padding = 4 - pkt->length % 4;
	}


	printf("inencode\n");
	pkt_encode(pkt, buffer, &buffersize);
	printf("outencode\n");

	printf("indecode\n");
	pkt_decode(buffer, buffersize + padding, pkt2);
	printf("outdecode\n");

	printf("type %d, window %d, seqnum %d, length %d, crc %u\n",
	       pkt_get_type(pkt2), pkt_get_window(pkt2), pkt_get_seqnum(pkt2), pkt_get_length(pkt2),
	        pkt_get_crc(pkt2));

        free(buffer);
	pkt_del(pkt);
	pkt_del(pkt2);

}
