/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/packet/query.c
*/

#include "question.h"

struct question **isr_deserialize_questions(unsigned char *req, struct header *header) {
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
		q->qtype = ntohs(*(uint16_t *)(body + cursor));
		cursor += 2;
		q->qclass = ntohs(*(uint16_t *)(body + cursor));
		cursor += 2;

		rst[i] = q;
	}

	return rst;
}
