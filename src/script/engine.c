/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/engine.c
*/

#include "engine.h"

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

struct eval_result *isr_evaluate(duk_context *ctx, struct question *question) {
    int stacksize = 0;
    /* Go reverse-alphabetic order, so we could look up from idx -2 */
    duk_get_global_string(ctx, "Forward");
    duk_get_global_string(ctx, "Answer");
    stacksize += 2;

    duk_get_global_string(ctx, "resolve");
    isr_push_question(ctx, question);
    duk_int_t res = duk_pcall(ctx, 1);
    stacksize += 1;
    if (res != DUK_EXEC_SUCCESS) goto error;

    if (duk_instanceof(ctx, -1, -2)) {
        duk_get_prop_string(ctx, -1, "rdata"); /* answer.rdata */
        duk_get_prop_string(ctx, -2, "type"); /* answer.type */
        duk_push_string(ctx, "toUint8Array");
        duk_int_t res = duk_pcall_prop(ctx, -3, 0); /* answer.rdata.toUint8Array() */
        stacksize += 3;
        if (res != DUK_EXEC_SUCCESS) goto error;

        struct eval_result_answer *ans = malloc(sizeof(struct eval_result_answer));
        ans->type = duk_get_number(ctx, -2);
        ans->rdata = duk_get_buffer_data(ctx, -1, &ans->rdlength);

        struct eval_result *rst = malloc(sizeof(struct eval_result));
        rst->type = ANSWER;
        rst->value.answer = ans;
        
        duk_pop_n(ctx, stacksize);
        return rst;
    } else if (duk_instanceof(ctx, -1, -3)) {
        duk_get_prop_string(ctx, -1, "ip"); /* forward.ip */
        stacksize += 1;

        struct eval_result_forward *fwd = malloc(sizeof(struct eval_result_forward));
        fwd->ip = duk_get_string(ctx, -1);

        struct eval_result *rst = malloc(sizeof(struct eval_result));
        rst->type = FORWARD;
        rst->value.forward = fwd;

        duk_pop_n(ctx, stacksize);
        return rst;
    } else {
        printf("isr: evaluate: Expected value to be Answer or Forward, but value was \'%s\'\n", duk_safe_to_string(ctx, -1));

        struct eval_result *rst = malloc(sizeof(struct eval_result));
        rst->type = FALLBACK;

        duk_pop_n(ctx, stacksize);
        return rst;
    }

error:
    printf("isr: evaluate: %s\n", duk_safe_to_string(ctx, -1)); // TODO: think of a better error msg?

    struct eval_result *rst = malloc(sizeof(struct eval_result));
    rst->type = FALLBACK;

    duk_pop_n(ctx, stacksize);
    return rst;
}