import { nativeEncode } from "native/encode"

export function TextEncoder() {

}

TextEncoder.prototype.encoding = "utf-8";

TextEncoder.prototype.encode = function(string) {
    return nativeEncode(string);
};

