/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/engine.h
*/

#ifndef ISR_SCRIPT_ENGINE
#define ISR_SCRIPT_ENGINE

#include <jerryscript.h>
#include <stdio.h>
#include <stdlib.h>

#include "module.h"
#include "../packet/question.h"

unsigned char *isr_from_jerry_typedarray(jerry_value_t jerry_typedarray, uint16_t *length);

struct resolve_result_answer {
	uint16_t type;
	uint16_t rdlength;
	unsigned char *rdata;
};

struct resolve_result_forward {
	char *ip;	
};

struct resolve_result {
	enum { ANSWER, FORWARD, FALLBACK } type;
	union {
		struct resolve_result_answer *answer;
		struct resolve_result_forward *forward;
	} value;
};

jerry_value_t isr_script_evaluate(const jerry_char_t *script, size_t script_size);

struct resolve_result *isr_script_run(jerry_value_t module, struct question *question);

#endif
