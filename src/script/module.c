/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/module.c
*/

#include "module.h"

#include "js/rdata_js.h"
#include "js/result_js.h"
#include "js/type_js.h"

bool isr_module_resolve_compiled_in(const jerry_value_t canonical_name, jerry_value_t *result) {
	jerry_char_t buff[16];
	jerry_size_t size = jerry_string_to_buffer(canonical_name, JERRY_ENCODING_UTF8, buff, 16);
	buff[size] = '\0';

	jerry_parse_options_t opts;
	opts.options = JERRY_PARSE_MODULE;

	if (strcmp((char *) buff, "rdata.js") == 0) {
		jerry_value_t ret = jerry_parse(rdata_js, rdata_js_len, &opts);
		if (!jerry_value_is_exception(ret)) {
			*result = ret;
			return true;
		}
	} else if (strcmp((char *) buff, "result.js") == 0) {
		jerry_value_t ret = jerry_parse(result_js, result_js_len, &opts);
		if (!jerry_value_is_exception(ret)) {
			*result = ret;
			return true;
		}
	} else if (strcmp((char *) buff, "type.js") == 0) {
		jerry_value_t ret = jerry_parse(type_js, type_js_len, &opts);
		if (!jerry_value_is_exception(ret)) {
			*result = ret;
			return true;
		}
	}

	return false;
}

const jerryx_module_resolver_t **isr_module_resolvers(size_t *count) {
	jerryx_module_resolver_t *compiled_in = malloc(sizeof(jerryx_module_resolver_t));
	compiled_in->get_canonical_name_p = NULL;
	compiled_in->resolve_p = &isr_module_resolve_compiled_in;

	*count = 1;
	const jerryx_module_resolver_t **ret = malloc(*count * sizeof(jerryx_module_resolver_t *));
	ret[0] = compiled_in;
	return ret;
}

jerry_value_t isr_module_resolve_callback(const jerry_value_t specifier, const jerry_value_t referrer, void *user_p) {
	size_t count;
	const jerryx_module_resolver_t **resolvers = isr_module_resolvers(&count);
	return jerryx_module_resolve(specifier, resolvers, count);
}

jerry_value_t isr_module_result() {
	size_t count;
	const jerryx_module_resolver_t **resolvers = isr_module_resolvers(&count);

	jerry_value_t rdatan = jerry_string_sz("rdata.js");
	jerry_value_t ret = jerryx_module_resolve(rdatan, resolvers, count);
	jerry_value_free(rdatan);

	return ret;
}