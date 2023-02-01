/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/packet/answer.c
*/

#include "answer.h"

unsigned char *isr_serialize_record(uint32_t *len, struct record *record) {
	/*
		Since we will only reply to one question(please refer to question.c),
		we could safely assume that our name will exist at index 12,
		so we could directly write 0xC0 0x0C for the name section.
		This will enable us to calculate the length of the whole message right away.
	*/

	*len = (uint32_t)(record->rdlength) + 12;

	unsigned char *rst;
	rst = malloc(*len * sizeof(char));

	rst[0] = 0xC0; 
	rst[1] = 0x0C;

	*(uint16_t *)(rst + 2) = htons(record->type);
	*(uint16_t *)(rst + 4) = htons(record->class);
	*(uint32_t *)(rst + 6) = htonl(record->ttl);
	*(uint16_t *)(rst + 10) = htons(record->rdlength);

	memcpy(rst + 12, record->rdata, record->rdlength);

	return rst;
}

