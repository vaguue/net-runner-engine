#!/usr/bin/env node

const Simulator = require('..');
const config = require('./exampleConfig');
const assert = require('assert');
const path = require('path');
const fs = require('fs');

const { fromConfig, Network, Hub, Switch, Host, TCPClient, TCPServer, UDPClient, UDPServer, Ping } = Simulator;

const dstDir = path.resolve(__dirname, 'files');
if (!fs.existsSync(dstDir)) {
  fs.mkdirSync(dstDir);
}

//console.log(JSON.stringify(config, null, 2));

async function f1() {
  setTimeout(() => fromConfig({ config, pcapPath: dstDir}), 10);
}

async function f2() {
  const net = new Network();
  net.setOptions({ verbose: true });
  const hosts = [...Array(3).keys()].map((_, i) => new Host({ x: 100*i, y: 100*i })).map(e => net.addNode(e));
  const hub = new Hub({ x: 0, y: 100, dataRate: '100Mbps', delay: '1ms' });
  net.addNode(hub);
  hosts.map((e, i) => e.connect(hub, { sourceIP: `192.168.1.${i}` }));
  hosts[0].setupApplication(new TCPClient({ dst: '192.168.1.2:3000' }));
  hosts[2].setupApplication(new TCPServer({ dst: '3000' }));
  net.run(dstDir);
  console.log('[*] done');
}

async function f3() {
  const net = new Network({ 
    animeLen: 5, // seconds, default is 10
  });
  const host1 = new Host({ 
    name: 'Alice', // this name will be used in resulting PCAP files, optional (some numbers will be used if not specified, e.g. 0-1-hub-csma-0.pcap)
  });
  const host2 = new Host({ name: 'Bob' });
  host1.setupApplication(new TCPClient({ 
    dst: '192.168.1.3:3000', // accepts dst in format <IP address>:<port>
  }));
  host2.setupApplication(new TCPServer({ 
    dst: '3000', // accepts only port number via dst field,
  }));
  // each host should be added to network BEFORE conneting
  net.addNode(host1); 
  net.addNode(host2);
  // connecting two hosts
  host1.connect(host2, { 
    sourceIP: '192.168.1.2', //IP of host1's device, required for Host node
    targetIP: '192.168.1.3', //IP of host2's device, required for Host node
    dataRate: '1Mbps', //optional, default is 5Mbps
    delay: '1ms', //optional, default is 5ms
  });

  //Openflow switch example
  const swtch = new Switch({ name: 'MyHomeSwitch' });
  net.addNode(swtch);
  const n = 4;
  const half = Math.floor(n / 2);
  const hosts = [...Array(n).keys()].map((_, i) => new Host({ name: `user${i + 1}` }));
  for (let i = 0; i < n; ++i) {
    net.addNode(hosts[i]);
    hosts[i].connect(swtch, {
      sourceIP: `192.168.0.${i + 1}`, //NOTE: Switch does not have IP address, only host
    });
  }
  //setting up UDP clients and servers
  for (let i = 0; i < half; ++i) {
    hosts[i].setupApplication(new UDPClient({ dst: `192.168.0.${i + half}:8888` }));
    hosts[n - i - 1].setupApplication(new UDPServer({ dst: '8888' }));
  }
  console.log(JSON.stringify(net.toObject(), null, 2));
  net.run(dstDir);
  console.log('[*] done');
}

async function f4() {
  const net = new Network({ 
    animeLen: 5,
  });
  const host1 = new Host({ name: 'Pinger' });
  const host2 = new Host({ name: 'PingTarget' });
  host1.setupApplication(new Ping({ dst: '192.168.1.3', interval: '1s' }));
  net.addNode(host1); 
  net.addNode(host2);
  host1.connect(host2, { 
    sourceIP: '192.168.1.2',
    targetIP: '192.168.1.3',
    dataRate: '1Mbps',
    delay: '1ms',
  });
  net.run(dstDir);
  console.log('[*] done');
}

async function f5() {
  const net = new Network({ 
    animeLen: 3,
  });
  const host1 = new Host({ name: 'TcpClient' });
  const host2 = new Host({ name: 'TcpServer' });
  host1.setupApplication(new UDPClient({ 
    dst: '192.168.1.3:3000', 
    //tickInterval: '1ms',
    onTick: ({ time, sendPacket, tick }) => {
      if (time > 1000) {
        const buf = Buffer.from("hello\0");
        sendPacket(buf);
      }
      tick('0.1s');
    },
  }));
  host2.setupApplication(new UDPServer({ 
    dst: 3000,
    onRecieve: ({ address, packet, reply }) => {
      console.log('[*] recieve', address, packet);
      const buf = Buffer.from("world?");
      reply(buf);
    },
  }));
  net.addNode(host1); 
  net.addNode(host2);
  host1.connect(host2, { 
    sourceIP: '192.168.1.2',
    targetIP: '192.168.1.3',
    dataRate: '1Mbps',
    delay: '1ms',
  });
  net.run(dstDir);
  console.log('[*] done');
}

//f1();
//f2();
//f3();
//f4();
f5();
