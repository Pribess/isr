/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/native/encode.c
*/

#include "encode.h"

static jerry_value_t isr_native_encode(const jerry_call_info_t *call_info_p, const jerry_value_t args_p[], const jerry_length_t args_cnt) {
	if (args_cnt != 1) return jerry_throw_value(jerry_string_sz("Called nativeEncode with wrong parameters"), true);

	jerry_value_t string = jerry_value_to_string(args_p[0]);
	jerry_size_t length = jerry_string_length(string);

	jerry_char_t *buff = malloc((length + 1) * sizeof(jerry_char_t));

	jerry_size_t copied = jerry_string_to_buffer(string, JERRY_ENCODING_UTF8, buff, length);
	jerry_value_free(string);
	buff[copied] = '\0';

	jerry_value_t arraybuffer = jerry_arraybuffer_external(buff, copied, buff);

	jerry_value_t ret = jerry_typedarray_with_buffer_span(JERRY_TYPEDARRAY_UINT8, arraybuffer, 0, copied);
	jerry_value_free(arraybuffer);

	return ret;
}

jerry_value_t isr_module_native_encode() {
	const jerry_value_t exports[1] = {
		jerry_string_sz("nativeEncode"),
	};

	jerry_value_t ret = jerry_native_module(NULL, exports, 1);

	jerry_value_t val0 = jerry_function_external(&isr_native_encode);
	if (jerry_value_is_exception(val0)) return val0;
	jerry_value_t set0 = jerry_native_module_set(ret, exports[0], val0);
	if (jerry_value_is_exception(set0)) return set0;
	jerry_value_free(set0);
	jerry_value_free(val0);
	jerry_value_free(exports[0]);

	return ret;
}
