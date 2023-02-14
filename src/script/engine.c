/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/engine.c
*/

#include "engine.h"

jerry_value_t isr_script_evaluate(const jerry_char_t *script, size_t script_size) {
	jerry_parse_options_t opts;
	opts.options = JERRY_PARSE_MODULE;

	jerry_value_t ret = jerry_parse(script, script_size, &opts);
	if (jerry_value_is_exception(ret)) return ret;

	jerry_value_t linked = jerry_module_link(ret, &isr_module_resolve_callback, NULL);
	if (jerry_value_is_exception(linked)) return linked;
	jerry_value_free(linked);

	jerry_value_t evaluated = jerry_module_evaluate(ret);
	if (jerry_value_is_exception(evaluated)) return evaluated;
	jerry_value_free(evaluated);

	return ret;
}

jerry_value_t isr_object_question(struct question *question) {
	jerry_value_t ret = jerry_object();

	jerry_value_t namek = jerry_string_sz("name");
	jerry_value_t name = jerry_object_set(ret, namek, jerry_string_sz(question->qname));
	jerry_value_free(namek);
	if (jerry_value_is_exception(name)) return name;
	jerry_value_free(name);

	jerry_value_t typek = jerry_string_sz("type");
	jerry_value_t type = jerry_object_set(ret, typek, jerry_number(question->qtype));
	jerry_value_free(typek);
	if (jerry_value_is_exception(type)) return type;
	jerry_value_free(type);

	jerry_value_t classk = jerry_string_sz("class");
	jerry_value_t class = jerry_object_set(ret, classk, jerry_number(question->qclass));
	jerry_value_free(classk);
	if (jerry_value_is_exception(class)) return class;
	jerry_value_free(class);

	return ret;
}

jerry_value_t isr_script_call_resolve(jerry_value_t module, struct question *question) {
	jerry_value_t namespace = jerry_module_namespace(module);
	if (jerry_value_is_exception(namespace)) return namespace;
	
	jerry_value_t resolvek = jerry_string_sz("resolve");
	jerry_value_t resolve = jerry_object_get(namespace, resolvek);
	jerry_value_free(namespace);
	jerry_value_free(resolvek);
	if (jerry_value_is_exception(resolve)) return resolve;
	
	jerry_value_t questiono = isr_object_question(question);
	if (jerry_value_is_exception(questiono)) return questiono;

	jerry_value_t args[] = { questiono };
	jerry_size_t argscnt = 1;

	jerry_value_t ret = jerry_call(resolve, jerry_undefined(), args, argscnt);
	jerry_value_free(resolve);
	jerry_value_free(questiono);
	return ret;
}

jerry_value_t isr_script_result_constructors(jerry_value_t *answerc, jerry_value_t *forwardc) {
	jerry_value_t result = isr_module_result();
	if (jerry_value_is_exception(result)) return result;

	jerry_value_t namespace = jerry_module_namespace(result);
	
	jerry_value_t answerk = jerry_string_sz("Answer");
	jerry_value_t answer = jerry_object_get(namespace, answerk);
	jerry_value_free(answerk);
	if (jerry_value_is_exception(answer)) return answer;
	*answerc = answer;

	jerry_value_t forwardk = jerry_string_sz("Forward");
	jerry_value_t forward = jerry_object_get(namespace, forwardk);
	jerry_value_free(forwardk);
	if (jerry_value_is_exception(forward)) return forward;
	*forwardc = forward;

	jerry_value_free(namespace);
	jerry_value_free(result);

	return jerry_boolean(true);
}

unsigned char *isr_from_jerry_typedarray(jerry_value_t jerry_typedarray, uint16_t *length) {
    jerry_length_t bufflength, buffoffset;
    jerry_value_t buff = jerry_typedarray_buffer(jerry_typedarray, &buffoffset, &bufflength);
    if (jerry_value_is_exception(buff)) return NULL;

    unsigned char *ret = malloc(bufflength * sizeof(unsigned char));
    jerry_length_t read = jerry_arraybuffer_read(buff, buffoffset, ret, bufflength);
    *length = read;

    jerry_value_free(buff);

    return ret;
}

struct resolve_result *isr_resolve_result_exception(jerry_value_t exception) {
	jerry_value_t str = jerry_value_to_string(jerry_exception_value(exception, true));
    jerry_size_t strsize = jerry_string_size(str, JERRY_ENCODING_UTF8);
    jerry_char_t *buff = malloc(strsize * sizeof(jerry_char_t));
	jerry_size_t buffsize = jerry_string_to_buffer(str, JERRY_ENCODING_UTF8, buff, jerry_string_size(str, JERRY_ENCODING_UTF8));
	buff[buffsize] = '\0';
	printf("%s\n", buff);
    free(buff);

