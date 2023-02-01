/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/packet/question.h
*/

#ifndef ISR_PACKET_QUESTION
#define ISR_PACKET_QUESTION

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"

struct question {
	char *qname;
	uint16_t qtype;
	uint16_t qclass;
};

struct question *isr_deserialize_question(unsigned char *req, struct header *header);

#endif
