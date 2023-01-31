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
