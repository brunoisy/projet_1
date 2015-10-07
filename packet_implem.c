#include "packet_interface.h"

/* Extra #includes */
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

void printBits(size_t const size, void const *const ptr);

struct __attribute__ ((__packed__)) pkt {
	ptypes_t type:3;
	uint8_t window:5;
	uint8_t seqnum;
	uint16_t length;
	char *payload;
	uint32_t crc;
};

pkt_t *pkt_new()
{
	pkt_t *ret = (pkt_t *) malloc(sizeof(pkt_t));	// Taille d'un packet
	return ret;
}

//Free payload ?
void pkt_del(pkt_t * pkt)
{
	free(pkt->payload);
	pkt->payload = NULL;
	free(pkt);
	pkt = NULL;
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t * pkt)
{
	if(len < 4){
           return E_NOHEADER;
       }

        if (len < 8) {		// si la taille du packet et < à la taille fixe nécessaire (type + window + ...)
		return E_UNCONSISTENT;
	}
        if(len%4 != 0){
          return E_UNCONSISTENT;
        }

        if((pkt->type == PTYPE_ACK || pkt->type == PTYPE_NACK) && len > 0){
         return E_UNCONSISTENT;
        }
       

	int err = pkt_set_type(pkt,data[0] >> 5);
	int err2 =pkt_set_window(pkt, (data[0]<<3)>>3);
	int err3 = pkt_set_seqnum(pkt,data[1]);
	pkt->length = data[2];
	pkt->length = pkt->length << 8;
	pkt->length = pkt->length | data[3];
	int err4 = pkt_set_length(pkt,ntohs(pkt->length));	// endianness !!

      /*  if(err!=PKT_OK){return err;}
//if(err2!=PKT_OK){return err2;}
if(err3!=PKT_OK){return err3;}
if(err4!=PKT_OK){return err4;}
*/
	//placé ici car "Unless the error is E_NOHEADER, the packet has at least the values of the header found in the data stream."
	if (pkt->type != PTYPE_DATA && pkt->type != PTYPE_ACK
	    && pkt->type != PTYPE_NACK) {
		printf("ERROR\n");
		return E_TYPE;
		printf("ERROR\n");
	}
	printf("PKT LENGTH:%d\n", pkt->length);
	printf("LEN:%d\n", (uint16_t) len);

	int padding = 0;
	if (pkt->length % 4 != 0) {
		padding = 4 - pkt->length % 4;
	}
   
	printf("PADDING:%d\n", padding);

	if (pkt->length + 8 + padding > (uint16_t) len) {	// <= ou != ?
		printf("ERROR LENGTH\n");
		return E_UNCONSISTENT;
		printf("ERROR\n");
	}

	if (pkt->length > 512) {
		printf("ERROR\n");
		return E_LENGTH;

	}

	int i;

	char * payload = (char *)malloc(pkt->length * sizeof(char));


	for (i = 0; i < pkt->length; i++) {
		payload[i] = data[4 + i];
	}

        int err5 = pkt_set_payload(pkt,payload,pkt->length);
        free(payload);
	int startcrcbyte = 4 + pkt->length + padding;

	pkt->crc = 0;
	uint32_t ithByte;	// le ième byte de crc
	uint32_t inter;
	for (i = 0; i < 4; i++) {
		pkt->crc = pkt->crc << 8;
		ithByte = (uint32_t) data[startcrcbyte + i];
		inter = 0b00000000000000000000000011111111 & ithByte;
		pkt->crc = pkt->crc | inter;
	}

	int err6 = pkt_set_crc(pkt,ntohl(pkt->crc));	
        if(err5!=PKT_OK){return err5;}
        if(err6!=PKT_OK){return err6;}
// endianness!!

	printf("decode_crc from buffer : %u\n", pkt->crc);

	uint32_t thiscrc = (uint32_t) crc32(0, (const Bytef *)data, pkt->length + 4 + padding);	//crc calculé sans le padding

	printf("decode_crc calculated = %u\n", thiscrc);

	if (pkt->crc != thiscrc) {	// passage barbare de int à uint
		return E_CRC;
	} 

	printf("data dans decode\n");
	for (i = 1; i <= pkt->length + padding + 8; i++) {
		printBits(1, &data[i - 1]);
		if (i % 4 == 0 && i != 0) {
			printf("\n");
		}
	}

	return PKT_OK;
}

