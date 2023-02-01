/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/packet/answer.h
*/

#ifndef ISR_PACKET_ANSWER
#define ISR_PACKET_ANSWER

#include <arpa/inet.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct record {
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    unsigned char *rdata;
};

unsigned char *isr_serialize_record(uint32_t *len, struct record *record);

#endif
