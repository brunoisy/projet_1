#include "packet_interface.h"

/* Extra #includes */
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>


void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    }
    puts("");
}


struct __attribute__ ((__packed__)) pkt
{
  ptypes_t type:3;
  uint8_t window:5;
  uint8_t seqnum;
  uint16_t length;
  char *payload;
  uint32_t crc;
};



pkt_t *
pkt_new ()
{
  pkt_t *ret = (pkt_t *) malloc (sizeof (pkt_t));	// Taille d'un packet
  return ret;
}

//Free payload ?
void
pkt_del (pkt_t * pkt)
{
  free (pkt);
  pkt = NULL;
}

pkt_status_code
pkt_decode (const char *data, const size_t len, pkt_t * pkt)
{	
  if (len < 8)
    {				// si la taille du packet et < à la taille fixe nécessaire (type + window + ...)
      return E_NOHEADER;
    }


  pkt->type = data[0]>>5;
  pkt->window = data[0];
  pkt->seqnum = data[1];
  pkt->length = data[2];
  pkt->length = pkt->length << 8;
  pkt->length = pkt->length | data[3];  
  pkt->length = ntohs(pkt->length); // endianness !!


  //placé ici car "Unless the error is E_NOHEADER, the packet has at least the values of the header found in the data stream."
  if (pkt->type != PTYPE_DATA && pkt->type != PTYPE_ACK && pkt->type != PTYPE_NACK)
    {printf("ERROR\n");
      return E_TYPE;
      printf("ERROR\n");
    }
printf("PKT LENGTH:%d\n", pkt->length);
printf("LEN:%d\n", (uint16_t)len);

int padding = 0;
if(pkt->length%4 != 0){
padding = 4 - pkt->length%4;
}
printf("PADDING:%d\n" , padding);
  if (pkt->length + 8 + padding != (uint16_t) len ){
    printf("ERROR LENGTH\n");
      return E_UNCONSISTENT;
printf("ERROR\n");
    }

  if (pkt->length > 512)
    {		printf("ERROR\n");		
      return E_LENGTH;

    }


  pkt->payload = (char *)malloc(pkt->length);

	int i;
  for (i = 0; i < pkt->length; i++)
    {
      (pkt->payload)[i] = data[4 + i];
    }

  int startcrcbyte = 4 + pkt->length;
  if (pkt->length % 4 != 0)
    {
      startcrcbyte = startcrcbyte + 4 - pkt->length%4;
    }
pkt->crc=0;
  for (i = 0; i < 4; i++)
    {
pkt->crc = pkt->crc <<8;
    pkt->crc = pkt->crc | data[startcrcbyte + i];

}

pkt->crc=ntohl(pkt->crc); // endianness!!



  if (pkt->crc != (uint32_t) crc32 (0, (const Bytef *) pkt->payload, len - 4))
    {				// passage barbare de int à uint
      return E_CRC;
    }

  return PKT_OK;
}



pkt_status_code
pkt_encode (const pkt_t * pkt, char *buf, size_t * len)
{

  if ((uint16_t)(*len) < 7 + (pkt->length))
    {
      return E_NOMEM;
    }


  int length_to_encode = htons(pkt->length);
  
 
  buf[0] = ((char)pkt->type)<<5 | (char) pkt->window;
  buf[1] = pkt->seqnum;
  buf[2] = (char) (length_to_encode>>8);
  buf[3] = (char) length_to_encode;
  
  

  /*char first_byte = (char) (((pkt->type) << 5) | (pkt->window));
  char second_byte = (char) pkt->seqnum;
  buf[0] = first_byte;
  buf[1] = second_byte;
  buf[2] = (char) ((pkt->length) >> 8);
  buf[3] = (char) (pkt->length); 
  int i; */
int i;
  for (i = 0; i < pkt->length; i++)
    {
      buf[4 + i] = (pkt->payload)[i];
    }

  for(i=0;i<(pkt->length)%4;i++){
     buf[4+pkt->length+i] = 0b00000000;
  }
  /*pkt_set_crc (pkt, (const uint32_t)crc32 (0, (const Bytef *)buf, *len - 4));
  buf[4 + (pkt->length)] = (char) ((pkt->crc) >> 24);
  buf[5 + (pkt->length)] = (char) ((pkt->crc) >> 16);
  buf[6 + (pkt->length)] = (char) ((pkt->crc) >> 8);
  buf[7 + (pkt->length)] = (char) (pkt->crc);
*/
int padding = 0;
if(pkt->length%4 != 0){
padding = 4 - pkt->length%4;
}

 uint32_t crc = htonl((const uint32_t)crc32 (0, (const Bytef *)buf, *len - 4));
  buf[4 + (pkt->length) + padding] = (char) (crc>> 24);
  buf[5 + (pkt->length)+ padding] = (char) (crc >> 16);
  buf[6 + (pkt->length)+ padding] = (char) (crc >> 8);
  buf[7 + (pkt->length)+ padding] = (char) crc;
  *len = 8 + (pkt->length);
  for(i=11;i>=0;i--){
 printBits(1,&buf[i]);
}
  return PKT_OK;
}

