import { TextEncoder } from "util.js"

/* Special type to allow combination of multiple data */

export class Concat {
    constructor(...rdatas) {
        this.rdatas = rdatas;
    }

    toUint8Array() {
        let arrays = this.rdatas.map(x => x.toUint8Array());
        let length = arrays.reduce((acc, c) => acc + c.length, 0);

        let ret = new Uint8Array(length);
        arrays.reduce((cursor, rdata) => {
            ret.set(rdata, cursor);
            return cursor + rdata.length;
        }, 0);

        return ret;
    }
}

/* Special type to allow writing raw octet-arrays */

export class RawRData {
    constructor(uint8Array) {
        this.uint8Array = uint8Array;
    }

    toUint8Array() {
        return this.uint8Array;
    }
}

/* Well-known RDATA formats, whose names and definitions implement their respective RFCs */

/* RFC 1035 */

export class CharacterString {
    constructor(str) {
        this.str = str;
    }

    toUint8Array() {
        let ret = new Uint8Array(this.str.length+1);
        var encoder = new TextEncoder();
        ret.set([this.str.length], 0);
        ret.set(encoder.encode(this.str), 1);
        return ret;
    }
}

/* RFC 1035 */
export class DomainName {
    constructor(str) {
        if (str.slice(-1) === ".") {
            this.str = str.slice(0, -1); /* Removing trailing dot for convenience */
        } else {
            this.str = str;
        }
    }

    toUint8Array() {
        let ret = new Uint8Array(this.str.length + 2);
        let encoder = new TextEncoder();
        let labels = this.str.split(".");
        let last = labels.reduce((cursor, label) => {
            ret.set([label.length], cursor);
            ret.set(encoder.encode(label), cursor+1);
            return cursor + label.length + 1;
        }, 0);
        ret.set([0], last);
        return ret;
    }
}

/* RFC 1035 */
export class IPV4 {
    constructor(ip) {
        this.ip = ip;
    }

    toUint8Array() {
        // TODO: implement validating & error throwing
        let ret = new Uint8Array(4);
        let octets = this.ip.split(".");
        octets.forEach((octet, idx) => ret.set([parseInt(octet)], idx));
        return ret;
    }
}
