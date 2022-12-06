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
      if (typeof edge.sourceIP == 'string' && edge.sourceIP.length == 0) {

      }
      else {
        throw Error(`invalid source IP: ${edge.sourceIP}`);
      }
    }
    if (edge.hasOwnProperty('targetIP') && !is.ip(edge.targetIP)) {
      if (typeof edge.targetIP == 'string' && edge.targetIP.length == 0) {

      }
      else {
        throw Error(`invalid target IP: ${edge.targetIP}`);
      }
    }
    if (edge.hasOwnProperty('sourceMask') && !is.ip(edge.sourceMask)) {
      throw Error(`invalid source mask: ${edge.sourceMask}`);
    }
    if (edge.hasOwnProperty('targetMask') && !is.ip(edge.targetMask)) {
      throw Error(`invalid target mask: ${edge.targetMask}`);
    }
    if (edge.hasOwnProperty('dataRate') && !is.dataRate(edge.dataRate)) {
      throw Error(`invalid dataRate: ${edge.dataRate}`);
    }
    if (edge.hasOwnProperty('delay') && !is.time(edge.delay)) {
      throw Error(`invalid delay: ${edge.delay}`);
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

function pointDest(p1, p2) {
  return Math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2);
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
  const defCoords = [];
  let undefI = 0;
  for (let i = 0; i < nodes.length; ++i) {
    const node = nodes[i];
    if (is.number(node.x) && is.number(node.x)) {
      defCoords.push([node.x, node.y]);
    }
    else if (is.number(node.x)) {
      defCoords.push([node.x, 0]);
      undefI++;
    }
    else if (is.number(node.y)) {
      defCoords.push([0, node.y]);
      undefI++;
    }
    else {
      undefI++;
    }
  }
  const middle = Array.from({ length: 2 }, (_, i) => defCoords.map(e => e[i]).reduce((res, e) => res + e, 0)/defCoords.length);
  const rp = Math.max(defCoords.reduce((res, e) => pointDest(middle, e) > res ? pointDest(middle, e) : res, 0) + 200, undefI * 150);
  let curI = 0;
  for (let i = 0; i < nodes.length; ++i) {
    const node = nodes[i];
    if (!is.number(node.x) && !is.number(node.y)) {
      node.x = rp*2*curI/undefI - rp;
      node.y = (curI % 2 == 0 ? -1 : 1) * Math.sqrt(rp**2 - node.x**2);
      curI++;
    }
    else if (!is.number(node.x)) {
      node.x = (curI % 2 == 0 ? -1 : 1) * Math.sqrt(rp**2 - node.y**2);
      curI++;
    }
    else if (!is.number(node.y)) {
      node.y = (curI % 2 == 0 ? -1 : 1) * Math.sqrt(rp**2 - node.x**2);
      curI++;
    }
  }
  return config;
}

module.exports = { validate, autoGen };
