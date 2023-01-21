/*
		Copyright (C) 2022
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/isr.c
*/

#define ISR_VERSION "0.01"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "");

	argc--;
	argv++;

	if (argc < 1) {
		printf("usage: isr [-v] file\n");
		return 1;
	}

	if (argc > 0 && (*argv)[0] == '-' && (*argv)[1] == 'v') {
		printf("isr %s (%s, %s)\n", ISR_VERSION, __DATE__, __TIME__);
		return 0;
	}

	FILE *fp;
	if ((fp = fopen(*argv, "r")) == NULL) {
		printf("isr: %s: %s\n", *argv, strerror(errno));
		return 1;
	}

	return 0;
}
