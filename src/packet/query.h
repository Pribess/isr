/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/packet/query.h
*/

#ifndef ISR_PACKET_QUERY
#define ISR_PACKET_QUERY

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct question {
	char *qname;
	uint16_t qtype;
	uint16_t qclass;
};

struct question **isr_parse_questions(unsigned char *req, struct header *header);

#endif