pkt_status_code pkt_encode(const pkt_t * pkt, char *buf, size_t * len)
{

	buf[0] = ((char)pkt->type) << 5 | (char)pkt->window;
	buf[1] = pkt->seqnum;
	uint16_t length = pkt_get_length(pkt);
	uint16_t ordering = htons(length);
	buf[2] = ordering & 0xFF;
	buf[3] = ordering>>8;

	int i;
	for (i = 0; i < pkt->length; i++) {
		buf[4 + i] = (pkt->payload)[i];
	}

	int padding = 0;
	if (pkt->length % 4 != 0) {
		padding = 4 - pkt->length % 4;
	}

	for (i = 0; i < padding; i++) {
		buf[4 + pkt->length + i] = 0b00000000;
	}

	uint32_t crc = htonl((const uint32_t)crc32(0, (const Bytef *)buf, pkt->length + 4 + padding));	//on calcule le crc sur le header et le payload
	

	//printf("encode_crc :%u\n",
	       //(const uint32_t)crc32(0, (const Bytef *)buf, pkt->length + 4));

	buf[7 + (pkt->length) + padding] = (char)(crc >> 24);
	buf[6 + (pkt->length) + padding] = (char)(crc >> 16);
	buf[5 + (pkt->length) + padding] = (char)(crc >> 8);
	buf[4 + (pkt->length) + padding] = (char)crc; 
	

	printf("buf dans encode\n");
	for (i = 1; i <= pkt->length + padding + 8; i++) {
		printBits(1, &buf[i - 1]);
		if (i % 4 == 0 && i != 0) {
			printf("\n");
		}
	}


        if ((uint16_t) (*len) < 8 + (pkt->length)+padding) {
		return E_NOMEM;
	}
        *len = 8 + (pkt->length) + padding;

	return PKT_OK;
}

ptypes_t pkt_get_type(const pkt_t * pkt)
{
	return pkt->type;

}

uint8_t pkt_get_window(const pkt_t * pkt)
{
	return pkt->window;
}

uint8_t pkt_get_seqnum(const pkt_t * pkt)
{
	return pkt->seqnum;
}

uint16_t pkt_get_length(const pkt_t * pkt)
{
	return pkt->length;
}

uint32_t pkt_get_crc(const pkt_t * pkt)
{
	return pkt->crc;
}

const char *pkt_get_payload(const pkt_t * pkt)
{
	return pkt->payload;
}

pkt_status_code pkt_set_type(pkt_t * pkt, const ptypes_t type)
{
	if ((type != PTYPE_DATA) && (type != PTYPE_ACK) && (type != PTYPE_NACK)) {
		return E_TYPE;
	}

	pkt->type = type;
	return PKT_OK;
}

pkt_status_code pkt_set_window(pkt_t * pkt, const uint8_t window)
{
	if (window > 31) {
		return E_WINDOW;
	}

	pkt->window = window;
	return PKT_OK;
}

pkt_status_code pkt_set_seqnum(pkt_t * pkt, const uint8_t seqnum)
{
	pkt->seqnum = seqnum;
	return PKT_OK;
}

pkt_status_code pkt_set_length(pkt_t * pkt, const uint16_t length)
{
	if (length > 512) {
		return E_LENGTH;
	}

	pkt->length = length;
	return PKT_OK;
}

pkt_status_code pkt_set_crc(pkt_t * pkt, const uint32_t crc)
{
	pkt->crc = crc;
	return PKT_OK;
}

pkt_status_code
pkt_set_payload(pkt_t * pkt, const char *data, const uint16_t length)
{

	if (length > 512) {	// ATTENTION length EST EGAL A PKT->length ?
		return E_LENGTH;
	}
        int padding = 0;
	if (length % 4 != 0) {
		padding = 4 - length % 4;
	}
        pkt->payload = (char *)malloc(length+padding);
	int i;
	for (i = 0; i < length; i++) {
		pkt->payload[i] = data[i];
	}
        for (i = 0; i < padding; i++) {
		pkt->payload[length + i] = 0b00000000;
	}
        pkt->length = length;
	return PKT_OK;

}


/*
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
*/


void printBits(size_t const size, void const *const ptr)
{
	unsigned char *b = (unsigned char *)ptr;
	unsigned char byte;
	int i, j;

	for (i = size - 1; i >= 0; i--) {
		for (j = 7; j >= 0; j--) {
			byte = b[i] & (1 << j);
			byte >>= j;
			printf("%u", byte);
		}
	}
	printf(" ");
}
