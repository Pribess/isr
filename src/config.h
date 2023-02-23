/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/config.h
*/

#ifndef ISR_CONFIG
#define ISR_CONFIG

#include <stdlib.h>
#include <string.h>

struct config {
	char *getter_script_dir;
};

void isr_load_config();

#endif
