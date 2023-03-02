/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/engine.c
*/

#include "engine.h"

unsigned char *isr_from_jerry_typedarray(jerry_value_t jerry_typedarray, uint16_t *length) {
	unsigned char *ret;

	jerry_length_t buff_len, buff_offset;
	jerry_value_t buff = jerry_typedarray_buffer(jerry_typedarray, &buff_offset, &buff_len);
	if (jerry_value_is_exception(buff)) { ret = NULL; goto free_buff; }

	ret = malloc(buff_len * sizeof(unsigned char));
	jerry_length_t readr = jerry_arraybuffer_read(buff, buff_offset, ret, buff_len);
	*length = readr;

free_buff:
	jerry_value_free(buff);

	return ret;
}

jerry_value_t isr_script_evaluate(const jerry_char_t *script, size_t script_size) {
	jerry_value_t ret;

	jerry_parse_options_t opts;
	opts.options = JERRY_PARSE_MODULE;

	ret = jerry_parse(script, script_size, &opts);
	if (jerry_value_is_exception(ret)) return ret;

	jerry_value_t linkr = jerry_module_link(ret, &isr_module_resolve_callback, NULL);
	if (jerry_value_is_exception(linkr)) return linkr;

	jerry_value_t evaluater = jerry_module_evaluate(ret);
	if (jerry_value_is_exception(evaluater)) { ret = evaluater; goto free_pre_evaluater; }

	jerry_value_free(evaluater);
free_pre_evaluater:
	jerry_value_free(linkr);

	return ret;
}

jerry_value_t isr_script_object_question(struct question *question) {
	jerry_value_t ret = jerry_object();

	jerry_value_t namev = jerry_string_sz(question->qname);
	jerry_value_t namer = jerry_object_set_sz(ret, "name", namev);
	jerry_value_free(namev);
	if (jerry_value_is_exception(namer)) return namer;

	jerry_value_t typev = jerry_number(question->qtype);
	jerry_value_t typer = jerry_object_set_sz(ret, "type", typev);
	jerry_value_free(typev);
	if (jerry_value_is_exception(typer)) { ret = typer; goto free_pre_typer; }

	jerry_value_t classv = jerry_number(question->qclass);
	jerry_value_t classr = jerry_object_set_sz(ret, "class", classv);
	jerry_value_free(classv);
	if (jerry_value_is_exception(classr)) { ret = classr; goto free_pre_classr; }

	jerry_value_free(classr);
free_pre_classr:
	jerry_value_free(typer);
free_pre_typer:
	jerry_value_free(namer);

	return ret;
}

jerry_value_t isr_script_call(jerry_value_t module, struct question *question, struct state_provider **providers, size_t providers_size) {
	jerry_value_t ret;

	jerry_value_t namespace = jerry_module_namespace(module);
	if (jerry_value_is_exception(namespace)) return namespace;
	
	jerry_value_t resolve = jerry_object_get_sz(namespace, "resolve");
	jerry_value_free(namespace);
	if (jerry_value_is_exception(resolve)) return resolve;
	
	jerry_value_t questiono = isr_script_object_question(question);
	if (jerry_value_is_exception(questiono)) { ret = questiono; goto free_pre_questiono; }

	jerry_value_t stateo = isr_script_object_state(providers, providers_size);
	if (jerry_value_is_exception(stateo)) { ret = stateo; goto free_pre_stateo; }

	jerry_value_t args[] = { questiono, stateo };
	jerry_size_t argscnt = 2;

	ret = jerry_call(resolve, jerry_undefined(), args, argscnt);

	jerry_value_free(stateo);
free_pre_stateo:
	jerry_value_free(questiono);
free_pre_questiono:
	jerry_value_free(resolve);

	return ret;
}

jerry_value_t isr_script_result_constructors(jerry_value_t *answerc, jerry_value_t *forwardc) {
	jerry_value_t ret;

	jerry_value_t result_module = isr_module_result();
	if (jerry_value_is_exception(result_module)) return result_module;

	jerry_value_t namespace = jerry_module_namespace(result_module);
	jerry_value_free(result_module);
	
	jerry_value_t answer = jerry_object_get_sz(namespace, "Answer");
	if (jerry_value_is_exception(answer)) { ret = answer; goto free_namespace; }
	*answerc = answer;

	jerry_value_t forward = jerry_object_get_sz(namespace, "Forward");
	if (jerry_value_is_exception(forward)) { ret = forward; jerry_value_free(answer); goto free_namespace; }
	*forwardc = forward;

	ret = jerry_boolean(true);

free_namespace:
	jerry_value_free(namespace);

	return ret;
}

/*
 * This function will jerry_value_free the given exception.
 */
struct resolve_result *isr_result_fallback(jerry_value_t exception) {
	jerry_value_t exception_val = jerry_exception_value(exception, true); /* This will jerry_value_free exception instead */
	jerry_value_t str = jerry_value_to_string(exception_val);
	jerry_value_free(exception_val);

	jerry_size_t str_size = jerry_string_size(str, JERRY_ENCODING_UTF8);

	jerry_char_t *buff = malloc(str_size * sizeof(jerry_char_t));
	jerry_size_t buff_size = jerry_string_to_buffer(str, JERRY_ENCODING_UTF8, buff, jerry_string_size(str, JERRY_ENCODING_UTF8));
	buff[buff_size] = '\0';
	jerry_value_free(str);

	printf("isr: isr.js: %s\n", buff);
	free(buff);

