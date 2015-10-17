#include "packet_interface.h"
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>


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
	pkt_t *ret = (pkt_t *) malloc(sizeof(pkt_t));
	return ret;
}

void pkt_del(pkt_t * pkt)
{
	free(pkt->payload);
	pkt->payload = NULL;
	free(pkt);
	pkt = NULL;
}

pkt_status_code pkt_decode(const char *data, const size_t len, pkt_t * pkt)
{

	if (len < 4) {
		return E_NOHEADER;
	}


	//initialisation du header
	pkt_status_code err = pkt_set_type(pkt, data[0] >> 5);
	pkt_status_code err2 =
	    pkt_set_window(pkt, (const uint8_t)(data[0] << 3) >> 3);
	pkt_status_code err3 = pkt_set_seqnum(pkt, data[1]);

	uint16_t length = data[3];
	length = length << 8;
	length = length | data[2];
	pkt_status_code err4 = pkt_set_length(pkt, ntohs(length));	// endianness !!


	if (len < 8) {		// si la taille du packet et < à la taille fixe nécessaire (type + window + ...)
		return E_UNCONSISTENT;
	}

	if (len % 4 != 0) {	// si la taille du packet ne respecte pas l'allignement sur 4 bytes
		return E_UNCONSISTENT;
	}

	//vérification erreurs du header
	if (err != PKT_OK) {
		return err;
	}
	if (err2 != PKT_OK) {	// potentiellement problématique (inginious)
		return err2;
	}
	if (err3 != PKT_OK) {
		return err3;
	}
	if (err4 != PKT_OK) {
		return err4;
	}
	//placé ici car "Unless the error is E_NOHEADER, the packet has at least the values of the header found in the data stream."
	if (pkt->type != PTYPE_DATA && pkt->type != PTYPE_ACK
	    && pkt->type != PTYPE_NACK) {
		return E_TYPE;
	}
 
	if (pkt_get_type(pkt) != PTYPE_DATA && pkt_get_length(pkt) > 0 ) { //si le type n'est pas data, la longueur du payload est nulle
		return E_UNCONSISTENT;
	}



	int padding = 0;
	if (pkt_get_length(pkt) % 4 != 0) {
		padding = 4 - pkt_get_length(pkt) % 4;
	}

	if (pkt_get_length(pkt) + 8 + padding != (uint16_t) len) {// si la longeur du package != len    
		return E_UNCONSISTENT;
	}

	char *payload = (char *)malloc(pkt_get_length(pkt) * sizeof(char));
	int i;
	for (i = 0; i < pkt_get_length(pkt); i++) {// initialisation du payload
		payload[i] = data[4 + i];
	}

	pkt_status_code err5 =
	    pkt_set_payload(pkt, payload, pkt_get_length(pkt));
	free(payload);

	int startcrcbyte = 4 + pkt_get_length(pkt) + padding;

	uint32_t crc = 0;
	uint32_t ithByte;// le ième byte de crc
	uint32_t inter;
	for (i = 3; i >= 0; i--) {
		crc = crc << 8;
		ithByte = (uint32_t) data[startcrcbyte + i];
		inter = 0b00000000000000000000000011111111 & ithByte;
		crc = crc | inter;
	}

	pkt_status_code err6 = pkt_set_crc(pkt, ntohl(crc));	// endianness!!

	uint32_t thiscrc = (uint32_t) crc32(0, (const Bytef *)data, pkt->length + 4 + padding);	//crc calculé sans le padding


	if (pkt_get_crc(pkt) != thiscrc) {
		return E_CRC;
	}
	if (err5 != PKT_OK) {
		return err5;
	}
	if (err6 != PKT_OK) {
		return err6;
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
	buf[3] = ordering >> 8;

	int i;
	for (i = 0; i < pkt_get_length(pkt); i++) {
		buf[4 + i] = (pkt->payload)[i];
	}

	int padding = 0;
	if (pkt_get_length(pkt) % 4 != 0) {
		padding = 4 - pkt_get_length(pkt) % 4;
	}

	for (i = 0; i < padding; i++) {
		buf[4 + pkt_get_length(pkt) + i] = 0b00000000;
	}

	
	//uint32_t crc = htonl((const uint32_t)crc32(0, (const Bytef *)buf, pkt->length + 4 + padding));	//on calcule le crc sur le header et le payload
        uint32_t crc = 0;

	buf[4 + pkt_get_length(pkt) + padding] = (char)crc;
	buf[5 + pkt_get_length(pkt) + padding] = (char)(crc >> 8);
	buf[6 + pkt_get_length(pkt) + padding] = (char)(crc >> 16);
	buf[7 + pkt_get_length(pkt) + padding] = (char)(crc >> 24);

	if ((uint16_t) (*len) < 8 + (pkt->length) + padding) {
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
	if (window > MAX_WINDOW_SIZE) {
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
	if (length > MAX_PAYLOAD_SIZE) {
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

	if (length > MAX_PAYLOAD_SIZE) {
		return E_LENGTH;
	}
	int padding = 0;
	if (length % 4 != 0) {
		padding = 4 - length % 4;
	}
	pkt->payload = (char *)malloc(length + padding);
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
