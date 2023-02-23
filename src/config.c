/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/config.c
*/

#include "config.h"

struct config isr_config;

// Temporary
void isr_load_config() {
	char dir[] = "/home/jhyub/isrtest/isr.d";
	isr_config.getter_script_dir = malloc(sizeof(dir));
	strcpy(isr_config.getter_script_dir, dir);
}

