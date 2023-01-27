/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/query.c
*/

#include "query.h"

struct header *isr_parse_header(unsigned char *req) {
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

struct question **isr_parse_questions(unsigned char *req, struct header *header) {
	struct question **rst;
	rst = malloc(header->qdcount * sizeof(struct question *));

    unsigned char *body = req + 12;
	uint16_t cursor = 0;
	for (int i = 0 ; i < header->qdcount ; i++) {
		struct question *q;

		q = malloc(sizeof(struct question));
        
		int namelen = 0;

		char name[255];

		while (true) {
			int labellen = body[cursor++];
			if (labellen == 0) break;

			memcpy(name + namelen, body + cursor, labellen);
			namelen += labellen;
			cursor += labellen;
			
			name[namelen++] = '.';
		}
		name[namelen-1] = '\0';
		cursor++;
		
		q->qname = malloc(namelen * sizeof(char));
		strcpy(q->qname, name);
		q->qtype = ((uint16_t *)body)[cursor];
		cursor += 2;
		q->qclass = ((uint16_t *)body)[cursor];
		cursor += 2;

		rst[i] = q;
	}

	return rst;
}