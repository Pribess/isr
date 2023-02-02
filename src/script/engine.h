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

#endif