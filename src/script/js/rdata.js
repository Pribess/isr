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

function RawRData(uint8Array) {
    this.uint8Array = uint8Array;
}

RawRData.prototype.toUint8Array = function() {
    return this.uint8Array;
};