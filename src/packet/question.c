/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/packet/question.c
*/

#include "question.h"

/*
	DNS specification seems to be originally intended to support multiple questions,
	but this is not supported in the real world - both cloudflare and google will refuse to
	respond multiple answers to multiple questions.
	Hence, we will only deserialize the first question and use it exclusively,
	and return an reply with qdcount=1.
	This results in that we won't need to understand compressed messages,
	since it won't be possible to compress anything while writing the first question.
*/
struct question *isr_deserialize_question(unsigned char *req, struct header *header) {
	struct question *rst;
	rst = malloc(sizeof(struct question));

	unsigned char *body = req + 12;
	uint16_t cursor = 0;
        
	char name[255];
	int namelen = 0;

	while (true) {
		int labellen = body[cursor++];
		if (labellen == 0) break;

		memcpy(name + namelen, body + cursor, labellen);
		namelen += labellen;
		cursor += labellen;
			
		name[namelen++] = '.';
	}
	name[namelen-1] = '\0';
		
	rst->qname = malloc(namelen * sizeof(char));
	strcpy(rst->qname, name);
	rst->qtype = ntohs(*(uint16_t *)(body + cursor));
	cursor += 2;
	rst->qclass = ntohs(*(uint16_t *)(body + cursor));
	cursor += 2;

	return rst;
}
