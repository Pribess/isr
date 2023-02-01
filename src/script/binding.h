/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/binding.h
*/

#ifndef ISR_SCRIPT_BINDING
#define ISR_SCRIPT_BINDING

#include "../packet/question.h"
#include "../../deps/duktape/duktape.h"
#include "../../deps/duktape/duk_config.h"

duk_bool_t isr_push_question(duk_context *ctx, struct question *question);

#endif