/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/isr.c
*/

#define ISR_VERSION "0.01"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <locale.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#include "query.h"

void udp_loop();

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "");

	argc--;
	argv++;

	if (argc < 1) {
		printf("usage: isr [-v] file dns\n");
		return 1;
	}

	if (argc > 0 && (*argv)[0] == '-' && (*argv)[1] == 'v') {
		printf("isr %s (%s, %s)\n", ISR_VERSION, __DATE__, __TIME__);
		return 0;
	}

	udp_loop();

	return 0;
}

void udp_loop() {
	printf("UDP server initializing...\n");

	int sockfd;

	struct sockaddr_in addr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(53);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("isr: ");
		exit(-1);
	}

	if (bind(sockfd, (struct sockaddr *)&addr, addrlen) < 0) {
		perror("isr: ");
		exit(-1);
	}

	printf("UDP server successfully initialized!\n");

	unsigned char *buf[512];

	while (true) {
		int cnt = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&clientaddr, &addrlen);
		
		// handling request with packet in buf and packet length in cnt
	}
}