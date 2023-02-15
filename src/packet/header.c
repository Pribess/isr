/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/packet/header.c
*/

#include "header.h"

struct header *isr_deserialize_header(unsigned char *req) {
	struct header *rst;

	rst = malloc(sizeof(struct header));

	rst->id = ntohs(*(uint16_t *)(req + 0));

	rst->qr = (req[2] & 0x80) >> 7;
	rst->opcode = (req[2] & 0x78) >> 3;
	rst->aa = (req[2] & 0x04) >> 2;
	rst->tc = (req[2] & 0x02) >> 1;
	rst->rd = (req[2] & 0x01) >> 0;
    
	rst->ra = (req[3] & 0x80) >> 7;
	rst->z = (req[3] & 0x70) >> 4;
	rst->rcode = (req[3] & 0x0F) >> 0;

	rst->qdcount = ntohs(*(uint16_t *)(req + 4));
	rst->ancount = ntohs(*(uint16_t *)(req + 6));
	rst->nscount = ntohs(*(uint16_t *)(req + 8));
	rst->arcount = ntohs(*(uint16_t *)(req + 10));

	return rst;
}

unsigned char *isr_serialize_header(size_t *len, struct header *header) {
	unsigned char *rst;
	rst = malloc(12 * sizeof(char));
	*len = 12;

	*(uint16_t *)(rst + 0) = htons(header->id);

	rst[2] = (header->qr << 7) 
		+ (header->opcode << 3)
		+ (header->aa << 2)
		+ (header->tc << 1)
		+ (header->rd << 0);

	rst[3] = (header->ra << 7)
	+ (header->z << 4)
	+ (header->rcode << 0);

	*(uint16_t *)(rst + 4) = htons(header->qdcount);
	*(uint16_t *)(rst + 6) = htons(header->ancount);
	*(uint16_t *)(rst + 8) = htons(header->nscount);
	*(uint16_t *)(rst + 10) = htons(header->arcount);

	return rst;
}
