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
struct question *isr_deserialize_question(unsigned char *body, uint16_t qdcount) {
	if (qdcount != 1) {
		return NULL;
	}

	struct question *rst;
	rst = malloc(sizeof(struct question));

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

unsigned char *isr_serialize_question(size_t *len, struct question *question) {
	/*
		question->qname is a plain domain string such as "example.com",
		while what we want is this: 7 e x a m p l e 3 c o m 0

		this results in: actual qname field length = qname string length + 2 (first length ocetet, last null octet)
	*/
	*len = strlen(question->qname) + 2 + 4;

	unsigned char *rst;
	rst = malloc(*len * sizeof(char));

	int cursor = 0;
	int labelstart = 0;
	for(int i=0; true; i++) {
		if(question->qname[i] == '.' || question->qname[i] == '\0') {
			rst[cursor++] = i - labelstart;
			memcpy(rst + cursor, question->qname + labelstart, i - labelstart);		
			cursor += (i - labelstart);
			labelstart = i + 1;
			if(question->qname[i] == '\0') {
				rst[cursor++] = 0;
				break;
			}
		}
	}

	*(uint16_t *)(rst + cursor) = htons(question->qtype);
	*(uint16_t *)(rst + cursor + 2) = htons(question->qclass);

	return rst;
}