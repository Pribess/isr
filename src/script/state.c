/*
		Copyright (C) 2023
			Pribess (Heewon Cho)
			Jhyub	(Janghyub Seo)
		src/script/state.c
*/

#include "state.h"

struct state_providers_and_size {
	struct state_provider **providers;
	size_t *size;
};

char **isr_script_state_path(jerry_char_t *path_str, jerry_size_t path_str_size, size_t *path_size) {
	*path_size = 0;
	char **ret = malloc((path_str_size / 2 + 5) * sizeof(char *)); /* temporary, resize later */

	int current_start = 0;
	for(int cursor = 0; cursor <= path_str_size; cursor++) {
		if (path_str[cursor] == '.') path_str[cursor] = '\0';
		if (path_str[cursor] == '\0') {
			if(cursor == current_start) continue; /* trailing dot? */

			ret[*path_size] = malloc((cursor - current_start) * sizeof(char));
			strcpy(ret[*path_size], (char *)path_str + current_start);
			*path_size += 1;

			current_start = cursor + 1;
		}
	}

	ret = realloc(ret, *path_size * sizeof(char *));

	return ret;
}

bool isr_script_register_state_maybe(const jerry_value_t name, const jerry_value_t value, void *user_data) {
	jerry_value_t enable_cb = jerry_object_get_sz(value, "enableCallback");
	jerry_value_t data_cb = jerry_object_get_sz(value, "dataCallback");

	jerry_value_t enable = jerry_call(enable_cb, jerry_undefined(), NULL, 0);
	jerry_value_free(enable_cb);

	if (jerry_value_is_exception(enable)
			|| !jerry_value_is_boolean(enable)
			|| !jerry_value_to_boolean(enable))
		return true;

	*(struct state_provider **)user_data = malloc(sizeof(struct state_provider));
	struct state_provider *provider = *(struct state_provider **)user_data;

	provider->callback = data_cb;

	return false;
}


bool isr_script_register_state_by_name(const jerry_value_t name, const jerry_value_t value, void *user_data) {
	struct state_providers_and_size *data = user_data;

	jerry_size_t path_str_size = jerry_string_size(name, JERRY_ENCODING_UTF8);
	jerry_char_t *path_str = malloc((path_str_size + 1) * sizeof(jerry_char_t));
	jerry_size_t copied = jerry_string_to_buffer(name, JERRY_ENCODING_UTF8, path_str, path_str_size);
	path_str[copied] = '\0';

	struct state_provider *provider = NULL;

	bool foreached = jerry_object_foreach(value, &isr_script_register_state_maybe, &provider);

	if (provider == NULL) {
		printf("isr: no valid StateProvider found for name %s\n", path_str);
		return true;
	}

	provider->path = isr_script_state_path(path_str, copied, &provider->path_length);
	free(path_str);

	data->providers[*data->size] = provider;
	*data->size += 1;
	return true;
}

struct state_provider **isr_script_state_providers(size_t *size) {
	jerry_value_t state_module = isr_module_state();
	if (jerry_value_is_exception(state_module)) return NULL;

	jerry_value_t namespace = jerry_module_namespace(state_module);
	jerry_value_free(state_module);

	jerry_value_t state_provider_class = jerry_object_get_sz(namespace, "StateProvider");
	jerry_value_free(namespace);
	if (jerry_value_is_exception(state_provider_class)) return NULL;

	jerry_value_t get_providers = jerry_object_get_sz(state_provider_class, "getProviders");
	if (jerry_value_is_exception(get_providers)) return NULL;

	jerry_value_t providers = jerry_call(get_providers, state_provider_class, NULL, 0);
	jerry_value_free(state_provider_class);
	if (jerry_value_is_exception(providers)) return NULL;

	jerry_value_t providers_keys = jerry_object_keys(providers);
	if (jerry_value_is_exception(providers_keys)) {
		jerry_value_free(providers);
		return NULL;
	}

	uint32_t providers_size = jerry_array_length(providers_keys);
	jerry_value_free(providers_keys);
	struct state_provider **ret = malloc(providers_size * sizeof(struct state_provider));
	*size = 0;

	struct state_providers_and_size data = { .providers = ret, .size = size };

	bool foreached = jerry_object_foreach(providers, &isr_script_register_state_by_name, &data);
	jerry_value_free(providers);

	return ret;
}

void isr_script_set_data_on_path(char **path, size_t path_length, jerry_value_t on, jerry_value_t data) {
	char *path_part = path[0];

	if (path_length == 1) {
		jerry_object_set_sz(on, path_part, data);
	} else {
		jerry_value_t new_on = jerry_object_get_sz(on, path_part);

		if (jerry_value_is_exception(new_on) || jerry_value_is_undefined(new_on)) {
			jerry_object_set_sz(on, path_part, jerry_object());
			isr_script_set_data_on_path(path + 1, path_length - 1, jerry_object_get_sz(on, path_part), data);
		} else {
			isr_script_set_data_on_path(path + 1, path_length - 1, new_on, data);
		}
	}
}

jerry_value_t isr_script_state(struct state_provider **providers, size_t size) {
	jerry_value_t ret = jerry_object();

	for (int i = 0; i < size; i++) {
		struct state_provider *provider = providers[i];

		jerry_value_t data = jerry_call(provider->callback, jerry_undefined(), NULL, 0);
		if (jerry_value_is_exception(data)) continue;

		isr_script_set_data_on_path(provider->path, provider->path_length, ret, data);
	}

	return ret;
}
