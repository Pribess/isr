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

bool isr_script_register_state(const jerry_value_t name, const jerry_value_t value, void *user_data) {
	jerry_value_t enable_cb = jerry_object_get_sz(value, "enableCallback");
	jerry_value_t data_cb = jerry_object_get_sz(value, "dataCallback");

	jerry_value_t enable = jerry_call(enable_cb, jerry_undefined(), NULL, 0);
	jerry_value_free(enable_cb);

	if (jerry_value_is_exception(enable)
			|| !jerry_value_is_boolean(enable)
			|| !jerry_value_to_boolean(enable))
		goto free_enable;

	struct state_providers_and_size *data = user_data;
	
	struct state_provider *provider = malloc(sizeof(struct state_provider));
	provider->callback = data_cb;

	data->providers[*data->size] = provider;
	*data->size += 1;

free_enable:
	jerry_value_free(enable);
	jerry_value_free(data_cb);

	return true;
}


bool isr_script_register_state_by_name(const jerry_value_t name, const jerry_value_t value, void *user_data) {
	struct state_providers_and_size *data = user_data;

	jerry_size_t path_str_size = jerry_string_size(name, JERRY_ENCODING_UTF8);
	jerry_char_t *path_str = malloc((path_str_size + 1) * sizeof(jerry_char_t));
	jerry_size_t copied = jerry_string_to_buffer(name, JERRY_ENCODING_UTF8, path_str, path_str_size);
	path_str[copied] = '\0';

	size_t state_providers_size_pre = *data->size;
	jerry_object_foreach(value, &isr_script_register_state, data);

	if (*data->size == state_providers_size_pre) {
		printf("isr: no valid StateProvider found for name %s\n", path_str);
		return true;
	}

	struct state_provider *first = data->providers[state_providers_size_pre];
	first->is_first = true;
	first->path = isr_script_state_path(path_str, copied, &first->path_length);
	free(path_str);

	return true;
}

struct state_provider **isr_script_state_providers(size_t *size) {
	struct state_provider **ret = NULL;

	jerry_value_t state_module = isr_module_state();
	if (jerry_value_is_exception(state_module)) goto free_state_module;

	jerry_value_t namespace = jerry_module_namespace(state_module);

	jerry_value_t state_provider_class = jerry_object_get_sz(namespace, "StateProvider");
	if (jerry_value_is_exception(state_provider_class)) goto free_state_provider_class;

	jerry_value_t get_providers = jerry_object_get_sz(state_provider_class, "getProviders");
	if (jerry_value_is_exception(get_providers)) goto free_get_providers;

	jerry_value_t providers = jerry_call(get_providers, state_provider_class, NULL, 0);
	if (jerry_value_is_exception(providers)) goto free_providers;

	jerry_value_t providers_keys = jerry_object_keys(providers);
	if (jerry_value_is_exception(providers_keys)) goto free_providers_keys;


	uint32_t providers_size = jerry_array_length(providers_keys);
	ret = malloc(providers_size * sizeof(struct state_provider));
	*size = 0;

	struct state_providers_and_size data = { .providers = ret, .size = size };

	jerry_object_foreach(providers, &isr_script_register_state_by_name, &data);

	ret = realloc(ret, *data.size * sizeof(struct state_provider));

free_providers_keys:
	jerry_value_free(providers_keys);
free_providers:
	jerry_value_free(providers);
free_get_providers:
	jerry_value_free(get_providers);
free_state_provider_class:
	jerry_value_free(state_provider_class);
	jerry_value_free(namespace);
free_state_module:
	jerry_value_free(state_module);

	return ret;
}

void isr_script_set_data_on_path(char **path, size_t path_length, jerry_value_t on, jerry_value_t data) {
	char *path_part = path[0];

	if (path_length == 1) {
		jerry_value_t setr = jerry_object_set_sz(on, path_part, data);
		jerry_value_free(setr);
	} else {
		jerry_value_t new_on = jerry_object_get_sz(on, path_part);

		if (jerry_value_is_exception(new_on) || jerry_value_is_undefined(new_on)) {
			jerry_value_t obj = jerry_object();
			jerry_object_set_sz(on, path_part, obj);
			jerry_value_free(obj);
			isr_script_set_data_on_path(path + 1, path_length - 1, jerry_object_get_sz(on, path_part), data);
		} else {
			isr_script_set_data_on_path(path + 1, path_length - 1, new_on, data);
		}

		jerry_value_free(new_on);
	}
}

jerry_value_t isr_script_object_state(struct state_provider **providers, size_t size) {
	jerry_value_t ret = jerry_object();

	struct state_provider *current_first = NULL;
	bool current_loaded = false;
	for (int i = 0; i < size; i++) {
		struct state_provider *provider = providers[i];

		if (provider->is_first) {
			current_first = provider;
			current_loaded = false;
		}

		if (current_loaded) 
			continue;

		jerry_value_t data = jerry_call(provider->callback, jerry_undefined(), NULL, 0);
		if (jerry_value_is_exception(data)) goto free_data;

		isr_script_set_data_on_path(current_first->path, current_first->path_length, ret, data);
		current_loaded = true;

free_data:
		jerry_value_free(data);
	}

	return ret;
}
