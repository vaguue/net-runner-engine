const is = require('./is');
const { Node, ...Nodes } = require('./nodes');
const { Application, ...Applications } = require('./applications');

const nodeTypes = Object.values(Nodes).map(e => e.type);
const applicationTypes = Object.values(Applications).reduce((res, e) => ({ ...res, [e.type]: e }), {});

function validApplication(app) {
  if (!is.object(app)) {
    throw Error('Application is not an object');
  }
  const { type } = app;
  if (!applicationTypes.hasOwnProperty(type)) {
    throw Error(`Unknown application type ${type}`);
  }
  applicationTypes[type].isValid(app);
  return true;
}

function validateNodes(config) {
  const { nodes } = config;
  if (!is.array(nodes)) {
    throw Error('expected array for nodes property');
  }
  for (let i = 0; i < config.nodes.length; ++i) {
    const node = config.nodes[i];
    if (node.id != i) {
      throw Error(`malformed node id ${node.id}`);
    }
    if (!nodeTypes.includes(node.type)) {
      throw Error(`invalid node type ${node.type}`);
    }
    if (node.hasOwnProperty('applications')) {
      if (!is.array(node.applications)) {
        throw Error('expected array for node applications');
      }
      node.applications.every(validApplication);
    }
  }
  return true;
}

function validateEdges(config) {
  const { edges, nodes } = config;
  if (!is.array(edges)) {
    throw Error('expected array for edges');
  }
  for (const edge of edges) {
    const { source, target } = edge;
    if (!nodes[source] || !nodes[target]) {
      throw Error('edge connects unexisting nodes');
    }
    if (edge.hasOwnProperty('sourceIP') && !is.ip(edge.sourceIP)) {
      throw Error(`invalid source IP: ${edge.sourceIP}`);
    }
    if (edge.hasOwnProperty('targetIP') && !is.ip(edge.targetIP)) {
      throw Error(`invalid target IP: ${edge.targetIP}`);
    }
  }
  return true;
}

function validateOptions(config) {
  return true;
}

function validate(config) {
  try {
    const valid = validateNodes(config) && validateEdges(config) && validateOptions(config);
    return config;
  } catch(e) {
    throw Error(`Invalid network configuration: ${e.message}`);
  }
}

function autoGen(config) {
  const { nodes } = config;
  let n = 1;
  for (let i = 0; i < nodes.length; ++i) {
    if (!nodes[i].name) {
      nodes[i].name = `node${n}`;
      n++;
    }
  }
  for (let i = 0; i < nodes.length; ++i) {
    const node = nodes[i];
    if (Number.isNaN(Number(node.x)) || Number.isNaN(Number(node.y))) {
      node.x = 400*i;
      node.y = 400*i;
    }
  }
  return config;
}

module.exports = { validate, autoGen };
