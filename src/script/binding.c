/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/binding.c
*/

#include "binding.h"

duk_bool_t isr_push_question(duk_context *ctx, struct question *question) {
    duk_idx_t idx = duk_push_object(ctx);

    duk_push_string(ctx, "name");
    duk_push_string(ctx, question->qname);
    duk_def_prop(ctx, idx, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_CLEAR_WEC);

    duk_push_string(ctx, "type");
    duk_push_uint(ctx, question->qtype);
    duk_def_prop(ctx, idx, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_CLEAR_WEC);

    duk_push_string(ctx, "class");
    duk_push_uint(ctx, question->qclass);
    duk_def_prop(ctx, idx, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_CLEAR_WEC);
}