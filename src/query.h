/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/query.h
*/

#ifndef ISR_QUERY
#define ISR_QUERY

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct header {
	uint16_t id;
	unsigned char qr: 1;
	unsigned char opcode: 4;
	unsigned char aa: 1;
	unsigned char tc: 1;
	unsigned char rd: 1;
	unsigned char ra: 1;
	unsigned char z : 3;
	unsigned char rcode: 4;
	uint16_t qdcount;
	uint16_t ancount;
	uint16_t nscount;
	uint16_t arcount;
};

struct header *isr_parse_header(unsigned char *req);

struct question {
	char *qname;
	uint16_t qtype;
	uint16_t qclass;
};

struct question **isr_parse_questions(unsigned char *req, struct header *header);

#endif