	struct resolve_result *ret = malloc(sizeof(struct resolve_result));
	ret->type = FALLBACK;
	return ret;
}

struct resolve_result *isr_from_call_result(jerry_value_t call_result, jerry_value_t answerc, jerry_value_t forwardc) {
	if (jerry_value_is_exception(call_result)) return isr_result_fallback(jerry_undefined());

	jerry_value_t is_answer_jerry = jerry_binary_op(JERRY_BIN_OP_INSTANCEOF, call_result, answerc);
	if (jerry_value_is_exception(is_answer_jerry)) return isr_result_fallback(is_answer_jerry);
	bool is_answer = jerry_value_to_boolean(is_answer_jerry);
	jerry_value_free(is_answer_jerry);

	jerry_value_t is_forward_jerry = jerry_binary_op(JERRY_BIN_OP_INSTANCEOF, call_result, forwardc);
	if (jerry_value_is_exception(is_forward_jerry)) return isr_result_fallback(is_forward_jerry);
	bool is_forward = jerry_value_to_boolean(is_forward_jerry);
	jerry_value_free(is_forward_jerry);

	if (is_answer) {
		struct resolve_result *ret;

		jerry_value_t type = jerry_object_get_sz(call_result, "type");
		if (jerry_value_is_exception(type)) return isr_result_fallback(type);
		if (!jerry_value_is_number(type)) {
			jerry_value_t exception = jerry_throw_value(jerry_string_sz("type is not a number"), true);
			ret = isr_result_fallback(exception);
			goto free_type;
		}

		jerry_value_t rdata = jerry_object_get_sz(call_result, "rdata");
		if (jerry_value_is_exception(rdata)) { ret = isr_result_fallback(rdata); goto free_pre_rdata; }
		if (!jerry_value_is_object(rdata)) {
			jerry_value_t exception = jerry_throw_value(jerry_string_sz("rdata is not an object"), true);
			ret = isr_result_fallback(exception);
			goto free_rdata;
		}

		jerry_value_t touint8array = jerry_object_get_sz(rdata, "toUint8Array");
		if (jerry_value_is_exception(touint8array)) { ret = isr_result_fallback(touint8array); goto free_pre_touint8array; }
		if (!jerry_value_is_function(touint8array)) {
			jerry_value_t exception = jerry_throw_value(jerry_string_sz("toUint8Array is not a function"), true);
			ret = isr_result_fallback(exception);
			goto free_touint8array;
		}

		jerry_value_t typedarray = jerry_call(touint8array, rdata, NULL, 0);
		if (jerry_value_is_exception(typedarray)) { ret = isr_result_fallback(typedarray); goto free_pre_typedarray; }
		if (!jerry_value_is_typedarray(typedarray)) {
			jerry_value_t exception = jerry_throw_value(jerry_string_sz("toUint8Array didn't return TypedArray"), true);
			ret =  isr_result_fallback(exception);
			goto free_typedarray;
		}

		uint16_t rdlength;
		unsigned char* rdatav = isr_from_jerry_typedarray(typedarray, &rdlength);

		ret = malloc(sizeof(struct resolve_result));
		ret->type = ANSWER;

		struct resolve_result_answer *ans = malloc(sizeof(struct resolve_result_answer));
		ans->type = jerry_value_as_uint32(type);
		ans->rdlength = rdlength;
		ans->rdata = rdatav;

		ret->value.answer = ans;

free_typedarray:
		jerry_value_free(typedarray);
free_touint8array:
free_pre_typedarray:
		jerry_value_free(touint8array);
free_rdata:
free_pre_touint8array:
		jerry_value_free(rdata);
free_type:
free_pre_rdata:
		jerry_value_free(type);

		return ret;
	} else if (is_forward) {
		jerry_value_t ip = jerry_object_get_sz(call_result, "ip");
		if (jerry_value_is_exception(ip)) return isr_result_fallback(ip);

		char *buff = malloc(16 * sizeof(char));
		jerry_size_t length = jerry_string_to_buffer(jerry_value_to_string(ip), JERRY_ENCODING_UTF8, (jerry_char_t *) buff, 16);	
		buff[length] = '\0';
		jerry_value_free(ip);

		struct resolve_result *ret = malloc(sizeof(struct resolve_result));
		ret->type = FORWARD;

		struct resolve_result_forward *fwd = malloc(sizeof(struct resolve_result_forward));
 		fwd->ip = buff;

		ret->value.forward = fwd;


		return ret;
	} else {
		jerry_value_t exception = jerry_throw_value(jerry_string_sz("Expected resolve to return either Answer or Forward, but none of them was returned"), true);

		return isr_result_fallback(exception);
	}
}

struct resolve_result *isr_script_run(jerry_value_t module, struct question *question, struct state_provider **providers, size_t providers_size) {
	struct resolve_result *ret;

	jerry_value_t callr = isr_script_call(module, question, providers, providers_size);
	if (jerry_value_is_exception(callr)) return isr_result_fallback(callr);

	jerry_value_t answerc, forwardc;
	jerry_value_t constructorsr = isr_script_result_constructors(&answerc, &forwardc);
	if (jerry_value_is_exception(constructorsr)) { ret = isr_result_fallback(constructorsr); goto free_pre_constructorsr; }

	ret = isr_from_call_result(callr, answerc, forwardc);

	jerry_value_free(answerc);
	jerry_value_free(forwardc);
	jerry_value_free(constructorsr);
free_pre_constructorsr:
	jerry_value_free(callr);

	return ret;
}
