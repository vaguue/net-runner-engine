const { v4: uuidv4 } = require('uuid');

function pickFromObject(obj, keys) {
  return keys.reduce((res, e) => obj[e] !== undefined ? {...res, [e]:obj[e]} : {...res}, {});
}

function getFromObject(object, keys) {
  const res = {};
  for (const key of keys) {
    if (!object.hasOwnProperty(key)) {
      throw Error(`Object has no ${key}`);
    }
    res[key] = object[key];
  }
  return res;
}

class Node {
  nodeFields = ['id', 'type', 'x', 'y', 'applications'];
  constructor(...args) {
    const { type = 'pc', name = null, x, y } = (args.length > 1 ? args[1] : args[0]) || {};
    this.id = uuidv4();
    this.type = type;
    this.name = name;
    this.x = x;
    this.y = y;
    this.applications = [];
    this.parentNetwork = null;
    this.parentNetworkId = null;

    if (args.length > 1) {
      const net = args[0];
      net.addNode(this);
    }
  }

  toObject() {
    let res = getFromObject(this, this.nodeFields);
    res.applications = res.applications.map(e => e.toObject());
    if (this.additionalFields) {
      res = { ...res, ...pickFromObject(this, this.additionalFields) };
    }
    if (this.name !== null) {
      res.name = this.name
    }
    return res;
  }

  clone() {
    return new Node(this.toObject());
  }

  connect(dstNode, options = {}) {
    if (!dstNode instanceof Node) {
      throw Error('Destionation is not an instance of Node class');
    }
    if (this.parentNetworkId === null || this.parentNetworkId != dstNode.parentNetworkId) {
      throw Error('Nodes belong to different networks of are not in any network');
    }
    this.parentNetwork.connectNodes(this, dstNode, options);
  }

  disconnect(dstNode) {
    if (!dstNode instanceof Node) {
      throw Error('Destionation is not an instance of Node class');
    }
    if (this.parentNetworkId === null || this.parentNetworkId != dstNode.parentNetworkId) {
      throw Error('Nodes belong to different networks of are not in any network');
    }
    this.parentNetwork.disconnectNodes(this, dstNode);
  }
}

class Switch extends Node {
  constructor(...args) {
    super(...args);
    this.type = 'switch';
  }

  static type = 'switch';
}

Switch.prototype.setupApplication = null;

class Hub extends Node {
  constructor(...args) {
    super(...args);
    if (args.delay) {
      this.delay = args.delay;
    }
    if (args.dataRate) {
      this.dataRate = args.dataRate;
    }
    this.additionalFields = ['delay', 'dataRate'];
    this.type = 'hub';
  }

  static type = 'hub';
}

Hub.prototype.setupApplication = null;

class Host extends Node {
  constructor(...args) {
    super(...args);
    this.type = 'pc';
  }

  static type = 'pc';

  setupApplication(app) {
    this.applications.push(app);
  }
  removeApplication(app) {
    const idx = this.applications.findIndex(e => e.id == app.id);
    if (idx != -1) {
      this.applications.splice(idx, 1);
    }
  }
}

module.exports = { Node, Host, Hub, Switch };
