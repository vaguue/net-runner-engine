//const binPath = '../build/Debug/wrapper-native';
const binPath = '../build/Release/wrapper-native';
const { Instance } = require(binPath);
const emu = new Instance();

const { v4: uuidv4 } = require('uuid');

function fromConfig({ pcapPath = '.', config, }) {
  emu.setPcapPath(pcapPath);
  emu.runFromConfig(config, { pcapPath });
}

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

const _extends = function(a, b) {
  function fn() {
    this.constructor = a;
  }
  fn.prototype = b.prototype;
  a.prototype = new fn();
};

function Application({ type, fields, ...rest }) {
  if (fields.some(e => !rest.hasOwnProperty(e))) {
    throw Error(`Can't initialize ${type} with ${JSON.stringify(rest)}`);
  }
  this.type = type;
  this.fields = fields;
  this.data = rest;
  this.id = uuidv4();
}

Application.prototype.toObject = function() {
  return { ...this.data, type: this.type };
}

Application.prototype.clone = function() {
  return new Application({ ...this.toObject() });
}

const TCPClient = (function(Application) {
  _extends(TCPClient, Application);
  function TCPClient(args = {}) {
    Application.call(this, { ...args, type: 'tcp-client', fields: ['dst'] });
  }
  return TCPClient;
})(Application);
const TCPServer = (function(Application) {
  _extends(TCPServer, Application);
  function TCPServer(args = {}) {
    Application.call(this, { ...args, type: 'tcp-server', fields: ['dst'] });
  }
  return TCPServer;
})(Application);
const UDPClient = (function(Application) {
  _extends(UDPClient, Application);
  function UDPClient(args = {}) {
    Application.call(this, { ...args, type: 'udp-client', fields: ['dst'] });
  }
  return UDPClient;
})(Application);
const UDPServer = (function(Application) {
  _extends(UDPServer, Application);
  function UDPServer(args = {}) {
    Application.call(this, { ...args, type: 'udp-server', fields: ['dst'] });
  }
  return UDPServer;
})(Application);
const Ping = (function(Application) {
  _extends(Ping, Application);
  function Ping(args = {}) {
    Application.call(this, { ...args, type: 'ping', fields: ['dst'] });
  }
  return Ping;
})(Application);
const Sink = (function(Application) {
  _extends(Sink, Application);
  function Sink(args = {}) {
    Application.call(this, { ...args, type: 'ping', fields: ['dst'] });
  }
  return Sink;
})(Application);


function Node({ type = 'pc', name = null, x = 0, y = 0 } = {}) {
  this.id = uuidv4();
  this.type = type;
  this.name = name;
  this.x = x;
  this.y = y;
  this.applications = [];
  this.parentNetwork = null;
  this.parentNetworkId = null;
}

const nodeFields = ['id', 'type', 'x', 'y', 'applications'];
Node.prototype.toObject = function() {
  let res = getFromObject(this, nodeFields);
  res.applications = res.applications.map(e => e.toObject());
  if (this.additionalFields) {
    res = { ...res, ...pickFromObject(this, this.additionalFields) };
  }
  if (this.name !== null) {
    res.name = this.name
  }
  return res;
}

Node.prototype.clone = function() {
  return new Node(this.toObject());
}

Node.prototype.connect = function(dstNode, options = {}) {
  if (!dstNode instanceof Node) {
    throw Error('Destionation is not an instance of Node class');
  }
  if (this.parentNetworkId === null || this.parentNetworkId != dstNode.parentNetworkId) {
    throw Error('Nodes belong to different networks of are not in any network');
  }
  this.parentNetwork.connectNodes(this, dstNode, options);
}

Node.prototype.disconnect = function(dstNode) {
  if (!dstNode instanceof Node) {
    throw Error('Destionation is not an instance of Node class');
  }
  if (this.parentNetworkId === null || this.parentNetworkId != dstNode.parentNetworkId) {
    throw Error('Nodes belong to different networks of are not in any network');
  }
  this.parentNetwork.disconnectNodes(this, dstNode);
}

