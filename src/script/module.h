/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/module.h
*/

#ifndef ISR_SCRIPT_MODULE
#define ISR_SCRIPT_MODULE

#include <jerryscript.h>
#include <jerryscript-ext/module.h>
#include <stdlib.h>
#include <string.h>

#include "native/encode.h"

jerry_value_t isr_module_resolve_callback(const jerry_value_t specifier, const jerry_value_t referrer, void *user_p);

jerry_value_t isr_module_result();

#endif
