/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/packet/header.h
*/

#ifndef ISR_PACKET_HEADER
#define ISR_PACKET_HEADER

#include <arpa/inet.h>
#include <stdlib.h>
#include <stdint.h>

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

struct header *isr_deserialize_header(unsigned char *req);

#endif
