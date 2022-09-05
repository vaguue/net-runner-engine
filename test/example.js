#!/usr/bin/env node

const Simulator = require('..');
const config = require('./exampleConfig');
const assert = require('assert');
const path = require('path');
const fs = require('fs');

const { fromConfig, Network, Hub, Switch, Host, TCPClient, TCPServer } = Simulator;

const dstDir = path.resolve(__dirname, 'files');
if (!fs.existsSync(dstDir)) {
  fs.mkdirSync(dstDir);
}

console.log(JSON.stringify(config, null, 2));

async function f1() {
  setTimeout(() => fromConfig({ config, pcapPath: dstDir}), 10);
}

async function f2() {
  const net = new Network();
  const hosts = [...Array(3).keys()].map((_, i) => new Host({ x: 100*i, y: 100*i })).map(e => net.addNode(e));
  const hub = new Hub({ x: 0, y: 100 });
  net.addNode(hub);
  hosts.map((e, i) => e.connect(hub, { sourceIP: `192.168.1.${i}` }));
  hosts[0].setupApplication(new TCPClient({ dst: '192.168.1.2', port: '3000' }));
  hosts[2].setupApplication(new TCPServer({ port: '3000' }));
  net.run(dstDir);
}
f1();