	struct resolve_result *ret = malloc(sizeof(struct resolve_result));
	ret->type = FALLBACK;
	return ret;
}

struct resolve_result *isr_resolve_result(jerry_value_t call_result, jerry_value_t answerc, jerry_value_t forwardc) {
	if (jerry_value_is_exception(call_result)) return isr_resolve_result_exception(jerry_undefined());

	jerry_value_t isanswer = jerry_binary_op(JERRY_BIN_OP_INSTANCEOF, call_result, answerc);
	if (jerry_value_is_exception(isanswer)) return isr_resolve_result_exception(isanswer);

	jerry_value_t isforward = jerry_binary_op(JERRY_BIN_OP_INSTANCEOF, call_result, forwardc);
	if (jerry_value_is_exception(isforward)) return isr_resolve_result_exception(isforward);

	if (jerry_value_to_boolean(isanswer)) {
		jerry_value_t typek = jerry_string_sz("type");
		jerry_value_t type = jerry_object_get(call_result, typek);
		jerry_value_free(typek);
		if (jerry_value_is_exception(type)) return isr_resolve_result_exception(type);
		if (!jerry_value_is_number(type)) {
			jerry_value_t exception = jerry_throw_value(jerry_string_sz("type is not a number"), true);
			return isr_resolve_result_exception(exception);
		}

		jerry_value_t rdatak = jerry_string_sz("rdata");
		jerry_value_t rdata = jerry_object_get(call_result, rdatak);
		jerry_value_free(rdatak);
		if (jerry_value_is_exception(rdata)) return isr_resolve_result_exception(rdata);
		if (!jerry_value_is_object(rdata)) {
			jerry_value_t exception = jerry_throw_value(jerry_string_sz("rdata is not an object"), true);
			return isr_resolve_result_exception(exception);
		}

		jerry_value_t touint8arrayk = jerry_string_sz("toUint8Array");
		jerry_value_t touint8array = jerry_object_get(rdata, touint8arrayk);
		jerry_value_free(touint8arrayk);
		if (jerry_value_is_exception(touint8array)) return isr_resolve_result_exception(touint8array);
		if (!jerry_value_is_function(touint8array)) {
			jerry_value_t exception = jerry_throw_value(jerry_string_sz("toUint8Array is not a function"), true);
			return isr_resolve_result_exception(exception);
		}

		jerry_value_t typedarray = jerry_call(touint8array, rdata, NULL, 0);
		if (jerry_value_is_exception(typedarray)) return isr_resolve_result_exception(typedarray);
		if (!jerry_value_is_typedarray(typedarray)) {
			jerry_value_t exception = jerry_throw_value(jerry_string_sz("toUint8Array didn't return TypedArray"), true);
			return isr_resolve_result_exception(exception);
		}

		uint16_t rdlength;
		unsigned char* value = isr_from_jerry_typedarray(typedarray, &rdlength);
		jerry_value_free(typedarray);

		struct resolve_result *ret = malloc(sizeof(struct resolve_result));
		ret->type = ANSWER;

        struct resolve_result_answer *ans = malloc(sizeof(struct resolve_result_answer));
        ans->type = jerry_value_as_uint32(type);
        ans->rdlength = rdlength;
        ans->rdata = value;

        ret->value.answer = ans;

		return ret;
	} else if (jerry_value_to_boolean(isforward)) {
		jerry_value_t ipk = jerry_string_sz("ip");
		jerry_value_t ip = jerry_object_get(call_result, ipk);
		jerry_value_free(ipk);
		if (jerry_value_is_exception(ip)) return isr_resolve_result_exception(ip);

		char *buff = malloc(16 * sizeof(char));
		jerry_size_t length = jerry_string_to_buffer(jerry_value_to_string(ip), JERRY_ENCODING_UTF8, (jerry_char_t *) buff, 16);	
		buff[length] = '\0';

		struct resolve_result *ret = malloc(sizeof(struct resolve_result));
		ret->type = FORWARD;

        struct resolve_result_forward *fwd = malloc(sizeof(struct resolve_result_forward));
        fwd->ip = buff;

        ret->value.forward = fwd;

        return ret;
	} else {
		struct resolve_result *ret = malloc(sizeof(struct resolve_result));
		ret->type = FALLBACK;
		
		return ret;
	}
}
	
