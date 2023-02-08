/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/engine.h
*/

#ifndef ISR_SCRIPT_ENGINE
#define ISR_SCRIPT_ENGINE

#include "../packet/question.h"
#include "../../deps/duktape/duktape.h"
#include "../../deps/duktape/duk_config.h"

duk_bool_t isr_push_question(duk_context *ctx, struct question *question);

struct eval_result_answer {
	uint16_t type;
	uint16_t rdlength;
	unsigned char *rdata;
};

struct eval_result_forward {
	char *ip;	
};

struct eval_result {
	enum { ANSWER, FORWARD, FALLBACK } type;
	union {
		struct eval_result_answer *answer;
		struct eval_result_forward *forward;
	} value;
};

struct eval_result *isr_evaluate(duk_context *ctx, struct question *question);

#endif