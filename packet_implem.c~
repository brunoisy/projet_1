#include "packet_interface.h"
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>

/* Extra #includes */
/* Your code will be inserted here */


struct __attribute__ ((__packed__)) pkt
{
  ptypes_t type;
  uint8_t window;
  uint8_t seqnum;
  char *payload;
  uint16_t length;
  uint32_t crc;
};


pkt_t *
pkt_new ()
{
  pkt_t *ret = (pkt_t *) malloc (sizeof (pkt_t));	// Taille d'un packet
  return ret;
}

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

  ptypes_t type = data[0] >> 5;	// a vérif

  pkt->type = type;

  pkt->window = (data[0]<< 3) >> 3;	// verif, comment avoir E_WINDOW?

  pkt->seqnum = data[1];

  pkt->length = data[2];
	pkt->length=pkt->length << 8;
	pkt->length=pkt->length | (uint16_t)data[3];



  //placé ici car "Unless the error is E_NOHEADER, the packet has at least the values of the header found in the data stream."
  if (type != PTYPE_DATA && type != PTYPE_ACK && type != PTYPE_NACK)
    {
      return E_TYPE;
    }

  if (pkt->length > 512 || pkt->length + 8 != (uint16_t)len)
    {				// "pkt->length + 8 != len" peut être pas nécessaire
      return E_LENGTH;
    }

  //pkt->payload = (data << 32) >> (len * 8 - 32);

	int i;
	  for (i = 0; i < pkt->length; i++)
	    {
	      (pkt->payload)[i]= data[4+i];
	    }


  pkt->crc = data[4+pkt->length];
  pkt->crc = pkt->crc << 8;
pkt->crc = pkt->crc | (int)data[5+pkt->length];
  pkt->crc = pkt->crc << 8;
pkt->crc = pkt->crc | (int)data[6+pkt->length];
  pkt->crc = pkt->crc << 8;
pkt->crc = pkt->length | (int)data[7+pkt->length];
  pkt->crc = pkt->crc << 8;




  if (pkt->crc != (uint32_t)crc32 (0, (const Bytef *) pkt->payload, len - 4))
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


  char first_byte = (char) (((pkt->type) << 5) | (pkt->window));
  char second_byte = (char) pkt->seqnum;
  buf[0] = first_byte;
  buf[1] = second_byte;
  buf[2] = (char) ((pkt->length) >> 8);
  buf[3] = (char) (pkt->length);
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

uint32_t crc =(const uint32_t)crc32 (0, (const Bytef *)buf, *len - 4);
buf[4 + (pkt->length) + (pkt->length)%4] = (char) (crc>> 24);
  buf[5 + (pkt->length)+(pkt->length)%4] = (char) (crc >> 16);
  buf[6 + (pkt->length)+(pkt->length)%4] = (char) (crc >> 8);
  buf[7 + (pkt->length)+(pkt->length)%4] = (char) crc;
  *len = 7 + (pkt->length);
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
