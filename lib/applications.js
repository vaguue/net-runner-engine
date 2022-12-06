const { v4: uuidv4 } = require('uuid');
const is = require('./is');

class Application {
  
  constructor({ type, fields, ...rest } = {}) {
    if (fields.some(e => !rest.hasOwnProperty(e))) {
      throw Error(`Can't initialize ${type} with ${JSON.stringify(rest)}`);
    }
    this.type = type;
    this.fields = fields;
    this.data = rest;
    this.id = uuidv4();
  }

  toObject() {
    return { ...this.data, type: this.type };
  }

  clone() {
    return new Application({ ...this.toObject() });
  }
}

class RawSocketClient extends Application {
  constructor(args = {}) {
    super({ ...args, type: 'raw-socket-client', fields: ['addr'] });
  }

  static type = 'raw-socket-client';
  
  static isValid(obj) {
    const { addr } = obj || {};
    if (!is.ip(addr)) {
      throw Error('addr field has to be IPv4 address');
    }
    return true;
  }
}

class RawSocketServer extends Application {
  constructor(args = {}) {
    super({ ...args, type: 'raw-socket-server', fields: [] });
  }

  static type = 'raw-socket-server';

  static isValid(obj = {}) {
    if (obj.hasOwnProperty('addr')) {
      const { addr } = obj;
      if (!is.ip(addr)) {
        throw Error('addr field has to be IPv4 address');
      }
    }
    return true;
  }
}

class TCPClient extends Application {
  constructor(args = {}) {
    super({ ...args, type: 'tcp-client', fields: ['addr', 'port'] });
  }

  static type = 'tcp-client';

  static isValid(obj) {
    const { addr, port } = obj;
    if (!is.ip(addr)) {
      throw Error('addr field has to be IPv4 address');
    }
    if (!is.number(parseInt(port)) || parseInt(port).toString() != port.toString()) {
      throw Error('invalid port');
    }
    return true;
  }
}

class TCPServer extends Application {
  constructor(args = {}) {
    super({ ...args, type: 'tcp-server', fields: ['port'] });
  }

  static type = 'tcp-server';

  static isValid(obj) {
    const { addr, port } = obj;
    if (obj.hasOwnProperty('addr') && !is.ip(addr)) {
      throw Error('addr field has to be IPv4 address');
    }
    if (!is.number(parseInt(port)) || parseInt(port).toString() != port.toString()) {
      throw Error('invalid port');
    }
    return true;
  }
}

class UDPClient extends Application {
  constructor(args = {}) {
    super({ ...args, type: 'udp-client', fields: ['addr', 'port'] });
  }

  static type = 'udp-client';

  static isValid(obj) {
    const { addr, port } = obj;
    if (!is.ip(addr)) {
      throw Error('addr field has to be IPv4 address');
    }
    if (!is.number(parseInt(port)) || parseInt(port).toString() != port.toString()) {
      throw Error('invalid port');
    }
    return true;
  }
}

class UDPServer extends Application {
  constructor(args = {}) {
    super({ ...args, type: 'udp-server', fields: ['port'] });
  }

  static type = 'udp-server';

  static isValid(obj) {
    const { addr, port } = obj;
    if (obj.hasOwnProperty('addr') && !is.ip(addr)) {
      throw Error('addr field has to be IPv4 address');
    }
    if (!is.number(parseInt(port)) || parseInt(port).toString() != port.toString()) {
      throw Error('invalid port');
    }
    return true;
  }
}

class Ping extends Application {
  constructor(args = {}) {
    super({ ...args, type: 'ping', fields: ['addr'] });
  }

  static type = 'ping';

  static isValid(obj) {
    const { addr } = obj;
    if (!is.ip(addr)) {
      throw Error('addr field has to be IPv4 address');
    }
  }
}

class Sink extends Application {
  constructor(args = {}) {
    super({ ...args, type: 'sink', fields: [] });
  }

  static type = 'sink';

  static isValid(obj) {
    if (obj.hasOwnProperty('addr')) {
      if (!is.ip(obj.addr)) {
        throw Error('addr field has to be IPv4 address');
      }
    }
    return true;
  }
}

module.exports = { Application, Ping, Sink, UDPClient, UDPServer, TCPServer, TCPClient, RawSocketClient, RawSocketServer };