const Switch = (function(Node) {
  _extends(Switch, Node);
  function Switch(args = {}) {
    Node.call(this, args);
    this.type = 'switch';
  }
  Switch.prototype.setupApplication = null;
  return Switch;
})(Node);
const Hub = (function(Node) {
  _extends(Hub, Node);
  function Hub(args = {}) {
    Node.call(this);
    if (args.delay) {
      this.delay = args.delay;
    }
    if (args.dataRate) {
      this.dataRate = args.dataRate;
    }
    this.additionalFields = ['delay', 'dataRate'];
    this.type = 'hub';
  }
  Hub.prototype.setupApplication = null;
  return Hub;
})(Node);
const Host = (function(Node) {
  _extends(Host, Node);
  function Host(args = {}) {
    Node.call(this, args);
    this.type = 'pc';
  }
  return Host;
})(Node);

Host.prototype.setupApplication = function(app) {
  if (!app instanceof Application) {
    throw Error('Argument is not application');
  }
  this.applications.push(app);
}

Host.prototype.removeApplication = function(app) {
  if (!app instanceof Application) {
    throw Error('Argument is not application');
  }
  const idx = this.applicaitons.findIndex(e => e.id == app.id);
  if (idx != -1) {
    this.applications.splice(idx, 1);
  }
}

function Network(options = {}) {
  this.id = uuidv4();
  this.config = {
    nodes: [],
    edges: [],
    options,
  };
}

Network.prototype.setOptions = function(options = {}) {
  this.config.options = { ...this.config.options, ...options };
  return this.options;
}

Network.prototype.isValidNode = function(node) {
  return (node instanceof Node && node.parentNetworkId == this.id);
}

Network.prototype.addNode = function(node) {
  if (node instanceof Node) {
    node.parentNetwork = this;
    node.parentNetworkId = this.id;
    //this.config.nodes.push(node.toObject());
    this.config.nodes.push(node);
    return node;
  }
  throw Error('Network.addNode accepts Node object');
}

Network.prototype.removeNode = function(node) {
  if (node instanceof Node) {
    node.parentNetwork = null;
    node.parentNetworkId = null;
    const idx = this.config.nodes.findIndex(e => e.id == node.id);
    if (idx != -1) {
      this.config.nodes.splice(idx, 1);
    }
    return node;
  }
  throw Error('Network.addNode accepts Node object');
}

Network.prototype.connectNodes = function(nodeA, nodeB, options = {}) {
  if (!this.isValidNode(nodeA) || !this.isValidNode(nodeB)) {
    throw Error('Network.connectNodes accepts Node object');
  }
  const con = { source: nodeA.id,
    target: nodeB.id,
    type: 'default',
    ...options,
  };
  this.config.edges.push(con);
  return con;
}

Network.prototype.disconnectNodes = function(nodeA, nodeB) {
  if (!this.isValidNode(nodeA) || !this.isValidNode(nodeB)) {
    throw Error('Network.disconnectNodes accepts Node object');
  }
  const { id: idA } = nodeA;
  const { id: idB } = nodeB;
  const conIdx = this.config.edges.findIndex(e => {
    const [source, target] = [e.source, e.target];
    return (source == idA && target == idB) || (source == idB && target == idA);
  });
  if (conIdx != -1) {
    this.config.edges.splice(conIdx, 1);
  }
}

Network.prototype.toObject = function() {
  const config = { ...this.config, nodes: [...this.config.nodes], edges: [...this.config.edges] };
  for (let i = 0; i < config.nodes.length; ++i) {
    config.nodes[i] = this.config.nodes[i].toObject();
    const { id } = config.nodes[i];
    config.nodes[i].id = i;
    for (const con of config.edges) {
      if (con.source == id) {
        con.source = i;
      }
      if (con.target == id) {
        con.target = i;
      }
    }
  }
  return config;
}

Network.prototype.run = function(pcapPath) {
  return fromConfig({ pcapPath, config: this.toObject() });
}

module.exports = { Network, Switch, Hub, Host, fromConfig, TCPClient, TCPServer, Ping, Sink, UDPClient, UDPServer };
