/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/state.h
*/

#ifndef ISR_SCRIPT_STATE
#define ISR_SCRIPT_STATE

#include <jerryscript.h>
#include <stdlib.h>
#include <stdio.h>

#include "module.h"

struct state_provider {
	jerry_value_t callback;
	char **path;
	size_t path_length;
};

struct state_provider **isr_script_state_providers(size_t *size);

jerry_value_t isr_script_state(struct state_provider **providers, size_t size);

#endif
