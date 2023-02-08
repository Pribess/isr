/* Special type to allow combination of multiple data */

function Concat(rdatas) {
    this.rdatas = rdatas;
}

Concat.prototype.toUint8Array = function() {
    var arrays = [];
    var length = 0;
    for(var i in this.rdatas) {
        arrays.push(this.rdatas[i].toUint8Array());
        length += arrays[i].length;
    }
    
    var ret = new Uint8Array(length);
    var cursor = 0;
    for(var i in arrays) {
        ret.set(arrays[i], cursor);
        cursor += arrays[i].length;
    }

    return ret;
};


/* Special type to allow writing raw octet-arrays */

function RawRData(uint8Array) {
    this.uint8Array = uint8Array;
}

RawRData.prototype.toUint8Array = function() {
    return this.uint8Array;
};

/* Well-known RDATA formats, whose names and definitions implement their respective RFCs */

/* RFC 1035 */
function CharacterString(str) {
    this.str = str;
}

CharacterString.prototype.toUint8Array = function() {
    var ret = new Uint8Array(this.str.length + 1); // TODO : Make sure of ascii string?
    var encoder = new TextEncoder();
    ret.set([this.str.length], 0);
    ret.set(encoder.encode(this.str), 1);
    return ret;
}

/* RFC 1035 */
function DomainName(str) {
    if (str.slice(-1) == '.') {
        this.str = str.slice(0, -1); /* Remove trailing dot for convenience */
    } else {
        this.str = str;
    }
}

DomainName.prototype.toUint8Array = function() { 
    var ret = new Uint8Array(this.str.length + 2);
    var cursor = 0;
    var encoder = new TextEncoder();
    var labels = this.str.split(".");
    for (var i in labels) {
        ret.set([labels[i].length], cursor);
        ret.set(encoder.encode(labels[i]), cursor+1);
        cursor += labels[i].length + 1;
    }
    ret.set([0], cursor);
    return ret;
};

/* RFC 1035 */
function IPV4(ip) {
    this.ip = ip;
}

IPV4.prototype.toUint8Array = function() {
    // TODO: implement validating & error throwing
    var ret = new Uint8Array(4);
    var octets = this.ip.split(".");
    for (var i in octets) {
        ret.set([parseInt(octets[i])], i);
    }
    return ret;
};