ptypes_t pkt_get_type (const pkt_t * pkt)
{
  return pkt->type;


}

uint8_t pkt_get_window (const pkt_t * pkt )
{
  return pkt->window;
}

uint8_t pkt_get_seqnum (const pkt_t * pkt)
{
  return pkt->seqnum;
}

uint16_t pkt_get_length (const pkt_t * pkt)
{
  return pkt->length;
}

uint32_t pkt_get_crc (const pkt_t * pkt)
{
  return pkt->crc;
}

const char * pkt_get_payload (const pkt_t * pkt)
{
  return pkt->payload;
}


pkt_status_code
pkt_set_type (pkt_t * pkt, const ptypes_t type)
{
  if ((type != PTYPE_DATA) && (type != PTYPE_ACK) && (type != PTYPE_NACK))
    {
      return E_TYPE;
    }

  pkt->type = type;
return PKT_OK;
}

pkt_status_code
pkt_set_window (pkt_t * pkt, const uint8_t window)
{
  if (window > 31)
    {
      return E_WINDOW;
    }

  pkt->window = window;
return PKT_OK;
}

pkt_status_code
pkt_set_seqnum (pkt_t * pkt, const uint8_t seqnum)
{
  pkt->seqnum = seqnum;
return PKT_OK;
}

pkt_status_code
pkt_set_length (pkt_t * pkt, const uint16_t length)
{
  if (length > 512)
    {
      return E_LENGTH;
    }

  pkt->length = length;
return PKT_OK;
}

pkt_status_code
pkt_set_crc (pkt_t * pkt, const uint32_t crc)
{
  pkt->crc = crc;
return PKT_OK;
}

pkt_status_code
pkt_set_payload (pkt_t * pkt, const char *data, const uint16_t length)
{

  if (length > 512)
    {				// ATTENTION length EST EGAL A PKT->length ?
      return E_LENGTH;
    }
  int i;
  for (i = 0; i < length; i++)
    {
      pkt->payload[i] = data[i];
    }
return PKT_OK;

}



int main(int argc, char *argv[]){
	pkt_t * pkt = malloc(sizeof(pkt_t));
        pkt_t * pkt2 = malloc(sizeof(pkt_t));
	pkt->type=PTYPE_DATA;// 1
	pkt->window=3;
	pkt->seqnum=1;
	pkt->length=5;
	char * payload = "abcd"; // avec \0, 7 charactères
	pkt->payload= payload;

	size_t buffersize = 8 + pkt->length; // taille fixe + taille payload
	if(pkt->length % 4 != 0){
		buffersize = buffersize + 4 - (pkt->length % 4); // + padding
	}
	char * buffer=(char *) malloc((size_t)buffersize);
        printBits(12, pkt);
        int padding = 0;
if(pkt->length%4 != 0){
padding = 4 - pkt->length%4;
}
	printf("inencode\n");
	pkt_encode(pkt, buffer, &buffersize);
	printf("outencode\n");
	printf("indecode\n");
        pkt->payload = "sdvdsqv";
	pkt_decode(buffer, buffersize + padding, pkt2);
	printf("outdecode\n");
        printBits(12, pkt);
	printf("type %d, window %d, seqnum %d, length %d, payload %s, crc %d\n", pkt2->type, pkt2->window, pkt2->seqnum, pkt2->length, pkt2->payload,pkt2->crc);
	
}
