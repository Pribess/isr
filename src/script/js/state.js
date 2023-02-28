export class StateProvider {
    static #providers = new Object();

    static register(name, provider) {
        // TODO: filter if name is a possible js property name?
        if (this.#providers.hasOwnProperty(name)) {
            this.#providers[name].push(provider);
        } else {
            this.#providers[name] = [provider];
        }
    }

	static getProviders() {
		return this.#providers;
	}

    constructor(enableCallback, dataCallback) {
        this.enableCallback = enableCallback;
        this.dataCallback = dataCallback;
    }
}
