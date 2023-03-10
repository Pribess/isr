/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/module.c
*/

#include "module.h"

#include "js/rdata_js.h"
#include "js/result_js.h"
#include "js/state_js.h"
#include "js/type_js.h"
#include "js/util_js.h"

extern struct config isr_config;

bool isr_module_resolve_native(const jerry_value_t canonical_name, jerry_value_t *result) {
	jerry_char_t buff[32];
	jerry_size_t size = jerry_string_to_buffer(canonical_name, JERRY_ENCODING_UTF8, buff, 32);
	buff[size] = '\0';

	if (strcmp((char *) buff, "native/encode") == 0) {
		jerry_value_t ret = isr_module_native_encode();
		if (!jerry_value_is_exception(ret)) {
			*result = ret;
			return true;
		}
		jerry_value_free(ret);
	}

	return false;
}

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
		jerry_value_free(ret);
	} else if (strcmp((char *) buff, "result.js") == 0) {
		jerry_value_t ret = jerry_parse(result_js, result_js_len, &opts);
		if (!jerry_value_is_exception(ret)) {
			*result = ret;
			return true;
		}
		jerry_value_free(ret);
	} else if (strcmp((char *) buff, "state.js") == 0) {
		jerry_value_t ret = jerry_parse(state_js, state_js_len, &opts);
		if (!jerry_value_is_exception(ret)) {
			*result = ret;
			return true;
		}
		jerry_value_free(ret);
	} else if (strcmp((char *) buff, "type.js") == 0) {
		jerry_value_t ret = jerry_parse(type_js, type_js_len, &opts);
		if (!jerry_value_is_exception(ret)) {
			*result = ret;
			return true;
		}
		jerry_value_free(ret);
	} else if (strcmp((char *) buff, "util.js") == 0) {
		jerry_value_t ret = jerry_parse(util_js, util_js_len, &opts);
		if (!jerry_value_is_exception(ret)) {
			*result = ret;
			return true;
		}
		jerry_value_free(ret);
	}

	return false;
}

jerry_value_t isr_module_get_canonical_name_file(const jerry_value_t name) {
	jerry_value_t getter_script_dir = jerry_string_sz(isr_config.getter_script_dir);
	jerry_value_t slash = jerry_string_sz("/");

	jerry_value_t ret = jerry_binary_op(JERRY_BIN_OP_ADD, jerry_binary_op(JERRY_BIN_OP_ADD, getter_script_dir, slash), name);

	jerry_value_free(getter_script_dir);
	jerry_value_free(slash);

	return ret;
};

bool isr_module_resolve_file(const jerry_value_t canonical_name, jerry_value_t *result) {
	jerry_char_t buff[4097];
	jerry_size_t size = jerry_string_to_buffer(canonical_name, JERRY_ENCODING_UTF8, buff, 4097);
	buff[size] = '\0';

	FILE *file = fopen((char *)buff, "r");
	if (file == NULL) {
		printf("isr: can't open %s\n", buff);
		return false;
	}

	fseek(file, 0L, SEEK_END);
	long sz = ftell(file);
	rewind(file);

	jerry_char_t *script = malloc(sz * sizeof(jerry_char_t));
	long script_sz = fread(script, 1, sz, file);

	jerry_parse_options_t opts;
	opts.options = JERRY_PARSE_MODULE;

	jerry_value_t ret = jerry_parse(script, script_sz, &opts);
	free(script);
	if (!jerry_value_is_exception(ret)) {
		*result = ret;
		return true;
	}
	jerry_value_free(ret);

	jerry_value_t exception_val = jerry_exception_value(ret, true);
	jerry_value_t str = jerry_value_to_string(exception_val);
	jerry_value_free(exception_val);
	jerry_size_t strsize = jerry_string_size(str, JERRY_ENCODING_UTF8);
	jerry_char_t *err = malloc(strsize * sizeof(jerry_char_t));
	jerry_size_t errsize = jerry_string_to_buffer(str, JERRY_ENCODING_UTF8, buff, jerry_string_size(str, JERRY_ENCODING_UTF8));
	err[errsize] = '\0';
	printf("isr: %s: %s\n", buff, err);

	jerry_value_free(str);
	free(err);

	return false;
}

const jerryx_module_resolver_t **isr_module_resolvers(size_t *count) {
	jerryx_module_resolver_t *native = malloc(sizeof(jerryx_module_resolver_t));
	native->get_canonical_name_p = NULL;
	native->resolve_p = &isr_module_resolve_native;

	jerryx_module_resolver_t *compiled_in = malloc(sizeof(jerryx_module_resolver_t));
	compiled_in->get_canonical_name_p = NULL;
	compiled_in->resolve_p = &isr_module_resolve_compiled_in;

	jerryx_module_resolver_t *file = malloc(sizeof(jerryx_module_resolver_t));
	file->get_canonical_name_p = &isr_module_get_canonical_name_file;
	file->resolve_p = &isr_module_resolve_file;

	*count = 3;
	const jerryx_module_resolver_t **ret = malloc(*count * sizeof(jerryx_module_resolver_t *));
	ret[0] = native;
	ret[1] = compiled_in;
	ret[2] = file;
	return ret;
}

jerry_value_t isr_module_resolve_callback(const jerry_value_t specifier, const jerry_value_t referrer, void *user_p) {
	size_t count;
	const jerryx_module_resolver_t **resolvers = isr_module_resolvers(&count);
	jerry_value_t ret = jerryx_module_resolve(specifier, resolvers, count);

	return ret;
}

jerry_value_t isr_module_result() {
	size_t count;
	const jerryx_module_resolver_t **resolvers = isr_module_resolvers(&count);

	jerry_value_t resultn = jerry_string_sz("result.js");
	jerry_value_t ret = jerryx_module_resolve(resultn, resolvers, count);
	jerry_value_free(resultn);

	return ret;
}

jerry_value_t isr_module_state() {
	size_t count;
	const jerryx_module_resolver_t **resolvers = isr_module_resolvers(&count);

	jerry_value_t staten = jerry_string_sz("state.js");
	jerry_value_t ret = jerryx_module_resolve(staten, resolvers, count);
	jerry_value_free(staten);

	return ret;
}

