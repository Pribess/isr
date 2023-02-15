import { nativeEncode } from "native/encode"

export class TextEncoder {
    get encoding() {
        return "utf-8";
    }

    encode(str) {
        return nativeEncode(str);
    }
}
