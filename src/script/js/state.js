export class StateProvider {
    static #providers = new Map();

    static register(name, provider) {
        // TODO: filter if name is a possible js property name?
        if (this.#providers.has(name)) {
            this.#providers.get(name).push(provider);
        } else {
            this.#providers.set(name, [provider]);
        }
    }

    constructor(enableCallback, dataCallback) {
        self.enableCallback = enableCallback;
        self.dataCallback = dataCallback;
    }
}
